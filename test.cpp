/**
 * @author Michal Witanowski
 */

#include "vfs.hpp"
#include <assert.h>

int main(int argc, char** argv)
{
    Vfs vfs("test.bin");
    vfs.Init(16 * 1024 * 1024);

    assert(true == vfs.CreateDir("aaa/bbb/ccc"));
    //assert(true == vfs.CreateDir("bbb"));
    //assert(true == vfs.CreateDir("aaa/ccc"));

    getchar();
    return 0;
}
