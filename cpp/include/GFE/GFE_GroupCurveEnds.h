
#pragma once

#ifndef __GFE_GroupCurveEnds_h__
#define __GFE_GroupCurveEnds_h__

#include "GFE/GFE_GroupCurveEnds.h"

#include "GFE/GFE_GeoFilter.h"



class GFE_GroupCurveEnds : public GFE_AttribFilter {


public:
    
public:
    using GFE_AttribFilter::GFE_AttribFilter;


    void
        setComputeParm(
            const size_t numEnds = 1,
            const exint subscribeRatio = 64,
            const exint minGrainSize = 1024
        )
    {
        setHasComputed();
        this->numEnds  = numEnds;
        this->subscribeRatio  = subscribeRatio;
        this->minGrainSize    = minGrainSize;
    }

    
#define GFE_GroupCurveEnds_SPECIALIZATION_SetAttrib(Name_Lower, Name_Capital)                                      \
        SYS_FORCE_INLINE GA_PointGroup* set##Name_Capital##Group(                                                  \
            const bool detached = false, const UT_StringRef& attribName = "")                                      \
        { return Name_Lower##Group = getOutGroupArray().findOrCreatePoint(detached, attribName); }                 \
                                                                                                                   \
                                                                                                                   \
        SYS_FORCE_INLINE GA_Attribute* set##Name_Capital##Attrib(                                                  \
            const bool detached = false,                                                                           \
            const GA_StorageClass storageClass = GA_STORECLASS_FLOAT,                                              \
            const GA_Storage storage = GA_STORE_INVALID,                                                           \
            const UT_StringRef& attribName = ""                                                                    \
        )                                                                                                          \
        {                                                                                                          \
            return getOutAttribArray().findOrCreateTuple(detached, GA_ATTRIB_POINT,                                \
                storageClass, storage, attribName, 1, GA_Defaults(GFE_INVALID_OFFSET));                            \
        }                                                                                                          \


GFE_GroupCurveEnds_SPECIALIZATION_SetAttrib(start, Start);
GFE_GroupCurveEnds_SPECIALIZATION_SetAttrib(end, End);
GFE_GroupCurveEnds_SPECIALIZATION_SetAttrib(ends, Ends);

#undef GFE_GroupCurveEnds_SPECIALIZATION_SetAttrib


    

    virtual void visualizeOutGroup(const GFE_CurveEndsType type)
    {
        switch (type)
        {
        case GFE_CurveEndsType::Ends:  visualizeGroup(endsGroup);  break;
        case GFE_CurveEndsType::Start: visualizeGroup(startGroup); break;
        case GFE_CurveEndsType::End:   visualizeGroup(endGroup);   break;
        }
    }
    
    virtual SYS_FORCE_INLINE void visualizeOutGroup() const override
    { GFE_AttribFilter::visualizeOutGroup(); }
    
    
    
private:

    // can not use in parallel unless for each GA_Detail
    virtual bool
        computeCore() override
    {
        if (getOutAttribArray().isEmpty() && getOutGroupArray().isEmpty())
            return false;

        if (!(startGroup || endGroup || endsGroup))
            return false;
        
        if (numEnds <= 0)
            return true;
        
        if (groupParser.isEmpty())
            return true;
        
        groupCurveEnds();
        // attribPtr = getOutAttribArray()[0];
        // // const GA_Storage storage = attribPtr->getAIFTuple()->getStorage(attribPtr);
        // switch (attribPtr->getAIFTuple()->getStorage(attribPtr))
        // {
        // case GA_STORE_INT16:  groupCurveEnds<int16>();           break;
        // case GA_STORE_INT32:  groupCurveEnds<int32>();           break;
        // case GA_STORE_INT64:  groupCurveEnds<int64>();           break;
        // case GA_STORE_REAL16: groupCurveEnds<fpreal16>();        break;
        // case GA_STORE_REAL32: groupCurveEnds<fpreal32>();        break;
        // case GA_STORE_REAL64: groupCurveEnds<fpreal64>();        break;
        // case GA_STORE_STRING: groupCurveEnds<UT_StringHolder>(); break;
        // default: break;
        // }

        return true;
    }


    //template<typename T>
    void groupCurveEnds()
    {
        UTparallelFor(groupParser.getPrimitiveSplittableRange(), [this](const GA_SplittableRange& r)
        {
            GA_Offset start, end;
            for (GA_Iterator it(r.begin()); it.blockAdvance(start, end); )
            {
                //attrib_ph.setPage(start);
                for (GA_Offset elemoff = start; elemoff < end; ++elemoff)
                {
                    const GA_Size numvtx = geo->getPrimitiveVertexCount(elemoff);
                    if (numvtx <= 0)
                        continue;
                    else if (numvtx == 1)
                    {
                        const GA_Offset endPtoff = geo->getPrimitivePointOffset(elemoff, 0);
                        if (startGroup)
                            startGroup->setElement(endPtoff, true);
                        if (endGroup)
                            endGroup->setElement(endPtoff, true);
                        if (endsGroup)
                            endsGroup->setElement(endPtoff, true);
                    }
                    
                    const GA_Size numEnds_forward  = SYSmin(numvtx, GA_Size(numEnds));
                    const GA_Size numEnds_backward = SYSmax(GA_Size(0), numvtx-GA_Size(numEnds));
                    if (endsGroup)
                    {
                        for (GA_Size vtxpnum = 0; vtxpnum < numEnds_forward; ++vtxpnum)
                        {
                            const GA_Offset endPtoff = geo->getPrimitivePointOffset(elemoff, vtxpnum);
                            endsGroup->setElement(endPtoff, true);
                            if (startGroup)
                                startGroup->setElement(endPtoff, true);
                        }
                        
                        for (GA_Size vtxpnum = numvtx-1; vtxpnum >= numEnds_backward; --vtxpnum)
                        {
                            const GA_Offset endPtoff = geo->getPrimitivePointOffset(elemoff, vtxpnum);
                            endsGroup->setElement(endPtoff, true);
                            if (endGroup)
                                endGroup->setElement(endPtoff, true);
                        }
                    }
                    else
                    {
                        for (GA_Size vtxpnum = 0; vtxpnum < numEnds_forward; ++vtxpnum)
                        {
                            const GA_Offset endPtoff = geo->getPrimitivePointOffset(elemoff, vtxpnum);
                            endsGroup->setElement(endPtoff, true);
                            if (startGroup)
                                startGroup->setElement(endPtoff, true);
                        }
                        
                        for (GA_Size vtxpnum = numvtx-1; vtxpnum >= numEnds_backward; --vtxpnum)
                        {
                            const GA_Offset endPtoff = geo->getPrimitivePointOffset(elemoff, vtxpnum);
                            endsGroup->setElement(endPtoff, true);
                            if (endGroup)
                                endGroup->setElement(endPtoff, true);
                        }
                    }
                }
            }
        }, subscribeRatio, minGrainSize);

        
    }



public:
    
    size_t numEnds = 1;
    
    GA_PointGroup* startGroup = nullptr;
    GA_PointGroup* endGroup = nullptr;
    GA_PointGroup* endsGroup = nullptr;
    
    GA_Attribute* startAttrib = nullptr;
    GA_Attribute* endAttrib = nullptr;
    GA_Attribute* endsAttrib = nullptr;

private:
    
    exint subscribeRatio = 64;
    exint minGrainSize = 1024;


}; // End of class GFE_GroupCurveEnds






#endif
