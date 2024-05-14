
#pragma once

#ifndef __GFE_SetToDefault_h__
#define __GFE_SetToDefault_h__

//#include "GU_FeE/GFE_SetToDefault.h"

#include "GA/GA_Detail.h"




namespace GFE_SetToDefault {




//GFE_SetToDefault::setToDefault(attrib_h)


#if 1
#define DEFFUNCsetToDefaultF(T)                                                        \
static void setToDefault(                                                              \
    const GA_RWHandleT<T>& attrib_h                                                    \
)                                                                                      \
{                                                                                      \
    const GA_ATINumeric* attrib = attrib_h.getAttribute();                             \
    const GA_AttributeOwner attribOwner = attrib->getOwner();                          \
                                                                                       \
    if (attribOwner == GA_ATTRIB_INVALID)                                              \
        return;                                                                        \
                                                                                       \
    const GA_Defaults defaults = attrib->getDefaults();                                \
    T defaultValue;                                                                    \
    defaults.get(0, defaultValue);                                                     \
    attrib_h.makeConstant(defaultValue);                                               \
}                                                                                      \


DEFFUNCsetToDefaultF(int32)
DEFFUNCsetToDefaultF(int64)
DEFFUNCsetToDefaultF(fpreal32)
DEFFUNCsetToDefaultF(fpreal64)

#else

//GFE_SetToDefault::setToDefault(attrib_h)
template<typename T>                                                      
static void setToDefault(                                                 
    const GA_RWHandleT<T>& attrib_h                                   
)                                                                         
{                                                                         
    const GA_ATINumeric* attrib = attrib_h.getAttribute();            
    const GA_AttributeOwner attribOwner = attrib->getOwner();             
                                                                          
    if (attribOwner == GA_ATTRIB_INVALID)                                 
        return;                                                           
                                                                          
    const GA_Defaults defaults = attrib->getDefaults();                   
    T defaultValue;                                                       
    defaults.get(0, defaultValue);                                        
    attrib_h.makeConstant(defaultValue);                              
}                                                                         
#endif

//static void setToDefault(
//    const GA_RWHandleT<UT_Vector3T<fpreal16>>& attrib_h
//)
//{
//    const GA_ATINumeric* attrib = attrib_h.getAttribute();
//    const GA_AttributeOwner attribOwner = attrib->getOwner();
//
//    if (attribOwner == GA_ATTRIB_INVALID)
//        return;
//
//    const GA_Defaults defaults = attrib->getDefaults();
//    fpreal32 defaultValue[3];
//    defaults.get(0, defaultValue[0]);
//    defaults.get(1, defaultValue[1]);
//    defaults.get(2, defaultValue[2]);
//
//    fpreal16 defaultValue16[3];
//    defaultValue16[0] = defaultValue[0];
//    defaultValue16[1] = defaultValue[1];
//    defaultValue16[2] = defaultValue[2];
//    attrib_h.makeConstantV(defaultValue16, 3);
//}


/*
static void setToDefault(
    const GA_RWHandleT<UT_Vector3T<fpreal32>>& attrib_h
)
{
    const GA_ATINumeric* attrib = attrib_h.getAttribute();
    const GA_AttributeOwner attribOwner = attrib->getOwner();

    if (attribOwner == GA_ATTRIB_INVALID)
        return;

    const GA_Defaults defaults = attrib->getDefaults();
    fpreal32 defaultValue[3];
    defaults.get(0, defaultValue[0]);
    defaults.get(1, defaultValue[1]);
    defaults.get(2, defaultValue[2]);
    attrib_h.makeConstantV(defaultValue, 3);
}

#define DEFFUNCsetToDefaultV3(T)                                                       \
static void setToDefault(                                                              \
    const GA_RWHandleT<UT_Vector3T<T>>& attrib_h                                       \
)                                                                                      \
{                                                                                      \
    const GA_ATINumeric* attrib = attrib_h.getAttribute();                             \
    const GA_AttributeOwner attribOwner = attrib->getOwner();                          \
                                                                                       \
    if (attribOwner == GA_ATTRIB_INVALID)                                              \
        return;                                                                        \
                                                                                       \
    const GA_Defaults defaults = attrib->getDefaults();                                \
                                                                                       \
    T defaultValue[3];                                                                 \
    defaults.get(0, defaultValue[0]);                                                  \
    defaults.get(1, defaultValue[1]);                                                  \
    defaults.get(2, defaultValue[2]);                                                  \
    attrib_h.makeConstantV(defaultValue, 3);                                           \
}                                                                                      

//DEFFUNCsetToDefaultV3(fpreal16)
DEFFUNCsetToDefaultV3(fpreal32)
DEFFUNCsetToDefaultV3(fpreal64)
DEFFUNCsetToDefaultV3(int32)
DEFFUNCsetToDefaultV3(int64)

*/

//GFE_SetToDefault::setToDefault(geo, attrib_h, geoGroup)
//GFE_SetToDefault::setToDefault(geo, attrib_h, geoGroup, subscribeRatio, minGrainSize)

template<typename T>
static void
setToDefault(
    const GA_Detail* const geo,
    const GA_RWHandleT<T>& attrib_h,
    const GA_ElementGroup* const geoGroup = nullptr,
    const int subscribeRatio = 16,
    const int minGrainSize = 1024
)
{
    const GA_ATINumeric* attrib = attrib_h.getAttribute();

    /*const GA_StorageClass storageClass = attrib->getStorageClass();
    const GA_Storage storage = attrib->getStorage();*/
    const GA_AttributeOwner attribOwner = attrib->getOwner();

    if (attribOwner == GA_ATTRIB_INVALID)
        return;

    GA_Defaults defaults = attrib->getDefaults();

    T defaultValue;
    int tupleSize = attrib->getTupleSize();
    for (size_t i = 0; i < tupleSize; i++)
    {
        defaults.get(i, defaultValue(i));
    }

    if (attribOwner == GA_ATTRIB_GLOBAL)
    {
        //attrib_h.set(0, defaults);
        attrib_h.makeConstant(defaultValue);
        return;
    }


    GA_Range geo0Range;
    switch (attribOwner)
    {
    case GA_ATTRIB_VERTEX:
    {
        geo0Range = geo->getVertexRange(static_cast<const GA_VertexGroup*>(geoGroup));
    }
    break;
    case GA_ATTRIB_POINT:
    {
        geo0Range = geo->getPointRange(static_cast<const GA_PointGroup*>(geoGroup));
    }
    break;
    case GA_ATTRIB_PRIMITIVE:
    {
        geo0Range = geo->getPrimitiveRange(static_cast<const GA_PrimitiveGroup*>(geoGroup));
    }
    break;
    default:
        return;
    }
    const GA_SplittableRange geo0SplittableRange(geo0Range);

    //attrib_h.makeConstant(defaultValue);
    UTparallelFor(geo0SplittableRange, [&attrib_h, &defaultValue](const GA_SplittableRange& r)
    {
        GA_Offset start, end;
        for (GA_Iterator it(r); it.blockAdvance(start, end); )
        {
            for (GA_Offset elemoff = start; elemoff < end; ++elemoff)
            {
                attrib_h.set(elemoff, defaultValue);
            }
        }
    }, subscribeRatio, minGrainSize);
}

#if 1

//GFE_SetToDefault::setToDefault(geo, attrib_h, geoGroup)
//GFE_SetToDefault::setToDefault(geo, attrib_h, geoGroup, subscribeRatio, minGrainSize)

#define DEFFUNCsetToDefaultVD(T)                                                                \
template<>                                                                                      \
static void setToDefault<T>(                                                                    \
    const GA_Detail* const geo,                                                                 \
    const GA_RWHandleT<T>& attrib_h,                                                            \
    const GA_ElementGroup* const geoGroup,                                                      \
    const int subscribeRatio,                                                                   \
    const int minGrainSize                                                                      \
)                                                                                               \
{                                                                                               \
    if (!geoGroup)                                                                              \
    {                                                                                           \
        setToDefault(attrib_h);                                                                 \
        return;                                                                                 \
    }                                                                                           \
    return setToDefault(geo, attrib_h, geoGroup, subscribeRatio, minGrainSize);                 \
}                                                                                               \

DEFFUNCsetToDefaultVD(int32)
DEFFUNCsetToDefaultVD(int64)
DEFFUNCsetToDefaultVD(fpreal32)
DEFFUNCsetToDefaultVD(fpreal64)

#else

template<>
static void setToDefault<int32>(
    const GA_Detail* const geo,
    const GA_RWHandleT<int32>& attrib_h,
    const GA_ElementGroup* const geoGroup,
    const int subscribeRatio,
    const int minGrainSize
    )
{
    if (!geoGroup)
    {
        setToDefault(attrib_h);
        return;
    }
    return setToDefault(geo, attrib_h, geoGroup, subscribeRatio, minGrainSize);
}


#endif


//GFE_SetToDefault::setToDefault(attrib_h, geoGroup)
//GFE_SetToDefault::setToDefault(attrib_h, geoGroup, subscribeRatio, minGrainSize)

template<typename T>
static void
setToDefault(
    const GA_RWHandleT<T>& attrib_h,
    const GA_ElementGroup* const geoGroup = nullptr,
    const int subscribeRatio = 16,
    const int minGrainSize = 1024
)
{
    if (!geoGroup)
    {
        setToDefault(attrib_h);
        return;
    }
    const GA_Detail* const geo = attrib_h.getAttribute()->getDetail();
    setToDefault(geo, attrib_h, geoGroup, subscribeRatio, minGrainSize);
}


//GFE_SetToDefault::setToDefault(geo, attrib, geoGroup)
//GFE_SetToDefault::setToDefault(geo, attrib, geoGroup, subscribeRatio, minGrainSize)


static void
setToDefault(
    const GA_Detail* const geo,
    GA_Attribute* const attrib,
    const GA_ElementGroup* const geoGroup = nullptr,
    const int subscribeRatio = 16,
    const int minGrainSize = 1024
)
{
    int typeId = attrib->getType().getTypeId();
    int tupleSize = attrib->getTupleSize();

    GA_RWHandleT<UT_Vector3T<fpreal32>> attrib_h(attrib);
    return GFE_SetToDefault::setToDefault(geo, attrib_h, geoGroup, subscribeRatio, minGrainSize);
}





} // End of namespace GFE_SetToDefault

#endif
