/**
 * @author Michal Witanowski
 */

#pragma once

#include <stdlib.h>
#include <stdio.h>

typedef int int32;
typedef unsigned int uint32;
typedef short int16;
typedef unsigned short uint16;
typedef char int8;
typedef unsigned char uint8;

struct Superblock
{
    uint32 vfsSize;           //< total VFS size (in bytes)
    uint32 inodeBlocks;       //< total blocks containing inodes
    uint32 inodeBitmapBlocks; //< number of blocks containing inodes bitmap
    uint32 dataBitmapBlocks;  //< number of blocks containing data blocks bitmap

    // TODO: stats, etc.
};

enum class INodeType : int
{
    File,
    Directory
};

struct INodeDesc
{
    uint8 type : 1; //< 0 - regular file, 1 - directory

    /**
     * Block pointers depth.
     * 0 - "blockPtr" are direct pointers to data blocks
     * 1 - "blockPtr" are pointers to blocks containing pointers to data blocks
     * 2 - "blockPtr" are pointers to blocks containing pointers to blocks containing pointers to data blocks
     * 3 - not used
     */
    uint8 ptrDepth : 2;
};

/**
 * Index Node structure
 */
struct INode
{
    INodeType type;
    uint32 size;
    uint32 blockPtr[14];
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

class Vfs;
class VfsFile;
