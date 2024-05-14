
#pragma once

#ifndef __GFE_AttributeNormalize_h__
#define __GFE_AttributeNormalize_h__

#include "GFE/GFE_AttributeNormalize.h"

#include "GFE/GFE_GeoFilter.h"

#include "GFE/GFE_AttributeExtremum.h"
#include "GFE/GFE_AttributeRemap.h"


class GFE_AttribNormalize : public GFE_AttribFilter {


public:

    using GFE_AttribFilter::GFE_AttribFilter;


    ~GFE_AttribNormalize()
    {
    }


void
    setComputeParm(
        const bool doNormalize = true,
        const fpreal64 uniScale = 1.0,
        const exint subscribeRatio = 64,
        const exint minGrainSize   = 1024
    )
{
    setHasComputed();
    this->doNormalize = doNormalize;
    this->uniScale = uniScale;
    this->subscribeRatio = subscribeRatio;
    this->minGrainSize = minGrainSize;
}




private:

    virtual bool
        computeCore() override
    {
        if (!getOutAttribArray().size())
            return false;

        if (!doNormalize && uniScale==1.0)
            return true;
        
        if (groupParser.isEmpty())
            return true;

        
        attrib = getOutAttribArray()[0];

        
        GFE_AttribExtremum attribExtremum(geo, nullptr, cookparms);
        attribExtremum.outAttribMin = true;
        attribExtremum.outAttribMax = true;
        attribExtremum.outAttribMinElemnum = true;
        attribExtremum.outAttribMaxElemnum = true;
        attribExtremum.outAsOffset = true;
        attribExtremum.setAttrib(attrib);
        attribExtremum.groupParser.setGroup(groupParser);
        attribExtremum.compute();

        
        return true;
    }


    template<typename _Ty, bool doScale>
    void attribNormalize()
    {
        using value_type = GFE_Type::get_value_type_t<_Ty>;
        
        const value_type uniScaleTmp = value_type(uniScale);
        UTparallelFor(groupParser.getSplittableRange(attrib), [this, uniScaleTmp](const GA_SplittableRange& r)
        {
            GFE_RWPageHandleT<_Ty> attrib_ph(attrib);
            for (GA_PageIterator pit = r.beginPages(); !pit.atEnd(); ++pit)
            {
                GA_Offset start, end;
                for (GA_Iterator it(pit.begin()); it.blockAdvance(start, end); )
                {
                    attrib_ph.setPage(start);
                    for (GA_Offset elemoff = start; elemoff < end; ++elemoff)
                    {
                        if constexpr (doScale)
                            attrib_ph.value(elemoff).normalize();
                        attrib_ph.value(elemoff) *= uniScaleTmp;
                    }
                }
            }
        }, subscribeRatio, minGrainSize);
    }



public:
    bool doNormalize = true;
    fpreal64 uniScale = 1;

private:
    GA_Attribute* attrib;

    
    exint subscribeRatio = 64;
    exint minGrainSize = 64;

    
};








#endif





