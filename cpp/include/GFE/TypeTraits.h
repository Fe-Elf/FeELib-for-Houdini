
#pragma once

#ifndef __GFE_TypeTraits_h__
#define __GFE_TypeTraits_h__

#include <GFE/TypeTraits.h>

#include <GFE/Core.h>



_GFE_BEGIN
inline namespace TypeTraits {
    
    namespace detail
    {
        template <class _Ty>
        class numeric_limits : public std::numeric_limits<_Ty> {};
    
        template <>
        class numeric_limits<fpreal16> : public std::_Num_float_base {
        public:
            _NODISCARD static constexpr float(min)() noexcept {
                return H_REAL16_NRM_MIN;
            }

            _NODISCARD static constexpr float(max)() noexcept {
                return H_REAL16_MAX;
            }

            _NODISCARD static constexpr float lowest() noexcept {
                return -(max)();
            }

            _NODISCARD static constexpr float epsilon() noexcept {
                return H_REAL16_EPSILON;
            }

            _NODISCARD static constexpr float round_error() noexcept {
                return fpreal(0.5);
            }

            _NODISCARD static constexpr float denorm_min() noexcept {
                return H_REAL16_MIN;
            }

            _NODISCARD static constexpr float infinity() noexcept {
                return __builtin_huge_valf();
            }

            _NODISCARD static constexpr float quiet_NaN() noexcept {
                return __builtin_nanf("0");
            }

            _NODISCARD static constexpr float signaling_NaN() noexcept {
                return __builtin_nansf("1");
            }

            static constexpr int digits         = FLT_MANT_DIG;
            static constexpr int digits10       = FLT_DIG;
            static constexpr int max_digits10   = 9;
            static constexpr int max_exponent   = FLT_MAX_EXP;
            static constexpr int max_exponent10 = FLT_MAX_10_EXP;
            static constexpr int min_exponent   = FLT_MIN_EXP;
            static constexpr int min_exponent10 = FLT_MIN_10_EXP;
        };

        
    
    } // End of Namespace detail
    


//#ifndef fpreal16
//#define fpreal16 float
//#endif
    
// template <class _Ty>
// _INLINE_VAR constexpr bool isScalar = std::is_same_v<_Ty, char>           ||
//                                       std::is_same_v<_Ty, wchar_t>        ||
//                                       std::is_same_v<_Ty, signed char>    ||
//                                       std::is_same_v<_Ty, unsigned char>  ||
//                                       std::is_same_v<_Ty, unsigned short> ||
//                                       std::is_same_v<_Ty, short>          ||
//                                       std::is_same_v<_Ty, unsigned int>   ||
//                                       std::is_same_v<_Ty, int64>          ||
//                                       std::is_same_v<_Ty, uint64>         ||
//                                       std::is_same_v<_Ty, fpreal16>       ||
//                                       std::is_same_v<_Ty, float>          ||
//                                       std::is_same_v<_Ty, double>         ;

template <class _Ty>
_INLINE_VAR constexpr bool isStringHolder = std::is_same_v<_Ty, UT_StringHolder>;

template <class _Ty>
_INLINE_VAR constexpr bool isString = std::is_same_v<_Ty, UT_StringHolder> ||
                                      std::is_same_v<_Ty, UT_StringRef>    ||
                                      std::is_same_v<_Ty, char>            ||
                                      std::is_same_v<_Ty, char*>            ;


template <class _Ty>
_INLINE_VAR constexpr bool isScalar = std::is_arithmetic_v<_Ty>      ||
                                      std::is_same_v<_Ty, int64>     ||
                                      std::is_same_v<_Ty, uint64>    ||
                                      std::is_same_v<_Ty, fpreal16>  ;


    
#define __GFE_SPECIALIZATION_IsVector(NUM)                                                                \
template <class _Ty>                                                                                      \
_INLINE_VAR constexpr bool isVector##NUM = std::is_same_v<_Ty, UT_Vector##NUM##T<char>           >||      \
                                           std::is_same_v<_Ty, UT_Vector##NUM##T<wchar_t>        >||      \
                                           std::is_same_v<_Ty, UT_Vector##NUM##T<signed char>    >||      \
                                           std::is_same_v<_Ty, UT_Vector##NUM##T<unsigned char>  >||      \
                                           std::is_same_v<_Ty, UT_Vector##NUM##T<unsigned short> >||      \
                                           std::is_same_v<_Ty, UT_Vector##NUM##T<short>          >||      \
                                           std::is_same_v<_Ty, UT_Vector##NUM##T<unsigned int>   >||      \
                                           std::is_same_v<_Ty, UT_Vector##NUM##T<int>            >||      \
                                           std::is_same_v<_Ty, UT_Vector##NUM##T<int64>          >||      \
                                           std::is_same_v<_Ty, UT_Vector##NUM##T<uint64>         >||      \
                                           std::is_same_v<_Ty, UT_Vector##NUM##T<fpreal16>       >||      \
                                           std::is_same_v<_Ty, UT_Vector##NUM##T<float>          >||      \
                                           std::is_same_v<_Ty, UT_Vector##NUM##T<double>         >;       \

__GFE_SPECIALIZATION_IsVector(2)
__GFE_SPECIALIZATION_IsVector(3)
__GFE_SPECIALIZATION_IsVector(4)

#undef  __GFE_SPECIALIZATION_IsVector



    
#define __GFE_SPECIALIZATION_IsVectorI(NUM)                                                                  \
template <class _Ty>                                                                                         \
_INLINE_VAR constexpr bool isVector##NUM##I = std::is_same_v<_Ty, UT_Vector##NUM##T<char>           >||      \
                                              std::is_same_v<_Ty, UT_Vector##NUM##T<wchar_t>        >||      \
                                              std::is_same_v<_Ty, UT_Vector##NUM##T<signed char>    >||      \
                                              std::is_same_v<_Ty, UT_Vector##NUM##T<unsigned char>  >||      \
                                              std::is_same_v<_Ty, UT_Vector##NUM##T<unsigned short> >||      \
                                              std::is_same_v<_Ty, UT_Vector##NUM##T<short>          >||      \
                                              std::is_same_v<_Ty, UT_Vector##NUM##T<unsigned int>   >||      \
                                              std::is_same_v<_Ty, UT_Vector##NUM##T<int>            >||      \
                                              std::is_same_v<_Ty, UT_Vector##NUM##T<int64>          >||      \
                                              std::is_same_v<_Ty, UT_Vector##NUM##T<uint64>         >;       \

__GFE_SPECIALIZATION_IsVectorI(2)
__GFE_SPECIALIZATION_IsVectorI(3)
__GFE_SPECIALIZATION_IsVectorI(4)

#undef  __GFE_SPECIALIZATION_IsVectorI


    
#define __GFE_SPECIALIZATION_IsVectorF(NUM)                                                                  \
template <class _Ty>                                                                                         \
_INLINE_VAR constexpr bool isVector##NUM##F = std::is_same_v<_Ty, UT_Vector##NUM##T<fpreal16>       >||      \
                                              std::is_same_v<_Ty, UT_Vector##NUM##T<float>          >||      \
                                              std::is_same_v<_Ty, UT_Vector##NUM##T<double>         >;       \

__GFE_SPECIALIZATION_IsVectorF(2)
__GFE_SPECIALIZATION_IsVectorF(3)
__GFE_SPECIALIZATION_IsVectorF(4)

#undef  __GFE_SPECIALIZATION_IsVectorF

    
#define __GFE_SPECIALIZATION_IsMatrix(NUM)                                                                \
template <class _Ty>                                                                                      \
_INLINE_VAR constexpr bool isMatrix##NUM = std::is_same_v<_Ty, UT_Matrix##NUM##T<char>           >||      \
                                           std::is_same_v<_Ty, UT_Matrix##NUM##T<wchar_t>        >||      \
                                           std::is_same_v<_Ty, UT_Matrix##NUM##T<signed char>    >||      \
                                           std::is_same_v<_Ty, UT_Matrix##NUM##T<unsigned char>  >||      \
                                           std::is_same_v<_Ty, UT_Matrix##NUM##T<unsigned short> >||      \
                                           std::is_same_v<_Ty, UT_Matrix##NUM##T<short>          >||      \
                                           std::is_same_v<_Ty, UT_Matrix##NUM##T<unsigned int>   >||      \
                                           std::is_same_v<_Ty, UT_Matrix##NUM##T<int64>          >||      \
                                           std::is_same_v<_Ty, UT_Matrix##NUM##T<uint64>         >||      \
                                           std::is_same_v<_Ty, UT_Matrix##NUM##T<fpreal16>       >||      \
                                           std::is_same_v<_Ty, UT_Matrix##NUM##T<float>          >||      \
                                           std::is_same_v<_Ty, UT_Matrix##NUM##T<double>         >;       \

__GFE_SPECIALIZATION_IsMatrix(2)
__GFE_SPECIALIZATION_IsMatrix(3)
__GFE_SPECIALIZATION_IsMatrix(4)

#undef  __GFE_SPECIALIZATION_IsMatrix



template <class _Ty>
_INLINE_VAR constexpr bool isVector = isVector2<_Ty> ||
                                      isVector3<_Ty> ||
                                      isVector4<_Ty> ;

template <class _Ty>
_INLINE_VAR constexpr bool isVectorF = isVector2F<_Ty> ||
                                       isVector3F<_Ty> ||
                                       isVector4F<_Ty> ;

template <class _Ty>
_INLINE_VAR constexpr bool isVectorI = isVector2I<_Ty> ||
                                       isVector3I<_Ty> ||
                                       isVector4I<_Ty> ;


template <class _Ty>
_INLINE_VAR constexpr bool isMatrix34 = isMatrix3<_Ty> ||
                                        isMatrix4<_Ty> ;

 

template <class _Ty>
_INLINE_VAR constexpr bool isMatrix = isMatrix2<_Ty> ||
                                      isMatrix3<_Ty> ||
                                      isMatrix4<_Ty> ;


template <class _Ty>
_INLINE_VAR constexpr bool isVM = isVector<_Ty> || isMatrix<_Ty> ;


template <class _Ty>
_INLINE_VAR constexpr bool isIFVM = isScalar<_Ty> || isVM<_Ty> ;

template <class _Ty>
_INLINE_VAR constexpr bool isTuple = isIFVM<_Ty>;




        
        
    template <bool _bool, typename _TyTrue, typename _TyFalse>
    struct select_type { using type = typename _TyTrue; };

    template <typename _TyTrue, typename _TyFalse>
    struct select_type<false, _TyTrue, _TyFalse> { using type = typename _TyFalse; };

    template <bool _bool, typename _Ty>
    struct select_value_type { using type = typename _Ty; };

    template <typename _Ty>
    struct select_value_type<false, _Ty> { using type = typename _Ty::value_type; };
    
    
    template <typename _Ty>
    struct get_value_type { using type = typename select_value_type<isScalar<_Ty>, _Ty>::type; };

    template <class _Ty>
    using get_value_type_t = typename get_value_type<_Ty>::type;

        
    template <typename _Ty>
    struct get_page_handle_type { using type = typename select_type<isTuple<_Ty>, _Ty, int8>::type; };
    template <class _Ty>
    using get_page_handle_type_t = typename get_page_handle_type<_Ty>::type;

        
    template <typename _Ty>
    struct get_page_handle_value_type { using type = typename select_type<isTuple<_Ty>, typename get_value_type_t<_Ty>, int8>::type; };

    template <class _Ty>
    using get_page_handle_value_type_t = typename get_page_handle_value_type<_Ty>::type;

        
    

    
    template <class _Ty>
    class numeric_limits : public detail::numeric_limits<get_value_type_t<_Ty>> {};


    

} // End of namespace TypeTraits
_GFE_END
#endif
