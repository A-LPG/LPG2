#include "resource_paths.h"

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
}

std::string FindLpgResourceRoot(const char *argv0)
{
    if (argv0 == NULL || argv0[0] == '\0')
        return std::string();

    std::error_code error;
    const std::filesystem::path executable =
        std::filesystem::weakly_canonical(ExecutablePath(argv0), error);
    const std::filesystem::path executable_directory =
        (error ? ExecutablePath(argv0) : executable).parent_path();

    const std::filesystem::path installed =
        executable_directory.parent_path() /
        "share/lpg2/lpg-generator-templates-2.1.00";
    if (IsResourceRoot(installed))
        return installed.string();

    std::filesystem::path ancestor = executable_directory;
    for (int i = 0; i < 5 && ! ancestor.empty(); ++i)
    {
        const std::filesystem::path source_tree =
            ancestor / "lpg-generator-templates-2.1.00";
        if (IsResourceRoot(source_tree))
            return source_tree.string();
        ancestor = ancestor.parent_path();
    }

    return std::string();
}
