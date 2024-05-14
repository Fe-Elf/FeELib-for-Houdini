
#pragma once

#ifndef __GFE_Core_h__
#define __GFE_Core_h__

#include <GFE/Core.h>

#define _GFE_BEGIN namespace gfe {
#define _GFE_END   } // End of Namespace gfe
#define _GFE       gfe::

#define _GFEL_BEGIN _GFE_BEGIN inline namespace Lib {
#define _GFEL_END   _GFE_END } // End of Namespace gfeLib
#define _GFEL       gfe::Lib::
#define _gfel       gfe::Lib
//#undef _gfel

#ifndef NDEBUG
#define GFE_DEBUG_MODE true
#endif





_GFE_BEGIN
inline namespace Core {

    
#if SYS_VERSION_MAJOR_INT > 19 || ( SYS_VERSION_MAJOR_INT == 19 && SYS_VERSION_MINOR_INT == 5 )
#else
#endif

static constexpr int MAX_LOOP_COUNT = std::numeric_limits<int32>::max();
static constexpr int FIND_INVALID_INDEX = -1;



#if 0
    static constexpr int32 INVALID_OFFSET32 = std::numeric_limits<int32>::max(); // 2147483647
    static constexpr int64 INVALID_OFFSET64 = std::numeric_limits<int64>::max(); // 9223372036854775807
#else
    static constexpr int32 INVALID_OFFSET32 = -1;
    static constexpr int64 INVALID_OFFSET64 = -1;
#endif

static constexpr int64 INVALID_OFFSET = INVALID_OFFSET32;



#if 0
        if constexpr (gfe::Core::DEBUG_MODE)
        {
        }
#endif



#ifndef NDEBUG
static constexpr bool DEBUG_MODE = true;
#else
static constexpr bool DEBUG_MODE = false;
#endif





} // End of namespace Core
_GFE_END
#endif
