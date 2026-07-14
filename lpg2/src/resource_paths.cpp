#include "resource_paths.h"

#include "lpg_resource_root.h"

#include <cstdlib>
#include <filesystem>
#include <string>
#include <vector>

#if defined(_WIN32)
#include <windows.h>
#elif defined(__APPLE__)
#include <mach-o/dyld.h>
#else
#include <unistd.h>
#endif

namespace
{
std::filesystem::path ExecutablePath(const char *argv0)
{
#if defined(_WIN32)
    std::vector<char> buffer(32768);
    const DWORD length =
        GetModuleFileNameA(NULL, buffer.data(), static_cast<DWORD>(buffer.size()));
    if (length > 0 && static_cast<size_t>(length) < buffer.size())
        return std::filesystem::path(std::string(buffer.data(), length));
#elif defined(__APPLE__)
    uint32_t size = 0;
    _NSGetExecutablePath(NULL, &size);
    std::vector<char> buffer(size);
    if (_NSGetExecutablePath(buffer.data(), &size) == 0)
        return std::filesystem::path(buffer.data());
#else
    std::vector<char> buffer(4096);
    const ssize_t length =
        readlink("/proc/self/exe", buffer.data(), buffer.size() - 1);
    if (length > 0)
        return std::filesystem::path(
            std::string(buffer.data(), static_cast<size_t>(length)));
#endif

    std::filesystem::path argument(argv0 == NULL ? "" : argv0);
    if (argument.has_parent_path())
        return std::filesystem::absolute(argument);

    const char *path_value = std::getenv("PATH");
    if (path_value != NULL)
    {
#if defined(_WIN32)
        const char separator = ';';
#else
        const char separator = ':';
#endif
        std::string paths(path_value);
        size_t start = 0;
        while (start <= paths.size())
        {
            const size_t end = paths.find(separator, start);
            const std::filesystem::path candidate =
                std::filesystem::path(paths.substr(start, end - start)) /
                argument;
            std::error_code error;
            if (std::filesystem::is_regular_file(candidate, error))
                return std::filesystem::absolute(candidate);
            if (end == std::string::npos)
                break;
            start = end + 1;
        }
    }

    return std::filesystem::absolute(argument);
}

bool IsResourceRoot(const std::filesystem::path &path)
{
    std::error_code error;
    return std::filesystem::is_directory(path / "templates", error) &&
           std::filesystem::is_directory(path / "include", error);
}

std::string FirstExistingResourceRoot(
    const std::vector<std::filesystem::path> &candidates)
{
    for (const auto &candidate : candidates)
    {
        if (candidate.empty())
            continue;
        std::error_code error;
        const auto canonical =
            std::filesystem::weakly_canonical(candidate, error);
        const auto &path = error ? candidate : canonical;
        if (IsResourceRoot(path))
            return path.string();
    }
    return std::string();
}
}

std::string FindLpgResourceRoot(const char *argv0)
{
    std::vector<std::filesystem::path> candidates;

    // Explicit override for packaging, tests, and out-of-tree CI builds.
    if (const char *env_root = std::getenv("LPG2_RESOURCE_ROOT"))
    {
        if (env_root[0] != '\0')
            candidates.emplace_back(env_root);
    }

#ifdef LPG2_DEFAULT_RESOURCE_ROOT
    // Compile-time path to the source-tree (or install-time) template root.
    // This is the primary fix for cmake -B $RUNNER_TEMP out-of-tree builds.
    candidates.emplace_back(LPG2_DEFAULT_RESOURCE_ROOT);
#endif

    if (argv0 != NULL && argv0[0] != '\0')
    {
        std::error_code error;
        const std::filesystem::path executable =
            std::filesystem::weakly_canonical(ExecutablePath(argv0), error);
        const std::filesystem::path executable_directory =
            (error ? ExecutablePath(argv0) : executable).parent_path();

        candidates.push_back(executable_directory.parent_path() /
                             "share/lpg2/lpg-generator-templates-2.1.00");

        // Walk far enough for nested build dirs (e.g. build/tests/).
        std::filesystem::path ancestor = executable_directory;
        for (int i = 0; i < 8 && ! ancestor.empty(); ++i)
        {
            candidates.push_back(ancestor / "lpg-generator-templates-2.1.00");
            ancestor = ancestor.parent_path();
        }
    }

    return FirstExistingResourceRoot(candidates);
}
