/**
 * @author Michal Witanowski
 * @brief  Directory listing tool for VFS.
 */

#include "../vfs.hpp"

void PrintUsage()
{
    std::cout << "Usage: vls [vfs image] [path]..." << std::endl;
}

int main(int argc, char** argv)
{
    if (argc < 2)
    {
        PrintUsage();
        return 1;
    }

    Vfs vfs;
    if (!vfs.Open(argv[1]))
    {
        return 1;
    }

    std::vector<std::string> list;

    // list root
    if (argc == 2)
    {
        vfs.DebugPrint();
    }

    for (int i = 2; i < argc; ++i)
    {
        if (vfs.List(argv[i], list))
        {
            std::cout << argv[i] << ':' << std::endl;
            for (const auto& obj : list)
                std::cout << obj << ' ';
            std::cout << std::endl;
        }
        else
            std::cout << "Failed to list path '" << argv[i] << "'" << std::endl;
    }

    return 0;
}
