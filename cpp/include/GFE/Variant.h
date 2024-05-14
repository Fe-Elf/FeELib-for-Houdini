
#pragma once

#ifndef __GFE_Variant_h__
#define __GFE_Variant_h__

#include <GFE/Variant.h>

#include <GFE/Type.h>


_GFE_BEGIN
inline namespace Variant {
    using namespace gfe::EnumClass;

    template <typename _Ty0, typename... _Ty1>
    struct variant_concat { using type = void; };

    //template <typename... _Ty0, typename... _Ty1>
    //struct variant_concat { using type = void; };

    //template <typename... _Ty0, typename... _Ty1>
    //struct variant_concat<std::variant<_Ty0...>, _Ty1...> { using type = std::variant<_Ty0..., _Ty1...>; };

    template <typename... _Ty0, typename... _Ty1>
    struct variant_concat<std::variant<_Ty0...>, std::variant<_Ty1...>> { using type = std::variant<_Ty0..., _Ty1...>; };

    //template <typename... _Ty0, typename... _Ty1, typename... _Ty2>
    //struct variant_concat<std::variant<_Ty0...>, std::variant<_Ty1...>, std::variant<_Ty2...>> { using type = std::variant<_Ty0..., _Ty1..., _Ty2...>; };

    template <typename... _Ty0, typename... _Ty1>
    using variant_concat_t = typename variant_concat<_Ty0..., _Ty1...>::type;

    //template <typename... _Ty0, typename... _Ty1, typename... _Ty2>
    //using variant_concat_t = typename variant_concat<_Ty0..., _Ty1..., _Ty2...>::type;

    template<typename _Ty, typename _TVarient>
    struct isVariantMember;

    template<typename _Ty, typename... _TVarientMember>
    struct isVariantMember<_Ty, std::variant<_TVarientMember...>> : public std::disjunction<std::is_same<_Ty, _TVarientMember>...> {};

    //template<typename _Ty, typename... ALL_T>
    //_INLINE_VAR constexpr bool isVariantMember_v = isVariantMember<_Ty, std::variant<ALL_T...>>::value;



    
template <int _Val>
using attribTupleSizeConstant = std::integral_constant<int, _Val>;

using attribTupleSizeConstant1  = attribTupleSizeConstant<1>;
using attribTupleSizeConstant2  = attribTupleSizeConstant<2>;
using attribTupleSizeConstant3  = attribTupleSizeConstant<3>;
using attribTupleSizeConstant4  = attribTupleSizeConstant<4>;
using attribTupleSizeConstant9  = attribTupleSizeConstant<9>;
using attribTupleSizeConstant16 = attribTupleSizeConstant<16>;
    
using attribTupleSizeVariant = std::variant<   attribTupleSizeConstant1 ,
                                            attribTupleSizeConstant2 ,
                                            attribTupleSizeConstant3 ,
                                            attribTupleSizeConstant4 ,
                                            attribTupleSizeConstant9 ,
                                            attribTupleSizeConstant16>;
        
SYS_FORCE_INLINE attribTupleSizeVariant getAttribTupleSizeVariant(const int v)
{
    switch (v)
    {
    case 1:  return attribTupleSizeConstant<1> {}; break;
    case 2:  return attribTupleSizeConstant<2> {}; break;
    case 3:  return attribTupleSizeConstant<3> {}; break;
    case 4:  return attribTupleSizeConstant<4> {}; break;
    case 9:  return attribTupleSizeConstant<9> {}; break;
    case 16: return attribTupleSizeConstant<16>{}; break;
    default: break;
    }
    UT_ASSERT_MSG(0, "Unhandled Attrib Tuple Size");
    return attribTupleSizeConstant<1>{};
}

        
SYS_FORCE_INLINE auto getAttribTupleSizeVariant(const GA_Attribute& attrib)
{ return getAttribTupleSizeVariant(attrib.getTupleSize()); }

SYS_FORCE_INLINE auto getAttribTupleSizeVariant(const GA_Attribute* const attrib)
{ UT_ASSERT_P(attrib); return getAttribTupleSizeVariant(*attrib); }

    
#define __GFE_Variant_CreateMacroStorage(MacroName) \
        MacroName(I)                                \
        MacroName(F)                                \
        MacroName(S)                                \
        MacroName(IF)                               \
        MacroName(IS)                               \
        MacroName(IFS)                              \
        MacroName(FVF)                              \
        MacroName(V2)                               \
        MacroName(V3)                               \
        MacroName(V4)                               \
        MacroName(V2I)                              \
        MacroName(V3I)                              \
        MacroName(V4I)                              \
        MacroName(V2F)                              \
        MacroName(V3F)                              \
        MacroName(V4F)                              \
        MacroName(VI)                               \
        MacroName(VF)                               \
        MacroName(V)                                \
        MacroName(VM)                               \
        MacroName(IFV)                              \
        MacroName(IFVM)                             \


    
    
    template <gfe::AttribStorage _Val>
    using attribStorageConstant = std::integral_constant<gfe::AttribStorage, _Val>;
    
    using attribStorageConstantI8   = attribStorageConstant<gfe::AttribStorage::Int8>;
    using attribStorageConstantI16  = attribStorageConstant<gfe::AttribStorage::Int16>;
    using attribStorageConstantI32  = attribStorageConstant<gfe::AttribStorage::Int32>;
    using attribStorageConstantI64  = attribStorageConstant<gfe::AttribStorage::Int64>;
    using attribStorageConstantF16  = attribStorageConstant<gfe::AttribStorage::Float16>;
    using attribStorageConstantF32  = attribStorageConstant<gfe::AttribStorage::Float32>;
    using attribStorageConstantF64  = attribStorageConstant<gfe::AttribStorage::Float64>;
    using attribStorageConstantS    = attribStorageConstant<gfe::AttribStorage::String>;
    
#define __GFE_Variant_SpecializationConstantV(NUM)\
        using attribStorageConstantV##NUM##I32  = attribStorageConstant<gfe::AttribStorage::Vector##NUM##I>;\
        using attribStorageConstantV##NUM##I64  = attribStorageConstant<gfe::AttribStorage::Vector##NUM##L>;\
        using attribStorageConstantV##NUM##F16  = attribStorageConstant<gfe::AttribStorage::Vector##NUM##H>;\
        using attribStorageConstantV##NUM##F32  = attribStorageConstant<gfe::AttribStorage::Vector##NUM##F>;\
        using attribStorageConstantV##NUM##F64  = attribStorageConstant<gfe::AttribStorage::Vector##NUM##D>;\
    
        __GFE_Variant_SpecializationConstantV(2)
        __GFE_Variant_SpecializationConstantV(3)
        __GFE_Variant_SpecializationConstantV(4)
        
#undef __GFE_Variant_SpecializationConstantV
        

#define __GFE_Variant_SpecializationConstantM(NUM)\
        using attribStorageConstantM##NUM##F32 = attribStorageConstant<gfe::AttribStorage::Matrix##NUM##F>;\
        using attribStorageConstantM##NUM##F64 = attribStorageConstant<gfe::AttribStorage::Matrix##NUM##D>;\
    
        __GFE_Variant_SpecializationConstantM(2)
        __GFE_Variant_SpecializationConstantM(3)
        __GFE_Variant_SpecializationConstantM(4)
        
#undef __GFE_Variant_SpecializationConstantM

    using attribStorageVariantS = std::variant<   attribStorageConstantS>;

    using attribStorageVariantI = std::variant<   attribStorageConstantI8 ,
                                               attribStorageConstantI16,
                                               attribStorageConstantI32,
                                               attribStorageConstantI64>;

    using attribStorageVariantF = std::variant<   attribStorageConstantF16,
                                               attribStorageConstantF32,
                                               attribStorageConstantF64>;
    
    using attribStorageVariantIF  = typename variant_concat_t<attribStorageVariantI, attribStorageVariantF>;
    
    using attribStorageVariantIS  = typename variant_concat_t<attribStorageVariantI, attribStorageVariantS>;
    using attribStorageVariantIFS = typename variant_concat_t<attribStorageVariantIF, attribStorageVariantS>;

    
    
#define __GFE_Variant_SpecializationConstantVI(NUM)                                              \
        using attribStorageVariantV##NUM##I = std::variant<   attribStorageConstantV##NUM##I32,  \
                                                           attribStorageConstantV##NUM##I64>;    \
    
        __GFE_Variant_SpecializationConstantVI(2)
        __GFE_Variant_SpecializationConstantVI(3)
        __GFE_Variant_SpecializationConstantVI(4)
        
#undef __GFE_Variant_SpecializationConstantVI

    
#define __GFE_Variant_SpecializationConstantVF(NUM) \
        using attribStorageVariantV##NUM##F = std::variant<   attribStorageConstantV##NUM##F16,  \
                                                           attribStorageConstantV##NUM##F32,     \
                                                           attribStorageConstantV##NUM##F64>;    \
    
        __GFE_Variant_SpecializationConstantVF(2)
        __GFE_Variant_SpecializationConstantVF(3)
        __GFE_Variant_SpecializationConstantVF(4)
        
#undef __GFE_Variant_SpecializationConstantVF
        
    
#define __GFE_Variant_SpecializationConstantV(NUM)  \
        using attribStorageVariantV##NUM## = typename variant_concat_t<attribStorageVariantV##NUM##I, attribStorageVariantV##NUM##F>;
    
        __GFE_Variant_SpecializationConstantV(2)
        __GFE_Variant_SpecializationConstantV(3)
        __GFE_Variant_SpecializationConstantV(4)
        
#undef __GFE_Variant_SpecializationConstantV

    
#define __GFE_Variant_SpecializationConstantM(NUM)\
        using attribStorageVariantM##NUM## = std::variant<   attribStorageConstantM##NUM##F32,  \
                                                          attribStorageConstantM##NUM##F64>; \
    
        __GFE_Variant_SpecializationConstantM(2)
        __GFE_Variant_SpecializationConstantM(3)
        __GFE_Variant_SpecializationConstantM(4)
        
#undef __GFE_Variant_SpecializationConstantM

    
    //using attribStorageVariantV   = typename variant_concat_t<attribStorageVariantV2, attribStorageVariantV3, attribStorageVariantV4>;
    //using attribStorageVariantM   = typename variant_concat_t<attribStorageVariantM2, attribStorageVariantM3, attribStorageVariantM4>;
    using attribStorageVariantV    = typename variant_concat_t<variant_concat_t<attribStorageVariantV2, attribStorageVariantV3>, attribStorageVariantV4>;
    using attribStorageVariantM    = typename variant_concat_t<variant_concat_t<attribStorageVariantM2, attribStorageVariantM3>, attribStorageVariantM4>;
    using attribStorageVariantVF   = typename variant_concat_t<variant_concat_t<attribStorageVariantV2F, attribStorageVariantV3F>, attribStorageVariantV4F>;
    using attribStorageVariantFVF  = typename variant_concat_t<attribStorageVariantF, attribStorageVariantVF>;
    using attribStorageVariantVI   = typename variant_concat_t<variant_concat_t<attribStorageVariantV2I, attribStorageVariantV3I>, attribStorageVariantV4I>;
    using attribStorageVariantIFV  = typename variant_concat_t<attribStorageVariantIF,  attribStorageVariantV>;
    using attribStorageVariantVM   = typename variant_concat_t<attribStorageVariantV,   attribStorageVariantM>;
    using attribStorageVariantIFVM = typename variant_concat_t<attribStorageVariantIFV, attribStorageVariantM>;




    

#define __GFE_Variant_SpecializationSwitchVariantS\
        case gfe::AttribStorage::String:   return attribStorageConstant<gfe::AttribStorage::String>  {}; break;\

#define __GFE_Variant_SpecializationSwitchVariantI\
        case gfe::AttribStorage::Int8:     return attribStorageConstant<gfe::AttribStorage::Int8>    {}; break;\
        case gfe::AttribStorage::Int16:    return attribStorageConstant<gfe::AttribStorage::Int16>   {}; break;\
        case gfe::AttribStorage::Int32:    return attribStorageConstant<gfe::AttribStorage::Int32>   {}; break;\
        case gfe::AttribStorage::Int64:    return attribStorageConstant<gfe::AttribStorage::Int64>   {}; break;\
    
#define __GFE_Variant_SpecializationSwitchVariantF\
        case gfe::AttribStorage::Float16: return attribStorageConstant<gfe::AttribStorage::Float16>{}; break;\
        case gfe::AttribStorage::Float32: return attribStorageConstant<gfe::AttribStorage::Float32>{}; break;\
        case gfe::AttribStorage::Float64: return attribStorageConstant<gfe::AttribStorage::Float64>{}; break;\

#define __GFE_Variant_SpecializationSwitchVariantVIN(NUM)\
        case gfe::AttribStorage::Vector##NUM##I: return attribStorageConstant<gfe::AttribStorage::Vector##NUM##I>{}; break;\
        case gfe::AttribStorage::Vector##NUM##L: return attribStorageConstant<gfe::AttribStorage::Vector##NUM##L>{}; break;\

#define __GFE_Variant_SpecializationSwitchVariantVFN(NUM)\
        case gfe::AttribStorage::Vector##NUM##H: return attribStorageConstant<gfe::AttribStorage::Vector##NUM##H>{}; break;\
        case gfe::AttribStorage::Vector##NUM##F: return attribStorageConstant<gfe::AttribStorage::Vector##NUM##F>{}; break;\
        case gfe::AttribStorage::Vector##NUM##D: return attribStorageConstant<gfe::AttribStorage::Vector##NUM##D>{}; break;\

#define __GFE_Variant_SpecializationSwitchVariantVN(NUM)   \
        __GFE_Variant_SpecializationSwitchVariantVIN(NUM)  \
        __GFE_Variant_SpecializationSwitchVariantVFN(NUM)  \
    

#define __GFE_Variant_SpecializationSwitchVariantMN(NUM) \
        case gfe::AttribStorage::Matrix##NUM##F: return attribStorageConstant<gfe::AttribStorage::Matrix##NUM##F>{}; break;\
        case gfe::AttribStorage::Matrix##NUM##D: return attribStorageConstant<gfe::AttribStorage::Matrix##NUM##D>{}; break;\

    
#define __GFE_Variant_SpecializationSwitchVariantVI       \
        __GFE_Variant_SpecializationSwitchVariantVIN(2)   \
        __GFE_Variant_SpecializationSwitchVariantVIN(3)   \
        __GFE_Variant_SpecializationSwitchVariantVIN(4)   \

    
#define __GFE_Variant_SpecializationSwitchVariantVF       \
        __GFE_Variant_SpecializationSwitchVariantVFN(2)   \
        __GFE_Variant_SpecializationSwitchVariantVFN(3)   \
        __GFE_Variant_SpecializationSwitchVariantVFN(4)   \

    
#define __GFE_Variant_SpecializationSwitchVariantFVF      \
        __GFE_Variant_SpecializationSwitchVariantF        \
        __GFE_Variant_SpecializationSwitchVariantVF       \

    
#define __GFE_Variant_SpecializationSwitchVariantV2I __GFE_Variant_SpecializationSwitchVariantVIN(2)
#define __GFE_Variant_SpecializationSwitchVariantV3I __GFE_Variant_SpecializationSwitchVariantVIN(3)
#define __GFE_Variant_SpecializationSwitchVariantV4I __GFE_Variant_SpecializationSwitchVariantVIN(4)

#define __GFE_Variant_SpecializationSwitchVariantV2F __GFE_Variant_SpecializationSwitchVariantVFN(2)
#define __GFE_Variant_SpecializationSwitchVariantV3F __GFE_Variant_SpecializationSwitchVariantVFN(3)
#define __GFE_Variant_SpecializationSwitchVariantV4F __GFE_Variant_SpecializationSwitchVariantVFN(4)

    
#define __GFE_Variant_SpecializationSwitchVariantIF \
        __GFE_Variant_SpecializationSwitchVariantI  \
        __GFE_Variant_SpecializationSwitchVariantF  \
        
#define __GFE_Variant_SpecializationSwitchVariantIS \
        __GFE_Variant_SpecializationSwitchVariantI  \
        __GFE_Variant_SpecializationSwitchVariantS  \
        
#define __GFE_Variant_SpecializationSwitchVariantIFS \
        __GFE_Variant_SpecializationSwitchVariantIF  \
        __GFE_Variant_SpecializationSwitchVariantS   \
        
#define __GFE_Variant_SpecializationSwitchVariantV      \
        __GFE_Variant_SpecializationSwitchVariantVN(2)  \
        __GFE_Variant_SpecializationSwitchVariantVN(3)  \
        __GFE_Variant_SpecializationSwitchVariantVN(4)  \

#define __GFE_Variant_SpecializationSwitchVariantV2 __GFE_Variant_SpecializationSwitchVariantVN(2)
#define __GFE_Variant_SpecializationSwitchVariantV3 __GFE_Variant_SpecializationSwitchVariantVN(3)
#define __GFE_Variant_SpecializationSwitchVariantV4 __GFE_Variant_SpecializationSwitchVariantVN(4)

#define __GFE_Variant_SpecializationSwitchVariantM      \
        __GFE_Variant_SpecializationSwitchVariantMN(2)  \
        __GFE_Variant_SpecializationSwitchVariantMN(3)  \
        __GFE_Variant_SpecializationSwitchVariantMN(4)  \

#define __GFE_Variant_SpecializationSwitchVariantM2 __GFE_Variant_SpecializationSwitchVariantMN(2)
#define __GFE_Variant_SpecializationSwitchVariantM3 __GFE_Variant_SpecializationSwitchVariantMN(3)
#define __GFE_Variant_SpecializationSwitchVariantM4 __GFE_Variant_SpecializationSwitchVariantMN(4)

#define __GFE_Variant_SpecializationSwitchVariantVM    \
        __GFE_Variant_SpecializationSwitchVariantV     \
        __GFE_Variant_SpecializationSwitchVariantM     \

#define __GFE_Variant_SpecializationSwitchVariantIFV   \
        __GFE_Variant_SpecializationSwitchVariantIF    \
        __GFE_Variant_SpecializationSwitchVariantV     \
        
#define __GFE_Variant_SpecializationSwitchVariantIFVM  \
        __GFE_Variant_SpecializationSwitchVariantIFV   \
        __GFE_Variant_SpecializationSwitchVariantM     \





    

#define __GFE_Variant_SpecializationSwitchVariantTrueS        \
        case gfe::AttribStorage::String:   return true; break; \

#define __GFE_Variant_SpecializationSwitchVariantTrueI        \
        case gfe::AttribStorage::Int8:     return true; break; \
        case gfe::AttribStorage::Int16:    return true; break; \
        case gfe::AttribStorage::Int32:    return true; break; \
        case gfe::AttribStorage::Int64:    return true; break; \

#define __GFE_Variant_SpecializationSwitchVariantTrueF        \
        case gfe::AttribStorage::Float16: return true; break; \
        case gfe::AttribStorage::Float32: return true; break; \
        case gfe::AttribStorage::Float64: return true; break; \

#define __GFE_Variant_SpecializationSwitchVariantTrueVIN(NUM)          \
        case gfe::AttribStorage::Vector##NUM##I: return true; break; \
        case gfe::AttribStorage::Vector##NUM##L: return true; break; \

#define __GFE_Variant_SpecializationSwitchVariantTrueVFN(NUM)          \
        case gfe::AttribStorage::Vector##NUM##H: return true; break; \
        case gfe::AttribStorage::Vector##NUM##F: return true; break; \
        case gfe::AttribStorage::Vector##NUM##D: return true; break; \

#define __GFE_Variant_SpecializationSwitchVariantTrueVN(NUM)   \
        __GFE_Variant_SpecializationSwitchVariantTrueVIN(NUM)  \
        __GFE_Variant_SpecializationSwitchVariantTrueVFN(NUM)  \
    

#define __GFE_Variant_SpecializationSwitchVariantTrueMN(NUM)           \
        case gfe::AttribStorage::Matrix##NUM##F: return true; break; \
        case gfe::AttribStorage::Matrix##NUM##D: return true; break; \

    
#define __GFE_Variant_SpecializationSwitchVariantTrueVI       \
        __GFE_Variant_SpecializationSwitchVariantTrueVIN(2)   \
        __GFE_Variant_SpecializationSwitchVariantTrueVIN(3)   \
        __GFE_Variant_SpecializationSwitchVariantTrueVIN(4)   \

    
#define __GFE_Variant_SpecializationSwitchVariantTrueVF       \
        __GFE_Variant_SpecializationSwitchVariantTrueVFN(2)   \
        __GFE_Variant_SpecializationSwitchVariantTrueVFN(3)   \
        __GFE_Variant_SpecializationSwitchVariantTrueVFN(4)   \

    
#define __GFE_Variant_SpecializationSwitchVariantTrueFVF      \
        __GFE_Variant_SpecializationSwitchVariantTrueF        \
        __GFE_Variant_SpecializationSwitchVariantTrueVF       \

    
#define __GFE_Variant_SpecializationSwitchVariantTrueV2I __GFE_Variant_SpecializationSwitchVariantTrueVIN(2)
#define __GFE_Variant_SpecializationSwitchVariantTrueV3I __GFE_Variant_SpecializationSwitchVariantTrueVIN(3)
#define __GFE_Variant_SpecializationSwitchVariantTrueV4I __GFE_Variant_SpecializationSwitchVariantTrueVIN(4)

#define __GFE_Variant_SpecializationSwitchVariantTrueV2F __GFE_Variant_SpecializationSwitchVariantTrueVFN(2)
#define __GFE_Variant_SpecializationSwitchVariantTrueV3F __GFE_Variant_SpecializationSwitchVariantTrueVFN(3)
#define __GFE_Variant_SpecializationSwitchVariantTrueV4F __GFE_Variant_SpecializationSwitchVariantTrueVFN(4)

    
#define __GFE_Variant_SpecializationSwitchVariantTrueIF \
        __GFE_Variant_SpecializationSwitchVariantTrueI  \
        __GFE_Variant_SpecializationSwitchVariantTrueF  \
        
#define __GFE_Variant_SpecializationSwitchVariantTrueIS \
        __GFE_Variant_SpecializationSwitchVariantTrueI  \
        __GFE_Variant_SpecializationSwitchVariantTrueS  \
        
#define __GFE_Variant_SpecializationSwitchVariantTrueIFS \
        __GFE_Variant_SpecializationSwitchVariantTrueIF  \
        __GFE_Variant_SpecializationSwitchVariantTrueS   \
        
#define __GFE_Variant_SpecializationSwitchVariantTrueV      \
        __GFE_Variant_SpecializationSwitchVariantTrueVN(2)  \
        __GFE_Variant_SpecializationSwitchVariantTrueVN(3)  \
        __GFE_Variant_SpecializationSwitchVariantTrueVN(4)  \

#define __GFE_Variant_SpecializationSwitchVariantTrueV2 __GFE_Variant_SpecializationSwitchVariantTrueVN(2)
#define __GFE_Variant_SpecializationSwitchVariantTrueV3 __GFE_Variant_SpecializationSwitchVariantTrueVN(3)
#define __GFE_Variant_SpecializationSwitchVariantTrueV4 __GFE_Variant_SpecializationSwitchVariantTrueVN(4)

#define __GFE_Variant_SpecializationSwitchVariantTrueM      \
        __GFE_Variant_SpecializationSwitchVariantTrueMN(2)  \
        __GFE_Variant_SpecializationSwitchVariantTrueMN(3)  \
        __GFE_Variant_SpecializationSwitchVariantTrueMN(4)  \

#define __GFE_Variant_SpecializationSwitchVariantTrueM2 __GFE_Variant_SpecializationSwitchVariantTrueMN(2)
#define __GFE_Variant_SpecializationSwitchVariantTrueM3 __GFE_Variant_SpecializationSwitchVariantTrueMN(3)
#define __GFE_Variant_SpecializationSwitchVariantTrueM4 __GFE_Variant_SpecializationSwitchVariantTrueMN(4)

#define __GFE_Variant_SpecializationSwitchVariantTrueVM    \
        __GFE_Variant_SpecializationSwitchVariantTrueV     \
        __GFE_Variant_SpecializationSwitchVariantTrueM     \

#define __GFE_Variant_SpecializationSwitchVariantTrueIFV   \
        __GFE_Variant_SpecializationSwitchVariantTrueIF    \
        __GFE_Variant_SpecializationSwitchVariantTrueV     \
        
#define __GFE_Variant_SpecializationSwitchVariantTrueIFVM  \
        __GFE_Variant_SpecializationSwitchVariantTrueIFV   \
        __GFE_Variant_SpecializationSwitchVariantTrueM     \



    
#define __GFE_Variant_SpecializationReturnStorageConstantReflection(STORAGE, STORAGE1)                                                \
        else if constexpr (isVariantMember<attribStorageConstant<gfe::AttribStorage::STORAGE1>, attribStorageVariant##STORAGE>::value)     \
            return attribStorageConstant<gfe::AttribStorage::STORAGE1>{};                                                                  \


    //attribStorageVariantI
    
#define __GFE_Variant_SpecializationGetAttribStorageVariant(STORAGE)                                                           \
        attribStorageVariant##STORAGE getAttribStorageVariant##STORAGE(const gfe::AttribStorage v)                                  \
        {                                                                                                                      \
            switch (v)                                                                                                         \
            {                                                                                                                  \
                __GFE_Variant_SpecializationSwitchVariant##STORAGE                                                             \
                default:  break;                                                                                               \
            }                                                                                                                  \
            UT_ASSERT_MSG(0, "Unhandled Attrib Tuple Type");                                                                   \
        if constexpr (0)                                                                                                       \
            return attribStorageConstant<gfe::AttribStorage::Invalid>{};                                                            \
        __GFE_Variant_SpecializationReturnStorageConstantReflection(STORAGE, Int32)                                            \
        __GFE_Variant_SpecializationReturnStorageConstantReflection(STORAGE, Int64)                                            \
        __GFE_Variant_SpecializationReturnStorageConstantReflection(STORAGE, Float32)                                          \
        __GFE_Variant_SpecializationReturnStorageConstantReflection(STORAGE, Float64)                                          \
        __GFE_Variant_SpecializationReturnStorageConstantReflection(STORAGE, Vector2F)                                         \
        __GFE_Variant_SpecializationReturnStorageConstantReflection(STORAGE, Vector3F)                                         \
        __GFE_Variant_SpecializationReturnStorageConstantReflection(STORAGE, Vector4F)                                         \
        __GFE_Variant_SpecializationReturnStorageConstantReflection(STORAGE, Matrix2F)                                         \
        __GFE_Variant_SpecializationReturnStorageConstantReflection(STORAGE, Matrix3F)                                         \
        __GFE_Variant_SpecializationReturnStorageConstantReflection(STORAGE, Matrix4F)                                         \
        __GFE_Variant_SpecializationReturnStorageConstantReflection(STORAGE, Vector2I)                                         \
        __GFE_Variant_SpecializationReturnStorageConstantReflection(STORAGE, Vector3I)                                         \
        __GFE_Variant_SpecializationReturnStorageConstantReflection(STORAGE, Vector4I)                                         \
        __GFE_Variant_SpecializationReturnStorageConstantReflection(STORAGE, String)                                           \
        __GFE_Variant_SpecializationReturnStorageConstantReflection(STORAGE, Dict)                                             \
        __GFE_Variant_SpecializationReturnStorageConstantReflection(STORAGE, Int32Array)                                       \
        __GFE_Variant_SpecializationReturnStorageConstantReflection(STORAGE, Int64Array)                                       \
        __GFE_Variant_SpecializationReturnStorageConstantReflection(STORAGE, Fpreal32Array)                                    \
        __GFE_Variant_SpecializationReturnStorageConstantReflection(STORAGE, Fpreal64Array)                                    \
        __GFE_Variant_SpecializationReturnStorageConstantReflection(STORAGE, Vector2FArray)                                    \
        __GFE_Variant_SpecializationReturnStorageConstantReflection(STORAGE, Vector3FArray)                                    \
        __GFE_Variant_SpecializationReturnStorageConstantReflection(STORAGE, Vector4FArray)                                    \
        __GFE_Variant_SpecializationReturnStorageConstantReflection(STORAGE, Matrix2FArray)                                    \
        __GFE_Variant_SpecializationReturnStorageConstantReflection(STORAGE, Matrix3FArray)                                    \
        __GFE_Variant_SpecializationReturnStorageConstantReflection(STORAGE, Matrix4FArray)                                    \
        __GFE_Variant_SpecializationReturnStorageConstantReflection(STORAGE, StringArray)                                      \
        __GFE_Variant_SpecializationReturnStorageConstantReflection(STORAGE, DictArray)                                        \
        }                                                                                                                      \

    __GFE_Variant_CreateMacroStorage(__GFE_Variant_SpecializationGetAttribStorageVariant)


#define __GFE_Variant_SpecializationIsAttribStorage(STORAGE)                                                                      \
        bool isAttribStorage##STORAGE(const gfe::AttribStorage v)                                                                      \
        {                                                                                                                         \
            switch (v)                                                                                                            \
            {                                                                                                                     \
                __GFE_Variant_SpecializationSwitchVariantTrue##STORAGE                                                            \
            }                                                                                                                     \
            return false;                                                                                                         \
        }                                                                                                                         \

    __GFE_Variant_CreateMacroStorage(__GFE_Variant_SpecializationIsAttribStorage)



    
#undef __GFE_Variant_SpecializationIsAttribStorage
#undef __GFE_Variant_SpecializationGetAttribStorageVariant
#undef __GFE_Variant_SpecializationReturnStorageConstantReflection
#undef __GFE_Variant_SpecializationGetAttribStorageVariant

    
#define __GFE_Variant_SpecializationGetAttribStorageVariant(STORAGE)                       \
        SYS_FORCE_INLINE auto getAttribStorageVariant##STORAGE(const GA_Attribute& attrib) \
        { return getAttribStorageVariant##STORAGE(gfe::Type::getAttribStorage(attrib)); }   \
    
    __GFE_Variant_CreateMacroStorage(__GFE_Variant_SpecializationGetAttribStorageVariant)
    
#undef __GFE_Variant_SpecializationGetAttribStorageVariant


    
#undef __GFE_Variant_SpecializationSwitchVariantI
#undef __GFE_Variant_SpecializationSwitchVariantF
#undef __GFE_Variant_SpecializationSwitchVariantS
#undef __GFE_Variant_SpecializationSwitchVariantV
#undef __GFE_Variant_SpecializationSwitchVariantM
    
#undef __GFE_Variant_SpecializationSwitchVariantIF
#undef __GFE_Variant_SpecializationSwitchVariantIS
#undef __GFE_Variant_SpecializationSwitchVariantIFS
#undef __GFE_Variant_SpecializationSwitchVariantV2F
#undef __GFE_Variant_SpecializationSwitchVariantV3F
#undef __GFE_Variant_SpecializationSwitchVariantV4F
#undef __GFE_Variant_SpecializationSwitchVariantVF
    
#undef __GFE_Variant_SpecializationSwitchVariantV2I
#undef __GFE_Variant_SpecializationSwitchVariantV3I
#undef __GFE_Variant_SpecializationSwitchVariantV4I
#undef __GFE_Variant_SpecializationSwitchVariantVI
    
#undef __GFE_Variant_SpecializationSwitchVariantIFV
#undef __GFE_Variant_SpecializationSwitchVariantIFVM

#undef __GFE_Variant_SpecializationSwitchVariantFVF
    
#undef __GFE_Variant_SpecializationSwitchVariantV2
#undef __GFE_Variant_SpecializationSwitchVariantV3
#undef __GFE_Variant_SpecializationSwitchVariantV4
#undef __GFE_Variant_SpecializationSwitchVariantVN
    
#undef __GFE_Variant_SpecializationSwitchVariantM2
#undef __GFE_Variant_SpecializationSwitchVariantM3
#undef __GFE_Variant_SpecializationSwitchVariantM4
#undef __GFE_Variant_SpecializationSwitchVariantMN
    
#undef __GFE_Variant_SpecializationSwitchVariantVM
#undef __GFE_Variant_SpecializationSwitchVariantVFN
#undef __GFE_Variant_SpecializationSwitchVariantVIN
        



#undef __GFE_Variant_SpecializationSwitchVariantTrueI
#undef __GFE_Variant_SpecializationSwitchVariantTrueF
#undef __GFE_Variant_SpecializationSwitchVariantTrueS
#undef __GFE_Variant_SpecializationSwitchVariantTrueV
#undef __GFE_Variant_SpecializationSwitchVariantTrueM
    
#undef __GFE_Variant_SpecializationSwitchVariantTrueIF
#undef __GFE_Variant_SpecializationSwitchVariantTrueIS
#undef __GFE_Variant_SpecializationSwitchVariantTrueIFS
#undef __GFE_Variant_SpecializationSwitchVariantTrueV2F
#undef __GFE_Variant_SpecializationSwitchVariantTrueV3F
#undef __GFE_Variant_SpecializationSwitchVariantTrueV4F
#undef __GFE_Variant_SpecializationSwitchVariantTrueVF
    
#undef __GFE_Variant_SpecializationSwitchVariantTrueV2I
#undef __GFE_Variant_SpecializationSwitchVariantTrueV3I
#undef __GFE_Variant_SpecializationSwitchVariantTrueV4I
    
#undef __GFE_Variant_SpecializationSwitchVariantTrueIFV
#undef __GFE_Variant_SpecializationSwitchVariantTrueIFVM

#undef __GFE_Variant_SpecializationSwitchVariantTrueFVF
    
#undef __GFE_Variant_SpecializationSwitchVariantTrueV2
#undef __GFE_Variant_SpecializationSwitchVariantTrueV3
#undef __GFE_Variant_SpecializationSwitchVariantTrueV4
#undef __GFE_Variant_SpecializationSwitchVariantTrueVN
#undef __GFE_Variant_SpecializationSwitchVariantTrueV
    
#undef __GFE_Variant_SpecializationSwitchVariantTrueM2
#undef __GFE_Variant_SpecializationSwitchVariantTrueM3
#undef __GFE_Variant_SpecializationSwitchVariantTrueM4
#undef __GFE_Variant_SpecializationSwitchVariantTrueMN
    
#undef __GFE_Variant_SpecializationSwitchVariantTrueVM
#undef __GFE_Variant_SpecializationSwitchVariantTrueVFN
#undef __GFE_Variant_SpecializationSwitchVariantTrueVIN

/*
         
template <GA_Storage _Val>
using attrib_store_constant = std::integral_constant<GA_Storage, _Val>;

using attrib_store_invalid_type = attrib_store_constant<GA_STORE_INVALID>;
using attrib_store_b_type       = attrib_store_constant<GA_STORE_BOOL>;
using attrib_store_i8_type      = attrib_store_constant<GA_STORE_INT8>;
using attrib_store_i16_type     = attrib_store_constant<GA_STORE_INT16>;
using attrib_store_i32_type     = attrib_store_constant<GA_STORE_INT32>;
using attrib_store_i64_type     = attrib_store_constant<GA_STORE_INT64>;
using attrib_store_f16_type     = attrib_store_constant<GA_STORE_REAL16>;
using attrib_store_f32_type     = attrib_store_constant<GA_STORE_REAL32>;
using attrib_store_f64_type     = attrib_store_constant<GA_STORE_REAL64>;
using attrib_store_fs_type      = attrib_store_constant<GA_STORE_STRING>;
using attrib_store_fd_type      = attrib_store_constant<GA_STORE_DICT>;



using attrib_store_variant = std::variant<attrib_store_invalid_type,
                                          attrib_store_b_type      ,
                                          attrib_store_i8_type     ,
                                          attrib_store_i16_type    ,
                                          attrib_store_i32_type    ,
                                          attrib_store_i64_type    ,
                                          attrib_store_f16_type    ,
                                          attrib_store_f32_type    ,
                                          attrib_store_f64_type    ,
                                          attrib_store_fs_type     ,
                                          attrib_store_fd_type     >;
        
SYS_FORCE_INLINE attrib_store_variant getAttribStorageVariant(const GA_Storage v)
{
    switch (v)
    {
    case GA_STORE_INVALID: return attrib_store_constant<GA_STORE_INVALID>{}; break;
    case GA_STORE_INT8:    return attrib_store_constant<GA_STORE_INT8>   {}; break;
    case GA_STORE_INT16:   return attrib_store_constant<GA_STORE_INT16>  {}; break;
    case GA_STORE_INT32:   return attrib_store_constant<GA_STORE_INT32>  {}; break;
    case GA_STORE_INT64:   return attrib_store_constant<GA_STORE_INT64>  {}; break;
    case GA_STORE_REAL16:  return attrib_store_constant<GA_STORE_REAL16> {}; break;
    case GA_STORE_REAL32:  return attrib_store_constant<GA_STORE_REAL32> {}; break;
    case GA_STORE_REAL64:  return attrib_store_constant<GA_STORE_REAL64> {}; break;
    case GA_STORE_STRING:  return attrib_store_constant<GA_STORE_STRING> {}; break;
    case GA_STORE_DICT:    return attrib_store_constant<GA_STORE_DICT>   {}; break;
    default: break;
    }
    UT_ASSERT_MSG(0, "Unhandled Attrib Storage");
    return attrib_store_constant<GA_STORE_INVALID>{};
}

        
SYS_FORCE_INLINE auto getAttribStorageVariant(const GA_Attribute& attrib)
{ return getAttribStorageVariant(attrib.getAIFTuple()->getStorage(&attrib)); }

SYS_FORCE_INLINE auto getAttribStorageVariant(const GA_Attribute* const attrib)
{ UT_ASSERT_P(attrib); return getAttribStorageVariant(attrib->getAIFTuple()->getStorage(attrib)); }

*/

    #undef __GFE_Variant_CreateMacroStorage



    

        
        
template <GA_AttributeOwner _Val>
using attribOwnerConstant = std::integral_constant<GA_AttributeOwner, _Val>;

using attribOwnerConstantInvalid = attribOwnerConstant<GA_ATTRIB_INVALID  >;
using attribOwnerConstantPrim    = attribOwnerConstant<GA_ATTRIB_PRIMITIVE>;
using attribOwnerConstantPoint   = attribOwnerConstant<GA_ATTRIB_POINT    >;
using attribOwnerConstantVertex  = attribOwnerConstant<GA_ATTRIB_VERTEX   >;
using attribOwnerConstantDetail  = attribOwnerConstant<GA_ATTRIB_DETAIL   >;
using attribOwnerConstantN       = attribOwnerConstant<GA_ATTRIB_OWNER_N  >;
    
using attribOwnerVariantPPVD = std::variant<attribOwnerConstantPrim   ,
                                            attribOwnerConstantPoint  ,
                                            attribOwnerConstantVertex ,
                                            attribOwnerConstantDetail >;

        
SYS_FORCE_INLINE attribOwnerVariantPPVD getAttribOwnerVariant(const GA_AttributeOwner v)
{
    switch (v)
    {
    case GA_ATTRIB_PRIMITIVE: return attribOwnerConstant<GA_ATTRIB_PRIMITIVE>{}; break;
    case GA_ATTRIB_POINT:     return attribOwnerConstant<GA_ATTRIB_POINT    >{}; break;
    case GA_ATTRIB_VERTEX:    return attribOwnerConstant<GA_ATTRIB_VERTEX   >{}; break;
    case GA_ATTRIB_DETAIL:    return attribOwnerConstant<GA_ATTRIB_DETAIL   >{}; break;
    }
    UT_ASSERT_MSG(0, "Unhandled Attrib Owner");
    return attribOwnerConstant<GA_ATTRIB_POINT>{};
}

SYS_FORCE_INLINE auto getAttribOwnerVariant(const GA_Attribute& attrib)
{ return getAttribOwnerVariant(attrib.getOwner()); }

SYS_FORCE_INLINE auto getAttribOwnerVariant(const GA_Attribute* const attrib)
{ UT_ASSERT_P(attrib); return getAttribOwnerVariant(*attrib); }




    
        
using boolVariant = std::variant<std::true_type, std::false_type>;

boolVariant getBoolVariant(const bool v)
{
    if (v)
        return std::true_type{};
    else
        return std::false_type{};
}







    
    template <gfe::AttribStorage _Ty>
    struct getAttribStorage { using type = void; };

    template <> struct getAttribStorage<gfe::AttribStorage::Int8>     { using type = int8; };
    template <> struct getAttribStorage<gfe::AttribStorage::Int16>    { using type = int16; };
    template <> struct getAttribStorage<gfe::AttribStorage::Int32>    { using type = int32; };
    template <> struct getAttribStorage<gfe::AttribStorage::Int64>    { using type = int64; };
    template <> struct getAttribStorage<gfe::AttribStorage::Float16> { using type = fpreal16; };
    template <> struct getAttribStorage<gfe::AttribStorage::Float32> { using type = fpreal32; };
    template <> struct getAttribStorage<gfe::AttribStorage::Float64> { using type = fpreal64; };
        
    template <> struct getAttribStorage<gfe::AttribStorage::String>   { using type = UT_StringHolder; };
    template <> struct getAttribStorage<gfe::AttribStorage::Dict>     { using type = UT_OptionsHolder; };


        
    template <> struct getAttribStorage<gfe::AttribStorage::Int32Array>    { using type = UT_ValArray<int32>; };
    template <> struct getAttribStorage<gfe::AttribStorage::Int64Array>    { using type = UT_ValArray<int64>; };
    template <> struct getAttribStorage<gfe::AttribStorage::Float32Array>  { using type = UT_ValArray<fpreal32>; };
    template <> struct getAttribStorage<gfe::AttribStorage::Float64Array>  { using type = UT_ValArray<fpreal64>; };


        
    template <> struct getAttribStorage<gfe::AttribStorage::StringArray>   { using type = UT_StringArray; };
    template <> struct getAttribStorage<gfe::AttribStorage::DictArray>     { using type = UT_Array<UT_OptionsHolder>; };
        
#define __GFE_Variant_SpecializationGetAttribStorageV(NUM)\
    template <> struct getAttribStorage<gfe::AttribStorage::Vector##NUM##I> { using type = UT_Vector##NUM##i; };\
    template <> struct getAttribStorage<gfe::AttribStorage::Vector##NUM##L> { using type = UT_Vector##NUM##I; };\
    template <> struct getAttribStorage<gfe::AttribStorage::Vector##NUM##H> { using type = UT_Vector##NUM##H; };\
    template <> struct getAttribStorage<gfe::AttribStorage::Vector##NUM##F> { using type = UT_Vector##NUM##F; };\
    template <> struct getAttribStorage<gfe::AttribStorage::Vector##NUM##D> { using type = UT_Vector##NUM##D; };\
    
        __GFE_Variant_SpecializationGetAttribStorageV(2)
        __GFE_Variant_SpecializationGetAttribStorageV(3)
        __GFE_Variant_SpecializationGetAttribStorageV(4)
        
#undef __GFE_Variant_SpecializationGetAttribStorageV
        
#define __GFE_Variant_SpecializationGetAttribStorageM(NUM)\
    template <> struct getAttribStorage<gfe::AttribStorage::Matrix##NUM##F> { using type = UT_Matrix##NUM##F; };\
    template <> struct getAttribStorage<gfe::AttribStorage::Matrix##NUM##D> { using type = UT_Matrix##NUM##D; };\
    
        __GFE_Variant_SpecializationGetAttribStorageM(2)
        __GFE_Variant_SpecializationGetAttribStorageM(3)
        __GFE_Variant_SpecializationGetAttribStorageM(4)
        
#undef __GFE_Variant_SpecializationGetAttribStorageM

        
#define __GFE_Variant_SpecializationGetAttribStorageVArray(NUM)\
    template <> struct getAttribStorage<gfe::AttribStorage::Vector##NUM##IArray> { using type = UT_ValArray<UT_Vector##NUM##i>; };\
    template <> struct getAttribStorage<gfe::AttribStorage::Vector##NUM##LArray> { using type = UT_ValArray<UT_Vector##NUM##I>; };\
    template <> struct getAttribStorage<gfe::AttribStorage::Vector##NUM##HArray> { using type = UT_ValArray<UT_Vector##NUM##H>; };\
    template <> struct getAttribStorage<gfe::AttribStorage::Vector##NUM##FArray> { using type = UT_ValArray<UT_Vector##NUM##F>; };\
    template <> struct getAttribStorage<gfe::AttribStorage::Vector##NUM##DArray> { using type = UT_ValArray<UT_Vector##NUM##D>; };\
    
        __GFE_Variant_SpecializationGetAttribStorageVArray(2)
        __GFE_Variant_SpecializationGetAttribStorageVArray(3)
        __GFE_Variant_SpecializationGetAttribStorageVArray(4)
        
#undef __GFE_Variant_SpecializationGetAttribStorageVArray

        
#define __GFE_Variant_SpecializationGetAttribStorageMArray(NUM)\
    template <> struct getAttribStorage<gfe::AttribStorage::Matrix##NUM##FArray> { using type = UT_ValArray<UT_Matrix##NUM##F>; };\
    template <> struct getAttribStorage<gfe::AttribStorage::Matrix##NUM##DArray> { using type = UT_ValArray<UT_Matrix##NUM##D>; };\
    
        __GFE_Variant_SpecializationGetAttribStorageMArray(2)
        __GFE_Variant_SpecializationGetAttribStorageMArray(3)
        __GFE_Variant_SpecializationGetAttribStorageMArray(4)
        
#undef __GFE_Variant_SpecializationGetAttribStorageMArray



        
template <gfe::AttribStorage _Ty>
using getAttribStorage_t = typename getAttribStorage<_Ty>::type;



/*
    
    template <GA_AttributeOwner _Ty>
    struct get_owner { using type = void; };

    template <>
    struct get_owner<GA_ATTRIB_PRIMITIVE>     { using type = GA_ATTRIB_PRIMITIVE; };
    template <>
    struct get_owner<GA_ATTRIB_POINT>    { using type = GA_ATTRIB_POINT; };
    template <>
    struct get_owner<GA_ATTRIB_VERTEX>    { using type = GA_ATTRIB_VERTEX; };
    template <>
    struct get_owner<GA_ATTRIB_DETAIL>    { using type = GA_ATTRIB_DETAIL; };

    
template <GA_AttributeOwner _Ty>
using get_owner_t = typename get_owner<_Ty>::type;
*/


} // End of namespace Variant
_GFE_END
#endif
