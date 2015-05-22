/**
 * @author Michal Witanowski
 */

#include "vfsfile.hpp"
#include "vfs.hpp"

int32 VfsFile::Read(uint32 bytes, void* data)
{
    return 0;
}

int32 VfsFile::Write(uint32 bytes, const void* data)
{
    return 0;
}

uint32 VfsFile::Seek(int32 offset, VfsSeekMode mode)
{
    return 0;
}
