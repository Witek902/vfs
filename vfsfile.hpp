#pragma once

#include "vfsstructures.hpp"

/**
 * @brief Class representing an open file in the VFS
 */
class VfsFile final
{
    friend class Vfs;

    Vfs* mVFS;
    uint32 mCursor;
    uint32 mINodeID;
    INode mINode;

    VfsFile(Vfs* vfs, uint32 inodeID, const INode& inode);

    // reorganize block pointers if there is no left space
    bool ExtendPointers(uint8 newDepth);

    // read data without affecting cursor
    int32 ReadOffset(uint32 bytes, uint32 offset, void* data);

    // write data without affecting cursor
    int32 WriteOffset(uint32 bytes, uint32 offset, const void* data);

public:
    /**
     * @brief Read data from the file
     * @param bytes Number of bytes to read
     * @param data  Target buffer pointer
     * @return      Number of bytes read or -1 on error
     */
    int32 Read(uint32 bytes, void* data);

    /**
     * @brief Write data to the file
     * @param bytes Number of bytes to write
     * @param data  Source buffer pointer
     * @return      Number of bytes written or -1 on error
     */
    int32 Write(uint32 bytes, const void* data);

    /**
     * @brief Change file cursor
     * @param offset Offset in bytes
     * @param mode Seeking mode
     * @return File currsor after seeking
     */
    uint32 Seek(int32 offset, VfsSeekMode mode);
};
