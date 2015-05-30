/**
 * @author Michal Witanowski
 */

#include "vfs.hpp"
#include <assert.h>
#include <iostream>
#include <sstream>

#define ROOT_INODE_INDEX 0


uint32 Vfs::ReserveBitmap(uint32 firstBitmapBlock, uint32 bitmapSize)
{
    fseek(mImage, VFS_BLOCK_SIZE * firstBitmapBlock, SEEK_SET);

    // iterate bitmap bytes
    for (uint32 i = 0; i < bitmapSize * VFS_BLOCK_SIZE; ++i) 
    {
        uint8 byte;
        assert(1 == fread(&byte, 1, 1, mImage));

        if (byte == 0xFF)
            continue;

        uint8 mask = 0x1;
        // iterate bitmap's byte bits
        for (uint32 j = 0; j < 8; ++j)
        {
            if ((byte & mask) == 0)
            {
                byte |= mask;
                fseek(mImage, -1, SEEK_CUR);
                assert(1 == fwrite(&byte, 1, 1, mImage));
                return 8 * i + j;
            }
            mask <<= 1;
        }
    }

    return INVALID_INDEX;
}

void Vfs::ReleaseBitmap(uint32 firstBitmapBlock, uint32 bitmapSize, uint32 id)
{
    assert(id < bitmapSize);
    uint32 byteOffset = VFS_BLOCK_SIZE * firstBitmapBlock + id / 8;
    uint8 mask = 1 << (id % 8);

    uint8 byte;
    fseek(mImage, byteOffset, SEEK_SET);
    assert(1 == fread(&byte, 1, 1, mImage));
    assert((byte & mask) == mask);

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

uint32 Vfs::GetINodeByPath(const std::string& path, bool getParent)
{
    std::vector<std::string> dirs;

    std::string dir;
    std::istringstream pathStream(path);
    while (std::getline(pathStream, dir, '/'))
    {
        if (dir.length() > 0)
            dirs.push_back(dir);
    }

    uint32 currInode = 0;
    for (const auto& dir : dirs)
    {
        // TODO: find object "dir" in directory "currInode"
    }

    return 0;
    // return INVALID_INDEX;
}

void Vfs::WriteINode(uint32 id, const INode& inode)
{
    uint32 offset = 1 + mSuperblock.dataBitmapBlocks + mSuperblock.inodeBitmapBlocks;
    fseek(mImage, VFS_BLOCK_SIZE * offset + id * sizeof(INode), SEEK_SET);
    assert(fwrite(&inode, sizeof(INode), 1, mImage) == 1);
}

void Vfs::ReadINode(uint32 id, INode& inode)
{
    uint32 offset = 1 + mSuperblock.dataBitmapBlocks + mSuperblock.inodeBitmapBlocks;
    fseek(mImage, VFS_BLOCK_SIZE * offset + id * sizeof(INode), SEEK_SET);
    assert(fread(&inode, sizeof(INode), 1, mImage) == 1);
}



Vfs::~Vfs()
{
    if (mImage)
        fclose(mImage);
}

Vfs::Vfs(const std::string& vfsPath)
{
    if (mImage)
        fclose(mImage);

    mImage = fopen(vfsPath.c_str(), "w+b");
}

bool Vfs::Init(uint32 size)
{
    if (mImage == 0)
    {
        printf("VFS is not open\n");
        return false;
    }

    // init superblock
    mSuperblock.magic = VFS_MAGIC;
    mSuperblock.blocks = CeilDivide<uint32>(size, VFS_BLOCK_SIZE);
    mSuperblock.vfsSize = mSuperblock.blocks * VFS_BLOCK_SIZE;
    mSuperblock.inodeBlocks = CeilDivide<uint32>(mSuperblock.blocks,
                                                 VFS_BLOCK_SIZE / VFS_INODE_SIZE);
    mSuperblock.dataBitmapBlocks = CeilDivide<uint32>(mSuperblock.blocks, VFS_BLOCK_SIZE * 8);
    mSuperblock.inodeBitmapBlocks = mSuperblock.dataBitmapBlocks;
    mSuperblock.firstDataBlock = 1 + mSuperblock.dataBitmapBlocks + mSuperblock.inodeBitmapBlocks;

    // clear VFS file
    static uint8 clearBlock[VFS_BLOCK_SIZE] = { 0x0 };
    fseek(mImage, 0, SEEK_SET);
    for (uint32 i = 0; i < mSuperblock.blocks; ++i)
    {
        assert(1 == fwrite(clearBlock, VFS_BLOCK_SIZE, 1, mImage));
    }

    // write superblock
    fseek(mImage, 0, SEEK_SET);
    fwrite(&mSuperblock, sizeof(Superblock), 1, mImage);

    assert(ReserveINode() == 0);
    INode rootInode;
    rootInode.type = INodeType::Directory;
    WriteINode(ROOT_INODE_INDEX, rootInode);

    return true;
}

VfsFile* Vfs::OpenFile(const std::string& path, bool create)
{
    if (create)
    {
        uint32 inodeId = GetINodeByPath(path, true);
    }
    else
    {

    }

    // 1. find parent directory inode
    // 2. find the file name in the directory
    //      a) if not found and create == true
    //          * allocate file inode and add to the directory
    //      b) if not found and create == false
    //          * return error
    // return new VfsFile(this, 0); // TODO

    return nullptr;
}

bool Vfs::Close(VfsFile* file)
{
    // TODO: write Inode
    delete file;
    return true;
}

bool Vfs::CreateDir(const std::string& path)
{
    uint32 parentInodeID = GetINodeByPath(path, true);
    if (parentInodeID == INVALID_INDEX)
    {
        LOG_ERROR("Invalid path: " << path);
        return false;
    }

    uint32 inodeID = ReserveINode();
    if (inodeID == INVALID_INDEX)
    {
        LOG_ERROR("Failed to reserve inode for directory");
        return false;
    }

    INode inode;
    inode.type = INodeType::Directory;
    WriteINode(inodeID, inode);

    INode parentInode;
    ReadINode(parentInodeID, parentInode);
    VfsFile parentDirFile(this, parentInodeID, parentInode);

    // TODO: update parent dir
    uint8 testData[5000];
    memset(testData, 'u', 5000);
    parentDirFile.WriteOffset(5000, 4090, testData);

    WriteINode(parentInodeID, parentInode);
    return true;
}

bool Vfs::Rename(const std::string& src, const std::string& dest)
{
    uint32 oldParentInode = GetINodeByPath(src, true);
    if (oldParentInode == INVALID_INDEX)
    {
        LOG_ERROR("Invalid path: " << src);
        return false;
    }

    uint32 newParentInode = GetINodeByPath(dest, true);
    if (newParentInode == INVALID_INDEX)
    {
        LOG_ERROR("Invalid path: " << dest);
        return false;
    }

    // TODO

    return true;
}

bool Vfs::Remove(const std::string& path)
{
    // TODO

    return true;
}

bool Vfs::List(std::vector<std::string>& nodes)
{
    // TODO

    return true;
}
