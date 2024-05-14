
#pragma once

#ifndef __GFE_CornerPoint_h__
#define __GFE_CornerPoint_h__

#include "GFE/GFE_CornerPoint.h"

#include "GFE/GFE_GeoFilter.h"


#include "GFE/GFE_Normal2D.h"

#define GFE_CornerPoint_ConvexConcave 0

class GFE_CornerPoint : public GFE_AttribFilter {

public:
    using GFE_AttribFilter::GFE_AttribFilter;

    ~GFE_CornerPoint()
    {
    }

    void
        setComputeParm(
            const bool groupPrimCornerPoint = true,
            const exint subscribeRatio = 64,
            const exint minGrainSize   = 1024
        )
    {
        setHasComputed();
        
        this->groupPrimCornerPoint = groupPrimCornerPoint;
        this->subscribeRatio = subscribeRatio;
        this->minGrainSize   = minGrainSize;
    }




#if GFE_CornerPoint_ConvexConcave
    SYS_FORCE_INLINE void setThresholdConvex(const fpreal angle)
    { thresholdConvex = angle; }

    SYS_FORCE_INLINE void setThresholdConvexRadians(const fpreal angle = PI * 0.5)
    { setThresholdConvex(sin(angle - PI * 0.5)); }

    SYS_FORCE_INLINE void setThresholdConvexDegrees(const fpreal angle = 180)
    { setThresholdConvexRadians(GFE_Math::radians(angle)); }


    
    SYS_FORCE_INLINE void setThresholdConcave(const fpreal angle)
    { thresholdConcave = angle; }

    SYS_FORCE_INLINE void setThresholdConcaveRadians(const fpreal angle = PI * 0.5)
    { setThresholdConcave(sin(angle - PI * 0.5)); }

    SYS_FORCE_INLINE void setThresholdConcaveDegrees(const fpreal angle = 180)
    { setThresholdConcaveRadians(GFE_Math::radians(angle)); }

#else
    SYS_FORCE_INLINE void setThreshold(const fpreal val)
    { threshold = val; }

    SYS_FORCE_INLINE void setThresholdRadians(const fpreal angle = PI * 0.5)
    { setThreshold(sin((angle-PI) * 0.5)); }

    SYS_FORCE_INLINE void setThresholdDegrees(const fpreal angle = 180)
    { setThresholdRadians(GFE_Math::radians(angle)); }
#endif



private:

    virtual bool
        computeCore() override
    {
        if (getOutGroupArray().isEmpty())
            return false;

#if GFE_CornerPoint_ConvexConcave
        outConvexPoint  = ;
        outConcavePoint = ;
#endif
        
        if (groupParser.isEmpty())
            return true;
        
        setValidConstPosAttrib();

        noneZeroThreshold = threshold == 0;
        
        GFE_Normal2D normal2D(geo, cookparms);
        normal2D.groupParser.setGroup(groupParser);
        normal2D.setPositionAttrib(posAttrib);
        normal2D.setNormal2DAttrib(true);
        //const float cuspAngleDegrees = GEO_DEFAULT_ADJUSTED_CUSP_ANGLE;
        //const GEO_NormalMethod method = GEO_NormalMethod::ANGLE_WEIGHTED;
        //const bool copyOrigIfZero = false;
        //normal2D.normal3D.setComputeParm(cuspAngleDegrees, method, copyOrigIfZero);
        //normal2D.defaultNormal3D = {0,1,0};
        normal2D.findOrCreateNormal3D(false);
        normal2D.setComputeParm(false, false, noneZeroThreshold);
        normal2D.compute();
        
        normal2DAttrib = normal2D.getAttrib();
        groupSetter = getOutGroupArray().getPointGroup(0);

        switch (posAttrib->getAIFTuple()->getStorage(posAttrib))
        {
        case GA_STORE_REAL16: cornerPoint<fpreal16>(); break;
        case GA_STORE_REAL32: cornerPoint<fpreal32>(); break;
        case GA_STORE_REAL64: cornerPoint<fpreal64>(); break;
        default: break;
        }


        return true;
    }


    template<typename FLOAT_T>
    void cornerPoint()
    {
        if (groupPrimCornerPoint)
        {
            const GA_ROHandleT<UT_Vector3T<FLOAT_T>> pos_h(posAttrib);
            const GA_ROHandleT<UT_Vector3T<FLOAT_T>> normal2D_h(normal2DAttrib);
            UTparallelFor(groupParser.getPrimitiveSplittableRange(), [this, &pos_h, &normal2D_h](const GA_SplittableRange& r)
            {
                UT_Vector3T<FLOAT_T> posCur, posNext, dir0, dir1;
                for (GA_PageIterator pit = r.beginPages(); !pit.atEnd(); ++pit)
                {
                    GA_Offset start, end;
                    for (GA_Iterator it(pit.begin()); it.blockAdvance(start, end); )
                    {
                        for (GA_Offset elemoff = start; elemoff < end; ++elemoff)
                        {
                            const GA_OffsetListRef& vertices = geo->getPrimitiveVertexList(elemoff);
                            const GA_Size numvtx = vertices.size();
                            const GA_Size lastIndex = numvtx-1;
                            
                            posCur = geo->vertexPointVal(pos_h, vertices[0]);
                            if (geo->getPrimitiveClosedFlag(elemoff))
                            {
                                dir0 = geo->vertexPointVal(pos_h, vertices[numvtx-1]) - posCur;
                                for (GA_Size vtxpnum = 0; vtxpnum < lastIndex; ++vtxpnum)
                                {
                                    const GA_Offset ptoff = geo->vertexPoint(vertices[vtxpnum]);
                                    posNext = geo->vertexPointVal(pos_h, vertices[vtxpnum+1]);
                                    dir1 = posNext - posCur;
                                    if (noneZeroThreshold)
                                    {
                                        dir1.normalize();
                                        dir0 += dir1;
                                        dir0.normalize();
                                    }
                                    const fpreal dotVal = dot(normal2D_h.get(ptoff), dir0);
#if GFE_CornerPoint_ConvexConcave
                                    if (outConvexPoint)
                                        groupSetterConvex.set(ptoff, dotVal > threshold);
                                    if (outConcavePoint)
                                        groupSetter.set(ptoff, dotVal < threshold);
#else
                                    groupSetter.set(ptoff, outAsConvexPoint ? dotVal > threshold : dotVal < threshold);
#endif
                                    posCur = posNext;
                                    dir0 = -dir1;
                                }
                                const GA_Offset ptoff = geo->vertexPoint(vertices[numvtx-1]);
                                posNext = geo->vertexPointVal(pos_h, vertices[0]);
                                dir1 = posNext - posCur;
                                if (noneZeroThreshold)
                                {
                                    dir1.normalize();
                                    dir0 += dir1;
                                    dir0.normalize();
                                }
                                const fpreal dotVal = dot(normal2D_h.get(ptoff), dir0);
#if GFE_CornerPoint_ConvexConcave
                                if (outConvexPoint)
                                    groupSetterConvex.set(ptoff, dotVal > threshold);
                                if (outConcavePoint)
                                    groupSetter.set(ptoff, dotVal < threshold);
#else
                                groupSetter.set(ptoff, outAsConvexPoint ? dotVal > threshold : dotVal < threshold);
#endif
                            }
                        }
                    }
                }
            }, subscribeRatio, minGrainSize);
        }
        else
        {
            /*
            const GA_ROHandleT<UT_Vector3T<FLOAT_T>> pos_h(posAttrib);
            const GA_ROHandleT<UT_Vector3T<FLOAT_T>> normal2D_h(normal2DAttrib);
            UTparallelFor(groupParser.getPointSplittableRange(), [this, &pos_h, &normal2D_h](const GA_SplittableRange& r)
            {
                UT_Vector3T<FLOAT_T> dir0, dir1;
                GA_PageHandleT<UT_Vector3T<FLOAT_T>, FLOAT_T, true, false, const GA_Attribute, const GA_ATINumeric, const GA_Detail> pos_ph(posAttrib);
                for (GA_PageIterator pit = r.beginPages(); !pit.atEnd(); ++pit)
                {
                    GA_Offset start, end;
                    for (GA_Iterator it(pit.begin()); it.blockAdvance(start, end); )
                    {
                        pos_ph.setPage(start);
                        for (GA_Offset elemoff = start; elemoff < end; ++elemoff)
                        {
                            cornerPointGroup->setElement(elemoff, );
                            dir0 = pos_h.get(elemoff) - pos_ph.value(elemoff);
                            dir1 = pos_h.get(elemoff) - pos_ph.value(elemoff);
                        }
                    }
                }
            }, subscribeRatio, minGrainSize);
            */
        }
    }



public:

        
#if GFE_CornerPoint_ConvexConcave
    bool outConvexPoint;
    bool outConcavePoint;
    fpreal thresholdConvex  = 0;
    fpreal thresholdConcave = 0;
#else
    fpreal threshold = 0;
    bool outAsConvexPoint = true;
#endif
        
    bool groupPrimCornerPoint = true;
        
private:
    GA_Attribute* normalAttrib;
    const GA_Attribute* normal2DAttrib;

    bool noneZeroThreshold;
    
    exint subscribeRatio = 64;
    exint minGrainSize = 1024;


}; // End of class GFE_CornerPoint



#endif
