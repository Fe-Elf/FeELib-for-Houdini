
#pragma once

#ifndef __GFE_Attribute_h__
#define __GFE_Attribute_h__

#include <GFE/Attribute.h>

#include <GA/GA_Detail.h>
#include <GA/GA_SplittableRange.h> // SOP_FeE_HasGroup_1_0 can not compile without this header
#include <GA/GA_PageHandle.h> 
#include <GA/GA_PageIterator.h> // SOP_FeE_HasGroup_1_0 can not compile without this header
#include <GA/GA_AttributeFilter.h>

#include <GFE/Math.h>
#include <GFE/Type.h>


_GFE_BEGIN
inline namespace Attribute {

    
enum class PieceAttribSearchOrder
{
    Primitive,
    Point,
    Vertex,
    PrimPoint,
    PointPrim,
    All,
    Invalid = -1,
};


enum class NormalSearchOrder
{
    Primitive,
    Point,
    Vertex,
    Detail,
    PointVertex,
    PrimVertex,
    All,
    Global = Detail,
    Invalid = -1,
};



template<typename FLOAT_T>
static void accumulateT(GA_Attribute& attrib, const GA_SplittableRange& r)
{
    FLOAT_T valuePrev = 0;
    GA_PageHandleT<FLOAT_T, FLOAT_T, true, true, GA_Attribute, GA_ATINumeric, GA_Detail> attrib_ph(&attrib);
    GA_Offset start, end;
    for (GA_PageIterator pit = r.beginPages(); !pit.atEnd(); ++pit)
    {
        for (GA_Iterator it(pit.begin()); it.blockAdvance(start, end); )
        {
            attrib_ph.setPage(start);
            for (GA_Offset elemoff = start; elemoff < end; ++elemoff)
            {
                valuePrev += attrib_ph.value(elemoff);
                attrib_ph.value(elemoff) = valuePrev;
            }
        }
    }
}


static bool isSameType(const GA_Attribute& srcAttrib, const GA_Attribute& dstAttrib)
{
    return srcAttrib.getStorageClass() == dstAttrib.getStorageClass() &&
           srcAttrib.getTupleSize() == dstAttrib.getTupleSize() &&
           (
               (srcAttrib.getAIFTuple()             && dstAttrib.getAIFTuple())             ||
               (srcAttrib.getAIFStringTuple()       && dstAttrib.getAIFStringTuple())       ||
               (srcAttrib.getAIFSharedDictTuple()   && dstAttrib.getAIFSharedDictTuple())   ||
               (srcAttrib.getAIFNumericArray()      && dstAttrib.getAIFNumericArray())      ||
               (srcAttrib.getAIFSharedStringArray() && dstAttrib.getAIFSharedStringArray()) ||
               (srcAttrib.getAIFSharedDictArray()   && dstAttrib.getAIFSharedDictArray())   
           );
}


static void computeIndexAttrib(GA_Attribute& attrib, const GA_SplittableRange* const srange = nullptr, const GA_Index startIndex = 0)
{
    UT_ASSERT_P(attrib.getAIFTuple());
    UT_ASSERT_P(attrib.getStorageClass() == GA_STORECLASS_INT);
    const GA_SplittableRange& r = srange ? *srange : GA_SplittableRange(GA_Range(attrib.getIndexMap()));
    GA_Index index = startIndex;
    
    GA_PageHandleT<GA_Index, GA_Index, true, true, GA_Attribute, GA_ATINumeric, GA_Detail> attrib_ph(&attrib);
    GA_Offset start, end;
    for (GA_PageIterator pit = r.beginPages(); !pit.atEnd(); ++pit)
    {
        for (GA_Iterator it(pit.begin()); it.blockAdvance(start, end); )
        {
            attrib_ph.setPage(start);
            for (GA_Offset elemoff = start; elemoff < end; ++elemoff)
            {
                attrib_ph.value(elemoff) = index++;
            }
        }
    }
}
    
static GA_AttributeUPtr createDetachedIndexAttrib(GA_Detail& geo, const GA_AttributeOwner owner, const GA_SplittableRange* const srange = nullptr, const GA_Index startIndex = 0)
{
    //GA_AttributeUPtr attribUPtr = geo.createDetachedTupleAttribute(owner, gfe::Type::getPreferredStorageI(geo), 1, GA_Defaults(GFE_INVALID_OFFSET));
    GA_AttributeUPtr attribUPtr = geo.createDetachedTupleAttribute(owner, GA_STORE_INT64, 1, GA_Defaults(gfe::INVALID_OFFSET));
    computeIndexAttrib(*attribUPtr.get(), srange, startIndex);
    return attribUPtr;
}


    












template<typename ARRAY_T>
static void cloneArray(GA_Attribute& outAttrib, const GA_Attribute& inAttrib,
    const exint subscribeRatio = 64, const exint minGrainSize = 1024)
{
    UT_ASSERT_MSG(outAttrib.getOwner() == inAttrib.getOwner(), "not same owner");
    
#if 1
    outAttrib.replace(inAttrib);
#else
    const GA_RWHandleT<ARRAY_T> out_h(&outAttrib);
    const GA_ROHandleT<ARRAY_T> in_h(&inAttrib);
    const GA_SplittableRange geoSplittableRange(GA_Range(outAttrib.getIndexMap()));
    UTparallelFor(geoSplittableRange, [&out_h, &in_h](const GA_SplittableRange& r)
    {
        ARRAY_T array;
        GA_Offset start, end;
        for (GA_PageIterator pit = r.beginPages(); !pit.atEnd(); ++pit)
        {
            for (GA_Iterator it(pit.begin()); it.blockAdvance(start, end); )
            {
                for (GA_Offset elemoff = start; elemoff < end; ++elemoff)
                {
#if 1
                    outAttrib.copy(elemoff, inAttrib, elemoff);
#else
                    in_h.get(elemoff, array);
                    out_h.set(elemoff, array);
#endif
                }
            }
        }
    }, subscribeRatio, minGrainSize);
#endif
}



template<typename VECTOR_T>
static void cloneVec(GA_Attribute& outAttrib, const GA_Attribute& inAttrib,
                     const exint subscribeRatio = 64, const exint minGrainSize = 1024
)
{
    UT_ASSERT_MSG(outAttrib.getOwner() == inAttrib.getOwner(), "not same owner");
    
    const GA_SplittableRange geoSplittableRange(GA_Range(outAttrib.getIndexMap()));
    UTparallelFor(geoSplittableRange, [&outAttrib, &inAttrib](const GA_SplittableRange& r)
    {
        GA_PageHandleT<VECTOR_T, typename VECTOR_T::value_type, true, true, GA_Attribute, GA_ATINumeric, GA_Detail> out_ph(&outAttrib);
        GA_PageHandleT<VECTOR_T, typename VECTOR_T::value_type, true, false, const GA_Attribute, const GA_ATINumeric, const GA_Detail> in_ph(&inAttrib);
        GA_Offset start, end;
        for (GA_PageIterator pit = r.beginPages(); !pit.atEnd(); ++pit)
        {
            for (GA_Iterator it(pit.begin()); it.blockAdvance(start, end); )
            {
                out_ph.setPage(start);
                in_ph.setPage(start);
                for (GA_Offset elemoff = start; elemoff < end; ++elemoff)
                {
                    out_ph.value(elemoff) = in_ph.value(elemoff);
                }
            }
        }
    }, subscribeRatio, minGrainSize);
}


template<typename FLOAT_T>
static void clone(GA_Attribute& outAttrib, const GA_Attribute& inAttrib,
    const exint subscribeRatio = 64, const exint minGrainSize = 1024)
{
    UT_ASSERT_MSG(outAttrib.getOwner() == inAttrib.getOwner(), "not same owner");
    
    const GA_SplittableRange geoSplittableRange(GA_Range(outAttrib.getIndexMap()));
    UTparallelFor(geoSplittableRange, [&outAttrib, &inAttrib](const GA_SplittableRange& r)
    {
        GA_PageHandleT<FLOAT_T, FLOAT_T, true, true, GA_Attribute, GA_ATINumeric, GA_Detail> out_ph(&outAttrib);
        GA_PageHandleT<FLOAT_T, FLOAT_T, true, false, const GA_Attribute, const GA_ATINumeric, const GA_Detail> in_ph(&inAttrib);
        GA_Offset start, end;
        for (GA_PageIterator pit = r.beginPages(); !pit.atEnd(); ++pit)
        {
            for (GA_Iterator it(pit.begin()); it.blockAdvance(start, end); )
            {
                out_ph.setPage(start);
                in_ph.setPage(start);
                for (GA_Offset elemoff = start; elemoff < end; ++elemoff)
                {
                    out_ph.value(elemoff) = in_ph.value(elemoff);
                }
            }
        }
    }, subscribeRatio, minGrainSize);
}
    
template<>
static void clone<UT_StringHolder>(
    GA_Attribute& outAttrib, const GA_Attribute& inAttrib,
    const exint subscribeRatio, const exint minGrainSize)
{
    UT_ASSERT_MSG(outAttrib.getOwner() == inAttrib.getOwner(), "not same owner");

    GA_RWHandleS out_h(&outAttrib);
    GA_ROHandleS in_h(&inAttrib);
    UTparallelFor(GA_SplittableRange(GA_Range(outAttrib.getIndexMap())), [&out_h, &in_h](const GA_SplittableRange& r)
    {
        GA_Offset start, end;
        for (GA_Iterator it(r); it.blockAdvance(start, end); )
        {
            for (GA_Offset elemoff = start; elemoff < end; ++elemoff)
            {
                out_h.set(elemoff, in_h.get(elemoff));
            }
        }
    }, subscribeRatio, minGrainSize);
}

template<>
static void clone<UT_OptionsHolder>(GA_Attribute& outAttrib, const GA_Attribute& inAttrib,
    const exint subscribeRatio, const exint minGrainSize)
{
    UT_ASSERT_MSG(outAttrib.getOwner() == inAttrib.getOwner(), "not same owner");

    GA_RWHandleDict out_h(&outAttrib);
    GA_ROHandleDict in_h(&inAttrib);
    UTparallelFor(GA_SplittableRange(GA_Range(outAttrib.getIndexMap())), [&out_h, &in_h](const GA_SplittableRange& r)
    {
        GA_Offset start, end;
        for (GA_Iterator it(r); it.blockAdvance(start, end); )
        {
            for (GA_Offset elemoff = start; elemoff < end; ++elemoff)
            {
                out_h.set(elemoff, in_h.get(elemoff));
            }
        }
    }, subscribeRatio, minGrainSize);
}


static void clone(GA_Attribute& outAttrib, const GA_Attribute& inAttrib,
    const exint subscribeRatio = 64, const exint minGrainSize = 1024)
{
    UT_ASSERT_MSG(outAttrib.getOwner()        == inAttrib.getOwner(),        "not same Owner");
    UT_ASSERT_MSG(outAttrib.getTupleSize()    == inAttrib.getTupleSize(),    "not same Tuple Size");
    UT_ASSERT_MSG(outAttrib.getStorageClass() == inAttrib.getStorageClass(), "not same Storage Class");
    
    GA_Storage storage;
    if (outAttrib.getAIFTuple())
        storage = outAttrib.getAIFTuple()->getStorage(&outAttrib);
    else if (outAttrib.getAIFStringTuple())
        storage = GA_STORE_STRING;
    else if (outAttrib.getAIFNumericArray())
        storage = outAttrib.getAIFNumericArray()->getStorage(&outAttrib);
    else
    {
        UT_ASSERT_MSG(0, "not correct storage");
        storage = GA_STORE_INVALID;
        return;
    }

    const int tupleSize = outAttrib.getTupleSize();
    switch (tupleSize)
    {
    case 1:
    case 2:
    case 3:
    case 4: break;
    default: UT_ASSERT_MSG(0, "not correct TupleSize");
    }

    if (outAttrib.getAIFTuple())
    {
        switch (storage)
        {
        // case GA_STORE_INT16:
        //     switch (tupleSize)
        //     {
        //     case 1: clone               <int16> (outAttrib, inAttrib, subscribeRatio, minGrainSize); break;
        //     case 2: cloneVec<UT_Vector2T<int16>>(outAttrib, inAttrib, subscribeRatio, minGrainSize); break;
        //     case 3: cloneVec<UT_Vector3T<int16>>(outAttrib, inAttrib, subscribeRatio, minGrainSize); break;
        //     case 4: cloneVec<UT_Vector4T<int16>>(outAttrib, inAttrib, subscribeRatio, minGrainSize); break;
        //     }
        break;
        case GA_STORE_INT32:
            switch (tupleSize)
            {
            case 1: clone               <int32> (outAttrib, inAttrib, subscribeRatio, minGrainSize); break;
            case 2: cloneVec<UT_Vector2T<int32>>(outAttrib, inAttrib, subscribeRatio, minGrainSize); break;
            case 3: cloneVec<UT_Vector3T<int32>>(outAttrib, inAttrib, subscribeRatio, minGrainSize); break;
            case 4: cloneVec<UT_Vector4T<int32>>(outAttrib, inAttrib, subscribeRatio, minGrainSize); break;
            }
        break;
        case GA_STORE_INT64:
            switch (tupleSize)
            {
            case 1: clone               <int64> (outAttrib, inAttrib, subscribeRatio, minGrainSize); break;
            case 2: cloneVec<UT_Vector2T<int64>>(outAttrib, inAttrib, subscribeRatio, minGrainSize); break;
            case 3: cloneVec<UT_Vector3T<int64>>(outAttrib, inAttrib, subscribeRatio, minGrainSize); break;
            case 4: cloneVec<UT_Vector4T<int64>>(outAttrib, inAttrib, subscribeRatio, minGrainSize); break;
            }
        break;
        case GA_STORE_REAL16:
            switch (tupleSize)
            {
            case 1: clone               <fpreal16> (outAttrib, inAttrib, subscribeRatio, minGrainSize); break;
            case 2: cloneVec<UT_Vector2T<fpreal16>>(outAttrib, inAttrib, subscribeRatio, minGrainSize); break;
            case 3: cloneVec<UT_Vector3T<fpreal16>>(outAttrib, inAttrib, subscribeRatio, minGrainSize); break;
            case 4: cloneVec<UT_Vector4T<fpreal16>>(outAttrib, inAttrib, subscribeRatio, minGrainSize); break;
            }
        break;
        case GA_STORE_REAL32:
            switch (tupleSize)
            {
            case 1: clone               <fpreal32> (outAttrib, inAttrib, subscribeRatio, minGrainSize); break;
            case 2: cloneVec<UT_Vector2T<fpreal32>>(outAttrib, inAttrib, subscribeRatio, minGrainSize); break;
            case 3: cloneVec<UT_Vector3T<fpreal32>>(outAttrib, inAttrib, subscribeRatio, minGrainSize); break;
            case 4: cloneVec<UT_Vector4T<fpreal32>>(outAttrib, inAttrib, subscribeRatio, minGrainSize); break;
            }
        break;
        case GA_STORE_REAL64:
            switch (tupleSize)
            {
            case 1: clone               <fpreal64> (outAttrib, inAttrib, subscribeRatio, minGrainSize); break;
            case 2: cloneVec<UT_Vector2T<fpreal64>>(outAttrib, inAttrib, subscribeRatio, minGrainSize); break;
            case 3: cloneVec<UT_Vector3T<fpreal64>>(outAttrib, inAttrib, subscribeRatio, minGrainSize); break;
            case 4: cloneVec<UT_Vector4T<fpreal64>>(outAttrib, inAttrib, subscribeRatio, minGrainSize); break;
            }
        break;
        case GA_STORE_STRING: clone<UT_StringHolder >(outAttrib, inAttrib, subscribeRatio, minGrainSize); break;
        case GA_STORE_DICT:   clone<UT_OptionsHolder>(outAttrib, inAttrib, subscribeRatio, minGrainSize); break;
        default: UT_ASSERT_MSG(0, "not correct storage"); break;
        }
    }
    else
    {
        outAttrib.replace(inAttrib);
    }
    // else if (outAttrib.getAIFStringTuple())
    // {
    //     clone<UT_StringHolder>(outAttrib, inAttrib, subscribeRatio, minGrainSize);
    // }
    // else if (outAttrib.getAIFSharedDictTuple())
    // {
    //     clone<UT_OptionsHolder>(outAttrib, inAttrib, subscribeRatio, minGrainSize);
    // }
    // else if (outAttrib.getAIFNumericArray())
    // {
    //     switch (storage)
    //     {
    //     // case GA_STORE_INT16:
    //     //     switch (tupleSize)
    //     //     {
    //     //     case 1: clone               <int16> (outAttrib, inAttrib, subscribeRatio, minGrainSize); break;
    //     //     case 2: cloneVec<UT_Vector2T<int16>>(outAttrib, inAttrib, subscribeRatio, minGrainSize); break;
    //     //     case 3: cloneVec<UT_Vector3T<int16>>(outAttrib, inAttrib, subscribeRatio, minGrainSize); break;
    //     //     case 4: cloneVec<UT_Vector4T<int16>>(outAttrib, inAttrib, subscribeRatio, minGrainSize); break;
    //     //     }
    //     break;
    //     case GA_STORE_INT32:
    //         switch (tupleSize)
    //         {
    //         case 1: cloneArray               <int32> (outAttrib, inAttrib, subscribeRatio, minGrainSize); break;
    //         case 2: cloneArray<UT_Vector2T<int32>>(outAttrib, inAttrib, subscribeRatio, minGrainSize); break;
    //         case 3: cloneArray<UT_Vector3T<int32>>(outAttrib, inAttrib, subscribeRatio, minGrainSize); break;
    //         case 4: cloneArray<UT_Vector4T<int32>>(outAttrib, inAttrib, subscribeRatio, minGrainSize); break;
    //         }
    //     break;
    //     case GA_STORE_INT64:
    //         switch (tupleSize)
    //         {
    //         case 1: cloneArray               <int64> (outAttrib, inAttrib, subscribeRatio, minGrainSize); break;
    //         case 2: cloneArray<UT_Vector2T<int64>>(outAttrib, inAttrib, subscribeRatio, minGrainSize); break;
    //         case 3: cloneArray<UT_Vector3T<int64>>(outAttrib, inAttrib, subscribeRatio, minGrainSize); break;
    //         case 4: cloneArray<UT_Vector4T<int64>>(outAttrib, inAttrib, subscribeRatio, minGrainSize); break;
    //         }
    //     break;
    //     case GA_STORE_REAL16:
    //         switch (tupleSize)
    //         {
    //         case 1: cloneArray               <fpreal16> (outAttrib, inAttrib, subscribeRatio, minGrainSize); break;
    //         case 2: cloneArray<UT_Vector2T<fpreal16>>(outAttrib, inAttrib, subscribeRatio, minGrainSize); break;
    //         case 3: cloneArray<UT_Vector3T<fpreal16>>(outAttrib, inAttrib, subscribeRatio, minGrainSize); break;
    //         case 4: cloneArray<UT_Vector4T<fpreal16>>(outAttrib, inAttrib, subscribeRatio, minGrainSize); break;
    //         }
    //     break;
    //     case GA_STORE_REAL32:
    //         switch (tupleSize)
    //         {
    //         case 1: cloneArray               <fpreal32> (outAttrib, inAttrib, subscribeRatio, minGrainSize); break;
    //         case 2: cloneArray<UT_Vector2T<fpreal32>>(outAttrib, inAttrib, subscribeRatio, minGrainSize); break;
    //         case 3: cloneArray<UT_Vector3T<fpreal32>>(outAttrib, inAttrib, subscribeRatio, minGrainSize); break;
    //         case 4: cloneArray<UT_Vector4T<fpreal32>>(outAttrib, inAttrib, subscribeRatio, minGrainSize); break;
    //         }
    //     break;
    //     case GA_STORE_REAL64:
    //         switch (tupleSize)
    //         {
    //         case 1: cloneArray               <fpreal64> (outAttrib, inAttrib, subscribeRatio, minGrainSize); break;
    //         case 2: cloneArray<UT_Vector2T<fpreal64>>(outAttrib, inAttrib, subscribeRatio, minGrainSize); break;
    //         case 3: cloneArray<UT_Vector3T<fpreal64>>(outAttrib, inAttrib, subscribeRatio, minGrainSize); break;
    //         case 4: cloneArray<UT_Vector4T<fpreal64>>(outAttrib, inAttrib, subscribeRatio, minGrainSize); break;
    //         }
    //     break;
    //     default: UT_ASSERT_MSG(0, "not correct storage"); break;
    //     }
    // }
    // else if (outAttrib.getAIFSharedStringArray())
    // {
    //     cloneArray<UT_StringArray>(outAttrib, inAttrib, subscribeRatio, minGrainSize);
    // }
    // else if (outAttrib.getAIFSharedDictArray())
    // {
    //     cloneArray<UT_Array<UT_OptionsHolder>>(outAttrib, inAttrib, subscribeRatio, minGrainSize);
    // }
}
    

SYS_FORCE_INLINE static GA_Attribute* clone(GA_Detail& geo, const GA_Attribute& inAttrib, const UT_StringRef& name,
    const exint subscribeRatio = 64, const exint minGrainSize = 1024)
{
    GA_Attribute* const outAttrib = geo.getAttributes().cloneAttribute(inAttrib.getOwner(), name, inAttrib, true);
    clone(*outAttrib, inAttrib, subscribeRatio, minGrainSize);
    return outAttrib;
}
    
SYS_FORCE_INLINE static GA_Attribute* clone(GA_Detail* const geo, const GA_Attribute* const inAttrib, const UT_StringRef& name,
    const exint subscribeRatio = 64, const exint minGrainSize = 1024)
{
    UT_ASSERT_P(geo);
    UT_ASSERT_P(inAttrib);
    return clone(*geo, *inAttrib, name, subscribeRatio, minGrainSize);
}
    
SYS_FORCE_INLINE static GA_Attribute* clone(GA_Attribute& inAttrib, const UT_StringRef& name,
    const exint subscribeRatio = 64, const exint minGrainSize = 1024)
{ return clone(inAttrib.getDetail(), inAttrib, name, subscribeRatio, minGrainSize); }
    
SYS_FORCE_INLINE static GA_Attribute* clone(GA_Attribute* const inAttrib, const UT_StringRef& name,
    const exint subscribeRatio = 64, const exint minGrainSize = 1024)
{ UT_ASSERT_P(inAttrib); return clone(inAttrib->getDetail(), *inAttrib, name, subscribeRatio, minGrainSize); }
    

// SYS_FORCE_INLINE static GA_Attribute* clone(GA_Detail& geo, const GA_Attribute& inAttrib, const char* const name,
//     const exint subscribeRatio = 64, const exint minGrainSize = 1024)
// { clone(geo, inAttrib, UT_StringRef(name), subscribeRatio, minGrainSize); }
    
    
static GA_AttributeUPtr clone(GA_Detail& geo, const GA_Attribute& inAttrib,
    const exint subscribeRatio = 64, const exint minGrainSize = 1024)
{
    GA_Attribute* const outAttrib = geo.getAttributes().cloneTempAttribute(inAttrib.getOwner(), inAttrib, true);
    //GA_AttributeUPtr outAttribUPtr;
    //outAttribUPtr.
    clone(*outAttrib, inAttrib, subscribeRatio, minGrainSize);
    return GA_AttributeUPtr(outAttrib);
}

    
SYS_FORCE_INLINE static GA_AttributeUPtr clone(GA_Attribute& inAttrib,
    const exint subscribeRatio = 64, const exint minGrainSize = 1024)
{ return clone(inAttrib.getDetail(), inAttrib, subscribeRatio, minGrainSize); }


    

    
    
static GA_Storage getStorage(const GA_Attribute& attrib)
{
    const GA_AIFTuple* const aifTuple = attrib.getAIFTuple();
    if (aifTuple)
    {
        return aifTuple->getStorage(&attrib);
    }
    const GA_AIFNumericArray* const aifNumericArray = attrib.getAIFNumericArray();
    if (aifNumericArray)
    {
        return aifNumericArray->getStorage(&attrib);
    }
    //const GA_AIFStringTuple* const aifString = attrib.getAIFStringTuple();
    if (attrib.getAIFStringTuple() || attrib.getAIFSharedStringArray())
    {
        return GA_STORE_STRING;
    }
    return GA_STORE_INVALID;
}
    
SYS_FORCE_INLINE static GA_Storage getStorage(const GA_Attribute* const attrib)
{ UT_ASSERT_P(attrib); return getStorage(*attrib); }



SYS_FORCE_INLINE static GA_Precision getPrecision(const GA_Attribute& attrib)
{ return gfe::Type::precisionFromStorage(getStorage(attrib)); }
    
SYS_FORCE_INLINE static GA_Precision getPrecision(const GA_Attribute* const attrib)
{ UT_ASSERT_P(attrib); return getPrecision(*attrib); }

    
    
static gfe::NormalSearchOrder toNormalSearchOrder(const GA_GroupType groupType)
{
    switch (groupType)
    {
    case GA_GROUP_PRIMITIVE: return gfe::NormalSearchOrder::Primitive; break;
    case GA_GROUP_POINT:     return gfe::NormalSearchOrder::Point;     break;
    case GA_GROUP_VERTEX:    return gfe::NormalSearchOrder::Vertex;    break;
    case GA_GROUP_N:         return gfe::NormalSearchOrder::All;       break;
    case GA_GROUP_INVALID:   return gfe::NormalSearchOrder::Invalid;   break;
    }
    return gfe::NormalSearchOrder::Invalid;
}
SYS_FORCE_INLINE static gfe::NormalSearchOrder toNormalSearchOrder(const GA_Group& group)
{ return toNormalSearchOrder(group.classType()); }
    
SYS_FORCE_INLINE static gfe::NormalSearchOrder toNormalSearchOrder(const GA_Group* const group)
{ UT_ASSERT_P(group); return toNormalSearchOrder(*group); }


    
static gfe::NormalSearchOrder toNormalSearchOrder(const GA_AttributeOwner owner)
{
    switch (owner)
    {
    case GA_ATTRIB_PRIMITIVE: return gfe::NormalSearchOrder::Primitive; break;
    case GA_ATTRIB_POINT:     return gfe::NormalSearchOrder::Point;     break;
    case GA_ATTRIB_VERTEX:    return gfe::NormalSearchOrder::Vertex;    break;
    case GA_ATTRIB_DETAIL:    return gfe::NormalSearchOrder::Detail;    break;
    case GA_ATTRIB_OWNER_N:   return gfe::NormalSearchOrder::All;       break;
    case GA_ATTRIB_INVALID:   return gfe::NormalSearchOrder::Invalid;   break;
    }
    return gfe::NormalSearchOrder::Invalid;
}

static GA_AttributeOwner toOwner(const gfe::NormalSearchOrder normalSearchOrder)
{
    switch (normalSearchOrder)
    {
    case gfe::NormalSearchOrder::Primitive:   return GA_ATTRIB_PRIMITIVE; break;
    case gfe::NormalSearchOrder::Point:       return GA_ATTRIB_POINT;     break;
    case gfe::NormalSearchOrder::Vertex:      return GA_ATTRIB_VERTEX;    break;
    case gfe::NormalSearchOrder::Detail:      return GA_ATTRIB_DETAIL;    break;
    case gfe::NormalSearchOrder::PointVertex: return GA_ATTRIB_VERTEX;    break;
    case gfe::NormalSearchOrder::All:         return GA_ATTRIB_OWNER_N;   break;
    case gfe::NormalSearchOrder::Invalid:     return GA_ATTRIB_INVALID;   break;
    }
    return GA_ATTRIB_INVALID;
}

static GA_AttributeOwner toValidOwner(const gfe::NormalSearchOrder normalSearchOrder)
{
    switch (normalSearchOrder)
    {
    case gfe::NormalSearchOrder::Primitive:   return GA_ATTRIB_PRIMITIVE;  break;
    case gfe::NormalSearchOrder::Point:       return GA_ATTRIB_POINT;      break;
    case gfe::NormalSearchOrder::Vertex:      return GA_ATTRIB_VERTEX;     break;
    case gfe::NormalSearchOrder::Detail:      return GA_ATTRIB_DETAIL;     break;
    case gfe::NormalSearchOrder::PointVertex: return GA_ATTRIB_VERTEX;     break;
    case gfe::NormalSearchOrder::All:         return GA_ATTRIB_VERTEX;     break;
    case gfe::NormalSearchOrder::Invalid:     return GA_ATTRIB_VERTEX;     break;
    default:                                 return GA_ATTRIB_VERTEX;     break;
    }
}

//
// static void bumpDataId(
//     GA_Detail& geo,
//     const GA_AttributeOwner owner,
//     const UT_StringRef& attribPattern
// )
// {
//     GA_AttributeFilter filter = GA_AttributeFilter::selectByPattern(attribPattern);
//     UT_Array<GA_Attribute*> attribList;
//     geo.getAttributes().matchAttributes(filter, owner, attribList);
//     for (GA_Size i = 0; i < attribList.size(); ++i)
//     {
//         attribList[i]->bumpDataId();
//     }
// }


static GA_Attribute* findPieceAttrib(
    GA_Detail& geo,
    const gfe::PieceAttribSearchOrder pieceAttribSearchOrder,
    const UT_StringRef& pieceAttribName
)
{
    GA_Attribute* attribPtr = nullptr;

    switch (pieceAttribSearchOrder)
    {
    case gfe::PieceAttribSearchOrder::Primitive:  attribPtr = geo.findAttribute(GA_ATTRIB_PRIMITIVE, pieceAttribName); break;
    case gfe::PieceAttribSearchOrder::Point:      attribPtr = geo.findAttribute(GA_ATTRIB_POINT,     pieceAttribName); break;
    case gfe::PieceAttribSearchOrder::Vertex:     attribPtr = geo.findAttribute(GA_ATTRIB_VERTEX,    pieceAttribName); break;
    case gfe::PieceAttribSearchOrder::PrimPoint:
    {
        GA_AttributeOwner searchOrder[2] = { GA_ATTRIB_PRIMITIVE, GA_ATTRIB_POINT };
        attribPtr = geo.findAttribute(pieceAttribName, searchOrder, 2);
    }
        break;
    case gfe::PieceAttribSearchOrder::PointPrim:
    {
        GA_AttributeOwner searchOrder[2] = { GA_ATTRIB_POINT, GA_ATTRIB_PRIMITIVE };
        attribPtr = geo.findAttribute(pieceAttribName, searchOrder, 2);
    }
        break;
    case gfe::PieceAttribSearchOrder::All:
    {
        GA_AttributeOwner searchOrder[3] = { GA_ATTRIB_PRIMITIVE, GA_ATTRIB_POINT, GA_ATTRIB_VERTEX };
        attribPtr = geo.findAttribute(pieceAttribName, searchOrder, 3);
    }
        break;
    default:
        UT_ASSERT_MSG(0, "Unhandled Geo Piece Attrib Search Order!");
        break;
    }
    return attribPtr;
}

// SYS_FORCE_INLINE
// static bool
// renameAttribute(
//     GA_Detail& geo,
//     const GA_Attribute& attrib,
//     const UT_StringRef& newName
// )
// {
//     return geo.renameAttribute(attrib.getOwner(), attrib.getScope(), attrib.getName(), newName);
// }
//
//
//
SYS_FORCE_INLINE static bool renameAttribute(GA_Attribute& attrib, const UT_StringRef& newName)
{ return attrib.getDetail().renameAttribute(attrib.getOwner(), attrib.getScope(), attrib.getName(), newName); }

    
static bool forceRenameAttribute(
    GA_Detail& geo,
    GA_Attribute& attrib,
    const UT_StringRef& newName
)
{
    GA_Attribute* const existAttribute = geo.findAttribute(attrib.getOwner(), newName);
    if (existAttribute)
        geo.getAttributes().destroyAttribute(existAttribute);
    return geo.renameAttribute(attrib.getOwner(), GA_SCOPE_PUBLIC, attrib.getName(), newName);
}


SYS_FORCE_INLINE static bool forceRenameAttribute(GA_Attribute& attrib,const UT_StringRef& newName)
{ return forceRenameAttribute(attrib.getDetail(), attrib, newName); }






//GFE_Attribute::findAttributePointVertex(geo, geo0AttribClass, geo0AttribNameSub, geo0AttribClassFinal)
//GFE_Attribute::findAttributePointVertex(geo, attribOwner, GA_SCOPE_PUBLIC, attribName, attribOwnerFianl);

static const GA_Attribute*
findAttributePointVertex(
    const GA_Detail& geo,
    const GA_AttributeOwner attribOwner,
    const GA_AttributeScope scope,
    const UT_StringRef& attribName,
    GA_AttributeOwner& attribOwnerFinal
)
{
    if (!attribName.isstring() || attribName.length() == 0)
        return nullptr;

    const GA_Attribute* attribPtr = nullptr;
    const GA_AttributeSet& geoAttribs = geo.getAttributes();
    if (attribOwner < 0 || attribOwner >= GA_ATTRIB_PRIMITIVE)//not point or vertex means Auto
    {
        attribPtr = geoAttribs.findAttribute(GA_ATTRIB_VERTEX, scope, attribName);
        if (attribPtr)
            attribOwnerFinal = GA_ATTRIB_VERTEX;
        else
        {
            attribPtr = geoAttribs.findAttribute(GA_ATTRIB_POINT, scope, attribName);
            if (!attribPtr)
            {
                attribOwnerFinal = GA_ATTRIB_INVALID;
                return nullptr;
            }
            attribOwnerFinal = GA_ATTRIB_POINT;
        }
    }
    else
    {
        attribPtr = geoAttribs.findAttribute(attribOwner, scope, attribName);
        if (!attribPtr)
        {
            attribOwnerFinal = GA_ATTRIB_INVALID;
            return nullptr;
        }
        attribOwnerFinal = attribOwner;
    }
    return attribPtr;
}

static const GA_Attribute*
findAttributePointVertex(
    const GA_Detail& geo,
    const GA_AttributeOwner attribOwner,
    const GA_AttributeScope scope,
    const UT_StringRef& attribName
)
{
    if (!attribName.isstring() || attribName.length() == 0)
        return nullptr;

    const GA_Attribute* attribPtr = nullptr;
    const GA_AttributeSet& geoAttribs = geo.getAttributes();
    if (attribOwner < 0 || attribOwner >= GA_ATTRIB_PRIMITIVE)//not point or vertex means Auto
    {
        attribPtr = geoAttribs.findAttribute(GA_ATTRIB_VERTEX, scope, attribName);
        if (!attribPtr)
        {
            attribPtr = geoAttribs.findAttribute(GA_ATTRIB_POINT, scope, attribName);
            if (!attribPtr)
            {
                return nullptr;
            }
        }
    }
    else
    {
        attribPtr = geoAttribs.findAttribute(attribOwner, scope, attribName);
        if (!attribPtr)
        {
            return nullptr;
        }
    }
    return attribPtr;
}





static GA_Attribute*
findAttributePointVertex(
    GA_Detail& geo,
    const GA_AttributeOwner attribOwner,
    const GA_AttributeScope scope,
    const UT_StringRef& attribName,
    GA_AttributeOwner& attribOwnerFinal
)
{
    if (!attribName.isstring() || attribName.length() == 0)
        return nullptr;

    GA_Attribute* attribPtr = nullptr;
    GA_AttributeSet& geoAttribs = geo.getAttributes();
    if (attribOwner < 0 || attribOwner >= GA_ATTRIB_PRIMITIVE)//not point or vertex means Auto
    {
        attribPtr = geoAttribs.findAttribute(GA_ATTRIB_VERTEX, scope, attribName);
        if (attribPtr)
            attribOwnerFinal = GA_ATTRIB_VERTEX;
        else
        {
            attribPtr = geoAttribs.findAttribute(GA_ATTRIB_POINT, scope, attribName);
            if (!attribPtr)
            {
                attribOwnerFinal = GA_ATTRIB_INVALID;
                return nullptr;
            }
            attribOwnerFinal = GA_ATTRIB_POINT;
        }
    }
    else
    {
        attribPtr = geoAttribs.findAttribute(attribOwner, scope, attribName);
        if (!attribPtr)
        {
            attribOwnerFinal = GA_ATTRIB_INVALID;
            return nullptr;
        }
        attribOwnerFinal = attribOwner;
    }
    return attribPtr;
}



static GA_Attribute*
findAttributePointVertex(
    GA_Detail& geo,
    const GA_AttributeOwner attribOwner,
    const GA_AttributeScope scope,
    const UT_StringRef& attribName
)
{
    if (!attribName.isstring() || attribName.length() == 0)
        return nullptr;

    GA_Attribute* attribPtr;
    GA_AttributeSet& geoAttribs = geo.getAttributes();
    if (attribOwner < 0 || attribOwner >= GA_ATTRIB_PRIMITIVE)//not point or vertex means Auto
    {
        attribPtr = geoAttribs.findAttribute(GA_ATTRIB_VERTEX, scope, attribName);
        if (!attribPtr)
            attribPtr = geoAttribs.findAttribute(GA_ATTRIB_POINT, scope, attribName);
    }
    else
        attribPtr = geoAttribs.findAttribute(attribOwner, scope, attribName);
    return attribPtr;
}




//GFE_Attribute::findAttributePointVertex(geo, geo0AttribClass, geo0AttribNameSub, geo0AttribClassFinal)
//GFE_Attribute::findAttributePointVertex(geo, attribOwner, attribName, attribOwnerFianl);



SYS_FORCE_INLINE
static const GA_Attribute*
findAttributePointVertex(
    const GA_Detail& geo,
    const GA_AttributeOwner attribOwner,
    const UT_StringRef& attribName,
    GA_AttributeOwner& attribOwnerFianl
)
{ return findAttributePointVertex(geo, attribOwner, GA_SCOPE_PUBLIC, attribName, attribOwnerFianl); }

SYS_FORCE_INLINE
static const GA_Attribute*
findAttributePointVertex(
    const GA_Detail& geo,
    const GA_AttributeOwner attribOwner,
    const UT_StringRef& attribName
)
{ return findAttributePointVertex(geo, attribOwner, GA_SCOPE_PUBLIC, attribName); }



SYS_FORCE_INLINE
static GA_Attribute*
findAttributePointVertex(
    GA_Detail& geo,
    const GA_AttributeOwner attribOwner,
    const UT_StringRef& attribName,
    GA_AttributeOwner& attribOwnerFianl
)
{ return findAttributePointVertex(geo, attribOwner, GA_SCOPE_PUBLIC, attribName, attribOwnerFianl); }

SYS_FORCE_INLINE
static GA_Attribute*
findAttributePointVertex(
    GA_Detail& geo,
    const GA_AttributeOwner attribOwner,
    const UT_StringRef& attribName
)
{ return findAttributePointVertex(geo, attribOwner, GA_SCOPE_PUBLIC, attribName); }




static GA_Attribute*
findUVAttributePointVertex(
    GA_Detail& geo,
    const GA_AttributeOwner uvAttribClass = GA_ATTRIB_INVALID,
    const UT_StringRef& uvAttribName = "uv"
)
{
    if (!uvAttribName.isstring() || uvAttribName.length() == 0)
        return nullptr;

    GA_Attribute* uvAttribPtr = findAttributePointVertex(geo, uvAttribClass, uvAttribName);
    if (uvAttribPtr)
    {
        const int tupleSize = uvAttribPtr->getTupleSize();
        if (tupleSize < 2 || tupleSize > 4)
        {
            //geo.getAttributes().destroyAttribute(uvAttribPtr);
            uvAttribPtr = nullptr;
        }
    }
    return uvAttribPtr;
}

static const GA_Attribute*
findUVAttributePointVertex(
    const GA_Detail& geo,
    const GA_AttributeOwner uvAttribClass = GA_ATTRIB_INVALID,
    const UT_StringRef& uvAttribName = "uv"
)
{
    if (!uvAttribName.isstring() || uvAttribName.length() == 0)
        return nullptr;

    const GA_Attribute* uvAttribPtr = findAttributePointVertex(geo, uvAttribClass, uvAttribName);
    if (uvAttribPtr)
    {
        const int tupleSize = uvAttribPtr->getTupleSize();
        if (tupleSize < 2 || tupleSize > 4)
        {
            //geo.getAttributes().destroyAttribute(uvAttribPtr);
            uvAttribPtr = nullptr;
        }
    }
    return uvAttribPtr;
}



static GA_Attribute*
findOrCreateUVAttributePointVertex(
    GA_Detail& geo,
    const GA_AttributeOwner uvAttribClass = GA_ATTRIB_INVALID,
    const UT_StringRef& uvAttribName = "uv",
    const GA_Storage storage = GA_STORE_INVALID
)
{
    if (!uvAttribName.isstring() || uvAttribName.length() == 0)
        return nullptr;

    GA_Attribute* uvAttribPtr = findAttributePointVertex(geo, uvAttribClass, uvAttribName);
    if (uvAttribPtr)
    {
        int tupleSize = uvAttribPtr->getTupleSize();
        if (tupleSize < 2 || tupleSize > 4)
        {
            geo.getAttributes().destroyAttribute(uvAttribPtr);
            uvAttribPtr = nullptr;
        }
    }
    if (!uvAttribPtr)
    {
        const GA_Storage finalStorageF = storage == GA_STORE_INVALID ? gfe::Type::getPreferredStorageF(geo) : storage;
#if 1
        uvAttribPtr = static_cast<GEO_Detail&>(geo).addTextureAttribute(uvAttribClass == GA_ATTRIB_POINT ? GA_ATTRIB_POINT : GA_ATTRIB_VERTEX, finalStorageF);
#else
        uvAttribPtr = geo.getAttributes().createTupleAttribute(
            uvAttribClass == GA_ATTRIB_POINT ? GA_ATTRIB_POINT : GA_ATTRIB_VERTEX, uvAttribName, finalStorageF, 3, GA_Defaults(0));
#endif
    }
    return uvAttribPtr;
}





static const GA_Attribute*
findNormal3D(
    const GA_Detail& geo,
    const gfe::NormalSearchOrder normalSearchOrder = gfe::NormalSearchOrder::Invalid,
    const UT_StringRef& normal3DAttribName = "N"
)
{
    if (!normal3DAttribName.isstring() || normal3DAttribName.length() == 0)
        return nullptr;

    const GA_Attribute* normal3DAttrib = nullptr;
    switch (normalSearchOrder)
    {
    case gfe::NormalSearchOrder::Primitive:
        normal3DAttrib = geo.findPrimitiveAttribute(normal3DAttribName);
        break;
    case gfe::NormalSearchOrder::Point:
        normal3DAttrib = geo.findPointAttribute(normal3DAttribName);
        break;
    case gfe::NormalSearchOrder::Vertex:
        normal3DAttrib = geo.findVertexAttribute(normal3DAttribName);
        break;
    case gfe::NormalSearchOrder::Detail:
        normal3DAttrib = geo.findGlobalAttribute(normal3DAttribName);
        break;
    case gfe::NormalSearchOrder::PointVertex:
        normal3DAttrib = geo.findPointAttribute(normal3DAttribName);
        if (!normal3DAttrib)
            normal3DAttrib = geo.findVertexAttribute(normal3DAttribName);
        break;
    case gfe::NormalSearchOrder::All:
        normal3DAttrib = geo.findPrimitiveAttribute(normal3DAttribName);
        if (!normal3DAttrib)
        {
            normal3DAttrib = geo.findPointAttribute(normal3DAttribName);
            if (!normal3DAttrib)
            {
                normal3DAttrib = geo.findVertexAttribute(normal3DAttribName);
                if (!normal3DAttrib)
                    normal3DAttrib = geo.findGlobalAttribute(normal3DAttribName);
            }
        }
        break;
    default:
        UT_ASSERT_MSG(0, "unhandled gfe::NormalSearchOrder");
        break;
    }
    return normal3DAttrib;
}


static GA_Attribute* findNormal3D(
    GA_Detail& geo,
    const gfe::NormalSearchOrder normalSearchOrder = gfe::NormalSearchOrder::Invalid,
    const UT_StringRef& name = "N"
)
{
    if (!name.isstring() || name.length() == 0)
        return nullptr;

    GA_Attribute* normal3DAttrib = nullptr;
    switch (normalSearchOrder)
    {
    case gfe::NormalSearchOrder::Primitive:   normal3DAttrib = geo.findPrimitiveAttribute(name); break;
    case gfe::NormalSearchOrder::Point:       normal3DAttrib = geo.findPointAttribute    (name); break;
    case gfe::NormalSearchOrder::Vertex:      normal3DAttrib = geo.findVertexAttribute   (name); break;
    case gfe::NormalSearchOrder::Detail:      normal3DAttrib = geo.findGlobalAttribute   (name); break;
    case gfe::NormalSearchOrder::PointVertex:
        normal3DAttrib = geo.findPointAttribute(name);
        if (!normal3DAttrib)
            normal3DAttrib = geo.findVertexAttribute(name);
    break;
    case gfe::NormalSearchOrder::All:
        normal3DAttrib = geo.findPrimitiveAttribute(name);
        if (!normal3DAttrib)
        {
            normal3DAttrib = geo.findPointAttribute(name);
            if (!normal3DAttrib)
            {
                normal3DAttrib = geo.findVertexAttribute(name);
                if (!normal3DAttrib)
                    normal3DAttrib = geo.findGlobalAttribute(name);
            }
        }
    break;
    default: UT_ASSERT_MSG(0, "unhandled gfe::NormalSearchOrder"); break;
    }
    return normal3DAttrib;
}















    
template<typename _Ty>
class ComputeAttribSum
{
public:
    ComputeAttribSum(const GA_ROHandleT<_Ty>& attrib_h)
        : attrib_h(attrib_h)
        , mySum(0)
    {}
    ComputeAttribSum(ComputeAttribSum& src, UT_Split)
        : attrib_h(src.attrib_h)
        , mySum(0)
    {}
    void operator()(const GA_SplittableRange& r)
    {
        UT_ASSERT_MSG(r.getOwner() == attrib_h->getOwner(), "not same owner");
        GA_PageHandleT<_Ty, _Ty, true, true, GA_Attribute, GA_ATINumeric, GA_Detail> attrib_ph(attrib_h.getAttribute());
        GA_Offset start, end;
        for (GA_PageIterator pit = r.beginPages(); !pit.atEnd(); ++pit)
        {
            for (GA_Iterator it(pit.begin()); it.blockAdvance(start, end); )
            {
                attrib_ph.setPage(start);
                for (GA_Offset elemoff = start; elemoff < end; ++elemoff)
                {
                    mySum += attrib_ph.value(elemoff);
                }
            }
        }
    }
    SYS_FORCE_INLINE void join(const ComputeAttribSum& other)
    { mySum += other.mySum; }
    SYS_FORCE_INLINE _Ty getSum() const
    { return mySum; }
private:
    _Ty mySum;
    const GA_ROHandleT<_Ty>& attrib_h;
}; // End of Class ComputeAttribSum




    template<typename _Ty>
    _Ty computeAttribSum(
        const GA_ROHandleT<_Ty>& attrib_h,
        const GA_SplittableRange& splittableRange,
        const exint subscribeRatio = 64,
        const exint minGrainSize   = 1024
    )
    {
        ComputeAttribSum body(attrib_h);
        UTparallelReduce(splittableRange, body, subscribeRatio, minGrainSize);
        return body.getSum();
    }
    
    template<typename _Ty>
    SYS_FORCE_INLINE _Ty computeAttribSum(
        const GA_ROHandleT<_Ty>& attrib_h,
        const GA_ElementGroup* const group = nullptr,
        const exint subscribeRatio = 64,
        const exint minGrainSize   = 1024
    )
    {
        //const GA_Range range(group->getIndexMap(), group);
        //const GA_SplittableRange splittableRange(range);
        const GA_SplittableRange splittableRange(GA_Range(attrib_h->getIndexMap(), group));
        return computeAttribSum<_Ty>(attrib_h, splittableRange, subscribeRatio, minGrainSize);
    }



    
    template<typename _Ty, bool _Min, bool _Max, bool _MinElemoff = false, bool _MaxElemoff = false>
    class ComputeAttribExtremum
    {
    using value_type = typename gfe::TypeTraits::get_value_type_t<_Ty>;
    public:
        ComputeAttribExtremum(const GA_Attribute* const attrib)
            : myAttrib(attrib)
            //, myMin(gfe::Type::numeric_limits<_Ty>::max())
            //, myMax(gfe::Type::numeric_limits<_Ty>::lowest())
            //, myMinElemoff(GFE_INVALID_OFFSET)
            //, myMaxElemoff(GFE_INVALID_OFFSET)
        {}
        ComputeAttribExtremum(ComputeAttribExtremum& src, UT_Split)
            : myAttrib(src.myAttrib)
            //, myMin(gfe::Type::numeric_limits<_Ty>::max())
            //, myMax(gfe::Type::numeric_limits<_Ty>::lowest())
            //, myMinElemoff(GFE_INVALID_OFFSET)
            //, myMaxElemoff(GFE_INVALID_OFFSET)
        {}
        void operator()(const GA_SplittableRange& r)
        {
            if constexpr (!_Min && !_Max && !_MinElemoff && !_MaxElemoff)
                return;
            UT_ASSERT_MSG(r.getOwner() == myAttrib->getOwner(), "not same owner");
            gfe::ROPageHandleT<_Ty> attrib_ph(myAttrib);
            GA_Offset start, end;
            for (GA_PageIterator pit = r.beginPages(); !pit.atEnd(); ++pit)
            {
                for (GA_Iterator it(pit.begin()); it.blockAdvance(start, end); )
                {
                    attrib_ph.setPage(start);

                    if constexpr ((_Min || _MinElemoff) && (_Max || _MaxElemoff))
                    {
                        const value_type length = gfe::length(attrib_ph.value(start));
                        if (length < myMin)
                        {
                            myMin = length;
                            if constexpr (_MinElemoff)
                                myMinElemoff = start;
                        }
                        if (length > myMax)
                        {
                            myMax = length;
                            if constexpr (_MaxElemoff)
                                myMaxElemoff = start;
                        }
                        for (GA_Offset elemoff = start+1; elemoff < end; ++elemoff)
                        {
                            const value_type length = gfe::length(attrib_ph.value(elemoff));
                            if (length < myMin)
                            {
                                myMin = length;
                                if constexpr (_MinElemoff)
                                    myMinElemoff = elemoff;
                            }
                            if (length > myMax)
                            {
                                myMax = length;
                                if constexpr (_MaxElemoff)
                                    myMaxElemoff = elemoff;
                            }
                        }
                    }
                    else
                    {
                        
                        for (GA_Offset elemoff = start; elemoff < end; ++elemoff)
                        {
                            const value_type length = gfe::length(attrib_ph.value(elemoff));
                            if constexpr (_Min || _MinElemoff)
                            {
                                if (length < myMin)
                                {
                                    myMin = length;
                                    if constexpr (_MinElemoff)
                                        myMinElemoff = elemoff;
                                }
                            }
                            if constexpr (_Max || _MaxElemoff)
                            {
                                if (length > myMax)
                                {
                                    myMax = length;
                                    if constexpr (_MaxElemoff)
                                        myMaxElemoff = elemoff;
                                }
                            }
                        }
                    }
                }
            }
        }
        SYS_FORCE_INLINE void join(const ComputeAttribExtremum& other)
        {
            if constexpr (_Min || _MinElemoff)
            {
                if (other.myMin < myMin)
                {
                    myMin = other.myMin;
                    if constexpr (_MinElemoff)
                        myMinElemoff = other.myMinElemoff;
                }
            }
            if constexpr (_Max || _MaxElemoff)
            {
                if (other.myMax > myMax)
                {
                    myMax = other.myMax;
                    if constexpr (_MaxElemoff)
                        myMaxElemoff = other.myMaxElemoff;
                }
            }
        }
        SYS_FORCE_INLINE value_type getMin() const
        { return myMin; }
        SYS_FORCE_INLINE value_type getMax() const
        { return myMax; }
        SYS_FORCE_INLINE GA_Offset getMinElemoff() const
        { return myMinElemoff; }
        SYS_FORCE_INLINE GA_Offset getMaxElemoff() const
        { return myMaxElemoff; }
    private:
        value_type myMin = gfe::Type::numeric_limits<_Ty>::max();
        value_type myMax = gfe::Type::numeric_limits<_Ty>::lowest();
        GA_Offset myMinElemoff = GFE_INVALID_OFFSET;
        GA_Offset myMaxElemoff = GFE_INVALID_OFFSET;
        const GA_Attribute* const myAttrib;
    }; // End of Class ComputeAttribExtremum



    

    template<typename _Ty>
    typename gfe::TypeTraits::get_value_type_t<_Ty> computeAttribMin(
        const GA_Attribute* const attrib,
        const GA_SplittableRange& splittableRange,
        const exint subscribeRatio = 64,
        const exint minGrainSize   = 1024
    )
    {
        ComputeAttribExtremum<_Ty, true, false> body(attrib);
        UTparallelReduce(splittableRange, body, subscribeRatio, minGrainSize);
        return body.getMin();
    }
    
    template<typename _Ty>
    typename gfe::TypeTraits::get_value_type_t<_Ty> computeAttribMax(
        const GA_Attribute* const attrib,
        const GA_SplittableRange& splittableRange,
        const exint subscribeRatio = 64,
        const exint minGrainSize   = 1024
    )
    {
        ComputeAttribExtremum<_Ty, false, true> body(attrib);
        UTparallelReduce(splittableRange, body, subscribeRatio, minGrainSize);
        return body.getMax();
    }
    
    template<typename _Ty>
    void computeAttribExtremum(
        const GA_Attribute* const attrib,
        const GA_SplittableRange& splittableRange,
        typename gfe::TypeTraits::get_value_type_t<_Ty>& min,
        typename gfe::TypeTraits::get_value_type_t<_Ty>& max,
        const exint subscribeRatio = 64,
        const exint minGrainSize   = 1024
    )
    {
        ComputeAttribExtremum<_Ty, true, true> body(attrib);
        UTparallelReduce(splittableRange, body, subscribeRatio, minGrainSize);
        min = body.getMin();
        min = body.getMax();
    }
    
    template<typename _Ty, bool _Min, bool _Max, bool _MinElemoff = false, bool _MaxElemoff = false>
        //std::enable_if<_Min || _Max || _MinElemoff || _MaxElemoff>>
    static void computeAttribExtremum(
        const GA_Attribute* const attrib,
        const GA_SplittableRange& splittableRange,
        typename gfe::TypeTraits::get_value_type_t<_Ty>& min,
        typename gfe::TypeTraits::get_value_type_t<_Ty>& max,
        GA_Offset& minElemoff,
        GA_Offset& maxElemoff,
        const exint subscribeRatio = 64,
        const exint minGrainSize   = 1024
    )
    {
        ComputeAttribExtremum<_Ty, _Min, _Max, _MinElemoff, _MaxElemoff> body(attrib);
        UTparallelReduce(splittableRange, body, subscribeRatio, minGrainSize);
        min = body.getMin();
        max = body.getMax();
        minElemoff = body.getMinElemoff();
        maxElemoff = body.getMaxElemoff();
    }
    
    template<typename _Ty>
    SYS_FORCE_INLINE typename gfe::TypeTraits::get_value_type_t<_Ty> computeAttribMin(
        const GA_Attribute* const attrib,
        const GA_ElementGroup* const group = nullptr,
        const exint subscribeRatio = 64,
        const exint minGrainSize   = 1024
    )
    {
        return computeAttribMin<_Ty>(attrib, GA_SplittableRange(GA_Range(attrib->getIndexMap(), group)), subscribeRatio, minGrainSize);
    }
    template<typename _Ty>
    SYS_FORCE_INLINE typename gfe::TypeTraits::get_value_type_t<_Ty> computeAttribMax(
        const GA_Attribute* const attrib,
        const GA_ElementGroup* const group = nullptr,
        const exint subscribeRatio = 64,
        const exint minGrainSize   = 1024
    )
    {
        return computeAttribMax<_Ty>(attrib, GA_SplittableRange(GA_Range(attrib->getIndexMap(), group)), subscribeRatio, minGrainSize);
    }

    template<typename _Ty>
    SYS_FORCE_INLINE typename gfe::TypeTraits::get_value_type_t<_Ty> computeAttribExtremum(
        const GA_Attribute* const attrib,
        const GA_ElementGroup* const group = nullptr,
        const exint subscribeRatio = 64,
        const exint minGrainSize   = 1024
    )
    {
        return computeAttribExtremum<_Ty>(attrib, GA_SplittableRange(GA_Range(attrib->getIndexMap(), group)), subscribeRatio, minGrainSize);
    }





    
} // End of namespace Attribute





_GFE_END






#endif
