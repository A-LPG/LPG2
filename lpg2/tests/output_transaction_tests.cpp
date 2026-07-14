#include "output_transaction.h"

#include <chrono>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <stdexcept>
#include <string>

namespace
{
void Check(bool condition)
{
    if (! condition)
        throw std::runtime_error("output transaction regression assertion failed");
}

std::string Read(const std::filesystem::path &path)
{
    std::ifstream input(path, std::ios::binary);
    return std::string(std::istreambuf_iterator<char>(input),
                       std::istreambuf_iterator<char>());
}

void Stage(const std::filesystem::path &path, const char *contents)
{
    FILE *file = OutputTransaction::Instance().Open(path.string().c_str());
    Check(file != nullptr);
    Check(fputs(contents, file) >= 0);
    Check(fclose(file) == 0);
}
}

int main()
{
    const std::filesystem::path root =
        std::filesystem::temp_directory_path() /
        ("lpg2-output-test-" +
         std::to_string(
             std::chrono::steady_clock::now().time_since_epoch().count()));
    std::filesystem::create_directories(root);
    const std::filesystem::path output = root / "parser.h";

    {
        std::ofstream initial(output, std::ios::binary);
        initial << "old";
    }

    Stage(output, "discarded");
    Check(Read(output) == "old");
    OutputTransaction::Instance().Rollback();
    Check(Read(output) == "old");

    Stage(output, "new");
    Check(Read(output) == "old");
    OutputTransaction::Instance().Commit();
    Check(Read(output) == "new");

    for (const auto &entry : std::filesystem::directory_iterator(root))
        Check(entry.path().filename() == output.filename());

    std::filesystem::remove_all(root);
    return 0;
}
