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
    vfs.Init(16 * 1024 * 1024);

    assert(true == vfs.CreateDir("0a"));
    assert(true == vfs.CreateDir("0a/1a"));
    assert(true == vfs.CreateDir("0a/1a/2a"));
    assert(true == vfs.CreateDir("0a/1b"));
    assert(true == vfs.CreateDir("0a/1c"));

    assert(true == vfs.CreateDir("0b"));
    assert(true == vfs.CreateDir("0b/1b"));

    assert(false == vfs.CreateDir("0a"));
    assert(false == vfs.CreateDir("0a/1a"));
    assert(false == vfs.CreateDir("0a/blah/2a"));

    std::vector<std::string> list;
    assert(false == vfs.List("blah", list));
    assert(true == vfs.List("0a", list));
    std::cout << "==================\n";
    for (const auto& str : list)
        std::cout << str << std::endl;


    assert(false == vfs.Remove("blah"));
    assert(false == vfs.Remove("0a/blah"));
    assert(false == vfs.Remove("0b"));
    assert(true == vfs.Remove("0b/1b"));
    assert(true == vfs.Remove("0b"));

    std::cout << "==================\n";
    vfs.DebugPrint();

    assert(false == vfs.Rename("blah", "blah"));
    assert(false == vfs.Rename("0a", "0a"));
    assert(true == vfs.Rename("0a/1b", "0moved"));
    assert(false == vfs.Rename("0moved", "0moved"));
    assert(true == vfs.Rename("0moved", "0moved2"));

    std::cout << "==================\n";
    vfs.DebugPrint();
}

void FileTest()
{
    Vfs vfs("test.bin");
    vfs.Init(16 * 1024 * 1024);

    assert(true == vfs.CreateDir("aaa"));

    VfsFile* file;
    int data;
    const int refData = 0x12345678;

    // open non-existing file
    file = vfs.OpenFile("aaa/file", false);
    assert(nullptr == file);

    file = vfs.OpenFile("aaa/file", true);
    assert(nullptr != file);
    assert(file->Read(sizeof(data), &data) == 0); // read empty file
    assert(file->Write(sizeof(refData), &refData) == sizeof(data)); // write something
    assert(file->Seek(0, VfsSeekMode::Begin) == 0); // seek to the beginning
    assert(file->Read(sizeof(data), &data) == sizeof(data)); // read written data
    assert(refData == data); // verify
    assert(file->Read(sizeof(data), &data) == 0); // read after file end
    assert(vfs.Close(file) == true); // close file
    assert(vfs.Close(file) == false); // try to close again

    std::cout << "==================\n";
    vfs.DebugPrint();

    // move the file to a new directory
    assert(true == vfs.CreateDir("bbb"));
    assert(true == vfs.Rename("aaa/file", "bbb/file"));

    file = vfs.OpenFile("bbb/file", true); // try to create existing file
    assert(nullptr == file);
    file = vfs.OpenFile("bbb/file", false);
    assert(nullptr != file);
    assert(file->Read(sizeof(data), &data) == sizeof(data));
    assert(refData == data);
    assert(vfs.Close(file) == true);

    std::cout << "==================\n";
    vfs.DebugPrint();
}

int main(int argc, char** argv)
{
    DirTest();
    FileTest();

    getchar();
    return 0;
}
