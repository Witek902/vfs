/**
 * @author Michal Witanowski
 */

#include "vfsfile.hpp"
#include "vfs.hpp"
#include "vfscommon.hpp"
#include <assert.h>
#include <algorithm>

VfsFile::VfsFile(Vfs* vfs, uint32 inodeID, bool readOnly)
{
    mVFS = vfs;
    mCursor = 0;
    mINodeID = inodeID;
    mVFS->ReadINode(mINodeID, mINode);
    // TODO: read only support
}

VfsFile::~VfsFile()
{
    mVFS->WriteINode(mINodeID, mINode);
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
    if (offset >= mINode.size)
        return 0;

    if (offset + bytes > mINode.size)
        bytes = mINode.size - offset;

    if (bytes == 0)
        return 0;

    uint32 read = 0;
    uint32 firstBlockId = offset / VFS_BLOCK_SIZE;
    uint32 lastBlockId = (offset + bytes - 1) / VFS_BLOCK_SIZE;
    char* dataPtr = (char*)data;

    // TODO: real block ID resolving for depth 1 and 2
    assert(lastBlockId < INODE_PTRS);

    for (uint32 i = firstBlockId; i <= lastBlockId; ++i)
    {
        if (mINode.blockPtr[i] == INVALID_INDEX)
            break;

        // calculate VFS read offset (in bytes)
        uint32 vfsOffset = VFS_BLOCK_SIZE * (mVFS->mSuperblock.firstDataBlock + mINode.blockPtr[i]);

        // calculate number of bytes to read
        uint32 toRead = VFS_BLOCK_SIZE;
        if (i == firstBlockId)
        {
            uint32 interBlockOffset = offset - VFS_BLOCK_SIZE * (offset / VFS_BLOCK_SIZE);
            vfsOffset += interBlockOffset;
            toRead = VFS_BLOCK_SIZE - interBlockOffset;
        }

        toRead = std::min(toRead, bytes - read);

        assert(fseek(mVFS->mImage, vfsOffset, SEEK_SET) == 0);
        assert(fread(dataPtr, toRead, 1, mVFS->mImage) == 1);
        dataPtr += toRead;
        read += toRead;
    }

    return read;
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
        uint32 toWrite = VFS_BLOCK_SIZE;
        if (i == firstBlockId)
        {
            uint32 interBlockOffset = offset - VFS_BLOCK_SIZE * (offset / VFS_BLOCK_SIZE);
            vfsOffset += interBlockOffset;
            toWrite = VFS_BLOCK_SIZE - interBlockOffset;
        }

        toWrite = std::min(toWrite, bytes - written);

        assert(fseek(mVFS->mImage, vfsOffset, SEEK_SET) == 0);
        assert(fwrite(dataPtr, toWrite, 1, mVFS->mImage) == 1);
        dataPtr += toWrite;
        written += toWrite;

        // update file size
        mINode.size = std::max(mINode.size, offset + written);
    }

    return written;
}

bool VfsFile::Remove()
{
    if (mINode.type == INodeType::Directory && mINode.usage != 0)
    {
        LOG_DEBUG("Directory is not empty");
        return false;
    }

    // TODO: support for pointers depth > 0
    for (uint32 i = 0; i < INODE_PTRS; ++i)
    {
        if (mINode.blockPtr[i] != INVALID_INDEX)
        {
            mVFS->ReleaseBlock(mINode.blockPtr[i]);
            mINode.blockPtr[i] = INVALID_INDEX;
        }
    }

    return true;
}

bool VfsFile::RemoveDirectoryEntry(uint32 inodeID)
{
    assert(mINode.type == INodeType::Directory);

    bool found = false;
    for (uint32 i = 0; i < mINode.usage; ++i)
    {
        Directory dirEntry;
        Read(sizeof(Directory), &dirEntry);

        // swap with last element - fast O(1) removal
        if (dirEntry.inodeID == inodeID)
        {
            if ((mINode.usage > 1) && (i < mINode.usage - 1))
            {
                ReadOffset(sizeof(Directory), (mINode.usage - 1) * sizeof(Directory), &dirEntry);
                WriteOffset(sizeof(Directory), i * sizeof(Directory), &dirEntry);
            }
            found = true;
            break;
        }
    }

    if (found)
        mINode.usage--;

    return found;
}

bool VfsFile::AddDirectoryEntry(const Directory& dir)
{
    assert(mINode.type == INodeType::Directory);

    if (WriteOffset(sizeof(Directory), mINode.usage * sizeof(Directory), &dir)
        != sizeof(Directory))
    {
        return false;
    }

    mINode.usage++;
    return true;
}

int32 VfsFile::Read(uint32 bytes, void* data)
{
    uint32 bytesRead = ReadOffset(bytes, mCursor, data);
    mCursor += bytesRead;
    return bytesRead;
}

int32 VfsFile::Write(uint32 bytes, const void* data)
{
    uint32 bytesWritten = WriteOffset(bytes, mCursor, data);
    mCursor += bytesWritten;
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
