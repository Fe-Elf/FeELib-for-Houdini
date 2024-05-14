
#pragma once

#ifndef __UFE_Math_h__
#define __UFE_Math_h__

#include <UFE/Math.h>

#include <UFE/Type.h>

_UFE_BEGIN
inline namespace Math {

    namespace detail
    {
        template<typename _Ts>
        SYS_FORCE_INLINE static _Ts scalarDistance2(const _Ts val0, const _Ts val1)
        {
            const _Ts dist = val0 > val1 ? val0 - val1 : val1 - val0;
            return dist * dist;
        }

        template<typename _Tv>
        SYS_FORCE_INLINE static typename _Tv::value_type vectorDistance2(const _Tv& val0, const _Tv& val1)
        { return val0.distance2(val1); }

        
        //template<typename _Ty>
        //SYS_FORCE_INLINE static get_value_type_t<_Ty> lengthVec(const _Ty& val)
        //{
        //    using value_type = typename GFE_Type::get_value_type_t<_Ty>;
        //    if constexpr (GFE_Type::isVector<_Ty>)
        //        return val.length();
        //    else
        //        return val;
        //}
        //
        //template<typename _Ty>
        //SYS_FORCE_INLINE static _Ty lengthScalar(const _Ty val)
        //{
        //    if constexpr (GFE_Type::isScalar<_Ty>)
        //        return val;
        //    else
        //        return val;
        //}
    } // End of Namespace detail



    
    /// From OpenVDB/math/Math.h
    /// @brief  Pi constant taken from Boost to match old behaviour
    /// @note   Available in C++20
    template <typename T> inline constexpr T           pi() { return 3.141592653589793238462643383279502884e+00 ; }
    template <>           inline constexpr float32     pi() { return 3.141592653589793238462643383279502884e+00F; }
    template <>           inline constexpr float64     pi() { return 3.141592653589793238462643383279502884e+00 ; }
    template <>           inline constexpr long double pi() { return 3.141592653589793238462643383279502884e+00L; }

    static constexpr float64 PI = 3.141592653589793238462643383279502884e+00;

    
    template<typename _Ty>
    SYS_FORCE_INLINE static _Ty lerp(const _Ty& v0, const _Ty& v1, const fpreal bias)
    { return v0 * (1.0 - bias) + v1 * bias; }
    
        
    template<typename _Ty>
    SYS_FORCE_INLINE static _Ty radians(const _Ty degrees)
    { return degrees * PI / 180; }

    template<typename _Ty>
    SYS_FORCE_INLINE static _Ty degrees(const _Ty radians)
    { return radians * 180 / PI; }


    template<typename _Ty>
    SYS_FORCE_INLINE static typename _Ty::value_type tupleDistance2(const _Ty& val0, const _Ty& val1)
    {
        if constexpr(::std::is_same_v<_Ty, int8> ||
                     ::std::is_same_v<_Ty, int16> ||
                     ::std::is_same_v<_Ty, int32> ||
                     ::std::is_same_v<_Ty, int64> ||
                     ::std::is_same_v<_Ty, fpreal16> ||
                     ::std::is_same_v<_Ty, fpreal32> ||
                     ::std::is_same_v<_Ty, fpreal64> )
        {
            return detail::scalarDistance2(val0, val1);
        }
        else
            return detail::vectorDistance2(val0, val1);
    }
        
    #define __GFE_Math_FUNC_tupleDistance2(TYPE)                                   \
    SYS_FORCE_INLINE static TYPE tupleDistance2(const TYPE val0, const TYPE val1)  \
    { return detail::scalarDistance2(val0, val1); }                                \

    __GFE_Math_FUNC_tupleDistance2(int8);
    __GFE_Math_FUNC_tupleDistance2(int16);
    __GFE_Math_FUNC_tupleDistance2(int32);
    __GFE_Math_FUNC_tupleDistance2(int64);
    __GFE_Math_FUNC_tupleDistance2(fpreal16);
    __GFE_Math_FUNC_tupleDistance2(fpreal32);
    __GFE_Math_FUNC_tupleDistance2(fpreal64);
        
    #undef __GFE_Math_FUNC_tupleDistance2
        

    template<typename VECTOR_T, typename T_value_type>
    SYS_FORCE_INLINE static VECTOR_T vlerp(const VECTOR_T a, const VECTOR_T b, const T_value_type t)
    { return lerp(a, b, t); }

    template<>
    SYS_FORCE_INLINE static int16 vlerp<int16, fpreal16>(const int16 a, const int16 b, const fpreal16 t)
    { return a + (b - a) * t; }

    template<>
    SYS_FORCE_INLINE static int vlerp<int, fpreal32>(const int a, const int b, const fpreal32 t)
    { return a + (b - a) * t; }

    template<>
    SYS_FORCE_INLINE static int64 vlerp<int64, fpreal64>(const int64 a, const int64 b, const fpreal64 t)
    { return a + (b - a) * t; }

    template<>
    SYS_FORCE_INLINE static fpreal16 vlerp<fpreal16, fpreal16>(const fpreal16 a, const fpreal16 b, const fpreal16 t)
    { return a + (b - a) * t; }

    template<>
    SYS_FORCE_INLINE static fpreal32 vlerp<fpreal32, fpreal32>(const fpreal32 a, const fpreal32 b, const fpreal32 t)
    { return lerp(a, b, t); }

    template<>
    SYS_FORCE_INLINE static fpreal vlerp<fpreal, fpreal>(const fpreal a, const fpreal b, const fpreal t)
    { return lerp(a, b, t); }



    
} // End of Namespace Math
_UFE_END


#endif
