/**
 * @author Michal Witanowski
 */

#pramga once

#include <stdlib.h>
#include <stdio.h>
#include <vector>

/**
 * VFS file seeking mode
 */
enum class VfsSeekMode
{
    Curr,  //< seek relative to the current position
    Begin, //< seek relative to the file beginning
    End    //< seek relative to the file end
};

/**
 * @brief Class representing an open file in the VFS
 */
class VfsFile
{
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

/**
 * @brief Class representing VFS
 */
class Vfs
{
public:
    /**
     * @brief Open or create filesystem image file.
     * @param vfsPath Image file path
     */
    Vfs(const char* vfsPath);

    /**
     * @brief Initialize filesystem. This will remove all data
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
