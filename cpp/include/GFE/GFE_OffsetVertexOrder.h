
#pragma once

#ifndef __GFE_OffsetVertexOrder_h__
#define __GFE_OffsetVertexOrder_h__

#include <GFE/GFE_OffsetVertexOrder.h>

#include <GFE/GeoFilter.h>

_GFEL_BEGIN
class OffsetVertexOrder : public AttribFilter {

public:
    using AttribFilter::AttribFilter;

    ~OffsetVertexOrder()
    {
    }

    SYS_FORCE_INLINE void setOffsetAttrib(const UT_StringRef& attribName)
    {
        offsetAttribNonConst = geo->findAttribute(GA_ATTRIB_PRIMITIVE, attribName);
        offsetAttrib = offsetAttribNonConst;
        checkOffsetAttrib();
    }

    SYS_FORCE_INLINE void setOffsetAttrib(const GA_Attribute* const attrib)
    {
        offsetAttribNonConst = nullptr;
        offsetAttrib = attrib;
        checkOffsetAttrib();
    }

    SYS_FORCE_INLINE void setOffsetAttrib(GA_Attribute* const attrib)
    {
        offsetAttribNonConst = attrib;
        offsetAttrib = attrib;
        checkOffsetAttrib();
    }


    void
        setComputeParm(
            const int64 offset = 0,
            const bool treatLoopedPrimAsClosed = true,
            const bool delSingleVertexPrim = true,
            const bool delUnusedPoint = true,
            const bool delOffsetAttrib = false,
            const exint subscribeRatio = 64,
            const exint minGrainSize   = 1024
        )
    {
        setHasComputed();
        this->offset = offset;
        this->treatLoopedPrimAsClosed = treatLoopedPrimAsClosed;
        this->delSingleVertexPrim = delSingleVertexPrim;
        this->delUnusedPoint = delUnusedPoint;
        this->delOffsetAttrib = delOffsetAttrib;
        this->subscribeRatio = subscribeRatio;
        this->minGrainSize   = minGrainSize;
    }

    
    SYS_FORCE_INLINE virtual void bumpDataId() const override
    { 
        //geo->bumpDataIdsForRewire();
        geo->bumpDataIdsForAddOrRemove(bumpPointDataId, true, bumpPrimDataId);
        //geo->bumpDataIdsForAddOrRemove(true, true, true);
    }
    
private:

    virtual bool
        computeCore() override
    {
        if (offset == 0 && !offsetAttrib || groupParser.isEmpty())
        {
            bumpPrimDataId = false;
            bumpPointDataId = false;
            bumpVertexDataId = false;
            return true;
        }
        
        
        GA_PrimitiveGroupUPtr delPrimGroupUPtr = geo->createDetachedPrimitiveGroup();
        delPrimGroup = delPrimGroupUPtr.get();
        
        GA_PointGroupUPtr delUnusedPointGroupUPtr;
        if (delUnusedPoint)
        {
            delUnusedPointGroupUPtr = geo->createDetachedPointGroup();
            delUnusedPointGroup = delUnusedPointGroupUPtr.get();
        }
                
        if (offsetAttrib)
            offsetVertexOrderByAttrib();
        else
            offsetVertexOrder();
        
        if (delUnusedPoint)
            delUnusedPointGroup->invalidateGroupEntries();
        delPrimGroup->invalidateGroupEntries();

        bumpPrimDataId = delPrimGroup->entries() > 0;
        bumpPointDataId = delUnusedPoint && delUnusedPointGroup->entries() > 0;
        bumpVertexDataId = true;
        
        if (bumpPointDataId)
            geo->destroyPointOffsets(geo->getPointRange(delUnusedPointGroup), GA_Detail::GA_LEAVE_PRIMITIVES, true);
        
        if (bumpPrimDataId)
            geo->destroyPrimitiveOffsets(geo->getPrimitiveRange(delPrimGroup), delUnusedPoint);
        
        if (delOffsetAttrib && offsetAttribNonConst)
            geo->destroyAttrib(offsetAttribNonConst);


        return true;
    }


#if 0
    void offsetVertexOrderByAttrib()
    {
        UT_ASSERT_P(offsetAttrib);

        GA_Topology& topo = geo->getTopology();

        GA_Offset ptoff_orig;
        std::vector<GA_Offset> ptoffs_rest;
        ptoffs_rest.reserve(16);
        GA_PageHandleT<exint, exint, true, false, const GA_Attribute, const GA_ATINumeric, const GA_Detail> attrib_ph(offsetAttrib);
        for (GA_PageIterator pit = groupParser.getPrimitiveSplittableRange().beginPages(); !pit.atEnd(); ++pit)
        {
            GA_Offset start, end;
            for (GA_Iterator it(pit.begin()); it.blockAdvance(start, end); )
            {
                attrib_ph.setPage(start);
                for (GA_Offset primoff = start; primoff < end; ++primoff)
                {
                    int64 offsetFinal = offset + attrib_ph.value(primoff);
                    if (offsetFinal == 0)
                        continue;
                    
                    const GA_Size numvtx = geo->getPrimitiveVertexCount(primoff);
                    const bool isLooped = geo->getPrimitiveClosedFlag(primoff) ? true : (treatLoopedPrimAsClosed ? geo->isPrimitiveLooped(primoff) : false);
                    
                    if (isLooped)
                    {
                        offsetFinal %= numvtx;
                        if (offsetFinal > 0)
                        {
                            if (2*offsetFinal > numvtx)
                                offsetFinal -= numvtx;
                        }
                        else
                        {
                            if (-2*offsetFinal > numvtx)
                                offsetFinal += numvtx;
                        }
                        ptoffs_rest.resize(offsetFinal > 0 ? offsetFinal : -offsetFinal);
                        
                        if (offsetFinal > 0)
                        {
                            const GA_Size vtxpnum_last = numvtx-offsetFinal;
                            for (GA_Size vtxpnum = 0; vtxpnum < offsetFinal; ++vtxpnum)
                            {
                                ptoffs_rest[vtxpnum] = geo->getPrimitivePointOffset(primoff, vtxpnum);
                            }
                            for (GA_Size vtxpnum = 0; vtxpnum < vtxpnum_last; ++vtxpnum)
                            {
                                const GA_Offset vtxoff = geo->getPrimitiveVertexOffset(primoff, vtxpnum);
                                if (delUnusedPoint)
                                    ptoff_orig = geo->vertexPoint(vtxoff);

                                const GA_Offset ptoff_target = geo->getPrimitivePointOffset(primoff, vtxpnum+offsetFinal);
                                topo.wireVertexPoint(vtxoff, ptoff_target);
                            
                                if (delUnusedPoint && geo->isUnusedPoint(ptoff_orig))
                                    geo->destroyPointOffset(ptoff_orig, GA_Detail::GA_LEAVE_PRIMITIVES, true);
                            }
                            for (GA_Size vtxpnum = 0; vtxpnum < offsetFinal; ++vtxpnum)
                            {
                                const GA_Offset vtxoff = geo->getPrimitiveVertexOffset(primoff, vtxpnum+offsetFinal);
                                if (delUnusedPoint)
                                    ptoff_orig = geo->vertexPoint(vtxoff);

                                topo.wireVertexPoint(vtxoff, ptoffs_rest[vtxpnum]);
                            
                                if (delUnusedPoint && geo->isUnusedPoint(ptoff_orig))
                                    geo->destroyPointOffset(ptoff_orig, GA_Detail::GA_LEAVE_PRIMITIVES, true);
                            }
                        }
                        else
                        {
                            const GA_Size offset_positive = -offsetFinal;
                            const GA_Size vtxpnum_last = numvtx + offsetFinal;
                            for (GA_Size vtxpnum = 0; vtxpnum < offset_positive; ++vtxpnum)
                            {
                                ptoffs_rest[vtxpnum] = geo->getPrimitivePointOffset(primoff, vtxpnum_last + vtxpnum);
                            }
                            for (GA_Size vtxpnum = offset_positive; vtxpnum < numvtx; ++vtxpnum)
                            {
                                const GA_Offset vtxoff = geo->getPrimitiveVertexOffset(primoff, vtxpnum);
                                if (delUnusedPoint)
                                    ptoff_orig = geo->vertexPoint(vtxoff);
                                
                                const GA_Offset ptoff_target = geo->getPrimitivePointOffset(primoff, vtxpnum+offsetFinal);
                                topo.wireVertexPoint(vtxoff, ptoff_target);
                            
                                if (delUnusedPoint && geo->isUnusedPoint(ptoff_orig))
                                    geo->destroyPointOffset(ptoff_orig, GA_Detail::GA_LEAVE_PRIMITIVES, true);
                            }
                            for (GA_Size vtxpnum = 0; vtxpnum < offset_positive; ++vtxpnum)
                            {
                                const GA_Offset vtxoff = geo->getPrimitiveVertexOffset(primoff, vtxpnum);
                                if (delUnusedPoint)
                                    ptoff_orig = geo->vertexPoint(vtxoff);

                                topo.wireVertexPoint(vtxoff, ptoffs_rest[vtxpnum]);
                            
                                if (delUnusedPoint && geo->isUnusedPoint(ptoff_orig))
                                    geo->destroyPointOffset(ptoff_orig, GA_Detail::GA_LEAVE_PRIMITIVES, true);
                            }
                        }
                    }
                    else
                    {
                        if (offsetFinal > 0)
                        {
                            if (offsetFinal >= numvtx)
                            {
                                geo->destroyPrimitiveOffset(primoff, delUnusedPoint);
                            }
                            else
                            {
                                const GA_Size vtxpnum_last = numvtx-offsetFinal-1;
                                for (GA_Size vtxpnum = 0; vtxpnum <= vtxpnum_last; ++vtxpnum)
                                {
                                    const GA_Offset vtxoff = geo->getPrimitiveVertexOffset(primoff, vtxpnum);
                                    if (delUnusedPoint)
                                        ptoff_orig = geo->vertexPoint(vtxoff);

                                    const GA_Offset ptoff_target = geo->getPrimitivePointOffset(primoff, vtxpnum+offsetFinal);
                                    topo.wireVertexPoint(vtxoff, ptoff_target);
                            
                                    if (delUnusedPoint && geo->isUnusedPoint(ptoff_orig))
                                        geo->destroyPointOffset(ptoff_orig, GA_Detail::GA_LEAVE_PRIMITIVES, true);
                                }
                                for (GA_Size vtxpnum = numvtx-1; vtxpnum > vtxpnum_last; --vtxpnum)
                                {
                                    const GA_Offset vtxoff = geo->getPrimitiveVertexOffset(primoff, vtxpnum);
                                    geo->getPrimitive(primoff)->releaseVertex(vtxoff);
                                    topo.delVertex(vtxoff);
                                    //geo->destroyVertexOffset(vtxoff);
                                }
                            }
                        }
                        else
                        {
                            const GA_Size offset_positive = -offsetFinal;
                            if (offset_positive >= numvtx)
                            {
                                geo->destroyPrimitiveOffset(primoff, delUnusedPoint);
                            }
                            else
                            {
                                for (GA_Size vtxpnum = numvtx-1; vtxpnum >= offset_positive; --vtxpnum)
                                {
                                    const GA_Offset vtxoff = geo->getPrimitiveVertexOffset(primoff, vtxpnum);
                                    if (delUnusedPoint)
                                        ptoff_orig = geo->vertexPoint(vtxoff);
                                    
                                    const GA_Offset ptoff_target = geo->getPrimitivePointOffset(primoff, vtxpnum+offsetFinal);
                                    topo.wireVertexPoint(vtxoff, ptoff_target);
                                    
                                    if (delUnusedPoint && geo->isUnusedPoint(ptoff_orig))
                                        geo->destroyPointOffset(ptoff_orig, GA_Detail::GA_LEAVE_PRIMITIVES, true);
                                }
                                for (GA_Size vtxpnum = offset_positive-1; vtxpnum >= 0; --vtxpnum)
                                {
                                    const GA_Offset vtxoff = geo->getPrimitiveVertexOffset(primoff, vtxpnum);
                                    geo->getPrimitive(primoff)->releaseVertex(vtxoff);
                                    topo.delVertex(vtxoff);
                                    //geo->destroyVertexOffset(vtxoff);
                                }
                            }
                        }
                    }
                }
            }
        }
    }


    void offsetVertexOrder()
    {
        GA_Topology& topo = geo->getTopology();

        GA_Offset ptoff_orig;
        std::vector<GA_Offset> ptoffs_rest;
        ptoffs_rest.reserve(16);
        GA_Offset start, end;
        for (GA_Iterator it(groupParser.getPrimitiveRange()); it.blockAdvance(start, end); )
        {
            for (GA_Offset primoff = start; primoff < end; ++primoff)
            {
                int64 offsetFinal = offset;
                if (offsetFinal == 0)
                    continue;
                
                const GA_Size numvtx = geo->getPrimitiveVertexCount(primoff);
                const bool isLooped = geo->getPrimitiveClosedFlag(primoff) ? true : (treatLoopedPrimAsClosed ? geo->isPrimitiveLooped(primoff) : false);
                
                if (isLooped)
                {
                    offsetFinal %= numvtx;
                    if (offsetFinal > 0)
                    {
                        if (2*offsetFinal > numvtx)
                            offsetFinal -= numvtx;
                    }
                    else
                    {
                        if (-2*offsetFinal > numvtx)
                            offsetFinal += numvtx;
                    }
                    ptoffs_rest.resize(offsetFinal > 0 ? offsetFinal : -offsetFinal);
                    
                    if (offsetFinal > 0)
                    {
                        const GA_Size vtxpnum_last = numvtx-offsetFinal;
                        for (GA_Size vtxpnum = 0; vtxpnum < offsetFinal; ++vtxpnum)
                        {
                            ptoffs_rest[vtxpnum] = geo->getPrimitivePointOffset(primoff, vtxpnum);
                        }
                        for (GA_Size vtxpnum = 0; vtxpnum < vtxpnum_last; ++vtxpnum)
                        {
                            const GA_Offset vtxoff = geo->getPrimitiveVertexOffset(primoff, vtxpnum);
                            if (delUnusedPoint)
                                ptoff_orig = geo->vertexPoint(vtxoff);
                            
                            const GA_Offset ptoff_target = geo->getPrimitivePointOffset(primoff, vtxpnum+offsetFinal);
                            topo.wireVertexPoint(vtxoff, ptoff_target);
                            
                            if (delUnusedPoint && geo->isUnusedPoint(ptoff_orig))
                                geo->destroyPointOffset(ptoff_orig, GA_Detail::GA_LEAVE_PRIMITIVES, true);
                        }
                        for (GA_Size vtxpnum = 0; vtxpnum < offsetFinal; ++vtxpnum)
                        {
                            const GA_Offset vtxoff = geo->getPrimitiveVertexOffset(primoff, vtxpnum+offsetFinal);
                            if (delUnusedPoint)
                                ptoff_orig = geo->vertexPoint(vtxoff);
                            
                            topo.wireVertexPoint(vtxoff, ptoffs_rest[vtxpnum]);
                            
                            if (delUnusedPoint && geo->isUnusedPoint(ptoff_orig))
                                geo->destroyPointOffset(ptoff_orig, GA_Detail::GA_LEAVE_PRIMITIVES, true);
                        }
                    }
                    else
                    {
                        const GA_Size offset_positive = -offsetFinal;
                        const GA_Size vtxpnum_last = numvtx + offsetFinal;
                        for (GA_Size vtxpnum = 0; vtxpnum < offset_positive; ++vtxpnum)
                        {
                            ptoffs_rest[vtxpnum] = geo->getPrimitivePointOffset(primoff, vtxpnum_last + vtxpnum);
                        }
                        for (GA_Size vtxpnum = offset_positive; vtxpnum < numvtx; ++vtxpnum)
                        {
                            const GA_Offset vtxoff = geo->getPrimitiveVertexOffset(primoff, vtxpnum);
                            if (delUnusedPoint)
                                ptoff_orig = geo->vertexPoint(vtxoff);
                            
                            const GA_Offset ptoff_target = geo->getPrimitivePointOffset(primoff, vtxpnum+offsetFinal);
                            topo.wireVertexPoint(vtxoff, ptoff_target);
                            
                            if (delUnusedPoint && geo->isUnusedPoint(ptoff_orig))
                                geo->destroyPointOffset(ptoff_orig, GA_Detail::GA_LEAVE_PRIMITIVES, true);
                        }
                        for (GA_Size vtxpnum = 0; vtxpnum < offset_positive; ++vtxpnum)
                        {
                            const GA_Offset vtxoff = geo->getPrimitiveVertexOffset(primoff, vtxpnum);
                            if (delUnusedPoint)
                                ptoff_orig = geo->vertexPoint(vtxoff);

                            topo.wireVertexPoint(vtxoff, ptoffs_rest[vtxpnum]);
                            
                            if (delUnusedPoint && geo->isUnusedPoint(ptoff_orig))
                                geo->destroyPointOffset(ptoff_orig, GA_Detail::GA_LEAVE_PRIMITIVES, true);
                        }
                    }
                }
                else
                {
                    if (offsetFinal > 0)
                    {
                        if (offsetFinal >= numvtx)
                        {
                            geo->destroyPrimitiveOffset(primoff, delUnusedPoint);
                        }
                        else
                        {
                            const GA_Size vtxpnum_last = numvtx-offsetFinal-1;
                            for (GA_Size vtxpnum = 0; vtxpnum <= vtxpnum_last; ++vtxpnum)
                            {
                                const GA_Offset vtxoff = geo->getPrimitiveVertexOffset(primoff, vtxpnum);
                                if (delUnusedPoint)
                                    ptoff_orig = geo->vertexPoint(vtxoff);

                                const GA_Offset ptoff_target = geo->getPrimitivePointOffset(primoff, vtxpnum+offsetFinal);
                                topo.wireVertexPoint(vtxoff, ptoff_target);
                            
                                if (delUnusedPoint && geo->isUnusedPoint(ptoff_orig))
                                    geo->destroyPointOffset(ptoff_orig, GA_Detail::GA_LEAVE_PRIMITIVES, true);
                            }
                            for (GA_Size vtxpnum = numvtx-1; vtxpnum > vtxpnum_last; --vtxpnum)
                            {
                                const GA_Offset vtxoff = geo->getPrimitiveVertexOffset(primoff, vtxpnum);
                                geo->getPrimitive(primoff)->releaseVertex(vtxoff);
                                topo.delVertex(vtxoff);
                                //geo->destroyVertexOffset(vtxoff);
                            }
                        }
                    }
                    else
                    {
                        const GA_Size offset_positive = -offsetFinal;
                        if (offset_positive >= numvtx)
                        {
                            geo->destroyPrimitiveOffset(primoff, delUnusedPoint);
                        }
                        else
                        {
                            for (GA_Size vtxpnum = numvtx-1; vtxpnum >= offset_positive; --vtxpnum)
                            {
                                const GA_Offset vtxoff = geo->getPrimitiveVertexOffset(primoff, vtxpnum);
                                if (delUnusedPoint)
                                    ptoff_orig = geo->vertexPoint(vtxoff);
                                
                                const GA_Offset ptoff_target = geo->getPrimitivePointOffset(primoff, vtxpnum+offsetFinal);
                                topo.wireVertexPoint(vtxoff, ptoff_target);
                            
                                if (delUnusedPoint && geo->isUnusedPoint(ptoff_orig))
                                    geo->destroyPointOffset(ptoff_orig, GA_Detail::GA_LEAVE_PRIMITIVES, true);
                            }
                            for (GA_Size vtxpnum = offset_positive-1; vtxpnum >= 0; --vtxpnum)
                            {
                                const GA_Offset vtxoff = geo->getPrimitiveVertexOffset(primoff, vtxpnum);
                                geo->getPrimitive(primoff)->releaseVertex(vtxoff);
                                topo.delVertex(vtxoff);
                                //geo->destroyVertexOffset(vtxoff);
                            }
                        }
                    }
                }
            }
        }
    }
#else
    void offsetVertexOrderByAttrib()
    {
        UT_ASSERT_P(offsetAttrib);
    
        GA_Topology& topo = geo->getTopology();
        
        UTparallelFor(groupParser.getPrimitiveSplittableRange(), [this, &topo](const GA_SplittableRange& r)
        {
            GA_Offset ptoff_orig;
            std::vector<GA_Offset> ptoffs_rest;
            ptoffs_rest.reserve(16);
            
            GA_PageHandleT<exint, exint, true, false, const GA_Attribute, const GA_ATINumeric, const GA_Detail> attrib_ph(offsetAttrib);
            for (GA_PageIterator pit = r.beginPages(); !pit.atEnd(); ++pit)
            {
                GA_Offset start, end;
                for (GA_Iterator it(r); it.blockAdvance(start, end); )
                {
                    attrib_ph.setPage(start);
                    for (GA_Offset primoff = start; primoff < end; ++primoff)
                    {
                        int64 offsetFinal = offset + attrib_ph.value(primoff);
                        if (offsetFinal == 0)
                            continue;

                        
                        const GA_OffsetListRef& vertices = geo->getPrimitiveVertexList(primoff);
                        const GA_Size numvtx = vertices.size();
                        const GA_Size numvtx_delPrim = delSingleVertexPrim ? numvtx-1 : numvtx;
                        const bool isLooped = geo->getPrimitiveClosedFlag(primoff) ? true : (treatLoopedPrimAsClosed ? geo->isPrimitiveLooped(primoff) : false);
                        
                        if (isLooped)
                        {
                            offsetFinal %= numvtx;
                            if (offsetFinal > 0)
                            {
                                if (2*offsetFinal > numvtx)
                                    offsetFinal -= numvtx;
                            }
                            else
                            {
                                if (-2*offsetFinal > numvtx)
                                    offsetFinal += numvtx;
                            }
                            ptoffs_rest.resize(offsetFinal > 0 ? offsetFinal : -offsetFinal);
                            
                            if (offsetFinal > 0)
                            {
                                const GA_Size vtxpnum_last = numvtx-offsetFinal;
                                for (GA_Size vtxpnum = 0; vtxpnum < offsetFinal; ++vtxpnum)
                                {
                                    ptoffs_rest[vtxpnum] = geo->vertexPoint(vertices[vtxpnum]);
                                }
                                for (GA_Size vtxpnum = 0; vtxpnum < vtxpnum_last; ++vtxpnum)
                                {
                                    const GA_Offset vtxoff = vertices[vtxpnum];
                                    if (delUnusedPoint)
                                        ptoff_orig = geo->vertexPoint(vtxoff);
                                    
                                    const GA_Offset ptoff_target = geo->vertexPoint(vertices[vtxpnum+offsetFinal]);
                                    topo.wireVertexPoint(vtxoff, ptoff_target);
                                    
                                    if (delUnusedPoint && geo->isUnusedPoint(ptoff_orig))
                                        delUnusedPointGroup->setElement(ptoff_orig, true);
                                        //geo->destroyPointOffset(ptoff_orig, GA_Detail::GA_LEAVE_PRIMITIVES, true);
                                }
                                for (GA_Size vtxpnum = 0; vtxpnum < offsetFinal; ++vtxpnum)
                                {
                                    const GA_Offset vtxoff = vertices[vtxpnum+offsetFinal];
                                    if (delUnusedPoint)
                                        ptoff_orig = geo->vertexPoint(vtxoff);
                                    
                                    topo.wireVertexPoint(vtxoff, ptoffs_rest[vtxpnum]);
                                    
                                    if (delUnusedPoint && geo->isUnusedPoint(ptoff_orig))
                                        delUnusedPointGroup->setElement(ptoff_orig, true);
                                        //geo->destroyPointOffset(ptoff_orig, GA_Detail::GA_LEAVE_PRIMITIVES, true);
                                }
                            }
                            else
                            {
                                const GA_Size offset_positive = -offsetFinal;
                                const GA_Size vtxpnum_last = numvtx + offsetFinal;
                                for (GA_Size vtxpnum = 0; vtxpnum < offset_positive; ++vtxpnum)
                                {
                                    ptoffs_rest[vtxpnum] = geo->vertexPoint(vertices[vtxpnum_last + vtxpnum]);
                                }
                                for (GA_Size vtxpnum = offset_positive; vtxpnum < numvtx; ++vtxpnum)
                                {
                                    const GA_Offset vtxoff = vertices[vtxpnum];
                                    if (delUnusedPoint)
                                        ptoff_orig = geo->vertexPoint(vtxoff);
                                    
                                    const GA_Offset ptoff_target = geo->vertexPoint(vertices[offsetFinal + vtxpnum]);
                                    topo.wireVertexPoint(vtxoff, ptoff_target);
                                    
                                    if (delUnusedPoint && geo->isUnusedPoint(ptoff_orig))
                                        delUnusedPointGroup->setElement(ptoff_orig, true);
                                        //geo->destroyPointOffset(ptoff_orig, GA_Detail::GA_LEAVE_PRIMITIVES, true);
                                }
                                for (GA_Size vtxpnum = 0; vtxpnum < offset_positive; ++vtxpnum)
                                {
                                    const GA_Offset vtxoff = vertices[vtxpnum];
                                    if (delUnusedPoint)
                                        ptoff_orig = geo->vertexPoint(vtxoff);

                                    topo.wireVertexPoint(vtxoff, ptoffs_rest[vtxpnum]);
                                    
                                    if (delUnusedPoint && geo->isUnusedPoint(ptoff_orig))
                                        delUnusedPointGroup->setElement(ptoff_orig, true);
                                        //geo->destroyPointOffset(ptoff_orig, GA_Detail::GA_LEAVE_PRIMITIVES, true);
                                }
                            }
                        }
                        else
                        {
                            if (offsetFinal > 0)
                            {
                                if (offsetFinal >= numvtx_delPrim)
                                {
                                    delPrimGroup->setElement(primoff, true);
                                    //geo->destroyPrimitiveOffset(primoff, delUnusedPoint);
                                }
                                else
                                {
                                    const GA_Size vtxpnum_last = numvtx-offsetFinal-1;
                                    for (GA_Size vtxpnum = 0; vtxpnum <= vtxpnum_last; ++vtxpnum)
                                    {
                                        const GA_Offset vtxoff = vertices[vtxpnum];
                                        if (delUnusedPoint)
                                            ptoff_orig = geo->vertexPoint(vtxoff);

                                        const GA_Offset ptoff_target = geo->vertexPoint(vertices[vtxpnum+offsetFinal]);
                                        topo.wireVertexPoint(vtxoff, ptoff_target);
                                    
                                        if (delUnusedPoint && geo->isUnusedPoint(ptoff_orig))
                                            delUnusedPointGroup->setElement(ptoff_orig, true);
                                            //geo->destroyPointOffset(ptoff_orig, GA_Detail::GA_LEAVE_PRIMITIVES, true);
                                    }
                                    for (GA_Size vtxpnum = numvtx-1; vtxpnum > vtxpnum_last; --vtxpnum)
                                    {
                                        const GA_Offset vtxoff = vertices[vtxpnum];
                                        geo->getPrimitive(primoff)->releaseVertex(vtxoff);
                                        topo.delVertex(vtxoff);
                                        //geo->destroyVertexOffset(vtxoff);
                                    }
                                }
                            }
                            else
                            {
                                const GA_Size offset_positive = -offsetFinal;
                                if (offset_positive >= numvtx_delPrim)
                                {
                                    delPrimGroup->setElement(primoff, true);
                                    //geo->destroyPrimitiveOffset(primoff, delUnusedPoint);
                                }
                                else
                                {
                                    for (GA_Size vtxpnum = numvtx-1; vtxpnum >= offset_positive; --vtxpnum)
                                    {
                                        const GA_Offset vtxoff = vertices[vtxpnum];
                                        if (delUnusedPoint)
                                            ptoff_orig = geo->vertexPoint(vtxoff);
                                        
                                        const GA_Offset ptoff_target = geo->vertexPoint(vertices[vtxpnum+offsetFinal]);
                                        topo.wireVertexPoint(vtxoff, ptoff_target);
                                    
                                        if (delUnusedPoint && geo->isUnusedPoint(ptoff_orig))
                                            delUnusedPointGroup->setElement(ptoff_orig, true);
                                            //geo->destroyPointOffset(ptoff_orig, GA_Detail::GA_LEAVE_PRIMITIVES, true);
                                    }
                                    for (GA_Size vtxpnum = offset_positive-1; vtxpnum >= 0; --vtxpnum)
                                    {
                                        const GA_Offset vtxoff = vertices[vtxpnum];
                                        geo->getPrimitive(primoff)->releaseVertex(vtxoff);
                                        topo.delVertex(vtxoff);
                                        //geo->destroyVertexOffset(vtxoff);
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }, subscribeRatio, minGrainSize);
    }
    
    
    
    void offsetVertexOrder()
    {
        GA_Topology& topo = geo->getTopology();

    
        UTparallelFor(groupParser.getPrimitiveSplittableRange(), [this, &topo](const GA_SplittableRange& r)
        {
            GA_Offset ptoff_orig;
            std::vector<GA_Offset> ptoffs_rest;
            ptoffs_rest.reserve(16);
            
            GA_Offset start, end;
            for (GA_Iterator it(r); it.blockAdvance(start, end); )
            {
                for (GA_Offset primoff = start; primoff < end; ++primoff)
                {
                    int64 offsetFinal = offset;
                    if (offsetFinal == 0)
                        continue;
                    
                    const GA_OffsetListRef& vertices = geo->getPrimitiveVertexList(primoff);
                    const GA_Size numvtx = vertices.size();
                    const GA_Size numvtx_delPrim = delSingleVertexPrim ? numvtx-1 : numvtx;
                    
                    const bool isLooped = geo->getPrimitiveClosedFlag(primoff) ? true : (treatLoopedPrimAsClosed ? geo->isPrimitiveLooped(primoff) : false);
                    
                    if (isLooped)
                    {
                        offsetFinal %= numvtx;
                        if (offsetFinal > 0)
                        {
                            if (2*offsetFinal > numvtx)
                                offsetFinal -= numvtx;
                        }
                        else
                        {
                            if (-2*offsetFinal > numvtx)
                                offsetFinal += numvtx;
                        }
                        ptoffs_rest.resize(offsetFinal > 0 ? offsetFinal : -offsetFinal);
                        
                        if (offsetFinal > 0)
                        {
                            const GA_Size vtxpnum_last = numvtx-offsetFinal;
                            for (GA_Size vtxpnum = 0; vtxpnum < offsetFinal; ++vtxpnum)
                            {
                                ptoffs_rest[vtxpnum] = geo->vertexPoint(vertices[vtxpnum]);
                            }
                            for (GA_Size vtxpnum = 0; vtxpnum < vtxpnum_last; ++vtxpnum)
                            {
                                const GA_Offset vtxoff = vertices[vtxpnum];
                                if (delUnusedPoint)
                                    ptoff_orig = geo->vertexPoint(vtxoff);
                                
                                const GA_Offset ptoff_target = geo->vertexPoint(vertices[vtxpnum+offsetFinal]);;
                                topo.wireVertexPoint(vtxoff, ptoff_target);
                                
                                if (delUnusedPoint && geo->isUnusedPoint(ptoff_orig))
                                    delUnusedPointGroup->setElement(ptoff_orig, true);
                                    //geo->destroyPointOffset(ptoff_orig, GA_Detail::GA_LEAVE_PRIMITIVES, true);
                            }
                            for (GA_Size vtxpnum = 0; vtxpnum < offsetFinal; ++vtxpnum)
                            {
                                const GA_Offset vtxoff = vertices[vtxpnum+offsetFinal];
                                if (delUnusedPoint)
                                    ptoff_orig = geo->vertexPoint(vtxoff);
                                
                                topo.wireVertexPoint(vtxoff, ptoffs_rest[vtxpnum]);
                                
                                if (delUnusedPoint && geo->isUnusedPoint(ptoff_orig))
                                    delUnusedPointGroup->setElement(ptoff_orig, true);
                                    //geo->destroyPointOffset(ptoff_orig, GA_Detail::GA_LEAVE_PRIMITIVES, true);
                            }
                        }
                        else
                        {
                            const GA_Size offset_positive = -offsetFinal;
                            const GA_Size vtxpnum_last = numvtx + offsetFinal;
                            for (GA_Size vtxpnum = 0; vtxpnum < offset_positive; ++vtxpnum)
                            {
                                ptoffs_rest[vtxpnum] = geo->vertexPoint(vertices[vtxpnum_last + vtxpnum]);
                            }
                            for (GA_Size vtxpnum = offset_positive; vtxpnum < numvtx; ++vtxpnum)
                            {
                                const GA_Offset vtxoff = vertices[vtxpnum];
                                if (delUnusedPoint)
                                    ptoff_orig = geo->vertexPoint(vtxoff);
                                
                                const GA_Offset ptoff_target = geo->vertexPoint(vertices[vtxpnum+offsetFinal]);
                                topo.wireVertexPoint(vtxoff, ptoff_target);
                                
                                if (delUnusedPoint && geo->isUnusedPoint(ptoff_orig))
                                    delUnusedPointGroup->setElement(ptoff_orig, true);
                                    //geo->destroyPointOffset(ptoff_orig, GA_Detail::GA_LEAVE_PRIMITIVES, true);
                            }
                            for (GA_Size vtxpnum = 0; vtxpnum < offset_positive; ++vtxpnum)
                            {
                                const GA_Offset vtxoff = vertices[vtxpnum];
                                if (delUnusedPoint)
                                    ptoff_orig = geo->vertexPoint(vtxoff);

                                topo.wireVertexPoint(vtxoff, ptoffs_rest[vtxpnum]);
                                
                                if (delUnusedPoint && geo->isUnusedPoint(ptoff_orig))
                                    delUnusedPointGroup->setElement(ptoff_orig, true);
                                    //geo->destroyPointOffset(ptoff_orig, GA_Detail::GA_LEAVE_PRIMITIVES, true);
                            }
                        }
                    }
                    else
                    {
                        if (offsetFinal > 0)
                        {
                            if (offsetFinal >= numvtx_delPrim)
                            {
                                delPrimGroup->setElement(primoff, true);
                                //geo->destroyPrimitiveOffset(primoff, delUnusedPoint);
                            }
                            else
                            {
                                const GA_Size vtxpnum_last = numvtx-offsetFinal-1;
                                for (GA_Size vtxpnum = 0; vtxpnum <= vtxpnum_last; ++vtxpnum)
                                {
                                    const GA_Offset vtxoff = vertices[vtxpnum];
                                    if (delUnusedPoint)
                                        ptoff_orig = geo->vertexPoint(vtxoff);

                                    const GA_Offset ptoff_target = geo->vertexPoint(vertices[vtxpnum+offsetFinal]);
                                    topo.wireVertexPoint(vtxoff, ptoff_target);
                                
                                    if (delUnusedPoint && geo->isUnusedPoint(ptoff_orig))
                                        delUnusedPointGroup->setElement(ptoff_orig, true);
                                        //geo->destroyPointOffset(ptoff_orig, GA_Detail::GA_LEAVE_PRIMITIVES, true);
                                }
                                for (GA_Size vtxpnum = numvtx-1; vtxpnum > vtxpnum_last; --vtxpnum)
                                {
                                    const GA_Offset vtxoff = vertices[vtxpnum];
                                    geo->getPrimitive(primoff)->releaseVertex(vtxoff);
                                    topo.delVertex(vtxoff);
                                    //geo->destroyVertexOffset(vtxoff);
                                }
                            }
                        }
                        else
                        {
                            const GA_Size offset_positive = -offsetFinal;
                            if (offset_positive >= numvtx_delPrim)
                            {
                                delPrimGroup->setElement(primoff, true);
                                //geo->destroyPrimitiveOffset(primoff, delUnusedPoint);
                            }
                            else
                            {
                                for (GA_Size vtxpnum = numvtx-1; vtxpnum >= offset_positive; --vtxpnum)
                                {
                                    const GA_Offset vtxoff = vertices[vtxpnum];
                                    if (delUnusedPoint)
                                        ptoff_orig = geo->vertexPoint(vtxoff);
                                    
                                    const GA_Offset ptoff_target = geo->vertexPoint(vertices[vtxpnum+offsetFinal]);
                                    topo.wireVertexPoint(vtxoff, ptoff_target);
                                
                                    if (delUnusedPoint && geo->isUnusedPoint(ptoff_orig))
                                        delUnusedPointGroup->setElement(ptoff_orig, true);
                                        //geo->destroyPointOffset(ptoff_orig, GA_Detail::GA_LEAVE_PRIMITIVES, true);
                                }
                                for (GA_Size vtxpnum = offset_positive-1; vtxpnum >= 0; --vtxpnum)
                                {
                                    const GA_Offset vtxoff = vertices[vtxpnum];
                                    geo->getPrimitive(primoff)->releaseVertex(vtxoff);
                                    topo.delVertex(vtxoff);
                                    //geo->destroyVertexOffset(vtxoff);
                                }
                            }
                        }
                    }
                }
            }
        }, subscribeRatio, minGrainSize);
    }
#endif

    SYS_FORCE_INLINE void checkOffsetAttrib()
    {
        if (!offsetAttrib)
            return;
        
        const GA_AIFTuple* const aifTuple = offsetAttrib->getAIFTuple();
        if (aifTuple && offsetAttrib->getStorageClass() == GA_STORECLASS_INT)
            return;
            
        offsetAttribNonConst = nullptr;
        offsetAttrib = nullptr;
    }

public:
    int64 offset = 0;
    bool treatLoopedPrimAsClosed = true;
    bool delSingleVertexPrim = true;
    bool delUnusedPoint = true;
    bool delOffsetAttrib = false;
    
private:
    
    const GA_Attribute* offsetAttrib = nullptr;
    GA_Attribute* offsetAttribNonConst = nullptr;
    GA_PointGroup* delUnusedPointGroup = nullptr;
    GA_PrimitiveGroup* delPrimGroup = nullptr;
        
    bool bumpPrimDataId;
    bool bumpPointDataId;
    bool bumpVertexDataId;

    exint subscribeRatio = 64;
    exint minGrainSize   = 1024;


}; // End of Class OffsetVertexOrder
_GFEL_END
#endif
