
#pragma once

#ifndef __GA_FeE_CurveSetExpand2d_h__
#define __GA_FeE_CurveSetExpand2d_h__

#include "GFE/GFE_CurveSetExpand2d.h"

#include "GFE/GFE_GeoFilter.h"



class GFE_CurveSetExpand2d : public GFE_AttribFilter {


public:
    using GFE_AttribFilter::GFE_AttribFilter;


    void
        setComputeParm(
            const exint subscribeRatio = 64,
            const exint minGrainSize   = 64
        )
    {
        setHasComputed();
        this->subscribeRatio = subscribeRatio;
        this->minGrainSize   = minGrainSize;
    }

    
private:

    // can not use in parallel unless for each GA_Detail
    virtual bool
        computeCore() override
    {
        if (groupParser.isEmpty())
            return true;
        
        
        attribPtr = getOutAttribArray()[0];
        // const GA_Storage storage = attribPtr->getAIFTuple()->getStorage(attribPtr);
        switch (attribPtr->getAIFTuple()->getStorage(attribPtr))
        {
        case GA_STORE_INT16:  enumerate<int16>();           break;
        case GA_STORE_INT32:  enumerate<int32>();           break;
        case GA_STORE_INT64:  enumerate<int64>();           break;
        case GA_STORE_REAL16: enumerate<fpreal16>();        break;
        case GA_STORE_REAL32: enumerate<fpreal32>();        break;
        case GA_STORE_REAL64: enumerate<fpreal64>();        break;
        case GA_STORE_STRING: enumerate<UT_StringHolder>(); break;
        default: break;
        }

        return true;
    }


    


public:

private:
    
    exint subscribeRatio = 64;
    exint minGrainSize = 1024;


}; // End of class GFE_CurveSetExpand2d



#endif
