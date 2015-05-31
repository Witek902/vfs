/**
 * @author Michal Witanowski
 */

#pragma once

#include "vfscommon.hpp"
#include <stdlib.h>
#include <stdio.h>

struct Superblock
{
    uint32 magic;
    uint32 blocks;            //< total number of blocks
    uint32 vfsSize;           //< total VFS size (in bytes)
    uint32 inodeBlocks;       //< total blocks containing inodes
    uint32 inodeBitmapBlocks; //< number of blocks containing inodes bitmap
    uint32 dataBitmapBlocks;  //< number of blocks containing data blocks bitmap
    uint32 firstDataBlock;    //< ID of the first block containing data

    // TODO: stats, etc.
};

enum class INodeType : uint8
{
    File,
    Directory
};

#define INODE_PTRS 5

/**
 * Index Node structure
 */
struct INode
{
    INodeType type;

    /**
     * Block pointers depth.
     * 0 - "blockPtr" are direct pointers to data blocks
     * 1 - "blockPtr" are pointers to blocks containing pointers to data blocks
     * 2 - "blockPtr" are pointers to blocks containing pointers to blocks containing pointers to data blocks
     */
    uint8 ptrDepth;
    uint32 size; //< file size in bytes
    uint32 usage; //< number of entries in the directory (only for dirs)
    uint32 blockPtr[INODE_PTRS];

    INode();
};

/**
 * Directory structure
 */
struct Directory
{
    uint32 inodeID;
    char name[252];

    Directory();
};

/**
 * VFS file seeking mode
 */
enum class VfsSeekMode
{
    Curr,  //< seek relative to the current position
    Begin, //< seek relative to the file beginning
    End    //< seek relative to the file end
};

extern const uint32 INVALID_INDEX;

class Vfs;
class VfsFile;
