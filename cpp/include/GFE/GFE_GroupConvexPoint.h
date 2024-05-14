
#pragma once

#ifndef __GFE_GroupConvexPoint_h__
#define __GFE_GroupConvexPoint_h__

#include "GFE/GFE_GroupConvexPoint.h"

#include "GFE/GFE_GeoFilter.h"




class GFE_GroupConvexPoint : public GFE_AttribFilter {



public:
    using GFE_AttribFilter::GFE_AttribFilter;

    ~GFE_GroupConvexPoint()
    {
    }

    void
        setComputeParm(
            const bool outAsOffset = true,
            const exint subscribeRatio = 64,
            const exint minGrainSize = 64
        )
    {
        setHasComputed();
        this->outAsOffset = outAsOffset;
        this->subscribeRatio = subscribeRatio;
        this->minGrainSize = minGrainSize;
    }


private:

    virtual bool
        computeCore() override
    {
        if (getOutAttribArray().isEmpty())
            return false;

        if (groupParser.isEmpty())
            return true;
        
        
        GA_Attribute* const attribPtr = getOutAttribArray()[0];
        // const GA_Storage storage = attribPtr->getAIFTuple()->getStorage(attribPtr);
        switch (attribPtr->getAIFTuple()->getStorage(attribPtr))
        {
        case GA_STORE_INT16:
            enumerate<int16>(attribPtr);
            break;
        case GA_STORE_INT32:
            enumerate<int32>(attribPtr);
            break;
        case GA_STORE_INT64:
            enumerate<int64>(attribPtr);
            break;
        case GA_STORE_REAL16:
            enumerate<fpreal16>(attribPtr);
            break;
        case GA_STORE_REAL32:
            enumerate<fpreal32>(attribPtr);
            break;
        case GA_STORE_REAL64:
            enumerate<fpreal64>(attribPtr);
            break;
        case GA_STORE_STRING:
            enumerate<UT_StringHolder>(attribPtr);
            break;
        default:
            break;
        }


        return true;
    }


    template<typename T>
    void
        enumerate(
            GA_Attribute* const attribPtr
        )
    {
        UT_ASSERT_P(attribPtr);
        const GA_SplittableRange geoSplittableRange0(groupParser.getRange(attribPtr->getOwner()));
        if(outAsOffset)
        {
            UTparallelFor(geoSplittableRange0, [attribPtr](const GA_SplittableRange& r)
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
                            attrib_ph.value(elemoff) = elemoff;
                        }
                    }
                }
            }, subscribeRatio, minGrainSize);
        }
        else
        {
            const GA_IndexMap& indexMap = attribPtr->getIndexMap();
            UTparallelFor(geoSplittableRange0, [attribPtr, &indexMap](const GA_SplittableRange& r)
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
                            attrib_ph.value(elemoff) = indexMap.indexFromOffset(elemoff);
                        }
                    }
                }
            }, subscribeRatio, minGrainSize);
        }
    }


    template<>
    void
        enumerate<UT_StringHolder>(
            GA_Attribute* const attribPtr
        )
    {
        UT_ASSERT_P(attribPtr);
        const GA_RWHandleS attrib_h(attribPtr);
        const GA_SplittableRange geoSplittableRange0(groupParser.getRange(attribPtr->getOwner()));
        if(outAsOffset)
        {
            UTparallelFor(geoSplittableRange0, [&attrib_h](const GA_SplittableRange& r)
            {
                GA_Offset start, end;
                for (GA_Iterator it(r); it.blockAdvance(start, end); )
                {
                    for (GA_Offset elemoff = start; elemoff < end; ++elemoff)
                    {
                        const char cElemoff = (char)(elemoff+'0');
                        attrib_h.set(elemoff, UT_StringHolder(&cElemoff));
                    }
                }
            }, subscribeRatio, minGrainSize);
        }
        else
        {
            const GA_IndexMap& indexMap = attribPtr->getIndexMap();
            UTparallelFor(geoSplittableRange0, [&attrib_h, &indexMap](const GA_SplittableRange& r)
            {
                GA_Offset start, end;
                for (GA_Iterator it(r); it.blockAdvance(start, end); )
                {
                    for (GA_Offset elemoff = start; elemoff < end; ++elemoff)
                    {
                        const char cElemoff = (char)(indexMap.indexFromOffset(elemoff)+'0');
                        attrib_h.set(elemoff, cElemoff);
                    }
                }
            }, subscribeRatio, minGrainSize);
        }
    }

    


public:
    bool outAsOffset = true;

private:
    exint subscribeRatio = 64;
    exint minGrainSize = 64;


}; // End of class GFE_GroupConvexPoint



#endif
