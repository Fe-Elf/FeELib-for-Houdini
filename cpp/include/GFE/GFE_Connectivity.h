
#pragma once

#ifndef __GFE_Connectivity_h__
#define __GFE_Connectivity_h__

#include <GFE/Connectivity.h>

#include <GFE/GeoFilter.h>


#include <GFE/GFE_MeshTopology.h>
#include <GFE/GFE_AttributeCast.h>
#include <GFE/GFE_AttributePromote.h>
#include <GFE/GFE_PackedArrayOfArrays.h>


#ifdef GFE_DEBUG_MODE
#define __GFE_Connectivity_OutTime 1
#else
#define __GFE_Connectivity_OutTime 1
#endif

_GFEL_BEGIN
class Connectivity : public GFE_AttribFilter
{

//#define GFE_TEMP_ConnectivityAttribName "__topo_GFE_ConnectivityAttrib"

public:


Connectivity(
    GFE_Detail* const geo,
    const SOP_NodeVerb::CookParms* const cookparms = nullptr
)
    : GFE_AttribFilter(geo, cookparms)
    , groupParserSeam(geo, groupParser.getGOPRef(), cookparms)
{
}

Connectivity(
    GA_Detail& geo,
    const SOP_NodeVerb::CookParms* const cookparms = nullptr
)
    : GFE_AttribFilter(geo, cookparms)
    , groupParserSeam(geo, groupParser.getGOPRef(), cookparms)
{
}
    
~Connectivity()
{
}



    
    void setComputeParm(
        const bool connectivityConstraint = false,
        const bool outTopoAttrib = true,
        const exint subscribeRatio = 64,
        const exint minGrainSize   = 1024
    )
    {
        setHasComputed();
        this->connectivityConstraint = connectivityConstraint;
        this->outTopoAttrib  = outTopoAttrib;
        this->subscribeRatio = subscribeRatio;
        this->minGrainSize   = minGrainSize;
    }

    void
        findOrCreateTuple(
            const bool detached = false,
            const GA_AttributeOwner attribOwner = GA_ATTRIB_PRIMITIVE,
            const GA_StorageClass storageClass = GA_STORECLASS_INT,
            const GA_Storage storage = GA_STORE_INVALID,
            const UT_StringRef& name = "__topo_connectivity"
        )
    {
        getOutAttribArray().clear();

        const GA_AttributeOwner connectivityOwner = connectivityConstraint ? GA_ATTRIB_PRIMITIVE : GA_ATTRIB_POINT;
        const bool isDetached = connectivityOwner != attribOwner || storageClass != GA_STORECLASS_INT;
#if 1
        if (isDetached)
            getOutAttribArray().findOrCreateTuple(detached, attribOwner, storageClass, storage, name, 1, GA_Defaults(GFE_INVALID_OFFSET));
    
        connectivityAttrib = getOutAttribArray().findOrCreateTuple(isDetached, connectivityOwner, GA_STORECLASS_INT, GA_STORE_INVALID,
            isDetached ? UT_StringRef("") : name, 1, GA_Defaults(GFE_INVALID_OFFSET));
#else
        connectivityAttrib = getOutAttribArray().findOrCreateTuple(true, connectivityOwner, GA_STORECLASS_INT, storage, isDetached ? GFE_TEMP_ConnectivityAttribName : name);
        if (isDetached)
            getOutAttribArray().findOrCreateTuple(detached, attribOwner, storageClass, storage, name);
#endif
    }

    SYS_FORCE_INLINE GA_Attribute* getConnectivityAttrib() const
    { return getOutAttribArray().isEmpty() ? nullptr : getOutAttribArray()[0]; }
            







private:


    virtual bool
        computeCore() override
    {
        if (getOutAttribArray().isEmpty())
            return false;
        
        if (groupParser.isEmpty())
            return true;

        if (computeTopologyAttrib)
            connectivity<GFE_OutArrayType::Attrib>();
        else
            connectivity<GFE_OutArrayType::Geo>();

        if (getOutAttribArray().size() > 1)
        {
            GA_Attribute* const outAttribPtr = getOutAttribArray()[0];
            const GA_AttributeOwner connectivityOriginOwner = connectivityConstraint ? GA_ATTRIB_PRIMITIVE : GA_ATTRIB_POINT;
            //const bool isDetached = connectivityOwner != outAttrib->get || storageClass != GA_STORECLASS_INT;

            const bool shouldPromote = outAttribPtr->getOwner() != connectivityOriginOwner;
            const bool shouldCast = outAttribPtr->getStorageClass() != GA_STORECLASS_INT;
        
            GFE_AttribPromote attribPromote(geo);
            if (shouldPromote)
            {
                //GA_Storage sto = GFE_Attribute::getStorage(connectivityAttrib);
                attribPromote.getInAttribArray().set(connectivityAttrib);
                
                if (shouldCast)
                    attribPromote.dstAttribClass = outAttribPtr->getOwner();
                else
                    attribPromote.getOutAttribArray().set(outAttribPtr);
                
                attribPromote.compute();
            }
            if (shouldCast)
            {
                GA_Attribute* const inAttribPtr = shouldPromote ? attribPromote.getOutAttribArray()[0] : connectivityAttrib;
                //GA_StorageClass storage537 = inAttribPtr->getStorageClass();
                //GA_Storage storage124 = GFE_Attribute::getStorage(inAttribPtr);
                GFE_AttribCast attribCast(geo);
                attribCast.getInAttribArray().set(inAttribPtr);
                attribCast.setDestinationAttribute(*outAttribPtr);
                //attribCast.newStorageClass = outAttribStorageClass;
                //attribCast.precision = GA_PRECISION_INVALID;
                attribCast.compute();
                //GA_Attribute* const attrib = attribCast.getOutAttribArray()[0];
                //const char* a = attrib->getName().c_str();
                //const char* b = outAttribPtr->getName().c_str();
            }
            //geo->forceRenameAttribute(outAttribPtr, outAttribPtr->getName());
        }

        return true;
    }



template<GFE_OutArrayType _OutArrayType>
void connectivity()
{
    GU_DetailHandle geoTmp_h;
    GU_Detail* geoTmp;
    if constexpr (_OutArrayType == GFE_OutArrayType::Attrib)
    {
        geoTmp = geo->asGU_Detail();
    }
    else
    {
        geoTmp_h = GFE_DetailBase::newDetail(geo);
        geoTmp = geoTmp_h.gdpNC();
    }
        
    GFE_MeshTopology meshTopology(geoTmp, cookparms);
    meshTopology.outAsOffset = false;
    meshTopology.outArrayType = _OutArrayType;
    meshTopology.outIntermediateAttrib = true;
    
    if (connectivityConstraint)
    {
        meshTopology.groupParser.setGroup(groupParser.getPrimitiveGroup());
        meshTopology.setPrimPrimEdge(true);
        meshTopology.compute();
        if constexpr (_OutArrayType == GFE_OutArrayType::Attrib)
            adjElemsAttrib_h.bind(meshTopology.getPrimPrimEdge());
    }
    else
    {
        meshTopology.groupParser.setGroup(groupParser.getPointGroup());
        meshTopology.setPointPointEdge(true);
        meshTopology.compute();
        if constexpr (_OutArrayType == GFE_OutArrayType::Attrib)
            adjElemsAttrib_h.bind(meshTopology.getPointPointEdge());
    }

#if __GFE_Connectivity_OutTime
    double timeTotal0 = 0;
    double timeTotal1 = 0;
    auto timeStart = std::chrono::steady_clock::now();
    auto timeEnd   = std::chrono::steady_clock::now();
    timeStart = std::chrono::steady_clock::now();
    timeEnd   = std::chrono::steady_clock::now();
    std::chrono::duration<double> diff;
#endif

    const GA_AttributeOwner connectivityOwner = connectivityConstraint ? GA_ATTRIB_PRIMITIVE : GA_ATTRIB_POINT;
    const GA_Size nelems = connectivityConstraint ? geo->getNumPrimitives() : geo->getNumPoints();

    ::std::vector<GA_Offset> elemHeap;
    elemHeap.reserve(((1 << 11) - 1) & nelems); //SYSmin(1 << 10, nelems)
        
    ::std::vector<GA_Offset> classnumArray(nelems, GFE_INVALID_OFFSET);
        
        
    const GA_Size numPrims = geoTmp->getNumPrimitives();

        
    if constexpr (_OutArrayType == GFE_OutArrayType::Attrib)
    {
        UT_PackedArrayOfArrays<GA_Offset> packedArray;
        
        packedArray.setDataCapacityIfNeeded(numPrims << 2);
        packedArray.setArrayCapacityIfNeeded(numPrims);
        adjElemsAttrib_h->getAIFNumericArray()->getPackedArrayFromIndices(adjElemsAttrib_h.getAttribute(), 0, nelems, packedArray);

        
        const UT_Array<exint>& rawOffsets = packedArray.rawOffsets();
        const UT_Array<GA_Offset>& rawData = packedArray.rawData();

        
#if __GFE_Connectivity_OutTime
        timeEnd = std::chrono::steady_clock::now();
        diff = timeEnd - timeStart;
        timeTotal0 += diff.count();
        timeStart = std::chrono::steady_clock::now();
#endif
        
        GA_Index classnum = startClassnum;
        for (GA_Size elemoff = 0; elemoff < nelems; ++elemoff)
        {
            if (classnumArray[elemoff] != GFE_INVALID_OFFSET)
                continue;
            classnumArray[elemoff] = classnum;
            elemHeap.emplace_back(elemoff);
            while (!elemHeap.empty())
            {
                const GA_Size lastElem = elemHeap[elemHeap.size() - 1];
                elemHeap.pop_back();
                const GA_Size rawOffsetEnd = rawOffsets[lastElem + 1];
                for (GA_Size i = rawOffsets[lastElem]; i < rawOffsetEnd; ++i)
                {
                    const GA_Offset rawDataVal = rawData[i];
                    if (classnumArray[rawDataVal] != GFE_INVALID_OFFSET)
                        continue;
                    classnumArray[rawDataVal] = classnum;
                    elemHeap.emplace_back(rawDataVal);
                }
            }
            ++classnum;
        }
        
    }
    else
    {
        constexpr int Method = 0;
        
        GFE_PackedOffsetArrayOfArrays packedArray;
        const size_t capacity = numPrims+1;
        
        packedArray.reserveOffsets(capacity);
        packedArray.emplace0();
        if constexpr (Method != 0)
            packedArray.reserveData(numPrims << 2);
            
        
        GA_PrimitiveList& primList = geoTmp->getPrimitiveList();

        
        if constexpr (Method == 0)
        {
            UTparallelFor(GA_SplittableRange(geoTmp->getPrimitiveRange()), [&packedArray, &primList, geoTmp](const GA_SplittableRange& r)
            {
                GA_Offset start, end;
                //for (GA_PageIterator pit = r.beginPages(); !pit.atEnd(); ++pit)
                //{
                    for (GA_Iterator it(r.begin()); it.blockAdvance(start, end); )
                    {
                        for (GA_Offset elemoff = start; elemoff < end; ++elemoff)
                        {
                            UT_ASSERT_P(elemoff == geoTmp->primitiveIndex(elemoff));
                            UT_ASSERT_P(elemoff+1 < packedArray.offsets().capacity());
                            const GA_Size numvtx = primList.getVertexCount(elemoff);
                            packedArray.offsets()[elemoff+1] = numvtx;
                        }
                    }
                //}
            }, subscribeRatio, minGrainSize);
            
#if __GFE_Connectivity_OutTime
            //geo->asGEO_Detail()->setDetailAttributeF("packedArraySize", packedArray.offsets().size());
#endif
            
            GA_Offset elemoff_prev = packedArray.offsets()[0];
            for (GA_Offset i = 1; i < capacity; ++i)
            {
                elemoff_prev += packedArray.offsets()[i];
                packedArray.offsets()[i] = elemoff_prev;
            }
            
            packedArray.data().reserve(packedArray.offsets()[numPrims]);
            UTparallelFor(GA_SplittableRange(geoTmp->getPrimitiveRange()), [&packedArray, &primList, geoTmp](const GA_SplittableRange& r)
            {
                GA_Offset start, end;
                //for (GA_PageIterator pit = r.beginPages(); !pit.atEnd(); ++pit)
                //{
                    for (GA_Iterator it(r.begin()); it.blockAdvance(start, end); )
                    {
                        for (GA_Offset elemoff = start; elemoff < end; ++elemoff)
                        {
                            const GA_OffsetListRef& vertices = primList.getVertexList(elemoff);
                            
                            const GA_Offset start_off = packedArray.offsets()[elemoff];
                            const GA_Offset end_off = packedArray.offsets()[elemoff+1];
                            const GA_Offset len = end_off - start_off;
                            UT_ASSERT_P(len == vertices.size());
                            for (GA_Offset i = 0; i < len; ++i)
                            {
                                packedArray.data()[i+start_off] = vertices[i];
                            }
                        }
                    }
                //}
            }, subscribeRatio, minGrainSize);
#if 0 && __GFE_Connectivity_OutTime
            for (size_t i = 0; i < SYSmin(capacity, size_t(20)); ++i)
            {
                UT_WorkBuffer firstname("packedArrayLastValue");
                UT_WorkBuffer istr;
                istr.sprintf("%d", i);
                firstname += istr;
                geo->asGEO_Detail()->setDetailAttributeF(firstname, packedArray.data()[i]);
            }
#endif


            
        }
        else if constexpr (Method == 1)
        {
#if __GFE_Connectivity_OutTime
            GA_Offset isTrivialTimes = 0;
#endif
            for (GA_Offset elemoff = 0; elemoff < numPrims; ++elemoff)
            {
                UT_ASSERT_P(GFE_Type::isValidOffset(geoTmp->getPrimitiveMap(), elemoff));
                const GA_OffsetListRef& vertices = primList.getVertexList(elemoff);
                packedArray.emplace_back_offsetList_unsafe(vertices);
#if __GFE_Connectivity_OutTime
                isTrivialTimes += vertices.isTrivial();
#endif
            }
            
#if __GFE_Connectivity_OutTime
            geo->asGEO_Detail()->setDetailAttributeF("isTrivialTimes", isTrivialTimes);
#endif
        }
        else
        {
            GA_PrimitiveList& primList = geoTmp->getPrimitiveList();
            
            if (primList.isFullRepresentation())
            {
                for (GA_Offset elemoff = 0; elemoff < numPrims; ++elemoff)
                {
                    const GA_OffsetListRef& vertices = primList.getVertexList(elemoff);
                    packedArray.emplace_back_offsetList_unsafe(vertices);
                }
            }
            else
            {
                const GA_PageNum pagenumMax = GAgetPageNum(numPrims);
                const GA_PageNum pageoffMax = GAgetPageOff(numPrims);
                for (GA_PageNum pagenum = 0; pagenum < pagenumMax; ++pagenum)
                {
                    const GA_OffsetListRef* const vertexListPage = primList.getConstantVertexListPage(pagenum);
                    const bool isVertexListPageConstant = primList.isVertexListPageConstant(pagenum);
                    if (isVertexListPageConstant)
                    {
                        for (GA_Offset pageoff = 0; pageoff < GA_PAGE_SIZE; ++pageoff)
                        {
                            const exint size = vertexListPage->size();
                            const GA_OffsetListRef& vertices = GA_OffsetListRef(vertexListPage->trivialStart() + size*pageoff, size, vertexListPage->getExtraFlag());
                            packedArray.emplace_back_offsetList_unsafe(vertices);
                        }
                    }
                    else
                    {
                        for (GA_Offset pageoff = 0; pageoff < GA_PAGE_SIZE; ++pageoff)
                        {
                            const GA_OffsetListRef& vertices = vertexListPage[pageoff];
                            packedArray.emplace_back_offsetList_unsafe(vertices);
                        }
                    }
                }
                const GA_OffsetListRef* const vertexListPage = primList.getConstantVertexListPage(pagenumMax);
                const bool isVertexListPageConstant = primList.isVertexListPageConstant(pagenumMax);
                for (GA_Offset pageoff = 0; pageoff < pageoffMax; ++pageoff)
                {
                    if (isVertexListPageConstant)
                    {
                        exint size = vertexListPage->size();
                        const GA_OffsetListRef& vertices = GA_OffsetListRef(vertexListPage->trivialStart() + size*pageoff, size, vertexListPage->getExtraFlag());
                        packedArray.emplace_back_offsetList_unsafe(vertices);
                    }
                    else
                    {
                        const GA_OffsetListRef& vertices = vertexListPage[pageoff];
                        packedArray.emplace_back_offsetList_unsafe(vertices);
                    }
                }
            }
        }

        const ::std::vector<exint>& rawOffsets = packedArray.offsets();
        const ::std::vector<GA_Offset>& rawData = packedArray.data();
            
#if __GFE_Connectivity_OutTime
        timeEnd = std::chrono::steady_clock::now();
        diff = timeEnd - timeStart;
        timeTotal0 += diff.count();
        timeStart = std::chrono::steady_clock::now();

#endif


        //if constexpr (_OutArrayType == GFE_OutArrayType::Attrib)
        if constexpr (1)
        {
            GA_Index classnum = startClassnum;
            for (GA_Size elemoff = 0; elemoff < nelems; ++elemoff)
            {
                if (classnumArray[elemoff] != GFE_INVALID_OFFSET)
                    continue;
                classnumArray[elemoff] = classnum;
                elemHeap.emplace_back(elemoff);
                while (!elemHeap.empty())
                {
                    const GA_Size lastElem = elemHeap[elemHeap.size() - 1];
                    elemHeap.pop_back();
                    const GA_Size rawOffsetEnd = rawOffsets[lastElem + 1];
                    for (GA_Size i = rawOffsets[lastElem]; i < rawOffsetEnd; ++i)
                    {
                        const GA_Offset rawDataVal = rawData[i];
                        if (classnumArray[rawDataVal] != GFE_INVALID_OFFSET)
                            continue;
                        classnumArray[rawDataVal] = classnum;
                        elemHeap.emplace_back(rawDataVal);
                    }
                }
                ++classnum;
            }
        }
        else
        {
            //size_t len = 0;
            GA_Index classnum = startClassnum;
            for (GA_Size elemoff = 0; elemoff < nelems; ++elemoff)
            {
                if (classnumArray[elemoff] != GFE_INVALID_OFFSET)
                    continue;
                classnumArray[elemoff] = classnum;
                elemHeap.emplace_back(elemoff);
                while (!elemHeap.empty())
                {
                    const GA_Size lastElem = elemHeap[elemHeap.size() - 1];
                    elemHeap.pop_back();

                    UT_ASSERT_P(GFE_Type::isValidOffset(geoTmp->getPrimitiveMap(), lastElem));
                    
                    const GA_OffsetListRef& vertices = geoTmp->getPrimitiveVertexList(lastElem);
                    if (vertices.isTrivial())
                    {
                        const GA_Offset trivialStart = vertices.trivialStart();
                        const GA_Size numvtx = trivialStart + vertices.size();
                        for (GA_Size rawDataVal = trivialStart; rawDataVal < numvtx; ++rawDataVal)
                        {
                            UT_ASSERT_P(rawDataVal != GFE_INVALID_OFFSET && rawDataVal < classnumArray.size());
                            if (classnumArray[rawDataVal] != GFE_INVALID_OFFSET)
                                continue;
                            classnumArray[rawDataVal] = classnum;
                            //elemHeap[len++] = rawDataVal;
                            elemHeap.emplace_back(rawDataVal);
                        }
                    }
                    else
                    {
                        const exint* const data = vertices.getArray();
                        const GA_Size numvtx = vertices.size();
                        for (GA_Size i = 0; i < numvtx; ++i)
                        {
                            const GA_Offset rawDataVal = data[i];
                            UT_ASSERT_P(rawDataVal != GFE_INVALID_OFFSET && rawDataVal < classnumArray.size());
                            if (classnumArray[rawDataVal] != GFE_INVALID_OFFSET)
                                continue;
                            classnumArray[rawDataVal] = classnum;
                            //elemHeap[len++] = rawDataVal;
                            elemHeap.emplace_back(rawDataVal);
                        }
                    }
                }
                ++classnum;
            }
        }

    }
        
    // if (connectivityConstraint)
    // {
    //     if (rawData.size() < pow(2, 12))
    //     {
    //         for (GA_Offset elemoff = 0; elemoff < rawData.size(); ++elemoff)
    //         {
    //             rawData[elemoff] = geo->primitiveIndex(rawData[elemoff]);
    //         }
    //     }
    //     else
    //     {
    //         UTparallelFor(UT_BlockedRange<GA_Size>(0, rawData.size()), [this, &rawData](const UT_BlockedRange<GA_Size>& r) {
    //             for (GA_Offset elemoff = r.begin(); elemoff != r.end(); ++elemoff)
    //             {
    //                 rawData[elemoff] = geo->primitiveIndex(rawData[elemoff]);
    //             }
    //         }, 16, 1024);
    //     }
    // }
    // else
    // {
    //     if (rawData.size() < pow(2, 12))
    //     {
    //         for (GA_Offset elemoff = 0; elemoff < rawData.size(); ++elemoff)
    //         {
    //             rawData[elemoff] = geo->pointIndex(rawData[elemoff]);
    //         }
    //     }
    //     else
    //     {
    //         UTparallelFor(UT_BlockedRange<GA_Size>(0, rawData.size()), [this, &rawData](const UT_BlockedRange<GA_Size>& r) {
    //             for (GA_Offset elemoff = r.begin(); elemoff != r.end(); ++elemoff)
    //             {
    //                 rawData[elemoff] = geo->pointIndex(rawData[elemoff]);
    //             }
    //         }, 16, 1024);
    //     }
    // }


#if __GFE_Connectivity_OutTime
    timeEnd = std::chrono::steady_clock::now();
    diff = timeEnd - timeStart;
    timeTotal1 += diff.count();
    timeStart = std::chrono::steady_clock::now();

    geo->asGEO_Detail()->setDetailAttributeF("time0", timeTotal0 * 1000);
    geo->asGEO_Detail()->setDetailAttributeF("time1", timeTotal1 * 1000);
#endif
        
        
    GA_Attribute* const connectivityAttrib = this->connectivityAttrib;
    const GA_IndexMap& indexMap = geo->getIndexMap(connectivityOwner);
    UTparallelFor(groupParser.getSplittableRange(connectivityOwner),
        [&indexMap, &classnumArray, connectivityAttrib](const GA_SplittableRange& r)
    {
        GA_PageHandleScalar<GA_Offset>::RWType attrib_ph(connectivityAttrib);
        for (GA_PageIterator pit = r.beginPages(); !pit.atEnd(); ++pit)
        {
            GA_Offset start, end;
            for (GA_Iterator it(pit.begin()); it.blockAdvance(start, end); )
            {
                attrib_ph.setPage(start);
                for (GA_Offset elemoff = start; elemoff < end; ++elemoff)
                {
                    attrib_ph.value(elemoff) = classnumArray[indexMap.indexFromOffset(elemoff)];
                }
            }
        }
    }, 16, 1024);
    //geo->setAttributeFromArray(attrib_h.getAttribute(), GA_Range(), UT_Array<GA_Size>(classnumArray));

}


//#undef GFE_TEMP_ConnectivityAttribName
    
public:
    gfe::GroupParser groupParserSeam;
    GA_Size startClassnum = 0;
    bool connectivityConstraint = false; // false means point  and  true means edge 
    //GA_AttributeOwner connectivityOwner = GA_ATTRIB_PRIMITIVE;
    // bool outAsOffset = true;
    bool computeTopologyAttrib = false;
    
private:
    GA_Attribute* connectivityAttrib;
    GA_ROHandleT<UT_ValArray<GA_Offset>> adjElemsAttrib_h;

    exint subscribeRatio = 64;
    exint minGrainSize   = 1024;

    //const char* GFE_TEMP_ConnectivityAttribName = "__TEMP_GFE_ConnectivityAttrib";

#undef __GFE_Connectivity_OutTime
    
}; // End of class Connectivity
_GFEL_END
#endif
