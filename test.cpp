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

int main(int argc, char** argv)
{
    DirTest();

    getchar();
    return 0;
}
