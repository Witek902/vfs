/**
 * @author Michal Witanowski
 */

#include "vfs.hpp"


ssize_t VfsFile::Read(size_t bytes, void* data)
{

}

ssize_t VfsFile::Write(size_t bytes, const void* data)
{

}

size_t VfsFile::Seek(ssize_t offset, VfsSeekMode mode)
{

}



Vfs::Vfs(const char* vfsPath)
{

}

bool Vfs::Init(size_t size)
{

}

VfsFile* Vfs::OpenFile(const char* path)
{

}

bool Vfs::Close(VfsFile* file)
{

}

bool Vfs::CreateDir(const char* path)
{

}

bool Vfs::Rename(const char* src, const char* dest)
{

}

bool Vfs::Remove(const char* path)
{

}

bool Vfs::List(std::vector<std::string>& nodes)
{

}
