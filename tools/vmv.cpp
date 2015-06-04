/**
 * @author Michal Witanowski
 * @brief  Move tool for VFS.
 */

#include "../vfs.hpp"

void PrintUsage()
{
    std::cout << "Usage: vmv [vfs image] [src] [dest]" << std::endl;
}

int main(int argc, char** argv)
{
    if (argc < 4)
    {
        PrintUsage();
        return 1;
    }

    Vfs vfs;
    if (!vfs.Open(argv[1]))
    {
        return 1;
    }

    if (vfs.Rename(argv[2], argv[3]))
        std::cout << argv[2] << " -> " << argv[3] << std::endl;
    else
        std::cout << "Failed to rename path '" << argv[2] << "'" << std::endl;

    return 0;
}
