
#pragma once

#ifndef __GFE_CurveExtendedLine_h__
#define __GFE_CurveExtendedLine_h__

#include "GFE/GFE_CurveExtendedLine.h"

#include "GFE/GFE_GeoFilter.h"



class GFE_CurveExtendedLine : public GFE_AttribFilter {


public:

    using GFE_AttribFilter::GFE_AttribFilter;


    void
        setComputeParm(
            const fpreal extendDist = 1.0,
            const GFE_CurveEndsType curveEndsType = GFE_CurveEndsType::Ends,
            const GFE_Axis flattenAxis = GFE_Axis::Invalid,
            const exint subscribeRatio = 64,
            const exint minGrainSize   = 64
        )
    {
        setHasComputed();
        this->extendDist = extendDist;
        this->curveEndsType = curveEndsType;
        this->flattenAxis = flattenAxis;
        this->subscribeRatio = subscribeRatio;
        this->minGrainSize   = minGrainSize;
    }

private:

    virtual bool
        computeCore() override
    {
        if (groupParser.isEmpty())
            return true;
        
        const GA_PrimitiveGroup* const geoPrimitiveGroup = groupParser.getPrimitiveGroup();
        GU_DetailHandle geoTmp_h;
        if (geoSrc)
        {
            geoTmp = geoSrc;
        }
        else
        {
            GU_Detail* const geoTmpGU = new GU_Detail;
            geoTmpGU->replaceWith(*geo);
            geoTmp_h.allocateAndSet(geoTmpGU);
            geoTmp = geoTmpGU;
        }
        
        geo->clear();
        
        GA_Offset vtxoffStart;
        geo->appendPrimitivesAndVertices(GA_PrimitiveTypeId(1), geoPrimitiveGroup->entries(), curveEndsType == GFE_CurveEndsType::Ends ? 2 : 1, vtxoffStart, false);
        
        setValidPosAttrib();
        UT_ASSERT_P(posAttrib->getAIFTuple());
        switch (posAttrib->getAIFTuple()->getStorage(posAttrib))
        {
        case GA_STORE_REAL16: curveExtendedLine<fpreal16>();        break;
        case GA_STORE_REAL32: curveExtendedLine<fpreal32>();        break;
        case GA_STORE_REAL64: curveExtendedLine<fpreal64>();        break;
        default: break;
        }

        return true;
    }


    template<typename FLOAT_T>
    void curveExtendedLine()
    {

        UT_ASSERT_P(posAttribNonConst);
        
        const GA_RWHandleT<UT_Vector3T<FLOAT_T>> pos_h(posAttribNonConst);
        const GA_ROHandleT<UT_Vector3T<FLOAT_T>> posRef_h(geoTmp->getP());
        
        UTparallelFor(groupParser.getPrimitiveSplittableRange(), [this, &pos_h, &posRef_h](const GA_SplittableRange& r)
        {
            //for (GA_PageIterator pit = r.beginPages(); !pit.atEnd(); ++pit)
            //{
                GA_Offset start, end;
                for (GA_Iterator it(r.begin()); it.blockAdvance(start, end); )
                {
                    for (GA_Offset elemoff = start; elemoff < end; ++elemoff)
                    {
                        const GA_OffsetListRef& vertices = geoTmp->getPrimitiveVertexList(elemoff);
                        const GA_Size numvtx = vertices.size();
                        if (numvtx < 2)
                            continue;

                        GA_Offset ptoff = geoTmp->primitiveIndex(elemoff);
                        switch (curveEndsType)
                        {
                        case GFE_CurveEndsType::Ends:
                            ptoff *= 2;
                            extendCurve<FLOAT_T, false>(vertices, pos_h, posRef_h, ptoff, 0);
                            extendCurve<FLOAT_T, true> (vertices, pos_h, posRef_h, ptoff+2, 0);
                        break;
                        case GFE_CurveEndsType::Start: extendCurve<FLOAT_T, false>(vertices, pos_h, posRef_h, ptoff, 0       ); break;
                        case GFE_CurveEndsType::End:   extendCurve<FLOAT_T, true> (vertices, pos_h, posRef_h, ptoff, numvtx-1); break;
                        default: UT_ASSERT_MSG(0, "Unhandled GFE_CurveEndsType"); break;
                        }
                    }
                }
            //}
        }, subscribeRatio, minGrainSize);
    }



    template<typename FLOAT_T, bool IsEnd>
    void extendCurve(
        const GA_OffsetListRef& vertices,
        const GA_RWHandleT<UT_Vector3T<FLOAT_T>>& pos_h,
        const GA_ROHandleT<UT_Vector3T<FLOAT_T>>& posRef_h,
        const GA_Offset ptoff,
        const GA_Size vtxpnum
    )
    {
        const GA_Size vtxpnum_neb = IsEnd ? vtxpnum-1 : vtxpnum+1;
        const GA_Offset ptoff0 = geoTmp->vertexPoint(vertices[vtxpnum]);
        const GA_Offset ptoff1 = geoTmp->vertexPoint(vertices[vtxpnum_neb]);
        const UT_Vector3T<FLOAT_T> pos0 = posRef_h.get(ptoff0);
        const UT_Vector3T<FLOAT_T> pos1 = posRef_h.get(ptoff1);
        
        const UT_Vector3T<FLOAT_T> dir = pos0 - pos1;
        if (flattenAxis != GFE_Axis::Invalid)
        {
            dir[flattenAxis] = 0;
        }
        dir.normalize();
        pos_h.set(ptoff,   pos0);
        pos_h.set(ptoff+1, pos0 + dir * extendDist);
    }


public:
    fpreal extendDist = 1.0;
    GFE_CurveEndsType curveEndsType = GFE_CurveEndsType::Ends;
    
    GFE_Axis flattenAxis = GFE_Axis::Invalid;

private:
    const GA_Detail* geoTmp;
    GA_Attribute* attrib;
    exint subscribeRatio = 64;
    exint minGrainSize = 1024;


}; // End of class GFE_CurveExtendedLine





#endif
