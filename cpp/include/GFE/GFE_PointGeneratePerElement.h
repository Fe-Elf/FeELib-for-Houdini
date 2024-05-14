
#pragma once

#ifndef __GFE_PointGeneratePerElement_h__
#define __GFE_PointGeneratePerElement_h__

#include "GFE_ExtractPoint.h"
#include "GFE/GFE_PointGeneratePerElement.h"

#include "GFE/GFE_GeoFilter.h"


#include "GFE/GFE_AttributeCopy.h"
#include "GFE/GFE_Math.h"
#include "GFE/GFE_MeshTopology.h"

//#include "SOP/SOP_Scatter-2.0.proto.h"


class GFE_PointGenPerElem : public GFE_AttribFilter {

#define __TEMP_GFE_PointGenPerElem_GroupName "__TEMP_GFE_PointGenPerElem_GROUP"

public:
    using GFE_AttribFilter::GFE_AttribFilter;


    void
        setComputeParm(
            const GA_GroupType elemType = GA_GROUP_PRIMITIVE,
            const bool setPositionOnElem = true,
            const char* const keepSrcElemAttribName = nullptr,
            const char* const keepSrcElemGroupName  = nullptr,
            const exint subscribeRatio = 64,
            const exint minGrainSize = 1024
        )
    {
        setHasComputed();
        this->elemType           = elemType;
        this->setPositionOnElem  = setPositionOnElem;
        this->keepSrcElemAttribName = keepSrcElemAttribName;
        this->keepSrcElemGroupName  = keepSrcElemGroupName;
        this->subscribeRatio     = subscribeRatio;
        this->minGrainSize       = minGrainSize;
    }

    SYS_FORCE_INLINE void setNumPointAttrib(const GA_Attribute* const inAttrib = nullptr)
    { numPointAttrib = inAttrib; }

    const GA_Attribute* setNumPointAttrib(const UT_StringRef& numPointAttribName)
    {
        numPointAttrib = (geoSrc ? geoSrc : geo)->findAttribute(GFE_Type::attributeOwner_groupType(elemType), numPointAttribName);
        if (numPointAttrib && !(numPointAttrib->getAIFTuple() &&
                                numPointAttrib->getTupleSize() == 1 &&
                                numPointAttrib->getStorageClass() == GA_STORECLASS_INT))
            numPointAttrib = nullptr;
        return numPointAttrib;
    }

    SYS_FORCE_INLINE GA_Attribute* getSrcElemoffAttrib() const
    { return srcElemoffAttrib; }

    //SYS_FORCE_INLINE GA_Attribute* setSrcElemoffAttrib(const GA_Attribute* const inAttrib = nullptr)
    //{ srcElemoffAttrib = inAttrib; }

    //const char* setSrcElemoffAttrib(const char* const inString)
    //{ return srcElemoffAttribName = inString; }
    
    
    
private:

    virtual bool
        computeCore() override
    {
        if (elemType == GA_GROUP_POINT)
        {
            if (groupParser.isEmpty())
                return true;
            pointGenPerPoint();
            return true;
        }
        
        elemOwner = GFE_Type::attributeOwner_groupType(elemType);

        UT_ASSERT_MSG(!numPointAttrib || numPointAttrib->getOwner() == elemOwner, "not correct numPointAttrib owner");
        UT_ASSERT_MSG(!numPointAttrib ||
                      (numPointAttrib->getAIFTuple() &&
                       numPointAttrib->getTupleSize() == 1 &&
                       numPointAttrib->getStorageClass() == GA_STORECLASS_INT),
              "not correct numPointAttrib type");
        
        if (groupParser.isEmpty())
            return true;
        
        if (keepSrcElemAttribName && std::strlen(keepSrcElemAttribName)==0)
            keepSrcElemAttribName = nullptr;
        
        if (keepSrcElemGroupName  && std::strlen(keepSrcElemGroupName)==0 )
            keepSrcElemGroupName  = nullptr;
        
        keepSrcElemAttrib = keepSrcElemAttribName || keepSrcElemGroupName;
        
        const bool isElement = GFE_Type::isElementGroup(elemType);
        
        GU_DetailHandle geoTmp_h;
        geoTmp = new GU_Detail();
        geoTmp_h.allocateAndSet(geoTmp);
        geoTmp->replaceWith(geoSrc ? *geoSrc : *geo);

        isGeoGroupFull = groupParser.isFull();
        if (isElement)
        {
            geoElemGroup = groupParser.isDetached(elemType) ? groupParser.getElementGroup(elemType) : geoTmp->getElementGroupTable(elemOwner).find(groupParser.getName());
            geoSplittableRange = GA_Range(geoTmp->getIndexMap(elemOwner), geoElemGroup);
        }
        
        GA_AttributeUPtr numPointAttribMidUPtr;
        if (isElement && numPointAttrib)
        {
            if (numPointAttrib->isDetached())
            {
                numPointAttribMidUPtr = GFE_Attribute::clone(*geoTmp, *numPointAttrib);
                numPointAttribMid = numPointAttribMidUPtr.get();
            }
            else
            {
                numPointAttribMid = geoTmp->findAttribute(elemOwner, numPointAttrib->getName());
            }
            GFE_Attribute::accumulateT<exint>(*numPointAttribMid, geoSplittableRange);
        }
        else if (isElement && !isGeoGroupFull)
        {
            //GA_SplittableRange srange = groupParser.getSplittableRange(elemOwner);
            numPointAttribMidUPtr = GFE_Attribute::createDetachedIndexAttrib(*geoTmp, elemOwner, &geoSplittableRange, 1);
            numPointAttribMid = numPointAttribMidUPtr.get();
        }
        else
        {
            numPointAttribMid = nullptr;
        }
        
        GFE_MeshTopology meshTopology(geoTmp, cookparms);
        GA_Size numpt;
        if (isElement)
        {
            if (numPointAttribMid)
            {
                GA_Offset start, end;
                for (GA_Iterator it(geoSplittableRange.begin()); it.fullBlockAdvance(start, end); );
                //{
                //    int a = 0;
                //    a = a;
                //}
                numPointAttribMid->getAIFTuple()->get(numPointAttribMid, end-1, numpt);
            }
            else
            {
                UT_ASSERT_P(isGeoGroupFull);
                numpt = geoTmp->getIndexMap(elemOwner).indexSize();
                // numpt = geoElemGroup ? geoElemGroup->entries() : geoTmp->getIndexMap(elemOwner).indexSize();
            }
        }
        else
        {
            meshTopology.outIntermediateAttrib = false;
            meshTopology.groupParser.setGroup(groupParser);
            creatingGroup = meshTopology.setVertexNextEquivNoLoopGroup(true);
            dstptAttrib = meshTopology.setVertexPointDst(true);
            meshTopology.compute();
        
            numpt = creatingGroup->entries();
            geoSplittableRange = GA_Range(*creatingGroup);
            ///const GA_SplittableRange srange = GA_Range(*creatingGroup);
            numPointAttribMidUPtr = GFE_Attribute::createDetachedIndexAttrib(*geoTmp, GA_ATTRIB_VERTEX, &geoSplittableRange, 0);
            numPointAttribMid = numPointAttribMidUPtr.get();
        }

        geo->clear();
        geo->appendPointBlock(numpt);

        
        if (srcElemoffAttribName || keepSrcElemAttrib)
            srcElemoffAttrib = getOutAttribArray().findOrCreateOffset(false, GA_ATTRIB_POINT, GA_STORE_INVALID, srcElemoffAttribName);
    
        if (!srcElemoffAttrib && !setPositionOnElem)
            return true;
    
        numPoint_h   = numPointAttribMid;
        srcElemoff_h = srcElemoffAttrib;
        pos_h        = geo->getP();
        posRef_h     = geoTmp->getP();

        switch (elemType)
        {
        case GA_GROUP_PRIMITIVE: pointGenPerPrim();   break;
        case GA_GROUP_EDGE:      pointGenPerEdge();   break;
        default: UT_ASSERT_MSG(0, "Unhandled Elem Type"); return false;
        }
    
        return true;
    }

    void pointGenPerPrim()
    {
        UTparallelFor(geoSplittableRange, [this](const GA_SplittableRange& r)
        {
            bool unCorrectVtxpnum;
            UT_Vector3T<fpreal> pos, pos0, pos1, pos2;
            GA_Offset ptoffEnd_prev = GFE_INVALID_OFFSET;
            
            GA_Offset start, end;
            for (GA_Iterator it(r.begin()); it.blockAdvance(start, end); )
            {
                if (numPointAttribMid && GFE_Type::isInvalidOffset(ptoffEnd_prev))
                {
                    GA_Offset elemoff_prev = GFE_INVALID_OFFSET;
                    GA_Offset start1, end1;
                    for (GA_Iterator it1(geoSplittableRange.begin()); it1.blockAdvance(start1, end1); )
                    {
                        if (start1 == start)
                            break;
                        elemoff_prev = end1;
                    }
                    ptoffEnd_prev = GFE_Type::isInvalidOffset(elemoff_prev) ? 0 : numPoint_h.get(elemoff_prev);
                }
            
                for (GA_Offset elemoff = start; elemoff < end; ++elemoff)
                {
                    if (setPositionOnElem)
                    {
                        const GA_Size numvtx = geoTmp->getPrimitiveVertexCount(elemoff);
                        if (numvtx > 3)
                        {
                            const GA_Size numvtx2 = numvtx-2;
                            for (GA_Size vtxpnum = 0; vtxpnum < numvtx; ++vtxpnum)
                            {
                                unCorrectVtxpnum = false;
                                const bool isEndVtx = vtxpnum >= numvtx2;
                                // const GA_Offset vtxoff = geoTmp->getPrimitiveVertexOffset(elemoff, vtxpnum);
                                // const GA_Offset ptoff  = geoTmp->vertexPoint(vtxoff);
                                pos0 = posRef_h.get(getPrimitivePointOffset(geoTmp, elemoff, vtxpnum));
                                pos1 = posRef_h.get(getPrimitivePointOffset(geoTmp, elemoff, isEndVtx ? (vtxpnum == numvtx2 ? numvtx-1 : 0) : vtxpnum + 1));
                                pos2 = posRef_h.get(getPrimitivePointOffset(geoTmp, elemoff, isEndVtx ? (vtxpnum == numvtx2 ? 0 : 1)        : vtxpnum + 2));
                                
                                for (GA_Size vtxpnum1 = isEndVtx ? (vtxpnum >= numvtx2 ? 2 : 1) : 0; vtxpnum1 < vtxpnum; ++vtxpnum1)
                                {
                                    if (GFE_Math::pointInTriangleT<fpreal>(posRef_h.get(getPrimitivePointOffset(geoTmp, elemoff, vtxpnum1)), pos0, pos1, pos2))
                                    {
                                        unCorrectVtxpnum = true;
                                        break;
                                    }
                                }
                                if (unCorrectVtxpnum)
                                    continue;
                                for (GA_Size vtxpnum1 = vtxpnum+3; vtxpnum1 < numvtx; ++vtxpnum1)
                                {
                                    if (GFE_Math::pointInTriangleT<fpreal>(posRef_h.get(getPrimitivePointOffset(geoTmp, elemoff, vtxpnum1)), pos0, pos1, pos2))
                                    {
                                        unCorrectVtxpnum = true;
                                        break;
                                    }
                                }
                                if (unCorrectVtxpnum)
                                    continue;
                                pos = (pos0 + pos1 + pos2) / 3.0;
                                break;
                            }
                            UT_ASSERT_MSG(!unCorrectVtxpnum, "not possible");
                        }
                        else
                        {
                            switch (numvtx)
                            {
                            default:
                            case 0: pos  = 0;                                                         break;
                            case 1: pos  = posRef_h.get(getPrimitivePointOffset(geoTmp, elemoff, 0)); break;
                            case 2: pos0 = posRef_h.get(getPrimitivePointOffset(geoTmp, elemoff, 0));
                                    pos1 = posRef_h.get(getPrimitivePointOffset(geoTmp, elemoff, 1));
                                    pos  = (pos0 + pos1) / 2.0;
                            break;
                            case 3: pos0 = posRef_h.get(getPrimitivePointOffset(geoTmp, elemoff, 0));
                                    pos1 = posRef_h.get(getPrimitivePointOffset(geoTmp, elemoff, 1));
                                    pos2 = posRef_h.get(getPrimitivePointOffset(geoTmp, elemoff, 2));
                                    pos  = (pos0 + pos1 + pos2) / 3.0;
                            break;
                            }
                        }
                    }
                    if (numPointAttribMid)
                    {
                        const GA_Offset ptoffEnd = numPoint_h.get(elemoff);
                        for (GA_Size ptoff = ptoffEnd_prev; ptoff < ptoffEnd; ++ptoff)
                        {
                            if (setPositionOnElem)
                                pos_h.set(ptoff, pos);
                            if (srcElemoffAttrib)
                            {
                                if (outAsOffset || keepSrcElemAttrib)
                                    srcElemoff_h.set(ptoff, elemoff);
                                else
                                    srcElemoff_h.set(ptoff, geoTmp->primitiveIndex(elemoff));
                            }
                        }
                        ptoffEnd_prev = ptoffEnd;
                    }
                    else
                    {
                        UT_ASSERT_P(isGeoGroupFull);
                        GA_Index ptoff = geoTmp->primitiveIndex(elemoff);
                        if (setPositionOnElem)
                        {
                            pos_h.set(ptoff, pos);
                        }
                        if (srcElemoffAttrib)
                        {
                            if (outAsOffset || keepSrcElemAttrib)
                                srcElemoff_h.set(ptoff, elemoff);
                            else
                                srcElemoff_h.set(ptoff, geoTmp->primitiveIndex(elemoff));
                        }
                    }
                }
            }
        }, subscribeRatio, minGrainSize);
        
        if (keepSrcElemAttrib)
        {
            GFE_AttribCopy attribCopy(geo, geoTmp, cookparms);
            attribCopy.ownerSrc = GA_ATTRIB_PRIMITIVE;
            attribCopy.ownerDst = GA_ATTRIB_POINT;
            attribCopy.attribMergeType = GFE_AttribMergeType::Set;
            attribCopy.iDAttribInput = false;
            attribCopy.setOffsetAttrib(*srcElemoffAttrib, true);
            
            attribCopy.getRef0GroupArray().appends(attribCopy.ownerSrc, keepSrcElemGroupName);
            attribCopy.appendGroups (keepSrcElemGroupName);
            attribCopy.appendAttribs(keepSrcElemAttribName);
            attribCopy.compute();
            if (!outAsOffset)
            {
                GFE_OffsetAttribToIndex offsetAttribToIndex(geo, cookparms);
                offsetAttribToIndex.offsetToIndex = true;
                offsetAttribToIndex.getOutAttribArray().append(srcElemoffAttrib);
                offsetAttribToIndex.compute();
            }
        }
    }

    SYS_FORCE_INLINE GA_Offset getPrimitivePointOffset(const GA_Detail* const geo, const GA_Offset primoff, const GA_Size vtxpnum) const
    { return geoTmp->vertexPoint(geoTmp->getPrimitiveVertexOffset(primoff, vtxpnum)); }



    void pointGenPerPoint()
    {
        GFE_ExtractPoint extractPoint(geo, geoSrc, cookparms);
        extractPoint.groupParser.setGroup(groupParser);
        extractPoint.keepPointAttrib  = keepSrcElemAttribName;
        extractPoint.keepPointGroup   = keepSrcElemGroupName;
        extractPoint.compute();
    }
    void pointGenPerVertex()
    {
        
    }
    
    void pointGenPerEdge()
    {
        UT_ASSERT_P(dstptAttrib);
        UT_ASSERT_P(numPointAttribMid);

        GFE_GroupArray edgeGroupArray(geoTmp, cookparms);
        if (keepSrcElemAttribName)
            edgeGroupArray.appends(GA_GROUP_EDGE, keepSrcElemAttribName);
        
        const bool flag = !edgeGroupArray.isEmpty();
        const size_t sizeGroup = edgeGroupArray.size();
        
        for (size_t i = 0; i < sizeGroup; ++i)
        {
            getOutGroupArray().findOrCreatePoint(false, edgeGroupArray[i]->getName());
        }
        
        const GA_ATITopology* const vtxPointRef = geoTmp->getTopology().getPointRef();
        
        UTparallelFor(geoSplittableRange, [this, vtxPointRef, &edgeGroupArray, flag, sizeGroup](const GA_SplittableRange& r)
        {
            UT_Vector3T<fpreal> pos0, pos1;
            GA_PageHandleT<GA_Offset, GA_Offset, true, false, const GA_Attribute, const GA_ATINumeric, const GA_Detail> dstpt_ph(dstptAttrib);
            GA_PageHandleT<GA_Index,  GA_Index,  true, false, const GA_Attribute, const GA_ATINumeric, const GA_Detail> ptnum_ph(numPointAttribMid);
            for (GA_PageIterator pit = r.beginPages(); !pit.atEnd(); ++pit)
            {
                GA_Offset start, end;
                for (GA_Iterator it(pit.begin()); it.blockAdvance(start, end); )
                {
                    dstpt_ph.setPage(start);
                    ptnum_ph.setPage(start);
                    for (GA_Offset elemoff = start; elemoff < end; ++elemoff)
                    {
                        if (srcElemoffAttrib)
                            srcElemoff_h.set(ptnum_ph.value(elemoff), elemoff);
                        
                        if (setPositionOnElem)
                        {
                            pos0 = posRef_h.get(vtxPointRef->getLink(elemoff));
                            pos1 = posRef_h.get(dstpt_ph.value(elemoff));
                            pos_h.set(ptnum_ph.value(elemoff), (pos0 + pos1) * 0.5);
                        }
                        
                        if (flag)
                        {
                            const GA_Edge edge(vtxPointRef->getLink(elemoff), dstpt_ph.value(elemoff));
                            for (size_t i = 0; i < sizeGroup; ++i)
                            {
                                if (edgeGroupArray.getEdgeGroup(i)->contains(edge))
                                {
                                    getOutGroupArray().getPointGroup(i)->setElement(ptnum_ph.value(elemoff), true);
                                }
                            }
                        }
                    }
                }
            }
        }, subscribeRatio, minGrainSize);
        
    }
    

    //void scatter()
    //{
    //    scatterParms.setGroup(__TEMP_GFE_PointGenPerElem_GroupName);
    //    //scatterParms.setPrimNumAttrib();
    //    //scatterParms.setPrimNumAttrib();
    //    
    //    SOP_NodeCache* const nodeCache = scatterVerb->allocCache();
    //    const auto scatterCookparms = GFE_NodeVerb::newCookParms(cookparms, scatterParms, nodeCache, nullptr, &inputgdh);
    //    
    //    scatterVerb->cook(scatterCookparms);
    //}

public:
    GA_GroupType elemType = GA_GROUP_PRIMITIVE;
    bool setPositionOnElem = true;
    bool keepSrcElemAttrib;
    const char* srcElemoffAttribName = nullptr;
    const char* keepSrcElemAttribName = nullptr;
    const char* keepSrcElemGroupName = nullptr;
    bool outAsOffset = true;
    
private:
    const GA_Attribute* numPointAttrib = nullptr;
    exint subscribeRatio = 64;
    exint minGrainSize = 1024;


private:
    GA_Attribute* srcElemoffAttrib = nullptr;
    
private:
    const GA_ElementGroup* geoElemGroup;
    const GA_EdgeGroup* geoEdgeGroup;
    bool isGeoGroupFull;
    GA_AttributeOwner elemOwner;
    
    GA_RWHandleT<GA_Size> srcElemoff_h;
    GA_ROHandleT<GA_Size> numPoint_h;
    GA_RWHandleT<UT_Vector3T<fpreal>> pos_h;
    GA_ROHandleT<UT_Vector3T<fpreal>> posRef_h;
    
    const GA_Attribute* dstptAttrib;
    const GA_VertexGroup* creatingGroup;
    
    GA_Attribute* numPointAttribMid = nullptr;
    
    GA_SplittableRange geoSplittableRange;
    
    GU_Detail* geoTmp;


private:
    
    //UT_Array<GU_ConstDetailHandle> inputgdh;
    //SOP_Scatter_2_0Parms scatterParms;
    //const SOP_NodeVerb* const scatterVerb = SOP_NodeVerb::lookupVerb("scatter::2.0");
    
#undef __TEMP_GFE_PointGenPerElem_GroupName
    
}; // End of class GFE_PointGenPerElem


















#endif
