
#pragma once

#ifndef __GFE_RestPy_h__
#define __GFE_RestPy_h__

//#include "GFE/GFE_RestPy.h"

#include "GA/GA_Detail.h"


#include "GFE/GFE_GeoFilter.h"



class GFE_RestPy : public GFE_AttribCreateFilter {

public:
    using GFE_AttribCreateFilter::GFE_AttribCreateFilter;

    

    void
        setComputeParm(
            const int vecComp = 0,
            const bool setConstant = true,
            //const typename VECTOR_T::value_type constantValue = 0,
            const fpreal constantValue = 0,
            const exint subscribeRatio = 64,
            const exint minGrainSize = 1024
        )
    {
        setHasComputed();
        setInAttribBumpDataId(setConstant);
        this->vecComp = vecComp;
        this->setConstant = setConstant;
        this->constantValue = constantValue;
        this->subscribeRatio = subscribeRatio;
        this->minGrainSize = minGrainSize;
    }

    void
        findOrCreateOutAttrib(
            const bool detached = false,
            const GA_AttributeOwner owner = GA_ATTRIB_POINT,
            const GA_Storage storage = GA_STORE_INVALID,
            const UT_StringHolder& attribName = "__topo_"
        )
    {
        getOutAttribArray().findOrCreateTuple(detached, owner, GA_STORECLASS_FLOAT, storage, attribName);
    }

private:

    virtual bool
        computeCore() override
    {
        if (groupParser.isEmpty())
            return true;

        const size_t len = getOutAttribArray().size();
        for (size_t i = 0; i < len; i++)
        {
            GA_Attribute* const outAttribPtr = getOutAttribArray()[i];
            GA_Attribute* const inAttribPtr = getInAttribArray()[i];
            restPy(outAttribPtr, inAttribPtr);
        }
        return true;
    }


    template<typename VECTOR_T>
    void
        restPy(
            const GA_RWHandleT<typename VECTOR_T::value_type>& rest_h,
            const GA_ROHandleT<VECTOR_T>& vec_h
        )
    {
        using value_type = typename VECTOR_T::value_type;
        UT_ASSERT_P(vec_h.getAttribute());
        UT_ASSERT_P(rest_h.getAttribute());

        const GA_SplittableRange geoSplittableRange(groupParser.getRange(vec_h.getAttribute()->getOwner()));
        //const GA_SplittableRange geoSplittableRange(GFE_Range::getSplittableRangeByAnyGroup(geo, geoGroup, vec_h.getAttribute()->getOwner()));
        UTparallelFor(geoSplittableRange, [&vec_h, &rest_h, this](const GA_SplittableRange& r)
        {
            //VECTOR_T vec;
            GA_PageHandleT<VECTOR_T, value_type, true, false, const GA_Attribute, const GA_ATINumeric, const GA_Detail> vec_ph(vec_h.getAttribute());
            GA_PageHandleT<value_type, value_type, true, true, GA_Attribute, GA_ATINumeric, GA_Detail> rest_ph(rest_h.getAttribute());
            for (GA_PageIterator pit = r.beginPages(); !pit.atEnd(); ++pit)
            {
                GA_Offset start, end;
                for (GA_Iterator it(pit.begin()); it.blockAdvance(start, end); )
                {
                    vec_ph.setPage(start);
                    rest_ph.setPage(start);
                    for (GA_Offset elemoff = start; elemoff < end; ++elemoff)
                    {
                        rest_ph.value(elemoff) = vec_ph.value(elemoff)[vecComp];
                    }
                }
            }
        }, subscribeRatio, minGrainSize);
    }

    template<typename VECTOR_T>
    void
        restPySetConstant(
            const GA_RWHandleT<typename VECTOR_T::value_type>& rest_h,
            const GA_RWHandleT<VECTOR_T>& vec_h
        )
    {
        using value_type = typename VECTOR_T::value_type;
        UT_ASSERT_P(vec_h.getAttribute());
        UT_ASSERT_P(rest_h.getAttribute());

        const GA_SplittableRange geoSplittableRange(groupParser.getRange(vec_h.getAttribute()->getOwner()));
        //const GA_SplittableRange geoSplittableRange(GFE_Range::getSplittableRangeByAnyGroup(geo, geoGroup, vec_h.getAttribute()->getOwner()));
        UTparallelFor(geoSplittableRange, [&vec_h, &rest_h, this](const GA_SplittableRange& r)
        {
            //VECTOR_T vec;
            GA_PageHandleT<VECTOR_T, value_type, true, true, GA_Attribute, GA_ATINumeric, GA_Detail> vec_ph(vec_h.getAttribute());
            GA_PageHandleT<value_type, value_type, true, true, GA_Attribute, GA_ATINumeric, GA_Detail> rest_ph(rest_h.getAttribute());
            for (GA_PageIterator pit = r.beginPages(); !pit.atEnd(); ++pit)
            {
                GA_Offset start, end;
                for (GA_Iterator it(pit.begin()); it.blockAdvance(start, end); )
                {
                    vec_ph.setPage(start);
                    rest_ph.setPage(start);
                    for (GA_Offset elemoff = start; elemoff < end; ++elemoff)
                    {
                        rest_ph.value(elemoff) = vec_ph.value(elemoff)[vecComp];
                        vec_ph.value(elemoff)[vecComp] = constantValue;
                    }
                }
            }
        }, subscribeRatio, minGrainSize);
    }

    bool
        restPy(
            GA_Attribute* const restAttribPtr,
            GA_Attribute* const vecAttribPtr
        )
    {
        UT_ASSERT_P(vecAttribPtr);
        UT_ASSERT_P(restAttribPtr);
        if (setConstant)
        {
            switch (vecAttribPtr->getTupleSize())
            {
            case 1:
                return false;
                break;
            case 2:
                switch (vecAttribPtr->getAIFTuple()->getStorage(vecAttribPtr))
                {
                //case GA_STORE_INT16:
                //    restPySetConstant<UT_Vector2T<int16>>(restAttribPtr, vecAttribPtr);
                //    break;
                case GA_STORE_INT32:
                    restPySetConstant<UT_Vector2T<int32>>(restAttribPtr, vecAttribPtr);
                    break;
                case GA_STORE_INT64:
                    restPySetConstant<UT_Vector2T<int64>>(restAttribPtr, vecAttribPtr);
                    break;
                case GA_STORE_REAL16:
                    restPySetConstant<UT_Vector2T<fpreal16>>(restAttribPtr, vecAttribPtr);
                    break;
                case GA_STORE_REAL32:
                    restPySetConstant<UT_Vector2T<fpreal32>>(restAttribPtr, vecAttribPtr);
                    break;
                case GA_STORE_REAL64:
                    restPySetConstant<UT_Vector2T<fpreal64>>(restAttribPtr, vecAttribPtr);
                    break;
                default:
                    return false;
                    break;
                }
                break;
            case 3:
                switch (vecAttribPtr->getAIFTuple()->getStorage(vecAttribPtr))
                {
                //case GA_STORE_INT16:
                //    restPySetConstant<UT_Vector3T<int16>>(restAttribPtr, vecAttribPtr);
                //    break;
                case GA_STORE_INT32:
                    restPySetConstant<UT_Vector3T<int32>>(restAttribPtr, vecAttribPtr);
                    break;
                case GA_STORE_INT64:
                    restPySetConstant<UT_Vector3T<int64>>(restAttribPtr, vecAttribPtr);
                    break;
                case GA_STORE_REAL16:
                    restPySetConstant<UT_Vector3T<fpreal16>>(restAttribPtr, vecAttribPtr);
                    break;
                case GA_STORE_REAL32:
                    restPySetConstant<UT_Vector3T<fpreal32>>(restAttribPtr, vecAttribPtr);
                    break;
                case GA_STORE_REAL64:
                    restPySetConstant<UT_Vector3T<fpreal64>>(restAttribPtr, vecAttribPtr);
                    break;
                default:
                    return false;
                    break;
                }
                break;
            case 4:
                switch (vecAttribPtr->getAIFTuple()->getStorage(vecAttribPtr))
                {
                //case GA_STORE_INT16:
                //    restPySetConstant<UT_Vector4T<int16>>(restAttribPtr, vecAttribPtr);
                //    break;
                case GA_STORE_INT32:
                    restPySetConstant<UT_Vector4T<int32>>(restAttribPtr, vecAttribPtr);
                    break;
                case GA_STORE_INT64:
                    restPySetConstant<UT_Vector4T<int64>>(restAttribPtr, vecAttribPtr);
                    break;
                case GA_STORE_REAL16:
                    restPySetConstant<UT_Vector4T<fpreal16>>(restAttribPtr, vecAttribPtr);
                    break;
                case GA_STORE_REAL32:
                    restPySetConstant<UT_Vector4T<fpreal32>>(restAttribPtr, vecAttribPtr);
                    break;
                    //case GA_STORE_REAL64:
                    //    restPySetConstant<UT_Vector4T<fpreal64>(restAttribPtr, vecAttribPtr);
                    //    break;
                default:
                    return false;
                    break;
                }
                break;
            default:
                return false;
                break;
            }
        }
        else
        {
            switch (vecAttribPtr->getTupleSize())
            {
            case 1:
                return false;
                break;
            case 2:
                switch (vecAttribPtr->getAIFTuple()->getStorage(vecAttribPtr))
                {
                //case GA_STORE_INT16:
                //    restPy<UT_Vector2T<int16>>(restAttribPtr, vecAttribPtr);
                //    break;
                case GA_STORE_INT32:
                    restPy<UT_Vector2T<int32>>(restAttribPtr, vecAttribPtr);
                    break;
                case GA_STORE_INT64:
                    restPy<UT_Vector2T<int64>>(restAttribPtr, vecAttribPtr);
                    break;
                case GA_STORE_REAL16:
                    restPy<UT_Vector2T<fpreal16>>(restAttribPtr, vecAttribPtr);
                    break;
                case GA_STORE_REAL32:
                    restPy<UT_Vector2T<fpreal32>>(restAttribPtr, vecAttribPtr);
                    break;
                case GA_STORE_REAL64:
                    restPy<UT_Vector2T<fpreal64>>(restAttribPtr, vecAttribPtr);
                    break;
                default:
                    return false;
                    break;
                }
                break;
            case 3:
                switch (vecAttribPtr->getAIFTuple()->getStorage(vecAttribPtr))
                {
                //case GA_STORE_INT16:
                //    restPy<UT_Vector3T<int16>>(restAttribPtr, vecAttribPtr);
                //    break;
                case GA_STORE_INT32:
                    restPy<UT_Vector3T<int32>>(restAttribPtr, vecAttribPtr);
                    break;
                case GA_STORE_INT64:
                    restPy<UT_Vector3T<int64>>(restAttribPtr, vecAttribPtr);
                    break;
                case GA_STORE_REAL16:
                    restPy<UT_Vector3T<fpreal16>>(restAttribPtr, vecAttribPtr);
                    break;
                case GA_STORE_REAL32:
                    restPy<UT_Vector3T<fpreal32>>(restAttribPtr, vecAttribPtr);
                    break;
                case GA_STORE_REAL64:
                    restPy<UT_Vector3T<fpreal64>>(restAttribPtr, vecAttribPtr);
                    break;
                default:
                    return false;
                    break;
                }
                break;
            case 4:
                switch (vecAttribPtr->getAIFTuple()->getStorage(vecAttribPtr))
                {
                //case GA_STORE_INT16:
                //    restPy<UT_Vector4T<int16>>(restAttribPtr, vecAttribPtr);
                //    break;
                case GA_STORE_INT32:
                    restPy<UT_Vector4T<int32>>(restAttribPtr, vecAttribPtr);
                    break;
                case GA_STORE_INT64:
                    restPy<UT_Vector4T<int64>>(restAttribPtr, vecAttribPtr);
                    break;
                case GA_STORE_REAL16:
                    restPy<UT_Vector4T<fpreal16>>(restAttribPtr, vecAttribPtr);
                    break;
                case GA_STORE_REAL32:
                    restPy<UT_Vector4T<fpreal32>>(restAttribPtr, vecAttribPtr);
                    break;
                    //case GA_STORE_REAL64:
                    //    restPy<UT_Vector4T<fpreal64>(restAttribPtr, vecAttribPtr);
                    //    break;
                default:
                    return false;
                    break;
                }
                break;
            default:
                return false;
                break;
            }
        }
        return true;
    }

public:
    int vecComp = 0;
    bool setConstant = true;
    //typename VECTOR_T::value_type constantValue = 0;
    fpreal constantValue = 0;

protected:
private:
    exint subscribeRatio = 64;
    exint minGrainSize = 1024;
};































//
//
//
//namespace GFE_RestPy_Namespace {
//
//
//
//
//
//template<typename VECTOR_T>
//static void
//    restPy(
//        const GA_Detail* const geo,
//        const GA_RWHandleT<VECTOR_T>& vec_h,
//        const GA_RWHandleT<typename VECTOR_T::value_type>& rest_h,
//        const GA_Group* const geoGroup,
//        const int vecComp,
//        const bool setConstant,
//        const typename VECTOR_T::value_type constantValue,
//        const exint subscribeRatio = 64,
//        const exint minGrainSize = 1024
//    )
//{
//    UT_ASSERT_P(vec_h.getAttribute());
//    UT_ASSERT_P(rest_h.getAttribute());
//
//    if (geoGroup && geoGroup->isEmpty())
//        return;
//    
//    const GA_SplittableRange geoSplittableRange(GFE_Range::getSplittableRangeByAnyGroup(geo, geoGroup, vec_h.getAttribute()->getOwner()));
//    UTparallelFor(geoSplittableRange, [&vec_h, &rest_h, vecComp, setConstant, constantValue](const GA_SplittableRange& r)
//    {
//        //VECTOR_T vec;
//        GA_PageHandleT<VECTOR_T, typename VECTOR_T::value_type, true, true, GA_Attribute, GA_ATINumeric, GA_Detail> vec_ph(vec_h.getAttribute());
//        GA_PageHandleT<typename VECTOR_T::value_type, typename VECTOR_T::value_type, true, true, GA_Attribute, GA_ATINumeric, GA_Detail> rest_ph(rest_h.getAttribute());
//        for (GA_PageIterator pit = r.beginPages(); !pit.atEnd(); ++pit)
//        {
//            GA_Offset start, end;
//            for (GA_Iterator it(pit.begin()); it.blockAdvance(start, end); )
//            {
//                vec_ph.setPage(start);
//                rest_ph.setPage(start);
//                for (GA_Offset elemoff = start; elemoff < end; ++elemoff)
//                {
//                    rest_ph.value(elemoff) = vec_ph.value(elemoff)[vecComp];
//                    if (setConstant)
//                    {
//                        vec_ph.value(elemoff)[vecComp] = constantValue;
//                    }
//                }
//            }
//        }
//    }, subscribeRatio, minGrainSize);
//
//}
//
//
//
//template<typename T>
//static bool
//restPy(
//    const GA_Detail* const geo,
//    GA_Attribute* const vecAttribPtr,
//    GA_Attribute* const restAttribPtr,
//    const GA_Group* const geoGroup,
//    const int vecComp,
//    const bool setConstant,
//    const T constantValue,
//    const exint subscribeRatio = 64,
//    const exint minGrainSize = 1024
//)
//{
//    UT_ASSERT_P(vecAttribPtr);
//    UT_ASSERT_P(restAttribPtr);
//
//    switch (vecAttribPtr->getTupleSize())
//    {
//    case 1:
//        return false;
//        break;
//    case 2:
//        switch (vecAttribPtr->getAIFTuple()->getStorage(vecAttribPtr))
//        {
//        case GA_STORE_INT16:
//            restPy(geo, GA_RWHandleT<UT_Vector2T<int16>>(vecAttribPtr), restAttribPtr, geoGroup,
//                vecComp, setConstant, constantValue,
//                subscribeRatio, minGrainSize);
//            break;
//        case GA_STORE_INT32:
//            restPy(geo, GA_RWHandleT<UT_Vector2T<int32>>(vecAttribPtr), restAttribPtr, geoGroup,
//                vecComp, setConstant, constantValue,
//                subscribeRatio, minGrainSize);
//            break;
//        case GA_STORE_INT64:
//            restPy(geo, GA_RWHandleT<UT_Vector2T<int64>>(vecAttribPtr), restAttribPtr, geoGroup,
//                vecComp, setConstant, constantValue,
//                subscribeRatio, minGrainSize);
//            break;
//        case GA_STORE_REAL16:
//            restPy(geo, GA_RWHandleT<UT_Vector2T<fpreal16>>(vecAttribPtr), restAttribPtr, geoGroup,
//                vecComp, setConstant, constantValue,
//                subscribeRatio, minGrainSize);
//            break;
//        case GA_STORE_REAL32:
//            restPy(geo, GA_RWHandleT<UT_Vector2T<fpreal32>>(vecAttribPtr), restAttribPtr, geoGroup,
//                vecComp, setConstant, constantValue,
//                subscribeRatio, minGrainSize);
//            break;
//        case GA_STORE_REAL64:
//            restPy(geo, GA_RWHandleT<UT_Vector2T<fpreal64>>(vecAttribPtr), restAttribPtr, geoGroup,
//                vecComp, setConstant, constantValue,
//                subscribeRatio, minGrainSize);
//            break;
//        default:
//            return false;
//            break;
//        }
//        break;
//    case 3:
//        switch (vecAttribPtr->getAIFTuple()->getStorage(vecAttribPtr))
//        {
//        case GA_STORE_INT16:
//            restPy(geo, GA_RWHandleT<UT_Vector3T<int16>>(vecAttribPtr), restAttribPtr, geoGroup,
//                vecComp, setConstant, constantValue,
//                subscribeRatio, minGrainSize);
//            break;
//        case GA_STORE_INT32:
//            restPy(geo, GA_RWHandleT<UT_Vector3T<int32>>(vecAttribPtr), restAttribPtr, geoGroup,
//                vecComp, setConstant, constantValue,
//                subscribeRatio, minGrainSize);
//            break;
//        case GA_STORE_INT64:
//            restPy(geo, GA_RWHandleT<UT_Vector3T<int64>>(vecAttribPtr), restAttribPtr, geoGroup,
//                vecComp, setConstant, constantValue,
//                subscribeRatio, minGrainSize);
//            break;
//        case GA_STORE_REAL16:
//            restPy(geo, GA_RWHandleT<UT_Vector3T<fpreal16>>(vecAttribPtr), restAttribPtr, geoGroup,
//                vecComp, setConstant, constantValue,
//                subscribeRatio, minGrainSize);
//            break;
//        case GA_STORE_REAL32:
//            restPy(geo, GA_RWHandleT<UT_Vector3T<fpreal32>>(vecAttribPtr), restAttribPtr, geoGroup,
//                vecComp, setConstant, constantValue,
//                subscribeRatio, minGrainSize);
//            break;
//        case GA_STORE_REAL64:
//            restPy(geo, GA_RWHandleT<UT_Vector3T<fpreal64>>(vecAttribPtr), restAttribPtr, geoGroup,
//                vecComp, setConstant, constantValue,
//                subscribeRatio, minGrainSize);
//            break;
//        default:
//            return false;
//            break;
//        }
//        break;
//    case 4:
//        switch (vecAttribPtr->getAIFTuple()->getStorage(vecAttribPtr))
//        {
//        case GA_STORE_INT16:
//            restPy(geo, GA_RWHandleT<UT_Vector4T<int16>>(vecAttribPtr), restAttribPtr, geoGroup,
//                vecComp, setConstant, constantValue,
//                subscribeRatio, minGrainSize);
//            break;
//        case GA_STORE_INT32:
//            restPy(geo, GA_RWHandleT<UT_Vector4T<int32>>(vecAttribPtr), restAttribPtr, geoGroup,
//                vecComp, setConstant, constantValue,
//                subscribeRatio, minGrainSize);
//            break;
//        case GA_STORE_INT64:
//            restPy(geo, GA_RWHandleT<UT_Vector4T<int64>>(vecAttribPtr), restAttribPtr, geoGroup,
//                vecComp, setConstant, constantValue,
//                subscribeRatio, minGrainSize);
//            break;
//        case GA_STORE_REAL16:
//            restPy(geo, GA_RWHandleT<UT_Vector4T<fpreal16>>(vecAttribPtr), restAttribPtr, geoGroup,
//                vecComp, setConstant, constantValue,
//                subscribeRatio, minGrainSize);
//            break;
//        case GA_STORE_REAL32:
//            restPy(geo, GA_RWHandleT<UT_Vector4T<fpreal32>>(vecAttribPtr), restAttribPtr, geoGroup,
//                vecComp, setConstant, constantValue,
//                subscribeRatio, minGrainSize);
//            break;
//        case GA_STORE_REAL64:
//            restPy(geo, GA_RWHandleT<UT_Vector4T<fpreal64>>(vecAttribPtr), restAttribPtr, geoGroup,
//                vecComp, setConstant, constantValue,
//                subscribeRatio, minGrainSize);
//            break;
//        default:
//            return false;
//            break;
//        }
//        break;
//    default:
//        return false;
//        break;
//    }
//
//}
//
//template<typename T>
//static GA_Attribute*
//restPy(
//    GA_Detail* const geo,
//    GA_Attribute* const vecAttribPtr,
//    const GA_Group* const geoGroup,
//    const GA_Storage storage,
//    const UT_StringHolder& restAttribName,
//    const int vecComp,
//    const bool setConstant,
//    const T constantValue,
//    const exint subscribeRatio = 64,
//    const exint minGrainSize = 1024
//)
//{
//    UT_ASSERT_P(vecAttribPtr);
//
//    const GA_AttributeOwner vecAttribClass = vecAttribPtr->getOwner();
//
//    GA_Attribute* restAttribPtr = geo->findAttribute(vecAttribClass, restAttribName);
//    if (restAttribPtr && restAttribPtr->getAIFTuple()->getStorage(restAttribPtr) != vecAttribPtr->getAIFTuple()->getStorage(vecAttribPtr))
//    {
//        geo->getAttributes().destroyAttribute(restAttribPtr);
//        restAttribPtr = nullptr;
//    }
//    if (!restAttribPtr)
//    {
//        GA_Storage finalStorage;
//        if (storage == GA_STORE_INVALID)
//        {
//            if (vecAttribPtr->getStorageClass() == GA_STORECLASS_INT)
//            {
//                finalStorage = GFE_Type::getPreferredStorageI(geo);
//            }
//            else
//            {
//                finalStorage = GFE_Type::getPreferredStorageF(geo);
//            }
//        }
//        else
//        {
//            finalStorage = storage;
//        }
//        restAttribPtr = geo->getAttributes().createTupleAttribute(vecAttribClass, restAttribName, finalStorage, 1, GA_Defaults(0));
//    }
//
//    UT_ASSERT_P(restAttribPtr);
//
//    restPy(geo, vecAttribPtr, restAttribPtr, geoGroup,
//        vecComp, setConstant, constantValue,
//        subscribeRatio, minGrainSize);
//
//    return restAttribPtr;
//}
//
//
//
//
//template<typename T>
//static GA_Attribute*
//    restPy(
//        GA_Detail* const geo,
//        const GA_Group* const geoGroup,
//        const GA_AttributeOwner vecAttribClass,
//        const UT_StringHolder& vecAttribName,
//        const GA_Storage storage,
//        const UT_StringHolder& restAttribName,
//        const int vecComp,
//        const bool setConstant,
//        const T constantValue,
//        const exint subscribeRatio = 64,
//        const exint minGrainSize = 1024
//    )
//{
//    GA_Attribute* vecAttribPtr = geo->findAttribute(vecAttribClass, restAttribName);
//
//    if (!vecAttribPtr)
//    {
//        return nullptr;
//    }
//    return restPy(geo, geoGroup, vecAttribPtr, storage, restAttribName,
//        vecComp, setConstant, constantValue,
//        subscribeRatio, minGrainSize);
//}
//
//
//
//
//
//template<typename T>
//static GA_Attribute*
//restPy(
//    const SOP_NodeVerb::CookParms& cookparms,
//    GA_Detail* const geo,
//    const GA_GroupType groupType,
//    const UT_StringHolder& groupName,
//    const GA_AttributeOwner vecAttribClass,
//    const UT_StringHolder& vecAttribName,
//    const GA_Storage storage,
//    const UT_StringHolder& restAttribName,
//    const int vecComp,
//    const bool setConstant,
//    const T constantValue,
//    const exint subscribeRatio = 64,
//    const exint minGrainSize = 1024
//)
//{
//    GOP_Manager gop;
//    const GA_Group* const geoGroup = GFE_GroupParser_Namespace::findOrParseGroupDetached(cookparms, geo, groupType, groupName, gop);
//
//    return restPy<T>(geo, geoGroup,
//        vecAttribClass, vecAttribName, storage, restAttribName,
//        vecComp, setConstant, constantValue,
//        subscribeRatio, minGrainSize);
//}
//
//
//





//} // End of namespace GFE_RestPy




#endif
