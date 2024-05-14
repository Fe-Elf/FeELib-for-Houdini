
#pragma once

#ifndef __UFE_Type_h__
#define __UFE_Type_h__

#include <UFE/Type.h>

#include <UFE/Core.h>

_UFE_BEGIN
inline namespace Type {
    
    typedef signed char        int8;
    typedef short              int16;
    typedef int                int32;
    typedef long long          int64;
    typedef unsigned char      uint8;
    typedef unsigned short     uint16;
    typedef unsigned int       uint32;
    typedef unsigned long long uint64;

    typedef signed char        int_least8;
    typedef short              int_least16;
    typedef int                int_least32;
    typedef long long          int_least64;
    typedef unsigned char      uint_least8;
    typedef unsigned short     uint_least16;
    typedef unsigned int       uint_least32;
    typedef unsigned long long uint_least64;

    typedef signed char        int_fast8;
    typedef int                int_fast16;
    typedef int                int_fast32;
    typedef long long          int_fast64;
    typedef unsigned char      uint_fast8;
    typedef unsigned int       uint_fast16;
    typedef unsigned int       uint_fast32;
    typedef unsigned long long uint_fast64;

    typedef long long          intmax;
    typedef unsigned long long uintmax;
    
    using float32 = float;
    using float64 = double;

    
} // End of namespace Type
_UFE_END
#endif
