#pragma once

#include "vfsstructures.hpp"

/**
 * @brief Class representing an open file in the VFS
 */
class VfsFile final
{
    friend class Vfs;

    Vfs* mVFS;

public:
    /**
     * @brief Read data from the file
     * @param bytes Number of bytes to read
     * @param data  Target buffer pointer
     * @return      Number of bytes read or -1 on error
     */
    ssize_t Read(size_t bytes, void* data);

    /**
     * @brief Write data to the file
     * @param bytes Number of bytes to write
     * @param data  Source buffer pointer
     * @return      Number of bytes written or -1 on error
     */
    ssize_t Write(size_t bytes, const void* data);

    /**
     * @brief Change file cursor
     * @param offset Offset in bytes
     * @param mode Seeking mode
     * @return File currsor after seeking
     */
    size_t Seek(ssize_t offset, VfsSeekMode mode);
};
