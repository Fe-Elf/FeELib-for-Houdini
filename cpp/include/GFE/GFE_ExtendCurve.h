
#pragma once

#ifndef __GFE_ExtendCurve_h__
#define __GFE_ExtendCurve_h__

#include "GFE/GFE_ExtendCurve.h"

#include "GFE/GFE_GeoFilter.h"



class GFE_ExtendCurve : public GFE_GeoFilter {


public:
    using GFE_GeoFilter::GFE_GeoFilter;


    void
    setComputeParm(
            const fpreal dist = 0,
            const bool extendStart       = true,
            const bool extendEnd         = true,
            const bool ignoreTwoNebPoint = false,
            const bool tryFindNextPointIfOverlap = false,
            const exint subscribeRatio = 64,
            const exint minGrainSize   = 64
        )
    {
        setHasComputed();
        this->dist = dist;
        this->extendStart       = extendStart;
        this->extendEnd         = extendEnd;
        this->ignoreTwoNebPoint = ignoreTwoNebPoint;
        this->tryFindNextPointIfOverlap = tryFindNextPointIfOverlap;
        this->subscribeRatio = subscribeRatio;
        this->minGrainSize   = minGrainSize;
    }

    
private:

    virtual bool
        computeCore() override
    {
        if (!extendStart && !extendEnd || dist == 0)
            return true;
            
        if (groupParser.isEmpty())
            return true;
        
        setValidConstPosAttrib();

        pointGroup = groupParser.classType() == GA_GROUP_PRIMITIVE ? nullptr : groupParser.getPointGroup();
        switch (posAttribNonConst->getAIFTuple()->getStorage(posAttribNonConst))
        {
        case GA_STORE_REAL16: extendCurveStraight<fpreal16>(); break;
        case GA_STORE_REAL32: extendCurveStraight<fpreal32>(); break;
        case GA_STORE_REAL64: extendCurveStraight<fpreal64>(); break;
        default: UT_ASSERT_MSG(0, "not possible storage"); break;
        }
        
        return true;
    }


template<typename FLOAT_T>
void extendCurveStraight()
{
    const GA_RWHandleT<UT_Vector3T<FLOAT_T>> pos_h(posAttribNonConst);
        
    UTparallelFor(groupParser.getPrimitiveSplittableRange(), [this, &pos_h](const GA_SplittableRange& r)
    {
        GA_Offset start, end;
        for (GA_Iterator it(r); it.blockAdvance(start, end); )
        {
            for (GA_Offset primoff = start; primoff < end; ++primoff)
            {
                const GA_Size numvtx = geo->getPrimitiveVertexCount(primoff);
                if (numvtx < 2)
                    continue;

                if (extendStart)
                    extendCurveStraight<FLOAT_T>(pos_h, primoff, numvtx, 0, 1);

                if (extendEnd)
                    extendCurveStraight<FLOAT_T>(pos_h, primoff, numvtx, numvtx-1, -1);
            }
        }
    }, subscribeRatio, minGrainSize);
}

template<typename FLOAT_T>
SYS_FORCE_INLINE void
extendCurveStraight(
    const GA_RWHandleT<UT_Vector3T<FLOAT_T>>& pos_h,
    const GA_Offset primoff,
    const GA_Size numvtx,
    const GA_Size vtxpnum,
    const GA_Size intArg
)
{
    const GA_Offset primvtx0 = geo->getPrimitiveVertexOffset(primoff, vtxpnum);
    const GA_Offset primpoint0 = geo->vertexPoint(primvtx0);

    if (pointGroup && !pointGroup->containsOffset(primpoint0))
        return;

    if (ignoreTwoNebPoint)
    {
        if (geo->pointVertex(primpoint0) != primvtx0)
            return;
        const GA_Offset vtxoff_next = geo->vertexToNextVertex(primvtx0);
        if (GFE_Type::isValidOffset(vtxoff_next) && vtxoff_next != primvtx0)
            return;
    }

    const UT_Vector3T<FLOAT_T> pos = pos_h.get(primpoint0);
    UT_Vector3T<FLOAT_T> nebpos;
    if (tryFindNextPointIfOverlap)
    {
        GA_Size vtxpnum_neb = vtxpnum;
        GA_Offset primpoint1;
        do {
            vtxpnum_neb += intArg;
            primpoint1 = geo->getPrimitivePointOffset(primoff, vtxpnum_neb);
            nebpos = pos_h.get(primpoint1);
        } while (vtxpnum_neb > 0 && vtxpnum_neb < numvtx && (primpoint0 == primpoint1 || nebpos == pos));
    }
    else
    {
        const GA_Offset primpoint1 = geo->getPrimitivePointOffset(primoff, intArg);
        nebpos = pos_h.get(primpoint1);
    }

    UT_Vector3T<FLOAT_T> dir(pos - nebpos);
    dir.normalize();
    dir = pos + dir * dist;
    pos_h.set(primpoint0, dir);
}


public:
    fpreal dist = 0;
    bool extendStart = true;
    bool extendEnd = true;
    bool ignoreTwoNebPoint = false;
    bool tryFindNextPointIfOverlap = false;

private:
    const GA_PointGroup* pointGroup;
    
    exint subscribeRatio = 64;
    exint minGrainSize   = 1024;


}; // End of class GFE_ExtendCurve







#endif
