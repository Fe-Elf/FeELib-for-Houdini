
#pragma once

#ifndef __GFE_ExtremePrim_h__
#define __GFE_ExtremePrim_h__

#include "GFE/GFE_ExtremePrim.h"

#include "GFE/GFE_GeoFilter.h"


#include "GFE/GFE_ExtremeElement.h"
#include "GFE/GFE_Measure.h"


class GFE_ExtremePrim : public GFE_AttribFilter {

public:
    //using GFE_AttribFilter::GFE_AttribFilter;

    GFE_ExtremePrim(
        GA_Detail* const geo,
        const SOP_NodeVerb::CookParms* const cookparms = nullptr
    )
        : GFE_AttribFilter(geo, cookparms)
        , extremeElem(geo, nullptr, cookparms);
    {
    }

    
    void
        setComputeParm(
            const GFE_StatisticalFunction statisticalFunction = GFE_StatisticalFunction::Min,
            const GFE_MeasureType measureType = GFE_MeasureType::Area,
            const exint subscribeRatio = 64,
            const exint minGrainSize   = 1024
        )
    {
        setHasComputed();
        this->statisticalFunction = statisticalFunction;
        this->measureType         = measureType;
        this->subscribeRatio = subscribeRatio;
        this->minGrainSize   = minGrainSize;
    }

    SYS_FORCE_INLINE GA_Attribute* setExtremeElemAttrib(const bool detached = false, const UT_StringRef& attribName = "")
    { return extremeElemAttrib = getOutAttribArray().findOrCreateTuple(detached, GA_ATTRIB_DETAIL,
                GA_STORECLASS_INT, GA_STORE_INVALID, attribName); }
    
    SYS_FORCE_INLINE GA_Attribute* setExtremeValueAttrib(const bool detached = false, const UT_StringRef& attribName = "")
    { return extremeValueAttrib = getOutAttribArray().findOrCreateTuple(detached, GA_ATTRIB_DETAIL,
                GA_STORECLASS_INT, GA_STORE_INVALID, attribName); }
    
    SYS_FORCE_INLINE GA_Offset getExtremePrimoff() const
    { return extremeElem.getExtremeElemoff(); }

    // SYS_FORCE_INLINE GA_Attribute* findOrCreateTuple(const bool detached = false, const UT_StringRef& attribName = "")
    // { return getOutAttribArray().findOrCreateTuple(detached, GA_ATTRIB_DETAIL,
    //     GA_STORECLASS_INT, GA_STORE_INVALID, attribName, 1, GA_Defaults(GFE_INVALID_OFFSET)); }
    // 
    // SYS_FORCE_INLINE virtual GA_Group* findOrCreateGroup(const bool detached = false, const UT_StringRef& groupName = "")
    // { return getOutGroupArray().findOrCreate(detached, GA_GROUP_PRIMITIVE, groupName); }



    
private:


    virtual bool
        computeCore() override
    {
        if (groupParser.isEmpty())
            return true;

        GFE_Measure measure(geo, cookparms);
        measure.groupParser.setGroup(groupParser);
        measureAttrib = measure.findOrCreateTuple(false);
        
        measure.measureType = measureType;
        measure.compute();


#if 1
    
        extremeElem.setComputeParm(statisticalFunction, false, subscribeRatio, minGrainSize);
        extremeElem.doDelGroupElement = doDelGroupElement;
        extremeElem.groupSetter.setParm(groupSetter.reverseGroup);
        
        extremeElem.groupParser.setGroup(groupParser);
    
        //extremeElem.measure.measureType = measureType;
    
        // if (sopparms.getUsePieceAttrib())
        // {
        //     extremeElem.setPieceAttrib(GA_ATTRIB_PRIMITIVE, sopparms.getPieceAttrib());
        // }
        
        extremeElem.getInAttribArray().set(measureAttrib);
    
        if (sopparms.getOutExtremeElemGroup())
            extremeElem.findOrCreateGroup(extremeAttribClass, sopparms.getExtremeElemGroupName());
    
        if (sopparms.getOutExtremeElemAttrib())
            extremeElem.setExtremeElemAttrib(false, sopparms.getExtremeElemAttribName());
    
        if (sopparms.getOutExtremeValueAttrib())
            extremeElem.getOutAttribArray().findOrCreateTuple(false, GA_ATTRIB_DETAIL,
                GA_STORECLASS_INT, GA_STORE_INVALID, sopparms.getExtremeValueAttribName());
    
        extremeElem.computeAndBumpDataId();
        extremeElem.delOrVisualizeGroup();

#else
        
        switch (measureAttrib->getAIFTuple()->getStorage(measureAttrib))
        {
        case GA_STORE_INT16:  computeExtremePrimoff<int16>();    break;
        case GA_STORE_INT32:  computeExtremePrimoff<int32>();    break;
        case GA_STORE_INT64:  computeExtremePrimoff<int64>();    break;
        case GA_STORE_REAL16: computeExtremePrimoff<fpreal16>(); break;
        case GA_STORE_REAL32: computeExtremePrimoff<fpreal32>(); break;
        case GA_STORE_REAL64: computeExtremePrimoff<fpreal64>(); break;
        }

        if (!getOutAttribArray().isEmpty())
        {
            GA_Attribute* const outAttrib = getOutAttribArray()[0];
            const GA_AIFTuple* aIFTuple = outAttrib->getAIFTuple();
            if (aIFTuple)
            {
                aIFTuple->set(outAttrib, 0, extremePrimoff);
            }
        }
        
        if (!getOutGroupArray().isEmpty())
        {
            if (getOutGroupArray()[0]->classType() == GA_GROUP_PRIMITIVE)
            {
                static_cast<GA_PrimitiveGroup*>(getOutGroupArray()[0])->setElement(extremePrimoff, true);
            }
        }
#endif
        
        return true;
    }


/*
    
    template<typename FLOAT_T>
    void computeExtremePrimoff()
    {
        ComputeExtremePrimoff<FLOAT_T> body(geo, measureAttrib, statisticalFunction);
        const GA_SplittableRange geoSplittableRange(geo->getPrimitiveRange(groupParser.getPrimitiveGroup()));
        UTparallelReduce(geoSplittableRange, body, subscribeRatio, minGrainSize);
        extremePrimoff = body.getExtremePrimoff();
    }
    
    
    template<typename FLOAT_T>
    class ComputeExtremePrimoff
    {
    public:
        ComputeExtremePrimoff(const GFE_Detail* const a, const GA_ROHandleT<FLOAT_T>& b, const GFE_StatisticalFunction c)
            : myGeo(a)
            , myAttrib_h(b)
            , myStatisticalFunction(c)
            , myExtremePrimoff(GA_INVALID_OFFSET)
            , mtExtremeValue(myStatisticalFunction == GFE_StatisticalFunction::Max ? SYS_FP64_MIN : SYS_FP64_MAX)
        {}
        ComputeExtremePrimoff(ComputeExtremePrimoff& src, UT_Split)
            : myGeo(src.myGeo)
            , myAttrib_h(src.myAttrib_h)
            , myStatisticalFunction(src.myStatisticalFunction)
            , myExtremePrimoff(GA_INVALID_OFFSET)
            , mtExtremeValue(myStatisticalFunction == GFE_StatisticalFunction::Max ? SYS_FP64_MIN : SYS_FP64_MAX)
        {}
        void operator()(const GA_SplittableRange& r)
        {
            GA_PageHandleT<FLOAT_T, FLOAT_T, true, false, const GA_Attribute, const GA_ATINumeric, const GA_Detail> attrib_ph(myAttrib_h.getAttribute());
            for (GA_PageIterator pit = r.beginPages(); !pit.atEnd(); ++pit)
            {
                GA_Offset start, end;
                for (GA_Iterator it(pit.begin()); it.blockAdvance(start, end); )
                {
                    attrib_ph.setPage(start);
                    for (GA_Offset elemoff = start; elemoff < end; ++elemoff)
                    {
                        switch (myStatisticalFunction)
                        {
                        case GFE_StatisticalFunction::Min:
                            if (attrib_ph.value(elemoff) < mtExtremeValue) 
                                mtExtremeValue = attrib_ph.value(elemoff);
                            break;
                        case GFE_StatisticalFunction::Max:
                            if (attrib_ph.value(elemoff) > mtExtremeValue)
                                mtExtremeValue = attrib_ph.value(elemoff);
                            break;
                        default: break;
                        }
                    }
                }
            }
        }
        void join(const ComputeExtremePrimoff& other)
        {
            myExtremePrimoff = other.myExtremePrimoff;
            mtExtremeValue = other.mtExtremeValue;
        }
        SYS_FORCE_INLINE GA_Offset getExtremePrimoff() const
        { return myExtremePrimoff; }
        
    private:
        const GFE_Detail* const myGeo;
        const GA_ROHandleT<FLOAT_T> myAttrib_h;
        GFE_StatisticalFunction myStatisticalFunction;
        GA_Offset myExtremePrimoff;
        fpreal mtExtremeValue;
    }; // End of Class ComputeExtremePrimoff
*/

public:
    
    GFE_StatisticalFunction statisticalFunction = GFE_StatisticalFunction::Min;
    GFE_MeasureType measureType = GFE_MeasureType::Area;


public:
    GFE_ExtremeElement extremeElem;
    
private:
    //GA_Offset extremePrimoff = GFE_INVALID_OFFSET;
    //GA_Attribute* measureAttrib;
    
    exint subscribeRatio = 64;
    exint minGrainSize = 1024;
    
}; // End of Class GFE_ExtremePrim






#endif
