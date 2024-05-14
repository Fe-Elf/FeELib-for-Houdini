
#pragma once

#ifndef __GFE_OffsetArrayAttributeToGeometry_h__
#define __GFE_OffsetArrayAttributeToGeometry_h__

#include <GFE/GFE_OffsetArrayAttributeToGeometry.h>

#include <GU/GU_PackedGeometry.h>

#include <GFE/GeometryBase.h>

_GFE_BEGIN
namespace OffsetArrayAttribToGeo {

    
    static GU_DetailHandle offsetArrayAttribToGeo(
        const GA_Attribute& attrib,
        const GA_ElementGroup* const group = nullptr, 
        exint subscribeRatio = 64,
        exint minGrainSize   = 1024
    )
    {
        UT_ASSERT_P(attrib.getAIFNumericArray());
        UT_ASSERT_P(attrib.getStorageClass() == GA_STORECLASS_INT);
        UT_ASSERT_P(attrib.getTupleSize() == 1);
        UT_ASSERT_P(!group || attrib.getOwner() == group->getOwner());
        UT_ASSERT_P(!group || group->entries() == group->computeGroupEntries());
        
        //const GA_AIFNumericArray* const aIFNumericArray = pointPointEdgeAttrib->getAIFNumericArray();
        const GFE_Detail& geo = reinterpret_cast<const GFE_Detail&>(attrib.getDetail());
        const GA_AttributeOwner owner = attrib.getOwner();
        const GA_IndexMap& indexMap = geo.getIndexMap(owner);
        const GA_ROHandleT<UT_ValArray<GA_Offset>> intArray_oh(&attrib);
        
        GU_DetailHandle geoTmp_h = gfe::GeoBase::newDetail();
        GU_Detail* const geoTmp = geoTmp_h.gdpNC();
        
        geoTmp->appendPrimitiveBlock(GEO_PRIMNONE, group ? group->entries() : geo.getNumElements(owner));
        GA_PrimitiveList& primList = geoTmp->getPrimitiveList();
        const bool isFullRepresentation = primList.isFullRepresentation();

        if (group && !isFullRepresentation)
        {
            const GA_OffsetList groupOffList = geo.getOffsetList(indexMap, group);
            const GA_SplittableRange srange = geoTmp->getPrimitiveRange();
            UTparallelFor(srange, [&primList, &intArray_oh, &groupOffList](const GA_SplittableRange& r)
            {
                UT_ValArray<GA_Offset> offArray(16);
                //GA_PageHandleScalar<GA_Offset>::RWType attrib_ph(attrib);
                GA_Offset start, end;
                for (GA_PageIterator pit = r.beginPages(); !pit.atEnd(); ++pit)
                {
                    for (GA_Iterator it(pit.begin()); it.blockAdvance(start, end); )
                    {
                        UT_ASSERT_P(GAgetPageNum(start) == GAgetPageNum(end));
                        
                        const GA_PageNum pageNum = GAgetPageNum(start);
                        auto po = primList.hardenVertexListPage(pageNum);
                        
                        for (GA_Offset elemoff = start; elemoff < end; ++elemoff)
                        {
                            intArray_oh.get(groupOffList[elemoff], offArray);
                            //intArray_oh.get(indexMap.offsetFromIndex(elemoff), offArray);
                            GA_OffsetList offList(offArray);
                            //UT_ASSERT_P(GAisValid(elemoff) && elemoff < primList.myVertexLists.size());
                            const GA_PageOff pageoff = GAgetPageOff(elemoff);
                            po[pageoff] = offList;
                        }
                    }
                }
            }, subscribeRatio, minGrainSize);
        }
        else
        {
            const GA_SplittableRange srange = GA_Range(indexMap, group);
            UTparallelFor(srange, [&primList, &indexMap, &intArray_oh](const GA_SplittableRange& r)
            {
                //GA_OffsetListRef vertices;
                UT_ValArray<GA_Offset> offArray(16);
                GA_Offset start, end;
                for (GA_Iterator it(r); it.blockAdvance(start, end); )
                {
                    for (GA_Offset elemoff = start; elemoff < end; ++elemoff)
                    {
                        intArray_oh.get(elemoff, offArray);
                        GA_OffsetList offList(offArray);
                        primList.setVertexList(indexMap.indexFromOffset(elemoff), offList);
                    }
                }
            }, subscribeRatio, minGrainSize);
            
        }
        
        return geoTmp_h;
    }


    SYS_FORCE_INLINE static GU_PrimPacked* offsetArrayAttribToPacked(
        GA_Detail& dstGeo,
        const GA_Attribute& attrib,
        const GA_ElementGroup* const group = nullptr, 
        exint subscribeRatio = 64,
        exint minGrainSize   = 1024
    )
    {
        const GU_DetailHandle geoTmp_h = offsetArrayAttribToGeo(attrib, group, subscribeRatio, minGrainSize);
        return GU_PackedGeometry::packGeometry(reinterpret_cast<GU_Detail&>(dstGeo), geoTmp_h);
    }


    SYS_FORCE_INLINE static GU_PrimPacked* offsetArrayAttribToPacked(
        GA_Detail* const dstGeo,
        const GA_Attribute* const attrib,
        const GA_ElementGroup* const group = nullptr, 
        exint subscribeRatio = 64,
        exint minGrainSize   = 1024
    )
    {
        UT_ASSERT_P(dstGeo);
        UT_ASSERT_P(attrib);
        return offsetArrayAttribToPacked(*dstGeo, *attrib, group, subscribeRatio, minGrainSize);
    }


} // End of namespace OffsetArrayAttribToGeo
_GFE_END
#endif
