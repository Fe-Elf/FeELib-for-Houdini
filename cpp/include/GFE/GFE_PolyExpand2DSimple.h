
#pragma once

#ifndef __GFE_PolyExpand2DSimple_h__
#define __GFE_PolyExpand2DSimple_h__



//#include "GFE/GFE_PolyExpand2DSimple.h"

#include "GFE/GFE_GeoFilter.h"


#include "GEO/GEO_PrimPoly.h"


enum class GFE_PolyExpand2DSimpleType
{
    AUTO,
    OPEN,
    CLOSE,
};




// can not use in parallel unless for each GA_Detail
class GFE_PolyExpand2DSimple : public GFE_AttribFilter {

public:
    //using GFE_AttribFilter::GFE_AttribFilter;

    GFE_PolyExpand2DSimple(
        GA_Detail& geo,
        const GA_Detail* const geoOrigin = nullptr,
        const SOP_NodeVerb::CookParms* const cookparms = nullptr
    )
        : GFE_AttribFilter(geo, cookparms)
        , groupParser_cutPoint(geo, groupParser.getGOP(), cookparms)
    {
        if (geoOrigin)
        {
            geo.replaceWith(*geoOrigin);
            this->geoOrigin = geoOrigin;
        }
        else
        {
            geoOriginTmp = new GU_Detail();
            geoOrigin_h.allocateAndSet(geoOriginTmp);
            geoOriginTmp->replaceWith(geo);

            this->geoOrigin = geoOriginTmp;
        }
    }


    ~GFE_PolyExpand2DSimple()
    {
    }


    void setGroup(
        const GA_PointGroup* const geoPointGroup = nullptr,
        const GA_PrimitiveGroup* const geoPrimGroup = nullptr
    )
    {
        groupParser_cutPoint.setGroup(geoPointGroup);
        groupParser.setGroup(geoPrimGroup);
    }

    void setGroup(
        const UT_StringHolder& cutPointGroupName = "",
        const UT_StringHolder& primGroupName = ""
    )
    {
        groupParser_cutPoint.setGroup(GA_GROUP_POINT, cutPointGroupName);
        groupParser.setGroup(GA_GROUP_PRIMITIVE, primGroupName);
    }

    SYS_FORCE_INLINE void createSrcPrimAttrib(
        const UT_StringHolder& attribName = "srcPrim",
        const GA_Storage storage = GA_STORE_INVALID
    )
    {
        getOutAttribArray().findOrCreateTuple(false, GA_ATTRIB_PRIMITIVE, GA_STORECLASS_INT, storage, attribName, 1, GA_Defaults(-1), false);
    }

    SYS_FORCE_INLINE void createSrcPointAttrib(
        const UT_StringHolder& attribName = "srcPoint",
        const GA_Storage storage = GA_STORE_INVALID
    )
    {
        getOutAttribArray().findOrCreateTuple(false, GA_ATTRIB_POINT, GA_STORECLASS_INT, storage, attribName, 1, GA_Defaults(-1));
    }


    void setComputeParm(
        const bool cutPoint = false,
        const bool mergePrimEndsIfClosed = true,
        const GFE_PolyExpand2DSimpleType polyType = GFE_PolyExpand2DSimpleType::AUTO
    )
    {
        setHasComputed();
        this->cutPoint = cutPoint;
        this->mergePrimEndsIfClosed = mergePrimEndsIfClosed;
        this->polyType = polyType;
    }



    virtual SYS_FORCE_INLINE void bumpDataIdsForAddOrRemove() const override
    {
        geo->bumpDataIdsForAddOrRemove(cutPoint, true, true);
        if (!cutPoint)
            bumpDataId();
    }

private:

    // can not use in parallel unless for each GA_Detail
    virtual bool
        computeCore() override
    {
        if (groupParser.isEmpty())
            return true;

        polyCut();

        return true;
    }



#define TMP_CutPointGroup_GFE_PolyExpand2DSimple "__tmp_cutPointGroup_GFE_PolyExpand2DSimple"


    SYS_FORCE_INLINE void appendPoint_polyCut(
        GA_Offset& primpoint,
        GA_PointGroup* const cutPointGroup
    )
    {
        UT_ASSERT_MSG(!cutPointGroup->isDetached(), "cutPointGroup must be Not Detached");
        GA_Offset new_primpoint = geo->appendPoint();
        geo->copyPoint(new_primpoint, primpoint);
        //bool a = cutPointGroup->isOrdered();
        cutPointGroup->setElement(new_primpoint, false);
        primpoint = new_primpoint;
    }


    SYS_FORCE_INLINE GEO_PrimPoly* appendPrimitive_polyCut(
        const bool closeFlag,
        const GA_Offset primpoint,
        const GA_Offset elemoff
    )
    {
        GEO_PrimPoly* const newPrim = static_cast<GEO_PrimPoly*>(geo->appendPrimitive(GA_PrimitiveTypeId(1)));
        geo->copyAttributes(GA_ATTRIB_PRIMITIVE, newPrim->getMapOffset(), *geoOrigin, elemoff);
        geo->setPrimitiveClosedFlag(newPrim->getMapOffset(), closeFlag);

        newPrim->appendVertex(primpoint);
        return newPrim;
    }


    // can not use in parallel unless for each GA_Detail
    void polyCut()
    {
        geo->replaceWithPoints(*geoOrigin);
        
        const GA_PointGroup* const cutPointGroup = groupParser_cutPoint.getPointGroup();
        const GA_PrimitiveGroup* const primGroup = groupParser.getPrimitiveGroup();

        GA_PointGroup* tmp_cutPointGroup = nullptr;
        if (cutPoint)
        {
            tmp_cutPointGroup = static_cast<GA_PointGroup*>(geo->pointGroups().newGroup(TMP_CutPointGroup_GFE_PolyExpand2DSimple));
            if (cutPointGroup)
            {
                //tmp_cutPointGroup->combine(cutPointGroup);
                //*tmp_cutPointGroup = *cutPointGroup;
                tmp_cutPointGroup->copyMembership(*cutPointGroup, false);
            }
            else
            {
                tmp_cutPointGroup->setElement(geo->getPointRange(), true);
            }
        }

        //const GA_Topology& topo = geo->getTopology();
        const GA_Topology& topoRef = geoOrigin->getTopology();
        const GA_ATITopology* const vtxPointRef = topoRef.getPointRef();

        bool closeFlag;
        switch (polyType)
        {
        case GFE_PolyExpand2DSimpleType::AUTO:
            break;
        case GFE_PolyExpand2DSimpleType::OPEN:
            closeFlag = false;
            break;
        case GFE_PolyExpand2DSimpleType::CLOSE:
            closeFlag = true;
            break;
        default:
            closeFlag = false;
            break;
        }
        GEO_PrimPoly* newPrim = nullptr;
        GA_Offset start, end;
        for (GA_Iterator it(geoOrigin->getPrimitiveRange(primGroup)); it.fullBlockAdvance(start, end); )
        {
            for (GA_Offset elemoff = start; elemoff < end; ++elemoff)
            {
                if (polyType == GFE_PolyExpand2DSimpleType::AUTO)
                {
                    closeFlag = geoOrigin->getPrimitiveClosedFlag(elemoff);
                }
                const GA_OffsetListRef& vertices = geoOrigin->getPrimitiveVertexList(elemoff);
                //const GA_Size numvtx = geoOrigin->getPrimitiveVertexCount(elemoff) - 1;
                const GA_Size numvtx = vertices.size() - 1;
                if (numvtx == -1) continue;

                //const GEO_Primitive* const srcPrim = static_cast<const GEO_Primitive*>(geoOrigin->getPrimitive(elemoff));

                GA_Offset primpoint = vtxPointRef->getLink(vertices[0]);


                if (mergePrimEndsIfClosed && (cutPointGroup && !cutPointGroup->contains(primpoint)) && (geoOrigin->getPrimitiveClosedFlag(elemoff) || primpoint == vtxPointRef->getLink(vertices[numvtx])))
                {
                    GA_Size firstPrimIndex = 0;
                    for (GA_Size vtxpnum = 1; vtxpnum <= numvtx; ++vtxpnum) {
                        primpoint = vtxPointRef->getLink(vertices[vtxpnum]);
                        if (cutPointGroup && !cutPointGroup->contains(primpoint)) continue;
                        firstPrimIndex = vtxpnum;
                        break;
                    }

                    if (firstPrimIndex == 0) {
                        primpoint = vtxPointRef->getLink(vertices[0]);
                    }

                    if (cutPoint)
                    {
                        appendPoint_polyCut(primpoint, tmp_cutPointGroup);
                    }
                    firstPrimIndex;
                    primpoint;

                    newPrim = appendPrimitive_polyCut(closeFlag, primpoint, elemoff);

                    if (cutPoint)
                    {
                        for (GA_Size vtxpnum = firstPrimIndex + 1; vtxpnum <= numvtx; ++vtxpnum) {
                            primpoint = vtxPointRef->getLink(vertices[vtxpnum]);
                            if (!cutPointGroup || cutPointGroup->contains(primpoint))
                            {
                                appendPoint_polyCut(primpoint, tmp_cutPointGroup);
                                newPrim->appendVertex(primpoint);
                                appendPoint_polyCut(primpoint, tmp_cutPointGroup);
                                newPrim = appendPrimitive_polyCut(closeFlag, primpoint, elemoff);
                            }
                            else
                            {
                                newPrim->appendVertex(primpoint);
                            }
                        }
                    }
                    else
                    {
                        for (GA_Size vtxpnum = firstPrimIndex + 1; vtxpnum <= numvtx; ++vtxpnum) {
                            primpoint = vtxPointRef->getLink(vertices[vtxpnum]);
                            newPrim->appendVertex(primpoint);
                            if (cutPointGroup && !cutPointGroup->contains(primpoint)) continue;

                            newPrim = appendPrimitive_polyCut(closeFlag, primpoint, elemoff);
                        }
                    }

                    for (GA_Size vtxpnum = !geoOrigin->getPrimitiveClosedFlag(elemoff); vtxpnum <= firstPrimIndex; ++vtxpnum) {
                        primpoint = vtxPointRef->getLink(vertices[vtxpnum]);
                        newPrim->appendVertex(primpoint);
                    }

                    if (cutPoint)
                    {
                        primpoint = vtxPointRef->getLink(vertices[firstPrimIndex]);
                        appendPoint_polyCut(primpoint, tmp_cutPointGroup);
                        newPrim->appendVertex(primpoint);
                    }
                }
                else
                {
                    if (cutPoint)
                    {
                        newPrim = static_cast<GEO_PrimPoly*>(geo->appendPrimitive(GA_PrimitiveTypeId(1)));
                        geo->copyAttributes(GA_ATTRIB_PRIMITIVE, newPrim->getMapOffset(), *geoOrigin, elemoff);
                        geo->setPrimitiveClosedFlag(newPrim->getMapOffset(), closeFlag);
                    }
                    else
                    {
                        newPrim = appendPrimitive_polyCut(closeFlag, primpoint, elemoff);
                    }

                    if (cutPoint)
                    {
                        if (!cutPointGroup || cutPointGroup->contains(primpoint))
                        {
                            appendPoint_polyCut(primpoint, tmp_cutPointGroup);
                        }
                        newPrim->appendVertex(primpoint);
                        for (GA_Size vtxpnum = 1; vtxpnum < numvtx; ++vtxpnum) {
                            primpoint = vtxPointRef->getLink(vertices[vtxpnum]);

                            if (!cutPointGroup || cutPointGroup->contains(primpoint))
                            {
                                appendPoint_polyCut(primpoint, tmp_cutPointGroup);
                                newPrim->appendVertex(primpoint);
                                appendPoint_polyCut(primpoint, tmp_cutPointGroup);
                                newPrim = appendPrimitive_polyCut(closeFlag, primpoint, elemoff);
                            }
                            else
                            {
                                newPrim->appendVertex(primpoint);
                            }
                        }
                    }
                    else
                    {
                        for (GA_Size vtxpnum = 1; vtxpnum < numvtx; ++vtxpnum) {
                            primpoint = vtxPointRef->getLink(vertices[vtxpnum]);
                            newPrim->appendVertex(primpoint);

                            if (cutPointGroup && !cutPointGroup->contains(primpoint)) continue;

                            newPrim = appendPrimitive_polyCut(closeFlag, primpoint, elemoff);
                        }
                    }


                    primpoint = vtxPointRef->getLink(vertices[numvtx]);
                    if (cutPoint && (!cutPointGroup || cutPointGroup->contains(primpoint)))
                    {
                        appendPoint_polyCut(primpoint, tmp_cutPointGroup);
                    }
                    newPrim->appendVertex(primpoint);

                    if (geoOrigin->getPrimitiveClosedFlag(elemoff) && polyType == GFE_PolyExpand2DSimpleType::OPEN) {

                        if (cutPoint)
                        {
                            appendPoint_polyCut(primpoint, tmp_cutPointGroup);
                        }

                        newPrim = appendPrimitive_polyCut(closeFlag, primpoint, elemoff);
                        primpoint = vtxPointRef->getLink(vertices[0]);

                        if (cutPoint)
                        {
                            appendPoint_polyCut(primpoint, tmp_cutPointGroup);
                        }

                        newPrim->appendVertex(primpoint);
                    }
                }
            }
        }
        if (cutPoint)
        {
            //geo->destroyUnusedPoints(geo->getPointRange(tmp_cutPointGroup));
            geo->destroyPointOffsets(geo->getPointRange(tmp_cutPointGroup), GA_Detail::GA_DestroyPointMode::GA_LEAVE_PRIMITIVES, true);
            geo->destroyGroup(tmp_cutPointGroup);

            //geo->edgeGroups()
        }
    }



public:
    GFE_GroupParser groupParser_cutPoint;

    bool cutPoint = false;
    bool mergePrimEndsIfClosed = true;
    GFE_PolyExpand2DSimpleType polyType = GFE_PolyExpand2DSimpleType::AUTO;

private:
    GU_DetailHandle geoOrigin_h;
    const GA_Detail* geoOrigin;
    GU_Detail* geoOriginTmp;
};































//
//
//
//
//
//namespace GFE_PolyExpand2DSimple_Namespace {
//
//
//
//
//#define TMP_CutPointGroup_GFE_PolyExpand2DSimple "__tmp_cutPointGroup_GFE_PolyExpand2DSimple"
//
//
//SYS_FORCE_INLINE
//static void
//appendPoint_polyCut(
//    GA_Detail* const geoPoint,
//    GA_Offset& primpoint,
//    GA_PointGroup* const cutPointGroup
//) {
//    UT_ASSERT_MSG(!cutPointGroup->isDetached(), "cutPointGroup must be Not Detached");
//    GA_Offset new_primpoint = geoPoint->appendPoint();
//    geoPoint->copyPoint(new_primpoint, primpoint);
//    //bool a = cutPointGroup->isOrdered();
//    cutPointGroup->setElement(new_primpoint, false);
//    primpoint = new_primpoint;
//}
//
//
//SYS_FORCE_INLINE
//static GEO_PrimPoly*
//appendPrimitive_polyCut(
//    GA_Detail* const geoPoint,
//    const GA_Detail* geoFull,
//    const bool closeFlag,
//    const GA_Offset primpoint,
//    const GA_Offset elemoff
//) {
//    GEO_PrimPoly* const newPrim = static_cast<GEO_PrimPoly*>(geoPoint->appendPrimitive(GA_PrimitiveTypeId(1)));
//    geoPoint->copyAttributes(GA_ATTRIB_PRIMITIVE, newPrim->getMapOffset(), *geoFull, elemoff);
//    geoPoint->setPrimitiveClosedFlag(newPrim->getMapOffset(), closeFlag);
//
//    newPrim->appendVertex(primpoint);
//    return newPrim;
//}
//
//
////SYS_FORCE_INLINE
////static void
////appendVertex_polyCut(
////    GA_Offset& primpoint,
////    GEO_PrimPoly* newPrim,
////    const GA_ATITopology* const vtxPointRef,
////    GA_OffsetListRef& vertices,
////    GA_Size vtxpnum
////) {
////    primpoint = vtxPointRef->getLink(vertices[vtxpnum]);
////    newPrim->appendVertex(primpoint);
////}
//
//
//
//
//
//
//// can not use in parallel unless for each GA_Detail
//static void
//polyCut(
//    GA_Detail* const geoPoint,
//    const GA_Detail* const geoFull,
//    const GA_PointGroup* const cutPointGroup,
//    const GA_PrimitiveGroup* const primGroup = nullptr,
//    const bool cutPoint = false,
//    const bool mergePrimEndsIfClosed = true,
//    const GFE_PolyExpand2DSimpleType polyType = GFE_PolyExpand2DSimpleType::AUTO
//)
//{
//    UT_ASSERT_P(geoPoint);
//    UT_ASSERT_P(geoFull);
//    UT_ASSERT_P(geoPoint->getNumPoints() == geoFull->getNumPoints());
//
//    geoPoint->replaceWithPoints(*geoFull);
//
//    //GA_RWHandleT<GA_Offset> srcPrimsAttrib_h;
//    //if (srcPrimsAttrib)
//    //{
//    //    srcPrimsAttrib_h = srcPrimsAttrib;
//    //}
//    //GA_RWHandleT<GA_Offset> srcPointsAttrib_h;
//    //if (cutPoint && srcPointsAttrib)
//    //{
//    //    srcPointsAttrib_h = srcPointsAttrib;
//    //}
//    geoPoint->replaceWithPoints(*geoFull);
//
//    //const GA_PointGroupUPtr tmp_cutPointGroupUPtr = geoPoint->createDetachedPointGroup();
//    //GA_PointGroup* const tmp_cutPointGroup = tmp_cutPointGroupUPtr.get();
//    GA_PointGroup* tmp_cutPointGroup = nullptr;
//    if (cutPoint)
//    {
//        tmp_cutPointGroup = static_cast<GA_PointGroup*>(geoPoint->pointGroups().newGroup(TMP_CutPointGroup_GFE_PolyExpand2DSimple));
//        if (cutPointGroup)
//        {
//            //tmp_cutPointGroup->combine(cutPointGroup);
//            //*tmp_cutPointGroup = *cutPointGroup;
//            tmp_cutPointGroup->copyMembership(*cutPointGroup, false);
//        }
//        else
//        {
//            tmp_cutPointGroup->setElement(geoPoint->getPointRange(), true);
//        }
//    }
//
//    //const GA_Topology& topo = geoPoint->getTopology();
//    const GA_Topology& topoRef = geoFull->getTopology();
//    const GA_ATITopology* const vtxPointRef = topoRef.getPointRef();
//
//    bool closeFlag;
//    switch (polyType)
//    {
//    case GFE_PolyExpand2DSimpleType::AUTO:
//        break;
//    case GFE_PolyExpand2DSimpleType::OPEN:
//        closeFlag = false;
//        break;
//    case GFE_PolyExpand2DSimpleType::CLOSE:
//        closeFlag = true;
//        break;
//    default:
//        closeFlag = false;
//        break;
//    }
//    GEO_PrimPoly* newPrim = nullptr;
//    GA_Offset start, end;
//    for (GA_Iterator it(geoFull->getPrimitiveRange(primGroup)); it.fullBlockAdvance(start, end); )
//    {
//        for (GA_Offset elemoff = start; elemoff < end; ++elemoff)
//        {
//            if (polyType == GFE_PolyExpand2DSimpleType::AUTO)
//            {
//                closeFlag = geoFull->getPrimitiveClosedFlag(elemoff);
//            }
//            const GA_OffsetListRef& vertices = geoFull->getPrimitiveVertexList(elemoff);
//            //const GA_Size numvtx = geoFull->getPrimitiveVertexCount(elemoff) - 1;
//            const GA_Size numvtx = vertices.size() - 1;
//            if (numvtx == -1) continue;
//
//            //const GEO_Primitive* const srcPrim = static_cast<const GEO_Primitive*>(geoFull->getPrimitive(elemoff));
//
//            GA_Offset primpoint = vtxPointRef->getLink(vertices[0]);
//
//
//            if (mergePrimEndsIfClosed && (cutPointGroup && !cutPointGroup->contains(primpoint)) && (geoFull->getPrimitiveClosedFlag(elemoff) || primpoint == vtxPointRef->getLink(vertices[numvtx])))
//            {
//                GA_Size firstPrimIndex = 0;
//                for (GA_Size vtxpnum = 1; vtxpnum <= numvtx; ++vtxpnum) {
//                    primpoint = vtxPointRef->getLink(vertices[vtxpnum]);
//                    if (cutPointGroup && !cutPointGroup->contains(primpoint)) continue;
//                    firstPrimIndex = vtxpnum;
//                    break;
//                }
//
//                if (firstPrimIndex == 0) {
//                    primpoint = vtxPointRef->getLink(vertices[0]);
//                }
//
//                if (cutPoint)
//                {
//                    appendPoint_polyCut(geoPoint, primpoint, tmp_cutPointGroup);
//                }
//                firstPrimIndex;
//                primpoint;
//
//                newPrim = appendPrimitive_polyCut(geoPoint, geoFull, closeFlag, primpoint, elemoff);
//
//                if (cutPoint)
//                {
//                    for (GA_Size vtxpnum = firstPrimIndex + 1; vtxpnum <= numvtx; ++vtxpnum) {
//                        primpoint = vtxPointRef->getLink(vertices[vtxpnum]);
//                        if (!cutPointGroup || cutPointGroup->contains(primpoint))
//                        {
//                            appendPoint_polyCut(geoPoint, primpoint, tmp_cutPointGroup);
//                            newPrim->appendVertex(primpoint);
//                            appendPoint_polyCut(geoPoint, primpoint, tmp_cutPointGroup);
//                            newPrim = appendPrimitive_polyCut(geoPoint, geoFull, closeFlag, primpoint, elemoff);
//                        }
//                        else
//                        {
//                            newPrim->appendVertex(primpoint);
//                        }
//                    }
//                }
//                else
//                {
//                    for (GA_Size vtxpnum = firstPrimIndex + 1; vtxpnum <= numvtx; ++vtxpnum) {
//                        primpoint = vtxPointRef->getLink(vertices[vtxpnum]);
//                        newPrim->appendVertex(primpoint);
//                        if (cutPointGroup && !cutPointGroup->contains(primpoint)) continue;
//
//                        newPrim = appendPrimitive_polyCut(geoPoint, geoFull, closeFlag, primpoint, elemoff);
//                    }
//                }
//
//                for (GA_Size vtxpnum = !geoFull->getPrimitiveClosedFlag(elemoff); vtxpnum <= firstPrimIndex; ++vtxpnum) {
//                    primpoint = vtxPointRef->getLink(vertices[vtxpnum]);
//                    newPrim->appendVertex(primpoint);
//                }
//
//                if (cutPoint)
//                {
//                    primpoint = vtxPointRef->getLink(vertices[firstPrimIndex]);
//                    appendPoint_polyCut(geoPoint, primpoint, tmp_cutPointGroup);
//                    newPrim->appendVertex(primpoint);
//                }
//            }
//            else
//            {
//                if (cutPoint)
//                {
//                    newPrim = static_cast<GEO_PrimPoly*>(geoPoint->appendPrimitive(GA_PrimitiveTypeId(1)));
//                    geoPoint->copyAttributes(GA_ATTRIB_PRIMITIVE, newPrim->getMapOffset(), *geoFull, elemoff);
//                    geoPoint->setPrimitiveClosedFlag(newPrim->getMapOffset(), closeFlag);
//                }
//                else
//                {
//                    newPrim = appendPrimitive_polyCut(geoPoint, geoFull, closeFlag, primpoint, elemoff);
//                }
//
//                if (cutPoint)
//                {
//                    if (!cutPointGroup || cutPointGroup->contains(primpoint))
//                    {
//                        appendPoint_polyCut(geoPoint, primpoint, tmp_cutPointGroup);
//                    }
//                    newPrim->appendVertex(primpoint);
//                    for (GA_Size vtxpnum = 1; vtxpnum < numvtx; ++vtxpnum) {
//                        primpoint = vtxPointRef->getLink(vertices[vtxpnum]);
//
//                        if (!cutPointGroup || cutPointGroup->contains(primpoint))
//                        {
//                            appendPoint_polyCut(geoPoint, primpoint, tmp_cutPointGroup);
//                            newPrim->appendVertex(primpoint);
//                            appendPoint_polyCut(geoPoint, primpoint, tmp_cutPointGroup);
//                            newPrim = appendPrimitive_polyCut(geoPoint, geoFull, closeFlag, primpoint, elemoff);
//                        }
//                        else
//                        {
//                            newPrim->appendVertex(primpoint);
//                        }
//                    }
//                }
//                else
//                {
//                    for (GA_Size vtxpnum = 1; vtxpnum < numvtx; ++vtxpnum) {
//                        primpoint = vtxPointRef->getLink(vertices[vtxpnum]);
//                        newPrim->appendVertex(primpoint);
//
//                        if (cutPointGroup && !cutPointGroup->contains(primpoint)) continue;
//
//                        newPrim = appendPrimitive_polyCut(geoPoint, geoFull, closeFlag, primpoint, elemoff);
//                    }
//                }
//
//
//                primpoint = vtxPointRef->getLink(vertices[numvtx]);
//                if (cutPoint && (!cutPointGroup || cutPointGroup->contains(primpoint)))
//                {
//                    appendPoint_polyCut(geoPoint, primpoint, tmp_cutPointGroup);
//                }
//                newPrim->appendVertex(primpoint);
//
//                if (geoFull->getPrimitiveClosedFlag(elemoff) && polyType == GFE_PolyExpand2DSimpleType::OPEN) {
//
//                    if (cutPoint)
//                    {
//                        appendPoint_polyCut(geoPoint, primpoint, tmp_cutPointGroup);
//                    }
//
//                    newPrim = appendPrimitive_polyCut(geoPoint, geoFull, closeFlag, primpoint, elemoff);
//                    primpoint = vtxPointRef->getLink(vertices[0]);
//
//                    if (cutPoint)
//                    {
//                        appendPoint_polyCut(geoPoint, primpoint, tmp_cutPointGroup);
//                    }
//
//                    newPrim->appendVertex(primpoint);
//                }
//            }
//        }
//    }
//    if (cutPoint)
//    {
//        //geoPoint->destroyUnusedPoints(geoPoint->getPointRange(tmp_cutPointGroup));
//        geoPoint->destroyPointOffsets(geoPoint->getPointRange(tmp_cutPointGroup), GA_Detail::GA_DestroyPointMode::GA_LEAVE_PRIMITIVES, true);
//        geoPoint->destroyGroup(tmp_cutPointGroup);
//
//        //geoPoint->edgeGroups()
//    }
//}
//
//
//// can not use in parallel unless for each GA_Detail
//static void
//polyCut(
//    GA_Detail* const geoPoint,
//    GA_PointGroup* const cutPointGroup,
//    const GA_PrimitiveGroup* const primGroup = nullptr,
//    const bool cutPoint = false,
//    const bool mergePrimEndsIfClosed = true,
//    const GFE_PolyExpand2DSimpleType polyType = GFE_PolyExpand2DSimpleType::AUTO
//)
//{
//    GU_DetailHandle geoFull_h;
//    GU_Detail* const geoFull = new GU_Detail();
//    geoFull_h.allocateAndSet(geoFull);
//    geoFull->replaceWith(*geoPoint);
//
//    //GU_DetailHandle geoTmp0_h;
//    //GU_Detail* geoTmp0 = new GU_Detail();
//    //geoTmp0_h.allocateAndSet(geoTmp0);
//    //geoTmp0->replaceWith(*geoFull);
//
//    polyCut(geoPoint, geoFull, cutPointGroup, primGroup,
//        cutPoint, mergePrimEndsIfClosed, polyType);
//}
////
////static void
////polyCut(
////    GA_Detail* const geoPoint,
////    const GA_Detail* const geoFull,
////    GA_PointGroup* const cutPointGroup,
////    const GA_PrimitiveGroup* const primGroup = nullptr,
////    const bool cutPoint = false,
////    const bool mergePrimEndsIfClosed = true,
////    const GFE_PolyExpand2DSimpleType polyType = GFE_PolyExpand2DSimpleType::AUTO,
////    const bool delInputPointGroup = false
////)
////{
////    polyCut(geoPoint, geoFull, cutPointGroup, primGroup, cutPoint, mergePrimEndsIfClosed, polyType);
////
////    if (delInputPointGroup && cutPointGroup && !cutPointGroup->isDetached())
////    {
////        geoPoint->destroyGroup(cutPointGroup);
////    }
////}
//
//
//// can not use in parallel unless for each GA_Detail
//static void
//polyCut(
//    GA_Detail* const geoPoint,
//    const GA_Detail* const geoFull,
//    //const bool createSrcPrimAttrib = false,
//    const UT_StringHolder& srcPrimAttribName,
//    //const bool createSrcPointAttrib = false,
//    const UT_StringHolder& srcPointAttribName,
//    const GA_PointGroup* const cutPointGroup,
//    const GA_Storage storage = GA_STORE_INVALID,
//    const GA_PrimitiveGroup* const primGroup = nullptr,
//    const bool cutPoint = false,
//    const bool mergePrimEndsIfClosed = true,
//    const GFE_PolyExpand2DSimpleType polyType = GFE_PolyExpand2DSimpleType::AUTO
//
//)
//{
//    const GA_Storage finalStorageI = storage == GA_STORE_INVALID ? GFE_Type::getPreferredStorageI(geoPoint) : storage;
//    if (srcPrimAttribName.length() != 0 && srcPrimAttribName.isstring())
//    {
//        geoPoint->getAttributes().createTupleAttribute(GA_ATTRIB_PRIMITIVE, srcPrimAttribName, finalStorageI, 1, GA_Defaults(-1));
//    }
//
//    if (srcPointAttribName.length() != 0 && srcPointAttribName.isstring())
//    {
//        geoPoint->getAttributes().createTupleAttribute(GA_ATTRIB_POINT, srcPointAttribName, finalStorageI, 1, GA_Defaults(-1));
//    }
//
//    polyCut(geoPoint, geoFull, cutPointGroup, primGroup,
//        cutPoint, mergePrimEndsIfClosed, polyType);
//}
//
//
//
//// can not use in parallel unless for each GA_Detail
//static void
//polyCut(
//    const SOP_NodeVerb::CookParms& cookparms,
//    GA_Detail* const geoPoint,
//    const GA_Detail* const geoFull,
//    const UT_StringHolder& cutPointGroupName = "",
//    const UT_StringHolder& primGroupName = "",
//    const bool cutPoint = false,
//    const bool mergePrimEndsIfClosed = true,
//    const GFE_PolyExpand2DSimpleType polyType = GFE_PolyExpand2DSimpleType::AUTO,
//
//    const GA_Storage storage = GA_STORE_INVALID,
//    //const bool createSrcPrimAttrib = false,
//    const UT_StringHolder& srcPrimAttribName = "",
//    //const bool createSrcPointAttrib = false,
//    const UT_StringHolder& srcPointAttribName = "",
//    const bool delInputPointGroup = false
//)
//{
//    GOP_Manager gop;
//    GA_PointGroup* const cutPointGroup = GFE_GroupParser_Namespace::findOrParsePointGroupDetached(cookparms, geoPoint, cutPointGroupName, gop);
//    if (cutPointGroup && cutPointGroup->isEmpty())
//    {
//        geoPoint->replaceWith(*geoFull);
//        return;
//    }
//
//    const GA_PrimitiveGroup* const primGroup = GFE_GroupParser_Namespace::findOrParsePrimitiveGroupDetached(cookparms, geoFull, primGroupName, gop);
//    if (primGroup && primGroup->isEmpty())
//    {
//        geoPoint->replaceWith(*geoFull);
//        return;
//    }
//
//    polyCut(geoPoint, geoFull,
//        srcPrimAttribName, srcPointAttribName, cutPointGroup, 
//        storage, primGroup,
//        cutPoint, mergePrimEndsIfClosed, polyType);
//
//    if (delInputPointGroup && cutPointGroup && !cutPointGroup->isDetached())
//    {
//        geoPoint->destroyGroup(cutPointGroup);
//    }
//}
//
//
//
//
//
//
//} // End of namespace GFE_PolyExpand2DSimple






#endif
