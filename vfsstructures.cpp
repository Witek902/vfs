/**
 * @author Michal Witanowski
 */

#include "vfsstructures.hpp"

const uint32 INVALID_INDEX = static_cast<uint32>(-1);

INode::INode()
{
    type = INodeType::File;
    ptrDepth = 0;
    size = 0;
    usage = 0;

    for (int i = 0; i < INODE_PTRS; ++i)
        blockPtr[i] = INVALID_INDEX;
}

Directory::Directory()
{
    inodeID = INVALID_INDEX;
    memset(name, 0, sizeof(name));
}