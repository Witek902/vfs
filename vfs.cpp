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
    if (mImage)
        fclose(mImage);
}

Vfs::Vfs(const char* vfsPath)
{
    if (mImage)
        fclose(mImage);

    mImage = fopen(vfsPath, "w+b");
}

// calculate ceil(a/b)
inline uint32 CeilDivide(uint32 a, uint32 b)
{
    return (a / b) + ((a % b > 0) ? 1 : 0);
}

bool Vfs::Init(uint32 size)
{
    if (mImage == 0)
    {
        printf("VFS is not open\n");
        return false;
    }

    // clear VFS file
    static char clearBlock[VFS_BLOCK_SIZE] = { NULL };
    for (uint32 i = 0; i < mSuperblock.blocks; ++i)
    {
        fwrite(clearBlock, VFS_BLOCK_SIZE, 1, mImage);
    }

    // init superblock
    mSuperblock.magic = VFS_MAGIC;
    mSuperblock.blocks = CeilDivide(size, VFS_BLOCK_SIZE);
    mSuperblock.vfsSize = mSuperblock.blocks * VFS_BLOCK_SIZE;
    mSuperblock.inodeBlocks = CeilDivide(mSuperblock.blocks, VFS_BLOCK_SIZE / VFS_INODE_SIZE);
    mSuperblock.dataBitmapBlocks = CeilDivide(mSuperblock.blocks, VFS_BLOCK_SIZE * 8);
    mSuperblock.inodeBitmapBlocks = mSuperblock.dataBitmapBlocks;

    fseek(mImage, 0, SEEK_SET);
    fwrite(&mSuperblock, sizeof(Superblock), 1, mImage);


    return true;
}

VfsFile* Vfs::OpenFile(const char* path, bool create)
{
    // 1. find parent directory inode
    // 2. find the file name in the directory
    //      a) if not found and create == true
    //          * allocate file inode and add to the directory
    //      b) if not found and create == false
    //          * return error
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
