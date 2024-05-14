
#pragma once

#ifndef __GFE_ConvertLine_h__
#define __GFE_ConvertLine_h__

#include "GFE/GFE_ConvertLine.h"

#include "GFE/GFE_GeoFilter.h"



#include "GFE/GFE_MeshTopology.h"


class GFE_ConvertLine : public GFE_AttribFilter {

public:
    using GFE_AttribFilter::GFE_AttribFilter;
    

    ~GFE_ConvertLine()
    {
    }

    

    void
        setComputeParm(
            const bool isClosed       = false,
            const bool keepSourcePrim = false
        )
    {
        setHasComputed();
        primoffFirst = GFE_INVALID_OFFSET;
        
        this->isClosed       = isClosed;
        this->keepSourcePrim = keepSourcePrim;
    }

    SYS_FORCE_INLINE void setCopyPrimitiveAttrib(const bool copyAttrib = false, const char* const copyAttribName = "*")
    {
        copyPrimAttrib = copyAttrib;
        copyPrimAttribName = copyAttribName;
    }

    SYS_FORCE_INLINE void setCopyVertexAttrib(const bool copyAttrib = false, const char* const copyAttribName = "*")
    {
        copyVertexAttrib = copyAttrib;
        copyVertexAttribName = copyAttribName;
    }
    
    SYS_FORCE_INLINE void createSrcPrimAttrib(
        const bool detached = false,
        const GA_Storage storage = GA_STORE_INVALID,
        const UT_StringRef& srcPrimAttribName = "srcPrim"
    )
    {
        srcPrimAttribDetached = detached;
        srcPrimAttribStorage = storage;
        outSrcPrimAttrib = true;
        this->srcPrimAttribName = srcPrimAttribName;
    }


    
    SYS_FORCE_INLINE virtual void bumpDataIdsForAddOrRemove() const override
    { geo->bumpDataIdsForAddOrRemove(false, true, true); }
    
    SYS_FORCE_INLINE GA_Offset getPrimoffFirst() const
    { return primoffFirst; }

    


private:

    virtual bool
        computeCore() override
    {
        if (groupParser.isEmpty())
            return true;

        
        GU_DetailHandle geoSrc_h;
        GU_Detail* geoSrcTmpGU = new GU_Detail();
        geoSrc_h.allocateAndSet(geoSrcTmpGU);
        
        GA_Detail* geoSrcTmp = geoSrcTmpGU;
        geoSrcTmp->replaceWith(geoSrc ? *geoSrc : *geo);
        
        if (!keepSourcePrim)
            geo->replaceWithPoints(*geoSrcTmp);

        
        GA_AttributeSet& geoAttribSet = geo->getAttributes();
        GA_Attribute* attribPtr = nullptr;
        ::std::vector<GA_Attribute*> copyPrimAttribArray;
        ::std::vector<GA_Attribute*> copyVertexAttribArray;
        copyPrimAttribArray.reserve(16);
        copyVertexAttribArray.reserve(16);

        const GA_AttributeFilter allAttribFilter = GA_AttributeFilter::selectPublic();

        if (copyPrimAttrib)
        {
            for (GA_AttributeDict::iterator it = geoAttribSet.begin(GA_ATTRIB_PRIMITIVE); !it.atEnd(); ++it)
            {
                attribPtr = it.attrib();
                //if (attribPtr->getScope() != GA_SCOPE_PUBLIC)
                //    continue;
                if (attribPtr->getName().multiMatch(copyPrimAttribName))
                    copyPrimAttribArray.emplace_back(attribPtr);
                else
                    geoAttribSet.destroyAttribute(attribPtr);
            }
        }
        else
        {
            geoAttribSet.destroyAttributes(GA_ATTRIB_PRIMITIVE, allAttribFilter);
        }

        if (copyVertexAttrib)
        {
            for (GA_AttributeDict::iterator it = geoSrcTmp->getAttributes().begin(GA_ATTRIB_VERTEX); !it.atEnd(); ++it)
            {
                attribPtr = it.attrib();

                if (attribPtr->getScope() != GA_SCOPE_PUBLIC)
                    continue;
                //const char* name = srcAttribPtr->getName().c_str();
                if (attribPtr->getName().multiMatch(copyVertexAttribName))
                    copyVertexAttribArray.emplace_back(attribPtr);
            }
        }
        geoAttribSet.destroyAttributes(GA_ATTRIB_VERTEX, allAttribFilter);

        
        GFE_MeshTopology meshTopology(geoSrcTmp, cookparms);
        meshTopology.outIntermediateAttrib = false;
        meshTopology.groupParser.setGroup(groupParser.getVertexGroup());
        const GA_VertexGroup* const creatingGroup = meshTopology.setVertexNextEquivNoLoopGroup(true);
        const GA_ROHandleT<GA_Offset> dstpt_h(meshTopology.setVertexPointDst(true));
        meshTopology.compute();
        
        const GA_Size entries = creatingGroup->entries();

        UT_ASSERT_P(dstpt_h.getAttribute());

        GA_Offset vtxoff_first;
        primoffFirst = geo->appendPrimitivesAndVertices(GA_PrimitiveTypeId(1), entries, 2, vtxoff_first, isClosed);
        //geo->appendPrimitivesAndVertices(GA_PrimitiveTypeId(1), entries, 2, vtxoff_first, isClosed);

        const bool createSrcPrimAttrib = copyPrimAttrib || outSrcPrimAttrib;

        GA_Attribute* srcPrimAttrib = nullptr;
        GA_Attribute* srcVtxAttrib  = nullptr;
        if (createSrcPrimAttrib)
            srcPrimAttrib = getOutAttribArray().findOrCreateTuple(
                srcPrimAttribDetached || !outSrcPrimAttrib,
                GA_ATTRIB_PRIMITIVE, GA_STORECLASS_INT,
                srcPrimAttribStorage, srcPrimAttribName, 1, GA_Defaults(GFE_INVALID_OFFSET));

        if (copyVertexAttrib)
            srcVtxAttrib = getOutAttribArray().findOrCreateTuple(true, GA_ATTRIB_PRIMITIVE, GA_STORECLASS_INT);

        GA_RWHandleT<GA_Offset> srcPrim_h(srcPrimAttrib);
        GA_RWHandleT<GA_Offset> srcVtx_h(srcVtxAttrib);


        GA_Topology& topo = geo->getTopology();
        const GA_ATITopology& vtxPrimRef = *topo.getPrimitiveRef();


        const GA_Topology& topo_tmpGeo0 = geoSrcTmp->getTopology();
        //topo_tmpGeo0.makePrimitiveRef();
        const GA_ATITopology& vtxPointRef_geoTmp = *topo_tmpGeo0.getPointRef();
        const GA_ATITopology& vtxPrimRef_geoTmp  = *topo_tmpGeo0.getPrimitiveRef();

        GA_PageHandleT<GA_Offset, GA_Offset, true, false, const GA_Attribute, const GA_ATINumeric, const GA_Detail> dstpt_ph(dstpt_h.getAttribute());
        const GA_SplittableRange geoSplittableRange(geoSrcTmp->getVertexRange(creatingGroup));
        for (GA_PageIterator pit = geoSplittableRange.beginPages(); !pit.atEnd(); ++pit)
        {
            GA_Offset start, end;
            for (GA_Iterator it(pit.begin()); it.blockAdvance(start, end); )
            {
                dstpt_ph.setPage(start);
                for (GA_Offset elemoff = start; elemoff < end; ++elemoff)
                {
                    if (createSrcPrimAttrib)
                        srcPrim_h.set(vtxPrimRef.getLink(vtxoff_first), vtxPrimRef_geoTmp.getLink(elemoff));
                    if (copyVertexAttrib)
                        srcVtx_h.set(vtxPrimRef.getLink(vtxoff_first), elemoff);
                    topo.wireVertexPoint(vtxoff_first, vtxPointRef_geoTmp.getLink(elemoff));
                    ++vtxoff_first;
                    topo.wireVertexPoint(vtxoff_first, dstpt_ph.value(elemoff));
                    ++vtxoff_first;
                }
            }
        }

        //////////////////// copy attributes //////////////////////

        if (copyPrimAttrib || copyVertexAttrib)
        {
            const GA_Offset primoff_last = primoffFirst + entries;

            if (copyPrimAttrib)
            {
                const size_t size = copyPrimAttribArray.size();
                for (size_t i = 0; i < size; i++)
                {
                    attribPtr = copyPrimAttribArray[i];
                    attribPtr->bumpDataId();

                    if (keepSourcePrim)
                    {
                        for (GA_Offset primoff = primoffFirst; primoff < primoff_last; ++primoff)
                        {
                            //const GA_Offset srcPrimVal = srcPrim_h.get(primoff);
                            attribPtr->copy(primoff, srcPrim_h.get(primoff));
                        }
                    }
                    else
                    {
                        GA_Attribute& srcAttrib = *geoSrcTmp->findPrimitiveAttribute(attribPtr->getName());
                        for (GA_Offset primoff = primoffFirst; primoff < primoff_last; ++primoff)
                        {
                            //const GA_Offset srcPrimVal = srcPrim_h.get(primoff);
                            attribPtr->copy(primoff, srcAttrib, srcPrim_h.get(primoff));
                        }
                    }
                }
            }

            if (copyVertexAttrib)
            {
                const size_t size = copyVertexAttribArray.size();
                for (size_t i = 0; i < size; i++)
                {
                    GA_Attribute& srcAttrib = *copyVertexAttribArray[i];

                    attribPtr = geo->findPrimitiveAttribute(srcAttrib.getName());
                    if (attribPtr)
                        continue;

                    attribPtr = geoAttribSet.cloneAttribute(GA_ATTRIB_PRIMITIVE, srcAttrib.getName(), srcAttrib, true);
                    for (GA_Offset primoff = primoffFirst; primoff < primoff_last; ++primoff)
                    {
                        //const GA_Offset srcVtxVal = srcVtx_h.get(primoff);
                        attribPtr->copy(primoff, srcAttrib, srcVtx_h.get(primoff));
                    }
                }
            }
        }


        //geoTmp_h.deleteGdp();

        return true;
    }



public:
    bool isClosed = false;
    bool copyPrimAttrib = false;
    bool copyVertexAttrib = false;
    bool keepSourcePrim = false;
    const char* copyPrimAttribName;
    const char* copyVertexAttribName;

private:
    
    bool outSrcPrimAttrib = false;
    bool srcPrimAttribDetached = false;
    const char* srcPrimAttribName;
    GA_Storage srcPrimAttribStorage = GA_STORE_INVALID;

    GA_Offset primoffFirst = GFE_INVALID_OFFSET;

    //GU_DetailHandle geoSrc_h;
    //GU_Detail* geoSrcTmp;
}; // End of class GFE_ConvertLine







#endif
