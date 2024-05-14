
#pragma once

#ifndef __GFE_GroupUnion_h__
#define __GFE_GroupUnion_h__

#include <GFE/GroupUnion.h>


#include <GA/GA_Detail.h>
#include <GA/GA_SplittableRange.h>
#include <GA/GA_PageHandle.h>
#include <GA/GA_PageIterator.h> // SOP_FeE_GroupPolyByWinding_1_0 can not compile with out this .h

#include <GFE/Type.h>
#include <GFE/GeometryBase.h>

_GFE_BEGIN
namespace GroupUnion {

    
    namespace detail {

        SYS_FORCE_INLINE static void groupUnionPrimitiveVertexEdge(GA_VertexGroup& group, const GA_EdgeGroup& groupRef, const bool reverse = false)
        { group.combine(&groupRef); }

        static void groupUnionPointVertexEdge(GA_VertexGroup& group, const GA_EdgeGroup& groupRef, const bool reverse = false)
        {
            const GA_PointGroupUPtr pointGroupUPtr = group.getDetail().createDetachedPointGroup();
            pointGroupUPtr->combine(&groupRef);
            group.combine(pointGroupUPtr.get());
        }

        /////////////////////////////////////// Very Slow //////////////////////////////////////////
        /////////////////////////////////////// Very Slow //////////////////////////////////////////
        /////////////////////////////////////// Very Slow //////////////////////////////////////////
        /////////////////////////////////////// Very Slow //////////////////////////////////////////
        /////////////////////////////////////// Very Slow //////////////////////////////////////////
        /////////////////////////////////////// Very Slow //////////////////////////////////////////
        static void groupUnionOneVertexEdge(GA_VertexGroup& group, const GA_EdgeGroup& groupRef, const bool reverse = false)
        {
            const GA_Detail& geo = group.getDetail();
            UT_ASSERT_P(&geo == &groupRef.getDetail());
            
            GA_EdgeGroupUPtr edgeGroupUPtr;
            const GA_EdgeGroup* edgeGroupRef;
            if (reverse)
            {
                edgeGroupUPtr = geo.createDetachedEdgeGroup();
                GA_EdgeGroup* const edgeGroupTmp = edgeGroupUPtr.get();
                edgeGroupTmp->combine(&groupRef);
                edgeGroupTmp->toggleEntries();
                edgeGroupRef = edgeGroupTmp;
            }
            else
            {
                edgeGroupRef = &groupRef;
            }
            for (GA_EdgeGroup::const_iterator it = edgeGroupRef->begin(); !it.atEnd(); ++it)
            {
                group.setElement(gfe::GeoBase::edgeVertex(geo, *it), true);
            }
        }
        
        static void groupUnionAllVertexEdge(GA_VertexGroup& group, const GA_EdgeGroup& groupRef, const bool reverse = false)
        {
            const GA_Detail& geo = group.getDetail();
            UT_ASSERT_P(&geo == &groupRef.getDetail());
            
            GA_EdgeGroupUPtr edgeGroupUPtr;
            const GA_EdgeGroup* edgeGroupRef;
            if (reverse)
            {
                edgeGroupUPtr = geo.createDetachedEdgeGroup();
                GA_EdgeGroup* const edgeGroupTmp = edgeGroupUPtr.get();
                edgeGroupTmp->combine(&groupRef);
                edgeGroupTmp->toggleEntries();
                edgeGroupRef = edgeGroupTmp;
            }
            else
            {
                edgeGroupRef = &groupRef;
            }
            for (GA_EdgeGroup::const_iterator it = edgeGroupRef->begin(); !it.atEnd(); ++it)
            {
                GA_Offset ptoff1 = it->p1();
                GA_Size vtxpnum_next;
                GA_Offset primPoint_next;
                for (GA_Offset vtxoff = geo.pointVertex(it->p0()); gfe::isValidOffset(vtxoff); vtxoff = geo.vertexToNextVertex(vtxoff))
                {
                    const GA_Offset primoff = geo.vertexPrimitive(vtxoff);
                    const GA_OffsetListRef& vertices = geo.getPrimitiveVertexList(primoff);
                    const GA_Size numvtx = vertices.size();
                    for (GA_Size vtxpnum = 0; vtxpnum < numvtx; ++vtxpnum)
                    {
                        if (vertices[vtxpnum] != vtxoff)
                            continue;
                    
                        vtxpnum_next = vtxpnum+1;
                        if (vtxpnum_next != numvtx || vertices.isClosed())
                        {
                            if (vtxpnum_next == numvtx)
                                vtxpnum_next = 0;
                            primPoint_next = geo.vertexPoint(vertices[vtxpnum_next]);
                            if (primPoint_next == ptoff1)
                                group.setElement(vtxoff, true);
                        }
                    
                        if (vtxpnum != 0 || vertices.isClosed())
                        {
                            vtxpnum_next = vtxpnum==0 ? numvtx-1 : vtxpnum-1;
                            primPoint_next = geo.vertexPoint(vertices[vtxpnum_next]);
                            if (primPoint_next == ptoff1)
                                group.setElement(vertices[vtxpnum_next], true);
                        }
                    }
                }
            }
        }
    } // End of Namespace detail
    


    
    /////////////////////////// Utility //////////////////////////

    SYS_FORCE_INLINE static void groupUnion(
        GA_Group& group, const GA_Group& groupRef,
        const bool reverse = false,
        const exint subscribeRatio = 64, const exint minGrainSize = 1024)
    {
        switch (group.classType())
        {
        case GA_GROUP_PRIMITIVE: groupUnion(static_cast<GA_PrimitiveGroup&>(group), groupRef, reverse, subscribeRatio, minGrainSize); break;
        case GA_GROUP_POINT:     groupUnion(static_cast<GA_PointGroup&    >(group), groupRef, reverse, subscribeRatio, minGrainSize); break;
        case GA_GROUP_VERTEX:    groupUnion(static_cast<GA_VertexGroup&   >(group), groupRef, reverse, subscribeRatio, minGrainSize); break;
        case GA_GROUP_EDGE:      groupUnion(static_cast<GA_EdgeGroup&     >(group), groupRef, reverse);                               break;
        }
    }

    template<typename GROUP_T>
    SYS_FORCE_INLINE static void groupUnion(
        GROUP_T& group, const GA_Group& groupRef,
        const bool reverse = false,
        const exint subscribeRatio = 64, const exint minGrainSize = 1024)
    {
        switch (groupRef.classType())
        {
        case GA_GROUP_PRIMITIVE: groupUnion(group, static_cast<const GA_PrimitiveGroup&>(groupRef), reverse, subscribeRatio, minGrainSize); break;
        case GA_GROUP_POINT:     groupUnion(group, static_cast<const GA_PointGroup&    >(groupRef), reverse, subscribeRatio, minGrainSize); break;
        case GA_GROUP_VERTEX:    groupUnion(group, static_cast<const GA_VertexGroup&   >(groupRef), reverse, subscribeRatio, minGrainSize); break;
        case GA_GROUP_EDGE:      groupUnion(group, static_cast<const GA_EdgeGroup&     >(groupRef), reverse);                               break;
        }
    }
    
    template<>
    SYS_FORCE_INLINE static void groupUnion<GA_EdgeGroup>(
        GA_EdgeGroup& group, const GA_Group& groupRef,
        const bool reverse,
        const exint subscribeRatio, const exint minGrainSize)
    {
        switch (groupRef.classType())
        {
        case GA_GROUP_PRIMITIVE: groupUnion(group, static_cast<const GA_PrimitiveGroup&>(groupRef), reverse); break;
        case GA_GROUP_POINT:     groupUnion(group, static_cast<const GA_PointGroup&    >(groupRef), reverse); break;
        case GA_GROUP_VERTEX:    groupUnion(group, static_cast<const GA_VertexGroup&   >(groupRef), reverse); break;
        case GA_GROUP_EDGE:      groupUnion(group, static_cast<const GA_EdgeGroup&     >(groupRef), reverse); break;
        }
    }
    
    SYS_FORCE_INLINE static void groupUnion(GA_EdgeGroup& group, const GA_Group& groupRef, const bool reverse = false)
    {
        switch (groupRef.classType())
        {
        case GA_GROUP_PRIMITIVE: groupUnion(group, static_cast<const GA_PrimitiveGroup&>(groupRef), reverse); break;
        case GA_GROUP_POINT:     groupUnion(group, static_cast<const GA_PointGroup&    >(groupRef), reverse); break;
        case GA_GROUP_VERTEX:    groupUnion(group, static_cast<const GA_VertexGroup&   >(groupRef), reverse); break;
        case GA_GROUP_EDGE:      groupUnion(group, static_cast<const GA_EdgeGroup&     >(groupRef), reverse); break;
        }
    }

#define __GroupUnion_FUNC_SPECILIZATION(GroupType)                                                                                \
        SYS_FORCE_INLINE static void groupUnion(GA_EdgeGroup& group, const GroupType* const groupRef, const bool reverse = false) \
        {                                                                                                                         \
            if (groupRef)                                                                                                         \
                groupUnion(group, *groupRef, reverse);                                                                            \
            else if (!reverse)                                                                                                    \
                group.addAll();                                                                                                   \
        }                                                                                                                         \

    __GroupUnion_FUNC_SPECILIZATION(GA_PrimitiveGroup);
    __GroupUnion_FUNC_SPECILIZATION(GA_PointGroup);
    __GroupUnion_FUNC_SPECILIZATION(GA_VertexGroup);
    
#undef __GroupUnion_FUNC_SPECILIZATION


    
    /////////////////////////// Utility For Group Pointer //////////////////////////
    
    template<typename GROUP_T>
    SYS_FORCE_INLINE static void groupUnion(GROUP_T& group, const GA_EdgeGroup* const groupRef, const bool reverse = false)
    {
        if (groupRef)
            groupUnion(group, *groupRef, reverse);
        else if (!reverse)
            group->addAll();
    }
    
    template<typename GROUP_T, typename GROUP_REF_T>
    SYS_FORCE_INLINE static void groupUnion(
        GROUP_T& group, const GROUP_REF_T* const groupRef,
        const bool reverse = false,
        const exint subscribeRatio = 64, const exint minGrainSize = 1024)
    {
        if (groupRef)
            groupUnion(group, *groupRef, reverse, subscribeRatio, minGrainSize);
        else if (!reverse)
            group.addAll();
    }
    
    template<typename GROUP_T, typename GROUP_REF_T>
    SYS_FORCE_INLINE static void groupUnion(
        GROUP_T* const group, const GROUP_REF_T* const groupRef,
        const bool reverse = false,
        const exint subscribeRatio = 64, const exint minGrainSize = 1024)
    { UT_ASSERT_P(group); groupUnion(*group, groupRef, reverse, subscribeRatio, minGrainSize); }
    


    
    



    
    /////////////////////////// Same Type //////////////////////////

    
#define __GroupUnion_FUNC_SPECILIZATION(GroupType)                           \
SYS_FORCE_INLINE static void groupUnion(                                     \
    GroupType& group, const GroupType& groupRef,                             \
    const bool reverse = false,                                              \
    const exint subscribeRatio = 64, const exint minGrainSize = 1024)        \
{                                                                            \
    group.combine(&groupRef);                                                \
    if (reverse)                                                             \
        group.toggleEntries();                                               \
}                                                                            \

    __GroupUnion_FUNC_SPECILIZATION(GA_PrimitiveGroup);
    __GroupUnion_FUNC_SPECILIZATION(GA_PointGroup);
    __GroupUnion_FUNC_SPECILIZATION(GA_VertexGroup);
    
#undef __GroupUnion_FUNC_SPECILIZATION
    
    static void groupUnion(GA_EdgeGroup& group, const GA_EdgeGroup& groupRef, const bool reverse = false)
    {
        group.combine(&groupRef);
        if (reverse)
            group.toggle();
    }



    // SYS_FORCE_INLINE static void groupUnion(GA_Group& group, const GA_VertexGroup* const groupRef)
    // {
    //     if (group.classType() == GA_GROUP_EDGE)
    //         groupUnion(static_cast<GA_EdgeGroup&>(group), groupRef);
    //     else
    //         static_cast<GA_ElementGroup&>(group).combine(groupRef);
    // }
    // 
    // SYS_FORCE_INLINE static void groupUnion(GA_Group& group, const GA_EdgeGroup* const groupRef)
    // {
    //     if (group.classType() == GA_GROUP_EDGE)
    //         groupUnion(static_cast<GA_EdgeGroup&>(group), groupRef);
    //     else
    //         static_cast<GA_ElementGroup&>(group).combine(groupRef);
    // }






    /////////////////////////// GA_PrimitiveGroup //////////////////////////


    
    static void groupUnion(
        GA_PrimitiveGroup& group, const GA_PointGroup& groupRef,
        const bool reverse = false,
        const exint subscribeRatio = 64, const exint minGrainSize = 1024)
    {
        // if (!reverse)
        // {
        //     group.combine(&groupRef);
        //     return;
        // }
        const GA_Detail& geo = group.getDetail();
        UTparallelFor(GA_SplittableRange(GA_Range(groupRef, reverse)), [&geo, &group](const GA_SplittableRange& r)
        {
            GA_Offset start, end;
            for (GA_Iterator it(r); it.blockAdvance(start, end); )
            {
                for (GA_Offset elemoff = start; elemoff < end; ++elemoff)
                {
                    for (GA_Offset vtxoff_next = geo.pointVertex(elemoff); gfe::isValidOffset(vtxoff_next); vtxoff_next = geo.vertexToNextVertex(vtxoff_next))
                    {
                        group.setElement(geo.vertexPrimitive(vtxoff_next), true);
                    }
                }
            }
        }, subscribeRatio, minGrainSize);
    }

    static void groupUnion(
        GA_PrimitiveGroup& group, const GA_VertexGroup& groupRef,
        const bool reverse = false,
        const exint subscribeRatio = 64, const exint minGrainSize = 1024)
    {
        const GA_Detail& geo = group.getDetail();
        UTparallelFor(GA_SplittableRange(GA_Range(groupRef, reverse)), [&geo, &group](const GA_SplittableRange& r)
        {
            GA_Offset start, end;
            for (GA_Iterator it(r); it.blockAdvance(start, end); )
            {
                for (GA_Offset elemoff = start; elemoff < end; ++elemoff)
                {
                    group.setElement(geo.vertexPrimitive(elemoff), true);
                }
            }
        }, subscribeRatio, minGrainSize);
    }
    
    SYS_FORCE_INLINE static void groupUnion(GA_PrimitiveGroup& group, const GA_EdgeGroup& groupRef, const bool reverse = false)
    {
        group.combine(&groupRef);
    }


    /////////////////////////// GA_PointGroup //////////////////////////


    static void groupUnion(
        GA_PointGroup& group, const GA_PrimitiveGroup& groupRef,
        const bool reverse = false,
        const exint subscribeRatio = 64, const exint minGrainSize = 1024)
    {
        const GA_Detail& geo = group.getDetail();
        
        UTparallelFor(GA_SplittableRange(GA_Range(groupRef, reverse)), [&geo, &group](const GA_SplittableRange& r)
        {
            GA_Offset start, end;
            for (GA_Iterator it(r); it.blockAdvance(start, end); )
            {
                for (GA_Offset elemoff = start; elemoff < end; ++elemoff)
                {
                    const GA_OffsetListRef& vertices = geo.getPrimitiveVertexList(elemoff);
                    const GA_Size numvtx = vertices.size();
                    for (GA_Size vtxpnum = 0; vtxpnum < numvtx; ++vtxpnum)
                    {
                        group.setElement(geo.vertexPoint(vertices[vtxpnum]), true);
                    }
                }
            }
        }, subscribeRatio, minGrainSize);
    }

    static void groupUnion(
        GA_PointGroup& group, const GA_VertexGroup& groupRef,
        const bool reverse = false,
        const exint subscribeRatio = 64, const exint minGrainSize = 1024)
    {
        const GA_Detail& geo = group.getDetail();
        UTparallelFor(GA_SplittableRange(GA_Range(groupRef, reverse)), [&geo, &group](const GA_SplittableRange& r)
        {
            GA_Offset start, end;
            for (GA_Iterator it(r); it.blockAdvance(start, end); )
            {
                for (GA_Offset elemoff = start; elemoff < end; ++elemoff)
                {
                    group.setElement(geo.vertexPoint(elemoff), true);
                }
            }
        }, subscribeRatio, minGrainSize);
    }
    
    SYS_FORCE_INLINE static void groupUnion(GA_PointGroup& group, const GA_EdgeGroup& groupRef, const bool reverse = false)
    {
        group.combine(&groupRef);
    }



    /////////////////////////// GA_VertexGroup //////////////////////////
    
    static void groupUnion(
        GA_VertexGroup& group, const GA_PrimitiveGroup& groupRef,
        const bool reverse = false,
        const exint subscribeRatio = 64, const exint minGrainSize = 1024)
    {
        if (!reverse)
        {
            group.combine(&groupRef);
            return;
        }
        const GA_Detail& geo = group.getDetail();
        UTparallelFor(GA_SplittableRange(GA_Range(groupRef, reverse)), [&geo, &group](const GA_SplittableRange& r)
        {
            GA_Offset start, end;
            for (GA_Iterator it(r); it.blockAdvance(start, end); )
            {
                for (GA_Offset elemoff = start; elemoff < end; ++elemoff)
                {
                    const GA_OffsetListRef& vertices = geo.getPrimitiveVertexList(elemoff);
                    const GA_Size numvtx = vertices.size();
                    for (GA_Size vtxpnum = 0; vtxpnum < numvtx; ++vtxpnum)
                    {
                        group.setElement(vertices[vtxpnum], true);
                    }
                }
            }
        }, subscribeRatio, minGrainSize);
    }

    static void groupUnion(
        GA_VertexGroup& group, const GA_PointGroup& groupRef,
        const bool reverse = false,
        const exint subscribeRatio = 64, const exint minGrainSize = 1024)
    {
        const GA_Detail& geo = group.getDetail();
        UTparallelFor(GA_SplittableRange(GA_Range(groupRef, reverse)), [&geo, &group](const GA_SplittableRange& r)
        {
            GA_Offset start, end;
            for (GA_Iterator it(r); it.blockAdvance(start, end); )
            {
                for (GA_Offset elemoff = start; elemoff < end; ++elemoff)
                {
                    for (GA_Offset vtxoff_next = geo.pointVertex(elemoff); gfe::isValidOffset(vtxoff_next); vtxoff_next = geo.vertexToNextVertex(vtxoff_next))
                    {
                        group.setElement(vtxoff_next, true);
                    }
                }
            }
        }, subscribeRatio, minGrainSize);
    }
    
    SYS_FORCE_INLINE static void groupUnion(GA_VertexGroup& group, const GA_EdgeGroup& groupRef, const bool reverse = false)
    { group.combine(&groupRef); }




    
    /////////////////////////////////////// Very Slow //////////////////////////////////////////
    /////////////////////////////////////// Very Slow //////////////////////////////////////////
    /////////////////////////////////////// Very Slow //////////////////////////////////////////
    /////////////////////////////////////// Very Slow //////////////////////////////////////////
    /////////////////////////////////////// Very Slow //////////////////////////////////////////
    /////////////////////////////////////// Very Slow //////////////////////////////////////////


    template<GA_AttributeOwner _ConnectElemType>
    static void groupUnion(GA_VertexGroup& group, const GA_EdgeGroup& groupRef, const bool reverse = false)
    {
        if constexpr (_ConnectElemType == GA_ATTRIB_PRIMITIVE)
            detail::groupUnionPrimitiveVertexEdge(group, groupRef, reverse);
        else if constexpr (_ConnectElemType == GA_ATTRIB_POINT)
            detail::groupUnionPointVertexEdge    (group, groupRef, reverse);
        else if constexpr (_ConnectElemType == GA_ATTRIB_VERTEX)
            detail::groupUnionOneVertexEdge      (group, groupRef, reverse);
        else if constexpr (_ConnectElemType == GA_ATTRIB_OWNER_N)
            detail::groupUnionAllVertexEdge      (group, groupRef, reverse);
        else
            UT_ASSERT_MSG(0, "Unhandled Group Union Owner");
    }

    static void groupUnion(GA_VertexGroup& group, const GA_EdgeGroup& groupRef, const GA_AttributeOwner connectElemType, const bool reverse = false)
    {
        switch (connectElemType)
        {
        case GA_ATTRIB_PRIMITIVE: detail::groupUnionPrimitiveVertexEdge(group, groupRef, reverse); break;
        case GA_ATTRIB_POINT:     detail::groupUnionPointVertexEdge    (group, groupRef, reverse); break;
        case GA_ATTRIB_VERTEX:    detail::groupUnionOneVertexEdge      (group, groupRef, reverse); break;
        case GA_ATTRIB_OWNER_N:   detail::groupUnionAllVertexEdge      (group, groupRef, reverse); break;
        default: UT_ASSERT_MSG(0, "Unhandled Group Union Owner"); break;
        }
    }

    template<GA_AttributeOwner _ConnectElemType>
    static void groupUnion(GA_VertexGroup& group, const GA_EdgeGroup* const groupRef, const bool reverse = false)
    {
        if (!groupRef)
        {
            if (!reverse)
                group.addAll();
            return;
        }
        groupUnion<_ConnectElemType>(group, *groupRef, reverse);
    }

    static void groupUnion(GA_VertexGroup& group, const GA_EdgeGroup* const groupRef, const GA_AttributeOwner connectElemType, const bool reverse = false)
    {
        if (!groupRef)
        {
            if (!reverse)
                group.addAll();
            return;
        }
        groupUnion(group, *groupRef, connectElemType, reverse);
    }





    
    /////////////////////////// GA_EdgeGroup //////////////////////////

    
    static void groupUnion(GA_EdgeGroup& group, const GA_PrimitiveGroup& groupRef, const bool reverse = false)
    {
        const GA_Detail& geo = groupRef.getDetail();
        const GA_ATITopology& pointRef = *geo.getTopology().getPointRef();

        GA_Offset start, end;
        for (GA_Iterator it(GA_Range(groupRef, reverse)); it.fullBlockAdvance(start, end); )
        {
            for (GA_Offset elemoff = start; elemoff < end; ++elemoff)
            {
                const GA_OffsetListRef& vertices = geo.getPrimitiveVertexList(elemoff);
                const GA_Size numvtx = vertices.size();

                const GA_Offset vtxoff0 = vertices[0];
                GA_Offset vtxoff_prev = pointRef.getLink(vtxoff0);
                GA_Offset vtxoff_next;
                for (GA_Size vtxpnum = 1; vtxpnum < numvtx; ++vtxpnum)
                {
                    vtxoff_next = pointRef.getLink(vertices[vtxpnum]);
                    group.add(vtxoff_prev, vtxoff_next);
                    vtxoff_prev = vtxoff_next;
                }
                if (vertices.isClosed())
                {
                    vtxoff_next = pointRef.getLink(vtxoff0);
                    group.add(vtxoff_prev, vtxoff_next);
                }
            }
        }
    }

    static void groupUnion(GA_EdgeGroup& group, const GA_PointGroup& groupRef, const bool reverse = false)
    {
        const GA_Detail& geo = groupRef.getDetail();
        
        const GA_Topology& topo = geo.getTopology();
        const GA_ATITopology& pointRef = *topo.getPointRef();
        
        GA_Offset start, end;
        for (GA_Iterator it(GA_Range(groupRef, reverse)); it.fullBlockAdvance(start, end); )
        {
            for (GA_Offset elemoff = start; elemoff < end; ++elemoff)
            {
                GA_Offset pt_next;
                for (GA_Offset vtxoff_next = geo.pointVertex(elemoff); gfe::isValidOffset(vtxoff_next); vtxoff_next = geo.vertexToNextVertex(vtxoff_next))
                {
                    const GA_Offset primoff = geo.vertexPrimitive(vtxoff_next);
                    const GA_OffsetListRef& vertices = geo.getPrimitiveVertexList(primoff);
                    const GA_Size numvtx = vertices.size();
                    const GA_Size vtxpnum = gfe::GeoBase::vertexPrimIndex(vertices, vtxoff_next);

                    if (vtxpnum == 0)
                    {
                        if (vertices.isClosed())
                        {
                            pt_next = pointRef.getLink(vertices[numvtx-1]);
                            group.add(elemoff, pt_next);
                        }
                    }
                    else
                    {
                        pt_next = pointRef.getLink(vertices[vtxpnum-1]);
                        group.add(elemoff, pt_next);
                    }

                    const GA_Size vtxpnum_next = vtxpnum+1;
                    if (vtxpnum_next == numvtx)
                    {
                        if (vertices.isClosed())
                        {
                            pt_next = pointRef.getLink(vertices[0]);
                            group.add(elemoff, pt_next);
                        }
                    }
                    else
                    {
                        pt_next = pointRef.getLink(vertices[vtxpnum_next]);
                        group.add(elemoff, pt_next);
                    }
                }
            }
        }
    }
    
    static void groupUnion(GA_EdgeGroup& group, const GA_VertexGroup& groupRef, const bool reverse = false)
    {
        const GA_Detail& geo = group.getDetail();
        const GA_ATITopology& pointRef = *geo.getTopology().getPointRef();
        if constexpr (gfe::Core::DEBUG_MODE)
        {
            GA_Size a = groupRef.computeGroupEntries();
            a = a;
        }
        GA_Offset start, end;
        for (GA_Iterator it(GA_Range(groupRef, reverse)); it.fullBlockAdvance(start, end); )
        {
            for (GA_Offset elemoff = start; elemoff < end; ++elemoff)
            {
                const GA_Offset dstpt = gfe::GeoBase::vertexPointDst(geo, elemoff);
                group.add(pointRef.getLink(elemoff), dstpt);
            }
        }
    }

    SYS_FORCE_INLINE static void
        groupUnion(
            GA_EdgeGroup& group,
            const GA_VertexGroup* const groupRef,
            const GA_Attribute& dstptAttrib,
            const bool reverse = false
        )
    {
        if (groupRef)
            groupUnion(group, groupRef, dstptAttrib, reverse);
        else if (!reverse)
            group.addAll();
    }
    
    static void
        groupUnion(
            GA_EdgeGroup& group,
            const GA_VertexGroup& groupRef,
            const GA_Attribute& dstptAttrib,
            const bool reverse = false
        )
    {
        // dstptAttrib is in geo but not in geoRef
        
        const GA_ATITopology& pointRef = *group.getDetail().getTopology().getPointRef();

        GA_PageHandleT<GA_Offset, GA_Offset, true, false, const GA_Attribute, const GA_ATINumeric, const GA_Detail> refAttrib_ph(&dstptAttrib);
        
        const GA_SplittableRange geoSplittableRange(GA_Range(groupRef.getIndexMap(), &groupRef, reverse));
        for (GA_PageIterator pit = geoSplittableRange.beginPages(); !pit.atEnd(); ++pit)
        {
            GA_Offset start, end;
            for (GA_Iterator it(pit.begin()); it.blockAdvance(start, end); )
            {
                refAttrib_ph.setPage(start);
                for (GA_Offset elemoff = start; elemoff < end; ++elemoff)
                {
                    group.add(pointRef.getLink(elemoff), refAttrib_ph.value(elemoff));
                }
            }
        }
        // const GA_ROHandleT<GA_Offset> refAttrib_h(&dstptAttrib);
        // GA_Offset start, end;
        // for (GA_Iterator it(); it.fullBlockAdvance(start, end); )
        // {
        //     for (GA_Offset elemoff = start; elemoff < end; ++elemoff)
        //     {
        //         group.add(pointRef.getLink(elemoff), refAttrib_h.get(elemoff));
        //     }
        // }
    }
    
    static void
        groupUnion_topoAttrib(
            GA_Detail& geo,
            GA_Group& group,
            const GA_VertexGroup* const groupRef,
            const bool reverse = false
        )
    {
        if (!groupRef)
        {
            if (!reverse)
                group.addAll();
            return;
        }
        if (group.classType() == GA_GROUP_EDGE)
        {
            if (groupRef->classType() == GA_GROUP_VERTEX)
                groupUnion_topoAttrib(geo, static_cast<GA_EdgeGroup&>(group), static_cast<const GA_VertexGroup*>(groupRef));
            else
                groupUnion(static_cast<GA_EdgeGroup&>(group), groupRef);
        }
        else
        {
            //group->combine(groupRef);
            static_cast<GA_ElementGroup&>(group).combine(groupRef);
        }
    }

    



    //static void
    //    edgeGroupUnionParallel(
    //        GA_Detail* const geo,
    //        GA_EdgeGroup* const group,
    //        const GA_Group* const groupRef,
    //        const exint subscribeRatio = 64,
    //        const exint minGrainSize = 128
    //    )
    //{
    //    GA_GroupType groupTypeRef = groupRef->classType();
    //    switch (groupTypeRef)
    //    {
    //    case GA_GROUP_PRIMITIVE:
    //    {
    //        const GA_Topology& topo = geo->getTopology();
    //        const GA_ATITopology* const pointRef = topo.getPointRef();

    //        const GA_SplittableRange geoSplittableRange(geo->getPrimitiveRange(static_cast<const GA_PrimitiveGroup*>(groupRef)));
    //        UTparallelFor(geoSplittableRange, [geo, group, pointRef](const GA_SplittableRange& r)
    //            {
    //                GA_Offset start, end;
    //                for (GA_Iterator it(r); it.blockAdvance(start, end); )
    //                {
    //                    for (GA_Offset elemoff = start; elemoff < end; ++elemoff)
    //                    {
    //                        const GA_OffsetListRef& vertices = geo->getPrimitiveVertexList(elemoff);
    //                        const GA_Size numvtx = vertices.size();

    //                        GA_Offset vtxoff_prev = pointRef->getLink(vertices[0]);
    //                        GA_Offset vtxoff_next;
    //                        for (GA_Size vtxpnum = 1; vtxpnum < numvtx; ++vtxpnum)
    //                        {
    //                            vtxoff_next = pointRef->getLink(vertices[vtxpnum]);
    //                            group->add(vtxoff_prev, vtxoff_next);
    //                            vtxoff_prev = vtxoff_next;
    //                        }
    //                        if (geo->getPrimitiveClosedFlag(elemoff))
    //                        {
    //                            vtxoff_next = pointRef->getLink(vertices[0]);
    //                            group->add(vtxoff_prev, vtxoff_next);
    //                        }
    //                    }
    //                }
    //            }, subscribeRatio, minGrainSize);
    //    }
    //    break;
    //    case GA_GROUP_POINT:
    //    {
    //        const GA_SplittableRange geoSplittableRange(geo->getPointRange(static_cast<const GA_PointGroup*>(groupRef)));
    //        UTparallelFor(geoSplittableRange, [geo, group](const GA_SplittableRange& r)
    //            {
    //                GA_Offset start, end;
    //                for (GA_Iterator it(r); it.blockAdvance(start, end); )
    //                {
    //                    for (GA_Offset elemoff = start; elemoff < end; ++elemoff)
    //                    {
    //                        GA_Offset pt_next;
    //                        for (GA_Offset vtxoff_next = geo->pointVertex(elemoff); vtxoff_next != GA_INVALID_OFFSET; vtxoff_next = geo->vertexToNextVertex(vtxoff_next))
    //                        {
    //                            const GA_Offset primoff = geo->vertexPrimitive(vtxoff_next);
    //                            const GA_Size numvtx = geo->getPrimitiveVertexCount(primoff);
    //                            const GA_Size vtxpnum = GFE_DetailBase::vertexPrimIndex(geo, primoff, vtxoff_next);

    //                            if (vtxpnum == 0)
    //                            {
    //                                if (geo->getPrimitiveClosedFlag(primoff))
    //                                {
    //                                    pt_next = geo->vertexPoint(geo->getPrimitiveVertexOffset(primoff, numvtx - 1));
    //                                    group->add(elemoff, pt_next);
    //                                }
    //                            }
    //                            else
    //                            {
    //                                pt_next = geo->vertexPoint(geo->getPrimitiveVertexOffset(primoff, vtxpnum - 1));
    //                                group->add(elemoff, pt_next);
    //                            }

    //                            const GA_Size vtxpnum_next = vtxpnum + 1;
    //                            if (vtxpnum_next == numvtx) {
    //                                if (geo->getPrimitiveClosedFlag(primoff))
    //                                {
    //                                    pt_next = geo->vertexPoint(geo->getPrimitiveVertexOffset(primoff, 0));
    //                                    group->add(elemoff, pt_next);
    //                                }
    //                            }
    //                            else
    //                            {
    //                                pt_next = geo->vertexPoint(geo->getPrimitiveVertexOffset(primoff, vtxpnum_next));
    //                                group->add(elemoff, pt_next);
    //                            }
    //                        }
    //                    }
    //                }
    //            }, subscribeRatio, minGrainSize);
    //    }
    //    break;
    //    case GA_GROUP_VERTEX:
    //    {
    //        const GA_SplittableRange geoSplittableRange(geo->getVertexRange(static_cast<const GA_VertexGroup*>(groupRef)));
    //        UTparallelFor(geoSplittableRange, [geo, group](const GA_SplittableRange& r)
    //            {
    //                GA_Offset start, end;
    //                for (GA_Iterator it(r); it.blockAdvance(start, end); )
    //                {
    //                    for (GA_Offset elemoff = start; elemoff < end; ++elemoff)
    //                    {
    //                        const GA_Offset dstpt = GFE_DetailBase::vertexPointDst(geo, elemoff);
    //                        group->add(geo->vertexPoint(elemoff), dstpt);
    //                    }
    //                }
    //            }, subscribeRatio, minGrainSize);
    //    }
    //    break;
    //    case GA_GROUP_EDGE:
    //        group->combine(groupRef);
    //        break;
    //    default:
    //        break;
    //    }
    //}



}; // End of Namespace GroupUnion
_GFE_END
#endif