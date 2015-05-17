/**
 * @author Michal Witanowski
 */

#pragma once

#include "vfsstructures.hpp"
#include "vfsfile.hpp"
#include <vector>
#include <string>

/**
 * @brief Class representing VFS
 */
class Vfs final
{
    friend class VfsFile;

    FILE* mImage;
    Superblock mSuperblock;

    uint32 ReserveBitmap(uint32 firstBitmapBlock, uint32 bitmapSize);
    void ReleaseBitmap(uint32 firstBitmapBlock, uint32 bitmapSize, uint32 id);

    uint32 ReserveBlock();
    void ReleaseBlock(uint32 id);
    uint32 ReserveINode();
    void ReleaseINode(uint32 id);

public:
    ~Vfs();

    /**
     * @brief Open or create filesystem image file.
     * @param vfsPath Image file path
     */
    Vfs(const char* vfsPath);

    /**
     * @brief Initialize filesystem. This will remove all data
     * @param size Virtual File System size in bytes
     */
    bool Init(size_t size);

    /**
     * @brief Open a file in the VFS
     * @param File path
     * @return File pointer
     */
    VfsFile* OpenFile(const char* path);

    /**
     * @brief Close an opened file
     * @param file File pointer
     */
    bool Close(VfsFile* file);

    /**
     * @brief Create directory
     */
    bool CreateDir(const char* path);

    /**
     * @brief Rename a file or a directory
     * @param src Old path
     * @param dest New path
     */
    bool Rename(const char* src, const char* dest);

    /**
     * @brief Remove a file or an empty directory
     * @param path File or directory path
     */
    bool Remove(const char* path);

    /**
     * @brief List all files and directories in a directory
     */
    bool List(std::vector<std::string>& nodes);

    // TODO:
    // * file system map (used/unused block, fragmentation, etc.)
};
