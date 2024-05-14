
#pragma once

#ifndef __GFE_Carve_h__
#define __GFE_Carve_h__

#include <GFE/GFE_Carve.h>

#include <GFE/GFE_GeoFilter.h>





#include <GFE/Attribute.h>
#include <GFE/GFE_CurveUV.h>
#include <GFE/GFE_DeleteVertex.h>
#include <GFE/GFE_Enumerate.h>
#include <GFE/GFE_Math.h>

enum class GFE_CarveSpace {
    CustomAttrib,
    LocalAverage,
    LocalArcLength,
    WorldAverage,
    WorldArcLength,
};


enum class GFE_CarveType {
    Start,
    End,
    Both,
};






class GFE_Carve : public GFE_AttribFilter {

public:
    //using GFE_AttribFilter::GFE_AttribFilter;

    GFE_Carve(
        GA_Detail& inGeo,
        //const GA_Detail* const geoSrc = nullptr,
        const SOP_NodeVerb::CookParms* const cookparms = nullptr
    )
        //: GFE_AttribFilter(inGeo, geoSrc, cookparms)
        : GFE_AttribFilter(inGeo, cookparms)
        , groupParser_carveStart(geo, groupParser.getGOPRef(), cookparms)
        , groupParser_carveEnd  (geo, groupParser.getGOPRef(), cookparms)
        , enumerate(geo, cookparms)
        //, interpAttribArray(geoSrc ? geoSrc : geo, cookparms)
        , interpAttribArray(geo, cookparms)
    {
    }

    //SYS_FORCE_INLINE void setGroup(const GA_PrimitiveGroup* const geoPrimGroup = nullptr)
    //{ groupParser.setGroup(geoPrimGroup); }

    //SYS_FORCE_INLINE void setGroup(const UT_StringRef& primGroupName = "")
    //{ groupParser.setGroup(GA_GROUP_PRIMITIVE, primGroupName);}

    //SYS_FORCE_INLINE void setCarveStartGroup(const GA_PrimitiveGroup* const geoPrimGroup = nullptr)
    //{ groupParser_carveStart.setGroup(geoPrimGroup); }

    //SYS_FORCE_INLINE void setCarveEndGroup(const GA_PrimitiveGroup* const geoPrimGroup = nullptr)
    //{ groupParser_carveEnd.setGroup(geoPrimGroup); }

    //SYS_FORCE_INLINE void setCarveStartGroup(const UT_StringRef& primGroupName = "")
    //{ groupParser_carveStart.setGroup(GA_GROUP_PRIMITIVE, primGroupName); }

    //SYS_FORCE_INLINE void setCarveEndGroup(const UT_StringRef& primGroupName = "")
    //{ groupParser_carveEnd.setGroup(GA_GROUP_PRIMITIVE, primGroupName); }

    void
        setComputeParm(
            const GFE_CarveSpace carveSpace = GFE_CarveSpace::WorldArcLength,
            const bool keepOutsideStart = false,
            const bool keepOutsideEnd = false,
            const bool keepInside = true,
            const bool absCarveUEnd = false,
            const bool delReversedInsidePrim = false,
            
            const exint subscribeRatio = 64,
            const exint minGrainSize = 1024
        )
    {
        setHasComputed();
        this->carveSpace = carveSpace;
        this->keepOutsideStart = keepOutsideStart;
        this->keepOutsideEnd   = keepOutsideEnd;
        this->keepInside       = keepInside;
        this->absCarveUEnd = absCarveUEnd;
        this->delReversedInsidePrim = delReversedInsidePrim;
        
        this->subscribeRatio = subscribeRatio;
        this->minGrainSize   = minGrainSize;
    }
    
    
   
    SYS_FORCE_INLINE GA_PrimitiveGroup* setCarvedStartPrimGroup(const bool detached = true, const UT_StringRef& groupName = "")
    { return hasCarveStartPrimGroup = getOutGroupArray().findOrCreatePrimitive(detached, groupName); }
    
    SYS_FORCE_INLINE GA_PrimitiveGroup* setCarvedEndPrimGroup(const bool detached = true, const UT_StringRef& groupName = "")
    { return hasCarveEndPrimGroup = getOutGroupArray().findOrCreatePrimitive(detached, groupName); }
    
    template<bool IsEnd>
    void setCarveU(const fpreal carveULocal, const fpreal carveUWorld)
    {
        const bool IsLocal = carveSpace == GFE_CarveSpace::LocalAverage || carveSpace == GFE_CarveSpace::LocalArcLength;
        const fpreal carveU = IsLocal ? carveULocal : carveUWorld;
        if constexpr(IsEnd)
            carveUEnd = carveU;
        else
            carveUStart = carveU;
    }
    
    void setUVAttrib(const UT_StringRef& name = "uv")
    {
        setHasComputed();
        if (carveSpace == GFE_CarveSpace::CustomAttrib)
            uvAttrib = GFE_Attribute::findUVAttributePointVertex(*geo, GA_ATTRIB_INVALID, name);
        else
            uvAttrib = nullptr;
    }

    
    void setCarveUStartAttrib(const GA_Attribute* const carveUAttrib, const bool delCarveUAttrib)
    {
        carveUAttribStart    = carveUAttrib;
        delCarveUAttribStart = delCarveUAttrib;
        if (carveUAttribStart)
            startCarveU_h = carveUAttribStart;

        UT_ASSERT_MSG(carveUAttribStart->getOwner() >= 3, "not correct attrib owner");
    }

    SYS_FORCE_INLINE void setCarveUStartAttrib(GA_Attribute* const carveUAttrib, const bool delCarveUAttrib)
    {
        carveUAttribStart_nonConst = carveUAttrib;
        setCarveUStartAttrib(static_cast<const GA_Attribute*>(carveUAttrib), delCarveUAttrib);
    }

    void setCarveUStartAttrib(const UT_StringRef& carveUAttribName = "carveUStart", const bool delCarveUAttrib = false)
    {
        carveUAttribStart_nonConst = geo->findPrimitiveAttribute(carveUAttribName);
        if (!carveUAttribStart_nonConst)
        {
            carveUAttribStart_nonConst = geo->findPointAttribute(carveUAttribName);
            if (!carveUAttribStart_nonConst)
            {
                carveUAttribStart_nonConst = geo->findVertexAttribute(carveUAttribName);
            }
        }
        if (carveUAttribStart_nonConst)
            setCarveUStartAttrib(carveUAttribStart_nonConst, delCarveUAttrib);
        else
            delCarveUAttribStart = delCarveUAttrib;
    }



    
    void setCarveUEndAttrib(const GA_Attribute* const carveUAttrib, const bool delCarveUAttrib)
    {
        endCarveUAttrib = carveUAttrib;
        delCarveUEndAttrib = delCarveUAttrib;
        if (endCarveUAttrib)
            endCarveU_h = endCarveUAttrib;

        UT_ASSERT_MSG(endCarveUAttrib->getOwner() >= 3, "not correct attrib owner");
    }

    SYS_FORCE_INLINE void setCarveUEndAttrib(GA_Attribute* const carveUAttrib, const bool delCarveUAttrib)
    {
        carveUEndAttrib_nonConst = carveUAttrib;
        setCarveUEndAttrib(static_cast<const GA_Attribute*>(carveUAttrib), delCarveUAttrib);
    }

    
    void setCarveUEndAttrib(const UT_StringRef& carveUAttribName = "carveUEnd", const bool delCarveUAttrib = false)
    {
        carveUEndAttrib_nonConst = geo->findPrimitiveAttribute(carveUAttribName);
        if (!carveUEndAttrib_nonConst)
        {
            carveUEndAttrib_nonConst = geo->findPointAttribute(carveUAttribName);
            if (!carveUEndAttrib_nonConst)
            {
                carveUEndAttrib_nonConst = geo->findVertexAttribute(carveUAttribName);
            }
        }
        if (carveUEndAttrib_nonConst)
        {
            setCarveUEndAttrib(carveUEndAttrib_nonConst, delCarveUAttrib);
        }
        else
        {
            delCarveUEndAttrib = delCarveUAttrib;
        }
    }


private:

virtual bool
    computeCore() override
{
    if (!(keepOutsideStart || keepOutsideEnd || keepInside) || groupParser.isEmpty() || (carveSpace == GFE_CarveSpace::CustomAttrib && !uvAttrib) )
    {
        geo->clear();
        return true;
    }
    
    if (geoSrc)
        geo->replaceWith(*geoSrc);
    
    if (!posAttrib)
        posAttrib = geo->getP();

    //interpAttribArray.uappend();
    
    enumerate.compute();
    
    GFE_CurveUV curveUV(geo, cookparms);
    curveUV.setPositionAttrib(posAttrib);
    if (carveSpace != GFE_CarveSpace::CustomAttrib)
    {
        switch (carveSpace)
        {
        default:
        case GFE_CarveSpace::LocalAverage:   curveUV.curveUVMethod = GFE_CurveUVMethod::LocalAverage;   break;
        case GFE_CarveSpace::LocalArcLength: curveUV.curveUVMethod = GFE_CurveUVMethod::LocalArcLength; break;
        case GFE_CarveSpace::WorldAverage:   curveUV.curveUVMethod = GFE_CurveUVMethod::WorldAverage;   break;
        case GFE_CarveSpace::WorldArcLength: curveUV.curveUVMethod = GFE_CurveUVMethod::WorldArcLength; break;
        }
        curveUV.groupParser.setGroup(groupParser.getPrimitiveGroup());
        uvAttrib = curveUV.getOutAttribArray().findOrCreateUV(true, GA_ATTRIB_VERTEX, GA_STORE_INVALID);
        uv_h = uvAttrib;
        curveUV.compute();
    }
    UT_ASSERT_MSG(!uvAttrib || uvAttrib->getOwner() == GA_ATTRIB_POINT || uvAttrib->getOwner() == GA_ATTRIB_VERTEX, "Unhandled UV Attrib Owner");

    carveSpace_isWorld = carveSpace == GFE_CarveSpace::CustomAttrib   ||
                         carveSpace == GFE_CarveSpace::WorldAverage   ||
                         carveSpace == GFE_CarveSpace::WorldArcLength;

    if (carveSpace != GFE_CarveSpace::LocalAverage)
        uvAIF = uvAttrib->getAIFTuple();
    isPointUV = uvAttrib && uvAttrib->getOwner() == GA_ATTRIB_POINT;
    //uvTupleSize = uvAttrib->getTupleSize();


    
    carve();

    if (delCarveUAttribStart && carveUAttribStart_nonConst)
        geo->getAttributes().destroyAttribute(carveUAttribStart_nonConst);
    
    if (delCarveUEndAttrib && carveUEndAttrib_nonConst)
        geo->getAttributes().destroyAttribute(carveUEndAttrib_nonConst);
    
    return true;
}



void carve()
{
    GU_DetailHandle geoSingle_h;
        
    GU_DetailHandle geoMerge_h;
    geoMerge = new GU_Detail();
    geoMerge_h.allocateAndSet(geoMerge);
    
    if ((keepOutsideStart && keepOutsideEnd) || ((keepOutsideStart || keepOutsideEnd) && keepInside))
    //if (1)
    {
        geoSingle = new GU_Detail();
        geoSingle_h.allocateAndSet(geoSingle);
        if (keepOutsideStart)
        {
            geoSingle->merge(*geo->asGEO_Detail());
            carve<GFE_CarveType::Start>();
            geoMerge->baseMerge(*geoSingle);
        }
        if (keepOutsideEnd)
        {
            geoSingle->clear();
            geoSingle->merge(*geo->asGEO_Detail());
            carve<GFE_CarveType::End>();
            geoMerge->baseMerge(*geoSingle);
        }
        if (keepInside)
        {
            geoSingle->clear();
            geoSingle->merge(*geo->asGEO_Detail());
            carve<GFE_CarveType::Both>();
            geoMerge->baseMerge(*geoSingle);
        }
        geo->replaceWith(*geoMerge);
    }
    else
    {
        if (keepOutsideStart)
        {
            geoSingle = geo->asGU_Detail();
            carve<GFE_CarveType::Start>();
        }
        else if (keepOutsideEnd)
        {
            geoSingle = geo->asGU_Detail();
            carve<GFE_CarveType::End>();
        }
        else// if (keepInside)
        {
            geoSingle = new GU_Detail();
            geoSingle_h.allocateAndSet(geoSingle);
            geoSingle->merge(*geo->asGEO_Detail());
            carve<GFE_CarveType::Both>();
            geo->replaceWith(*geoSingle);
        }
    }
}



    
template<GFE_CarveType CARVE_TYPE>
void carve()
{
    GA_PrimitiveGroupUPtr delPrimGroupUPtr   = geoSingle->createDetachedPrimitiveGroup();
    //GA_PointGroupUPtr     delPointGroupUPtr  = geoSingle->createDetachedPointGroup();
    GA_VertexGroupUPtr    delVertexGroupUPtr = geoSingle->createDetachedVertexGroup();
    delPrimGroup   = delPrimGroupUPtr.get();
    //delPointGroup  = delPointGroupUPtr.get();
    delVertexGroup = delVertexGroupUPtr.get();
    
    carveStartPrimGroup = groupParser_carveStart.getPrimitiveGroup();
    carveEndPrimGroup   = groupParser_carveEnd.getPrimitiveGroup();

    UTparallelFor(groupParser.getPrimitiveSplittableRange(), [this](const GA_SplittableRange& r)
    {
        fpreal domainUStart = 0, domainUEnd = 0;
        GA_Offset primvtx0 = 0, primpt0 = 0, primvtx1 = 0, primpt1 = 0;
        GA_Offset start, end;
        for (GA_Iterator it(r); it.blockAdvance(start, end); )
        {
            for (GA_Offset primoff = start; primoff < end; ++primoff)
            {
                if constexpr(CARVE_TYPE == GFE_CarveType::Start)
                {
                    if (carveStartPrimGroup && !carveStartPrimGroup->contains(primoff))
                    {
                        delPrimGroup->setElement(primoff, true);
                        continue;
                    }
                }
                if constexpr(CARVE_TYPE == GFE_CarveType::End)
                {
                    if (carveEndPrimGroup && !carveEndPrimGroup->contains(primoff))
                    {
                        delPrimGroup->setElement(primoff, true);
                        continue;
                    }
                }
                const GA_OffsetListRef& vertices = geoSingle->getPrimitiveVertexList(primoff);
                const GA_Size numvtx = vertices.size();
                
                if constexpr(CARVE_TYPE != GFE_CarveType::End)
                {
                    primvtx0 = vertices[0];
                    primpt0  = geoSingle->vertexPoint(primvtx0);
                }
                if constexpr(CARVE_TYPE != GFE_CarveType::Start)
                {
                    primvtx1 = vertices[numvtx-1];
                    primpt1  = geoSingle->vertexPoint(primvtx1);
                }
                if constexpr(CARVE_TYPE != GFE_CarveType::End)
                {
                    if (carveUAttribStart)
                    {
                        switch (carveUAttribStart->getOwner())
                        {
                        case GA_ATTRIB_PRIMITIVE: domainUStart = startCarveU_h.get(primoff);  break;
                        case GA_ATTRIB_POINT:     domainUStart = startCarveU_h.get(primpt0);  break;
                        case GA_ATTRIB_VERTEX:    domainUStart = startCarveU_h.get(primvtx0); break;
                        default: UT_ASSERT_MSG(0, "unhandled carveU Attrib class");           break;
                        }
                    }
                    else
                    {
                        domainUStart = carveUStart;
                    }

                    if constexpr(CARVE_TYPE == GFE_CarveType::Both)
                    {
                        fpreal uvMax;
                        if (carveSpace_isWorld)
                        {
                            UT_ASSERT_P(uvAttrib);
                            switch (uvAttrib->getOwner())
                            {
                            case GA_ATTRIB_POINT:  uvMax = uv_h.get(primpt1);  break;
                            case GA_ATTRIB_VERTEX: uvMax = uv_h.get(primvtx1); break;
                            default: UT_ASSERT_MSG(0, "unhandled uv class");   break;
                            }
                        }
                        else
                        {
                            uvMax = 1;
                        }
                        if (domainUStart >= uvMax && (!carveStartPrimGroup || carveStartPrimGroup->contains(primoff)))
                        {
                            delPrimGroup->setElement(primoff, true);
                            continue;
                        }
                    }
                    else if (domainUStart <= 0)
                    {
                        delPrimGroup->setElement(primoff, true);
                        continue;
                    }
                }

                if constexpr(CARVE_TYPE != GFE_CarveType::Start)
                {
                    if (endCarveUAttrib)
                    {
                        switch (endCarveUAttrib->getOwner())
                        {
                        case GA_ATTRIB_PRIMITIVE: domainUEnd = endCarveU_h.get(primoff);  break;
                        case GA_ATTRIB_POINT:     domainUEnd = endCarveU_h.get(primpt0);  break;
                        case GA_ATTRIB_VERTEX:    domainUEnd = endCarveU_h.get(primvtx0); break;
                        default: break;
                        }
                    }
                    else
                    {
                        domainUEnd = carveUEnd;
                    }
                    
                    // cout(domainUEnd);
                    if (!absCarveUEnd)
                    {
                        if constexpr(CARVE_TYPE != GFE_CarveType::Both)
                        {
                            // if ( domainUEnd <= domainUStart ) {
                            if (domainUEnd <= 0)
                            {
                                delPrimGroup->setElement(primoff, true);
                                continue;
                            }
                        }
                        if (carveSpace_isWorld)
                        {
                            fpreal uvMax;
                            UT_ASSERT_P(uvAttrib);
                            switch (uvAttrib->getOwner())
                            {
                            case GA_ATTRIB_POINT:  uvMax = uv_h.get(primpt1);  break;
                            case GA_ATTRIB_VERTEX: uvMax = uv_h.get(primvtx1); break;
                            default: UT_ASSERT_MSG(0, "unhandled uv class");    break;
                            }
                            domainUEnd = uvMax - domainUEnd;
                        }
                        else
                        {
                            domainUEnd = 1.0 - domainUEnd;
                        }
                    }
                    if constexpr(CARVE_TYPE == GFE_CarveType::Both)
                    {
                        if (domainUEnd <= 0 && (!carveEndPrimGroup && carveEndPrimGroup->contains(primoff)) )
                        {
                            delPrimGroup->setElement(primoff, true);
                            continue;
                        }
                    }
                }
                
                if constexpr(CARVE_TYPE == GFE_CarveType::Both)
                {
                    if (domainUEnd < domainUStart)
                    {
                        if (delReversedInsidePrim)
                        {
                            delPrimGroup->setElement(primoff, true);
                            continue;
                        }
                        else
                        {
                            const fpreal tempFlt = domainUEnd;
                            domainUEnd = domainUStart;
                            domainUStart = tempFlt;
                        }
                    }
                }

                if constexpr(CARVE_TYPE != GFE_CarveType::End)
                    carveSingle<CARVE_TYPE, 0>(vertices, primoff, numvtx, domainUStart);
                
                if constexpr(CARVE_TYPE != GFE_CarveType::Start)
                    carveSingle<CARVE_TYPE, 1>(vertices, primoff, numvtx, domainUEnd);
            }
        }
    }, subscribeRatio, minGrainSize);
                    
    delPrimGroup->invalidateGroupEntries();
    //delPointGroup->invalidateGroupEntries();
    delVertexGroup->invalidateGroupEntries();
    
    geoSingle->destroyPrimitiveOffsets(geoSingle->getPrimitiveRange(delPrimGroup), true);
    //geoSingle->destroyPointOffsets(geoSingle->getPointRange(delPointGroup));
                    
    GFE_DelVertex delVertex(geoSingle, cookparms);
    delVertex.groupParser.setGroup(delVertexGroup);
    delVertex.compute();
    
    if constexpr(CARVE_TYPE == GFE_CarveType::Start)
        if (reverseOutsideStart)
            geoSingle->reversePolys();
    
    if constexpr(CARVE_TYPE == GFE_CarveType::End)
        if (reverseOutsideEnd)
            geoSingle->reversePolys();
}


template<GFE_CarveType CARVE_TYPE, bool isEnd>
void carveSingle(
    const GA_OffsetListRef& vertices,
    const GA_Offset primoff,
    const GA_Size numvtx,
    const fpreal domainU
)
{
    if constexpr(!isEnd && CARVE_TYPE != GFE_CarveType::End)
    {
        if (carveStartPrimGroup && carveStartPrimGroup->contains(primoff))
            return;
    }
    else if constexpr(isEnd && CARVE_TYPE != GFE_CarveType::Start)
    {
        if (carveEndPrimGroup && carveEndPrimGroup->contains(primoff))
            return;
    }
        
    fpreal uv_rest = 0;
    fpreal uv, step;
    if (carveSpace == GFE_CarveSpace::LocalAverage)
        uv = isEnd;
        step = 1.0 / (numvtx-1);
    
    if constexpr(isEnd)
    {
        for (GA_Size vtxpnum = numvtx-1; ; --vtxpnum)
        {
            //flag = constexpr(isEnd) ? vtxpnum >= 0 : vtxpnum < numvtx;
            GA_Offset vtxoff = vertices[vtxpnum];
            GA_Offset ptoff  = geoSingle->vertexPoint(vtxoff);

            if (carveSpace != GFE_CarveSpace::LocalAverage)
                uvAIF->get(uvAttrib, isPointUV ? ptoff : vtxoff, uv);

            if (uv <= domainU)
            {
                const bool arriveBound = uv == domainU;
                //const GA_Size breakVtxpnum = vtxpnum + (uv != domainUEnd);
                //const GA_Size breakVtxpnum = vtxpnum + 1;
                if constexpr(CARVE_TYPE == GFE_CarveType::Both)
                {
                    for (GA_Size vtxpnum_prev = vtxpnum + (arriveBound ? 1 : 2); vtxpnum_prev < numvtx; ++vtxpnum_prev)
                        delVertexGroup->setElement(vertices[vtxpnum_prev], true);
                }
                else
                {
                    if (arriveBound)
                    {
                        for (GA_Size vtxpnum_prev = 0; vtxpnum_prev <= vtxpnum; ++vtxpnum_prev)
                            delVertexGroup->setElement(vertices[vtxpnum_prev], true);
                    }
                    else
                    {
                        for (GA_Size vtxpnum_prev = 0; vtxpnum_prev < vtxpnum; ++vtxpnum_prev)
                            delVertexGroup->setElement(vertices[vtxpnum_prev], true);
                    }
                }

                if (vtxpnum == numvtx-1 && !arriveBound)
                    break;
                
                if constexpr(CARVE_TYPE == GFE_CarveType::Both)
                {
                    if (hasCarveEndPrimGroup)
                        hasCarveEndPrimGroup->setElement(primoff, true);
                }
                else
                {
                    if (hasCarveStartPrimGroup)
                        hasCarveStartPrimGroup->setElement(primoff, true);
                }
                
                if (vtxpnum == numvtx-1)
                    break;

                const GA_Offset ptoff0 = geoSingle->vertexPoint(vertices[vtxpnum]);
                const GA_Offset ptoff1 = geoSingle->vertexPoint(vertices[vtxpnum+1]);
                if constexpr(CARVE_TYPE == GFE_CarveType::Both)
                    interpAttribs(ptoff1, ptoff0,       (domainU - uv_rest) / (uv - uv_rest));
                else
                    interpAttribs(ptoff0, ptoff1, 1.0 - (domainU - uv_rest) / (uv - uv_rest));
                break;
            }

            uv_rest = uv;
            if (carveSpace == GFE_CarveSpace::LocalAverage)
                uv -= step;
            
            if (vtxpnum <= 0)
                break;
        }
    }
    else
    {
        for (GA_Size vtxpnum = 0; vtxpnum < numvtx; ++vtxpnum)
        {
            //flag = constexpr(isEnd) ? vtxpnum >= 0 : vtxpnum < numvtx;
            GA_Offset vtxoff = vertices[vtxpnum];
            GA_Offset ptoff  = geoSingle->vertexPoint(vtxoff);

            if (carveSpace != GFE_CarveSpace::LocalAverage)
                uvAIF->get(uvAttrib, isPointUV ? ptoff : vtxoff, uv);

            if (uv >= domainU)
            {
                const bool arriveBound = uv == domainU;
                //const GA_Size breakVtxpnum = vtxpnum - (uv != domainUStart);
                if constexpr(CARVE_TYPE == GFE_CarveType::Both)
                {
                    if (arriveBound)
                    {
                        for (GA_Size vtxpnum_prev = 0; vtxpnum_prev < vtxpnum; ++vtxpnum_prev)
                            delVertexGroup->setElement(vertices[vtxpnum_prev], true);
                    }
                    else
                    {
                        for (GA_Size vtxpnum_prev = 0; vtxpnum_prev < vtxpnum-1; ++vtxpnum_prev)
                            delVertexGroup->setElement(vertices[vtxpnum_prev], true);
                    }
                }
                else
                {
                    for (GA_Size vtxpnum_prev = vtxpnum+1; vtxpnum_prev < numvtx; ++vtxpnum_prev)
                        delVertexGroup->setElement(vertices[vtxpnum_prev], true);
                }

                //if (vtxpnum == 0 && uv == domainUStart)
                //    break;
                if (!arriveBound && vtxpnum == 0)
                    break;
                
                if constexpr(CARVE_TYPE == GFE_CarveType::Both)
                {
                    if (hasCarveStartPrimGroup)
                        hasCarveStartPrimGroup->setElement(primoff, true);
                }
                else
                {
                    if (hasCarveEndPrimGroup)
                        hasCarveEndPrimGroup->setElement(primoff, true);
                }
                
                if (vtxpnum == 0)
                    break;
                
                const GA_Offset ptoff0 = geoSingle->vertexPoint(vertices[vtxpnum]);
                const GA_Offset ptoff1 = geoSingle->vertexPoint(vertices[vtxpnum-1]);
                if constexpr(CARVE_TYPE == GFE_CarveType::Both)
                    interpAttribs(ptoff1, ptoff0,       (domainU - uv_rest) / (uv - uv_rest));
                else
                    interpAttribs(ptoff0, ptoff1, 1.0 - (domainU - uv_rest) / (uv - uv_rest));
                break;
            }
            uv_rest = uv;
            if (carveSpace == GFE_CarveSpace::LocalAverage)
                uv += step;
        }
    }
}


private:
            
    template<typename T>
    void
        setAttribVal(
            const GA_RWHandleT<T>& attrib_h,
            const GA_ROHandleT<T>& attribSrc_h,
            const GA_Offset ptoff0,
            const GA_Offset ptoff1,
            const T bias
        )
    {
        T attrib0 = attribSrc_h.get(ptoff0);
        const T attrib1 = attribSrc_h.get(ptoff1);
        //attrib0 = GFE_Math::vlerp(attrib0, attrib1, bias);
        attrib0 = attrib0 + (attrib1 - attrib0) * bias;
        attrib_h.set(ptoff0, attrib0);
    }
    
    template<typename VECTOR_T>
    void
        setAttribVector(
            const GA_RWHandleT<VECTOR_T>& attrib_h,
            const GA_ROHandleT<VECTOR_T>& attribSrc_h,
            const GA_Offset ptoff0,
            const GA_Offset ptoff1,
            const typename VECTOR_T::value_type bias
        )
    {
        VECTOR_T attrib0 = attribSrc_h.get(ptoff0);
        const VECTOR_T& attrib1 = attribSrc_h.get(ptoff1);
        //attrib0 = GFE_Math::vlerp(attrib0, attrib1, bias);
        attrib0 = attrib0 + (attrib1 - attrib0) * bias;
        attrib_h.set(ptoff0, attrib0);
    }
    
    //template<typename T>
    //void
    //    setAttribValInt(
    //        const GA_RWHandleT<T>& attrib_h,
    //        const GA_Offset ptoff0,
    //        const GA_Offset ptoff1,
    //        const T bias
    //    )
    //{
    //    T attrib0 = attrib_h.get(ptoff0);
    //    const T attrib1 = attrib_h.get(ptoff1);
    //    attrib0 = attrib0 + (attrib1 - attrib0) * bias;
    //    attrib_h.set(ptoff0, attrib0);
    //}
    
    template<typename FLOAT_T>
    void
        setAttribDir(
            const GA_RWHandleT<UT_Vector3T<FLOAT_T>>& attrib_h,
            const GA_ROHandleT<UT_Vector3T<FLOAT_T>>& attribSrc_h,
            const GA_Offset ptoff0,
            const GA_Offset ptoff1,
            const fpreal bias
        )
    {
        UT_Vector3T<FLOAT_T> attrib0 = attribSrc_h.get(ptoff0);
        UT_Vector3T<FLOAT_T> attrib1 = attribSrc_h.get(ptoff1);
        //aifTuple.set();
        FLOAT_T length0 = length2(attrib0);
        const FLOAT_T length1 = length2(attrib1);
        length0 = sqrt(SYSlerp(length0, length1, bias));

        attrib0.normalize();
        attrib1.normalize();

        UT_QuaternionT<fpreal> q;
        q.updateFromVectors(attrib0, attrib1);
        q = UT_QuaternionT<fpreal>(0, 0, 0, 1).interpolate(q, bias);

        attrib0 = q.rotate(attrib0);
        attrib0 *= length0;

        attrib_h.set(ptoff0, attrib0);
    }


    void interpAttribs(const GA_Offset ptoff0, const GA_Offset ptoff1, const fpreal bias)
    {
        const size_t size = interpAttribArray.size();
        for (size_t i = 0; i < size; i++)
        {
            const GA_Attribute* const attribSrc = interpAttribArray[i];
            
            UT_ASSERT_MSG(!attribSrc->isDetached(), "unsupport detached attrib");
            if (attribSrc->isDetached())
                continue;
            
            GA_Attribute* const attrib = geoSingle->findAttribute(attribSrc->getOwner(), attribSrc->getName());
            UT_ASSERT_P(attrib);
            const GA_AIFTuple* const aifTuple = attrib->getAIFTuple();
            
            if (aifTuple)
            {
                const int tupleSize = aifTuple->getTupleSize(attrib);
                const GA_Storage storage = aifTuple->getStorage(attrib);
                switch (tupleSize)
                {
                case 1:
                    switch (storage)
                    {
                        //case GA_STORE_BOOL: setAttribVal<bool    >(attrib, attribSrc, ptoff0, ptoff1, bias); break;
                        case GA_STORE_UINT8:  setAttribVal<uint8   >(attrib, attribSrc, ptoff0, ptoff1, bias); break;
                        case GA_STORE_INT8:   setAttribVal<int8    >(attrib, attribSrc, ptoff0, ptoff1, bias); break;
                        case GA_STORE_INT16:  setAttribVal<int16   >(attrib, attribSrc, ptoff0, ptoff1, bias); break;
                        case GA_STORE_INT32:  setAttribVal<int32   >(attrib, attribSrc, ptoff0, ptoff1, bias); break;
                        case GA_STORE_INT64:  setAttribVal<int64   >(attrib, attribSrc, ptoff0, ptoff1, bias); break;
                        case GA_STORE_REAL16: setAttribVal<fpreal16>(attrib, attribSrc, ptoff0, ptoff1, bias); break;
                        case GA_STORE_REAL32: setAttribVal<fpreal32>(attrib, attribSrc, ptoff0, ptoff1, bias); break;
                        case GA_STORE_REAL64: setAttribVal<fpreal64>(attrib, attribSrc, ptoff0, ptoff1, bias); break;
                        case GA_STORE_STRING:  break;
                        case GA_STORE_DICT:    break;
                        case GA_STORE_INVALID: break;
                        default: break;
                    }
                    break;
                case 2:
                    switch (storage)
                    {
                        //case GA_STORE_BOOL:  setAttribVector<UT_Vector2T<bool    >>(attrib, attribSrc, ptoff0, ptoff1, bias); break;
                        //case GA_STORE_UINT8: setAttribVector<UT_Vector2T<uint8   >>(attrib, attribSrc, ptoff0, ptoff1, bias); break;
                        //case GA_STORE_INT8:  setAttribVector<UT_Vector2T<int8    >>(attrib, attribSrc, ptoff0, ptoff1, bias); break;
                        //case GA_STORE_INT16: setAttribVector<UT_Vector2T<int16   >>(attrib, attribSrc, ptoff0, ptoff1, bias); break;
                        case GA_STORE_INT32:   setAttribVector<UT_Vector2T<int32   >>(attrib, attribSrc, ptoff0, ptoff1, bias); break;
                        case GA_STORE_INT64:   setAttribVector<UT_Vector2T<int64   >>(attrib, attribSrc, ptoff0, ptoff1, bias); break;
                        case GA_STORE_REAL16:  setAttribVector<UT_Vector2T<fpreal16>>(attrib, attribSrc, ptoff0, ptoff1, bias); break;
                        case GA_STORE_REAL32:  setAttribVector<UT_Vector2T<fpreal32>>(attrib, attribSrc, ptoff0, ptoff1, bias); break;
                        case GA_STORE_REAL64:  setAttribVector<UT_Vector2T<fpreal64>>(attrib, attribSrc, ptoff0, ptoff1, bias); break;
                        case GA_STORE_STRING:  break;
                        case GA_STORE_DICT:    break;
                        case GA_STORE_INVALID: break;
                        default: break;
                    }
                    break;
                case 3:
                    if (attrib->getTypeInfo() == GA_TYPE_VECTOR || attrib->getTypeInfo() == GA_TYPE_NORMAL)
                    {
                        switch (storage)
                        {
                        case GA_STORE_BOOL:     break;
                        case GA_STORE_UINT8:    break;
                        case GA_STORE_INT8:     break;
                        case GA_STORE_INT16:    break;
                        case GA_STORE_INT32:    break;
                        case GA_STORE_INT64:    break;
                        case GA_STORE_REAL16:  setAttribDir<fpreal16>(attrib, attribSrc, ptoff0, ptoff1, bias); break;
                        case GA_STORE_REAL32:  setAttribDir<fpreal32>(attrib, attribSrc, ptoff0, ptoff1, bias); break;
                        case GA_STORE_REAL64:  setAttribDir<fpreal64>(attrib, attribSrc, ptoff0, ptoff1, bias); break;
                        case GA_STORE_STRING:   break;
                        case GA_STORE_DICT:     break;
                        case GA_STORE_INVALID:  break;
                        default:                break;
                        }
                        continue;
                    }
                    else
                    {
                        switch (storage)
                        {
                            //case GA_STORE_BOOL:  setAttribVector<UT_Vector3T<bool    >>(attrib, attribSrc, ptoff0, ptoff1, bias); break;
                            //case GA_STORE_UINT8: setAttribVector<UT_Vector3T<uint8   >>(attrib, attribSrc, ptoff0, ptoff1, bias); break;
                            //case GA_STORE_INT8:  setAttribVector<UT_Vector3T<int8    >>(attrib, attribSrc, ptoff0, ptoff1, bias); break;
                            //case GA_STORE_INT16: setAttribVector<UT_Vector3T<int16   >>(attrib, attribSrc, ptoff0, ptoff1, bias); break;
                            case GA_STORE_INT32:   setAttribVector<UT_Vector3T<int32   >>(attrib, attribSrc, ptoff0, ptoff1, bias); break;
                            case GA_STORE_INT64:   setAttribVector<UT_Vector3T<int64   >>(attrib, attribSrc, ptoff0, ptoff1, bias); break;
                            case GA_STORE_REAL16:  setAttribVector<UT_Vector3T<fpreal16>>(attrib, attribSrc, ptoff0, ptoff1, bias); break;
                            case GA_STORE_REAL32:  setAttribVector<UT_Vector3T<fpreal32>>(attrib, attribSrc, ptoff0, ptoff1, bias); break;
                            case GA_STORE_REAL64:  setAttribVector<UT_Vector3T<fpreal64>>(attrib, attribSrc, ptoff0, ptoff1, bias); break;
                            case GA_STORE_STRING:  break;
                            case GA_STORE_DICT:    break;
                            case GA_STORE_INVALID: break;
                            default: break;
                        }
                        
                    }
                    break;
                    
                case 4:
                    switch (storage)
                    {
                        //case GA_STORE_BOOL:  setAttribVector<UT_Vector4T<bool    >>(attrib, attribSrc, ptoff0, ptoff1, bias); break;
                        //case GA_STORE_UINT8: setAttribVector<UT_Vector4T<uint8   >>(attrib, attribSrc, ptoff0, ptoff1, bias); break;
                        //case GA_STORE_INT8:  setAttribVector<UT_Vector4T<int8    >>(attrib, attribSrc, ptoff0, ptoff1, bias); break;
                        //case GA_STORE_INT16: setAttribVector<UT_Vector4T<int16   >>(attrib, attribSrc, ptoff0, ptoff1, bias); break;
                        case GA_STORE_INT32:   setAttribVector<UT_Vector4T<int32   >>(attrib, attribSrc, ptoff0, ptoff1, bias); break;
                        case GA_STORE_INT64:   setAttribVector<UT_Vector4T<int64   >>(attrib, attribSrc, ptoff0, ptoff1, bias); break;
                        case GA_STORE_REAL16:  setAttribVector<UT_Vector4T<fpreal16>>(attrib, attribSrc, ptoff0, ptoff1, bias); break;
                        case GA_STORE_REAL32:  setAttribVector<UT_Vector4T<fpreal32>>(attrib, attribSrc, ptoff0, ptoff1, bias); break;
                        case GA_STORE_REAL64:  setAttribVector<UT_Vector4T<fpreal64>>(attrib, attribSrc, ptoff0, ptoff1, bias); break;
                        case GA_STORE_STRING:  break;
                        case GA_STORE_DICT:    break;
                        case GA_STORE_INVALID: break;
                        default: break;
                    }
                    break;
                }
            }
            else if (attrib->getAIFNumericArray())
            {
            }
            else
            {
            }
        }
    }



public:
    GFE_GroupParser groupParser_carveStart;
    GFE_GroupParser groupParser_carveEnd;
    
    
    GFE_Enumerate enumerate;
    GFE_RefAttributeArray interpAttribArray;

    GFE_CarveSpace carveSpace = GFE_CarveSpace::WorldArcLength;
    bool keepOutsideStart = false;
    bool keepOutsideEnd   = false;
    bool keepInside       = true;
    
    bool reverseOutsideStart = false;
    bool reverseOutsideEnd   = false;
    
    fpreal carveUStart = 0.25;
    fpreal carveUEnd   = 0.25;

    bool absCarveUEnd = false;
    bool delReversedInsidePrim = false;

    bool delCarveUAttribStart = false;
    bool delCarveUEndAttrib   = false;

    GA_PrimitiveGroup* hasCarveStartPrimGroup = nullptr;
    GA_PrimitiveGroup* hasCarveEndPrimGroup   = nullptr;


private:

    exint subscribeRatio = 64;
    exint minGrainSize = 1024;
    
private:// temp parm

    const GA_PrimitiveGroup* carveStartPrimGroup;
    const GA_PrimitiveGroup* carveEndPrimGroup;
    
    const GA_Attribute* uvAttrib = nullptr;
    //GA_Attribute* uvAttrib_nonConst = nullptr;
    GA_ROHandleT<fpreal> uv_h;
    const GA_AIFTuple* uvAIF;
    bool isPointUV;
    //int uvTupleSize;
        
    const GA_Attribute* carveUAttribStart = nullptr;
    const GA_Attribute* endCarveUAttrib = nullptr;
    GA_ROHandleT<fpreal> startCarveU_h;
    GA_ROHandleT<fpreal> endCarveU_h;
        
        
    GA_Attribute* carveUAttribStart_nonConst = nullptr;
    GA_Attribute* carveUEndAttrib_nonConst = nullptr;
        
    bool carveSpace_isWorld;
    GA_PrimitiveGroup* delPrimGroup;
    //GA_PointGroup*     delPointGroup;
    GA_VertexGroup*    delVertexGroup;

    //GU_Detail* geoOrigin;
    GU_Detail* geoMerge;
    GU_Detail* geoSingle;
    //GU_Detail* geoOriginGU;



}; // End of class GFE_Carve

#endif
