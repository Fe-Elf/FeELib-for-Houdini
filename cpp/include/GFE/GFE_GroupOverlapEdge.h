
#pragma once

#ifndef __GFE_GroupOverlapEdge_h__
#define __GFE_GroupOverlapEdge_h__

#include "GFE/GFE_GroupOverlapEdge.h"

#include "GFE/GFE_GeoFilter.h"



#include "GFE/GFE_MeshTopology.h"

#include "GU/GU_Snap.h"

// Ref GFE_EdgeGroupTransfer
class GFE_GroupOverlapEdge : public GFE_AttribFilterWithRef0 {


public:
    using GFE_AttribFilterWithRef0::GFE_AttribFilterWithRef0;


    void
        setComputeParm(
            const bool useSnapDist = true,
            const fpreal snapDist = 0.001,
            const bool runOverGeoRef = false,
            
            const exint subscribeRatio = 64,
            const exint minGrainSize = 64
        )
    {
        setHasComputed();
        this->useSnapDist      = useSnapDist;
        this->snapDist         = snapDist;
        this->runOverGeoRef    = runOverGeoRef;
        this->subscribeRatio   = subscribeRatio;
        this->minGrainSize     = minGrainSize;
    }

    
    SYS_FORCE_INLINE void findOrCreateGroup(
        const bool detached = false,
        const bool outVertexEdgeGroup = false,
        const UT_StringRef& name = ""
    )
    {
        if(outVertexEdgeGroup)
            findOrCreateVertexGroup(detached, name);
        else
            findOrCreateEdgeGroup(detached, name);
    }

    SYS_FORCE_INLINE GA_VertexGroup* findOrCreateVertexGroup(
        const bool detached = false,
        const UT_StringRef& name = ""
    )
    { return vertexEdgeGroup = getOutGroupArray().findOrCreateVertex(detached, name); }

    SYS_FORCE_INLINE GA_EdgeGroup* findOrCreateEdgeGroup(
        const bool detached = false,
        const UT_StringRef& name = ""
    )
    { return edgeGroup = getOutGroupArray().findOrCreateEdge(detached, name); }

private:

    virtual bool
        computeCore() override
    {
        if (getOutGroupArray().isEmpty())
            return false;

        if (groupParser.isEmpty())
            return true;

        

    
        const GA_ATINumericUPtr snapPtoffAttribUPtr = (runOverGeoRef ? geoRef0 : geo)->getAttributes().
            createDetachedTupleAttribute(GA_ATTRIB_POINT, GA_STORE_INT64, 1, GA_Defaults(GFE_INVALID_OFFSET));
        snapPtoffAttrib = snapPtoffAttribUPtr.get();
        snappt_h = snapPtoffAttrib;
        
        GU_Snap::PointSnapParms pointSnapParms;
        pointSnapParms.myAlgorithm = GU_Snap::PointSnapParms::ALGORITHM_CLOSEST_POINT;
        pointSnapParms.myDistance = useSnapDist ? snapDist : SYS_FP64_MAX;
        pointSnapParms.myTPosH = (runOverGeoRef ? geo : geoRef0)->getP();
        pointSnapParms.myOutputAttribH = snapPtoffAttrib;

        if (runOverGeoRef)
        {
            geoRef0Tmp = new GU_Detail();
            geoRef0_h.allocateAndSet(geoRef0Tmp);
            geoRef0Tmp->replaceWith(*geoRef0->asGA_Detail());
        }
        GFE_MeshTopology meshTopology(geo, cookparms);
        GFE_MeshTopology meshTopologyRef0(geoRef0Tmp);
        if (runOverGeoRef)
        {
            pointSnapParms.myQPosH = geoRef0Tmp->getP();
            GU_Snap::snapPoints(*geoRef0Tmp, geo->asGU_Detail(), pointSnapParms);

            creatingGroup = meshTopologyRef0.setVertexNextEquivNoLoopGroup();
            vertexPointDstAttrib = meshTopologyRef0.setVertexPointDst();
            meshTopologyRef0.compute();
            
            if (groupSetter.reverseGroup)
            {
                meshTopology.groupParser.setGroup(groupParser);
                creatingGroupTemp = meshTopology.setVertexNextEquivNoLoopGroup();
                meshTopology.compute();
            }
        }
        else
        {
            pointSnapParms.myQPosH = geo->getP();
            GU_Snap::snapPoints(*geo->asGU_Detail(), geoRef0->asGU_Detail(), pointSnapParms);
            
            meshTopology.groupParser.setGroup(groupParser);
            creatingGroup = meshTopology.setVertexNextEquivNoLoopGroup();
            vertexPointDstAttrib = meshTopology.setVertexPointDst();
            meshTopology.compute();
            //dstpt_h = vertexPointDstAttrib;
        }

    
        const size_t size = getOutGroupArray().size();
        for (size_t i = 0; i < size; ++i)
        {
            switch (getOutGroupArray()[i]->classType())
            {
            //case GA_GROUP_PRIMITIVE:  groupNewEdge(getOutGroupArray().getPrimitiveGroup(i));  break;
            //case GA_GROUP_POINT:      groupNewEdge(getOutGroupArray().getPointGroup((i));     break;
            case GA_GROUP_VERTEX:     groupNewEdge(*getOutGroupArray().getVertexGroup(i));     break;
            case GA_GROUP_EDGE:       groupNewEdge(*getOutGroupArray().getEdgeGroup(i));       break;
            default: break;
            }
        }


        return true;
    }



    void groupNewEdge(GA_VertexGroup& group)
    {
        const GA_ATITopology& vtxPointRef = *(runOverGeoRef ? geoRef0 : geo)->getTopology().getPointRef();
        UTparallelFor((runOverGeoRef ? geoRef0 : geo)->getVertexSplittableRange(creatingGroup), [this, &group, &vtxPointRef](const GA_SplittableRange& r)
        {
            GA_PageHandleT<GA_Offset, GA_Offset, true, false, const GA_Attribute, const GA_ATINumeric, const GA_Detail> dstpt_ph(vertexPointDstAttrib);
            for (GA_PageIterator pit = r.beginPages(); !pit.atEnd(); ++pit)
            {
                GA_Offset start, end;
                for (GA_Iterator it(pit.begin()); it.blockAdvance(start, end); )
                {
                    dstpt_ph.setPage(start);
                    for (GA_Offset elemoff = start; elemoff < end; ++elemoff)
                    {
                        const GA_Offset ptoff0 = snappt_h.get(vtxPointRef.getLink(elemoff));
                        if (GFE_Type::isInvalidOffset(ptoff0))
                            continue;
                        const GA_Offset ptoff1 = snappt_h.get(dstpt_ph.value(elemoff));
                        if (GFE_Type::isInvalidOffset(ptoff1))
                            continue;
                        const GA_Offset vtxoff = (runOverGeoRef ? geo : geoRef0)->edgeVertex(ptoff0, ptoff1);
                        if (GFE_Type::isValidOffset(vtxoff))
                            group.setElement(runOverGeoRef ? vtxoff : elemoff, true);
                    }
                }
            }
        }, subscribeRatio, minGrainSize);
        group.invalidateGroupEntries();
        
        if (groupSetter.reverseGroup)
        {
            group.toggleEntries();
            if (runOverGeoRef)
            {
                UT_ASSERT_P(creatingGroupTemp);
                group &= *creatingGroupTemp;
            }
            else
            {
                group &= *creatingGroup;
            }
        }
    }


    void groupNewEdge(GA_EdgeGroup& group)
    {
#if 1
        const GA_VertexGroupUPtr vtxGroupUPtr = geo->createDetachedVertexGroup();
        GA_VertexGroup& vtxGroup = *vtxGroupUPtr.get();
        groupNewEdge(vtxGroup);
        GFE_GroupUnion::groupUnion(group, vtxGroup);
#else
        const GA_ATITopology& vtxPointRef = *(runOverGeoRef ? geoRef0 : geo)->getTopology().getPointRef();
        const GA_SplittableRange gepSplittableRange = (runOverGeoRef ? geoRef0 : geo)->getVertexSplittableRange(creatingGroup);
    
        GA_PageHandleT<GA_Offset, GA_Offset, true, false, const GA_Attribute, const GA_ATINumeric, const GA_Detail> dstpt_ph(vertexPointDstAttrib);
        for (GA_PageIterator pit = gepSplittableRange.beginPages(); !pit.atEnd(); ++pit)
        {
            GA_Offset start, end;
            for (GA_Iterator it(pit.begin()); it.blockAdvance(start, end); )
            {
                dstpt_ph.setPage(start);
                for (GA_Offset elemoff = start; elemoff < end; ++elemoff)
                {
                    const GA_Offset ptoff0 = snappt_h.get(vtxPointRef.getLink(elemoff));
                    if (GFE_Type::isInvalidOffset(ptoff0))
                        continue;
                    const GA_Offset ptoff1 = snappt_h.get(dstpt_ph.value(elemoff));
                    if (GFE_Type::isInvalidOffset(ptoff1))
                        continue;
                    //const GA_Offset vtxoff = (runOverGeoRef ? geo : geoRef0)->edgeVertex(ptoff0, ptoff1);
                    //if (GFE_Type::isValidOffset(vtxoff))
                    if (runOverGeoRef)
                        group.add(ptoff0, ptoff1);
                    else
                        group.add(vtxPointRef.getLink(elemoff), dstpt_ph.value(elemoff));
                }
            }
        }
        
        if (!groupSetter.reverseGroup)
            group.toggleEntries();
#endif
    }

    


public:
    bool useSnapDist = true;
    fpreal snapDist = 0.001;
    bool runOverGeoRef = false;

private:
    GU_DetailHandle geoRef0_h;
    GU_Detail* geoRef0Tmp;

    
    GA_EdgeGroup* edgeGroup = nullptr;
    GA_VertexGroup* vertexEdgeGroup = nullptr;

    
    GA_Attribute* snapPtoffAttrib;
    //GA_ROHandleT<GA_Offset> snapPtoff_h;
    GA_ROHandleT<GA_Offset> snappt_h;

    const GA_VertexGroup* creatingGroup;
    const GA_VertexGroup* creatingGroupTemp;
    const GA_Attribute* vertexPointDstAttrib;
    //GA_ROHandleT<GA_Offset> dstpt_h;
    
    exint subscribeRatio = 64;
    exint minGrainSize = 1024;


}; // End of class GFE_GroupOverlapEdge





#endif
