/**
 * @author Michal Witanowski
 * @brief  VFS creation tool.
 */

#include "../vfs.hpp"

void PrintUsage()
{
    std::cout << "Usage: vmkfs [size] [path]" << std::endl;
}

int main(int argc, char** argv)
{
    if (argc < 3)
    {
        PrintUsage();
        return 1;
    }

    uint32 size = atoi(argv[1]);
    std::string path = argv[2];

    if (size == 0)
    {
        std::cout << "Invalid filesystem size" << std::endl;
        return 1;
    }

    Vfs vfs;
    if (!vfs.Init(path, size))
    {
        return 1;
    }

    std::cout << "Filesytem '" << path << "' created" << std::endl;
    return 0;
}
