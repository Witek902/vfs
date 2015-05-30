/**
 * @author Michal Witanowski
 */

#include "vfsfile.hpp"
#include "vfs.hpp"
#include "vfscommon.hpp"
#include <assert.h>
#include <algorithm>

VfsFile::VfsFile(Vfs* vfs, uint32 inodeID, const INode& inode)
{
    mVFS = vfs;
    mCursor = 0;
    mINodeID = inodeID;
    mINode = inode;
}

bool VfsFile::ExtendPointers(uint8 newDepth)
{
    assert(newDepth == (mINode.ptrDepth + 1));
    assert(newDepth < 3);

    if (newDepth == 1)
    {
        // TODO
    }

    if (newDepth == 2)
    {
        // TODO
    }

    mINode.ptrDepth = newDepth;
    return true;
}

int32 VfsFile::ReadOffset(uint32 bytes, uint32 offset, void* data)
{
    if (bytes == 0)
        return 0;

    uint32 firstBlockId = offset / VFS_BLOCK_SIZE;
    uint32 lastBlockId = (offset + bytes - 1) / VFS_BLOCK_SIZE;

    // TODO: real block ID resolving for depth 1 and 2

    return bytes;
}

int32 VfsFile::WriteOffset(uint32 bytes, uint32 offset, const void* data)
{
    if (bytes == 0)
        return 0;

    uint32 written = 0;
    uint32 firstBlockId = offset / VFS_BLOCK_SIZE;
    uint32 lastBlockId = (offset + bytes - 1) / VFS_BLOCK_SIZE;
    const char* dataPtr = (const char*)data;

    // TODO: real block ID resolving for depth 1 and 2
    assert(lastBlockId < INODE_PTRS); 

    for (uint32 i = firstBlockId; i <= lastBlockId; ++i)
    {
        if (mINode.blockPtr[i] == INVALID_INDEX)
        {
            if ((mINode.blockPtr[i] = mVFS->ReserveBlock()) == INVALID_INDEX)
            {
                LOG_DEBUG("No blocks left");
                return written;
            }
        }

        // calculate VFS write offset (in bytes)
        uint32 vfsOffset = VFS_BLOCK_SIZE * (mVFS->mSuperblock.firstDataBlock + mINode.blockPtr[i]);

        // calculate number of bytes to write
        uint32 toWrite = 0;
        if (i == firstBlockId)
        {
            uint32 interBlockOffset = offset - VFS_BLOCK_SIZE * (offset / VFS_BLOCK_SIZE);
            vfsOffset += interBlockOffset;
            toWrite = VFS_BLOCK_SIZE - interBlockOffset;
        }
        else if (i == lastBlockId)
            toWrite = bytes - written;
        else
            toWrite = VFS_BLOCK_SIZE;

        assert(fseek(mVFS->mImage, vfsOffset, SEEK_SET) == 0);
        assert(fwrite(dataPtr, toWrite, 1, mVFS->mImage) == 1);
        dataPtr += toWrite;
        written += toWrite;

        // update file size
        mINode.size = std::max(mINode.size, offset + written);
    }

    return written;
}

int32 VfsFile::Read(uint32 bytes, void* data)
{
    uint32 bytesRead = ReadOffset(bytes, mCursor, data);
    mCursor += bytes;
    return bytesRead;
}

int32 VfsFile::Write(uint32 bytes, const void* data)
{
    uint32 bytesWritten = WriteOffset(bytes, mCursor, data);
    mCursor += bytes;
    return bytesWritten;
}

uint32 VfsFile::Seek(int32 offset, VfsSeekMode mode)
{
    switch (mode)
    {
    case VfsSeekMode::Begin:
        mCursor = offset;
        break;
    case VfsSeekMode::End:
        mCursor += offset;
        break;
    case VfsSeekMode::Curr:
        mCursor += offset;
        break;
    }

    return mCursor;
}
