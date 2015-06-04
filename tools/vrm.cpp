/**
 * @author Michal Witanowski
 * @brief  Removal tool for VFS.
 */

#include "../vfs.hpp"

void PrintUsage()
{
    std::cout << "Usage: vrm [vfs image] [path]..." << std::endl;
}

int main(int argc, char** argv)
{
    if (argc < 3)
    {
        PrintUsage();
        return 1;
    }

    Vfs vfs;
    if (!vfs.Open(argv[1]))
    {
        return 1;
    }

    for (int i = 2; i < argc; ++i)
    {
        if (vfs.Remove(argv[i]))
            std::cout << "Path '" << argv[i] << "' removed" << std::endl;
        else
            std::cout << "Failed to remove '" << argv[i] << "' path" << std::endl;
    }

    return 0;
}
