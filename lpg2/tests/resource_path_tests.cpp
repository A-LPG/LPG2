#include "resource_paths.h"

#include <filesystem>
#include <iostream>

int main(int argc, char *argv[])
{
    const std::string root = FindLpgResourceRoot(argc > 0 ? argv[0] : "");
    if (root.empty() ||
        ! std::filesystem::is_directory(
            std::filesystem::path(root) / "templates/rt_cpp") ||
        ! std::filesystem::is_directory(
            std::filesystem::path(root) / "include/rt_cpp"))
    {
        std::cerr << "Unable to discover LPG2 resources from " << argv[0]
                  << "; root=" << root << '\n';
        return 1;
    }
    return 0;
}
