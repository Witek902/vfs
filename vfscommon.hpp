/**
 * @author Michal Witanowski
 */

#pragma once

#include <iostream>

typedef int int32;
typedef unsigned int uint32;
typedef short int16;
typedef unsigned short uint16;
typedef char int8;
typedef unsigned char uint8;

#define LOG_ERROR(x) std::cout << __FILE__ << ':' << __LINE__ << ": ERROR: " << x << std::endl

#ifdef _DEBUG
    #define LOG_DEBUG(x) std::cout << __FILE__ << ':' << __LINE__ << ": " << x << std::endl
#else
    #define LOG_DEBUG(x)
#endif

// calculate ceil(a/b)
template<typename T>
inline T CeilDivide(T a, T b)
{
    return (a / b) + ((a % b > 0) ? 1 : 0);
}
