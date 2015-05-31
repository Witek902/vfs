/**
 * @author Michal Witanowski
 */

#include "vfs.hpp"

#include <assert.h>
#include <iostream>
#include <string>

void DirTest()
{
    Vfs vfs("test.bin");
    VFS_ASSERT(vfs.Init(16 * 1024 * 1024));

    VFS_ASSERT(true == vfs.CreateDir("0a"));
    VFS_ASSERT(true == vfs.CreateDir("0a/1a"));
    VFS_ASSERT(true == vfs.CreateDir("0a/1a/2a"));
    VFS_ASSERT(true == vfs.CreateDir("0a/1b"));
    VFS_ASSERT(true == vfs.CreateDir("0a/1c"));

    VFS_ASSERT(true == vfs.CreateDir("0b"));
    VFS_ASSERT(true == vfs.CreateDir("0b/1b"));

    VFS_ASSERT(false == vfs.CreateDir("0a"));
    VFS_ASSERT(false == vfs.CreateDir("0a/1a"));
    VFS_ASSERT(false == vfs.CreateDir("0a/blah/2a"));

    std::vector<std::string> list;
    VFS_ASSERT(false == vfs.List("blah", list));
    VFS_ASSERT(true == vfs.List("0a", list));
    std::cout << "==================\n";
    for (const auto& str : list)
        std::cout << str << std::endl;


    VFS_ASSERT(false == vfs.Remove("blah"));
    VFS_ASSERT(false == vfs.Remove("0a/blah"));
    VFS_ASSERT(false == vfs.Remove("0b"));
    VFS_ASSERT(true == vfs.Remove("0b/1b"));
    VFS_ASSERT(true == vfs.Remove("0b"));

    std::cout << "==================\n";
    vfs.DebugPrint();

    VFS_ASSERT(false == vfs.Rename("blah", "blah"));
    VFS_ASSERT(false == vfs.Rename("0a", "0a"));
    VFS_ASSERT(true == vfs.Rename("0a/1b", "0moved"));
    VFS_ASSERT(false == vfs.Rename("0moved", "0moved"));
    VFS_ASSERT(true == vfs.Rename("0moved", "0moved2"));

    std::cout << "==================\n";
    vfs.DebugPrint();
}

void FileTest()
{
    Vfs vfs("test.bin");
    VFS_ASSERT(vfs.Init(16 * 1024 * 1024));

    VFS_ASSERT(true == vfs.CreateDir("aaa"));

    VfsFile* file;
    int data;
    const int refData = 0x12345678;

    // open non-existing file
    file = vfs.OpenFile("aaa/file", false);
    VFS_ASSERT(nullptr == file);

    file = vfs.OpenFile("aaa/file", true);
    VFS_ASSERT(nullptr != file);
    VFS_ASSERT(file->Read(sizeof(data), &data) == 0); // read empty file
    VFS_ASSERT(file->Write(sizeof(refData), &refData) == sizeof(data)); // write something
    VFS_ASSERT(file->Seek(0, VfsSeekMode::Begin) == 0); // seek to the beginning
    VFS_ASSERT(file->Read(sizeof(data), &data) == sizeof(data)); // read written data
    VFS_ASSERT(refData == data); // verify
    VFS_ASSERT(file->Read(sizeof(data), &data) == 0); // read after file end
    VFS_ASSERT(vfs.Close(file) == true); // close file
    VFS_ASSERT(vfs.Close(file) == false); // try to close again

    std::cout << "==================\n";
    vfs.DebugPrint();

    // move the file to a new directory
    VFS_ASSERT(true == vfs.CreateDir("bbb"));
    VFS_ASSERT(true == vfs.Rename("aaa/file", "bbb/file"));

    file = vfs.OpenFile("bbb/file", true); // try to create existing file
    VFS_ASSERT(nullptr == file);
    file = vfs.OpenFile("bbb/file", false);
    VFS_ASSERT(nullptr != file);
    VFS_ASSERT(file->Read(sizeof(data), &data) == sizeof(data));
    VFS_ASSERT(refData == data);
    VFS_ASSERT(vfs.Close(file) == true);

    std::cout << "==================\n";
    vfs.DebugPrint();
}


void BigFileTest()
{
    const uint32 fsSize = 16 * 1024 * 1024;
    const int salt = 0x2356abef;

    VfsFile* file;
    Vfs vfs("test.bin");
    VFS_ASSERT(vfs.Init(fsSize));

    file = vfs.OpenFile("file", true);
    uint32 written = 0;
    for (uint32 i = 0; i < fsSize / sizeof(int); ++i)
    {
        uint32 data = i + salt;
        uint32 ret = file->Write(sizeof(data), &data);
        written += ret;
    }
    VFS_ASSERT(file->Seek(0, VfsSeekMode::Curr) == written);
    vfs.Close(file);

    std::cout << "Bytes written: " << written << std::endl;
    VFS_ASSERT(written < fsSize);
    // VFS_ASSERT(written > fsSize / 2);


    file = vfs.OpenFile("file", false);
    uint32 read = 0;
    for (uint32 i = 0; i < fsSize / sizeof(int); ++i)
    {
        int data;
        uint32 ret = file->Read(sizeof(int), &data);
        if (ret < sizeof(int)) // EOF
            break;
        VFS_ASSERT(data == i + salt);
        read += ret;
    }
    VFS_ASSERT(file->Seek(0, VfsSeekMode::Curr) == read);
    vfs.Close(file);

    VFS_ASSERT(read == written);
}

int main(int argc, char** argv)
{
    DirTest();
    FileTest();
    BigFileTest();

    getchar();
    return 0;
}
