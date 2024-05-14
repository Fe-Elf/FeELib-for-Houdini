
#pragma once

#ifndef __GFE_GeometryBase_h__
#define __GFE_GeometryBase_h__

#include <GFE/GeometryBase.h>

#include <GA/GA_Detail.h>
#include <GFE/Type.h>

_GFE_BEGIN
namespace GeoBase {





    
#if 0
        GU_DetailHandle geoTemp_h;
        GU_Detail* geoTemp;
        geoTemp_h = GFE_DetailBase::newDetail(geo);
        geoTemp = geoTemp_h.gdpNC();
        
        GU_DetailHandle geoTemp_h = GFE_DetailBase::newDetail(geo);
        GU_Detail* const geoTemp  = geoTemp_h.gdpNC();
#endif
    static GU_DetailHandle newDetail()
    {
        GU_Detail* const geo = new GU_Detail;
        GU_DetailHandle geo_h;
        geo_h.allocateAndSet(geo);
        return geo_h;
    }
    
    static GU_DetailHandle newDetail(const GA_Detail& geoSrc)
    {
        GU_DetailHandle geo_h = newDetail();
        geo_h.gdpNC()->replaceWith(geoSrc);
        return geo_h;
    }


    SYS_FORCE_INLINE static GU_DetailHandle newDetail(const GA_Detail* const geoSrc)
    { UT_ASSERT_P(geoSrc); return newDetail(*geoSrc); }







    
    static GA_Size vertexPrimIndex(const GA_OffsetListRef& vertices, const GA_Offset vtxoff)
    {
        const GA_Size numvtx = vertices.size();
        for (GA_Size vtxpnum = 0; vtxpnum < numvtx; ++vtxpnum)
        {
            if (vertices[vtxpnum] == vtxoff)
                return vtxpnum;
        }
        return gfe::INVALID_OFFSET;
    }
    
    SYS_FORCE_INLINE static GA_Size vertexPrimIndex(const GA_PrimitiveList& primList, const GA_Offset primoff, const GA_Offset vtxoff)
    { return vertexPrimIndex(primList.getVertexList(primoff), vtxoff); }
    
    SYS_FORCE_INLINE static GA_Size vertexPrimIndex(const GA_Detail& geo, const GA_Offset primoff, const GA_Offset vtxoff)
    { return vertexPrimIndex(geo.getPrimitiveVertexList(primoff), vtxoff); }
    
    SYS_FORCE_INLINE static GA_Size vertexPrimIndex(const GA_Detail& geo, const GA_Offset vtxoff)
    { return vertexPrimIndex(geo, geo.vertexPrimitive(vtxoff), vtxoff); }

        
    static GA_Offset vertexNext(const GA_Detail& geo, const GA_Offset primoff, const GA_Size vtxpnum)
    {
        const GA_OffsetListRef& vertices = geo.getPrimitiveVertexList(primoff);
        const GA_Size vtxpnum_next = vtxpnum+1;
        const bool isEnd = vtxpnum_next == vertices.size();
        return vertices.isClosed() || !isEnd
               ? vertices[isEnd ? 0 : vtxpnum_next]
               : gfe::INVALID_OFFSET;
    }
    
    static GA_Offset vertexPointDst(const GA_Detail& geo, const GA_Offset primoff, const GA_Size vtxpnum)
    {
        const GA_OffsetListRef& vertices = geo.getPrimitiveVertexList(primoff);
        const GA_Size vtxpnum_next = vtxpnum+1;
        const bool isEnd = vtxpnum_next == vertices.size();
        return vertices.isClosed() || !isEnd
               ? geo.vertexPoint(vertices[isEnd ? 0 : vtxpnum_next])
               : gfe::INVALID_OFFSET;
    }

    SYS_FORCE_INLINE static GA_Offset vertexNext(const GA_Detail& geo, const GA_Offset vtxoff)
    {
        const GA_Offset primoff = geo.vertexPrimitive(vtxoff);
        return vertexNext(geo, primoff, vertexPrimIndex(geo, primoff, vtxoff));
    }
    
    SYS_FORCE_INLINE static GA_Offset vertexPointDst(const GA_Detail& geo, const GA_Offset vtxoff)
    {
        const GA_Offset primoff = geo.vertexPrimitive(vtxoff);
        return vertexPointDst(geo, primoff, vertexPrimIndex(geo, primoff, vtxoff));
    }



    static GA_Offset edgeVertexSub(const GA_Detail& geo, const GA_Offset ptoff0, const GA_Offset ptoff1)
    {
        GA_Offset primPoint_next;
        for (GA_Offset vtxoff = geo.pointVertex(ptoff0); gfe::isValidOffset(vtxoff); vtxoff = geo.vertexToNextVertex(vtxoff))
        {
            const GA_Offset primoff = geo.vertexPrimitive(vtxoff);
            const GA_OffsetListRef& vertices = geo.getPrimitiveVertexList(primoff);
            const GA_Size numvtx = vertices.size();
            for (GA_Size vtxpnum = 0; vtxpnum < numvtx; ++vtxpnum)
            {
                if (vertices[vtxpnum] != vtxoff)
                    continue;
                
                GA_Size vtxpnum_next = vtxpnum+1;
                if (vtxpnum_next != numvtx || vertices.isClosed())
                {
                    if (vtxpnum_next == numvtx)
                        vtxpnum_next = 0;
                    primPoint_next = geo.vertexPoint(vertices[vtxpnum_next]);
                    if (primPoint_next == ptoff1)
                        return vtxoff;
                }
                
                if (vtxpnum != 0 || vertices.isClosed())
                {
                    vtxpnum_next = vtxpnum==0 ? numvtx-1 : vtxpnum-1;
                    primPoint_next = geo.vertexPoint(vertices[vtxpnum_next]);
                    if (primPoint_next == ptoff1)
                        return vertices[vtxpnum_next];
                }
            }
        }
        return gfe::INVALID_OFFSET;
    }

    static GA_Offset edgeVertex(const GA_Detail& geo, const GA_Offset ptoff0, const GA_Offset ptoff1)
    {
        GA_Offset result = edgeVertexSub(geo, ptoff0, ptoff1);
        return gfe::isValidOffset(result) ? edgeVertexSub(geo, ptoff1, ptoff0) : result;
    }

    SYS_FORCE_INLINE static GA_Offset edgeVertex(const GA_Detail& geo, const GA_Edge& edge)
    { return edgeVertex(geo, edge.p0(), edge.p1()); }










SYS_FORCE_INLINE static GA_Offset getFirstElement(const GA_IndexMap& indexMap)
{ return indexMap.offsetSize() > 0 ? indexMap.offsetFromIndex(0) : gfe::INVALID_OFFSET; }

// SYS_FORCE_INLINE static GA_Offset getFirstElement(const GA_IndexMap& indexMap, const GA_ElementGroup* const group)
// { return group ? GFE_Group::getFirstElement(*group) : getFirstElement(indexMap); }

SYS_FORCE_INLINE static GA_Offset getFirstElement(const GA_Detail& geo, const GA_AttributeOwner owner)
{ return getFirstElement(geo.getIndexMap(owner)); }

template<GA_AttributeOwner _Owner>
SYS_FORCE_INLINE static GA_Offset getFirstElement(const GA_Detail& geo)
{ return getFirstElement(geo.getIndexMap(_Owner)); }

template<GA_AttributeOwner _Owner>
SYS_FORCE_INLINE static GA_Offset getFirstElement(const GA_Detail* const geo)
{ UT_ASSERT_P(geo); return getFirstElement<_Owner>(*geo); }








    
} // End of namespace GeoBase
_GFE_END



#endif
