
#pragma once

#ifndef __GFE_GroupUnshared_h__
#define __GFE_GroupUnshared_h__

#include "GFE/GFE_GroupUnshared.h"

#include "GFE/GFE_GeoFilter.h"



#include "GFE/GFE_MeshTopology.h"
#include "GFE/GFE_GroupUnion.h"
#include "GU/GU_Snap.h"




class GFE_GroupUnshared : public GFE_AttribCreateFilter {

public:
    // using GFE_AttribCreateFilter::GFE_AttribCreateFilter;
    
    GFE_GroupUnshared(
        GFE_Detail* const geo,
        const SOP_NodeVerb::CookParms* const cookparms = nullptr
    )
        : GFE_AttribCreateFilter(geo, cookparms)
        , meshTopology(geo, cookparms)
    {
    }

    GFE_GroupUnshared(
        GA_Detail& geo,
        const SOP_NodeVerb::CookParms* const cookparms = nullptr
    )
        : GFE_AttribCreateFilter(geo, cookparms)
        , meshTopology(geo, cookparms)
    {
    }

    void
        setComputeParm(
            const bool preFusePoint = false,
            const fpreal fuseDist = 1e-05,
            const exint subscribeRatio = 64,
            const exint minGrainSize = 1024
        )
    {
        setHasComputed();

        this->preFusePoint = preFusePoint;
        this->fuseDist = fuseDist;
        this->subscribeRatio = subscribeRatio;
        this->minGrainSize = minGrainSize;
        
        meshTopology.setComputeParm(true, subscribeRatio, minGrainSize);
    }

/*
    virtual GA_Group* findOrCreateGroup(
        const bool detached = false,
        const GA_GroupType groupType = GA_GROUP_VERTEX,
        const UT_StringRef& name = "__topo_unshared"
    ) override
    {
        switch (groupType)
        {
        case GA_GROUP_PRIMITIVE:    break;
        case GA_GROUP_POINT:        break;
        case GA_GROUP_VERTEX:    return findOrCreateVertexGroup(detached, name); break;
        case GA_GROUP_EDGE:      return findOrCreateEdgeGroup(detached, name);   break;
        }
        return nullptr;
    }

    virtual void findOrCreateGroup(
        const bool detached = false,
        const bool outVertexGroup = true,
        const UT_StringRef& name = "__topo_unshared"
    )
    {
        if (outVertexGroup)
            findOrCreateVertexGroup(detached, name);
        else
            findOrCreateEdgeGroup(detached, name);
    }

    SYS_FORCE_INLINE virtual GA_VertexGroup* findOrCreateVertexGroup(const bool detached = false, const UT_StringRef& name = "__topo_unshared") override
    { return unsharedVertexGroup = meshTopology.setVertexNextEquivGroup(detached, name); }

    SYS_FORCE_INLINE virtual GA_EdgeGroup* findOrCreateEdgeGroup(const bool detached = false, const UT_StringRef& name = "__topo_unshared") override
    { return unsharedEdgeGroup = meshTopology.setUnsharedEdgeGroup(detached, name); }

    SYS_FORCE_INLINE GA_VertexGroup* getVertexGroup() const
    { return meshTopology.getVertexNextEquivGroup(); }

    SYS_FORCE_INLINE GA_EdgeGroup* getEdgeGroup() const
    { return meshTopology.getUnsharedEdgeGroup(); }
*/








private:


    virtual bool
        computeCore() override
    {
        if (getOutGroupArray().isEmpty())
            return false;
        
        if (groupParser.isEmpty())
            return true;

        GU_DetailHandle geoOrigin_h;
        GU_Detail* geoOriginTmp = nullptr;
        if (preFusePoint)
        {
            geoOriginTmp = new GU_Detail();
            geoOrigin_h.allocateAndSet(geoOriginTmp);
            geoOriginTmp->replaceWith(*geo);

            GU_Snap::PointSnapParms fuseParms;
            fuseParms.myDistance = fuseDist;
            fuseParms.myAlgorithm = GU_Snap::PointSnapParms::SnapAlgorithm::ALGORITHM_CLOSEST_POINT;
            fuseParms.myConsolidate = true;
            fuseParms.myDeleteConsolidated = true;
            fuseParms.myQPosH = geoOriginTmp->getP();
            fuseParms.myTPosH = geoOriginTmp->getP();
            GU_Snap::snapPoints(*geoOriginTmp, nullptr, fuseParms);
            //GU_Snap::snapPoints(*geoOriginTmp, static_cast<const GU_Detail*>(geo), fuseParms);
            
            meshTopology.reset(geoOriginTmp, cookparms);
        }
        else
        {
            geoOriginTmp = geo->asGU_Detail();
            
            meshTopology.reset(geo, cookparms);
        }
        
        GA_VertexGroup* const unsharedVertexGroup = meshTopology.setVertexNextEquivGroup();
        
        meshTopology.groupParser.setGroup(groupParser);
        meshTopology.compute();

        
        const GA_GroupType groupType = getOutGroupArray()[0]->classType();

        GA_VertexGroupUPtr geoUnsharedVertexGroupUPtr;
        GA_VertexGroup* geoUnsharedVertexGroup;
        
        if (preFusePoint && (groupType == GA_GROUP_POINT || groupType == GA_GROUP_EDGE))
        {
            geoUnsharedVertexGroupUPtr = geo->createDetachedVertexGroup();
            geoUnsharedVertexGroup = geoUnsharedVertexGroupUPtr.get();
            GFE_GroupUnion::groupUnion(geoUnsharedVertexGroup, unsharedVertexGroup);
        }
        else
        {
            geoUnsharedVertexGroup = unsharedVertexGroup;
        }
                
        if (groupType == GA_GROUP_POINT)
        {
            GA_PointGroup* const outPointGroup = getOutGroupArray().getPointGroup(0);
            GA_Attribute* const vertexPointDstAttrib = meshTopology.getVertexPointDst();
            const GA_ATITopology* const vtxPointRef = geo->getTopology().getPointRef();
            
            const GA_SplittableRange geoSplittableRange0(geo->getVertexRange(geoUnsharedVertexGroup));
            UTparallelFor(geoSplittableRange0, [vtxPointRef, outPointGroup, vertexPointDstAttrib](const GA_SplittableRange& r)
            {
                GA_PageHandleScalar<GA_Offset>::ROType dstpt_ph(vertexPointDstAttrib);
                for (GA_PageIterator pit = r.beginPages(); !pit.atEnd(); ++pit)
                {
                    GA_Offset start, end;
                    for (GA_Iterator it(pit.begin()); it.blockAdvance(start, end); )
                    {
                        dstpt_ph.setPage(start);
                        for (GA_Offset elemoff = start; elemoff < end; ++elemoff)
                        {
                            outPointGroup->setElement(vtxPointRef->getLink(elemoff), true);
                            outPointGroup->setElement(dstpt_ph.value(elemoff), true);
                        }
                    }
                }
            }, subscribeRatio, minGrainSize);
            outPointGroup->invalidateGroupEntries();
        }
        else
        {
            GFE_GroupUnion::groupUnion(*getOutGroupArray()[0], geoUnsharedVertexGroup);
        }
        
        if (doDelGroupElement)
            delGroupElement();
        
        //GFE_GroupUnion::groupUnion_topoAttrib(geo, *getOutGroupArray()[0], unsharedVertexGroup);
    

        return true;
    }




public:
    bool preFusePoint = false;
    fpreal fuseDist = 1e-05;

private:
    GFE_MeshTopology meshTopology;
    
    
    exint subscribeRatio = 64;
    exint minGrainSize = 64;
    
    //GA_VertexGroup* unsharedVertexGroup = nullptr;
    //GA_EdgeGroup*   unsharedEdgeGroup   = nullptr;

    //exint subscribeRatio = 64;
    //exint minGrainSize = 1024;
    
};// End of class GFE_GroupUnshared



#endif