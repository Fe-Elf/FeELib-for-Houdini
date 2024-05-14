
#pragma once

#ifndef __UFE_Core_h__
#define __UFE_Core_h__

#include <UFE/Core.h>




#define _UFE_BEGIN namespace ufe {
#define _UFE_END   } // End of namespace ufe
#define _UFE       ::ufe::

#ifndef NDEBUG
#define UFE_DEBUG_MODE true
#endif


_UFE_BEGIN
inline namespace Core {

    


#if 0
    if constexpr (ufe::Core::DEBUG_MODE)
    {
    }
#endif


#ifndef NDEBUG
    static constexpr bool DEBUG_MODE = true;
#else
    static constexpr bool DEBUG_MODE = false;
#endif



} // End of namespace Core


_UFE_END




#endif
