/**
 * @author Michal Witanowski
 */

#include "vfs.hpp"
#include <assert.h>


uint32 Vfs::ReserveBitmap(uint32 firstBitmapBlock, uint32 bitmapSize)
{
    fseek(mImage, VFS_BLOCK_SIZE * firstBitmapBlock, SEEK_SET);
    for (uint32 i = 0; i < bitmapSize / 8; ++i)
    {
        uint8 byte;
        assert(1 == fread(&byte, 1, 1, mImage));

        if (byte == 0xFF)
            continue;

        // TODO
    }

    return static_cast<uint32>(-1);
}

void Vfs::ReleaseBitmap(uint32 firstBitmapBlock, uint32 bitmapSize, uint32 id)
{
    assert(id < bitmapSize);
    uint32 byteOffset = VFS_BLOCK_SIZE * firstBitmapBlock + id / 8;
    uint8 mask = 1 << (id % 8);

    uint8 byte;
    fseek(mImage, byteOffset, SEEK_SET);
    assert(1 == fread(&byte, 1, 1, mImage));
    assert(byte & mask == mask);

    byte &= ~mask;

    fseek(mImage, byteOffset, SEEK_SET);
    assert(1 == fwrite(&byte, 1, 1, mImage));
}

uint32 Vfs::ReserveBlock()
{
    return ReserveBitmap(mSuperblock.inodeBitmapBlocks + 1, mSuperblock.dataBitmapBlocks);
}

void Vfs::ReleaseBlock(uint32 id)
{
    ReleaseBitmap(mSuperblock.inodeBitmapBlocks + 1, mSuperblock.dataBitmapBlocks, id);
}

uint32 Vfs::ReserveINode()
{
    return ReserveBitmap(1, mSuperblock.inodeBitmapBlocks);
}

void Vfs::ReleaseINode(uint32 id)
{
    ReleaseBitmap(1, mSuperblock.inodeBitmapBlocks, id);
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
    return VfsFile();
}

bool Vfs::Close(VfsFile* file)
{
    delete file;
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
