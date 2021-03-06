/**
 * @author Michal Witanowski
 */

#include "vfs.hpp"

#include <assert.h>
#include <string.h>
#include <iostream>
#include <sstream>
#include <stack>
#include <iomanip>

#define ROOT_INODE_INDEX 0

// NOTE: this is slow - O(n) worst case time complexity
uint32 Vfs::ReserveBitmap(uint32 firstBitmapBlock, uint32 bitmapSize)
{
    fseek(mImage, VFS_BLOCK_SIZE * firstBitmapBlock, SEEK_SET);

    // iterate bitmap bytes
    for (uint32 i = 0; i < bitmapSize / 8; ++i) 
    {
        uint8 byte = 0;
        VFS_ASSERT(1 == fread(&byte, 1, 1, mImage));

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
                VFS_ASSERT(1 == fwrite(&byte, 1, 1, mImage));
                return 8 * i + j;
            }
            mask <<= 1;
        }
    }

    return INVALID_INDEX;
}

// NOTE: this is slow - O(n) worst case time complexity
void Vfs::ReleaseBitmap(uint32 firstBitmapBlock, uint32 bitmapSize, uint32 id)
{
    VFS_ASSERT(id < bitmapSize);
    uint32 byteOffset = VFS_BLOCK_SIZE * firstBitmapBlock + id / 8;
    uint8 mask = 1 << (id % 8);

    uint8 byte;
    fseek(mImage, byteOffset, SEEK_SET);
    VFS_ASSERT(1 == fread(&byte, 1, 1, mImage));
    VFS_ASSERT((byte & mask) == mask);

    byte &= ~mask;

    fseek(mImage, byteOffset, SEEK_SET);
    VFS_ASSERT(1 == fwrite(&byte, 1, 1, mImage));
}

uint32 Vfs::ReserveBlock()
{
    return ReserveBitmap(mSuperblock.inodeBitmapBlocks + 1, mSuperblock.dataBlocks);
}

void Vfs::ReleaseBlock(uint32 id)
{
    ReleaseBitmap(mSuperblock.inodeBitmapBlocks + 1, mSuperblock.dataBlocks, id);
}

uint32 Vfs::ReserveINode()
{
    return ReserveBitmap(1, VFS_BLOCK_SIZE * mSuperblock.inodeBlocks / sizeof(INode));
}

void Vfs::ReleaseINode(uint32 id)
{
    ReleaseBitmap(1, VFS_BLOCK_SIZE * mSuperblock.inodeBlocks / sizeof(INode), id);
}

std::string Vfs::NameFromPath(const std::string& path)
{
    std::vector<std::string> dirs;
    std::string dir;
    std::istringstream pathStream(path);
    while (std::getline(pathStream, dir, '/'))
    {
        if (dir.length() > 0)
            dirs.push_back(dir);
    }

    if (dirs.size() == 0)
        return std::string();

    return dirs[dirs.size() - 1];
}

void Vfs::GetINodeByPath(const std::string& path, uint32& inodeID, uint32& parentINodeID)
{
    /// split path
    std::vector<std::string> dirs;
    std::string dir;
    std::istringstream pathStream(path);
    while (std::getline(pathStream, dir, '/'))
    {
        if (dir.length() > 0)
            dirs.push_back(dir);
    }

    inodeID = INVALID_INDEX;
    parentINodeID = INVALID_INDEX;

    if (dirs.size() == 1)
        parentINodeID = ROOT_INODE_INDEX;
    if (dirs.size() == 0)
        inodeID = ROOT_INODE_INDEX;

    uint32 currINodeID = 0;
    for (size_t j = 0; j < dirs.size(); ++j)
    {
        const auto& dir = dirs[j];
        bool found = false;

        Directory dirEntry;
        VfsFile dirFile(this, currINodeID, true);
        // NOTE: this is slow - O(n) worst case time complexity
        for (uint32 i = 0; i < dirFile.mINode.usage; ++i)
        {
            dirFile.Read(sizeof(Directory), &dirEntry);
            if (dir == dirEntry.name)
            {
                currINodeID = dirEntry.inodeID;
                found = true;
                break;
            }
        }

        if (!found)
            return;

        if (j == dirs.size() - 1) // target path found
            inodeID = currINodeID;
        else if (j == dirs.size() - 2) // parent directory found
            parentINodeID = currINodeID;
    }
}

void Vfs::WriteINode(uint32 id, const INode& inode)
{
    uint32 offset = 1 + mSuperblock.dataBitmapBlocks + mSuperblock.inodeBitmapBlocks;
    VFS_ASSERT(fseek(mImage, VFS_BLOCK_SIZE * offset + id * sizeof(INode), SEEK_SET) == 0);
    VFS_ASSERT(fwrite(&inode, sizeof(INode), 1, mImage) == 1);
}

void Vfs::ReadINode(uint32 id, INode& inode)
{
    uint32 offset = 1 + mSuperblock.dataBitmapBlocks + mSuperblock.inodeBitmapBlocks;
    VFS_ASSERT(fseek(mImage, VFS_BLOCK_SIZE * offset + id * sizeof(INode), SEEK_SET) == 0);
    VFS_ASSERT(fread(&inode, sizeof(INode), 1, mImage) == 1);
}

//=================================================================================================

Vfs::Vfs()
{
    mImage = nullptr;
}

Vfs::~Vfs()
{
    Release();
}

void Vfs::Release()
{
    if (!mOpenedFiles.empty())
    {
        LOG_DEBUG(mOpenedFiles.size() << " files were not closed");
        for (auto& ptr : mOpenedFiles)
            delete ptr;
        mOpenedFiles.clear();
    }

    if (mImage)
    {
        fclose(mImage);
        mImage = nullptr;
    }
}


bool Vfs::Open(const std::string& imagePath)
{
    Release();

    mImage = fopen(imagePath.c_str(), "r+b");
    if (mImage == 0)
    {
        LOG_ERROR("Failed to open VFS");
        return false;
    }

    fseek(mImage, 0, SEEK_SET);
    if (fread(&mSuperblock, sizeof(Superblock), 1, mImage) != 1)
    {
        LOG_ERROR("Failed to read superblock");
        Release();
        return false;
    }

    if (mSuperblock.magic != VFS_MAGIC)
    {
        LOG_ERROR("Invalid superblock signature");
        Release();
        return false;
    }

    return true;
}

bool Vfs::Init(const std::string& imagePath, uint32 size)
{
    Release();

    mImage = fopen(imagePath.c_str(), "w+b");
    if (mImage == 0)
    {
        LOG_ERROR("Failed to open VFS");
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
    mSuperblock.firstDataBlock = 1 +
                                 mSuperblock.dataBitmapBlocks +
                                 mSuperblock.inodeBitmapBlocks +
                                 mSuperblock.inodeBlocks;
    mSuperblock.dataBlocks = mSuperblock.blocks - mSuperblock.firstDataBlock;

    // clear VFS file
    static uint8 clearBlock[VFS_BLOCK_SIZE] = { 0x0 };
    fseek(mImage, 0, SEEK_SET);
    for (uint32 i = 0; i < mSuperblock.blocks; ++i)
    {
        VFS_ASSERT(1 == fwrite(clearBlock, VFS_BLOCK_SIZE, 1, mImage));
    }

    // write superblock
    fseek(mImage, 0, SEEK_SET);
    fwrite(&mSuperblock, sizeof(Superblock), 1, mImage);

    VFS_ASSERT(ReserveINode() == 0);
    INode rootInode;
    rootInode.type = INodeType::Directory;
    WriteINode(ROOT_INODE_INDEX, rootInode);

    return true;
}

VfsFile* Vfs::OpenFile(const std::string& path, bool create)
{
    uint32 inodeID, parentInodeID;
    GetINodeByPath(path, inodeID, parentInodeID);

    if (create)
    {
        if (inodeID != INVALID_INDEX)
        {
            LOG_ERROR("Path '" << path << "' already exists");
            return nullptr;
        }

        // create an inode for the new file
        inodeID = ReserveINode();
        if (inodeID == INVALID_INDEX)
        {
            LOG_ERROR("Failed to reserve inode for a file");
            return nullptr;
        }

        INode inode;
        inode.type = INodeType::File;
        WriteINode(inodeID, inode);

        // NOTE: name was extracted in GetINodeByPath()
        std::string fileName = NameFromPath(path);
        Directory dirEntry;
        dirEntry.inodeID = inodeID;
        strcpy(dirEntry.name, fileName.c_str());

        // update parent directory table
        VfsFile parentDirFile(this, parentInodeID);
        if (!parentDirFile.AddDirectoryEntry(dirEntry))
        {
            LOG_ERROR("Failed create file");
            ReleaseINode(inodeID);
            return nullptr;
        }
    }
    else
    {
        if (inodeID == INVALID_INDEX)
        {
            LOG_ERROR("Invalid path: " << path);
            return nullptr;
        }
    }
    
    VfsFile* fileHandle = new VfsFile(this, inodeID, false);
    if (fileHandle->mINode.type != INodeType::File)
    {
        delete fileHandle;
        LOG_ERROR("Path '" << path << "' is not a file");
        return nullptr;
    }

    mOpenedFiles.insert(fileHandle);
    return fileHandle;
}

bool Vfs::Close(VfsFile* file)
{
    auto it = mOpenedFiles.find(file);
    if (it == mOpenedFiles.end())
    {
        LOG_ERROR("This file is not opended");
        return false;
    }

    mOpenedFiles.erase(it);
    delete file;
    return true;
}

bool Vfs::CreateDir(const std::string& path)
{
    uint32 inodeID, parentInodeID;
    GetINodeByPath(path, inodeID, parentInodeID);

    if (parentInodeID == INVALID_INDEX)
    {
        LOG_ERROR("Invalid path: " << path);
        return false;
    }

    if (inodeID != INVALID_INDEX)
    {
        LOG_ERROR("Directory '" << path << "' already exists");
        return false;
    }

    inodeID = ReserveINode();
    if (inodeID == INVALID_INDEX)
    {
        LOG_ERROR("Failed to reserve inode for directory");
        return false;
    }

    // NOTE: name was extracted in GetINodeByPath()
    std::string dirName = NameFromPath(path);
    Directory dirEntry;
    dirEntry.inodeID = inodeID;
    strcpy(dirEntry.name, dirName.c_str());

    INode inode;
    inode.type = INodeType::Directory;
    WriteINode(inodeID, inode);

    // update parent directory table
    VfsFile parentDirFile(this, parentInodeID);
    if (!parentDirFile.AddDirectoryEntry(dirEntry))
    {
        LOG_ERROR("Failed create directory");
        ReleaseINode(inodeID);
        return false;
    }

    return true;
}

bool Vfs::Rename(const std::string& src, const std::string& dest)
{
    /// get old path info
    uint32 oldParentInodeID, oldInodeID;
    GetINodeByPath(src, oldInodeID, oldParentInodeID);
    if (oldInodeID == INVALID_INDEX)
    {
        LOG_ERROR("Invalid path: " << src);
        return false;
    }

    // get new path info
    uint32 newParentInodeID, newInodeID;
    GetINodeByPath(dest, newInodeID, newParentInodeID);
    if (newParentInodeID == INVALID_INDEX)
    {
        LOG_ERROR("Invalid path: " << dest);
        return false;
    }

    if (newInodeID != INVALID_INDEX)
    {
        LOG_ERROR("Path '" << dest << "' already exists");
        return false;
    }

    // create new directory table entry
    // NOTE: name was extracted in GetINodeByPath()
    std::string dirName = NameFromPath(dest);
    Directory dirEntry;
    dirEntry.inodeID = oldInodeID;
    strcpy(dirEntry.name, dirName.c_str());

    // update new parent directory table
    {
        VfsFile newParentDirFile(this, newParentInodeID);
        if (!newParentDirFile.AddDirectoryEntry(dirEntry))
        {
            LOG_ERROR("Failed move object");
            return false;
        }
    }

    // update old parent directory table
    {
        VfsFile oldParentDirFile(this, oldParentInodeID);
        VFS_ASSERT(oldParentDirFile.RemoveDirectoryEntry(oldInodeID));
    }

    return true;
}

bool Vfs::Remove(const std::string& path)
{
    uint32 inodeID, parentInodeID;
    GetINodeByPath(path, inodeID, parentInodeID);
    if (inodeID == INVALID_INDEX)
    {
        LOG_ERROR("Invalid path: " << path);
        return false;
    }

    VfsFile dirFile(this, inodeID);
    if (!dirFile.Remove())
        return false;

    VfsFile parentDirFile(this, parentInodeID);
    VFS_ASSERT(parentDirFile.RemoveDirectoryEntry(inodeID));

    ReleaseINode(inodeID);
    return true;
}

bool Vfs::List(const std::string& path, std::vector<std::string>& nodes)
{
    uint32 inodeID, parentInodeID;
    GetINodeByPath(path, inodeID, parentInodeID);
    if (inodeID == INVALID_INDEX)
    {
        LOG_ERROR("Invalid path: " << path);
        return false;
    }

    VfsFile dirFile(this, inodeID, true);
    if (dirFile.mINode.type != INodeType::Directory)
    {
        LOG_ERROR("The path '" << path << "' is not a directory");
        return false;
    }

    nodes.clear();
    for (uint32 i = 0; i < dirFile.mINode.usage; ++i)
    {
        Directory dirEntry;
        dirFile.Read(sizeof(Directory), &dirEntry);
        nodes.push_back(dirEntry.name);
    }

    return true;
}

bool Vfs::GetInfo(const std::string& path, PathInfo& info)
{
    uint32 inodeID, parentInodeID;
    GetINodeByPath(path, inodeID, parentInodeID);

    if (inodeID == INVALID_INDEX)
    {
        LOG_DEBUG("Invalid path: " << path);
        return false;
    }

    VfsFile file (this, inodeID, false);
    info.directory = file.mINode.type == INodeType::Directory;
    info.size = info.directory ? file.mINode.usage : file.mINode.size;
    return true;
}

void Vfs::DebugPrint()
{
    struct Node
    {
        int depth;
        uint32 inodeID;
        std::string fullPath;
        std::string name;
    };

    const std::string INDENT = "  ";
    std::stack<Node> dirStack;

    Node root;
    root.depth = 0;
    root.inodeID = ROOT_INODE_INDEX;
    root.fullPath = "";
    root.name = "<root>";
    dirStack.push(root);

    uint32* blockMap = new uint32 [mSuperblock.blocks];
    for (uint32 i = 0; i < mSuperblock.blocks; ++i)
        blockMap[i] = INVALID_INDEX;

    while (!dirStack.empty())
    {
        Node dir = dirStack.top();
        dirStack.pop();

        VfsFile file(this, dir.inodeID, true);
        for (uint32 i = 0; i < file.mINode.usage; ++i)
        {
            Directory dirEntry;
            file.Read(sizeof(Directory), &dirEntry);
            
            Node child;
            child.depth = dir.depth + 1;
            child.fullPath = dir.fullPath + '/' + dirEntry.name;
            child.inodeID = dirEntry.inodeID;
            child.name = dirEntry.name;
            dirStack.push(child);
        }

        for (int i = 0; i < dir.depth; ++i)
            std::cout << INDENT;
        std::string type = " ";
        if (file.mINode.type == INodeType::Directory)
            type = " [DIR] ";

        std::cout << "* " << std::setw(4) << std::setfill(' ') << dir.inodeID;
        std::cout << type << dir.name << " (" << file.mINode.size << " bytes)  { ";

        std::vector<uint32> blocks = file.GetBlocksMap();
        for (uint32 b : blocks)
        {
            VFS_ASSERT(b < mSuperblock.blocks);
            uint32 fsBlock = b + mSuperblock.firstDataBlock;
            blockMap[fsBlock] = file.mINodeID;
            std::cout << fsBlock << ' ';
        }
        std::cout << '}' << std::endl;
    }

    std::cout << "BLOCKS MAP:" << std::endl;

    const int perRow = 8;
    for (uint32 row = 0; row < mSuperblock.blocks; row += perRow)
    {
        for (uint32 id = row; id < row + perRow && id < mSuperblock.blocks; id++)
        {
            std::cout << std::setw(10) << std::setfill(' ') << blockMap[id] << "  ";
        }
        std::cout << std::endl;
    }
}
