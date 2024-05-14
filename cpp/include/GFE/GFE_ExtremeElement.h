
#pragma once

#ifndef __GFE_ExtremeElement_h__
#define __GFE_ExtremeElement_h__

#include "GFE/GFE_ExtremeElement.h"



#include "GFE/GFE_GeoFilter.h"


#include "GFE/GFE_Measure.h"


class GFE_ExtremeElement : public GFE_AttribCreateFilterWithRef0 {

public:
    //using GFE_AttribCreateFilterWithRef0::GFE_AttribCreateFilterWithRef0;
    
    GFE_ExtremeElement(
        GA_Detail& geo,
        const GA_Detail* const geoRef0,
        const SOP_NodeVerb::CookParms* const cookparms = nullptr
    )
        : GFE_AttribCreateFilterWithRef0(geo, geoRef0, cookparms)
        , measure(geo, cookparms)
    {
    }
    
    void
        setComputeParm(
            const GFE_StatisticalFunction statisticalFunction = GFE_StatisticalFunction::Min,
            const bool delExtremeAttrib = false,
            const exint subscribeRatio = 64,
            const exint minGrainSize   = 1024
        )
    {
        setHasComputed();
        this->statisticalFunction = statisticalFunction;
        this->delExtremeAttrib    = delExtremeAttrib;
        this->subscribeRatio = subscribeRatio;
        this->minGrainSize   = minGrainSize;
    }

    
    SYS_FORCE_INLINE void setMeasureAttrib()
    { useMeasureAttrib = false; }
    
    SYS_FORCE_INLINE void setMeasureAttrib(const bool useMeasureAttrib)
    { this->useMeasureAttrib = useMeasureAttrib; }

    
    SYS_FORCE_INLINE GA_Attribute* setExtremeElemAttrib(const bool detached = false, const GA_Storage storage = GA_STORE_INVALID, const UT_StringRef& attribName = "")
    { return extremeElemAttrib = getOutAttribArray().findOrCreateTuple(detached, GA_ATTRIB_DETAIL, GA_STORECLASS_INT, GA_STORE_INVALID, attribName); }
    
    SYS_FORCE_INLINE GA_Attribute* setExtremeElemAttrib(const bool detached = false, const UT_StringRef& attribName = "")
    { return setExtremeElemAttrib(detached, GA_STORE_INVALID, attribName); }
    
    //SYS_FORCE_INLINE GA_Attribute* setExtremeValueAttrib(const bool detached = false, const GA_Storage storage = GA_STORE_INVALID, const UT_StringRef& attribName = "")
    //{ return extremeValueAttrib = getOutAttribArray().findOrCreateTuple(detached, GA_ATTRIB_DETAIL, GA_STORECLASS_INT, GA_STORE_INVALID, attribName); }
    //
    //SYS_FORCE_INLINE GA_Attribute* setExtremeValueAttrib(const bool detached = false, const UT_StringRef& attribName = "")
    //{ return setExtremeValueAttrib(detached, GA_STORE_INVALID, attribName); }
    
    SYS_FORCE_INLINE void setExtremeValueAttrib()
    { extremeValueAttribName = nullptr; }
    
    SYS_FORCE_INLINE void setExtremeValueAttrib(const bool detached, const UT_StringRef& attribName)
    { extremeValueAttribDetached = detached; extremeValueAttribName = &attribName; }
    
    
    SYS_FORCE_INLINE GA_Offset getExtremeElemoff() const
    { return extremeElemoff; }
    

    
private:


    virtual bool
        computeCore() override
    {
        if ( (getInAttribArray().isEmpty() && !useMeasureAttrib) || (getOutGroupArray().isEmpty() && !extremeValueAttribName && !extremeElemAttrib) )
            return false;

        if (groupParser.isEmpty())
            return true;

        if (!useMeasureAttrib)
        {
            extremeAttribNonConst = getInAttribArray()[0];
            extremeAttrib = extremeAttribNonConst;
        }
        
        const GA_Size numElems = useMeasureAttrib ? geo->getNumPrimitives() : geo->getNumElements(extremeAttrib);
        
        if (numElems <= 1)
        {
            if (numElems == 1)
                extremeElemoff = 0;
            else
                extremeElemoff = GFE_INVALID_OFFSET;
            
            if (delExtremeAttrib && extremeAttribNonConst)
                geo->destroyAttrib(extremeAttribNonConst);
            return true;
        }

        if (useMeasureAttrib)
        {
            measure.groupParser.setGroup(groupParser);
            extremeAttrib = measure.findOrCreateTuple(true);
            extremeAttribNonConst = nullptr;
            measure.compute();
        }
        
        switch (extremeAttrib->getAIFTuple()->getStorage(extremeAttrib))
        {
        case GA_STORE_INT16:  computeExtremeElemoff<int16>();    break;
        case GA_STORE_INT32:  computeExtremeElemoff<int32>();    break;
        case GA_STORE_INT64:  computeExtremeElemoff<int64>();    break;
        case GA_STORE_REAL16: computeExtremeElemoff<fpreal16>(); break;
        case GA_STORE_REAL32: computeExtremeElemoff<fpreal32>(); break;
        case GA_STORE_REAL64: computeExtremeElemoff<fpreal64>(); break;
        }
        
        //if (extremeValueAttrib)
        //{
        //    const GA_AIFTuple* const aIFTuple = extremeValueAttrib->getAIFTuple();
        //    UT_ASSERT_P(aIFTuple);
        //    aIFTuple->set(extremeValueAttrib, 0, extremeValueAttrib);
        //}
        
        if (extremeElemAttrib)
        {
            const GA_AIFTuple* const aIFTuple = extremeElemAttrib->getAIFTuple();
            UT_ASSERT_P(aIFTuple);
            aIFTuple->set(extremeElemAttrib, 0, extremeElemoff);
        }
        
        if (!getOutGroupArray().isEmpty())
        {
            GA_ElementGroup* const outGroup = getOutGroupArray().getElementGroup(0);
            if (outGroup->getOwner() == extremeAttrib->getOwner())
            {
                if (groupSetter.reverseGroup)
                    outGroup->addAll();
                outGroup->setElement(extremeElemoff, !groupSetter.reverseGroup);
            }
        }

        if (delExtremeAttrib && extremeAttribNonConst)
            geo->destroyAttrib(extremeAttribNonConst);

        if (useMeasureAttrib)
            measure.getOutAttribArray().clear();
        
        return true;
    }



    
    template<typename FLOAT_T>
    void computeExtremeElemoff()
    {
        ComputeExtremeElemoff<FLOAT_T> body(geo, extremeAttrib, statisticalFunction);
        UTparallelReduce(groupParser.getSplittableRange(extremeAttrib), body, subscribeRatio, minGrainSize);
        extremeElemoff = body.getExtremeElemoff();
        if (extremeValueAttribName)
        {
            GA_Attribute* const extremeValueAttrib = getOutAttribArray().findOrCreateTuple<FLOAT_T>(false, GA_ATTRIB_DETAIL, *extremeValueAttribName);
            const GA_AIFTuple* const aIFTuple = extremeValueAttrib->getAIFTuple();
            UT_ASSERT_P(aIFTuple);
            aIFTuple->set(extremeValueAttrib, 0, body.getExtremeValue());
        }
    }
    
    
    template<typename T>
    class ComputeExtremeElemoff
    {
    public:
        ComputeExtremeElemoff(const GFE_Detail* const a, const GA_Attribute* const b, const GFE_StatisticalFunction c)
            : myGeo(a)
            , myAttrib(b)
            , myStatisticalFunction(c)
            , myExtremeElemoff(GFE_INVALID_OFFSET)
            , myExtremeValue(myStatisticalFunction == GFE_StatisticalFunction::Max ? SYS_FP64_MIN : SYS_FP64_MAX)
        {}
        ComputeExtremeElemoff(ComputeExtremeElemoff& src, UT_Split)
            : myGeo(src.myGeo)
            , myAttrib(src.myAttrib)
            , myStatisticalFunction(src.myStatisticalFunction)
            , myExtremeElemoff(GFE_INVALID_OFFSET)
            , myExtremeValue(myStatisticalFunction == GFE_StatisticalFunction::Max ? SYS_FP64_MIN : SYS_FP64_MAX)
        {}
        void operator()(const GA_SplittableRange& r)
        {
            GA_PageHandleT<T, T, true, false, const GA_Attribute, const GA_ATINumeric, const GA_Detail> attrib_ph(myAttrib);
            for (GA_PageIterator pit = r.beginPages(); !pit.atEnd(); ++pit)
            {
                GA_Offset start, end;
                for (GA_Iterator it(pit.begin()); it.blockAdvance(start, end); )
                {
                    attrib_ph.setPage(start);
                    for (GA_Offset elemoff = start; elemoff < end; ++elemoff)
                    {
                        bool flag;
                        switch (myStatisticalFunction)
                        {
                        default:
                        case GFE_StatisticalFunction::Min: flag = attrib_ph.value(elemoff) < myExtremeValue; break;
                        case GFE_StatisticalFunction::Max: flag = attrib_ph.value(elemoff) > myExtremeValue; break;
                        }
                        if (flag)
                        {
                            myExtremeValue = attrib_ph.value(elemoff);
                            myExtremeElemoff = elemoff;
                        }
                    }
                }
            }
        }
        void join(const ComputeExtremeElemoff& other)
        {
            myExtremeElemoff = other.myExtremeElemoff;
            myExtremeValue   = other.myExtremeValue;
        }
        SYS_FORCE_INLINE GA_Offset getExtremeElemoff() const
        { return myExtremeElemoff; }
        SYS_FORCE_INLINE T getExtremeValue() const
        { return myExtremeValue; }
    private:
        const GFE_Detail* const myGeo;
        const GA_Attribute* const myAttrib;
        GFE_StatisticalFunction myStatisticalFunction;
        GA_Offset myExtremeElemoff;
        T myExtremeValue;
    }; // End of Class ComputeExtremeElemoff


public:
    
    GFE_Measure measure;
    
    bool useMeasureAttrib = false;
    
    GFE_StatisticalFunction statisticalFunction = GFE_StatisticalFunction::Min;
    bool delExtremeAttrib = false;
    
    //const UT_StringRef* extremeElemGroupName = nullptr;
    bool extremeValueAttribDetached = false;
    const UT_StringRef* extremeValueAttribName = nullptr;
private:
    GA_Offset extremeElemoff = GFE_INVALID_OFFSET;
    const GA_Attribute* extremeAttrib;
    GA_Attribute* extremeAttribNonConst;
    
    GA_Attribute* extremeElemAttrib = nullptr;
    //GA_Attribute* extremeValueAttrib = nullptr;

    
    exint subscribeRatio = 64;
    exint minGrainSize = 1024;
    
    
}; // End of Class GFE_ExtremeElement






#endif
