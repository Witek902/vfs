/**
 * @author Michal Witanowski
 * @brief  Directory creation tool for VFS.
 */

#include "../vfs.hpp"

void PrintUsage()
{
    std::cout << "Usage: vmkdir [vfs image] [path]..." << std::endl;
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
        if (vfs.CreateDir(argv[i]))
            std::cout << "Directory '" << argv[i] << "' created" << std::endl;
        else
            std::cout << "Failed to create '" << argv[i] << "' directory" << std::endl;
    }

    return 0;
}
