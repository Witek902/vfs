/**
 * @author Michal Witanowski
 */

#include "vfs.hpp"


uint32 Vfs::ReserveBitmap(uint32 firstBitmapBlock, uint32 bitmapSize)
{
    return static_cast<uint32>(-1);
}

void Vfs::ReleaseBitmap(uint32 firstBitmapBlock, uint32 bitmapSize, uint32 id)
{

}

uint32 Vfs::ReserveBlock()
{
    return static_cast<uint32>(-1);
}

void Vfs::ReleaseBlock(uint32 id)
{

}

uint32 Vfs::ReserveINode()
{
    return static_cast<uint32>(-1);
}

void Vfs::ReleaseINode(uint32 id)
{

}


Vfs::~Vfs()
{

}

Vfs::Vfs(const char* vfsPath)
{
    if (mImage)
        fclose(mImage);

    mImage = fopen(vfsPath, "rwb");
}

bool Vfs::Init(size_t size)
{
    if (mImage == 0)
    {
        printf("VFS is not open\n");
        return false;
    }

    return true;
}

VfsFile* Vfs::OpenFile(const char* path)
{
    return nullptr;
}

bool Vfs::Close(VfsFile* file)
{
    return true;
}

bool Vfs::CreateDir(const char* path)
{
    return true;
}

bool Vfs::Rename(const char* src, const char* dest)
{
    return true;
}

bool Vfs::Remove(const char* path)
{
    return true;
}

bool Vfs::List(std::vector<std::string>& nodes)
{
    return true;
}
