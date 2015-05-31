/**
 * @author Michal Witanowski
 */

#pragma once

#include "vfsstructures.hpp"
#include "vfsfile.hpp"
#include <vector>
#include <string>

// block size in bytes
#define VFS_BLOCK_SIZE 4096

// inode size in bytes
#define VFS_INODE_SIZE (4*16)

#define VFS_MAGIC 'vfs!'

/**
 * @brief Class representing VFS
 */
class Vfs final
{
    friend class VfsFile;

    FILE* mImage;
    Superblock mSuperblock;

    /**
     * Reserve a single item in a bitmap (write bit "1" in an empty field).
     * @param firstBitmapBlock Index of the first bitmap block
     * @param bitmapSize       Bitmap size (in bits)
     * @return Reserved bit index or (-1) if bitmap is full.
     */
    uint32 ReserveBitmap(uint32 firstBitmapBlock, uint32 bitmapSize);

    /**
     * Release a single item in a bitmap (write bit "0" in the field).
     * @param firstBitmapBlock Index of the first bitmap block
     * @param bitmapSize       Bitmap size (in bits)
     * @param id               Field ID
     */
    void ReleaseBitmap(uint32 firstBitmapBlock, uint32 bitmapSize, uint32 id);

    uint32 ReserveBlock();
    void ReleaseBlock(uint32 id);
    uint32 ReserveINode();
    void ReleaseINode(uint32 id);

    static std::string NameFromPath(const std::string& path);
    void GetINodeByPath(const std::string& path, uint32& inodeID, uint32& parentINodeID);
    void WriteINode(uint32 id, const INode& inode);
    void ReadINode(uint32 id, INode& inode);

public:
    ~Vfs();

    /**
     * @brief Open or create filesystem image file.
     * @param vfsPath Image file path
     */
    Vfs(const std::string& vfsPath);

    /**
     * @brief Initialize filesystem. This will remove all data
     * @param size Virtual File System size in bytes
     */
    bool Init(uint32 size);

    /**
     * @brief Open a file in the VFS
     * @param path   File path
     * @param create Create if does not exist
     * @return File pointer
     */
    VfsFile* OpenFile(const std::string& path, bool create);

    /**
     * @brief Close an opened file
     * @param file File pointer
     */
    bool Close(VfsFile* file);

    /**
     * @brief Create directory
     */
    bool CreateDir(const std::string& path);

    /**
     * @brief Rename a file or a directory
     * @param src Old path
     * @param dest New path
     */
    bool Rename(const std::string& src, const std::string& dest);

    /**
     * @brief Remove a file or an empty directory
     * @param path File or directory path
     */
    bool Remove(const std::string& path);

    /**
     * @brief List all files and directories in a directory
     */
    bool List(const std::string& path, std::vector<std::string>& nodes);

    // TODO:
    // * file system map (used/unused block, fragmentation, etc.)

    void DebugPrint();
};