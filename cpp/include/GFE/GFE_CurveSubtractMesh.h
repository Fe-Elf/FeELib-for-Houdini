
#pragma once

#ifndef __GFE_CurveSubtractMesh_h__
#define __GFE_CurveSubtractMesh_h__

#include <GU/GU_RayIntersect.h>#include "GFE/GFE_CurveSubtractMesh.h"

#include "GFE/GFE_GeoFilter.h"


#include "GFE/GFE_Intersection.h"

class GFE_CurveSubtractMesh : public GFE_AttribFilter {

public:
    using GFE_AttribFilter::GFE_AttribFilter;
    
    void
        setComputeParm(
            const GA_Size firstIndex   = 0,
            const bool negativeIndex   = false,
            const bool outAsOffset     = true,
            const exint subscribeRatio = 64,
            const exint minGrainSize   = 64
        )
    {
        setHasComputed();
        this->firstIndex     = firstIndex;
        this->negativeIndex  = negativeIndex;
        this->outAsOffset    = outAsOffset;
        this->subscribeRatio = subscribeRatio;
        this->minGrainSize   = minGrainSize;
    }

private:

    virtual bool
        computeCore() override
    {
        if (getOutAttribArray().isEmpty())
            return false;

        if (groupParser.isEmpty())
            return true;


        
        GFE_Intersection intersection
        attribPtr = getOutAttribArray()[0];
        // const GA_Storage storage = attribPtr->getAIFTuple()->getStorage(attribPtr);
        switch (attribPtr->getAIFTuple()->getStorage(attribPtr))
        {
        case GA_STORE_INT16:  curveSubtractMesh<int16>();           break;
        case GA_STORE_INT32:  curveSubtractMesh<int32>();           break;
        case GA_STORE_INT64:  curveSubtractMesh<int64>();           break;
        case GA_STORE_REAL16: curveSubtractMesh<fpreal16>();        break;
        case GA_STORE_REAL32: curveSubtractMesh<fpreal32>();        break;
        case GA_STORE_REAL64: curveSubtractMesh<fpreal64>();        break;
        case GA_STORE_STRING: curveSubtractMesh<UT_StringHolder>(); break;
        default: break;
        }

        return true;
    }


    template<typename T>
    void curveSubtractMesh()
    {

        GU_RayInfo
        UT_ASSERT_P(attribPtr);
        const GA_SplittableRange geoSplittableRange0(groupParser.getRange(attribPtr->getOwner()));
        UTparallelFor(geoSplittableRange0, [this](const GA_SplittableRange& r)
        {
            GA_PageHandleT<T, T, true, true, GA_Attribute, GA_ATINumeric, GA_Detail> attrib_ph(attribPtr);
            for (GA_PageIterator pit = r.beginPages(); !pit.atEnd(); ++pit)
            {
                GA_Offset start, end;
                for (GA_Iterator it(pit.begin()); it.blockAdvance(start, end); )
                {
                    attrib_ph.setPage(start);
                    for (GA_Offset elemoff = start; elemoff < end; ++elemoff)
                    {
                        attrib_ph.value(elemoff) = firstIndex + elemoff;
                    }
                }
            }
        }, subscribeRatio, minGrainSize);
    }




public:
    GA_Size firstIndex = 0;
    bool outAsOffset = true;
    bool negativeIndex = false;
    

private:
    
    GA_Attribute* attrib;
    exint subscribeRatio = 64;
    exint minGrainSize = 1024;


}; // End of class GFE_CurveSubtractMesh





#endif
