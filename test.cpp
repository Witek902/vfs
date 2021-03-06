/**
 * @author Michal Witanowski
 */

#include "vfs.hpp"

#include <assert.h>
#include <string.h>
#include <iostream>
#include <string>

void DirTest()
{
    Vfs vfs;
    VFS_ASSERT(vfs.Init("test.bin", 16 * 1024 * 1024));

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
    Vfs vfs;
    VFS_ASSERT(vfs.Init("test.bin", 16 * 1024 * 1024));

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
    const uint32 fsSize = 64 * 1024 * 1024;
    const uint32 bufferSize = 40841; // (in bytes)
    uint8 buffer[bufferSize];

    VfsFile* file;
    Vfs vfs;
    VFS_ASSERT(vfs.Init("test.bin", fsSize));

    file = vfs.OpenFile("file", true);
    uint32 written = 0;
    for (uint32 i = 0; i < fsSize; i += bufferSize)
    {
        for (uint32 j = 0; j < bufferSize; j++)
            buffer[j] = static_cast<uint8>(i + j);

        uint32 ret = file->Write(sizeof(buffer), buffer);
        written += ret;
        if (ret < sizeof(buffer)) // EOF
            break;
    }
    VFS_ASSERT(file->Seek(0, VfsSeekMode::Curr) == written);
    vfs.Close(file);

    std::cout << "Bytes written: " << written << std::endl;
    VFS_ASSERT(written < fsSize);
    VFS_ASSERT(written > fsSize / 2);


    file = vfs.OpenFile("file", false);
    uint32 read = 0;
    for (uint32 i = 0; i < fsSize; i += bufferSize)
    {
        uint32 ret = file->Read(sizeof(buffer), buffer);
        read += ret;
        if (ret < sizeof(buffer)) // EOF
            break;

        for (uint32 j = 0; j < ret; j++)
            VFS_ASSERT(buffer[j] == static_cast<uint8>(i + j));
    }
    VFS_ASSERT(file->Seek(0, VfsSeekMode::Curr) == read);
    vfs.Close(file);

    VFS_ASSERT(read == written);
}

void FileStressTest()
{
    struct FileInfo
    {
        std::string name;
        uint32 size;
    };

    const uint32 fsSize = 16 * 1024 * 1024;
    std::vector<FileInfo> files;

    VfsFile* file;

    {
        Vfs vfs;
        VFS_ASSERT(vfs.Init("test.bin", fsSize));

        for (int i = 0; i < 2000; ++i)
        {
            if (files.size() > 0 && (i % 4 == 3)) // remove random file with 25% probability
            {
                size_t index = rand() % files.size();
                const std::string& path = files[index].name;
                VFS_ASSERT(vfs.Remove(path));

                files[index] = files[files.size() - 1];
                files.pop_back();
            }
            else // add random file with 75% probability
            {
                std::string fileName = "f" + std::to_string(i);
                file = vfs.OpenFile(fileName, true);
                if (file != nullptr)
                {
                    uint32 desiredSize = rand() % (64 * 1024) + 1;
                    uint8* buff = new uint8[desiredSize];
                    memset(buff, i, desiredSize);
                    uint32 written = file->Write(desiredSize, buff);
                    VFS_ASSERT(written <= desiredSize);
                    vfs.Close(file);
                    delete buff;

                    FileInfo fileInfo;
                    fileInfo.name = fileName;
                    fileInfo.size = written;
                    files.push_back(fileInfo);
                }
            }
        }
    }

    {
        Vfs vfs;
        VFS_ASSERT(vfs.Open("test.bin"));
        for (const auto& f : files)
        {
            file = vfs.OpenFile(f.name, false);
            VFS_ASSERT(file != nullptr);

            uint32 size = file->Seek(0, VfsSeekMode::End);

            VFS_ASSERT(size == f.size);
            vfs.Close(file);
        }
    }
}

int main(int argc, char** argv)
{
    DirTest();
    FileTest();
    BigFileTest();
    FileStressTest();

    std::cout << "DONE." << std::endl;
    getchar();
    return 0;
}
