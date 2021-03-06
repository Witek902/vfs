#pragma once

#include "vfsstructures.hpp"

#include <vector>

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
    bool mReadOnly;

    VfsFile(const VfsFile& file) = delete;
    VfsFile(Vfs* vfs, uint32 inodeID, bool readOnly = false);

    // translate block index into real index in the VFS
    uint32 GetRealBlockID(uint32 id, bool allocate);

    // reorganize block pointers if there is no left space
    bool ExtendPointers();

    // read data without affecting cursor
    uint32 ReadOffset(uint32 bytes, uint32 offset, void* data);

    // write data without affecting cursor
    uint32 WriteOffset(uint32 bytes, uint32 offset, const void* data);

    // remove all file blocks (or directory table if empty)
    bool Remove();

    bool RemoveDirectoryEntry(uint32 inodeID);
    bool AddDirectoryEntry(const Directory& dir);

    /**
     * Query list of all blocks used by this file.
     */
    std::vector<uint32> GetBlocksMap();

public:
    ~VfsFile();

    /**
     * @brief Read data from the file
     * @param bytes Number of bytes to read
     * @param data  Target buffer pointer
     * @return      Number of bytes read or -1 on error
     */
    uint32 Read(uint32 bytes, void* data);

    /**
     * @brief Write data to the file
     * @param bytes Number of bytes to write
     * @param data  Source buffer pointer
     * @return      Number of bytes written or -1 on error
     */
    uint32 Write(uint32 bytes, const void* data);

    /**
     * @brief Change file cursor
     * @param offset Offset in bytes
     * @param mode Seeking mode
     * @return File currsor after seeking
     */
    uint32 Seek(int32 offset, VfsSeekMode mode);
};
