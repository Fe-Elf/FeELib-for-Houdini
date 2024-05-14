
#pragma once

#ifndef __GFE_GroupLoopedPrim_h__
#define __GFE_GroupLoopedPrim_h__

#include "GFE/GFE_GroupLoopedPrim.h"

#include "GFE/GFE_GeoFilter.h"




#include "GFE/GFE_Fuse.h"

class GFE_GroupLoopedPrim : public GFE_AttribFilter {

public:
    using GFE_AttribFilter::GFE_AttribFilter;

    void
        setComputeParm(
            const bool preFusePoint    = false,
            const fpreal fuseDist      = 1e-05,
            const exint subscribeRatio = 64,
            const exint minGrainSize   = 1024
        )
    {
        setHasComputed();

        this->preFusePoint   = preFusePoint;
        this->fuseDist       = fuseDist;
        this->subscribeRatio = subscribeRatio;
        this->minGrainSize   = minGrainSize;
    }

private:


    virtual bool
        computeCore() override
    {
        if (groupParser.isEmpty())
            return true;

        GU_DetailHandle geoOrigin_h;
        if (preFusePoint)
        {
            geoOriginTmp = new GU_Detail();
            geoOrigin_h.allocateAndSet(geoOriginTmp);
            geoOriginTmp->replaceWith(*geo);
            
            GFE_Fuse fuse(geoOriginTmp, nullptr, cookparms);
            fuse.method = GFE_Fuse::Method::Point;
            fuse.setPointComputeParm(true, fuseDist, true);
            fuse.setConsolidate();
            //fuse.setPositionAttrib("P");
            fuse.compute();
        }
        else
        {
            geoOriginTmp = geo->asGU_Detail();
        }

        const GA_ATITopology* const vtxPointRef = geoOriginTmp->getTopology().getPointRef();
        UTparallelFor(groupParser.getPrimitiveSplittableRange(), [this, vtxPointRef](const GA_SplittableRange& r)
        {
            GA_Offset start, end;
            for (GA_Iterator it(r); it.blockAdvance(start, end); )
            {
                for (GA_Offset elemoff = start; elemoff < end; ++elemoff)
                {
                    if (geoOriginTmp->getPrimitiveClosedFlag(elemoff))
                    {
                        loopedPrimGroup->setElement(elemoff, true);
                        continue;
                    }
                    const GA_OffsetListRef& vertices = geoOriginTmp->getPrimitiveVertexList(elemoff);
                    const GA_Size numvtx = vertices.size();
                    if (vtxPointRef->getLink(vertices[0]) == vtxPointRef->getLink(vertices[numvtx-1]))
                    {
                        loopedPrimGroup->setElement(elemoff, true);
                        continue;
                    }
                }
            }
        }, subscribeRatio, minGrainSize);
        
        return true;
    }


public:
    bool preFusePoint = false;
    fpreal fuseDist = 1e-05;

private:
    GU_Snap::PointSnapParms fuseParms;
    GA_PrimitiveGroup* loopedPrimGroup;
    
    GU_Detail* geoOriginTmp;
    
    exint subscribeRatio = 64;
    exint minGrainSize = 1024;
    
}; // End of Class GFE_GroupLoopedPrim




#endif