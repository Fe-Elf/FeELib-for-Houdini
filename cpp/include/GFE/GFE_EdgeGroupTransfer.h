
#pragma once

#ifndef __GFE_EdgeGroupTransfer_h__
#define __GFE_EdgeGroupTransfer_h__

#include "GFE/GFE_EdgeGroupTransfer.h"

#include "GFE/GFE_GeoFilter.h"


#include "UFE/UFE_SplittableString.h"
#include "GFE/GFE_MeshTopology.h"


#include "GU/GU_Snap.h"



// Ref GFE_GroupNewEdge
class GFE_EdgeGroupTransfer : public GFE_AttribFilterWithRef0 {

public:
    using GFE_AttribFilterWithRef0::GFE_AttribFilterWithRef0;

    ~GFE_EdgeGroupTransfer()
    {
    }

    

    void
    setComputeParm(
        const bool useSnapDist = true,
        const fpreal snapDist = 0.001,
        const bool outAsVertexGroup = false,
        const exint subscribeRatio = 64,
        const exint minGrainSize = 1024
    )
    {
        setHasComputed();
        this->useSnapDist      = useSnapDist;
        this->snapDist         = snapDist;
        this->outAsVertexGroup = outAsVertexGroup;
        this->subscribeRatio   = subscribeRatio;
        this->minGrainSize     = minGrainSize;
    }

    


private:

// can not use in parallel unless for each GA_Detail
virtual bool
    computeCore() override
{
    if (getOutGroupArray().isEmpty())
        return false;
    
    if (groupParser.isEmpty())
        return true;

    geoRef0Tmp = new GU_Detail();
    geoRef0_h.allocateAndSet(geoRef0Tmp);
    geoRef0Tmp->replaceWith(*geoRef0->asGA_Detail());


    
    const GA_ATINumericUPtr snapPtoffAttribUPtr = geoRef0->getAttributes().
        createDetachedTupleAttribute(GA_ATTRIB_POINT, GA_STORE_INT64, 1, GA_Defaults(GFE_INVALID_OFFSET));
    
    snapPtoffAttrib = snapPtoffAttribUPtr.get();
    //const GA_RWHandleID snapPtoff_h(snapPtoffAttribPtr);


    GU_Snap::PointSnapParms pointSnapParms;
    pointSnapParms.myAlgorithm = GU_Snap::PointSnapParms::ALGORITHM_CLOSEST_POINT;
    pointSnapParms.myDistance = useSnapDist ? snapDist : SYS_FP64_MAX;
    pointSnapParms.myOutputAttribH = snapPtoffAttrib;
    snapPtoff_h = snapPtoffAttrib;

    GU_Snap::snapPoints(*geoRef0Tmp, geo->asGU_Detail(), pointSnapParms);
    //GU_Snap::snapPoints(*geo->asGU_Detail(), geoRef0, pointSnapParms);


    
    //GFE_MeshTopology meshTopology(geo, cookparms);
    //vertexPointDstAttrib = meshTopology.setVertexPointDst(true);
    //meshTopology.compute();
    //dstpt_h = vertexPointDstAttrib;
    
    GFE_MeshTopology meshTopologyRef0(geoRef0Tmp);
    vertexPointDstRef0Attrib = meshTopologyRef0.setVertexPointDst(true);
    meshTopologyRef0.compute();

    
    
    const auto arrayLen = getRef0GroupArray().size();
    for (size_t i = 0; i < arrayLen; i++)
    {
        switch (getRef0GroupArray()[i]->classType())
        {
        case GA_GROUP_EDGE:
            edgeGroup = getRef0GroupArray().getEdgeGroup(i);
            edgeGroupTransfer();
            break;
        case GA_GROUP_VERTEX:
            vertexEdgeGroup = getRef0GroupArray().getVertexGroup(i);
            vertexEdgeGroupTransfer();
            break;
        default: break;
        }
    }

    

    
    return true;
}

void edgeGroupTransfer()
{
    const UT_StringHolder& newName = newEdgeGroupNames.getIsValid() ?
                                     newEdgeGroupNames.getNext<UT_StringHolder>() :
                                     edgeGroup->getName();
    
    const bool detached = !GFE_Type::isPublicAttribName(newName);

    GA_RWHandleT<GA_Offset> dstptRef0_h(vertexPointDstRef0Attrib);
    
    if ( outAsVertexGroup )
    {
        newVertexEdgeGroup = getOutGroupArray().findOrCreateVertex(detached, newName);
        newVertexEdgeGroup->clear();
        
        for (GA_EdgeGroup::const_iterator it = edgeGroup->begin(); it.atEnd(); ++it)
        {
            const GA_Edge& edge = *it;
            
            const GA_Offset snapDstPtoff0 = snapPtoff_h.get(edge.p0());
            if (GFE_Type::isInvalidOffset(snapDstPtoff0))
                continue;
                    
            const GA_Offset snapDstPtoff1 = snapPtoff_h.get(edge.p1());
            if (GFE_Type::isInvalidOffset(snapDstPtoff1))
                continue;

            const GA_Offset vertexFromEdge = geo->edgeVertex(snapDstPtoff0, snapDstPtoff1);
            UT_ASSERT_MSG(GFE_Type::isInvalidOffset(vertexFromEdge), "not valid");
            newVertexEdgeGroup->setElement(vertexFromEdge, true);
        }
    }
    else
    {
        newEdgeGroup = getOutGroupArray().findOrCreateEdge(detached, newName);
        newEdgeGroup->clear();
        
        for (GA_EdgeGroup::const_iterator it = edgeGroup->begin(); it.atEnd(); ++it)
        {
            const GA_Edge& edge = *it;
            
            const GA_Offset snapDstPtoff0 = snapPtoff_h.get(edge.p0());
            if (GFE_Type::isInvalidOffset(snapDstPtoff0))
                continue;
                    
            const GA_Offset snapDstPtoff1 = snapPtoff_h.get(edge.p1());
            if (GFE_Type::isInvalidOffset(snapDstPtoff1))
                continue;
                    
            newEdgeGroup->add(snapDstPtoff0, snapDstPtoff1);
        }
    }
}

void vertexEdgeGroupTransfer()
{
    const UT_StringHolder& newName = newVertexEdgeGroupNames.getIsValid() ?
                                     newVertexEdgeGroupNames.getNext<UT_StringHolder>() :
                                     vertexEdgeGroup->getName();
    const bool detached = !GFE_Type::isPublicAttribName(newName);

    const GA_SplittableRange geoRefSplittableRange(geoRef0->getVertexRange(vertexEdgeGroup));
    
    if (outAsVertexGroup)
    {
        newVertexEdgeGroup = getOutGroupArray().findOrCreateVertex(detached, newName);
        newVertexEdgeGroup->clear();
        UTparallelFor(geoRefSplittableRange, [this](const GA_SplittableRange& r)
        {
            GA_PageHandleT<GA_Offset, GA_Offset, true, false, const GA_Attribute, const GA_ATINumeric, const GA_Detail> dstptRef0_ph(vertexPointDstRef0Attrib);

            for (GA_PageIterator pit = r.beginPages(); !pit.atEnd(); ++pit)
            {
                GA_Offset start, end;
                for (GA_Iterator it(pit.begin()); it.blockAdvance(start, end); )
                {
                    dstptRef0_ph.setPage(start);
                    for (GA_Offset elemoff = start; elemoff < end; ++elemoff)
                    {
                        const GA_Offset ptoff = geoRef0->vertexPoint(elemoff);
                        
                        const GA_Offset snapDstPtoff0 = snapPtoff_h.get(ptoff);
                        if (GFE_Type::isInvalidOffset(snapDstPtoff0))
                            continue;
                        
                        const GA_Offset snapDstPtoff1 = snapPtoff_h.get(dstptRef0_ph.value(elemoff));
                        if (GFE_Type::isInvalidOffset(snapDstPtoff1))
                            continue;
                        
                        if (1)
                        {
                            const GA_Offset vertexFromEdge = geo->edgeVertex(snapDstPtoff0, snapDstPtoff1);
                            UT_ASSERT_MSG(GFE_Type::isValidOffset(vertexFromEdge), "not valid");
                            newVertexEdgeGroup->setElement(vertexFromEdge, true);
                        }
                        else
                        {
                            // for (GA_Offset vtxoff = geo->pointVertex(snapDstPtoff0); vtxoff != GA_INVALID_OFFSET; vtxoff = geo->vertexToNextVertex(vtxoff))
                            // {
                            //     if (dstpt_h.get(vtxoff) == snapDstPtoff1)
                            //     {
                            //         newVertexEdgeGroup->setElement(vtxoff, true);
                            //         break;
                            //     }
                            // }
                        }
                    }
                }
            }
        }, subscribeRatio, minGrainSize);
        newVertexEdgeGroup->invalidateGroupEntries();
    }
    else
    {
        newEdgeGroup = getOutGroupArray().findOrCreateEdge(detached, newName);
        newEdgeGroup->clear();
        
        GA_PageHandleT<GA_Offset, GA_Offset, true, false, const GA_Attribute, const GA_ATINumeric, const GA_Detail> dstptRef0_ph(vertexPointDstRef0Attrib);

        for (GA_PageIterator pit = geoRefSplittableRange.beginPages(); !pit.atEnd(); ++pit)
        {
            GA_Offset start, end;
            for (GA_Iterator it(pit.begin()); it.blockAdvance(start, end); )
            {
                dstptRef0_ph.setPage(start);
                for (GA_Offset elemoff = start; elemoff < end; ++elemoff)
                {
                    const GA_Offset ptoff = geoRef0->vertexPoint(elemoff);
                    
                    const GA_Offset snapDstPtoff0 = snapPtoff_h.get(ptoff);
                    if (GFE_Type::isInvalidOffset(snapDstPtoff0))
                        continue;
                    
                    const GA_Offset snapDstPtoff1 = snapPtoff_h.get(dstptRef0_ph.value(elemoff));
                    if (GFE_Type::isInvalidOffset(snapDstPtoff1))
                        continue;

                    newEdgeGroup->add(snapDstPtoff0, snapDstPtoff1);
                }
            }
        }
    }
}

public:
    
    bool useSnapDist = true;
    fpreal snapDist = 0.001;

    bool outAsVertexGroup = false;
    
    UFE_SplittableString newEdgeGroupNames;
    UFE_SplittableString newVertexEdgeGroupNames;

private:
    GU_DetailHandle geoRef0_h;
    GU_Detail* geoRef0Tmp;

    
    const GA_EdgeGroup* edgeGroup;
    const GA_VertexGroup* vertexEdgeGroup;
    
    GA_EdgeGroup* newEdgeGroup;
    GA_VertexGroup* newVertexEdgeGroup;
    
    GA_Attribute* snapPtoffAttrib;
    GA_RWHandleT<GA_Offset> snapPtoff_h;
    
    GA_Attribute* vertexPointDstRef0Attrib;
    
    //GA_Attribute* vertexPointDstAttrib;
    //GA_RWHandleT<GA_Offset> dstpt_h;
        
    
    exint subscribeRatio = 64;
    exint minGrainSize = 1024;
    
}; // End of class GFE_EdgeGroupTransfer



#endif
