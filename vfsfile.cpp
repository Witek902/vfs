/**
 * @author Michal Witanowski
 */

#include "vfsfile.hpp"
#include "vfs.hpp"

VfsFile::VfsFile(Vfs* vfs, uint32 inode)
{
    mVFS = vfs;
    mCursor = 0;
    mINode = inode;
}

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
