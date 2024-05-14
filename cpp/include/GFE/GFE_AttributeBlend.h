
#pragma once

#ifndef __GFE_AttributeBlend_h__
#define __GFE_AttributeBlend_h__

#include "GFE/GFE_AttributeBlend.h"

#include "GFE/GFE_GeoFilter.h"



#include "GFE/GFE_GroupUnion.h"
#include "GFE/GFE_Math.h"
#include "UFE/UFE_SplittableString.h"

class GFE_AttribBlend : public GFE_AttribCreateFilterWithRef0
{
public:
    using GFE_AttribCreateFilterWithRef0::GFE_AttribCreateFilterWithRef0;
    
    void
        setComputeParm(
            const fpreal blend = 1,
            const exint subscribeRatio = 64,
            const exint minGrainSize   = 1024
        )
    {
        setHasComputed();
        this->blend = blend;
        this->subscribeRatio = subscribeRatio;
        this->minGrainSize   = minGrainSize;
    }
private:

    virtual bool
        computeCore() override
    {
        if (geoRef0 ? getRef0AttribArray().isEmpty() : getInAttribArray().isEmpty())
            return true;
        
        if (groupParser.isEmpty())
            return true;
        
        geoTemp = geoRef0 ? geoRef0 : geo;
        
        const size_t sizeInAttrib  = geoRef0 ? getRef0AttribArray().size() : getInAttribArray().size();
        const size_t sizeOutAttrib = getOutAttribArray().size();

        if (sizeOutAttrib > 0)
        {
            const size_t attribSize = SYSmin(sizeInAttrib, sizeOutAttrib);
            for(size_t i = 0; i < attribSize; i++)
            {
                dstAttribPtr = getOutAttribArray()[i];
                srcAttribPtr = geoRef0 ? getRef0AttribArray()[i] : getInAttribArray()[i];
                
                if (!dstAttribPtr->getAIFTuple() || !srcAttribPtr->getAIFTuple()) continue;
                
                dstStorage = dstAttribPtr->getAIFTuple()->getStorage(dstAttribPtr);
                srcStorage = srcAttribPtr->getAIFTuple()->getStorage(srcAttribPtr);
        
                if (dstStorage == srcStorage)
                {
                    attribBlend();
                }
            }
        }
        
        return true;
    }

    template<typename _VECTOR_T, GA_AttributeOwner dstOwner, GA_AttributeOwner srcOwner>
    void attribBlendVector()
    {
        const GA_IndexMap& indexMap = srcAttribPtr->getIndexMap();
        const GA_ROHandleT<_VECTOR_T> srcAttrib_h(srcAttribPtr);
        //const GA_IndexMap& indexMapSrc = geoRef0->getIndexMap(SrcOwner);

        UTparallelFor(groupParser.getSplittableRange(dstOwner), [this, &srcAttrib_h, &indexMap](const GA_SplittableRange& r)
        {
            GA_PageHandleT<_VECTOR_T, typename _VECTOR_T::value_type, true, true, GA_Attribute, GA_ATINumeric, GA_Detail> dstAttrib_ph(dstAttribPtr);
            GA_Offset start, end;
            for(GA_PageIterator pit = r.beginPages(); !pit.atEnd(); ++pit)
            {
                for(GA_Iterator it(pit.begin()); it.blockAdvance(start, end);)
                {
                    dstAttrib_ph.setPage(start);
                    for(GA_Offset elemoff = start; elemoff < end; elemoff++)
                    {
                        const GA_Offset srcOff = geo->offsetPromote<dstOwner, srcOwner>(elemoff);
                        if (GFE_Type::isValidOffset(indexMap, srcOff))
                        {
                            dstAttrib_ph.value(elemoff) = GFE_Math::lerp(dstAttrib_ph.value(elemoff), srcAttrib_h.get(srcOff), blend);
                        }
                    }
                }
            }
        }, subscribeRatio, minGrainSize);
    }

    
    template<typename FLOAT_T, GA_AttributeOwner dstOwner, GA_AttributeOwner srcOwner>
    void attribBlendScalar()
    {
        const GA_IndexMap& indexMap = srcAttribPtr->getIndexMap();
        const GA_ROHandleT<FLOAT_T> srcAttrib_h(srcAttribPtr);
        //const GA_IndexMap& indexMapSrc = geoRef0->getIndexMap(SrcOwner);

        UTparallelFor(groupParser.getSplittableRange(dstOwner), [this, &srcAttrib_h, &indexMap](const GA_SplittableRange& r)
        {
            GA_PageHandleT<FLOAT_T, FLOAT_T, true, true, GA_Attribute, GA_ATINumeric, GA_Detail> dstAttrib_ph(dstAttribPtr);
            GA_Offset start, end;
            for(GA_PageIterator pit = r.beginPages(); !pit.atEnd(); ++pit)
            {
                for(GA_Iterator it(pit.begin()); it.blockAdvance(start, end);)
                {
                    dstAttrib_ph.setPage(start);
                    for(GA_Offset elemoff = start; elemoff < end; elemoff++)
                    {
                        const GA_Offset srcOff = geo->offsetPromote<dstOwner, srcOwner>(elemoff);
                        if (GFE_Type::isValidOffset(indexMap, srcOff))
                        {
                            dstAttrib_ph.value(elemoff) = GFE_Math::lerp(dstAttrib_ph.value(elemoff), srcAttrib_h.get(srcOff), blend);
                        }
                    }
                }
            }
        }, subscribeRatio, minGrainSize);
    }
    
    
    template<GA_AttributeOwner dstOwner, GA_AttributeOwner srcOwner>
    void attribBlend()
    {
        switch (srcAttribPtr->getTupleSize())
        {
        case 1:
            switch (dstStorage)
            {
                case GA_STORE_INT32:   attribBlendScalar<int32,    dstOwner, srcOwner>(); return; break;
                case GA_STORE_INT64:   attribBlendScalar<int64,    dstOwner, srcOwner>(); return; break;
                case GA_STORE_REAL32:  attribBlendScalar<fpreal32, dstOwner, srcOwner>(); return; break;
                case GA_STORE_REAL64:  attribBlendScalar<fpreal64, dstOwner, srcOwner>(); return; break;
                default: UT_ASSERT_MSG(0, "Unhandled Storage"); break;
            }
        break;
        case 2:
            switch (dstStorage)
            {
                case GA_STORE_INT32:   attribBlendVector<UT_Vector2T<int32>,    dstOwner, srcOwner>(); return; break;
                case GA_STORE_INT64:   attribBlendVector<UT_Vector2T<int64>,    dstOwner, srcOwner>(); return; break;
                case GA_STORE_REAL32:  attribBlendVector<UT_Vector2T<fpreal32>, dstOwner, srcOwner>(); return; break;
                case GA_STORE_REAL64:  attribBlendVector<UT_Vector2T<fpreal64>, dstOwner, srcOwner>(); return; break;
                default: UT_ASSERT_MSG(0, "Unhandled Storage"); break;
            }
        break;
        case 3:
            switch (dstStorage)
            {
                case GA_STORE_INT32:   attribBlendVector<UT_Vector3T<int32>,    dstOwner, srcOwner>(); return; break;
                case GA_STORE_INT64:   attribBlendVector<UT_Vector3T<int64>,    dstOwner, srcOwner>(); return; break;
                case GA_STORE_REAL32:  attribBlendVector<UT_Vector3T<fpreal32>, dstOwner, srcOwner>(); return; break;
                case GA_STORE_REAL64:  attribBlendVector<UT_Vector3T<fpreal64>, dstOwner, srcOwner>(); return; break;
                default: UT_ASSERT_MSG(0, "Unhandled Storage"); break;
            }
        break;
        case 4:
            switch (dstStorage)
            {
                case GA_STORE_INT32:   attribBlendVector<UT_Vector4T<int32>,    dstOwner, srcOwner>(); return; break;
                case GA_STORE_INT64:   attribBlendVector<UT_Vector4T<int64>,    dstOwner, srcOwner>(); return; break;
                case GA_STORE_REAL32:  attribBlendVector<UT_Vector4T<fpreal32>, dstOwner, srcOwner>(); return; break;
                case GA_STORE_REAL64:  attribBlendVector<UT_Vector4T<fpreal64>, dstOwner, srcOwner>(); return; break;
                default: UT_ASSERT_MSG(0, "Unhandled Storage"); break;
            }
        break;
        }
    }
    
    template<GA_AttributeOwner dstOwner>
    void attribBlend()
    {
        switch (srcAttribPtr->getOwner())
        {
        case GA_ATTRIB_POINT:     attribBlend<dstOwner, GA_ATTRIB_POINT>();     return; break;
        case GA_ATTRIB_VERTEX:    attribBlend<dstOwner, GA_ATTRIB_VERTEX>();    return; break;
        case GA_ATTRIB_PRIMITIVE: attribBlend<dstOwner, GA_ATTRIB_PRIMITIVE>(); return; break;
        case GA_ATTRIB_DETAIL:    attribBlend<dstOwner, GA_ATTRIB_DETAIL>();    return; break;
        default: break;
        }
    }

    
    void attribBlend()
    {
        switch (dstAttribPtr->getOwner())
        {
        case GA_ATTRIB_POINT:     attribBlend<GA_ATTRIB_POINT>();     return; break;
        case GA_ATTRIB_VERTEX:    attribBlend<GA_ATTRIB_VERTEX>();    return; break;
        case GA_ATTRIB_PRIMITIVE: attribBlend<GA_ATTRIB_PRIMITIVE>(); return; break;
        case GA_ATTRIB_DETAIL:    attribBlend<GA_ATTRIB_DETAIL>();    return; break;
        default: break;
        }
    }

    
public:
    fpreal blend = 1.0;
    GA_AttributeOwner sourceOwner = GA_ATTRIB_INVALID;
    GA_AttributeOwner destinationOwner = GA_ATTRIB_INVALID;

    
private:
    GA_Storage dstStorage;
    GA_Storage srcStorage;

private:
    const GFE_Detail* geoTemp;
    const GA_Attribute* srcAttribPtr;
    GA_Attribute* dstAttribPtr;

    exint subscribeRatio = 64;
    exint minGrainSize   = 1024;


}; // End of class GFE_AttribBlend






#endif
