/**
 * @author Michal Witanowski
 * @brief  VFS creation tool.
 */

#include "../vfs.hpp"

enum class Direction
{
    Internal,
    Up,
    Down,
};

void PrintUsage()
{
    std::cout << "Usage: vcp [vfs image] dir [source]... [destination]..." << std::endl;
}

#define BUFFER_SIZE 64*1024
static unsigned char buffer[BUFFER_SIZE];

int CopyDown(Vfs& vfs, int argc, char** argv)
{
    for (int i = 3; i < argc - 1; ++i)
    {
        std::string source = argv[i];
        std::string dest = argv[argc - 1];
        dest += '/';
        dest += source;

        /// open source file
        VfsFile* srcFile = vfs.OpenFile(source, false);
        if (srcFile == 0)
        {
            std::cout << "Failed to open '" << source << "' file" << std::endl;
            return 1;
        }

        /// create destination file
        FILE* destFile = fopen(dest.c_str(), "wb");
        if (destFile == 0)
        {
            std::cout << "Failed to open '" << dest << "' file for writing" << std::endl;
            vfs.Close(srcFile);
            return 1;
        }

        /// copy
        size_t bytesRead;
        while ((bytesRead = srcFile->Read(BUFFER_SIZE, buffer)) > 0)
        {
            if (fwrite(buffer, 1, bytesRead, destFile) < bytesRead)
            {
                std::cout << "Failed to write '" << dest << "'. Skipping." << std::endl;
                break;
            }
        }

        vfs.Close(srcFile);
        fclose(destFile);
        std::cout << "Copied '" << source << "' to '" << dest << "'" << std::endl;
    }

    return 0;
}

int CopyUp(Vfs& vfs, int argc, char** argv)
{
    bool singleCopy = true;
    PathInfo info;
    if (vfs.GetInfo(argv[argc - 1], info))
    {
        if (info.directory)
        {
            singleCopy = false;
        }
    }

    for (int i = 3; i < argc - 1; ++i)
    {
        std::string source = argv[i];
        std::string dest = argv[argc - 1];

        if (!singleCopy)
        {
            dest += '/';
            dest += source;
        }

        /// open source file
        FILE* srcFile = fopen(source.c_str(), "rb");
        if (srcFile == 0)
        {
            std::cout << "Failed to open '" << source << "' file" << std::endl;
            return 1;
        }

        /// create destination file
        VfsFile* destFile = vfs.OpenFile(dest, true);
        if (destFile == 0)
        {
            std::cout << "Failed to open '" << dest << "' file for writing" << std::endl;
            fclose(srcFile);
            return 1;
        }

        /// copy
        size_t bytesRead;
        while ((bytesRead = fread(buffer, 1, BUFFER_SIZE, srcFile)) > 0)
        {
            if (destFile->Write(bytesRead, buffer) < bytesRead)
            {
                std::cout << "Failed to write '" << dest << "'. Skipping." << std::endl;
                break;
            }
        }

        vfs.Close(destFile);
        fclose(srcFile);
        std::cout << "Copied '" << source << "' to '" << dest << "'" << std::endl;

        if (singleCopy)
            break;
    }

    return 0;
}

int CopyInternal(Vfs& vfs, int argc, char** argv)
{
    bool singleCopy = true;
    PathInfo info;
    if (vfs.GetInfo(argv[argc - 1], info))
    {
        if (info.directory)
        {
            singleCopy = false;
        }
    }

    for (int i = 3; i < argc - 1; ++i)
    {
        std::string source = argv[i];
        std::string dest = argv[argc - 1];

        if (!singleCopy)
        {
            dest += '/';
            dest += source;
        }

        /// open source file
        VfsFile* srcFile = vfs.OpenFile(source, false);
        if (srcFile == 0)
        {
            std::cout << "Failed to open '" << source << "' file" << std::endl;
            return 1;
        }

        /// create destination file
        VfsFile* destFile = vfs.OpenFile(dest, true);
        if (destFile == 0)
        {
            std::cout << "Failed to open '" << dest << "' file for writing" << std::endl;
            vfs.Close(srcFile);
            return 1;
        }

        /// copy
        size_t bytesRead;
        while ((bytesRead = srcFile->Read(BUFFER_SIZE, buffer)) > 0)
        {
            if (destFile->Write(bytesRead, buffer) < bytesRead)
            {
                std::cout << "Failed to write '" << dest << "'. Skipping." << std::endl;
                break;
            }
        }

        vfs.Close(destFile);
        vfs.Close(srcFile);
        std::cout << "Copied '" << source << "' to '" << dest << "'" << std::endl;

        if (singleCopy)
            break;
    }

    return 0;
}

int main(int argc, char** argv)
{
    if (argc < 5)
    {
        PrintUsage();
        return 1;
    }

    Direction dir;
    std::string dirStr = argv[2];
    if (dirStr == "-u" || dirStr == "--up")
        dir = Direction::Up;
    else if (dirStr == "-d" || dirStr == "--down")
        dir = Direction::Down;
    else if (dirStr == "-i" || dirStr == "--internal" || dirStr == "--inter")
        dir = Direction::Internal;
    else
    {
        std::cout << "Invalid direction flag. Valid values are:" << std::endl <<
                     "-u, --up        - upload file(s) to the VFS" << std::endl <<
                     "-d, --down      - download file(s) from the VFS" << std::endl <<
                     "-i, --internal  - internal file copy" << std::endl;
        return 1;
    }

    Vfs vfs;
    if (!vfs.Open(argv[1]))
    {
        return 1;
    }

    switch (dir)
    {
    case Direction::Up:
        return CopyUp(vfs, argc, argv);
    case Direction::Down:
        return CopyDown(vfs, argc, argv);
    case Direction::Internal:
        return CopyInternal(vfs, argc, argv);
    }

    return 0;
}
