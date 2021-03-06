/**
 * @author Michal Witanowski
 */

#include "vfsfile.hpp"
#include "vfs.hpp"
#include "vfscommon.hpp"

#include <assert.h>
#include <algorithm>

#define VFS_PTRS_PER_BLOCK (VFS_BLOCK_SIZE / sizeof(uint32))

VfsFile::VfsFile(Vfs* vfs, uint32 inodeID, bool readOnly)
{
    mVFS = vfs;
    mCursor = 0;
    mINodeID = inodeID;
    mReadOnly = readOnly;
    mVFS->ReadINode(mINodeID, mINode);
}

VfsFile::~VfsFile()
{
    if (!mReadOnly)
        mVFS->WriteINode(mINodeID, mINode);
}

bool VfsFile::ExtendPointers()
{
    VFS_ASSERT(mINode.ptrDepth < 2);

    uint32 pointersBlockId = mVFS->ReserveBlock();
    if (pointersBlockId == INVALID_INDEX)
    {
        LOG_DEBUG("No blocks left");
        return false;
    }

    uint32 offset = VFS_BLOCK_SIZE * (mVFS->mSuperblock.firstDataBlock + pointersBlockId);
    VFS_ASSERT(fseek(mVFS->mImage, offset, SEEK_SET) == 0);

    // copy old indode's pointers to the new pointers block
    VFS_ASSERT(fwrite(mINode.blockPtr, sizeof(uint32), INODE_PTRS, mVFS->mImage) == INODE_PTRS);
    // initialize the rest of the pointers with invalid indicies
    for (uint32 i = INODE_PTRS; i < VFS_PTRS_PER_BLOCK; ++i)
        VFS_ASSERT(fwrite(&INVALID_INDEX, sizeof(uint32), 1, mVFS->mImage) == 1);

    // update inode's pointers
    mINode.blockPtr[0] = pointersBlockId;
    for (uint32 i = 1; i < INODE_PTRS; ++i)
        mINode.blockPtr[i] = INVALID_INDEX;

    mINode.ptrDepth++;
    return true;
}

uint32 VfsFile::GetRealBlockID(uint32 id, bool allocate)
{
    VFS_ASSERT(mINode.ptrDepth < 3);

    uint32 realBlockId = INVALID_INDEX;
    uint32 offset;

    if (mINode.ptrDepth == 0) // we have direct block pointers
    {
        if (id >= INODE_PTRS)
        {
            if (!allocate)
                return INVALID_INDEX;

            if (!ExtendPointers())
                return INVALID_INDEX;
        }
        else
        {
            if (mINode.blockPtr[id] == INVALID_INDEX && allocate)
            {
                // reserve data block if not reserved yet
                if ((mINode.blockPtr[id] = mVFS->ReserveBlock()) == INVALID_INDEX)
                {
                    LOG_DEBUG("No blocks left");
                    return INVALID_INDEX;
                }
            }
            realBlockId = mINode.blockPtr[id];
        }
    }

    if (mINode.ptrDepth == 1) // we have indirect block pointers
    {
        if (id >= INODE_PTRS * VFS_PTRS_PER_BLOCK)
        {
            if (!allocate)
                return INVALID_INDEX;

            if (!ExtendPointers())
                return INVALID_INDEX;
        }
        else
        {
            uint32 inodePtrId = id / VFS_PTRS_PER_BLOCK;
            uint32 blockPtrId = id % VFS_PTRS_PER_BLOCK;

            // reserve block for indirect pointers
            if (mINode.blockPtr[inodePtrId] == INVALID_INDEX && allocate)
            {
                if ((mINode.blockPtr[inodePtrId] = mVFS->ReserveBlock()) == INVALID_INDEX)
                {
                    LOG_DEBUG("No blocks left");
                    return INVALID_INDEX;
                }

                // initialize allocated pointers block (write invalid indicies)
                offset = VFS_BLOCK_SIZE * (mVFS->mSuperblock.firstDataBlock +
                                           mINode.blockPtr[inodePtrId]);
                VFS_ASSERT(offset < mVFS->mSuperblock.vfsSize);
                VFS_ASSERT(fseek(mVFS->mImage, offset, SEEK_SET) == 0);
                for (uint32 i = 0; i < VFS_PTRS_PER_BLOCK; ++i)
                    VFS_ASSERT(fwrite(&INVALID_INDEX, sizeof(uint32), 1, mVFS->mImage) == 1);
            }

            if (mINode.blockPtr[inodePtrId] == INVALID_INDEX)
                return INVALID_INDEX;

            // read direct pointer from pointers block
            offset = VFS_BLOCK_SIZE * (mVFS->mSuperblock.firstDataBlock +
                                       mINode.blockPtr[inodePtrId]);
            offset += sizeof(uint32) * blockPtrId;
            VFS_ASSERT(offset < mVFS->mSuperblock.vfsSize);
            VFS_ASSERT(fseek(mVFS->mImage, offset, SEEK_SET) == 0);
            VFS_ASSERT(fread(&realBlockId, sizeof(uint32), 1, mVFS->mImage) == 1);

            // reserve data block if not reserved yet
            if (realBlockId == INVALID_INDEX && allocate)
            {
                if ((realBlockId = mVFS->ReserveBlock()) == INVALID_INDEX)
                {
                    LOG_DEBUG("No blocks left");
                    return INVALID_INDEX;
                }

                // write direct pointer to pointers block
                VFS_ASSERT(fseek(mVFS->mImage, offset, SEEK_SET) == 0);
                VFS_ASSERT(fwrite(&realBlockId, sizeof(uint32), 1, mVFS->mImage) == 1);
            }
        }
    }

    if (mINode.ptrDepth == 2) // we have double-indirect block pointers
    {
        if (id >= INODE_PTRS * VFS_PTRS_PER_BLOCK * VFS_PTRS_PER_BLOCK)
            return INVALID_INDEX; // we can't extend pointers further

        uint32 inodePtrId = id / (VFS_PTRS_PER_BLOCK * VFS_PTRS_PER_BLOCK);
        id -= inodePtrId * (VFS_PTRS_PER_BLOCK * VFS_PTRS_PER_BLOCK);
        uint32 blockPtrId = id / VFS_PTRS_PER_BLOCK;
        uint32 secondBlockPtrId = id % VFS_PTRS_PER_BLOCK;

        VFS_ASSERT(blockPtrId < VFS_PTRS_PER_BLOCK);
        VFS_ASSERT(inodePtrId < INODE_PTRS);

        // reserve block for double-indirect pointers
        if (mINode.blockPtr[inodePtrId] == INVALID_INDEX && allocate)
        {
            if ((mINode.blockPtr[inodePtrId] = mVFS->ReserveBlock()) == INVALID_INDEX)
            {
                LOG_DEBUG("No blocks left");
                return INVALID_INDEX;
            }

            // initialize allocated pointers block (write invalid indicies)
            offset = VFS_BLOCK_SIZE * (mVFS->mSuperblock.firstDataBlock +
                                       mINode.blockPtr[inodePtrId]);
            VFS_ASSERT(fseek(mVFS->mImage, offset, SEEK_SET) == 0);
            for (uint32 i = 0; i < VFS_PTRS_PER_BLOCK; ++i)
                VFS_ASSERT(fwrite(&INVALID_INDEX, sizeof(uint32), 1, mVFS->mImage) == 1);
        }

        if (mINode.blockPtr[inodePtrId] == INVALID_INDEX)
            return INVALID_INDEX;

        // read indirect pointer from pointers block
        uint32 ptrBlockId = INVALID_INDEX;
        offset = VFS_BLOCK_SIZE * (mVFS->mSuperblock.firstDataBlock + mINode.blockPtr[inodePtrId]);
        offset += sizeof(uint32) * blockPtrId;
        VFS_ASSERT(offset < mVFS->mSuperblock.vfsSize);
        VFS_ASSERT(fseek(mVFS->mImage, offset, SEEK_SET) == 0);
        VFS_ASSERT(fread(&ptrBlockId, sizeof(uint32), 1, mVFS->mImage) == 1);

        // reserve pointers block if not reserved yet
        if (ptrBlockId == INVALID_INDEX && allocate)
        {
            if ((ptrBlockId = mVFS->ReserveBlock()) == INVALID_INDEX)
            {
                LOG_DEBUG("No blocks left");
                return INVALID_INDEX;
            }

            // initialize allocated pointers block (write invalid indicies)
            uint32 offset2 = VFS_BLOCK_SIZE * (mVFS->mSuperblock.firstDataBlock + ptrBlockId);
            VFS_ASSERT(fseek(mVFS->mImage, offset2, SEEK_SET) == 0);
            for (uint32 i = 0; i < VFS_PTRS_PER_BLOCK; ++i)
                VFS_ASSERT(fwrite(&INVALID_INDEX, sizeof(uint32), 1, mVFS->mImage) == 1);

            // write indirect pointer to pointers block
            VFS_ASSERT(fseek(mVFS->mImage, offset, SEEK_SET) == 0);
            VFS_ASSERT(fwrite(&ptrBlockId, sizeof(uint32), 1, mVFS->mImage) == 1);
        }

        if (ptrBlockId == INVALID_INDEX)
            return INVALID_INDEX;

        // read indirect pointer from pointers block
        offset = VFS_BLOCK_SIZE * (mVFS->mSuperblock.firstDataBlock + ptrBlockId);
        offset += sizeof(uint32) * secondBlockPtrId;
        VFS_ASSERT(offset < mVFS->mSuperblock.vfsSize);
        VFS_ASSERT(fseek(mVFS->mImage, offset, SEEK_SET) == 0);
        VFS_ASSERT(fread(&realBlockId, sizeof(uint32), 1, mVFS->mImage) == 1);

        // reserve pointers block if not reserved yet
        if (realBlockId == INVALID_INDEX && allocate)
        {
            if ((realBlockId = mVFS->ReserveBlock()) == INVALID_INDEX)
            {
                LOG_DEBUG("No blocks left");
                return INVALID_INDEX;
            }

            // write indirect pointer to pointers block
            VFS_ASSERT(fseek(mVFS->mImage, offset, SEEK_SET) == 0);
            VFS_ASSERT(fwrite(&realBlockId, sizeof(uint32), 1, mVFS->mImage) == 1);
        }
    }

    return realBlockId;
}

uint32 VfsFile::ReadOffset(uint32 bytes, uint32 offset, void* data)
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

    for (uint32 i = firstBlockId; i <= lastBlockId; ++i)
    {
        uint32 blockID = GetRealBlockID(i, false);
        if (blockID == INVALID_INDEX)
            break;

        // calculate VFS read offset (in bytes)
        uint32 vfsOffset = VFS_BLOCK_SIZE * (mVFS->mSuperblock.firstDataBlock + blockID);

        // calculate number of bytes to read
        uint32 toRead = VFS_BLOCK_SIZE;
        if (i == firstBlockId)
        {
            uint32 interBlockOffset = offset - VFS_BLOCK_SIZE * (offset / VFS_BLOCK_SIZE);
            vfsOffset += interBlockOffset;
            toRead = VFS_BLOCK_SIZE - interBlockOffset;
        }

        toRead = std::min(toRead, bytes - read);

        VFS_ASSERT(fseek(mVFS->mImage, vfsOffset, SEEK_SET) == 0);
        VFS_ASSERT(fread(dataPtr, toRead, 1, mVFS->mImage) == 1);
        dataPtr += toRead;
        read += toRead;
    }

    return read;
}

uint32 VfsFile::WriteOffset(uint32 bytes, uint32 offset, const void* data)
{
    if (bytes == 0)
        return 0;

    if (mReadOnly)
    {
        LOG_DEBUG("Trying to write read-only file");
        return 0;
    }

    uint32 written = 0;
    uint32 firstBlockId = offset / VFS_BLOCK_SIZE;
    uint32 lastBlockId = (offset + bytes - 1) / VFS_BLOCK_SIZE;
    const char* dataPtr = (const char*)data;

    for (uint32 i = firstBlockId; i <= lastBlockId; ++i)
    {
        uint32 blockID = GetRealBlockID(i, true);
        if (blockID == INVALID_INDEX)
            break;

        // calculate VFS write offset (in bytes)
        uint32 vfsOffset = VFS_BLOCK_SIZE * (mVFS->mSuperblock.firstDataBlock + blockID);

        // calculate number of bytes to write
        uint32 toWrite = VFS_BLOCK_SIZE;
        if (i == firstBlockId)
        {
            uint32 interBlockOffset = offset - VFS_BLOCK_SIZE * (offset / VFS_BLOCK_SIZE);
            vfsOffset += interBlockOffset;
            toWrite = VFS_BLOCK_SIZE - interBlockOffset;
        }

        toWrite = std::min(toWrite, bytes - written);

        VFS_ASSERT(fseek(mVFS->mImage, vfsOffset, SEEK_SET) == 0);
        VFS_ASSERT(fwrite(dataPtr, toWrite, 1, mVFS->mImage) == 1);
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
    VFS_ASSERT(mINode.type == INodeType::Directory);

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
    VFS_ASSERT(mINode.type == INodeType::Directory);

    if (WriteOffset(sizeof(Directory), mINode.usage * sizeof(Directory), &dir)
        != sizeof(Directory))
    {
        return false;
    }

    mINode.usage++;
    return true;
}

uint32 VfsFile::Read(uint32 bytes, void* data)
{
    uint32 bytesRead = ReadOffset(bytes, mCursor, data);
    mCursor += bytesRead;
    return bytesRead;
}

uint32 VfsFile::Write(uint32 bytes, const void* data)
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
        mCursor = mINode.size + offset;
        break;
    case VfsSeekMode::Curr:
        mCursor += offset;
        break;
    }

    return mCursor;
}

std::vector<uint32> VfsFile::GetBlocksMap()
{
    std::vector<uint32> result;
    uint32 blocks = CeilDivide<uint32>(mINode.size, VFS_BLOCK_SIZE);

    for (uint32 i = 0; i < blocks; ++i)
    {
        uint32 realBlockId = GetRealBlockID(i, false);
        if (realBlockId != INVALID_INDEX)
            result.push_back(realBlockId);
    }

    return result;
}
