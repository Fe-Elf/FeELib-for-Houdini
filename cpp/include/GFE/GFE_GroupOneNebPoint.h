
#pragma once

#ifndef __GFE_GroupOneNebPoint_h__
#define __GFE_GroupOneNebPoint_h__

#include "GFE/GFE_GroupOneNebPoint.h"

#include "GFE/GFE_GeoFilter.h"



#include "GU/GU_Snap.h"

class GFE_GroupOneNebPoint : public GFE_AttribFilter {

public:
    using GFE_AttribFilter::GFE_AttribFilter;

    ~GFE_GroupOneNebPoint()
    {
    }

    
    void
        setComputeParm(
            const bool preFusePoint    = false,
            const fpreal fuseDist      = 1e-05,
            const exint subscribeRatio = 64,
            const exint minGrainSize   = 64
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
        if (getOutGroupArray().isEmpty() || getOutGroupArray()[0]->classType() != GA_GROUP_POINT)
            return false;
        
        if (groupParser.isEmpty())
            return true;

        GU_DetailHandle geoOrigin_h;
        if (preFusePoint)
        {
            geoOriginTmp = new GU_Detail();
            geoOrigin_h.allocateAndSet(geoOriginTmp);
            geoOriginTmp->replaceWith(*geo);

            fuseParms.myDistance = fuseDist;
            fuseParms.myAlgorithm = GU_Snap::PointSnapParms::SnapAlgorithm::ALGORITHM_CLOSEST_POINT;
            fuseParms.myConsolidate = true;
            fuseParms.myDeleteConsolidated = true;
            fuseParms.myQPosH = geoOriginTmp->getP();
            fuseParms.myTPosH = geoOriginTmp->getP();
            GU_Snap::snapPoints(*geoOriginTmp, nullptr, fuseParms);
            //GU_Snap::snapPoints(*geoOriginTmp, static_cast<const GU_Detail*>(geo), fuseParms);
        }
        else
        {
            geoOriginTmp = geo->asGU_Detail();
        }
        
        groupSetter = getOutGroupArray().getPointGroup(0);


        
        const GA_Topology& topo = geo->getTopology();
        //topo.makeVertexRef();
        vtxPointRef = topo.getPointRef();
        pointVtxRef = topo.getVertexRef();
        vtxNextRef  = topo.getVertexNextRef();
        
        switch (groupParser.classType())
        {
        default:
        case GA_GROUP_PRIMITIVE: groupOneNeb<GA_GROUP_PRIMITIVE>(); break;
        case GA_GROUP_POINT:     groupOneNeb<GA_GROUP_POINT>    (); break;
        case GA_GROUP_VERTEX:    groupOneNeb<GA_GROUP_VERTEX>   (); break;
        }

        return true;
    }

    template<GA_GroupType GroupType_T>
    void groupOneNeb()
    {
        //UT_ASSERT_P(geoGroup);
        

        if constexpr (GroupType_T == GA_GROUP_PRIMITIVE || GroupType_T == GA_GROUP_POINT || GroupType_T == GA_GROUP_VERTEX)
            geoElementGroup = groupParser.getElementGroup();
        else
            geoElementGroup = nullptr;
        
        UTparallelFor(groupParser.getPrimitiveSplittableRange(), [this](const GA_SplittableRange& r)
        {
            GA_Offset vtxoff, vtxoff_next, ptoff;
            GA_Offset start, end;
            for (GA_Iterator it(r); it.blockAdvance(start, end); )
            {
                for (GA_Offset elemoff = start; elemoff < end; ++elemoff)
                {
                    const GA_Size numvtx = geo->getPrimitiveVertexCount(elemoff);
                    if (numvtx <= 0)
                        continue;

                    groupOneNeb<GroupType_T>(elemoff, 0);
                    
                    if (numvtx <= 1)
                        continue;
                    
                    groupOneNeb<GroupType_T>(elemoff, numvtx-1);
                }
            }
        }, subscribeRatio, minGrainSize);
        groupSetter.invalidateGroupEntries();
    }

    template<GA_GroupType GroupType_T>
    SYS_FORCE_INLINE void groupOneNeb(const GA_Offset primoff, const GA_Size vtxpnum)
    {
        const GA_Offset vtxoff = geo->getPrimitiveVertexOffset(primoff, vtxpnum);
        if constexpr (GroupType_T == GA_GROUP_VERTEX)
            if (!geoElementGroup->contains(vtxoff))
                return;
                    
        const GA_Offset ptoff = vtxPointRef->getLink(vtxoff);
        if constexpr (GroupType_T == GA_GROUP_POINT)
            if (!geoElementGroup->contains(ptoff))
                return;
        
        GA_Offset vtxoff_next = pointVtxRef->getLink(ptoff);
        if (vtxoff_next != vtxoff)
            return;
        
        vtxoff_next = vtxNextRef->getLink(vtxoff_next);
        if (GFE_Type::isValidOffset(vtxoff_next))
            return;
        
        groupSetter.set(ptoff, true);
    }

    SYS_FORCE_INLINE GA_Offset getPrimitivePointOffset(const GU_Detail* const geo, const GA_Offset primoff, const GA_Size vtxpnum) const
    { return geo->vertexPoint(geo->getPrimitiveVertexOffset(primoff, vtxpnum)); }



public:
    bool preFusePoint = false;
    fpreal fuseDist = 1e-05;
    
private:
    GU_Snap::PointSnapParms fuseParms;
    
private:
    const GA_ATITopology* vtxPointRef;
    const GA_ATITopology* pointVtxRef;
    const GA_ATITopology* vtxNextRef;
    const GA_ElementGroup* geoElementGroup;
    
    GU_DetailHandle geoRef0_h;
    GU_Detail* geoRef0Tmp;
    GU_Detail* geoOriginTmp;
    
    exint subscribeRatio = 64;
    exint minGrainSize = 256;

}; // End of class GFE_GroupOneNebPoint











#endif
