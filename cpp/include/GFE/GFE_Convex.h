
#pragma once

#ifndef __GFE_Convex_h__
#define __GFE_Convex_h__

#include "GFE/GFE_Convex.h"



#include "GFE/GFE_GeoFilter.h"

class GFE_Convex : public GFE_GeoFilter {


public:
    using GFE_GeoFilter::GFE_GeoFilter;


    void
        setComputeParm(
            const GA_Size maxPoint = 3,
            const bool flipEdge = true,
            const bool avoidDegeneracy = false
        )
    {
        setHasComputed();
        this->maxPoint        = maxPoint;
        this->flipEdge        = flipEdge;
        this->avoidDegeneracy = avoidDegeneracy;
    }

    SYS_FORCE_INLINE virtual void bumpDataId() const override
    { geo->bumpDataIdsForAddOrRemove(false, true, true); }
    
private:

    // can not use in parallel unless for each GA_Detail
    virtual bool
        computeCore() override
    {
        if (groupParser.isEmpty())
            return true;
        
        GA_PrimitiveGroupUPtr geoPrimGroupUPtr;
        GA_PrimitiveGroup* geoPrimGroup = nullptr;
        const GA_PrimitiveGroup* const groupParserPrimGroup = groupParser.getPrimitiveGroup();
        if (groupParserPrimGroup)
        {
            geoPrimGroupUPtr = geo->createDetachedPrimitiveGroup();
            geoPrimGroup = geoPrimGroupUPtr.get();
            geoPrimGroup->combine(groupParserPrimGroup);
        }
        
#if 1
        geo->asGU_Detail()->convex(maxPoint, geoPrimGroup, nullptr, flipEdge, avoidDegeneracy);
        
#else
        GU_TriangleMeshT<fpreal> triangleMesh(static_cast<const GU_Detail*>(&outGeo0));
        triangleMesh.buildMesh();
#endif

        return true;
    }


public:
    GA_Size maxPoint = 3;
    bool flipEdge = true;
    bool avoidDegeneracy = false;

private:
    
}; // End of class GFE_Convex









#endif
