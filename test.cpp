/**
 * @author Michal Witanowski
 */

#include "vfs.hpp"

int main(int argc, char** argv)
{
    Vfs vfs("test.bin");
    vfs.Init(16 * 1024 * 1024);

    return 0;
}
