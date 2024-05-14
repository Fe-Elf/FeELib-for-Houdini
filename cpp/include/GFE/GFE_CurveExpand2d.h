
#pragma once

#ifndef __GFE_CurveExpand2d_h__
#define __GFE_CurveExpand2d_h__

#include "GFE/GFE_CurveExpand2d.h"

#include "GFE/GFE_GeoFilter.h"




enum class GFE_CurveExpand2dType
{
    GFE_CurveExpand2dType_AUTO,
    GFE_CurveExpand2dType_OPEN,
    GFE_CurveExpand2dType_CLOSE,
};



class GFE_CurveExpand2d : public GFE_AttribFilter {

public:
    using GFE_AttribFilter::GFE_AttribFilter;

    ~GFE_CurveExpand2d()
    {
    }


    void
        setComputeParm(
            const exint subscribeRatio = 64,
            const exint minGrainSize = 64
        )
    {
        setHasComputed();
        this->subscribeRatio = subscribeRatio;
        this->minGrainSize = minGrainSize;
    }


private:

    // can not use in parallel unless for each GA_Detail
    virtual bool
        computeCore() override
    {
        if (groupParser.isEmpty())
            return true;

        if (!posAttrib)
            posAttrib = geo->getP();
        
        switch (posAttrib->getAIFTuple()->getStorage(posAttrib))
        {
        case GA_STORE_REAL16: curveExpand2d<UT_Vector3T<fpreal16>>(); break;
        case GA_STORE_REAL32: curveExpand2d<UT_Vector3T<fpreal32>>(); break;
        case GA_STORE_REAL64: curveExpand2d<UT_Vector3T<fpreal64>>(); break;
        default: break;
        }

        return true;
    }



    template<typename VECTOR_T>
    void curveExpand2d()
    {
    }


    

public:


private:
    exint subscribeRatio = 64;
    exint minGrainSize = 64;


}; // End of class GFE_CurveExpand2d




#endif
