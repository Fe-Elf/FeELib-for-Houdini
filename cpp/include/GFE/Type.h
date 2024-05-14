
#pragma once

#ifndef __GFE_Type_h__
#define __GFE_Type_h__

#include <GFE/Type.h>

#include <GA/GA_Types.h>
#include <GA/GA_PageHandle.h>

#include <GFE/EnumClass.h>
#include <GFE/TypeTraits.h>

#if 1
    #include <GU/GU_Snap.h>
#else
    namespace GU_Snap {
        enum AttributeMergeMethod;
    }
#endif


_GFE_BEGIN
inline namespace Type {
        

#if 0
    static constexpr GA_AttributeScope TOPO_SCOPE = GA_AttributeScope::GA_SCOPE_PRIVATE;
#else
    static constexpr GA_AttributeScope TOPO_SCOPE = GA_AttributeScope::GA_SCOPE_PUBLIC;
#endif

    template<typename _Ty>
    using RWPageHandleT = GA_PageHandleT<typename gfe::TypeTraits::get_page_handle_type_t<_Ty>,
                                         typename gfe::TypeTraits::get_page_handle_value_type_t<_Ty>,
                                         true, true, GA_Attribute, GA_ATINumeric, GA_Detail>;

    template<typename _Ty>
    using ROPageHandleT = GA_PageHandleT<typename gfe::TypeTraits::get_page_handle_type_t<_Ty>,
                                         typename gfe::TypeTraits::get_page_handle_value_type_t<_Ty>,
                                         true, false, const GA_Attribute, const GA_ATINumeric, const GA_Detail>;

        
    //SYS_FORCE_INLINE static GA_PageNum getNumPage(const GA_Offset v)
    //{ return GA_PageNum(v >> GA_PAGE_BITS); }

    //SYS_FORCE_INLINE static GA_PrimitiveTypeId typeId(const GEO_PrimitiveType type)
    //{ return GA_PrimitiveTypeId(static_cast<int>(type)); }
    
    static GroupMergeMethod attribGroupMergeMethod(const AttribMergeMethod mergeMethod)
    {
        switch (mergeMethod)
        {
        case AttribMergeMethod::First:          return GroupMergeMethod::First;     break;
        case AttribMergeMethod::Last:           return GroupMergeMethod::Last;      break;
        case AttribMergeMethod::Min:            return GroupMergeMethod::Min;       break;
        case AttribMergeMethod::Max:            return GroupMergeMethod::Max;       break;
        case AttribMergeMethod::Mode:           return GroupMergeMethod::Mode;      break;
        default:                                return GroupMergeMethod::Max;       break;
        }
        UT_ASSERT_MSG(0, "Unhandled Attrib Merge Method!");
        return GroupMergeMethod::First;
    }

    static AttribMergeMethod attribGroupMergeMethod(const GroupMergeMethod mergeMethod)
    {
        switch (mergeMethod)
        {
        case GroupMergeMethod::First:          return AttribMergeMethod::First;     break;
        case GroupMergeMethod::Last:           return AttribMergeMethod::Last;      break;
        case GroupMergeMethod::Min:            return AttribMergeMethod::Min;       break;
        case GroupMergeMethod::Max:            return AttribMergeMethod::Max;       break;
        case GroupMergeMethod::Mode:           return AttribMergeMethod::Mode;      break;
        }
        UT_ASSERT_MSG(0, "Unhandled Group Merge Method!");
        return AttribMergeMethod::First;
    }

    static GU_Snap::AttributeMergeMethod snapMergeMethod(const AttribMergeMethod mergeMethod)
    {
        switch (mergeMethod)
        {
        case AttribMergeMethod::First:          return GU_Snap::AttributeMergeMethod::MERGE_ATTRIBUTE_FIRST;     break;
        case AttribMergeMethod::Last:           return GU_Snap::AttributeMergeMethod::MERGE_ATTRIBUTE_LAST;      break;
        case AttribMergeMethod::Min:            return GU_Snap::AttributeMergeMethod::MERGE_ATTRIBUTE_MIN;       break;
        case AttribMergeMethod::Max:            return GU_Snap::AttributeMergeMethod::MERGE_ATTRIBUTE_MAX;       break;
        case AttribMergeMethod::Mode:           return GU_Snap::AttributeMergeMethod::MERGE_ATTRIBUTE_MODE;      break;
        case AttribMergeMethod::Mean:           return GU_Snap::AttributeMergeMethod::MERGE_ATTRIBUTE_MEAN;      break;
        case AttribMergeMethod::Median:         return GU_Snap::AttributeMergeMethod::MERGE_ATTRIBUTE_MEDIAN;    break;
        case AttribMergeMethod::Sum:            return GU_Snap::AttributeMergeMethod::MERGE_ATTRIBUTE_SUM;       break;
        case AttribMergeMethod::SumSquare:      return GU_Snap::AttributeMergeMethod::MERGE_ATTRIBUTE_SUMSQUARE; break;
        case AttribMergeMethod::RootMeanSquare: return GU_Snap::AttributeMergeMethod::MERGE_ATTRIBUTE_RMS;       break;
        }
        UT_ASSERT_MSG(0, "Unhandled Attrib Merge Method!");
        return GU_Snap::AttributeMergeMethod::MERGE_ATTRIBUTE_FIRST;
    }

    static GU_Snap::AttributeMergeMethod snapMergeMethod(const GroupMergeMethod mergeMethod)
    {
        switch (mergeMethod)
        {
        case GroupMergeMethod::First: return GU_Snap::AttributeMergeMethod::MERGE_ATTRIBUTE_FIRST;     break;
        case GroupMergeMethod::Last:  return GU_Snap::AttributeMergeMethod::MERGE_ATTRIBUTE_LAST;      break;
        case GroupMergeMethod::Min:   return GU_Snap::AttributeMergeMethod::MERGE_ATTRIBUTE_MIN;       break;
        case GroupMergeMethod::Max:   return GU_Snap::AttributeMergeMethod::MERGE_ATTRIBUTE_MAX;       break;
        case GroupMergeMethod::Mode:  return GU_Snap::AttributeMergeMethod::MERGE_ATTRIBUTE_MODE;      break;
        }
        UT_ASSERT_MSG(0, "Unhandled Group Merge Method!");
        return GU_Snap::AttributeMergeMethod::MERGE_ATTRIBUTE_FIRST;
    }

        
        

    SYS_FORCE_INLINE static bool isPoly(const int id)
    { return id == GA_PRIMPOLY; } // 1
        
    SYS_FORCE_INLINE static bool isHoudiniVolume(const int id)
    { return id == GA_PRIMVOLUME; } // 20

    SYS_FORCE_INLINE static bool isVDB(const int id)
    { return id == GA_PRIMVDB; } // 23

    SYS_FORCE_INLINE static bool isVolume(const int id)
    { return isHoudiniVolume(id) || isVDB(id); }


    SYS_FORCE_INLINE static bool isPackedGeometry(const int id)
    { return id == 25; } // 25

    SYS_FORCE_INLINE static bool isPackedFragment(const int id)
    { return id == 27; } // 27

    SYS_FORCE_INLINE static bool isPacked(const int id)
    { return isPackedGeometry(id) || isPackedFragment(id); }


    SYS_FORCE_INLINE static bool stringEqual(const char* const str0, const char* const str1)
    #ifdef GFE_DEBUG_MODE
        {
            //const char* const str01 = str0;
            //const char* const str02 = str1;
            //const bool equal0 = str0 == str1;
            //const bool equal1 = strcmp(str0, str1) == 0;
            return str0 == str1 || strcmp(str0, str1) == 0;
        }
    #else
        { return str0 == str1 || strcmp(str0, str1) == 0; }
    #endif
        
    SYS_FORCE_INLINE static bool stringEqual(const UT_StringRef& str0, const UT_StringRef& str1)
    { return stringEqual(str0.c_str(), str1.c_str()); }

    SYS_FORCE_INLINE static bool stringEqual(const UT_StringRef& str0, const char* const str1)
    { return stringEqual(str0.c_str(), str1); }

    SYS_FORCE_INLINE static bool stringEqual(const char* const str0, const UT_StringRef& str1)
    { return stringEqual(str0, str1.c_str()); }

    SYS_FORCE_INLINE static bool stringEqualP(const UT_StringRef& str0)
    { return stringEqual(str0, "P"); }
    
    SYS_FORCE_INLINE static bool stringEqualP(const char* const str0)
    { return stringEqual(str0, "P"); }
    
        
    SYS_FORCE_INLINE static bool isValidOffset(const GA_Offset elemoff)
    {
        if constexpr (INVALID_OFFSET >= 0)
            return elemoff < INVALID_OFFSET && elemoff >= 0;
        else
            return elemoff < 0;
    }

    SYS_FORCE_INLINE static bool isInvalidOffset(const GA_Offset elemoff)
    { return !isValidOffset(elemoff); }


    SYS_FORCE_INLINE static bool isValidOffset(const GA_IndexMap& indexMap, const GA_Offset elemoff)
    #ifdef GFE_DEBUG_MODE
    {
        const GA_Index index = indexMap.indexFromOffset(elemoff);
        const bool isOffsetActiveFast = indexMap.isOffsetActive(elemoff);
        return indexMap.isOffsetActive(elemoff);
    }
    #else
    { return indexMap.isOffsetActive(elemoff); }
    #endif

    SYS_FORCE_INLINE static bool isInvalidOffset(const GA_IndexMap& indexMap, const GA_Offset elemoff)
    { return isInvalidOffset(elemoff) || !indexMap.isOffsetActiveFast(elemoff); }


        
    SYS_FORCE_INLINE static bool isValid(const UT_StringRef& str)
    { return str.isstring() && str.length() > 0; }

    SYS_FORCE_INLINE static bool isInvalid(const UT_StringRef& str)
    { return !str.isstring() || str.length() <= 0; }

    SYS_FORCE_INLINE static bool isPublicAttribName(const UT_StringRef& attribName)
    { return isValid(attribName) && !stringEqual(attribName, "P"); }

    SYS_FORCE_INLINE static bool isPublicAttribName(const char* const attribName)
    { return attribName && !stringEqual(attribName, "P"); }


        
    static GA_GroupType attributeOwner_groupType(const GA_AttributeOwner attribOwner)
    {
        switch (attribOwner)
        {
        case GA_ATTRIB_PRIMITIVE:       return GA_GROUP_PRIMITIVE;    break;
        case GA_ATTRIB_POINT:           return GA_GROUP_POINT;        break;
        case GA_ATTRIB_VERTEX:          return GA_GROUP_VERTEX;       break;
        case GA_ATTRIB_DETAIL:          return GA_GROUP_EDGE;         break;
        case GA_ATTRIB_OWNER_N:         return GA_GROUP_N;            break;
        case GA_ATTRIB_INVALID:         return GA_GROUP_INVALID;      break;
        }
        UT_ASSERT_MSG(0, "Unhandled Attrib Owner!");
        return GA_GROUP_INVALID;
    }

    static GA_AttributeOwner attributeOwner_groupType(const GA_GroupType groupType)
    {
        switch (groupType)
        {
        case GA_GROUP_PRIMITIVE:       return GA_ATTRIB_PRIMITIVE;   break;
        case GA_GROUP_POINT:           return GA_ATTRIB_POINT;       break;
        case GA_GROUP_VERTEX:          return GA_ATTRIB_VERTEX;      break;
        case GA_GROUP_EDGE:            return GA_ATTRIB_DETAIL;      break;
        case GA_GROUP_BREAKPOINT:      return GA_ATTRIB_INVALID;     break;
        case GA_GROUP_N:               return GA_ATTRIB_OWNER_N;     break;
        case GA_GROUP_INVALID:         return GA_ATTRIB_INVALID;     break;
        }
        UT_ASSERT_MSG(0, "Unhandled Group Type!");
        return GA_ATTRIB_INVALID;
    }
        
    static GA_Precision precisionFromStorage(const GA_Storage storage)
    {
        switch (storage)
        {
        case GA_STORE_INVALID:       return GA_PRECISION_INVALID;   break;
        case GA_STORE_BOOL:          return GA_PRECISION_1;         break;
        case GA_STORE_UINT8:         return GA_PRECISION_8;         break;
        case GA_STORE_INT8:          return GA_PRECISION_8;         break;
        case GA_STORE_INT16:         return GA_PRECISION_16;        break;
        case GA_STORE_INT32:         return GA_PRECISION_32;        break;
        case GA_STORE_INT64:         return GA_PRECISION_64;        break;
        case GA_STORE_REAL16:        return GA_PRECISION_16;        break;
        case GA_STORE_REAL32:        return GA_PRECISION_32;        break;
        case GA_STORE_REAL64:        return GA_PRECISION_64;        break;
        case GA_STORE_STRING:        return GA_PRECISION_INVALID;   break;
        case GA_STORE_DICT:          return GA_PRECISION_INVALID;   break;
        // case GA_STORE_UTF8:          return GA_PRECISION_8;       break;
        // case GA_STORE_UTF16:         return GA_PRECISION_16;      break;
        // case GA_STORE_UTF32:         return GA_PRECISION_32;      break;
        }
        UT_ASSERT_MSG(0, "Unhandled Storage!");
        return GA_PRECISION_INVALID;
    }

    SYS_FORCE_INLINE static bool isValid(const GA_AttributeOwner attribOwner)
    {
        return attribOwner == GA_ATTRIB_PRIMITIVE ||
               attribOwner == GA_ATTRIB_POINT     ||
               attribOwner == GA_ATTRIB_VERTEX    ||
               attribOwner == GA_ATTRIB_GLOBAL    ;
    }

    SYS_FORCE_INLINE static bool isValid(const GA_GroupType groupType)
    {
        return groupType == GA_GROUP_PRIMITIVE ||
               groupType == GA_GROUP_POINT     ||
               groupType == GA_GROUP_VERTEX    ||
               groupType == GA_GROUP_EDGE;
    }

    SYS_FORCE_INLINE static bool isInvalid(const GA_AttributeOwner attribOwner)
    {
        return attribOwner != GA_ATTRIB_PRIMITIVE &&
               attribOwner != GA_ATTRIB_POINT     &&
               attribOwner != GA_ATTRIB_VERTEX    &&
               attribOwner != GA_ATTRIB_GLOBAL;
    }

    SYS_FORCE_INLINE static bool isInvalid(const GA_GroupType groupType)
    {
        return groupType != GA_GROUP_PRIMITIVE &&
               groupType != GA_GROUP_POINT     &&
               groupType != GA_GROUP_VERTEX    &&
               groupType != GA_GROUP_EDGE;
    }


    SYS_FORCE_INLINE static bool isElementGroup(const GA_AttributeOwner attribOwner)
    { return attribOwner == GA_ATTRIB_PRIMITIVE || attribOwner == GA_ATTRIB_POINT || attribOwner == GA_ATTRIB_VERTEX; }
        
    SYS_FORCE_INLINE static bool isElementGroup(const GA_GroupType groupType)
    { return groupType == GA_GROUP_PRIMITIVE || groupType == GA_GROUP_POINT || groupType == GA_GROUP_VERTEX; }






    static GA_Storage getPreferredStorageF(const GA_Precision precision)
    {
        switch (precision)
        {
        case GA_PRECISION_16: return GA_STORE_REAL16; break;
        case GA_PRECISION_32: return GA_STORE_REAL32; break;
        case GA_PRECISION_64: return GA_STORE_REAL64; break;
        default:              break;
        }
        //UT_ASSERT_MSG(0, "Unhandled Precision!");
        return GA_STORE_INVALID;
    }
        

    static GA_Storage getPreferredStorageI(const GA_Precision precision)
    {
        switch (precision)
        {
        case GA_PRECISION_8:    return GA_STORE_INT8;    break;
        case GA_PRECISION_16:   return GA_STORE_INT16;   break;
        case GA_PRECISION_32:   return GA_STORE_INT32;   break;
        case GA_PRECISION_64:   return GA_STORE_INT64;   break;
        default: break;
        }
        //UT_ASSERT_MSG(0, "Unhandled Precision!");
        return GA_STORE_INVALID;
    }
        
    SYS_FORCE_INLINE static GA_Storage getPreferredStorage(const GA_StorageClass storageClass, const GA_Precision precision)
    {
        switch (storageClass)
        {
        case GA_STORECLASS_REAL:    return getPreferredStorageF(precision);  break;
        case GA_STORECLASS_INT:     return getPreferredStorageI(precision);  break;
        case GA_STORECLASS_STRING:  return GA_STORE_STRING;                  break;
        case GA_STORECLASS_DICT:    return GA_STORE_DICT;                    break;
        default:                    return getPreferredStorageF(precision);  break;
        }
    }
        
    SYS_FORCE_INLINE static GA_Storage getPreferredStorageI(const GA_Detail& geo)
    { return getPreferredStorageI(geo.getPreferredPrecision()); }


    SYS_FORCE_INLINE static GA_Storage getPreferredStorageF(const GA_Detail& geo)
    { return getPreferredStorageF(geo.getPreferredPrecision()); }



    SYS_FORCE_INLINE static GA_Storage getPreferredStorage(const GA_Detail& geo, const GA_StorageClass storageClass)
    { return getPreferredStorage(storageClass, geo.getPreferredPrecision()); }


    SYS_FORCE_INLINE static GA_Storage getPreferredStorageF(const GA_Detail& geo, const GA_Storage storage)
    { return storage == GA_STORE_INVALID ? getPreferredStorageF(geo) : storage; }

    SYS_FORCE_INLINE static GA_Storage getPreferredStorageI(const GA_Detail& geo, const GA_Storage storage)
    { return storage == GA_STORE_INVALID ? getPreferredStorageI(geo) : storage; }

    SYS_FORCE_INLINE static GA_Storage getPreferredStorage(const GA_Detail& geo, const GA_StorageClass storageClass, const GA_Storage storage)
    { return storage == GA_STORE_INVALID ? getPreferredStorage(geo, storageClass) : storage; }

    SYS_FORCE_INLINE static GA_Storage getPreferredStorage(const GA_Detail* const geo, const GA_StorageClass storageClass, const GA_Storage storage)
    { return getPreferredStorage(*geo, storageClass, storage); }




        

    template<typename VECTOR_T>
    static VECTOR_T axisDir(const Axis axis)
    {
        switch (axis)
        {
        case Axis::X: return VECTOR_T(1,0,0); break;
        case Axis::Y: return VECTOR_T(0,1,0); break;
        case Axis::Z: return VECTOR_T(0,0,1); break;
        }
        UT_ASSERT_MSG(0, "Unhandled Axis");
        return VECTOR_T(0,1,0);
    }

    SYS_FORCE_INLINE static UT_Vector3T<fpreal16> axisDirH(const Axis axis)
    { return axisDir<UT_Vector3T<fpreal16>>(axis); }

    SYS_FORCE_INLINE static UT_Vector3T<fpreal32> axisDirF(const Axis axis)
    { return axisDir<UT_Vector3T<fpreal32>>(axis); }
        
    SYS_FORCE_INLINE static UT_Vector3T<fpreal64> axisDirD(const Axis axis)
    { return axisDir<UT_Vector3T<fpreal64>>(axis); }




        
static bool isAttribTypeEqual(const GA_Attribute* const attrib0, const GA_Attribute* const attrib1)
{
    UT_ASSERT_P(attrib0);
    UT_ASSERT_P(attrib1);
    return &attrib0->getDetail()       == &attrib1->getDetail()       &&
            attrib0->getTupleSize()    ==  attrib1->getTupleSize()    &&
            attrib0->getStorageClass() ==  attrib1->getStorageClass() &&
            (
                !attrib0->getAIFTuple() ||
                 attrib0->getAIFTuple() &&
                 attrib1->getAIFTuple() &&
                 attrib0->getAIFTuple()->getStorage(attrib0) == attrib1->getAIFTuple()->getStorage(attrib1)
            );
}

    static bool checkTupleAttrib(
        const GA_Attribute* const attrib,
        const GA_StorageClass storageClass = GA_STORECLASS_INVALID,
        const GA_Precision precision = GA_PRECISION_INVALID,
        const int tupleSize = 1,
        const GA_Defaults* const defaults = nullptr
    )
    {
        if (!attrib)
            return false;
        if (attrib->getTupleSize() != tupleSize)
            return false;
        if (storageClass != GA_STORECLASS_INVALID && attrib->getStorageClass() != storageClass)
            return false;
        
        const GA_AIFTuple* const aifTuple = attrib->getAIFTuple();
        if (aifTuple)
        {
            if ((precision != GA_PRECISION_INVALID && precisionFromStorage(aifTuple->getStorage(attrib)) != precision) ||
                (defaults && aifTuple->getDefaults(attrib) != *defaults))
                    return false;
        }
        else
        {
            if (storageClass != GA_STORECLASS_INVALID && storageClass != GA_STORECLASS_STRING)
                return false;
            //const GA_AIFStringTuple* const aifStrTuple = attribPtr->getAIFStringTuple();
        }
        return true;
    }
    
static bool checkTupleAttrib(
    const GA_Attribute* const attrib,
    const GA_StorageClass storageClass = GA_STORECLASS_INVALID,
    const GA_Storage storage = GA_STORE_INVALID,
    const int tupleSize = 1,
    const GA_Defaults* const defaults = nullptr
)
{
    if (!attrib)
        return false;
    if (attrib->getTupleSize() != tupleSize)
        return false;
    if (storageClass != GA_STORECLASS_INVALID && attrib->getStorageClass() != storageClass)
        return false;
        
    const GA_AIFTuple* const aifTuple = attrib->getAIFTuple();
    if (aifTuple)
    {
        if ((storage != GA_STORE_INVALID && aifTuple->getStorage(attrib) != storage) ||
            (defaults && aifTuple->getDefaults(attrib) != *defaults))
            return false;
    }
    else
    {
        if (storageClass != GA_STORECLASS_INVALID && storageClass != GA_STORECLASS_STRING)
            return false;
        if (storage != GA_STORE_INVALID && storage != GA_STORE_STRING)
            return false;
        //const GA_AIFStringTuple* const aifStrTuple = attribPtr->getAIFStringTuple();
    }
    return true;
}

SYS_FORCE_INLINE static bool checkDirAttrib(const GA_Attribute* const attrib, const GA_StorageClass storageClass = GA_STORECLASS_INVALID, const GA_Storage storage = GA_STORE_INVALID)
{ return checkTupleAttrib(attrib, storageClass, storage, 3); }

static bool checkArrayAttrib(
    const GA_Attribute* const attrib,
    const GA_StorageClass storageClass = GA_STORECLASS_INVALID,
    const GA_Storage storage = GA_STORE_INVALID,
    const int tupleSize = 3
)
{
    if (!attrib)
        return false;
    if (attrib->getTupleSize() != tupleSize)
        return false;
    if (storageClass != GA_STORECLASS_INVALID && attrib->getStorageClass() != storageClass)
        return false;
    const GA_AIFNumericArray* const aifNumericArray = attrib->getAIFNumericArray();
    if (aifNumericArray)
    {
        if (storage != GA_STORE_INVALID && aifNumericArray->getStorage(attrib) != storage)
            return false;
    }
    else
    {
        if (storageClass != GA_STORECLASS_INVALID && storageClass != GA_STORECLASS_STRING)
            return false;
        if (storage != GA_STORE_INVALID && storage != GA_STORE_STRING)
            return false;
        //const GA_AIFSharedStringArray* const aifStrArray = attribPtr->getAIFSharedStringArray();
    }
    return true;
}


SYS_FORCE_INLINE static bool isValidPosAttrib(const GA_Attribute& posAttrib)
{ return posAttrib.getOwner() == GA_ATTRIB_POINT && posAttrib.getAIFTuple() && posAttrib.getTupleSize()==3; }

SYS_FORCE_INLINE static bool isValidPosAttrib(const GA_Attribute* const posAttrib)
{ return posAttrib && isValidPosAttrib(*posAttrib); }

SYS_FORCE_INLINE static bool isInvalidPosAttrib(const GA_Attribute& posAttrib)
{ return !isValidPosAttrib(posAttrib); }
    
SYS_FORCE_INLINE static bool isInvalidPosAttrib(const GA_Attribute* const posAttrib)
{ return !isValidPosAttrib(posAttrib); }


    AttribStorage getAttribStorageTuple(const int tupleSize, const GA_Storage store)
    {
        switch (tupleSize)
        {
        case 1:
            switch (store)
            {
            case GA_STORE_INT8:   return AttribStorage::Int8;     break;
            case GA_STORE_INT16:  return AttribStorage::Int16;    break;
            case GA_STORE_INT32:  return AttribStorage::Int32;    break;
            case GA_STORE_INT64:  return AttribStorage::Int64;    break;
            case GA_STORE_REAL16: return AttribStorage::Fpreal16; break;
            case GA_STORE_REAL32: return AttribStorage::Fpreal32; break;
            case GA_STORE_REAL64: return AttribStorage::Fpreal64; break;
            default: UT_ASSERT_MSG(0, "Unhandled Attrib Storage"); break;
            }
        break;
        case 2:
            switch (store)
            {
            case GA_STORE_INT32:  return AttribStorage::Vector2I; break;
            case GA_STORE_INT64:  return AttribStorage::Vector2L; break;
            case GA_STORE_REAL16: return AttribStorage::Vector2H; break;
            case GA_STORE_REAL32: return AttribStorage::Vector2F; break;
            case GA_STORE_REAL64: return AttribStorage::Vector2D; break;
            default: UT_ASSERT_MSG(0, "Unhandled Attrib Storage"); break;
            }
        break;
        case 3:
            switch (store)
            {
            case GA_STORE_INT32:  return AttribStorage::Vector3I; break;
            case GA_STORE_INT64:  return AttribStorage::Vector3L; break;
            case GA_STORE_REAL16: return AttribStorage::Vector3H; break;
            case GA_STORE_REAL32: return AttribStorage::Vector3F; break;
            case GA_STORE_REAL64: return AttribStorage::Vector3D; break;
            default: UT_ASSERT_MSG(0, "Unhandled Attrib Storage"); break;
            }
        break;
        case 4:
            switch (store)
            {
            case GA_STORE_INT32:  return AttribStorage::Vector4I; break;
            case GA_STORE_INT64:  return AttribStorage::Vector4L; break;
            case GA_STORE_REAL16: return AttribStorage::Vector4H; break;
            case GA_STORE_REAL32: return AttribStorage::Vector4F; break;
            case GA_STORE_REAL64: return AttribStorage::Vector4D; break;
            default: UT_ASSERT_MSG(0, "Unhandled Attrib Storage"); break;
            }
        break;
        default: UT_ASSERT_MSG(0, "Unhandled Attrib Tuple Size"); break;
        }
        return AttribStorage::Invalid;
    }

    AttribStorage getAttribStorageArray(const int tupleSize, const GA_Storage store)
    {
        switch (tupleSize)
        {
        case 1:
            switch (store)
            {
            case GA_STORE_INT32:  return AttribStorage::Int32Array;    break;
            case GA_STORE_INT64:  return AttribStorage::Int64Array;    break;
            case GA_STORE_REAL32: return AttribStorage::Fpreal32Array; break;
            case GA_STORE_REAL64: return AttribStorage::Fpreal64Array; break;
            default: UT_ASSERT_MSG(0, "Unhandled Attrib Storage"); break;
            }
        break;
        case 2:
            switch (store)
            {
            case GA_STORE_INT32:  return AttribStorage::Vector2IArray; break;
            case GA_STORE_INT64:  return AttribStorage::Vector2LArray; break;
            case GA_STORE_REAL32: return AttribStorage::Vector2FArray; break;
            case GA_STORE_REAL64: return AttribStorage::Vector2DArray; break;
            default: UT_ASSERT_MSG(0, "Unhandled Attrib Storage"); break;
            }
        break;
        case 3:
            switch (store)
            {
            case GA_STORE_INT32:  return AttribStorage::Vector3IArray; break;
            case GA_STORE_INT64:  return AttribStorage::Vector3LArray; break;
            case GA_STORE_REAL32: return AttribStorage::Vector3FArray; break;
            case GA_STORE_REAL64: return AttribStorage::Vector3DArray; break;
            default: UT_ASSERT_MSG(0, "Unhandled Attrib Storage"); break;
            }
        break;
        case 4:
            switch (store)
            {
            case GA_STORE_INT32:  return AttribStorage::Vector4IArray; break;
            case GA_STORE_INT64:  return AttribStorage::Vector4LArray; break;
            case GA_STORE_REAL32: return AttribStorage::Vector4FArray; break;
            case GA_STORE_REAL64: return AttribStorage::Vector4DArray; break;
            default: UT_ASSERT_MSG(0, "Unhandled Attrib Storage"); break;
            }
        break;
        default: UT_ASSERT_MSG(0, "Unhandled Attrib Tuple Size"); break;
        }
        return AttribStorage::Invalid;
    }


    
    AttribStorage getAttribStorage(const GA_Attribute& attrib)
    {
        const GA_AIFTuple* const aifTuple = attrib.getAIFTuple();
        if (aifTuple)
            return getAttribStorageTuple(attrib.getTupleSize(), aifTuple->getStorage(&attrib));
        
        //const GA_AIFStringTuple* const aifStrTuple = attrib.getAIFStringTuple();
        if (attrib.getAIFStringTuple())
            return AttribStorage::String;
        
        //const GA_AIFSharedDictTuple* const aifDictTuple = attrib.getAIFSharedDictTuple();
        if (attrib.getAIFSharedDictTuple())
            return AttribStorage::Dict;

        const GA_AIFNumericArray* const aifNumArray = attrib.getAIFNumericArray();
        if (aifNumArray)
            return getAttribStorageArray(attrib.getTupleSize(), aifNumArray->getStorage(&attrib));
        
        //const GA_AIFSharedDictArray* const aifDictArray = attrib.getAIFSharedDictArray();
        if (attrib.getAIFSharedDictArray())
            return AttribStorage::DictArray;
        
        //const GA_AIFSharedStringArray* const aifStrArray = attrib.getAIFSharedStringArray();
        if (attrib.getAIFSharedStringArray())
            return AttribStorage::StringArray;
        
        return AttribStorage::Invalid;
    }
    
    SYS_FORCE_INLINE AttribStorage getAttribStorage(const GA_Attribute* const attrib)
    { UT_ASSERT_P(attrib); return getAttribStorage(*attrib); }



} // End of namespace GFE_Type





_GFE_END


#endif
