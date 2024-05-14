
#pragma once

#ifndef __GFE_FlatEdge_h__
#define __GFE_FlatEdge_h__

#include "GFE/GFE_FlatEdge.h"

#include "GFE/GFE_GeoFilter.h"


#include "GFE/GFE_Math.h"
#include "GFE/GFE_MeshTopology.h"
#include "GFE/GFE_Normal3D.h"


class GFE_FlatEdge : public GFE_AttribFilter {

public:
    enum NonManifoldEdgeOp
    {
        None,
        All,
        Min,
        Max,
    };
    
    //using GFE_AttribFilter::GFE_AttribFilter;

    GFE_FlatEdge(
        GA_Detail* const geo,
        const SOP_NodeVerb::CookParms* const cookparms = nullptr
    )
        : GFE_AttribFilter(geo, cookparms)
        , normal3D(geo, cookparms)
    {
    }

    GFE_FlatEdge(
        GA_Detail& geo,
        const SOP_NodeVerb::CookParms* const cookparms = nullptr
    )
        : GFE_AttribFilter(geo, cookparms)
        , normal3D(geo, cookparms)
    {
    }

    ~GFE_FlatEdge()
    {
    }

    void
        setComputeParm(
            const bool includeUnsharedEdge = true,
            const NonManifoldEdgeOp nonManifoldEdgeOp = NonManifoldEdgeOp::None,
            const bool absDot = true,
            
            const exint subscribeRatio = 64,
            const exint minGrainSize = 1024
        )
    {
        setHasComputed();
        this->includeUnsharedEdge = includeUnsharedEdge;
        this->nonManifoldEdgeOp   = nonManifoldEdgeOp;
        this->absDot         = absDot;
        
        this->subscribeRatio = subscribeRatio;
        this->minGrainSize = minGrainSize;
    }

    SYS_FORCE_INLINE void setAngleThresholdByDegrees(const fpreal threshold)
    { flatEdgeAngleThreshold = fmod(GFE_Math::radians(threshold), PI); }

    
    SYS_FORCE_INLINE GA_Attribute* findOrCreateNormal3D(
        const bool findNormal3D,
        const bool detached,
        const GFE_NormalSearchOrder searchOrder,
        const GA_Storage storage = GA_STORE_INVALID,
        const UT_StringRef& attribName = "",
        const int tupleSize = 3
        )
    { return normal3D.findOrCreateNormal3D(findNormal3D, detached,
        findNormal3D ? searchOrder : GFE_NormalSearchOrder::Primitive, storage, attribName, tupleSize); }

    
    
private:

virtual bool
    computeCore() override
{
    if (getOutGroupArray().isEmpty())
        return true;

    
    outVertexGroup = nullptr;
    outEdgeGroup   = nullptr;
    const size_t size = getOutGroupArray().size();
    for (size_t i = 0; i < size; ++i)
    {
        switch (getOutGroupArray()[i]->classType())
        {
        case GA_GROUP_VERTEX: outVertexGroup = getOutGroupArray().getVertexGroup(i); break;
        case GA_GROUP_EDGE:   outEdgeGroup   = getOutGroupArray().getEdgeGroup(i);   break;
        default: break;
        }
    }
    
    GA_VertexGroupUPtr vertexGroupUPtr;
    if (!outVertexGroup)
    {
        if (outEdgeGroup)
        {
            vertexGroupUPtr = geo->createDetachedVertexGroup();
            outVertexGroup = vertexGroupUPtr.get();
        }
        else
        {
            return false;
        }
    }

    if (groupParser.isEmpty())
        return true;

    
    flatEdgeAngleThreshold = abs(flatEdgeAngleThreshold);
    
    normal3DAttrib = normal3D.getAttrib();
    if (!normal3DAttrib)
        normal3DAttrib = normal3D.findOrCreateNormal3D(false, true, GA_GROUP_PRIMITIVE);
    normal3D.compute();
    
    normal3DOwner = normal3DAttrib->getOwner();
    if (!GFE_Type::isElementGroup(normal3DOwner))
        return false;

    
    GFE_MeshTopology meshTopology(geo, cookparms);
    vertexNextEquivNoLoopGroup = meshTopology.setVertexNextEquivNoLoopGroup();
    vertexNextEquiv_h          = meshTopology.setVertexNextEquiv();
    meshTopology.compute();

    groupSetter = outVertexGroup;
    switch (normal3DAttrib->getAIFTuple()->getStorage(normal3DAttrib))
    {
    case GA_STORE_REAL16: flatEdge<fpreal16>(); break;
    case GA_STORE_REAL32: flatEdge<fpreal32>(); break;
    case GA_STORE_REAL64: flatEdge<fpreal64>(); break;
    default: break;
    }

    if (outEdgeGroup)
        GFE_GroupUnion::groupUnion(outEdgeGroup, outVertexGroup);
        
    if (doDelGroupElement && outEdgeGroup)
        delGroupElement(outEdgeGroup);
    
    return true;
}


template<typename FLOAT_T>
void flatEdge()
{
    const GA_ROHandleT<UT_Vector3T<FLOAT_T>> normal_h(normal3DAttrib);

    const GA_Topology& topo = geo->getTopology();
    const GA_ATITopology& vtxPrim  = *topo.getPrimitiveRef();
    const GA_ATITopology& vtxPoint = *topo.getPointRef();

    
    const FLOAT_T flatEdgeThreshold = cos(flatEdgeAngleThreshold);
    UTparallelFor(geo->getVertexSplittableRange(vertexNextEquivNoLoopGroup), [this, &normal_h, &vtxPrim, &vtxPoint, flatEdgeThreshold](const GA_SplittableRange& r)
    {
        // GA_PageHandleT<UT_Vector3T<FLOAT_T>, T, true, true, GA_Attribute, GA_ATINumeric, GA_Detail> normal_ph(normalAttrib);
        // GA_PageHandleT<GA_Offset, GA_Offset, true, true, GA_Attribute, GA_ATINumeric, GA_Detail> vertexNextEquivNoLoop_ph(vertexNextEquivNoLoopAttrib);
        //GA_PageHandleT<GA_Offset, GA_Offset, true, false, const GA_Attribute, const GA_ATINumeric, const GA_Detail> vtxNextEquiv_ph(vertexNextEquivAttrib);
        for (GA_PageIterator pit = r.beginPages(); !pit.atEnd(); ++pit)
        {
            GA_Offset start, end;
            for (GA_Iterator it(pit.begin()); it.blockAdvance(start, end); )
            {
                //vtxNextEquiv_ph.setPage(start);
                for (GA_Offset vtxoff = start; vtxoff < end; ++vtxoff)
                {
                    const GA_Offset vtxNextEquiv = vertexNextEquiv_h.get(vtxoff);
                    if (GFE_Type::isInvalidOffset(vtxNextEquiv))
                    {
                        groupSetter.set(vtxoff, includeUnsharedEdge);
                        continue;
                    }
                    
                    FLOAT_T dotVal;
                    if (vertexNextEquiv_h.get(vtxNextEquiv) == vtxoff)
                    {
                        switch (normal3DOwner)
                        {
                        case GA_ATTRIB_PRIMITIVE: dotVal = normal_h.get(vtxPrim.getLink(vtxoff)) .dot(normal_h.get(vtxPrim.getLink(vtxNextEquiv)));  break;
                        case GA_ATTRIB_POINT:     dotVal = normal_h.get(vtxPoint.getLink(vtxoff)).dot(normal_h.get(vtxPoint.getLink(vtxNextEquiv))); break;
                        case GA_ATTRIB_VERTEX:    dotVal = normal_h.get(vtxoff)                  .dot(normal_h.get(vtxNextEquiv));                   break;
                        default: break;
                        }
                    }
                    else
                    {
                        if (nonManifoldEdgeOp == NonManifoldEdgeOp::None)
                        {
                            groupSetter.set(vtxoff, false);
                            continue;
                        }
                        else if (nonManifoldEdgeOp == NonManifoldEdgeOp::All)
                        {
                            groupSetter.set(vtxoff, true);
                            continue;
                        }
                        else
                        {
                            switch (nonManifoldEdgeOp)
                            {
                            case NonManifoldEdgeOp::Max:  break;
                            case NonManifoldEdgeOp::Min:  break;
                            default:                      break;
                            }
                        }
                    }
                    if (absDot)
                        dotVal = abs(dotVal);

                    groupSetter.set(vtxoff, dotVal > flatEdgeThreshold);
                }
            }
        }
    }, subscribeRatio, minGrainSize);
    groupSetter.invalidateGroupEntries();
}




public:
    GFE_Normal3D normal3D;
    
    bool includeUnsharedEdge = true;
    NonManifoldEdgeOp nonManifoldEdgeOp;
    bool absDot = true;
    
    fpreal flatEdgeAngleThreshold = 1e-05;
    
private:
    GA_VertexGroup* outVertexGroup;
    GA_EdgeGroup*   outEdgeGroup;
    const GA_Attribute* normal3DAttrib;
    const GA_VertexGroup* vertexNextEquivNoLoopGroup;
    GA_ROHandleT<GA_Offset> vertexNextEquiv_h;
    
    fpreal flatEdgeThreshold;
    GA_AttributeOwner normal3DOwner;
    
    exint subscribeRatio = 64;
    exint minGrainSize = 1024;


}; // End of class GFE_FlatEdge



#endif
