
#pragma once

#ifndef __GFE_Geometry_h__
#define __GFE_Geometry_h__

#include <GFE/Geometry.h>

#include <GA/GA_Detail.h>
#include <GA/GA_PageHandle.h>
#include <GA/GA_PageIterator.h>

//#include <GEO/GEO_Primitive.h>
//#include <GU/GU_Primitive.h>
#include <GU/GU_PrimPoly.h>

#include <GFE/Variant.h>
#include <GFE/Group.h>
#include <GFE/GroupUnion.h>
#include <GFE/AttributeDelete.h>
#include <GFE/GeometryBase.h>

class GEO_PrimPoly;
class GU_PrimPoly;
class GEO_PrimVDB;
class GU_PrimVDB;
class GEO_PrimVolume;
class GU_PrimVolume;

#define __GFE_Geometry_Inherited_From_GU_Detail 1


_GFE_BEGIN
#ifdef __GFE_Geometry_Inherited_From_GU_Detail
    class Geometry : public GU_Detail {
    public:
        using GU_Detail::GU_Detail;
#else
    class Geometry : public GA_Detail {
    public:
        using GA_Detail::GA_Detail;
#endif


public:
    
    SYS_FORCE_INLINE bool hasPrimitive() const
    { return getNumPrimitives() >= 0; }
    //{ return getPrimitiveMap().isOffsetInRange(getPrimitiveMap().indexFromOffset(0)); }

    SYS_FORCE_INLINE bool hasPoint() const
    { return getNumPoints() >= 0; }

    SYS_FORCE_INLINE bool hasVertex() const
    { return getNumVertices() >= 0; }


    
    SYS_FORCE_INLINE bool isPoly(const GA_Offset primoff) const
    { return gfe::isPoly(getPrimitiveTypeId(primoff)); }
    
    SYS_FORCE_INLINE bool isHoudiniVolume(const GA_Offset primoff) const
    { return gfe::isHoudiniVolume(getPrimitiveTypeId(primoff)); }
    
    SYS_FORCE_INLINE bool isVDB(const GA_Offset primoff) const
    { return gfe::isVDB(getPrimitiveTypeId(primoff)); }
    
    SYS_FORCE_INLINE bool isVolume(const GA_Offset primoff) const
    { return isHoudiniVolume(primoff) || isVDB(primoff); }



    

    SYS_FORCE_INLINE GA_Offset getFirstElement(const GA_AttributeOwner owner) const
    { return gfe::GeoBase::getFirstElement(getIndexMap(owner)); }

    template<GA_AttributeOwner _Owner>
    SYS_FORCE_INLINE GA_Offset getFirstElement() const
    { return gfe::GeoBase::getFirstElement(getIndexMap(_Owner)); }

    SYS_FORCE_INLINE GA_Offset getFirstElement(const GA_PrimitiveGroup* const geoPrimGroup) const
    {
        const GA_Offset elemoff = geoPrimGroup
                                ? gfe::Group::getFirstElement(*geoPrimGroup)
                                : gfe::GeoBase::getFirstElement<GA_ATTRIB_PRIMITIVE>(this);
        
        return isInvalidOffset<GA_ATTRIB_PRIMITIVE>(elemoff) ? gfe::INVALID_OFFSET : elemoff;
    }

    
    SYS_FORCE_INLINE GA_Offset getFirstVolumeoff(const GA_PrimitiveGroup* const geoPrimGroup) const
    {
        const GA_Offset volumeoff = getFirstElement(geoPrimGroup);
        return isVolume(volumeoff) ? volumeoff : gfe::INVALID_OFFSET;
    }

    SYS_FORCE_INLINE GA_Offset getFirstHoudiniVolumeoff(const GA_PrimitiveGroup* const geoPrimGroup) const
    {
        const GA_Offset volumeoff = getFirstElement(geoPrimGroup);
        return isHoudiniVolume(volumeoff) ? volumeoff : gfe::INVALID_OFFSET;
    }

    SYS_FORCE_INLINE void getVectorHoudiniVolumePrimitive(GEO_PrimVolume const ** vvol, const GA_PrimitiveGroup* const geoPrimGroup) const
    {
        int8 idx = 0;
        const GEO_Primitive* prim;
	    GA_FOR_ALL_OPT_GROUP_PRIMITIVES(this, geoPrimGroup, prim)
	    {
			if (prim->getTypeId() == GEO_PRIMVOLUME)
			{
			    vvol[idx++] = reinterpret_cast<const GEO_PrimVolume*>(prim);

			    if (idx == 3)
					break;
			}
	    }
    }


    
#if 0
    const GA_Offset vdboff = geo->getFirstVDBoff(groupParser.getPrimitiveGroup());
#endif
    SYS_FORCE_INLINE GA_Offset getFirstVDBoff(const GA_PrimitiveGroup* const geoPrimGroup) const
    {
        const GA_Offset vdboff = getFirstElement(geoPrimGroup);
        return isVDB(vdboff) ? vdboff : gfe::INVALID_OFFSET;
    }

    
    SYS_FORCE_INLINE const GEO_PrimVolume* getFirstHoudiniVolumePrimitive(const GA_PrimitiveGroup* const geoPrimGroup) const
    {
        const GA_Offset voloff = getFirstHoudiniVolumeoff(geoPrimGroup);
        return getPrimitiveT<const GEO_PrimVolume*>(voloff);
    }


    
    template<GA_AttributeOwner _Owner>
    SYS_FORCE_INLINE bool isInvalidOffset(const GA_Offset elemoff) const
    { return gfe::isInvalidOffset(getIndexMap(_Owner), elemoff); }

    SYS_FORCE_INLINE bool isInvalidOffset(const GA_AttributeOwner owner, const GA_Offset elemoff) const
    { return gfe::isInvalidOffset(getIndexMap(owner), elemoff); }

    
    SYS_FORCE_INLINE void setValidPosAttrib(GA_Attribute*& attrib)
    { if (gfe::isInvalidPosAttrib(attrib)) attrib = getP(); }
    
    SYS_FORCE_INLINE void setValidPosAttrib(const GA_Attribute*& attrib)
    { if (gfe::isInvalidPosAttrib(attrib)) attrib = getP(); }

    SYS_FORCE_INLINE void setValidPosAttrib(const GA_Attribute*& attrib) const
    { if (gfe::isInvalidPosAttrib(attrib)) attrib = getP(); }

     
    SYS_FORCE_INLINE static void setValidPosAttrib(GA_Detail* const geo, GA_Attribute*& attrib)
    { if (gfe::isInvalidPosAttrib(attrib)) attrib = geo->getP(); }
    
    SYS_FORCE_INLINE static void setValidPosAttrib(GA_Detail* const geo, const GA_Attribute*& attrib)
    { if (gfe::isInvalidPosAttrib(attrib)) attrib = geo->getP(); }

    SYS_FORCE_INLINE static void setValidPosAttrib(const GA_Detail* const geo, const GA_Attribute*& attrib)
    { if (gfe::isInvalidPosAttrib(attrib)) attrib = geo->getP(); }



    
//#define _GFE_Specialization(class)\

    template<typename _TClass>
    SYS_FORCE_INLINE _TClass getPrimitiveT(const GA_Offset primoff)
    {
        UT_ASSERT_P(gfe::isValidOffset(getPrimitiveMap(), primoff));
        if constexpr      (std::is_same_v<_TClass, GEO_PrimPoly*>   || std::is_same_v<_TClass, GU_PrimPoly*>)
            UT_ASSERT_P(isPoly(primoff));
        else if constexpr (std::is_same_v<_TClass, GEO_PrimVolume*> || std::is_same_v<_TClass, GU_PrimVolume*>)
            UT_ASSERT_P(isVolume(primoff));
        else if constexpr (std::is_same_v<_TClass, GEO_PrimVDB*>    || std::is_same_v<_TClass, GU_PrimVDB*>)
            UT_ASSERT_P(isVDB(primoff));
        
        return reinterpret_cast<_TClass>(getPrimitive(primoff));
    }
    
    template<typename _TClass>
    SYS_FORCE_INLINE _TClass getPrimitiveT(const GA_Offset primoff) const
    {
        UT_ASSERT_P(gfe::isValidOffset(getPrimitiveMap(), primoff));
        if constexpr      (std::is_same_v<std::remove_const_t<_TClass>, GEO_PrimPoly*>   || std::is_same_v<std::remove_const_t<_TClass>, GU_PrimPoly*>)
            UT_ASSERT_P(isPoly(primoff));
        else if constexpr (std::is_same_v<std::remove_const_t<_TClass>, GEO_PrimVolume*> || std::is_same_v<std::remove_const_t<_TClass>, GU_PrimVolume*>)
            UT_ASSERT_P(isVolume(primoff));
        else if constexpr (std::is_same_v<std::remove_const_t<_TClass>, GEO_PrimVDB*>    || std::is_same_v<std::remove_const_t<_TClass>, GU_PrimVDB*>)
            UT_ASSERT_P(isVDB(primoff));
        
        return reinterpret_cast<_TClass>(getPrimitive(primoff));
    }
    
    
    
    //SYS_FORCE_INLINE GEO_Primitive createGEOPrimitive(const GA_Offset primoff)
    //{
    //    UT_ASSERT_P(gfe::isValidOffset(getPrimitiveMap(), primoff));
    //    return GEO_Primitive(this, primoff);
    //}
    //
    //SYS_FORCE_INLINE GEO_PrimPoly createGEOPrimPoly(const GA_Offset primoff)
    //{
    //    UT_ASSERT_P(gfe::isValidOffset(getPrimitiveMap(), primoff));
    //    UT_ASSERT_P(isPoly(primoff));
    //    return GU_PrimPoly(this, primoff);
    //    
    //}
    //
    //SYS_FORCE_INLINE GEO_PrimVDB createVDBPrimitive(const GA_Offset primoff)
    //{
    //    UT_ASSERT_P(gfe::isValidOffset(getPrimitiveMap(), primoff));
    //    UT_ASSERT_P(isVDB(primoff));
    //    return GEO_PrimVDB(this->asGEO_Detail(), primoff);
    //}


        
    
    SYS_FORCE_INLINE GA_Size stealVertex(const GA_Offset primoff, const GA_Offset vtxoff, const GA_Offset insertBeforeVtx = GA_INVALID_OFFSET)
    { return getPrimitiveT<GEO_PrimPoly*>(primoff)->stealVertex(vtxoff, insertBeforeVtx); }

    
    /// Clear all the points/primitives out of this detail
    SYS_FORCE_INLINE void clearAndDestroy()
    {
        clearCaches();
        incrementMetaCacheCount();
        clear();
    }


    SYS_FORCE_INLINE void delVertex(const GA_Offset primoff, const GA_Offset vtxoff)
    {
        getPrimitive(primoff)->releaseVertex(vtxoff);
        //getTopology().delVertex(vtxoff);
        destroyVertexOffset(vtxoff);
    }


    
    SYS_FORCE_INLINE bool isUsedPoint(const GA_Offset ptoff) const
    { return gfe::isValidOffset(pointVertex(ptoff)); }

    SYS_FORCE_INLINE bool isUnusedPoint(const GA_Offset ptoff) const
    { return gfe::isInvalidOffset(pointVertex(ptoff)); }


    
    SYS_FORCE_INLINE bool isPacked(const GA_Offset primoff) const
    { return gfe::isPacked(getPrimitiveTypeId(primoff)); }





    
#if __GFE_Geometry_Inherited_From_GU_Detail
    
    SYS_FORCE_INLINE GA_Detail* asGA_Detail()
    { return reinterpret_cast<GA_Detail*>(this); }
    
    SYS_FORCE_INLINE const GA_Detail* asGA_Detail() const
    { return reinterpret_cast<const GA_Detail*>(this); }

    SYS_FORCE_INLINE GEO_Detail* asGEO_Detail()
    { return reinterpret_cast<GEO_Detail*>(this); }

    SYS_FORCE_INLINE const GEO_Detail* asGEO_Detail() const
    { return reinterpret_cast<const GEO_Detail*>(this); }

    SYS_FORCE_INLINE GU_Detail* asGU_Detail()
    { return reinterpret_cast<GU_Detail*>(this); }

    SYS_FORCE_INLINE const GU_Detail* asGU_Detail() const
    { return reinterpret_cast<const GU_Detail*>(this); }
    
#else
    
    SYS_FORCE_INLINE GA_Detail* asGA_Detail()
    { return reinterpret_cast<GA_Detail*>(this); }
    
        SYS_FORCE_INLINE const GA_Detail* asGA_Detail() const
    { return reinterpret_cast<const GA_Detail*>(this); }

        SYS_FORCE_INLINE GEO_Detail* asGEO_Detail()
    { return reinterpret_cast<GEO_Detail*>(asGA_Detail()); }

        SYS_FORCE_INLINE const GEO_Detail* asGEO_Detail() const
    { return reinterpret_cast<const GEO_Detail*>(asGA_Detail()); }

        SYS_FORCE_INLINE GU_Detail* asGU_Detail()
    { return reinterpret_cast<GU_Detail*>(asGA_Detail()); }

        SYS_FORCE_INLINE const GU_Detail* asGU_Detail() const
    { return reinterpret_cast<const GU_Detail*>(asGA_Detail()); }
        
#endif



        
    
    SYS_FORCE_INLINE const GA_AttributeDict	&getPrimitiveAttribDict() const
    { return getAttributes().getDict(GA_ATTRIB_PRIMITIVE); }
    SYS_FORCE_INLINE const GA_AttributeDict	&getPointAttribDict()     const
    { return getAttributes().getDict(GA_ATTRIB_POINT); }
    SYS_FORCE_INLINE const GA_AttributeDict	&getVertexAttribDict()    const
    { return getAttributes().getDict(GA_ATTRIB_VERTEX); }
    SYS_FORCE_INLINE const GA_AttributeDict	&getDetailAttribDict()    const
    { return getAttributes().getDict(GA_ATTRIB_DETAIL); }


    void cloneAllMissingAttribGroup(const GA_Detail& geoSrc)
    {
        const GA_AttributeFilter attribFilter = GA_AttributeFilter::selectPublic();
        //cloneMissingAttributes(geoSrc, GA_ATTRIB_OWNER_N,   attribFilter);
        //cloneMissingGroups    (geoSrc, GA_ATTRIB_OWNER_N,   attribFilter);
            
        cloneMissingAttributes(geoSrc, GA_ATTRIB_PRIMITIVE, attribFilter);
        cloneMissingAttributes(geoSrc, GA_ATTRIB_POINT,     attribFilter);
        cloneMissingAttributes(geoSrc, GA_ATTRIB_VERTEX,    attribFilter);
        cloneMissingAttributes(geoSrc, GA_ATTRIB_DETAIL,    attribFilter);
        
        cloneMissingGroups    (geoSrc, GA_ATTRIB_PRIMITIVE, attribFilter);
        cloneMissingGroups    (geoSrc, GA_ATTRIB_POINT,     attribFilter);
        cloneMissingGroups    (geoSrc, GA_ATTRIB_VERTEX,    attribFilter);
    }
    
    SYS_FORCE_INLINE void cloneAllMissingAttribGroup(const GA_Detail* const geoSrc)
    { cloneAllMissingAttribGroup(*geoSrc); }



    void dissolveVertexEdgeGroup(const GA_VertexGroup* group = nullptr,
        const bool delInlinePoint              = false,
        const fpreal inlineTol                 = 1e-05,
        const bool delUnusedPoint              = true,
        const GU_Detail::BridgeMode bridgeMode = GU_Detail::GU_BRIDGEMODE_BRIDGE,
        const bool delDegenerateBridge         = false,
        const bool boundaryCurves              = false
    )
    {
        const GA_EdgeGroupUPtr edgeGroupUPtr = createDetachedEdgeGroup();
        GA_EdgeGroup& edgeGroup = *edgeGroupUPtr.get();
        gfe::GroupUnion::groupUnion(edgeGroup, group);
        asGU_Detail()->dissolveEdges(edgeGroup,
            delInlinePoint, inlineTol, delUnusedPoint, bridgeMode, delDegenerateBridge, boundaryCurves);
    }
            
    
    //const GA_Storage posStorage = geo->getPStorage();
    SYS_FORCE_INLINE GA_Storage getPStorage() const
    { return getP()->getAIFTuple()->getStorage(getP()); }
    
    
    template<typename FLOAT_T>
    UT_BoundingBoxT<FLOAT_T> stdBoundingBox(const GA_Range& range, const GA_Attribute* const posAttrib = nullptr) const
    {
#if 0
        #define FLOAT_T_MAX std::numeric_limits<FLOAT_T>::max();
        #define FLOAT_T_MIN std::numeric_limits<FLOAT_T>::min();
        UT_BoundingBoxT<FLOAT_T> geoBBox(FLOAT_T_MAX, FLOAT_T_MAX, FLOAT_T_MAX, FLOAT_T_MIN, FLOAT_T_MIN, FLOAT_T_MIN);
#else
        UT_BoundingBoxT<float> geoBBox(SYS_FP32_MAX, SYS_FP32_MAX, SYS_FP32_MAX, SYS_FP32_MIN, SYS_FP32_MIN, SYS_FP32_MIN);
#endif
        enlargeBoundingBox(geoBBox, range, posAttrib);
        return geoBBox;
    }

    template<typename FLOAT_T>
    SYS_FORCE_INLINE UT_BoundingBoxT<FLOAT_T> stdBoundingBox(const GA_ElementGroup* const group = nullptr, const GA_Attribute* const posAttrib = nullptr) const
    { return stdBoundingBox<FLOAT_T>(GA_Range(group ? group->getIndexMap() : getPointMap(), group), posAttrib); }

    template<typename FLOAT_T>
    SYS_FORCE_INLINE UT_BoundingBoxT<FLOAT_T> stdBoundingBox(const GA_Group* const group, const GA_Attribute* const posAttrib = nullptr) const
    {
        UT_ASSERT_MSG(!group || group->isElementGroup(), "Must be Element Group");
        return stdBoundingBox<FLOAT_T>(static_cast<const GA_ElementGroup*>(group), posAttrib);
    }
    
    
    template<GA_AttributeOwner FROM, GA_AttributeOwner TO>
    SYS_FORCE_INLINE GA_Offset offsetPromote(const GA_Offset elemoffFrom) const
    {
        if constexpr(TO == GA_ATTRIB_GLOBAL)
        {
            return 0;
        }
        if constexpr(FROM == GA_ATTRIB_GLOBAL)
        {
            return 0;
        }
        else if constexpr(FROM == GA_ATTRIB_PRIMITIVE)
        {
            if constexpr(TO == GA_ATTRIB_PRIMITIVE)
                return elemoffFrom;
            else if constexpr(TO == GA_ATTRIB_POINT)
                return getPrimitivePointOffset(elemoffFrom, 0);
            else if constexpr(TO == GA_ATTRIB_VERTEX)
                return getPrimitiveVertexOffset(elemoffFrom, 0);
        }
        else if constexpr(FROM == GA_ATTRIB_POINT)
        {
            if constexpr(TO == GA_ATTRIB_PRIMITIVE)
                return pointPrim(elemoffFrom);
            else if constexpr(TO == GA_ATTRIB_POINT)
                return elemoffFrom;
            else if constexpr(TO == GA_ATTRIB_VERTEX)
                return pointVertex(elemoffFrom);
        }
        else if constexpr(FROM == GA_ATTRIB_VERTEX)
        {
            if constexpr(TO == GA_ATTRIB_PRIMITIVE)
                return vertexPrimitive(elemoffFrom);
            else if constexpr(TO == GA_ATTRIB_POINT)
                return vertexPoint(elemoffFrom);
            else if constexpr(TO == GA_ATTRIB_VERTEX)
                return elemoffFrom;
        }
        else
            return gfe::INVALID_OFFSET;
    }


    SYS_FORCE_INLINE bool isPrimitiveClosedLooped(const GA_Offset primoff) const
    { return getPrimitiveClosedFlag(primoff) || isPrimitiveLooped(primoff); }
    
    SYS_FORCE_INLINE bool isPrimitiveLooped(const GA_Offset primoff) const
    {
        const GA_Offset primpoint1 = getPrimitivePointOffset(primoff, getPrimitiveVertexCount(primoff)-1);
        return primpoint1 == getPrimitivePointOffset(primoff, 0);
    }

    SYS_FORCE_INLINE GA_Range getRange(const GA_ElementGroup& group, const bool reverse = false) const
    { return GA_Range(group, reverse); }

    SYS_FORCE_INLINE GA_Range getPrimitiveRange(const GA_PrimitiveGroup* const group = nullptr, const bool reverse = false) const
    { return GA_Range(getPrimitiveMap(), group, reverse); }

    SYS_FORCE_INLINE GA_Range getPointRange(const GA_PointGroup* const group = nullptr, const bool reverse = false) const
    { return GA_Range(getPointMap(), group, reverse); }
    
    SYS_FORCE_INLINE GA_Range getVertexRange(const GA_VertexGroup* const group = nullptr, const bool reverse = false) const
    { return GA_Range(getVertexMap(), group, reverse); }

    
    SYS_FORCE_INLINE GA_SplittableRange getSplittableRange(const GA_ElementGroup& group, const bool reverse = false) const
    { return GA_SplittableRange(getRange(group, reverse)); }

    SYS_FORCE_INLINE GA_SplittableRange getPrimitiveSplittableRange(const GA_PrimitiveGroup* const group = nullptr, const bool reverse = false) const
    { return GA_SplittableRange(getPrimitiveRange(group, reverse)); }

    SYS_FORCE_INLINE GA_SplittableRange getPointSplittableRange(const GA_PointGroup* const group = nullptr, const bool reverse = false) const
    { return GA_SplittableRange(getPointRange(group, reverse)); }
    
    SYS_FORCE_INLINE GA_SplittableRange getVertexSplittableRange(const GA_VertexGroup* const group = nullptr, const bool reverse = false) const
    { return GA_SplittableRange(getVertexRange(group, reverse)); }

    
    SYS_FORCE_INLINE GA_Range getRange(const GA_PrimitiveGroup* group = nullptr, const bool reverse = false) const
    { return getPrimitiveRange(group, reverse); }

    SYS_FORCE_INLINE GA_Range getRange(const GA_PointGroup* group = nullptr, const bool reverse = false) const
    { return getPointRange(group, reverse); }
    
    SYS_FORCE_INLINE GA_Range getRange(const GA_VertexGroup* group = nullptr, const bool reverse = false) const
    { return getVertexRange(group, reverse); }


    
private:
    
    SYS_FORCE_INLINE GA_Offset edgeVertexSub(const GA_Offset ptoff0, const GA_Offset ptoff1) const
    { return gfe::GeoBase::edgeVertexSub(*this, ptoff0, ptoff1); }
    
public:
    
    SYS_FORCE_INLINE GA_Offset edgeVertex(const GA_Offset ptoff0, const GA_Offset ptoff1) const
    { return gfe::GeoBase::edgeVertex(*this, ptoff0, ptoff1); }
    
    SYS_FORCE_INLINE GA_Offset edgeVertex(const GA_Edge& edge) const
    { return gfe::GeoBase::edgeVertex(*this, edge); }
    
    

    void delTopoAttrib()
    {
        GA_AttributeSet& attribSet = getAttributes();
        GA_AttributeFilter filter = GA_AttributeFilter::selectByPattern("__topo_*");
        filter = GA_AttributeFilter::selectAnd(filter, GA_AttributeFilter::selectPublic());
        filter = GA_AttributeFilter::selectAnd(filter, GA_AttributeFilter::selectNot(GA_AttributeFilter::selectGroup()));
        attribSet.destroyAttributes(GA_ATTRIB_PRIMITIVE, filter);
        attribSet.destroyAttributes(GA_ATTRIB_POINT,     filter);
        attribSet.destroyAttributes(GA_ATTRIB_VERTEX,    filter);
        attribSet.destroyAttributes(GA_ATTRIB_DETAIL,    filter);

        GA_GroupTable* groupTable = nullptr;
        GA_Group* groupPtr = nullptr;
        for (GA_GroupType groupType : {GA_GROUP_PRIMITIVE, GA_GROUP_POINT, GA_GROUP_VERTEX, GA_GROUP_EDGE})
        {
            groupTable = getGroupTable(groupType);
            //for (GA_GroupTable::iterator it = groupTable->beginTraverse(); !it.atEnd(); it.operator++())
            for (GA_GroupTable::iterator<GA_Group> it = groupTable->beginTraverse(); !it.atEnd(); ++it)
            {
                groupPtr = it.group();
                //if (groupPtr->isDetached())
                //    continue;
                if (!groupPtr->getName().startsWith("__topo_"))
                    continue;
                groupTable->destroy(groupPtr);
            }
        }
    }


    

GA_Group& groupDuplicate(const GA_Group& group, const UT_StringHolder& groupName)
{
    GA_Group& newGroup = *getGroupTable(group.classType())->newGroup(groupName);
    if (group.isElementGroup())
        static_cast<GA_ElementGroup&>(newGroup).combine(&group);
    else
        static_cast<GA_EdgeGroup&>(newGroup).combine(&group);
    return newGroup;
}


GA_Group& groupDuplicate(const GA_Group& group)
{
    GA_Group& newGroup = *getGroupTable(group.classType())->newDetachedGroup();
    if (group.isElementGroup())
        static_cast<GA_ElementGroup&>(newGroup).combine(&group);
    else
        static_cast<GA_EdgeGroup&>(newGroup).combine(&group);
    return newGroup;
}

SYS_FORCE_INLINE GA_Group& groupDuplicateDetached(const GA_Group& group)
{ return groupDuplicate(group); }






SYS_FORCE_INLINE void delStdAttribute(
    const char* primAttribPattern,
    const char* pointAttribPattern,
    const char* vertexAttribPattern,
    const char* detailAttribPattern
)
{ gfe::AttributeDelete::delStdAttribute(getAttributes(), primAttribPattern, pointAttribPattern, vertexAttribPattern, detailAttribPattern); }
SYS_FORCE_INLINE void keepStdAttribute(
    const char* primAttribPattern,
    const char* pointAttribPattern,
    const char* vertexAttribPattern,
    const char* detailAttribPattern
)
{ gfe::AttributeDelete::keepStdAttribute(getAttributes(), primAttribPattern, pointAttribPattern, vertexAttribPattern, detailAttribPattern); }



SYS_FORCE_INLINE void groupToggle(const GA_GroupType groupType, const char* groupName)
{ gfe::Group::groupToggle(*getGroupTable(groupType), groupName); }

SYS_FORCE_INLINE void delStdGroup(const GA_GroupType groupType, const char* groupPattern)
{ gfe::Group::delStdGroup(*getGroupTable(groupType), groupPattern); }

SYS_FORCE_INLINE void keepStdGroup(const GA_GroupType groupType, const char* keepGroupPattern)
{ gfe::Group::keepStdGroup(*getGroupTable(groupType), keepGroupPattern); }

SYS_FORCE_INLINE void keepStdAttribute(const GA_AttributeOwner attribOwner, const char* keepGroupPattern)
{ gfe::AttributeDelete::keepStdAttribute(getAttributes(), attribOwner, keepGroupPattern); }

void delStdGroup(
    const char* primGroupPattern,
    const char* pointGroupPattern,
    const char* vertexGroupPattern,
    const char* edgeGroupPattern
)
{
    delStdGroup(GA_GROUP_PRIMITIVE, primGroupPattern);
    delStdGroup(GA_GROUP_POINT,     pointGroupPattern);
    delStdGroup(GA_GROUP_VERTEX,    vertexGroupPattern);
    delStdGroup(GA_GROUP_EDGE,      edgeGroupPattern);
}

void keepStdGroup(
    const char* primGroupPattern,
    const char* pointGroupPattern,
    const char* vertexGroupPattern
)
{
    keepStdGroup(GA_GROUP_PRIMITIVE, primGroupPattern);
    keepStdGroup(GA_GROUP_POINT, pointGroupPattern);
    keepStdGroup(GA_GROUP_VERTEX, vertexGroupPattern);
}

void keepStdGroup(
    const char* primGroupPattern,
    const char* pointGroupPattern,
    const char* vertexGroupPattern,
    const char* edgeGroupPattern
)
{
    keepStdGroup(GA_GROUP_PRIMITIVE, primGroupPattern);
    keepStdGroup(GA_GROUP_POINT,     pointGroupPattern);
    keepStdGroup(GA_GROUP_VERTEX,    vertexGroupPattern);
    keepStdGroup(GA_GROUP_EDGE,      edgeGroupPattern);
}



#if SYS_VERSION_MAJOR_INT <= 19 && !( SYS_VERSION_MAJOR_INT == 19 && SYS_VERSION_MINOR_INT == 5 )
    /// Given a vertex, set the corresponding point offset.
    void setVertexPoint(GA_Offset vertex, GA_Offset ptoff)
    {
        UT_ASSERT_P(!getVertexMap().isOffsetVacant(vertex));
        getTopology().wireVertexPoint(vertex, ptoff);
    }
#endif

    
#if SYS_VERSION_MAJOR_INT <= 19 && !( SYS_VERSION_MAJOR_INT == 19 && SYS_VERSION_MINOR_INT == 5 )
    /// The ptoff passed is the point offset. @see vertexPoint()
    SYS_FORCE_INLINE UT_Vector3 getPos3(GA_Offset ptoff) const
    {
        //UT_Vector3 pos;
        //return getP()->getAIFTuple()->get(getP(), ptoff, pos, 0);
        const GA_PageArray<fpreal32,3>& myExactMatch = static_cast<const GA_ATINumeric*>(getP())->getData().castType<fpreal32>().template castTupleSize<3>();
        return UT_Vector3(myExactMatch.template getVector<fpreal32,3>(ptoff));
        //return myHandlePV3.get(ptoff);
    }
    
    
    SYS_FORCE_INLINE UT_Vector3D getPos3D(GA_Offset ptoff) const
    {
        const GA_PageArray<fpreal64,3>& myAlmostMatch = static_cast<const GA_ATINumeric*>(getP())->getData().castType<typename UT_StorageNum<fpreal32>::SecondGuess>().template castTupleSize<3>();
        return myAlmostMatch.template getVector<fpreal64,3>(ptoff);
        //return UTmakeVector3T(myHandlePV3.getAlt(ptoff));
    }
    template <typename T>
    UT_Vector3T<T> getPos3T(GA_Offset ptoff) const
    {
        if constexpr(std::is_same<T, float>::value)
            return getPos3(ptoff);
        else
            return getPos3D(ptoff);
    }
#endif

        
SYS_FORCE_INLINE static GA_Size vertexPrimIndex(const GA_OffsetListRef& vertices, const GA_Offset vtxoff)
{ return gfe::GeoBase::vertexPrimIndex(vertices, vtxoff); }
        
SYS_FORCE_INLINE static GA_Size vertexPrimIndex(const GA_PrimitiveList& primList, const GA_Offset primoff, const GA_Offset vtxoff)
{ return gfe::GeoBase::vertexPrimIndex(primList, primoff, vtxoff); }
    
SYS_FORCE_INLINE GA_Size vertexPrimIndex(const GA_Offset primoff, const GA_Offset vtxoff)
{ return gfe::GeoBase::vertexPrimIndex(*this, primoff, vtxoff); }

SYS_FORCE_INLINE GA_Size vertexPrimIndex(const GA_Offset vtxoff)
{ return gfe::GeoBase::vertexPrimIndex(*this, vtxoff); }

SYS_FORCE_INLINE GA_Offset vertexPointDst(const GA_Offset primoff, const GA_Size vtxpnum)
{ return gfe::GeoBase::vertexPointDst(*this, primoff, vtxpnum); }

SYS_FORCE_INLINE GA_Offset vertexPointDst(const GA_Offset vtxoff)
{ return gfe::GeoBase::vertexPointDst(*this, vtxoff); }

        

        
SYS_FORCE_INLINE GA_Offset getPrimitivePointOffset(const GA_OffsetListRef& vertices, const GA_Size vtxpnum = 0) const
{ return vertexPoint(vertices[vtxpnum]); }

SYS_FORCE_INLINE GA_Offset primPoint(const GA_OffsetListRef& vertices, const GA_Size vtxpnum = 0) const
{ return getPrimitivePointOffset(vertices, vtxpnum); }

SYS_FORCE_INLINE GA_Offset getPrimitivePointIndex(const GA_OffsetListRef& vertices, const GA_Size vtxpnum = 0) const
{ return pointIndex(getPrimitivePointOffset(vertices, vtxpnum)); }

SYS_FORCE_INLINE GA_Offset primPointIndex(const GA_OffsetListRef& vertices, const GA_Size vtxpnum = 0) const
{ return getPrimitivePointIndex(vertices, vtxpnum); }

template<typename T>
SYS_FORCE_INLINE T primPointVal(const GA_ROHandleT<T>& pos_h, const GA_OffsetListRef& vertices, const GA_Size vtxpnum = 0) const
{ return pos_h.get(primPoint(vertices, vtxpnum)); }

        
template<typename T>
SYS_FORCE_INLINE T vertexPointVal(const GA_ROHandleT<T>& pos_h, const GA_Offset vtxoff) const
{ return pos_h.get(vertexPoint(vtxoff)); }

        
        
SYS_FORCE_INLINE GA_Offset getPrimitivePointOffset(const GA_Offset primoff, const GA_Size vtxpnum = 0) const
{ return vertexPoint(getPrimitiveVertexOffset(primoff, vtxpnum)); }

SYS_FORCE_INLINE GA_Offset primPoint(const GA_Offset primoff, const GA_Size vtxpnum = 0) const
{ return getPrimitivePointOffset(primoff, vtxpnum); }

SYS_FORCE_INLINE GA_Index getPrimitivePointIndex(const GA_Offset primoff, const GA_Size vtxpnum = 0) const
{ return pointIndex(getPrimitivePointOffset(primoff, vtxpnum)); }

SYS_FORCE_INLINE GA_Index primPointIndex(const GA_Offset primoff, const GA_Size vtxpnum = 0) const
{ return getPrimitivePointIndex(primoff, vtxpnum); }

    
SYS_FORCE_INLINE GA_Offset primVertex(const GA_Offset primoff, const GA_Size vtxpnum = 0) const
{ return getPrimitiveVertexOffset(primoff, vtxpnum); }
    
SYS_FORCE_INLINE GA_Offset pointPrim(const GA_Offset ptoff) const
{ return vertexPrimitive(pointVertex(ptoff)); }

SYS_FORCE_INLINE GA_Offset vertexPrim(const GA_Offset vtxoff) const
{ return vertexPrimitive(vtxoff); }
    

    

SYS_FORCE_INLINE UT_Vector3 getPrimitivePointPos3(const GA_Offset primoff,const GA_Size vtxpnum = 0) const
{ return getPos3(getPrimitivePointOffset(primoff, vtxpnum)); }

#if SYS_VERSION_MAJOR_INT > 19 || ( SYS_VERSION_MAJOR_INT == 19 && SYS_VERSION_MINOR_INT == 5 )

SYS_FORCE_INLINE UT_Vector3D getPrimitivePointPos3D(const GA_Offset primoff,const GA_Size vtxpnum = 0) const
{ return getPos3D(getPrimitivePointOffset(primoff, vtxpnum)); }

template<typename T>
SYS_FORCE_INLINE UT_Vector3T<T> getPrimitivePointPos3T(const GA_Offset primoff,const GA_Size vtxpnum = 0) const
{ return getPos3T<T>(getPrimitivePointOffset(primoff, vtxpnum)); }

#endif


GA_Size getNumElements(const GA_AttributeOwner attribClass) const
{
    switch (attribClass)
    {
    case GA_ATTRIB_PRIMITIVE: return getNumPrimitives(); break;
    case GA_ATTRIB_POINT:     return getNumPoints();     break;
    case GA_ATTRIB_VERTEX:    return getNumVertices();   break;
    }
    return gfe::INVALID_OFFSET;
}

GA_Size getNumElements(const GA_GroupType groupType) const
{
    switch (groupType)
    {
    case GA_GROUP_PRIMITIVE: return getNumPrimitives(); break;
    case GA_GROUP_POINT:     return getNumPoints();     break;
    case GA_GROUP_VERTEX:    return getNumVertices();   break;
    }
    return gfe::INVALID_OFFSET;
}

SYS_FORCE_INLINE GA_Size getNumElements(const GA_Attribute* const attrib) const
{ return getNumElements(attrib->getOwner()); }

SYS_FORCE_INLINE GA_Size getNumElements(const GA_Attribute& attrib) const
{ return getNumElements(attrib.getOwner()); }

SYS_FORCE_INLINE GA_Size getNumElements(const GA_Group* const group) const
{ return getNumElements(group->classType()); }

SYS_FORCE_INLINE GA_Size getNumElements(const GA_Group& group) const
{ return getNumElements(group.classType()); }

    
void clearElement()
{
    //clear();
    clearTopologyAttributes();
    getIndexMap(GA_ATTRIB_PRIMITIVE).clear(true);
    getIndexMap(GA_ATTRIB_POINT    ).clear(true);
    getIndexMap(GA_ATTRIB_VERTEX   ).clear(true);
    getPrimitiveList()              .clear(true);
    createTopologyAttributes();
}

    
GA_OffsetList getOffsetList(const GA_IndexMap& indexMap, const GA_ElementGroup* const group = nullptr, const bool reverse = false) const
{
    GA_OffsetList offList;
    if (!group && indexMap.isTrivialMap())
    {
        if (!reverse)
            offList.setTrivial(GA_Offset(0), indexMap.indexSize());
    }
    else
    {
        GA_Offset start, end;
        for (GA_Iterator it(GA_Range(indexMap, group, reverse)); it.fullBlockAdvance(start, end); )
        {
            offList.setTrivialRange(offList.size(), start, end - start);
        }
    }
    return offList;
}
    
//GA_OffsetList offList = GFE_Detail::getOffsetList(group);
    
SYS_FORCE_INLINE GA_OffsetList getOffsetList(const GA_AttributeOwner owner, const GA_ElementGroup* const group = nullptr, const bool reverse = false) const
{ return getOffsetList(getIndexMap(owner), group, reverse); }

SYS_FORCE_INLINE GA_OffsetList getOffsetList(const GA_ElementGroup& group, const bool reverse = false) const
{ return getOffsetList(group.getOwner(), &group, reverse); }

SYS_FORCE_INLINE GA_OffsetList getOffsetList(const GA_PrimitiveGroup* const group, const bool reverse = false) const
{ return getOffsetList(getPrimitiveMap(), group, reverse); }

SYS_FORCE_INLINE GA_OffsetList getOffsetList(const GA_PointGroup* const group, const bool reverse = false) const
{ return getOffsetList(getPointMap(), group, reverse); }

SYS_FORCE_INLINE GA_OffsetList getOffsetList(const GA_VertexGroup* const group, const bool reverse = false) const
{ return getOffsetList(getVertexMap(), group, reverse); }



SYS_FORCE_INLINE GA_Offset offsetFromIndex(const GA_AttributeOwner owner, const GA_Size elemoff) const
{ return getIndexMap(owner).offsetFromIndex(elemoff); }

SYS_FORCE_INLINE GA_Size indexFromOffset(const GA_AttributeOwner owner, const GA_Offset elemoff) const
{ return getIndexMap(owner).indexFromOffset(elemoff); }

SYS_FORCE_INLINE GA_Size primitiveIndexFromOffset(const GA_Offset elemoff) const
{ return getPrimitiveMap().indexFromOffset(elemoff); }


    
    
/////////////////////////////////// deleteElements ////////////////////////////////////////

    SYS_FORCE_INLINE bool deletePointOffsetOnly(const GA_Offset ptoff)
    { return destroyPointOffset(ptoff, GA_Detail::GA_LEAVE_PRIMITIVES, true); }
    
    SYS_FORCE_INLINE void deletePrimitiveOffsets(
        const GA_PrimitiveGroup* const group,
        const bool reverseGroup = false,
        const bool withPoint = false
    )
    { destroyPrimitiveOffsets(GA_Range(getPrimitiveMap(), group, reverseGroup), withPoint); }

    
    SYS_FORCE_INLINE void deletePointOffsets(
        const GA_PointGroup* const group,
        const bool reverseGroup = false,
        const GA_DestroyPointMode mode = GA_LEAVE_PRIMITIVES,
        const bool guaranteeNoVertexReferences = false
    )
    { destroyPointOffsets(GA_Range(getPointMap(), group, reverseGroup), mode, guaranteeNoVertexReferences); }

    
    SYS_FORCE_INLINE void deleteVertexOffsets(
        const GA_VertexGroup* const group,
        const bool reverseGroup = false
    )
    { destroyVertexOffsets(GA_Range(getVertexMap(), group, reverseGroup)); }



    
    SYS_FORCE_INLINE void deletePrimitiveOffsets(
        const GA_Group* const group, const bool reverseGroup = false,
        const bool withPoint = false
    )
    { deletePrimitiveOffsets(static_cast<const GA_PrimitiveGroup*>(group), reverseGroup, withPoint); }
    
    SYS_FORCE_INLINE void deletePointOffsets(
        const GA_Group* const group, const bool reverseGroup = false,
        const GA_DestroyPointMode mode = GA_LEAVE_PRIMITIVES,
        const bool guaranteeNoVertexReferences = false
    )
    { deletePointOffsets(static_cast<const GA_PointGroup*>(group), reverseGroup, mode, guaranteeNoVertexReferences); }

    SYS_FORCE_INLINE void deleteVertexOffsets(const GA_Group* const group, const bool reverseGroup = false)
    { deleteVertexOffsets(static_cast<const GA_VertexGroup*>(group), reverseGroup); }

    
    void deleteElements(
        const GA_Group* const group,
        const bool reverseGroup = false,
        const bool withPoint = false,
        const GA_DestroyPointMode mode = GA_LEAVE_PRIMITIVES,
        const bool guaranteeNoVertexReferences = false
    )
    {
        if (group)
        {
            switch (group->classType())
            {
            case GA_GROUP_PRIMITIVE: deletePrimitiveOffsets(group, reverseGroup, withPoint);                     break;
            case GA_GROUP_POINT:     deletePointOffsets(group, reverseGroup, mode, guaranteeNoVertexReferences); break;
            case GA_GROUP_VERTEX:    deleteVertexOffsets(group, reverseGroup);                                   break;
            }
        }
        else
        {
            if (reverseGroup)
                return;
            deletePrimitiveOffsets(group, reverseGroup, withPoint);
        }
    }



    
    
    /////////////////////////////////// deleteOneElement ////////////////////////////////////////


    void deleteOnePrimitive(
        const GA_Offset elemoff,
        const bool reverseGroup = false,
        const bool withPoint = false
    )
    {
        if (reverseGroup)
        {
            const GA_IndexMap& indexMap = getPrimitiveMap();
            destroyPrimitiveOffsets(GA_Range(indexMap, 0, elemoff), true);
            destroyPrimitiveOffsets(GA_Range(indexMap, elemoff+1, indexMap.offsetSize()), true);
        }
        else
        {
            destroyPrimitiveOffset(elemoff, withPoint);
        }
    }


    void deleteOnePoint(
        const GA_Offset elemoff,
        const bool reverseGroup = false,
        const GA_DestroyPointMode mode = GA_LEAVE_PRIMITIVES,
        const bool guaranteeNoVertexReferences = false
    )
    {
        if (reverseGroup)
        {
            const GA_IndexMap& indexMap = getPointMap();
            destroyPointOffsets(GA_Range(indexMap, 0, elemoff), mode, guaranteeNoVertexReferences);
            destroyPointOffsets(GA_Range(indexMap, elemoff+1, indexMap.offsetSize()), mode, guaranteeNoVertexReferences);
        }
        else
        {
            destroyPointOffset(elemoff, mode, guaranteeNoVertexReferences);
        }
    }
    
    void deleteOneVertex(const GA_Offset elemoff, const bool reverseGroup = false)
    {
        if (reverseGroup)
        {
            const GA_IndexMap& indexMap = getVertexMap();
            destroyVertexOffsets(GA_Range(indexMap, 0, elemoff));
            destroyVertexOffsets(GA_Range(indexMap, elemoff+1, indexMap.offsetSize()));
        }
        else
        {
            destroyVertexOffset(elemoff);
        }
    }
    //
    // void deleteOneEdge(
    //     const GA_Offset elemoff0,
    //     const GA_Offset elemoff1,
    //     const bool reverseGroup = false
    // )
    // {
    // }
    
    
    void deleteOneElement(const GA_AttributeOwner owner, const GA_Offset elemoff, const bool reverseGroup = false
    )
    {
        switch (owner)
        {
        case GA_ATTRIB_PRIMITIVE: deleteOnePrimitive(elemoff, reverseGroup); break;
        case GA_ATTRIB_POINT:     deleteOnePoint(elemoff, reverseGroup);     break;
        case GA_ATTRIB_VERTEX:    deleteOneVertex(elemoff, reverseGroup);    break;
        }
    }

    void deleteOneElement(const GA_GroupType owner, const GA_Offset elemoff, const bool reverseGroup = false
    )
    {
        switch (owner)
        {
        case GA_GROUP_PRIMITIVE: deleteOnePrimitive(elemoff, reverseGroup); break;
        case GA_GROUP_POINT:     deleteOnePoint(elemoff, reverseGroup);     break;
        case GA_GROUP_VERTEX:    deleteOneVertex(elemoff, reverseGroup);    break;
        //case GA_GROUP_EDGE:      deleteOneEdge(elemoff0, elemoff1, reverseGroup);    break;
        }
    }




    /////////////////////////////////// delete Elment Skip N Elem ////////////////////////////////////////

    void deletePrimitiveSkipNElem(
        const GA_Offset elemoff,
        const GA_Size skipSize,
        const bool reverseGroup = false,
        const bool withPoint = false
    )
    {
        if (skipSize <= 0)
        {
            if (!reverseGroup)
                destroyPrimitiveOffsets(getPrimitiveRange(), withPoint);
            return;
        }
        
        GA_Size delSize = primitiveIndexFromOffset(elemoff);
        delSize %= skipSize;
        
        GA_Offset start, end;
        for (GA_Iterator it(getPrimitiveRange()); it.fullBlockAdvance(start, end); )
        {
            for (GA_Offset elemoff = start; elemoff < end; ++elemoff)
            {
                if (delSize!=0 ^ reverseGroup)
                    destroyPrimitiveOffset(elemoff, withPoint);
                
                if (delSize == skipSize)
                    delSize = 0;
                else
                    ++delSize;
            }
        }
    }




SYS_FORCE_INLINE GA_Precision getValidPrecision(const GA_Precision precision) const
{ return precision < GA_PRECISION_16 ? getPreferredPrecision() : precision; }


SYS_FORCE_INLINE GA_Precision getPreferredPrecision() const
{ return GA_Detail::getPreferredPrecision(); }

SYS_FORCE_INLINE GA_Precision getPreferredPrecision(const GA_Precision precision) const
{ return precision == GA_PRECISION_INVALID ? getPreferredPrecision() : precision; }
    
SYS_FORCE_INLINE GA_Storage getPreferredStorageI() const
{ return gfe::getPreferredStorageI(getPreferredPrecision()); }

SYS_FORCE_INLINE GA_Storage getPreferredStorageF() const
{ return gfe::getPreferredStorageF(getPreferredPrecision()); }

SYS_FORCE_INLINE GA_Storage getPreferredStorage(const GA_StorageClass storageClass) const
{ return gfe::getPreferredStorage(storageClass, getPreferredPrecision()); }

SYS_FORCE_INLINE GA_Storage getPreferredStorage(const GA_Storage storage = GA_STORE_INVALID) const
{ return storage == GA_STORE_INVALID ? getPreferredStorage(storage) : storage; }

SYS_FORCE_INLINE GA_Storage getPreferredStorageI(const GA_Storage storage) const
{ return storage == GA_STORE_INVALID ? getPreferredStorageI() : storage; }

SYS_FORCE_INLINE GA_Storage getPreferredStorage(const GA_Storage storage,const GA_StorageClass storageClass) const
{ return storage == GA_STORE_INVALID ? getPreferredStorage(storageClass) : storage; }




















GA_Group* findGroup(const GA_GroupType groupType, const UT_StringRef& groupName) const
{
    if (groupType == GA_GROUP_N)
        return findGroupN(groupName);
    else
        return findGroupBase(groupType, groupName);
}

SYS_FORCE_INLINE GA_ElementGroup* findElementGroup(const GA_GroupType groupType, const UT_StringRef& groupName) const
{
    UT_ASSERT(groupType != GA_GROUP_EDGE);
    return static_cast<GA_ElementGroup*>(findGroup(groupType, groupName));
}
// SYS_FORCE_INLINE const GA_ElementGroup* findElementGroup(const GA_GroupType groupType, const UT_StringRef& groupName) const
// {
//     UT_ASSERT(groupType != GA_GROUP_EDGE);
//     return static_cast<const GA_ElementGroup*>(findGroup(groupType, groupName));
// }

SYS_FORCE_INLINE GA_ElementGroup* findElementGroup(const GA_AttributeOwner groupType, const UT_StringRef& groupName) const
{
    UT_ASSERT(groupType != GA_GROUP_EDGE);
    return findElementGroup(attributeOwner_groupType(groupType), groupName);
}

// SYS_FORCE_INLINE const GA_ElementGroup* findElementGroup(const GA_AttributeOwner groupType, const UT_StringRef& groupName) const
// {
//     UT_ASSERT(groupType != GA_GROUP_EDGE);
//     return findElementGroup(attributeOwner_groupType(groupType), groupName);
// }

    
SYS_FORCE_INLINE GA_PrimitiveGroup* findPrimitiveGroup(const UT_StringRef& groupName) const
{ return static_cast<GA_PrimitiveGroup*>(findGroup(GA_GROUP_PRIMITIVE, groupName)); }

SYS_FORCE_INLINE GA_PointGroup* findPointGroup(const UT_StringRef& groupName) const
{ return static_cast<GA_PointGroup*>(findGroup(GA_GROUP_POINT, groupName)); }

SYS_FORCE_INLINE GA_VertexGroup* findVertexGroup(const UT_StringRef& groupName) const
{ return static_cast<GA_VertexGroup*>(findGroup(GA_GROUP_VERTEX, groupName)); }

SYS_FORCE_INLINE GA_EdgeGroup* findEdgeGroup(const UT_StringRef& groupName) const
{ return static_cast<GA_EdgeGroup*>(findGroup(GA_GROUP_EDGE, groupName)); }



    
SYS_FORCE_INLINE bool destroyAttrib(GA_Attribute& attrib)
{ return getAttributes().destroyAttribute(&attrib); }

SYS_FORCE_INLINE bool destroyAttrib(GA_Attribute* const attrib)
{ return attrib ? getAttributes().destroyAttribute(attrib) : false; }

SYS_FORCE_INLINE bool destroyNonDetachedAttrib(GA_Attribute& attrib)
{
    if (attrib.isDetached())
        return false;
    return getAttributes().destroyAttribute(&attrib);
}

SYS_FORCE_INLINE bool destroyNonDetachedAttrib(GA_Attribute* const attrib)
{
    if (attrib->isDetached())
        return false;
    return getAttributes().destroyAttribute(attrib);
}


void attribBumpDataId(const GA_AttributeOwner owner, const char* const pattern)
{
    if (!pattern || pattern == "")
        return;
    for (GA_AttributeDict::iterator it = getAttributes().begin(owner); !it.atEnd(); ++it)
    {
        GA_Attribute& attribPtr = **it;
        if (!attribPtr.getName().multiMatch(pattern))
            continue;
        attribPtr.bumpDataId();
    }
}

SYS_FORCE_INLINE void groupBumpDataId(const GA_GroupType groupType, const char* const groupPattern)
{ return gfe::Group::groupBumpDataId(*getGroupTable(groupType), groupPattern); }

    
SYS_FORCE_INLINE bool renameAttrib(const GA_Attribute& attrib, const UT_StringRef& newName)
{
    UT_ASSERT(!attrib.isDetached());
    return renameAttribute(attrib.getOwner(), attrib.getScope(), attrib.getName(), newName);
}
    
SYS_FORCE_INLINE bool renameAttrib(const GA_Attribute* const attrib, const UT_StringRef& newName)
{ return attrib ? renameAttrib(*attrib, newName) : false; }
    
bool forceRenameAttribute(GA_Attribute& attrib, const UT_StringRef& newName)
{
    if (attrib.isDetached() || gfe::stringEqual(attrib.getName(), newName))
        return false;
    GA_Attribute* const existAttrib = findAttribute(attrib.getOwner(), newName);
    if (existAttrib)
        getAttributes().destroyAttribute(existAttrib);
    return renameAttrib(attrib, newName);
}

SYS_FORCE_INLINE bool forceRenameAttribute(GA_Attribute* const attrib, const UT_StringRef& newName)
{ return attrib ? forceRenameAttribute(*attrib, newName) : false; }

SYS_FORCE_INLINE bool forceRenameAttribute(const GA_AttributeOwner owner, const UT_StringRef& attribName, const UT_StringRef& newName)
{ return strcmp(attribName.c_str(), newName.c_str()) == 0 ? false : forceRenameAttribute(findAttribute(owner, attribName), newName); }




    SYS_FORCE_INLINE bool renameGroup(const GA_Group& group, const UT_StringHolder& newName)
    {
        UT_ASSERT(!group.isDetached());
        return getGroupTable(group.classType())->renameGroup(group.getName(), newName);
    }

    SYS_FORCE_INLINE bool renameGroup(const GA_Group* const group, const UT_StringHolder& newName)
    { return group ? renameGroup(*group, newName) : false; }

    bool forceRenameGroup(GA_Group& group, const UT_StringRef& newName)
    {
        if (group.isDetached())
            return false;
        GA_GroupTable* const groupTable = getGroupTable(group.classType());
        GA_Group* const existAttrib = groupTable->find(newName);
        if (existAttrib)
            groupTable->destroy(existAttrib);
        return renameGroup(group, newName);
    }

    SYS_FORCE_INLINE bool forceRenameGroup(GA_Group* const group, const UT_StringRef& newName)
    { return group ? forceRenameGroup(*group, newName) : false; }

    SYS_FORCE_INLINE bool forceRenameGroup(const GA_GroupType owner, const UT_StringRef& groupName, const UT_StringRef& newName)
    { return strcmp(groupName.c_str(), newName.c_str()) == 0 ? false : forceRenameGroup(findGroup(owner, groupName), newName); }



    
SYS_FORCE_INLINE GA_GroupType attributeOwner_groupType(const GA_AttributeOwner attribOwner) const
{ return gfe::attributeOwner_groupType(attribOwner); }  

SYS_FORCE_INLINE GA_AttributeOwner attributeOwner_groupType(const GA_GroupType groupType) const
{ return gfe::attributeOwner_groupType(groupType); }



public:



SYS_FORCE_INLINE GA_Range getRangeByAnyGroup(const GA_ElementGroup& group) const
{ return GA_Range(group); }

SYS_FORCE_INLINE GA_Range getRangeByAnyGroup(const GA_ElementGroup* const group) const
{
    if (!group)
        return GA_Range();
    return GA_Range(*group);
}

    
SYS_FORCE_INLINE GA_Range getRangeByAnyGroup(const GA_Group* const group) const
{ return getRangeByAnyGroup(static_cast<const GA_ElementGroup*>(group)); }

SYS_FORCE_INLINE GA_Range getRangeByAnyGroup(const GA_Group& group) const
{ return getRangeByAnyGroup(static_cast<const GA_ElementGroup&>(group)); }

SYS_FORCE_INLINE GA_SplittableRange getSplittableRangeByAnyGroup(const GA_ElementGroup* const group) const
{ return GA_SplittableRange(getRangeByAnyGroup(group)); }

SYS_FORCE_INLINE GA_SplittableRange getSplittableRangeByAnyGroup(const GA_ElementGroup& group) const
{ return getSplittableRangeByAnyGroup(&group); }
    
SYS_FORCE_INLINE GA_SplittableRange getSplittableRangeByAnyGroup(const GA_Group* const group) const
{ return getSplittableRangeByAnyGroup(static_cast<const GA_ElementGroup*>(group)); }

SYS_FORCE_INLINE GA_SplittableRange getSplittableRangeByAnyGroup(const GA_Group& group) const
{ return getSplittableRangeByAnyGroup(&group); }

    
public:







//     
//     
// SYS_FORCE_INLINE
// GA_Attribute*
// createDetachedAttribute(
//     const GA_AttributeOwner owner = GA_ATTRIB_POINT,
//     const GA_StorageClass storageClass = GA_STORECLASS_FLOAT,
//     const GA_Storage storage = GA_STORE_INVALID,
//     const bool emplaceBack = true,
//     const UT_StringRef& attribtype = "numeric",
//     const UT_Options* create_args = nullptr,
//     const GA_AttributeOptions* attribute_options = nullptr
// )
// {
//     const GA_Storage finalStorage = getPreferredStorage(storage, storageClass);
//
//     attribUPtrArray.emplace_back(createDetachedAttribute(owner, attribtype, create_args, attribute_options));
//     GA_Attribute* attribPtr = attribUPtrArray[attribUPtrArray.size() - 1].get();
//
//     if (emplaceBack)
//         attribArray.emplace_back(attribPtr);
//     return attribPtr;
// }
//
// SYS_FORCE_INLINE
// GA_Attribute*
// createDetachedAttribute(
//     const GA_AttributeOwner owner,
//     const GA_StorageClass storageClass,
//     const GA_Storage storage,
//     const bool emplaceBack,
//     const GA_AttributeType& attribtype,
//     const UT_Options* create_args = nullptr,
//     const GA_AttributeOptions* attribute_options = nullptr
// )
// {
//     const GA_Storage finalStorage = getPreferredStorage(storage, storageClass);
//
//     attribUPtrArray.emplace_back(createDetachedAttribute(owner, attribtype, create_args, attribute_options));
//     GA_Attribute* attribPtr = attribUPtrArray[attribUPtrArray.size() - 1].get();
//
//     if (emplaceBack)
//         attribArray.emplace_back(attribPtr);
//     return attribPtr;
// }
//
// SYS_FORCE_INLINE
// GA_Attribute*
// createDetachedTupleAttribute(
//     const GA_AttributeOwner owner = GA_ATTRIB_POINT,
//     const GA_StorageClass storageClass = GA_STORECLASS_FLOAT,
//     const GA_Storage storage = GA_STORE_INVALID,
//     const int tuple_size = 1,
//     const GA_Defaults& defaults = GA_Defaults(0.0f),
//     const bool emplaceBack = true,
//     const GA_AttributeOptions* attribute_options = nullptr
// )
// {
//     const GA_Storage finalStorage = getPreferredStorage(storage, storageClass);
//     
//     attribUPtrArray.emplace_back(createDetachedTupleAttribute(owner, finalStorage, tuple_size, defaults, attribute_options));
//     GA_Attribute* attribPtr = attribUPtrArray[attribUPtrArray.size() - 1].get();
//
//     if (emplaceBack)
//         attribArray.emplace_back(attribPtr);
//     return attribPtr;
// }
//
// SYS_FORCE_INLINE
// GA_Attribute*
// createDetachedArrayAttribute(
//     const GA_AttributeOwner owner = GA_ATTRIB_POINT,
//     const GA_StorageClass storageClass = GA_STORECLASS_FLOAT,
//     const GA_Storage storage = GA_STORE_INVALID,
//     const int tuple_size = 1,
//     const bool emplaceBack = true,
//     const UT_Options* create_args = nullptr,
//     const GA_AttributeOptions* attribute_options = nullptr
// )
// {
//     const GA_Storage finalStorage = getPreferredStorage(storage, storageClass);
//
//     attribUPtrArray.emplace_back(createDetachedAttribute(owner, "arraydata", create_args, attribute_options));
//     GA_Attribute* attribPtr = attribUPtrArray[attribUPtrArray.size() - 1].get();
//
//     if (emplaceBack)
//         attribArray.emplace_back(attribPtr);
//     return attribPtr;
// }
//
//
//
//
// GA_Attribute*
// findOrCreateTuple(
//     const bool detached = false,
//     const GA_AttributeOwner owner = GA_ATTRIB_POINT,
//     const GA_StorageClass storageClass = GA_STORECLASS_FLOAT,
//     const GA_Storage storage = GA_STORE_INVALID,
//     const UT_StringRef& attribName = "",
//     const int tuple_size = 1,
//     const GA_Defaults& defaults = GA_Defaults(0.0f),
//     const bool emplaceBack = true,
//     const UT_Options* create_args = nullptr,
//     const GA_AttributeOptions* attribute_options = nullptr
// )
// {
//     const GA_Storage finalStorage = getPreferredStorage(storage, storageClass);
//
//     GA_Attribute* attribPtr = findAttribute(owner, attribName);
//     if (attribPtr)
//     {
//         const GA_AIFTuple* const aifTuple = attribPtr->getAIFTuple();
//         if (!aifTuple ||
//             attribPtr->getTupleSize() != tuple_size ||
//             aifTuple->getStorage(attribPtr) != finalStorage ||
//             aifTuple->getDefaults(attribPtr) != defaults)
//         {
//             getAttributes().destroyAttribute(attribPtr);
//             attribPtr = nullptr;
//         }
//         else
//         {
//             if (emplaceBack)
//                 attribArray.emplace_back(attribPtr);
//             return attribPtr;
//         }
//     }
//
//     if (detached)
//     {
//         attribUPtrArray.emplace_back(createDetachedTupleAttribute(owner, finalStorage, 1));
//         attribPtr = attribUPtrArray[attribUPtrArray.size() - 1].get();
//         //attribPtr = attribUPtr.get();
//     }
//     else
//     {
//         if (!attribPtr)
//             attribPtr = createTupleAttribute(owner, attribName, finalStorage,
//                 tuple_size, defaults, create_args, attribute_options);
//
//         if (!attribPtr)
//         {
//             UT_ASSERT_MSG(attribPtr, "No Attrib");
//             return nullptr;
//         }
//     }
//     if (emplaceBack)
//         attribArray.emplace_back(attribPtr);
//     return attribPtr;
// }
//
//
//
//     GA_Attribute*
//     findOrCreateArray(
//         const bool detached = false,
//         const GA_AttributeOwner owner = GA_ATTRIB_POINT,
//         const GA_StorageClass storageClass = GA_STORECLASS_FLOAT,
//         const GA_Storage storage = GA_STORE_INVALID,
//         const UT_StringRef& attribName = "",
//         const int tuple_size = 1,
//         const bool emplaceBack = true,
//         const UT_Options* create_args = nullptr,
//         const GA_AttributeOptions* attribute_options = nullptr
//     )
//     {
//         const GA_Storage finalStorage = getPreferredStorage(storage, storageClass);
//
//         GA_Attribute* attribPtr = findAttribute(owner, attribName);
//         if (attribPtr)
//         {
//             const GA_AIFNumericArray* const aifNumericArray = attribPtr->getAIFNumericArray();
//             if (!aifNumericArray ||
//                 attribPtr->getTupleSize() != tuple_size ||
//                 aifNumericArray->getStorage(attribPtr) != finalStorage)
//             {
//                 getAttributes().destroyAttribute(attribPtr);
//                 attribPtr = nullptr;
//             }
//             else
//             {
//                 if (emplaceBack)
//                     attribArray.emplace_back(attribPtr);
//                 return attribPtr;
//             }
//         }
//
//         if (detached)
//         {
// #if 0
//             switch (finalStorage)
//             {
//             case GA_STORE_INT16:
//                 break;
//             case GA_STORE_INT32:
//                 break;
//             case GA_STORE_INT64:
//                 break;
//             case GA_STORE_REAL16:
//                 break;
//             case GA_STORE_REAL32:
//                 break;
//             case GA_STORE_REAL64:
//                 break;
//             case GA_STORE_STRING:
//                 break;
//             case GA_STORE_DICT:
//                 break;
//             default:
//                 break;
//             }
//             attribUPtrArray.emplace_back(static_cast<GEO_Detail*>(geo)->createDetachedAttribute(owner, "arraydata", tuple_size, create_args, attribute_options));
// #else
//             attribUPtrArray.emplace_back(getAttributes().createDetachedAttribute(owner, "arraydata", create_args, attribute_options));
// #endif
//             attribPtr = attribUPtrArray[attribUPtrArray.size() - 1].get();
//             //attribPtr = attribUPtr.get();
//         }
//         else
//         {
//             if (!attribPtr)
//                 attribPtr = getAttributes().createArrayAttribute(owner, GA_SCOPE_PUBLIC, attribName, finalStorage,
//                     tuple_size, create_args, attribute_options);
//
//             if (!attribPtr)
//             {
//                 UT_ASSERT_MSG(attribPtr, "No Attrib");
//                 return nullptr;
//             }
//         }
//         if (emplaceBack)
//             attribArray.emplace_back(attribPtr);
//         return attribPtr;
//     }
//
//
//
//     GA_Attribute*
//     findOrCreatePiece(
//         const bool detached = false,
//         const GFE_PieceAttribSearchOrder pieceAttribSearchOrder = GFE_PieceAttribSearchOrder::PRIM,
//         const GA_AttributeOwner owner = GA_ATTRIB_PRIMITIVE,
//         const GA_StorageClass storageClass = GA_STORECLASS_INT,
//         const GA_Storage storage = GA_STORE_INVALID,
//         const UT_StringRef& attribName = "",
//         const int tuple_size = 1,
//         const GA_Defaults& defaults = GA_Defaults(0.0f),
//         const bool emplaceBack = true,
//         const UT_Options* create_args = nullptr,
//         const GA_AttributeOptions* attribute_options = nullptr
//     )
//     {
//         const GA_Storage finalStorage = getPreferredStorage(storage, storageClass);
//
//         GA_Attribute* attribPtr = findPieceAttrib(pieceAttribSearchOrder, attribName);
//
//         // if (attribPtr)
//         // {
//         //     const bool promoteFromOtherClass = sopparms.getPromoteFromOtherClass();
//         //     if (promoteFromOtherClass)
//         //     {
//         //         if (geo0AttribClass != attribPtr->getOwner())
//         //         {
//         //             attribPtr = GFE_Attribpr::attribPromote(outGeo0, attribPtr, geo0AttribClass);
//         //             //attribPtr = GFE_AttribPromote::promote(*static_cast<GU_Detail*>(outGeo0), attribPtr, geo0AttribClass, sopparms.getDelOriginalAttrib(), GU_Promote::GU_PROMOTE_FIRST);
//         //         }
//         //     }
//         //
//         //     const bool forceCastAttribType = sopparms.getForceCastAttribType();
//         //     if (forceCastAttribType)
//         //     {
//         //         GFE_AttributeCast::attribCast(outGeo0, attribPtr, connectivityStorageClass, "", outGeo0->getPreferredPrecision());
//         //     }
//         //     return;
//         // }
//         
//         if (attribPtr)
//         {
//             const GA_AIFTuple* const aifTuple = attribPtr->getAIFTuple();
//             if (!aifTuple ||
//                 attribPtr->getTupleSize() != tuple_size ||
//                 aifTuple->getStorage(attribPtr) != finalStorage ||
//                 aifTuple->getDefaults(attribPtr) != defaults)
//             {
//                 getAttributes().destroyAttribute(attribPtr);
//                 attribPtr = nullptr;
//             }
//             else
//             {
//                 if (emplaceBack)
//                     attribArray.emplace_back(attribPtr);
//                 return attribPtr;
//             }
//         }
//
//         if (detached)
//         {
//             attribUPtrArray.emplace_back(createDetachedTupleAttribute(owner, finalStorage, 1));
//             attribPtr = attribUPtrArray[attribUPtrArray.size() - 1].get();
//             //attribPtr = attribUPtr.get();
//         }
//         else
//         {
//             if (!attribPtr)
//                 attribPtr = createTupleAttribute(owner, attribName, finalStorage,
//                     tuple_size, defaults, create_args, attribute_options);
//
//             if (!attribPtr)
//             {
//                 UT_ASSERT_MSG(attribPtr, "No Attrib");
//                 return nullptr;
//             }
//         }
//         if (emplaceBack)
//             attribArray.emplace_back(attribPtr);
//         return attribPtr;
//     }
//
//
//     GA_Attribute*
//     findPieceAttrib(
//         const GFE_PieceAttribSearchOrder pieceAttribSearchOrder,
//         const UT_StringRef& pieceAttribName
//     )
//     {
//         GA_Attribute* attribPtr = nullptr;
//
//         switch (pieceAttribSearchOrder)
//         {
//         case GFE_PieceAttribSearchOrder::PRIM:       attribPtr = findAttribute(GA_ATTRIB_PRIMITIVE, pieceAttribName); break;
//         case GFE_PieceAttribSearchOrder::POINT:      attribPtr = findAttribute(GA_ATTRIB_POINT,     pieceAttribName); break;
//         case GFE_PieceAttribSearchOrder::VERTEX:     attribPtr = findAttribute(GA_ATTRIB_VERTEX,    pieceAttribName); break;
//         case GFE_PieceAttribSearchOrder::PRIMPOINT:
//             {
//                 GA_AttributeOwner searchOrder[2] = { GA_ATTRIB_PRIMITIVE, GA_ATTRIB_POINT };
//                 attribPtr = findAttribute(pieceAttribName, searchOrder, 2);
//             }
//             break;
//         case GFE_PieceAttribSearchOrder::POINTPRIM:
//             {
//                 GA_AttributeOwner searchOrder[2] = { GA_ATTRIB_POINT, GA_ATTRIB_PRIMITIVE };
//                 attribPtr = findAttribute(pieceAttribName, searchOrder, 2);
//             }
//             break;
//         case GFE_PieceAttribSearchOrder::ALL:
//             {
//                 GA_AttributeOwner searchOrder[3] = { GA_ATTRIB_PRIMITIVE, GA_ATTRIB_POINT, GA_ATTRIB_VERTEX };
//                 attribPtr = findAttribute(pieceAttribName, searchOrder, 3);
//             }
//             break;
//         default:
//             UT_ASSERT_MSG(0, "Unhandled Geo Piece Attrib Search Order!");
//             break;
//         }
//         return attribPtr;
//     }
//
//
//
//
//
//     
// GA_Attribute*
// findOrCreateUV(
//     const bool detached = false,
//     const GA_AttributeOwner owner = GA_ATTRIB_POINT,
//     const GA_Storage storage = GA_STORE_INVALID,
//     const UT_StringRef& attribName = "",
//     const int tuple_size = 3,
//     const GA_Defaults& defaults = GA_Defaults(0.0f),
//     const bool emplaceBack = true,
//     const UT_Options* create_args = nullptr,
//     const GA_AttributeOptions* attribute_options = nullptr,
//     const GA_StorageClass storageClass = GA_STORECLASS_FLOAT
// )
// {
//     const GA_Storage finalStorage = getPreferredStorage(storage, storageClass);
//
//     GA_Attribute* attribPtr = GFE_Attribute::findAttributePointVertex(owner, attribName);
//     if (attribPtr)
//     {
//         const GA_AIFTuple* const aifTuple = attribPtr->getAIFTuple();
//         if (!aifTuple ||
//             attribPtr->getTupleSize() != tuple_size ||
//             aifTuple->getStorage(attribPtr) != finalStorage ||
//             aifTuple->getDefaults(attribPtr) != defaults)
//         {
//             getAttributes().destroyAttribute(attribPtr);
//             attribPtr = nullptr;
//         }
//         else
//         {
//             if (emplaceBack)
//                 attribArray.emplace_back(attribPtr);
//             return attribPtr;
//         }
//     }
//
//     const GA_AttributeOwner validOwner = owner == GA_ATTRIB_POINT ? GA_ATTRIB_POINT : GA_ATTRIB_VERTEX;
//     if (detached)
//     {
//         attribUPtrArray.emplace_back(createDetachedTupleAttribute(validOwner, finalStorage, 1));
//         attribPtr = attribUPtrArray[attribUPtrArray.size() - 1].get();
//         //attribPtr = attribUPtr.get();
//     }
//     else
//     {
//         if (!attribPtr)
//         {
// #if 1
//             const UT_Options& finalCreateArgs = create_args ? *create_args : UT_Options("uvw");
//
//             //GA_AttributeOptions& finalOptions = attribute_options ? *attribute_options : GA_AttributeOptions();
//             if (attribute_options)
//             {
//                 attribPtr = createTupleAttribute(validOwner, attribName, finalStorage,
//                     tuple_size, defaults, &finalCreateArgs, attribute_options);
//             }
//             else
//             {
//                 GA_AttributeOptions finalOptions = GA_AttributeOptions();
//                 finalOptions.setTypeInfo(GA_TYPE_TEXTURE_COORD);
//                 attribPtr = createTupleAttribute(validOwner, attribName, finalStorage,
//                     tuple_size, defaults, &finalCreateArgs, &finalOptions);
//             }
// #else
//             attribPtr = static_cast<GEO_Detail*>(geo)->addTextureAttribute(owner == GA_ATTRIB_POINT ? GA_ATTRIB_POINT : GA_ATTRIB_VERTEX, finalStorage);
// #endif
//         }
//
//         if (!attribPtr)
//         {
//             UT_ASSERT_MSG(attribPtr, "No Attrib");
//             return nullptr;
//         }
//         GFE_Attribute::renameAttribute(*attribPtr, attribName);
//     }
//     if (emplaceBack)
//         attribArray.emplace_back(attribPtr);
//     return attribPtr;
// }
//
// GA_Attribute*
// findOrCreateDir(
//     const bool detached = false,
//     const GA_AttributeOwner owner = GA_ATTRIB_POINT,
//     const GA_Storage storage = GA_STORE_INVALID,
//     const UT_StringRef& attribName = "",
//     const int tuple_size = 3,
//     const GA_Defaults& defaults = GA_Defaults(0.0f),
//     const bool emplaceBack = true,
//     const UT_Options* create_args = nullptr,
//     const GA_AttributeOptions* attribute_options = nullptr,
//     const GA_StorageClass storageClass = GA_STORECLASS_FLOAT
// )
// {
//     const GA_Storage finalStorage = getPreferredStorage(storage, storageClass);
//
//     GA_Attribute* attribPtr = findAttribute(owner, attribName);
//     if (attribPtr)
//     {
//         const GA_AIFTuple* const aifTuple = attribPtr->getAIFTuple();
//         if (!aifTuple ||
//             attribPtr->getTupleSize() != tuple_size ||
//             aifTuple->getStorage(attribPtr) != finalStorage ||
//             aifTuple->getDefaults(attribPtr) != defaults)
//         {
//             getAttributes().destroyAttribute(attribPtr);
//             attribPtr = nullptr;
//         }
//         else
//         {
//             if (emplaceBack)
//                 attribArray.emplace_back(attribPtr);
//             return attribPtr;
//         }
//     }
//
//     if (detached)
//     {
//         attribUPtrArray.emplace_back(createDetachedTupleAttribute(owner, finalStorage, 1));
//         attribPtr = attribUPtrArray[attribUPtrArray.size() - 1].get();
//         //attribPtr = attribUPtr.get();
//     }
//     else
//     {
//         if (!attribPtr)
//         {
// #if 1
//             const UT_Options& finalCreateArgs = create_args ? *create_args : UT_Options("vector3");
//
//             //GA_AttributeOptions& finalOptions = attribute_options ? *attribute_options : GA_AttributeOptions();
//             if (attribute_options)
//             {
//                 attribPtr = createTupleAttribute(owner, attribName, finalStorage,
//                     tuple_size, defaults, &finalCreateArgs, attribute_options);
//             }
//             else
//             {
//                 GA_AttributeOptions finalOptions = GA_AttributeOptions();
//                 finalOptions.setTypeInfo(GA_TYPE_NORMAL);
//                 attribPtr = createTupleAttribute(owner, attribName, finalStorage,
//                     tuple_size, defaults, &finalCreateArgs, &finalOptions);
//             }
// #else
//             attribPtr = static_cast<GEO_Detail*>(geo)->addTextureAttribute(owner == GA_ATTRIB_POINT ? GA_ATTRIB_POINT : GA_ATTRIB_VERTEX, finalStorage);
// #endif
//         }
//
//         if (!attribPtr)
//         {
//             if (cookparms)
//                 cookparms->sopAddError(SOP_ATTRIBUTE_INVALID, attribName);
//             UT_ASSERT_MSG(attribPtr, "No Attrib");
//             return nullptr;
//         }
//         GFE_Attribute::renameAttribute(*attribPtr, attribName);
//     }
//     if (emplaceBack)
//         attribArray.emplace_back(attribPtr);
//     return attribPtr;
// }
//
//
// GA_Attribute*
// findOrCreateNormal3D(
//     const bool detached = false,
//     const GFE_NormalSearchOrder owner = GFE_NormalSearchOrder::ALL,
//     const GA_Storage storage = GA_STORE_INVALID,
//     const UT_StringRef& attribName = "",
//     const int tuple_size = 3,
//     const GA_Defaults& defaults = GA_Defaults(0.0f),
//     const bool emplaceBack = true,
//     const UT_Options* create_args = nullptr,
//     const GA_AttributeOptions* attribute_options = nullptr,
//     const GA_StorageClass storageClass = GA_STORECLASS_FLOAT
// )
// {
//     const GA_Storage finalStorage = getPreferredStorage(storage, storageClass);
//
//
//     GA_Attribute* attribPtr = GFE_Attribute::findNormal3D(owner, attribName);
//     if (attribPtr)
//     {
//         const GA_AIFTuple* const aifTuple = attribPtr->getAIFTuple();
//         if (!aifTuple ||
//             attribPtr->getTupleSize() != tuple_size ||
//             aifTuple->getStorage(attribPtr) != finalStorage ||
//             aifTuple->getDefaults(attribPtr) != defaults)
//         {
//             getAttributes().destroyAttribute(attribPtr);
//             attribPtr = nullptr;
//         }
//         else
//         {
//             if (emplaceBack)
//                 attribArray.emplace_back(attribPtr);
//             return attribPtr;
//         }
//     }
//
//     const GA_AttributeOwner validOwner = GFE_Attribute::toValidOwner(owner);
//     if (detached)
//     {
//         attribUPtrArray.emplace_back(createDetachedTupleAttribute(validOwner, finalStorage, 1));
//         attribPtr = attribUPtrArray[attribUPtrArray.size() - 1].get();
//         //attribPtr = attribUPtr.get();
//     }
//     else
//     {
//         if (!attribPtr)
//         {
// #if 1
//             const UT_Options& finalCreateArgs = create_args ? *create_args : UT_Options("vector3");
//
//             //GA_AttributeOptions& finalOptions = attribute_options ? *attribute_options : GA_AttributeOptions();
//             if (attribute_options)
//             {
//                 attribPtr = createTupleAttribute(validOwner, attribName, finalStorage,
//                     tuple_size, defaults, &finalCreateArgs, attribute_options);
//             }
//             else
//             {
//                 GA_AttributeOptions finalOptions = GA_AttributeOptions();
//                 finalOptions.setTypeInfo(GA_TYPE_NORMAL);
//                 attribPtr = createTupleAttribute(validOwner, attribName, finalStorage,
//                     tuple_size, defaults, &finalCreateArgs, &finalOptions);
//             }
// #else
//             attribPtr = static_cast<GEO_Detail*>(geo)->addTextureAttribute(owner == GA_ATTRIB_POINT ? GA_ATTRIB_POINT : GA_ATTRIB_VERTEX, finalStorage);
// #endif
//         }
//
//         if (!attribPtr)
//         {
//             UT_ASSERT_MSG(attribPtr, "No Attrib");
//             return nullptr;
//         }
//         GFE_Attribute::renameAttribute(*attribPtr, attribName);
//     }
//     if (emplaceBack)
//         attribArray.emplace_back(attribPtr);
//     return attribPtr;
// }


public:
    
    void groupIntersect(GA_VertexGroup& group, const GA_PointGroup* const groupRef, const exint subscribeRatio = 64, const exint minGrainSize = 1024) const
    {
        if (!groupRef)
            return;
        const GA_SplittableRange geoSplittableRange(getRange(groupRef, true));
        UTparallelFor(geoSplittableRange, [this, &group](const GA_SplittableRange& r)
        {
            GA_Offset start, end;
            for (GA_Iterator it(r); it.blockAdvance(start, end); )
            {
                
                for (GA_Offset elemoff = start; elemoff < end; ++elemoff)
                {
                    for (GA_Offset promoff = pointVertex(elemoff); gfe::isValidOffset(promoff); promoff = vertexToNextVertex(promoff))
                    {
                        group.setElement(promoff, false);
                    }
                }
            }
        }, subscribeRatio, minGrainSize);


    }
    

    


    

private:

    GA_Group* findGroupBase(const GA_GroupType groupType, const UT_StringRef& groupName) const
    {
        const GA_GroupTable* const groupTable = getGroupTable(groupType);
        if (!groupTable)
            return nullptr;
        return groupTable->find(groupName);
    }
    
    GA_Group* findGroupN(const UT_StringRef& groupName) const
    {
        GA_Group* outGroup = findGroupBase(GA_GROUP_PRIMITIVE, groupName);
        if (outGroup)
            return outGroup;
        outGroup = findGroupBase(GA_GROUP_POINT, groupName);
        if (outGroup)
            return outGroup;
        outGroup = findGroupBase(GA_GROUP_VERTEX, groupName);
        if (outGroup)
            return outGroup;
        return findGroupBase(GA_GROUP_EDGE, groupName);
    }


}; // End of Class Geometry
using GFE_Detail = Geometry;
_GFE_END
#endif
