cmake_minimum_required(VERSION 2.6)
project(vfs)

SET(VFS_SOURCES vfs.cpp vfsfile.cpp vfsstructures.cpp)
SET(VFS_HEADERS vfs.hpp vfsfile.hpp vfsstructures.hpp vfscommon.hpp)

SET(CMAKE_CXX_FLAGS       "${CMAKE_CXX_FLAGS} -O2 -std=c++0x")
SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -g -std=c++0x")
ADD_DEFINITIONS("-Wall -Wpedantic")

add_executable(vfsTest test.cpp ${VFS_SOURCES} ${VFS_HEADERS})

IF(CMAKE_BUILD_TYPE MATCHES "Debug")
    ADD_DEFINITIONS("-D_DEBUG")
ENDIF(CMAKE_BUILD_TYPE MATCHES "Debug")

add_executable(vmkfs tools/vmkfs.cpp ${VFS_SOURCES} ${VFS_HEADERS})
add_executable(vmkdir tools/vmkdir.cpp ${VFS_SOURCES} ${VFS_HEADERS})
add_executable(vcp tools/vcp.cpp ${VFS_SOURCES} ${VFS_HEADERS})
add_executable(vmv tools/vmv.cpp ${VFS_SOURCES} ${VFS_HEADERS})
add_executable(vrm tools/vrm.cpp ${VFS_SOURCES} ${VFS_HEADERS})
add_executable(vls tools/vls.cpp ${VFS_SOURCES} ${VFS_HEADERS})
