
#pragma once

#ifndef __GFE_GroupBoolean_h__
#define __GFE_GroupBoolean_h__

#include "GFE/GFE_GroupBoolean.h"

#include "GA/GA_Detail.h"


//#include "GFE/GFE_GroupUnion.h"
//#include "GFE/GFE_GroupPromote.h"

//#include "GA/GA_SplittableRange.h"

//#include "GFE/GFE_Type.h"

//#include "GU/GU_Group.h"


//enum GA_GroupBooleanOp
//{
//    GA_GROUP_BOOLEAN_COPY,
//    GA_GROUP_BOOLEAN_OR,
//    GA_GROUP_BOOLEAN_AND,
//    GA_GROUP_BOOLEAN_XOR,
//    GA_GROUP_BOOLEAN_MINUS,
//};





namespace GFE_GroupBoolean
{
    
    
    static void groupIntersect(GA_VertexGroup& group, const GA_PointGroup* const groupRef,
        const exint subscribeRatio = 64, const exint minGrainSize = 1024
    )
    {
        if (!groupRef)
            return;
        /*
        if (group.entries() > groupRef->entries())
        {
        }
        else
        {
        }
        */
        const GA_Detail& geo = groupRef->getDetail();
        const GA_SplittableRange geoSplittableRange(GA_Range(groupRef->getIndexMap(), groupRef, true));
        UTparallelFor(geoSplittableRange, [&geo, &group](const GA_SplittableRange& r)
        {
            GA_Offset start, end;
            for (GA_Iterator it(r); it.blockAdvance(start, end); )
            {
                for (GA_Offset elemoff = start; elemoff < end; ++elemoff)
                {
                    for (GA_Offset vtxoff_next = geo.pointVertex(elemoff); GFE_Type::isValidOffset(vtxoff_next); vtxoff_next = geo.vertexToNextVertex(vtxoff_next))
                    {
                        group.setElement(vtxoff_next, false);
                    }
                }
            }
        }, subscribeRatio, minGrainSize);
    }
    
} // End of namespace GFE_GroupBoolean



/*
namespace GFE_GroupBoolean {

    

    SYS_FORCE_INLINE
        static void
        groupIntersect(
            const GA_Detail* const geo,
            GA_EdgeGroup* const group,
            const GA_PrimitiveGroup* const groupRef
        )
    {
        UT_ASSERT_P(geo);
        UT_ASSERT_P(group);
        if (!groupRef)
            return;
        GA_GroupType groupTypeRef = groupRef->classType();

        GA_EdgeGroupUPtr restGroupUptr = geo->createDetachedEdgeGroup();
        GA_EdgeGroup* restGroup = restGroupUptr.get();
        restGroup->combine(group);
        group->clear();

        const GA_Topology& topo = geo->getTopology();
        const GA_ATITopology* vtxPointRef = topo.getPointRef();

        const GA_Range range = geo->getPrimitiveRange(static_cast<const GA_PrimitiveGroup*>(groupRef));
        GA_Offset start, end;
        for (GA_Iterator it(range); it.fullBlockAdvance(start, end); )
        {
            for (GA_Offset elemoff = start; elemoff < end; ++elemoff)
            {
                const GA_OffsetListRef& vertices = geo->getPrimitiveVertexList(elemoff);
                const GA_Size numvtx = vertices.size();

                GA_Offset vtxoff_prev = vtxPointRef->getLink(vertices[0]);
                GA_Offset vtxoff_next;
                for (GA_Size vtxpnum = 1; vtxpnum < numvtx; ++vtxpnum)
                {
                    vtxoff_next = vtxPointRef->getLink(vertices[vtxpnum]);
                    if (restGroup->contains(GA_Edge(vtxoff_prev, vtxoff_next)))
                        group->add(vtxoff_prev, vtxoff_next);
                    vtxoff_prev = vtxoff_next;
                }
                if (geo->getPrimitiveClosedFlag(elemoff))
                {
                    vtxoff_next = vtxPointRef->getLink(vertices[0]);
                    if (restGroup->contains(GA_Edge(vtxoff_prev, vtxoff_next)))
                        group->add(vtxoff_prev, vtxoff_next);
                    group->add(vtxoff_prev, vtxoff_next);
                }
            }
        }
    }

    SYS_FORCE_INLINE
        static void
        groupIntersect(
            const GA_Detail* const geo,
            GA_EdgeGroup* const group,
            const GA_PointGroup* const groupRef
        )
    {
        UT_ASSERT_P(geo);
        UT_ASSERT_P(group);
        if (!groupRef)
            return;
        GA_GroupType groupTypeRef = groupRef->classType();

        GA_EdgeGroupUPtr restGroupUptr = geo->createDetachedEdgeGroup();
        GA_EdgeGroup* restGroup = restGroupUptr.get();
        restGroup->combine(group);
        group->clear();

        const GA_Topology& topo = geo->getTopology();
        const GA_ATITopology* vtxPointRef = topo.getPointRef();

        //topo.makePrimitiveRef();
        //topo.makeVertexRef();
        const GA_ATITopology* pointVtxRef = topo.getVertexRef();
        const GA_ATITopology* vtxPrimRef = topo.getPrimitiveRef();
        const GA_ATITopology* vtxNextRef = topo.getVertexNextRef();

        const GA_Range range = geo->getPointRange(static_cast<const GA_PointGroup*>(groupRef));
        GA_Offset start, end;
        for (GA_Iterator it(range); it.fullBlockAdvance(start, end); )
        {
            for (GA_Offset elemoff = start; elemoff < end; ++elemoff)
            {
                GA_Offset pt_next;
                for (GA_Offset vtxoff_next = pointVtxRef->getLink(elemoff); vtxoff_next != GA_INVALID_OFFSET; vtxoff_next = vtxNextRef->getLink(vtxoff_next))
                {
                    const GA_Offset primoff = vtxPrimRef->getLink(vtxoff_next);
                    const GA_Size numvtx = geo->getPrimitiveVertexCount(primoff);
                    const GA_Size vtxpnum = GFE_TopologyReference::vertexPrimIndex(geo, primoff, vtxoff_next);

                    if (vtxpnum == 0)
                    {
                        if (geo->getPrimitiveClosedFlag(primoff))
                        {
                            pt_next = vtxPointRef->getLink(geo->getPrimitiveVertexOffset(primoff, numvtx - 1));
                            if (restGroup->contains(GA_Edge(elemoff, pt_next)))
                                group->add(elemoff, pt_next);
                        }
                    }
                    else
                    {
                        pt_next = vtxPointRef->getLink(geo->getPrimitiveVertexOffset(primoff, vtxpnum - 1));
                        if (restGroup->contains(GA_Edge(elemoff, pt_next)))
                            group->add(elemoff, pt_next);
                    }

                    const GA_Size vtxpnum_next = vtxpnum + 1;
                    if (vtxpnum_next == numvtx) {
                        if (geo->getPrimitiveClosedFlag(primoff))
                        {
                            pt_next = vtxPointRef->getLink(geo->getPrimitiveVertexOffset(primoff, 0));
                            if (restGroup->contains(GA_Edge(elemoff, pt_next)))
                                group->add(elemoff, pt_next);
                        }
                    }
                    else
                    {
                        pt_next = vtxPointRef->getLink(geo->getPrimitiveVertexOffset(primoff, vtxpnum_next));
                        if (restGroup->contains(GA_Edge(elemoff, pt_next)))
                            group->add(elemoff, pt_next);
                    }
                }
            }
        }
    }
    SYS_FORCE_INLINE
        static void
        groupIntersect(
            const GA_Detail* const geo,
            GA_EdgeGroup* const group,
            const GA_VertexGroup* const groupRef
        )
    {
        UT_ASSERT_P(geo);
        UT_ASSERT_P(group);
        if (!groupRef)
            return;
        GA_GroupType groupTypeRef = groupRef->classType();

        GA_EdgeGroupUPtr restGroupUptr = geo->createDetachedEdgeGroup();
        GA_EdgeGroup* restGroup = restGroupUptr.get();
        restGroup->combine(group);
        group->clear();

        const GA_Topology& topo = geo->getTopology();
        const GA_ATITopology* vtxPointRef = topo.getPointRef();

        const GA_Range range = geo->getVertexRange(static_cast<const GA_VertexGroup*>(groupRef));
        GA_Offset start, end;
        for (GA_Iterator it(range); it.fullBlockAdvance(start, end); )
        {
            for (GA_Offset elemoff = start; elemoff < end; ++elemoff)
            {
                const GA_Offset ptoff = vtxPointRef->getLink(elemoff);
                const GA_Offset dstpt = GFE_TopologyReference::vertexPointDst(geo, elemoff);
                if (restGroup->contains(GA_Edge(ptoff, dstpt)))
                    group->add(ptoff, dstpt);
            }
        }
    }

    SYS_FORCE_INLINE
        static void
        groupIntersect(
            const GA_Detail* const geo,
            GA_EdgeGroup* const group,
            const GA_EdgeGroup* const groupRef
        )
    {
        if (!groupRef)
            return;
        *group &= *groupRef;
        //for (GA_EdgeGroup::iterator it = group->begin(); !it.atEnd(); it.advance())
        //{
        //    if (groupRef->contains(it.getEdge()))
        //    {

        //    }
        //}
    }


    static void
        groupIntersect(
            const GA_Detail* const geo,
            GA_EdgeGroup* const group,
            const GA_Group* const groupRef
        )
    {
        UT_ASSERT_P(geo);
        UT_ASSERT_P(group);
        if (!groupRef)
            return;
        GA_GroupType groupTypeRef = groupRef->classType();
        if (groupTypeRef == GA_GROUP_EDGE)
        {
            groupIntersect(geo, group, static_cast<const GA_EdgeGroup*>(groupRef));
            return;
        }
        GA_EdgeGroupUPtr restGroupUptr = geo->createDetachedEdgeGroup();
        GA_EdgeGroup* restGroup = restGroupUptr.get();
        restGroup->combine(group);
        group->clear();

        const GA_Topology& topo = geo->getTopology();
        const GA_ATITopology* vtxPointRef = topo.getPointRef();

        switch (groupTypeRef)
        {
        case GA_GROUP_PRIMITIVE:
        {
            const GA_Range range = geo->getPrimitiveRange(static_cast<const GA_PrimitiveGroup*>(groupRef));
            GA_Offset start, end;
            for (GA_Iterator it(range); it.fullBlockAdvance(start, end); )
            {
                for (GA_Offset elemoff = start; elemoff < end; ++elemoff)
                {
                    const GA_OffsetListRef& vertices = geo->getPrimitiveVertexList(elemoff);
                    const GA_Size numvtx = vertices.size();

                    GA_Offset vtxoff_prev = vtxPointRef->getLink(vertices[0]);
                    GA_Offset vtxoff_next;
                    for (GA_Size vtxpnum = 1; vtxpnum < numvtx; ++vtxpnum)
                    {
                        vtxoff_next = vtxPointRef->getLink(vertices[vtxpnum]);
                        if (restGroup->contains(GA_Edge(vtxoff_prev, vtxoff_next)))
                            group->add(vtxoff_prev, vtxoff_next);
                        vtxoff_prev = vtxoff_next;
                    }
                    if (geo->getPrimitiveClosedFlag(elemoff))
                    {
                        vtxoff_next = vtxPointRef->getLink(vertices[0]);
                        if (restGroup->contains(GA_Edge(vtxoff_prev, vtxoff_next)))
                            group->add(vtxoff_prev, vtxoff_next);
                        group->add(vtxoff_prev, vtxoff_next);
                    }
                }
            }
        }
        break;
        case GA_GROUP_POINT:
        {
            //topo.makePrimitiveRef();
            //topo.makeVertexRef();
            const GA_ATITopology* pointVtxRef = topo.getVertexRef();
            const GA_ATITopology* vtxPrimRef = topo.getPrimitiveRef();
            const GA_ATITopology* vtxNextRef = topo.getVertexNextRef();

            const GA_Range range = geo->getPointRange(static_cast<const GA_PointGroup*>(groupRef));
            GA_Offset start, end;
            for (GA_Iterator it(range); it.fullBlockAdvance(start, end); )
            {
                for (GA_Offset elemoff = start; elemoff < end; ++elemoff)
                {
                    GA_Offset pt_next;
                    for (GA_Offset vtxoff_next = pointVtxRef->getLink(elemoff); vtxoff_next != GA_INVALID_OFFSET; vtxoff_next = vtxNextRef->getLink(vtxoff_next))
                    {
                        const GA_Offset primoff = vtxPrimRef->getLink(vtxoff_next);
                        const GA_Size numvtx = geo->getPrimitiveVertexCount(primoff);
                        const GA_Size vtxpnum = GFE_TopologyReference::vertexPrimIndex(geo, primoff, vtxoff_next);

                        if (vtxpnum == 0)
                        {
                            if (geo->getPrimitiveClosedFlag(primoff))
                            {
                                pt_next = vtxPointRef->getLink(geo->getPrimitiveVertexOffset(primoff, numvtx - 1));
                                if (restGroup->contains(GA_Edge(elemoff, pt_next)))
                                    group->add(elemoff, pt_next);
                            }
                        }
                        else
                        {
                            pt_next = vtxPointRef->getLink(geo->getPrimitiveVertexOffset(primoff, vtxpnum - 1));
                            if (restGroup->contains(GA_Edge(elemoff, pt_next)))
                                group->add(elemoff, pt_next);
                        }

                        const GA_Size vtxpnum_next = vtxpnum + 1;
                        if (vtxpnum_next == numvtx) {
                            if (geo->getPrimitiveClosedFlag(primoff))
                            {
                                pt_next = vtxPointRef->getLink(geo->getPrimitiveVertexOffset(primoff, 0));
                                if (restGroup->contains(GA_Edge(elemoff, pt_next)))
                                    group->add(elemoff, pt_next);
                            }
                        }
                        else
                        {
                            pt_next = vtxPointRef->getLink(geo->getPrimitiveVertexOffset(primoff, vtxpnum_next));
                            if (restGroup->contains(GA_Edge(elemoff, pt_next)))
                                group->add(elemoff, pt_next);
                        }
                    }
                }
            }
        }
        break;
        case GA_GROUP_VERTEX:
        {
            const GA_Range range = geo->getVertexRange(static_cast<const GA_VertexGroup*>(groupRef));
            GA_Offset start, end;
            for (GA_Iterator it(range); it.fullBlockAdvance(start, end); )
            {
                for (GA_Offset elemoff = start; elemoff < end; ++elemoff)
                {
                    const GA_Offset ptoff = vtxPointRef->getLink(elemoff);
                    const GA_Offset dstpt = GFE_TopologyReference::vertexPointDst(geo, elemoff);
                    if (restGroup->contains(GA_Edge(ptoff, dstpt)))
                        group->add(ptoff, dstpt);
                }
            }
        }
        break;
        default:
            break;
        }
    }



    static void
        groupIntersect(
            const GA_Detail* const geo,
            GA_PrimitiveGroup* const group,
            const GA_PrimitiveGroup* const groupRef
        )
    {
        UT_ASSERT_P(geo);
        UT_ASSERT_P(group);
        if (!groupRef)
            return;
        *group &= *groupRef;
        //const GA_SplittableRange geoSplittableRange(geo->getPrimitiveRange(group));
        //UTparallelFor(geoSplittableRange, [geo, group, groupRef](const GA_SplittableRange& r)
        //{
        //    GA_Offset start, end;
        //    for (GA_Iterator it(r); it.blockAdvance(start, end); )
        //    {
        //        for (GA_Offset elemoff = start; elemoff < end; ++elemoff)
        //        {
        //            if (!groupRef->contains(elemoff))
        //                group->setElement(elemoff, false);
        //        }
        //    }
        //});
    }

    static void
        groupIntersect(
            const GA_Detail* const geo,
            GA_PointGroup* const group,
            const GA_PointGroup* const groupRef
        )
    {
        UT_ASSERT_P(geo);
        UT_ASSERT_P(group);
        if (!groupRef)
            return;
        *group &= *groupRef;
        //const GA_SplittableRange geoSplittableRange(geo->getPointRange(group));
        //UTparallelFor(geoSplittableRange, [geo, group, groupRef](const GA_SplittableRange& r)
        //{
        //    GA_Offset start, end;
        //    for (GA_Iterator it(r); it.blockAdvance(start, end); )
        //    {
        //        for (GA_Offset elemoff = start; elemoff < end; ++elemoff)
        //        {
        //            if (!groupRef->contains(elemoff))
        //                group->setElement(elemoff, false);
        //        }
        //    }
        //});
    }

    static void
        groupIntersect(
            const GA_Detail* const geo,
            GA_VertexGroup* const group,
            const GA_VertexGroup* const groupRef
        )
    {
        UT_ASSERT_P(geo);
        UT_ASSERT_P(group);
        if (!groupRef)
            return;
        *group &= *groupRef;
    }







    static void
        groupIntersect(
            const GA_Detail* const geo,
            GA_PrimitiveGroup* const group,
            const GA_PointGroup* const groupRef
        )
    {
        UT_ASSERT_P(geo);
        UT_ASSERT_P(group);
        if (!groupRef)
            return;
        const GA_PrimitiveGroupUPtr promoGroupRef = GFE_GroupPromote::groupPromotePrimitiveDetached(geo, groupRef);
        groupIntersect(geo, group, promoGroupRef.get());
    }

    SYS_FORCE_INLINE
        static void
        groupIntersect(
            const GA_Detail* const geo,
            GA_PrimitiveGroup* const group,
            const GA_VertexGroup* const groupRef
        )
    {
        UT_ASSERT_P(geo);
        UT_ASSERT_P(group);
        if (!groupRef)
            return;
        const GA_PrimitiveGroupUPtr promoGroupRef = GFE_GroupPromote::groupPromotePrimitiveDetached(geo, groupRef);
        groupIntersect(geo, group, promoGroupRef.get());
    }

    SYS_FORCE_INLINE
        static void
        groupIntersect(
            const GA_Detail* const geo,
            GA_PrimitiveGroup* const group,
            const GA_EdgeGroup* const groupRef
        )
    {
        UT_ASSERT_P(geo);
        UT_ASSERT_P(group);
        if (!groupRef)
            return;
        const GA_PrimitiveGroupUPtr promoGroupRef = GFE_GroupPromote::groupPromotePrimitiveDetached(geo, groupRef);
        groupIntersect(geo, group, promoGroupRef.get());
    }


    static void
        groupIntersect(
            const GA_Detail* const geo,
            GA_PrimitiveGroup* const group,
            const GA_Group* const groupRef
        )
    {
        UT_ASSERT_P(geo);
        UT_ASSERT_P(group);
        if (!groupRef)
            return;
        switch (groupRef->classType())
        {
        case GA_GROUP_PRIMITIVE:
        {
            groupIntersect(geo, group, static_cast<const GA_PrimitiveGroup*>(groupRef));
        }
        break;
        case GA_GROUP_POINT:
        {
            groupIntersect(geo, group, static_cast<const GA_PointGroup*>(groupRef));
        }
        break;
        case GA_GROUP_VERTEX:
        {
            groupIntersect(geo, group, static_cast<const GA_VertexGroup*>(groupRef));
        }
        break;
        case GA_GROUP_EDGE:
        {
            groupIntersect(geo, group, static_cast<const GA_EdgeGroup*>(groupRef));
        }
        break;
        default:
            break;
        }
    }




    
    static void
        groupIntersect(
            const GA_Detail* const geo,
            GA_PointGroup* const group,
            const GA_PrimitiveGroup* const groupRef
        )
    {
        UT_ASSERT_P(geo);
        UT_ASSERT_P(group);
        if (!groupRef)
            return;
        const GA_PointGroupUPtr promoGroupRef = GFE_GroupPromote::groupPromotePointDetached(geo, groupRef);
        groupIntersect(geo, group, promoGroupRef.get());
    }

    static void
        groupIntersect(
            const GA_Detail* const geo,
            GA_PointGroup* const group,
            const GA_VertexGroup* const groupRef
        )
    {
        UT_ASSERT_P(geo);
        UT_ASSERT_P(group);
        if (!groupRef)
            return;
        const GA_PointGroupUPtr promoGroupRef = GFE_GroupPromote::groupPromotePointDetached(geo, groupRef);
        groupIntersect(geo, group, promoGroupRef.get());
    }

    static void
        groupIntersect(
            const GA_Detail* const geo,
            GA_PointGroup* const group,
            const GA_EdgeGroup* const groupRef
        )
    {
        UT_ASSERT_P(geo);
        UT_ASSERT_P(group);
        if (!groupRef)
            return;
        const GA_PointGroupUPtr promoGroupRef = GFE_GroupPromote::groupPromotePointDetached(geo, groupRef);
        groupIntersect(geo, group, promoGroupRef.get());
    }


    static void
        groupIntersect(
            const GA_Detail* const geo,
            GA_PointGroup* const group,
            const GA_Group* const groupRef
        )
    {
        UT_ASSERT_P(geo);
        UT_ASSERT_P(group);
        if (!groupRef)
            return;
        switch (groupRef->classType())
        {
        case GA_GROUP_PRIMITIVE:
        {
            groupIntersect(geo, group, static_cast<const GA_PrimitiveGroup*>(groupRef));
        }
        break;
        case GA_GROUP_POINT:
        {
            groupIntersect(geo, group, static_cast<const GA_PointGroup*>(groupRef));
        }
        break;
        case GA_GROUP_VERTEX:
        {
            groupIntersect(geo, group, static_cast<const GA_VertexGroup*>(groupRef));
        }
        break;
        case GA_GROUP_EDGE:
        {
            groupIntersect(geo, group, static_cast<const GA_EdgeGroup*>(groupRef));
        }
        break;
        default:
            break;
        }
    }





    static void
        groupIntersect(
            const GA_Detail* const geo,
            GA_VertexGroup* const group,
            const GA_PrimitiveGroup* const groupRef
        )
    {
        UT_ASSERT_P(geo);
        UT_ASSERT_P(group);
        if (!groupRef)
            return;
        const GA_VertexGroupUPtr promoGroupRef = GFE_GroupPromote::groupPromoteVertexDetached(geo, groupRef);
        groupIntersect(geo, group, promoGroupRef.get());
    }

    static void
        groupIntersect(
            const GA_Detail* const geo,
            GA_VertexGroup* const group,
            const GA_PointGroup* const groupRef
        )
    {
        UT_ASSERT_P(geo);
        UT_ASSERT_P(group);
        if (!groupRef)
            return;
        const GA_VertexGroupUPtr promoGroupRef = GFE_GroupPromote::groupPromoteVertexDetached(geo, groupRef);
        groupIntersect(geo, group, promoGroupRef.get());
    }

    static void
        groupIntersect(
            const GA_Detail* const geo,
            GA_VertexGroup* const group,
            const GA_EdgeGroup* const groupRef
        )
    {
        UT_ASSERT_P(geo);
        UT_ASSERT_P(group);
        if (!groupRef)
            return;
        const GA_VertexGroupUPtr promoGroupRef = GFE_GroupPromote::groupPromoteVertexDetached(geo, groupRef);
        groupIntersect(geo, group, promoGroupRef.get());
    }


    static void
        groupIntersect(
            const GA_Detail* const geo,
            GA_VertexGroup* const group,
            const GA_Group* const groupRef
        )
    {
        UT_ASSERT_P(geo);
        UT_ASSERT_P(group);
        if (!groupRef)
            return;
        switch (groupRef->classType())
        {
        case GA_GROUP_PRIMITIVE:
        {
            groupIntersect(geo, group, static_cast<const GA_PrimitiveGroup*>(groupRef));
        }
        break;
        case GA_GROUP_POINT:
        {
            groupIntersect(geo, group, static_cast<const GA_PointGroup*>(groupRef));
        }
        break;
        case GA_GROUP_VERTEX:
        {
            groupIntersect(geo, group, static_cast<const GA_VertexGroup*>(groupRef));
        }
        break;
        case GA_GROUP_EDGE:
        {
            groupIntersect(geo, group, static_cast<const GA_EdgeGroup*>(groupRef));
        }
        break;
        default:
            break;
        }
    }









    SYS_FORCE_INLINE
        static void
        groupIntersect(
            const GA_Detail* const geo,
            GA_Group* const group,
            const GA_Group* const groupRef
        )
    {
        UT_ASSERT_P(geo);
        UT_ASSERT_P(group);
        if (!groupRef)
            return;
        switch (group->classType())
        {
        case GA_GROUP_PRIMITIVE:
        {
            groupIntersect(geo, UTverify_cast<GA_PrimitiveGroup*>(group), groupRef);
        }
        break;
        case GA_GROUP_POINT:
        {
            groupIntersect(geo, UTverify_cast<GA_PointGroup*>(group), groupRef);
        }
        break;
        case GA_GROUP_VERTEX:
        {
            groupIntersect(geo, UTverify_cast<GA_VertexGroup*>(group), groupRef);
        }
        break;
        case GA_GROUP_EDGE:
        {
            groupIntersect(geo, UTverify_cast<GA_EdgeGroup*>(group), groupRef);
        }
        break;
        default:
            break;
        }
    }
    






    //static void
    //    groupIntersect(
    //        const GA_Detail* const geo,
    //        GA_Group* const group,
    //        const GA_Group* const groupRef
    //    )
    //{
    //    UT_ASSERT_P(geo);
    //    UT_ASSERT_P(group);
    //    if (group->classType() == GA_GROUP_EDGE)
    //    {
    //        edgeGroupIntersect(geo, group, groupRef);
    //    }
    //    else
    //    {
    //        //group->combine(groupRef);
    //        static_cast<GA_ElementGroup*>(group)->intersect(groupRef);
    //    }
    //}


    //
    //static void
    //    pointGroupBoolean(
    //        const GA_Detail* const geo,
    //        GA_Group* const group,
    //        const GA_PointGroup* const groupRef,
    //        const GA_GroupBooleanOp groupBooleanOp
    //    )
    //{

    //    UT_ASSERT_P(geo);
    //    UT_ASSERT_P(group);
    //    if (!groupRef)
    //        return;
    //    if (group->classType() == GA_GROUP_EDGE)
    //    {
    //        GFE_GroupUnion::edgeGroupUnion(geo, group, groupRef);
    //    }
    //    else
    //    {
    //        switch (groupBooleanOp)
    //        {
    //        case GA_GROUP_BOOLEAN_REP:
    //            *static_cast<GA_ElementGroup*>(group) = *static_cast<const GA_ElementGroup*>(groupRef);
    //            break;
    //        case GA_GROUP_BOOLEAN_OR:
    //            *static_cast<GA_ElementGroup*>(group) |= *static_cast<const GA_ElementGroup*>(groupRef);
    //            //*static_cast<GA_PointGroup*>(group) &= *static_cast<const GA_PointGroup*>(groupRef);
    //            //static_cast<GA_PointGroup*>(group)->operator&=(*static_cast<const GA_PointGroup*>(groupRef));
    //            //*static_cast<GA_ElementGroup*>(group) += *static_cast<GA_ElementGroup*>(groupRef);
    //            //static_cast<GA_ElementGroup*>(group)->combine(groupRef);
    //            break;
    //        case GA_GROUP_BOOLEAN_AND:
    //            *static_cast<GA_ElementGroup*>(group) &= *static_cast<const GA_ElementGroup*>(groupRef);
    //            break;
    //        case GA_GROUP_BOOLEAN_XOR:
    //            *static_cast<GA_ElementGroup*>(group) ^= *static_cast<const GA_ElementGroup*>(groupRef);
    //            break;
    //        case GA_GROUP_BOOLEAN_SUB:
    //            *static_cast<GA_ElementGroup*>(group) -= *static_cast<const GA_ElementGroup*>(groupRef);
    //            break;
    //        }
    //    }
    //}
    //


    //static void
    //    groupBoolean(
    //        const GA_Detail* const geo,
    //        GA_Group* const group,
    //        const GA_Group* const groupRef,
    //        const GA_GroupBooleanOp groupBooleanOp
    //    )
    //{
    //    UT_ASSERT_P(geo);
    //    UT_ASSERT_P(group);
    //    if (!groupRef)
    //        return;
    //    if (group->classType() == GA_GROUP_EDGE)
    //    {
    //        GFE_GroupUnion::edgeGroupUnion(geo, group, groupRef);
    //    }
    //    else
    //    {
    //        GA_ElementGroup* elementGroup = static_cast<GA_ElementGroup*>(group);
    //        const GA_ElementGroup* elementRefGroup = static_cast<const GA_ElementGroup*>(groupRef);
    //        switch (groupBooleanOp)
    //        {
    //        case GA_GROUP_BOOLEAN_COPY:
    //            elementGroup->copyMembership(*elementRefGroup);
    //            break;
    //        case GA_GROUP_BOOLEAN_OR:
    //            elementGroup->operator|=(groupRef);
    //            //elementGroup->orEqual(elementRefGroup);
    //            break;
    //        case GA_GROUP_BOOLEAN_AND:
    //            elementGroup->andEqual(elementRefGroup);
    //            break;
    //        case GA_GROUP_BOOLEAN_XOR:
    //            elementGroup->xorEqual(elementRefGroup);
    //            break;
    //        case GA_GROUP_BOOLEAN_SUB:
    //            elementGroup->subEqual(elementRefGroup);
    //            break;
    //        }
    //    }
    //}


static void
unorderedGroupBoolean(
    const GA_Detail* const geo,
    GA_Group* const group,
    const GA_Group* const groupRef,
    const GU_GroupBoolOp groupBooleanOp
)
{
    UT_ASSERT_P(geo);
    UT_ASSERT_P(group);
    UT_ASSERT_P(!group->isOrdered());
    UT_ASSERT_P(!groupRef->isOrdered());
    if (!groupRef)
        return;
    if (group->classType() == GA_GROUP_EDGE)
    {
        GFE_GroupUnion::edgeGroupUnion(geo, group, groupRef);
    }
    else
    {
        GA_ElementGroup* elementGroup = static_cast<GA_ElementGroup*>(group);
        const GA_ElementGroup* elementRefGroup = static_cast<const GA_ElementGroup*>(groupRef);
        switch (groupBooleanOp)
        {
        case GU_GROUP_BOOLOP_OR:
            elementGroup->combine(groupRef);
            //elementGroup->orEqual(elementRefGroup);
            break;
        case GU_GROUP_BOOLOP_AND:
            elementGroup->andEqual(elementRefGroup);
            break;
        case GU_GROUP_BOOLOP_XOR:
            elementGroup->xorEqual(elementRefGroup);
            break;
        case GU_GROUP_BOOLOP_MINUS:
            elementGroup->subEqual(elementRefGroup);
            break;
        }
    }
}

    //static void
    //    unorderedGroupBoolean(
    //        const GA_Detail* const geo,
    //        GA_Group* const group,
    //        const GA_Group* const groupRef,
    //        const GU_GroupBoolOp groupBooleanOp
    //    )
    //{
    //    UT_ASSERT_P(geo);
    //    UT_ASSERT_P(group);
    //    UT_ASSERT_P(!group->isOrdered());
    //    UT_ASSERT_P(!groupRef->isOrdered());
        //if (!groupRef)
        //    return;
    //    if (group->classType() == GA_GROUP_EDGE)
    //    {
    //        GFE_GroupUnion::edgeGroupUnion(geo, group, groupRef);
    //    }
    //    else
    //    {
    //        GA_ElementGroup* elementGroup = static_cast<GA_ElementGroup*>(group);
    //        const GA_ElementGroup* elementRefGroup = static_cast<const GA_ElementGroup*>(groupRef);
    //        switch (groupBooleanOp)
    //        {
    //        case GA_GROUP_BOOLEAN_COPY:
    //            elementGroup->copyMembership(*elementRefGroup);
    //            break;
    //        case GA_GROUP_BOOLEAN_OR:
    //            elementGroup->combine(groupRef);
    //            //elementGroup->orEqual(elementRefGroup);
    //            break;
    //        case GA_GROUP_BOOLEAN_AND:
    //            elementGroup->andEqual(elementRefGroup);
    //            break;
    //        case GA_GROUP_BOOLEAN_XOR:
    //            elementGroup->xorEqual(elementRefGroup);
    //            break;
    //        case GA_GROUP_BOOLEAN_SUB:
    //            elementGroup->subEqual(elementRefGroup);
    //            break;
    //        }
    //    }
    //}




} // End of namespace GFE_GroupBoolean
*/

#endif
