/**
 * @author Michal Witanowski
 */

#include "vfsfile.hpp"
#include "vfs.hpp"

ssize_t VfsFile::Read(size_t bytes, void* data)
{
    return 0;
}

ssize_t VfsFile::Write(size_t bytes, const void* data)
{
    return 0;
}

size_t VfsFile::Seek(ssize_t offset, VfsSeekMode mode)
{
    return 0;
}
