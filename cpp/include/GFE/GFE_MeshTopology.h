
#pragma once

#ifndef __GFE_MeshTopology_h__
#define __GFE_MeshTopology_h__

#include <GFE/GFE_MeshTopology.h>

#include <GFE/GeoFilter.h>

#include <GFE/ArrayOp.h>
#include <GFE/GFE_OffsetArrayAttributeToGeometry.h>
#include <GFE/GFE_OffsetAttributeToIndex.h>


#if 0
    MeshTopology meshTopology(geo, cookparms);
    meshTopology.groupParser.setGroup(groupParser);
    meshTopology.setVertexPrimIndex(false, vertexPrimIndexAttribName);
    const GA_VertexGroup* const vtxNextEquivNoLoopGroup = meshTopology.setVertexNextEquivNoLoopGroup(true);
    meshTopology.compute();
#endif


#define __GFE_MeshTopology_TestSpped 0
#define __GFE_MeshTopology_OptimizeTrivialVertices 1
#define __GFE_MeshTopology_VertexNextEquivDual 1


_GFEL_BEGIN
class MeshTopology : public AttribFilter {

public:
    enum Type {
        VertexPrimIndex,
        VertexVertexPrim,
        VertexVertexPrimPrev,
        VertexVertexPrimNext,
        VertexNextEquiv,
        VertexNextEquivNoLoop,
        VertexPointDst,
        PointPointEdge,
        PointPointPrim,
        PrimPrimEdge,
        PrimPrimPoint,
    };


    //using AttribFilter::AttribFilter;

    MeshTopology(
        GFE_Detail* const geo,
        const SOP_NodeVerb::CookParms* const cookparms = nullptr
    )
        : AttribFilter(geo, cookparms)
        , pointSeamGroup(geo, groupParser.getGOPRef(), cookparms)
        , vertexEdgeSeamGroup(geo, groupParser.getGOPRef(), cookparms)
        , edgeSeamGroup(geo, groupParser.getGOPRef(), cookparms)
    {
    }

    MeshTopology(
        GA_Detail& geo,
        const SOP_NodeVerb::CookParms* const cookparms = nullptr
    )
        : AttribFilter(geo, cookparms)
        , pointSeamGroup(geo, groupParser.getGOPRef(), cookparms)
        , vertexEdgeSeamGroup(geo, groupParser.getGOPRef(), cookparms)
        , edgeSeamGroup(geo, groupParser.getGOPRef(), cookparms)
    {
    }

    MeshTopology(
        GA_Detail* const geo,
        const SOP_NodeVerb::CookParms* const cookparms = nullptr
    )
        : AttribFilter(geo, cookparms)
        , pointSeamGroup(geo, groupParser.getGOPRef(), cookparms)
        , vertexEdgeSeamGroup(geo, groupParser.getGOPRef(), cookparms)
        , edgeSeamGroup(geo, groupParser.getGOPRef(), cookparms)
    {
    }

    void clear()
    {
        vertexPrimIndexAttrib       = nullptr;
        vertexVertexPrimPrevAttrib  = nullptr;
        vertexVertexPrimNextAttrib  = nullptr;
        vertexPointDstAttrib        = nullptr;
        vertexNextEquivAttrib       = nullptr;
        vertexNextEquivNoLoopAttrib = nullptr;
        pointPointEdgeAttrib        = nullptr;
        pointPointPrimAttrib        = nullptr;
        primPrimEdgeAttrib          = nullptr;
        primPrimPointAttrib         = nullptr;
        vertexNextEquivGroup        = nullptr;
        vertexNextEquivNoLoopGroup  = nullptr;
        //unsharedEdgeGroup           = nullptr;
    }
    
    virtual void reset(GA_Detail& inGeo, const SOP_NodeVerb::CookParms* const inCookparms = nullptr) override
    {
        if (geo == &inGeo && cookparms == inCookparms)
            return;
        AttribFilter::reset(inGeo, inCookparms);
        pointSeamGroup.reset(inGeo, inCookparms);
        vertexEdgeSeamGroup.reset(inGeo, inCookparms);
        edgeSeamGroup.reset(inGeo, inCookparms);
        clear();
    }

    SYS_FORCE_INLINE virtual void reset(GA_Detail* const geo, const SOP_NodeVerb::CookParms* const cookparms = nullptr) override
    { reset(*geo, cookparms); }


    
    void setComputeParm(
        const bool outAsOffset = true,
        const exint subscribeRatio = 64,
        const exint minGrainSize   = 1024
    )
    {
        setHasComputed();
        this->outAsOffset = outAsOffset;
        this->subscribeRatio = subscribeRatio;
        this->minGrainSize   = minGrainSize;
    }

    SYS_FORCE_INLINE void setKernel(const uint8 kernel)
    { this->kernel = kernel; }



    // SYS_FORCE_INLINE GA_EdgeGroup* setUnsharedEdgeGroup(const bool detached = false, const UT_StringRef& groupName = "__topo_unshared")
    // {
    //     unsharedEdgeGroup = getOutGroupArray().findOrCreateEdge(detached, groupName);
    //     return unsharedEdgeGroup;
    // }
    // SYS_FORCE_INLINE void setUnsharedEdgeGroup(GA_EdgeGroup* const groupPtr)
    // { unsharedEdgeGroup = groupPtr; }
    // 
    // SYS_FORCE_INLINE GA_EdgeGroup* getUnsharedEdgeGroup() const
    // { return unsharedEdgeGroup; }
    


    

#define GFE_SETATTRIB_SPECIALIZATION(ATTRIB_NAME_UPPER, ATTRIB_NAME_LOWNER, ATTRIB_OWNER)                 \
    SYS_FORCE_INLINE GA_Attribute* set##ATTRIB_NAME_UPPER(                                                \
        const bool detached = true,                                                                       \
        const UT_StringRef& attribName = "__topo_"#ATTRIB_NAME_LOWNER,                                    \
        const GA_Storage storage = GA_STORE_INVALID,                                                      \
        const bool findInputAttrib = false                                                                \
    )                                                                                                     \
    {                                                                                                     \
        if (findInputAttrib)                                                                                          \
        {                                                                                                             \
            ATTRIB_NAME_LOWNER##Attrib = geo->findAttribute(ATTRIB_OWNER, attribName);                                \
            if (ATTRIB_NAME_LOWNER##Attrib)                                                                           \
            {                                                                                                         \
                if (gfe::checkTupleAttrib(ATTRIB_NAME_LOWNER##Attrib, GA_STORECLASS_INT, storage, 1, nullptr))   \
                {                                                                                                     \
                    cal##ATTRIB_NAME_UPPER##Attrib = false;                                                           \
                    return ATTRIB_NAME_LOWNER##Attrib;                                                                \
                }                                                                                                     \
            }                                                                                                         \
        }                                                                                                             \
        cal##ATTRIB_NAME_UPPER##Attrib = true;                                                            \
        ATTRIB_NAME_LOWNER##Attrib = getOutAttribArray().findOrCreateTuple(detached,                      \
            ATTRIB_OWNER, GA_STORECLASS_INT, storage, attribName, 1, GA_Defaults(gfe::INVALID_OFFSET));    \
        return ATTRIB_NAME_LOWNER##Attrib;                                                                \
    }                                                                                                     \
    SYS_FORCE_INLINE void set##ATTRIB_NAME_UPPER(GA_Attribute* const attribPtr, const bool cal = true)    \
    { ATTRIB_NAME_LOWNER##Attrib = attribPtr; cal##ATTRIB_NAME_UPPER##Attrib = cal; }                     \
    
    GFE_SETATTRIB_SPECIALIZATION(VertexVertexPrimPrev,  vertexVertexPrimPrev,  GA_ATTRIB_VERTEX)
    GFE_SETATTRIB_SPECIALIZATION(VertexVertexPrimNext,  vertexVertexPrimNext,  GA_ATTRIB_VERTEX)
    GFE_SETATTRIB_SPECIALIZATION(VertexPrimIndex,       vertexPrimIndex,       GA_ATTRIB_VERTEX)
    GFE_SETATTRIB_SPECIALIZATION(VertexPointDst,        vertexPointDst,        GA_ATTRIB_VERTEX)
    GFE_SETATTRIB_SPECIALIZATION(VertexNextEquiv,       vertexNextEquiv,       GA_ATTRIB_VERTEX)
    GFE_SETATTRIB_SPECIALIZATION(VertexNextEquivNoLoop, vertexNextEquivNoLoop, GA_ATTRIB_VERTEX)

#undef GFE_SETATTRIB_SPECIALIZATION
    

    SYS_FORCE_INLINE void setVertexVertexPrim(
        const bool detached = true,
        const UT_StringRef& attribName0 = "__topo_vertexVertexPrimPrev",
        const UT_StringRef& attribName1 = "__topo_vertexVertexPrimNext",
        const GA_Storage storage = GA_STORE_INVALID,
        const bool findInputAttrib = false
    )
    {
        setVertexVertexPrimPrev(detached, attribName0, storage, findInputAttrib);
        setVertexVertexPrimNext(detached, attribName1, storage, findInputAttrib);
        //vertexVertexPrimPrevAttrib = getOutAttribArray().findOrCreateTuple(detached,
        //    GA_ATTRIB_VERTEX, GA_STORECLASS_INT, storage, attribName0, 1, GA_Defaults(gfe::INVALID_OFFSET));
        //vertexVertexPrimNextAttrib = getOutAttribArray().findOrCreateTuple(detached,
        //    GA_ATTRIB_VERTEX, GA_STORECLASS_INT, storage, attribName1, 1, GA_Defaults(gfe::INVALID_OFFSET));
    }
    SYS_FORCE_INLINE void setVertexVertexPrim(GA_Attribute* const attribPtr0, GA_Attribute* const attribPtr1, const bool cal = true)
    {
        setVertexVertexPrimPrev(attribPtr0, cal);
        setVertexVertexPrimNext(attribPtr1, cal);
        //vertexVertexPrimPrevAttrib = attribPtr0;
        //vertexVertexPrimNextAttrib = attribPtr1;
    }
    
        



#define GFE_SETATTRIB_ARRAY_SPECIALIZATION(ATTRIB_NAME_UPPER, ATTRIB_NAME_LOWNER, ATTRIB_OWNER)  \
    SYS_FORCE_INLINE GA_Attribute* set##ATTRIB_NAME_UPPER(                                       \
        const bool detached = false,                                                             \
        const UT_StringRef& attribName = "__topo_"#ATTRIB_NAME_LOWNER,                           \
        const GA_Storage storage = GA_STORE_INVALID,                                             \
        const bool findInputAttrib = false                                                       \
    )                                                                                            \
    {                                                                                            \
        if (findInputAttrib)                                                                                \
        {                                                                                                   \
            ATTRIB_NAME_LOWNER##Attrib = geo->findAttribute(ATTRIB_OWNER, attribName);                      \
            if (ATTRIB_NAME_LOWNER##Attrib)                                                                 \
            {                                                                                               \
                if (gfe::checkArrayAttrib(ATTRIB_NAME_LOWNER##Attrib, GA_STORECLASS_INT, storage, 1))  \
                {                                                                                           \
                    cal##ATTRIB_NAME_UPPER##Attrib = false;                                                 \
                    return ATTRIB_NAME_LOWNER##Attrib;                                                      \
                }                                                                                           \
            }                                                                                               \
        }                                                                                                   \
        cal##ATTRIB_NAME_UPPER##Attrib = true;                                                   \
        ATTRIB_NAME_LOWNER##Attrib = getOutAttribArray().findOrCreateArray(detached,             \
            ATTRIB_OWNER, GA_STORECLASS_INT, storage, attribName);                               \
        return ATTRIB_NAME_LOWNER##Attrib;                                                       \
    }                                                                                            \
    SYS_FORCE_INLINE void set##ATTRIB_NAME_UPPER(GA_Attribute* const attribPtr, const bool cal = true)    \
    { ATTRIB_NAME_LOWNER##Attrib = attribPtr; cal##ATTRIB_NAME_UPPER##Attrib = cal; }                     \

    GFE_SETATTRIB_ARRAY_SPECIALIZATION(PointPointEdge, pointPointEdge, GA_ATTRIB_POINT)
    GFE_SETATTRIB_ARRAY_SPECIALIZATION(PointPointPrim, pointPointPrim, GA_ATTRIB_POINT)
    GFE_SETATTRIB_ARRAY_SPECIALIZATION(PrimPrimEdge,   primPrimEdge,   GA_ATTRIB_PRIMITIVE)
    GFE_SETATTRIB_ARRAY_SPECIALIZATION(PrimPrimPoint,  primPrimPoint,  GA_ATTRIB_PRIMITIVE)

#undef GFE_SETATTRIB_ARRAY_SPECIALIZATION



    

#define GFE_SETGROUP_SPECIALIZATION(ATTRIB_NAME_UPPER, ATTRIB_NAME_LOWNER, ATTRIB_OWNER)     \
    SYS_FORCE_INLINE GA_VertexGroup* set##ATTRIB_NAME_UPPER##Group(                          \
        const bool detached = true,                                                          \
        const UT_StringRef& groupName = "__topo_"#ATTRIB_NAME_LOWNER,                        \
        const bool findInput = false                                                         \
    )                                                                                        \
    {                                                                                        \
        if (findInput)                                                                       \
        {                                                                                    \
            ATTRIB_NAME_LOWNER##Group = geo->findVertexGroup(groupName);                     \
            if (ATTRIB_NAME_LOWNER##Group)                                                   \
            {                                                                                \
                cal##ATTRIB_NAME_UPPER##Group = false;                                       \
                return ATTRIB_NAME_LOWNER##Group;                                            \
            }                                                                                \
        }                                                                                    \
        cal##ATTRIB_NAME_UPPER##Group = true;                                                \
        ATTRIB_NAME_LOWNER##Group = static_cast<GA_VertexGroup*>(                            \
            getOutGroupArray().findOrCreate(detached, ATTRIB_OWNER, groupName));             \
        return ATTRIB_NAME_LOWNER##Group;                                                    \
    }                                                                                        \
    SYS_FORCE_INLINE void set##ATTRIB_NAME_UPPER(GA_VertexGroup* const groupPtr, const bool cal = true) \
    { ATTRIB_NAME_LOWNER##Group = groupPtr; cal##ATTRIB_NAME_UPPER##Group = cal; }                      \
    
    GFE_SETGROUP_SPECIALIZATION(VertexNextEquiv,        vertexNextEquiv,        GA_GROUP_VERTEX)
    GFE_SETGROUP_SPECIALIZATION(VertexNextEquivNoLoop,  vertexNextEquivNoLoop,  GA_GROUP_VERTEX)
    
#undef GFE_SETGROUP_SPECIALIZATION

    

#define GFE_GETATTRIB_SPECIALIZATION(ATTRIB_NAME_UPPER, ATTRIB_NAME_LOWNER)   \
    SYS_FORCE_INLINE GA_Attribute* get##ATTRIB_NAME_UPPER() const             \
    { return ATTRIB_NAME_LOWNER##Attrib; }                                    \

    GFE_GETATTRIB_SPECIALIZATION(VertexPrimIndex,       vertexPrimIndex)
    GFE_GETATTRIB_SPECIALIZATION(VertexPointDst,        vertexPointDst)
    GFE_GETATTRIB_SPECIALIZATION(VertexNextEquiv,       vertexNextEquiv)
    GFE_GETATTRIB_SPECIALIZATION(VertexNextEquivNoLoop, vertexNextEquivNoLoop)
    GFE_GETATTRIB_SPECIALIZATION(VertexVertexPrimPrev,  vertexVertexPrimPrev)
    GFE_GETATTRIB_SPECIALIZATION(VertexVertexPrimNext,  vertexVertexPrimNext)
    GFE_GETATTRIB_SPECIALIZATION(PointPointEdge,        pointPointEdge)
    GFE_GETATTRIB_SPECIALIZATION(PointPointPrim,        pointPointPrim)
    GFE_GETATTRIB_SPECIALIZATION(PrimPrimEdge,          primPrimEdge)
    GFE_GETATTRIB_SPECIALIZATION(PrimPrimPoint,         primPrimPoint)

#undef GFE_GETATTRIB_SPECIALIZATION

        

#define GFE_GETGROUP_SPECIALIZATION(ATTRIB_NAME_UPPER, ATTRIB_NAME_LOWNER)     \
SYS_FORCE_INLINE GA_VertexGroup* get##ATTRIB_NAME_UPPER##Group() const         \
{ return ATTRIB_NAME_LOWNER##Group; }                                          \

GFE_GETGROUP_SPECIALIZATION(VertexNextEquiv,       vertexNextEquiv)
GFE_GETGROUP_SPECIALIZATION(VertexNextEquivNoLoop, vertexNextEquivNoLoop)

#undef GFE_GETGROUP_SPECIALIZATION




    GA_Attribute* setAdjacency(
        const bool detached = false,
        const GA_AttributeOwner attribOwner = GA_ATTRIB_POINT,
        const bool largeConnectivity = true,
        const UT_StringRef& attribName = "__topo_adjacency",
        const GA_Storage storage = GA_STORE_INVALID
    )
    {
        if (largeConnectivity)
        {
            switch (attribOwner)
            {
            case GA_ATTRIB_PRIMITIVE: return setPrimPrimPoint (detached, attribName, storage); break;
            case GA_ATTRIB_POINT:     return setPointPointPrim(detached, attribName, storage); break;
            case GA_ATTRIB_VERTEX:    break;
            }
        }
        else
        {
            switch (attribOwner)
            {
            case GA_ATTRIB_PRIMITIVE: return setPrimPrimEdge  (detached, attribName, storage); break;
            case GA_ATTRIB_POINT:     return setPointPointEdge(detached, attribName, storage); break;
            case GA_ATTRIB_VERTEX:    break;
            }
        }
        return nullptr;
    }
   
    
    SYS_FORCE_INLINE GA_Attribute* setAdjacency(
        const bool detached = false,
        const GA_GroupType groupType = GA_GROUP_POINT,
        const bool largeConnectivity = true,
        const UT_StringRef& attribName = "__topo_adjacency",
        const GA_Storage storage = GA_STORE_INVALID
    )
    { return setAdjacency(detached, gfe::attributeOwner_groupType(groupType), largeConnectivity, attribName, storage); }
   



    
private:



    
    virtual bool
        computeCore() override
    {
        if (!vertexPrimIndexAttrib       &&
            !vertexVertexPrimPrevAttrib  &&
            !vertexVertexPrimNextAttrib  &&
            !vertexPointDstAttrib        &&
            !vertexNextEquivAttrib       &&
            !vertexNextEquivNoLoopAttrib &&
            !pointPointEdgeAttrib        &&
            !pointPointPrimAttrib        &&
            !primPrimEdgeAttrib          &&
            !primPrimPointAttrib         &&
            !vertexNextEquivGroup        &&
            !vertexNextEquivNoLoopGroup)
            return false;

        if (!calVertexPrimIndexAttrib       &&
            !calVertexVertexPrimPrevAttrib  &&
            !calVertexVertexPrimNextAttrib  &&
            !calVertexPointDstAttrib        &&
            !calVertexNextEquivAttrib       &&
            !calVertexNextEquivNoLoopAttrib &&
            !calPointPointEdgeAttrib        &&
            !calPointPointPrimAttrib        &&
            !calPrimPrimEdgeAttrib          &&
            !calPrimPrimPointAttrib         &&
            !calVertexNextEquivGroup        &&
            !calVertexNextEquivNoLoopGroup)
            return false;

        if (groupParser.isEmpty())
            return true;
        
        //if (vertexEdgeSeamGroup)
        //    GFE_GroupUnion::groupUnion(vertexEdgeSeamGroup, edgeSeamGroup);

        
#if __GFE_MeshTopology_VertexNextEquivDual
        if (primPrimEdgeAttrib && !vertexNextEquivAttrib)
            setVertexNextEquiv(!outIntermediateAttrib);
        
        const bool calVertexNextEquivDual =
                  (calVertexNextEquivNoLoopAttrib ||
                   calVertexNextEquivNoLoopGroup  ||
                   calVertexNextEquivAttrib       ||
                   calVertexNextEquivGroup        ||
                   calPrimPrimEdgeAttrib)         &&
                       
                  (vertexNextEquivNoLoopAttrib    ||
                   vertexNextEquivNoLoopGroup     ||
                   vertexNextEquivAttrib          ||
                   vertexNextEquivGroup           ||
                   primPrimEdgeAttrib);

#else
        const bool calVertexNextEquivNoLoop =
                   vertexNextEquivNoLoopAttrib ||
                   vertexNextEquivNoLoopGroup;

        const bool calVertexNextEquiv =
                   vertexNextEquivAttrib       ||
                   vertexNextEquivGroup        ||
                   primPrimEdgeAttrib;
        
        const bool calVertexNextEquivDual =
                   calVertexNextEquivNoLoop    ||
                   calVertexNextEquiv;
#endif
        
        const bool calVertexVertexPrim =
                  (calVertexVertexPrimPrevAttrib  ||
                   calVertexVertexPrimNextAttrib) &&
                  (vertexVertexPrimPrevAttrib     ||
                   vertexVertexPrimNextAttrib);
        
        const bool calVertexPointDst =
                   calVertexPointDstAttrib        &&
                  (vertexPointDstAttrib           ||
                   calVertexNextEquivDual);
        
        //const bool calVertexPrimIndex =
        //           vertexPrimIndexAttrib        ||
        //           pointPointEdgeAttrib;
        
        const bool calVertexPrimIndex =
                   calVertexPrimIndexAttrib       &&
                  (vertexPrimIndexAttrib          ||
                  (pointPointEdgeAttrib && !(vertexVertexPrimPrevAttrib && vertexVertexPrimNextAttrib)));
        
        
        
        if (calVertexPrimIndex)
        {
            if (calVertexPointDst)
            {
                vertexPrimIndex_vertexPointDst();
            }
            else
            {
#if __GFE_MeshTopology_TestSpped
                switch (kernel)
                {
                default:
                case 0: vertexPrimIndex();  break;
                case 1: vertexPrimIndex1(); break;
                }
#else
                vertexPrimIndex();
#endif
            }
            //calVertexPointDst ? vertexPrimIndex_vertexPointDst() : vertexPrimIndex();
        }
        
        
        if (calVertexVertexPrim)
        {
            calTmpVertexVertexPrimPrevAttrib = calVertexVertexPrimPrevAttrib && vertexVertexPrimPrevAttrib;
            calTmpVertexVertexPrimNextAttrib = calVertexVertexPrimNextAttrib && vertexVertexPrimNextAttrib;
#if __GFE_MeshTopology_TestSpped
            switch (pointPointEdgeAttrib ? 0 : kernel)
            {
            default:
            case 0: vertexVertexPrim();       break;
            case 1: vertexVertexPrimSideFx(); break;
            }
#else
            vertexVertexPrim();
#endif
        }
        
        if (calVertexPointDst && !calVertexPrimIndex)
        {
            if (vertexVertexPrimNextAttrib)
                vertexPointDstByVertexVertexPrimNext();
            else if (vertexPrimIndexAttrib)
                vertexPointDstByVtxpnum();
            else
                vertexPointDst();
        }
        
#if __GFE_MeshTopology_VertexNextEquivDual
        if (calVertexNextEquivDual)
        {
            calTmpVertexNextEquivAttrib       = calVertexNextEquivAttrib       && vertexNextEquivAttrib      ;
            calTmpVertexNextEquivNoLoopAttrib = calVertexNextEquivNoLoopAttrib && vertexNextEquivNoLoopAttrib;
            calTmpVertexNextEquivGroup        = calVertexNextEquivGroup        && vertexNextEquivGroup       ;
            calTmpVertexNextEquivNoLoopGroup  = calVertexNextEquivNoLoopGroup  && vertexNextEquivNoLoopGroup ;
            vertexNextEquivDualByVertexPointDst();
        }
#else
        if (calVertexNextEquiv)
            vertexNextEquiv();
        if (calVertexNextEquivNoLoop)
            vertexNextEquivNoLoop();
#endif
            
        if (calPointPointEdgeAttrib && pointPointEdgeAttrib)
        {
            if (vertexVertexPrimPrevAttrib && vertexVertexPrimNextAttrib)
            {
                switch (outArrayType)
                {
                case gfe::OutArrayType::Attrib: pointPointEdgeByVertexVertexPrim<gfe::OutArrayType::Attrib>(); break;
                case gfe::OutArrayType::Geo:    pointPointEdgeByVertexVertexPrim<gfe::OutArrayType::Geo>();    break;
                case gfe::OutArrayType::Packed: pointPointEdgeByVertexVertexPrim<gfe::OutArrayType::Packed>(); break;
                default: UT_ASSERT_MSG(0, "Unhandled Out Array Type"); break;
                }
            }
            else if (vertexPrimIndexAttrib)
            {
                switch (outArrayType)
                {
                case gfe::OutArrayType::Attrib: pointPointEdgeByVertexPrimIndex<gfe::OutArrayType::Attrib>(); break;
                case gfe::OutArrayType::Geo:    pointPointEdgeByVertexPrimIndex<gfe::OutArrayType::Geo>();    break;
                case gfe::OutArrayType::Packed: pointPointEdgeByVertexPrimIndex<gfe::OutArrayType::Packed>(); break;
                default: UT_ASSERT_MSG(0, "Unhandled Out Array Type"); break;
                }
            }
            else
                pointPointEdge();
        }
            
        if (pointPointPrimAttrib)
            //pointPointPrim();
            
        if (primPrimEdgeAttrib)
        {
            if (vertexNextEquivAttrib)
                primPrimEdgeByVertexNextEquiv();
            else if (vertexPointDstAttrib)
            {
#if __GFE_MeshTopology_TestSpped
                switch (kernel)
                {
                default:
                case 0: primPrimEdgeByVertexPointDst0(); break;
                case 1: primPrimEdgeByVertexPointDst1(); break;
                case 2: primPrimEdgeByVertexPointDst2(); break;
                }
#else
                primPrimEdgeByVertexPointDst0();
#endif
            }
            else
                primPrimEdge();
        }
            
        if (primPrimPointAttrib)
            primPrimPoint();

#if 0
        if (pointPointEdgeAttrib && outArrayType == gfe::OutArrayType::Attrib)
            GFE_OffsetArrayAttribToGeo::offsetArrayAttribToPacked(geo, pointPointEdgeAttrib, groupParser.getPointGroup());
#endif
            
        if (outAsOffset || outArrayType != gfe::OutArrayType::Attrib)
            return true;

            
        _GFEL OffsetAttribToIndex offsetAttribToIndex(geo);
        //offsetAttribToIndex.offsetToIndex = true;
            
        if (calVertexVertexPrimPrevAttrib  && vertexVertexPrimPrevAttrib )offsetAttribToIndex.getOutAttribArray().append(vertexVertexPrimPrevAttrib);
        if (calVertexVertexPrimNextAttrib  && vertexVertexPrimNextAttrib )offsetAttribToIndex.getOutAttribArray().append(vertexVertexPrimNextAttrib);
        if (calVertexPointDstAttrib        && vertexPointDstAttrib       )offsetAttribToIndex.getOutAttribArray().append(vertexPointDstAttrib);
        if (calVertexNextEquivAttrib       && vertexNextEquivAttrib      )offsetAttribToIndex.getOutAttribArray().append(vertexNextEquivAttrib);
        if (calVertexNextEquivNoLoopAttrib && vertexNextEquivNoLoopAttrib)offsetAttribToIndex.getOutAttribArray().append(vertexNextEquivNoLoopAttrib);
        if (calPointPointEdgeAttrib        && pointPointEdgeAttrib       )offsetAttribToIndex.getOutAttribArray().append(pointPointEdgeAttrib);
        if (calPointPointPrimAttrib        && pointPointPrimAttrib       )offsetAttribToIndex.getOutAttribArray().append(pointPointPrimAttrib);
        if (calPrimPrimEdgeAttrib          && primPrimEdgeAttrib         )offsetAttribToIndex.getOutAttribArray().append(primPrimEdgeAttrib);
        if (calPrimPrimPointAttrib         && primPrimPointAttrib        )offsetAttribToIndex.getOutAttribArray().append(primPrimPointAttrib);

        offsetAttribToIndex.compute();
        return true;
    }


    

    void vertexPrimIndex_vertexPointDst()
    {
        if (!vertexPrimIndexAttrib)
            setVertexPrimIndex(!outIntermediateAttrib);
        if (!vertexPointDstAttrib)
            setVertexPointDst(!outIntermediateAttrib);
        
        const GA_ATITopology* const pointRef = geo->getTopology().getPointRef();

        int0_wh = vertexPrimIndexAttrib;
        int1_wh = vertexPointDstAttrib;
        if (groupParser.isFull() || groupParser.classType() == GA_GROUP_PRIMITIVE)
        {
            UTparallelFor(groupParser.getPrimitiveSplittableRange(), [this, pointRef](const GA_SplittableRange& r)
            {
                GA_Offset vtxoff_prev, vtxoff;
                GA_Offset start, end;
                for (GA_Iterator it(r); it.blockAdvance(start, end); )
                {
                    for (GA_Offset elemoff = start; elemoff < end; ++elemoff)
                    {
                        const GA_OffsetListRef& vertices = geo->getPrimitiveVertexList(elemoff);
                        const GA_Size numvtx = vertices.size();
#if __GFE_MeshTopology_OptimizeTrivialVertices
                        if (vertices.isTrivial())
                        {
                            vtxoff = vertices.trivialStart();
                            vtxoff_prev = vtxoff + numvtx-1;
                        }
                        else
                        {
                            const exint* const data = vertices.getArray();
                            vtxoff = data[0];
                            vtxoff_prev = data[numvtx-1];
                        }
#else
                        vtxoff = vertices[0];
                        vtxoff_prev = vertices[numvtx-1];
#endif
                        int0_wh.set(vtxoff, 0);
                        int1_wh.set(vtxoff_prev, vertices.isClosed() ? pointRef->getLink(vtxoff) : gfe::INVALID_OFFSET);
#if __GFE_MeshTopology_OptimizeTrivialVertices
                        if (vertices.isTrivial())
                        {
                            const GA_Size startVtxpnum = vertices.trivialStart();
                            for (GA_Size vtxpnum = 1; vtxpnum < numvtx; ++vtxpnum)
                            {
                                vtxoff_prev = vtxoff;
                                vtxoff = startVtxpnum + vtxpnum;
                                int0_wh.set(vtxoff, vtxpnum);
                                int1_wh.set(vtxoff_prev, pointRef->getLink(vtxoff));
                            }
                        }
                        else
                        {
                            const exint* const data = vertices.getArray();
                            for (GA_Size vtxpnum = 1; vtxpnum < numvtx; ++vtxpnum)
                            {
                                vtxoff_prev = vtxoff;
                                vtxoff = data[vtxpnum];
                                int0_wh.set(vtxoff, vtxpnum);
                                int1_wh.set(vtxoff_prev, pointRef->getLink(vtxoff));
                            }
                        }
#else
                        for (GA_Size vtxpnum = 1; vtxpnum < numvtx; ++vtxpnum)
                        {
                            vtxoff_prev = vtxoff;
                            vtxoff = vertices[vtxpnum];
                            int0_wh.set(vtxoff, vtxpnum);
                            int1_wh.set(vtxoff_prev, pointRef->getLink(vtxoff));
                        }
#endif
                    }
                }
            }, subscribeRatio, minGrainSize);
        }
        else
        {
            const GA_VertexGroup* const vtxGroup = groupParser.getVertexGroup();
            UT_ASSERT_P(vtxGroup);
            UTparallelFor(groupParser.getPrimitiveSplittableRange(), [this, pointRef, vtxGroup](const GA_SplittableRange& r)
            {
                GA_Size vtxpnum_prev;
                GA_Offset vtxoff_prev, vtxoff;
                GA_Offset start, end;
                for (GA_Iterator it(r); it.blockAdvance(start, end); )
                {
                    for (GA_Offset elemoff = start; elemoff < end; ++elemoff)
                    {
                        const GA_OffsetListRef& vertices = geo->getPrimitiveVertexList(elemoff);
                        const GA_Size numvtx = vertices.size();
                        vtxpnum_prev = numvtx-1;
                        
                        vtxoff_prev = vertices[vtxpnum_prev];
                        vtxoff = vertices[0];
                        if (vtxGroup->contains(vtxoff_prev))
                        {
                            int0_wh.set(vtxoff_prev, vtxpnum_prev);
                            int1_wh.set(vtxoff_prev, vertices.isClosed() ? pointRef->getLink(vtxoff) : gfe::INVALID_OFFSET);
                        }
                        
                        vtxpnum_prev = 0;
                        for (GA_Size vtxpnum = 1; vtxpnum < numvtx; ++vtxpnum)
                        {
                            vtxoff_prev = vtxoff;
                            vtxoff = vertices[vtxpnum];
                            if (vtxGroup->contains(vtxoff_prev))
                            {
                                int0_wh.set(vtxoff_prev, vtxpnum_prev);
                                int1_wh.set(vtxoff_prev, pointRef->getLink(vtxoff));
                            }
                            vtxpnum_prev = vtxpnum;
                        }
                    }
                }
            }, subscribeRatio, minGrainSize);
        }
    }


    

    void vertexPrimIndex()
    {
        if (!vertexPrimIndexAttrib)
            setVertexPrimIndex(!outIntermediateAttrib);
        
        int0_wh = vertexPrimIndexAttrib;
        if (groupParser.isFull() || groupParser.classType() == GA_GROUP_PRIMITIVE)
        {
            UTparallelFor(groupParser.getPrimitiveSplittableRange(), [this](const GA_SplittableRange& r)
            {
                GA_Offset start, end;
                for (GA_Iterator it(r); it.blockAdvance(start, end); )
                {
                    for (GA_Offset elemoff = start; elemoff < end; ++elemoff)
                    {
                        const GA_OffsetListRef& vertices = geo->getPrimitiveVertexList(elemoff);
                        const GA_Size numvtx = vertices.size();
#if __GFE_MeshTopology_OptimizeTrivialVertices
                        if (vertices.isTrivial())
                        {
                            const GA_Size startVtxpnum = vertices.trivialStart();
                            for (GA_Size vtxpnum = 0; vtxpnum < numvtx; ++vtxpnum)
                            {
                                int0_wh.set(startVtxpnum + vtxpnum, vtxpnum);
                            }
                        }
                        else
                        {
                            const exint* const data = vertices.getArray();
                            for (GA_Size vtxpnum = 0; vtxpnum < numvtx; ++vtxpnum)
                            {
                                int0_wh.set(data[vtxpnum], vtxpnum);
                            }
                        }
#else
                        for (GA_Size vtxpnum = 0; vtxpnum < numvtx; ++vtxpnum)
                        {
                            int0_wh.set(vertices[vtxpnum], vtxpnum);
                        }
#endif
                    }
                }
            }, subscribeRatio, minGrainSize);
        }
        else
        {
            const GA_VertexGroup* const vtxGroup = groupParser.getVertexGroup();
            UT_ASSERT_P(vtxGroup);
            UTparallelFor(groupParser.getPrimitiveSplittableRange(), [this, vtxGroup](const GA_SplittableRange& r)
            {
                GA_Offset start, end;
                for (GA_Iterator it(r); it.blockAdvance(start, end); )
                {
                    for (GA_Offset elemoff = start; elemoff < end; ++elemoff)
                    {
                        const GA_OffsetListRef& vertices = geo->getPrimitiveVertexList(elemoff);
                        const GA_Size numvtx = vertices.size();
#if __GFE_MeshTopology_OptimizeTrivialVertices
                        if (vertices.isTrivial())
                        {
                            const GA_Size startVtxpnum = vertices.trivialStart();
                            for (GA_Size vtxpnum = 0; vtxpnum < numvtx; ++vtxpnum)
                            {
                                const GA_Offset vtxoff = startVtxpnum + vtxpnum;
                                if (vtxGroup->contains(vtxoff))
                                    int0_wh.set(vtxoff, vtxpnum);
                            }
                        }
                        else
                        {
                            const exint* const data = vertices.getArray();
                            for (GA_Size vtxpnum = 0; vtxpnum < numvtx; ++vtxpnum)
                            {
                                if (vtxGroup->contains(data[vtxpnum]))
                                    int0_wh.set(data[vtxpnum], vtxpnum);
                            }
                        }
#else
                        for (GA_Size vtxpnum = 0; vtxpnum < numvtx; ++vtxpnum)
                        {
                            const GA_Offset vtxoff = vertices[vtxpnum];
                            if (vtxGroup->contains(vtxoff))
                                int0_wh.set(vtxoff, vtxpnum);
                        }
#endif
                    }
                }
            }, subscribeRatio, minGrainSize);
        }
    }




#if __GFE_MeshTopology_TestSpped
    void vertexPrimIndex1()
    {
        if (!vertexPrimIndexAttrib)
            setVertexPrimIndex(!outIntermediateAttrib);

        
        ::std::vector<GA_Size> vtxpnums;
        vtxpnums.reserve(geo->getVertexMap().offsetSize());
        
        if (groupParser.isFull() || groupParser.classType() == GA_GROUP_PRIMITIVE)
        {
            UTparallelFor(groupParser.getPrimitiveSplittableRange(), [this, &vtxpnums](const GA_SplittableRange& r)
            {
                GA_Offset start, end;
                for (GA_Iterator it(r); it.blockAdvance(start, end); )
                {
                    for (GA_Offset elemoff = start; elemoff < end; ++elemoff)
                    {
                        const GA_OffsetListRef& vertices = geo->getPrimitiveVertexList(elemoff);
                        const GA_Size numvtx = vertices.size();
#if __GFE_MeshTopology_OptimizeTrivialVertices
                        if (vertices.isTrivial())
                        {
                            const GA_Size startVtxpnum = vertices.trivialStart();
                            for (GA_Size vtxpnum = 0; vtxpnum < numvtx; ++vtxpnum)
                            {
                                vtxpnums[startVtxpnum + vtxpnum] = vtxpnum;
                            }
                        }
                        else
                        {
                            const exint* const data = vertices.getArray();
                            for (GA_Size vtxpnum = 0; vtxpnum < numvtx; ++vtxpnum)
                            {
                                vtxpnums[data[vtxpnum]] = vtxpnum;
                            }
                        }
#else
                        for (GA_Size vtxpnum = 0; vtxpnum < numvtx; ++vtxpnum)
                        {
                            vtxpnums[vertices[vtxpnum]] = vtxpnum;
                        }
#endif
                    }
                }
            }, subscribeRatio, minGrainSize);
        }
        else
        {
            const GA_VertexGroup* const vtxGroup = groupParser.getVertexGroup();
            UT_ASSERT_P(vtxGroup);
            UTparallelFor(groupParser.getPrimitiveSplittableRange(), [this, &vtxpnums, vtxGroup](const GA_SplittableRange& r)
            {
                GA_Offset start, end;
                for (GA_Iterator it(r); it.blockAdvance(start, end); )
                {
                    for (GA_Offset elemoff = start; elemoff < end; ++elemoff)
                    {
                        const GA_OffsetListRef& vertices = geo->getPrimitiveVertexList(elemoff);
                        const GA_Size numvtx = vertices.size();
#if __GFE_MeshTopology_OptimizeTrivialVertices
                        if (vertices.isTrivial())
                        {
                            const GA_Size startVtxpnum = vertices.trivialStart();
                            for (GA_Size vtxpnum = 0; vtxpnum < numvtx; ++vtxpnum)
                            {
                                const GA_Offset vtxoff = startVtxpnum + vtxpnum;
                                if (vtxGroup->contains(vtxoff))
                                    vtxpnums[vtxoff] = vtxpnum;
                            }
                        }
                        else
                        {
                            const exint* const data = vertices.getArray();
                            for (GA_Size vtxpnum = 0; vtxpnum < numvtx; ++vtxpnum)
                            {
                                if (vtxGroup->contains(data[vtxpnum]))
                                    vtxpnums[data[vtxpnum]] = vtxpnum;
                            }
                        }
#else
                        for (GA_Size vtxpnum = 0; vtxpnum < numvtx; ++vtxpnum)
                        {
                            const GA_Offset vtxoff = vertices[vtxpnum];
                            if (vtxGroup->contains(vtxoff))
                                vtxpnums[vtxoff] = vtxpnum;
                        }
#endif
                    }
                }
            }, subscribeRatio, minGrainSize);
        }
        UTparallelFor(groupParser.getVertexSplittableRange(), [this, &vtxpnums](const GA_SplittableRange& r)
        {
            GA_PageHandleScalar<GA_Offset>::RWType vtxpnum_ph(vertexPrimIndexAttrib);
            GA_Offset start, end;
            for (GA_PageIterator pit = r.beginPages(); !pit.atEnd(); ++pit)
            {
                for (GA_Iterator it(pit.begin()); it.blockAdvance(start, end); )
                {
                    vtxpnum_ph.setPage(start);
                    for (GA_Offset elemoff = start; elemoff < end; ++elemoff)
                    {
                        vtxpnum_ph.value(elemoff) = vtxpnums[elemoff];
                    }
                }
            }
        }, subscribeRatio, minGrainSize);
    }



#endif

    


    void vertexPointDst()
    {
        if (!vertexPointDstAttrib)
            setVertexPointDst(!outIntermediateAttrib);
        
        int0_wh = vertexPointDstAttrib;
        const GA_ATITopology* const pointRef = geo->getTopology().getPointRef();

        if (groupParser.isFull() || groupParser.classType() == GA_GROUP_PRIMITIVE)
        {
            UTparallelFor(groupParser.getPrimitiveSplittableRange(), [this, pointRef](const GA_SplittableRange& r)
            {
                GA_Offset vtxoff, vtxoff_prev;
                GA_Offset start, end;
                for (GA_Iterator it(r); it.blockAdvance(start, end); )
                {
                    for (GA_Offset elemoff = start; elemoff < end; ++elemoff)
                    {
                        const GA_OffsetListRef& vertices = geo->getPrimitiveVertexList(elemoff);
                        const GA_Size numvtx = vertices.size();
                        vtxoff_prev = vertices[numvtx-1];
                    
                        vtxoff = vertices[0];
                        int0_wh.set(vtxoff_prev, vertices.isClosed() ? pointRef->getLink(vtxoff) : gfe::INVALID_OFFSET);
                        for (GA_Size vtxpnum = 1; vtxpnum < numvtx; ++vtxpnum)
                        {
                            vtxoff_prev = vtxoff;
                            vtxoff = vertices[vtxpnum];
                            int0_wh.set(vtxoff_prev, pointRef->getLink(vtxoff));
                        }
                    }
                }
            }, subscribeRatio, minGrainSize);
        }
        else
        {
            const GA_VertexGroup* const vtxGroup = groupParser.getVertexGroup();
            UT_ASSERT_P(vtxGroup);
            UTparallelFor(groupParser.getPrimitiveSplittableRange(), [this, pointRef, vtxGroup](const GA_SplittableRange& r)
            {
                GA_Offset vtxoff, vtxoff_prev;
                GA_Offset start, end;
                for (GA_Iterator it(r); it.blockAdvance(start, end); )
                {
                    for (GA_Offset elemoff = start; elemoff < end; ++elemoff)
                    {
                        const GA_OffsetListRef& vertices = geo->getPrimitiveVertexList(elemoff);
                        const GA_Size numvtx = vertices.size();
                        vtxoff_prev = vertices[numvtx-1];
                    
                        vtxoff = vertices[0];
                        if (vtxGroup->contains(vtxoff_prev))
                            int0_wh.set(vtxoff_prev, vertices.isClosed() ? pointRef->getLink(vtxoff) : gfe::INVALID_OFFSET);
                        for (GA_Size vtxpnum = 1; vtxpnum < numvtx; ++vtxpnum)
                        {
                            vtxoff_prev = vtxoff;
                            vtxoff = vertices[vtxpnum];
                            if (vtxGroup->contains(vtxoff_prev))
                                int0_wh.set(vtxoff_prev, pointRef->getLink(vtxoff));
                        }
                    }
                }
            }, subscribeRatio, minGrainSize);
        }
    }

    void vertexPointDstByVertexVertexPrimNext()
    {
        UT_ASSERT_P(vertexVertexPrimNextAttrib);
        
        if (!vertexPointDstAttrib)
            setVertexPointDst(!outIntermediateAttrib);
        
        const GA_ATITopology* const pointRef = geo->getTopology().getPointRef();

        //const GA_VertexGroup* const geoGroup = groupParser.getVertexGroup();
        //const GA_SplittableRange geoSplittableRange0(geo->getVertexRange(geoGroup));
        UTparallelFor(groupParser.getVertexSplittableRange(), [this, pointRef](const GA_SplittableRange& r)
        {
            GA_PageHandleScalar<GA_Offset>::RWType dstpt_ph(vertexPointDstAttrib);
            GA_PageHandleScalar<GA_Offset>::ROType vtxPrimNext_ph(vertexVertexPrimNextAttrib);
            GA_Offset start, end;
            for (GA_PageIterator pit = r.beginPages(); !pit.atEnd(); ++pit)
            {
                for (GA_Iterator it(pit.begin()); it.blockAdvance(start, end); )
                {
                    dstpt_ph.setPage(start);
                    vtxPrimNext_ph.setPage(start);
                    for (GA_Offset elemoff = start; elemoff < end; ++elemoff)
                    {
                        dstpt_ph.value(elemoff) = gfe::isValidOffset(vtxPrimNext_ph.value(elemoff))
                                                ? pointRef->getLink(vtxPrimNext_ph.value(elemoff))
                                                : gfe::INVALID_OFFSET;
                    }
                }
            }
        }, subscribeRatio, minGrainSize);
    }




    void vertexPointDstByVtxpnum()
    {
        UT_ASSERT_P(vertexPrimIndexAttrib);
        
        if (!vertexPointDstAttrib)
            setVertexPointDst(!outIntermediateAttrib);
        
        const GA_Topology& topo = geo->getTopology();
        //topo.makePrimitiveRef();
        const GA_ATITopology* const primRef = topo.getPrimitiveRef();

        UTparallelFor(groupParser.getVertexSplittableRange(), [this, primRef](const GA_SplittableRange& r)
        {
            GA_PageHandleScalar<GA_Offset>::RWType dstpt_ph(vertexPointDstAttrib);
            GA_PageHandleScalar<GA_Offset>::RWType vtxpnum_ph(vertexPrimIndexAttrib);
            GA_Offset start, end;
            for (GA_PageIterator pit = r.beginPages(); !pit.atEnd(); ++pit)
            {
                for (GA_Iterator it(pit.begin()); it.blockAdvance(start, end); )
                {
                    dstpt_ph.setPage(start);
                    vtxpnum_ph.setPage(start);
                    for (GA_Offset elemoff = start; elemoff < end; ++elemoff)
                    {
                        UT_ASSERT_P(geo->vertexPrimIndex(elemoff) == vtxpnum_ph.value(elemoff));
                        dstpt_ph.value(elemoff) = geo->vertexPointDst(primRef->getLink(elemoff), vtxpnum_ph.value(elemoff));
                    }
                }
            }
        }, subscribeRatio, minGrainSize);
    }




    void vertexVertexPrim()
    {
        if (!vertexVertexPrimPrevAttrib && !vertexVertexPrimNextAttrib)
            setVertexVertexPrim(!outIntermediateAttrib);
        
        int0_wh = vertexVertexPrimPrevAttrib;
        int1_wh = vertexVertexPrimNextAttrib;
        
        if (groupParser.isFull() || groupParser.classType() == GA_GROUP_PRIMITIVE)
        {
            UTparallelFor(groupParser.getPrimitiveSplittableRange(), [this](const GA_SplittableRange& r)
            {
                GA_Offset start, end;
                for (GA_Iterator it(r); it.blockAdvance(start, end); )
                {
                    for (GA_Offset elemoff = start; elemoff < end; ++elemoff)
                    {
                        const GA_OffsetListRef& vertices = geo->getPrimitiveVertexList(elemoff);
                        const GA_Size numvtx  = vertices.size();
                        GA_Offset vtxoff_prev = vertices[numvtx-1];
                        GA_Offset vtxoff_next = vertices[0];
                        
                        if (calTmpVertexVertexPrimPrevAttrib)
                            int0_wh.set(vtxoff_next, vertices.isClosed() ? vtxoff_prev : gfe::INVALID_OFFSET);
                        if (calTmpVertexVertexPrimNextAttrib)
                            int1_wh.set(vtxoff_prev, vertices.isClosed() ? vtxoff_next : gfe::INVALID_OFFSET);
                        vtxoff_prev = vtxoff_next;
#if __GFE_MeshTopology_OptimizeTrivialVertices
                        if (vertices.isTrivial())
                        {
                            const GA_Size startVtxpnum = vertices.trivialStart();
                            for (GA_Size vtxpnum = 1; vtxpnum < numvtx; ++vtxpnum)
                            {
                                vtxoff_next = startVtxpnum + vtxpnum;
                                if (calTmpVertexVertexPrimPrevAttrib)
                                    int0_wh.set(vtxoff_next, vtxoff_prev);
                                if (calTmpVertexVertexPrimNextAttrib)
                                    int1_wh.set(vtxoff_prev, vtxoff_next);
                                vtxoff_prev = vtxoff_next;
                            }
                        }
                        else
                        {
                            const exint* const data = vertices.getArray();
                            for (GA_Size vtxpnum = 1; vtxpnum < numvtx; ++vtxpnum)
                            {
                                vtxoff_next = data[vtxpnum];
                                if (calTmpVertexVertexPrimPrevAttrib)
                                    int0_wh.set(vtxoff_next, vtxoff_prev);
                                if (calTmpVertexVertexPrimNextAttrib)
                                    int1_wh.set(vtxoff_prev, vtxoff_next);
                                vtxoff_prev = vtxoff_next;
                            }
                        }
#else
                        for (GA_Size vtxpnum = 1; vtxpnum < numvtx; ++vtxpnum)
                        {
                            vtxoff_next = vertices[vtxpnum];
                            if (calTmpVertexVertexPrimPrevAttrib)
                                int0_wh.set(vtxoff_next, vtxoff_prev);
                            if (calTmpVertexVertexPrimNextAttrib)
                                int1_wh.set(vtxoff_prev, vtxoff_next);
                            vtxoff_prev = vtxoff_next;
                        }
#endif
                    }
                }
            }, subscribeRatio, minGrainSize);
        }
        else
        {
            const GA_VertexGroup* const vtxGroup = groupParser.getVertexGroup();
            UT_ASSERT_P(vtxGroup);
            UTparallelFor(groupParser.getPrimitiveSplittableRange(), [this, vtxGroup](const GA_SplittableRange& r)
            {
                GA_Offset start, end;
                for (GA_Iterator it(r); it.blockAdvance(start, end); )
                {
                    for (GA_Offset elemoff = start; elemoff < end; ++elemoff)
                    {
                        const GA_OffsetListRef& vertices = geo->getPrimitiveVertexList(elemoff);
                        const GA_Size numvtx = vertices.size();
                        GA_Offset vtxoff_prev = vertices[numvtx-1];
                        GA_Offset vtxoff_next = vertices[0];
                        
                        if (calTmpVertexVertexPrimPrevAttrib && vtxGroup->contains(vtxoff_next))
                            int0_wh.set(vtxoff_next, vertices.isClosed() ? vtxoff_prev : gfe::INVALID_OFFSET);
                        if (calTmpVertexVertexPrimNextAttrib && vtxGroup->contains(vtxoff_prev))
                            int1_wh.set(vtxoff_prev, vertices.isClosed() ? vtxoff_next : gfe::INVALID_OFFSET);
                        
                        vtxoff_prev = vtxoff_next;
#if __GFE_MeshTopology_OptimizeTrivialVertices
                        if (vertices.isTrivial())
                        {
                            const GA_Size startVtxpnum = vertices.trivialStart();
                            for (GA_Size vtxpnum = 1; vtxpnum < numvtx; ++vtxpnum)
                            {
                                vtxoff_next = startVtxpnum + vtxpnum;
                                if (calTmpVertexVertexPrimPrevAttrib && vtxGroup->contains(vtxoff_next))
                                    int0_wh.set(vtxoff_next, vtxoff_prev);
                                if (calTmpVertexVertexPrimNextAttrib && vtxGroup->contains(vtxoff_prev))
                                    int1_wh.set(vtxoff_prev, vtxoff_next);
                                vtxoff_prev = vtxoff_next;
                            }
                        }
                        else
                        {
                            const exint* const data = vertices.getArray();
                            for (GA_Size vtxpnum = 1; vtxpnum < numvtx; ++vtxpnum)
                            {
                                vtxoff_next = data[vtxpnum];
                                if (calTmpVertexVertexPrimPrevAttrib && vtxGroup->contains(vtxoff_next))
                                    int0_wh.set(vtxoff_next, vtxoff_prev);
                                if (calTmpVertexVertexPrimNextAttrib && vtxGroup->contains(vtxoff_prev))
                                    int1_wh.set(vtxoff_prev, vtxoff_next);
                                vtxoff_prev = vtxoff_next;
                            }
                        }
#else
                        for (GA_Size vtxpnum = 1; vtxpnum < numvtx; ++vtxpnum)
                        {
                            vtxoff_next = vertices[vtxpnum];
                            if (calTmpVertexVertexPrimPrevAttrib && vtxGroup->contains(vtxoff_next))
                                int0_wh.set(vtxoff_next, vtxoff_prev);
                            if (calTmpVertexVertexPrimNextAttrib && vtxGroup->contains(vtxoff_prev))
                                int1_wh.set(vtxoff_prev, vtxoff_next);
                            vtxoff_prev = vtxoff_next;
                        }
#endif
                    }
                }
            }, subscribeRatio, minGrainSize);
        }
    }
    
    void vertexVertexPrimSideFx()
    {
        if (!vertexVertexPrimPrevAttrib && !vertexVertexPrimNextAttrib)
            setVertexVertexPrim(!outIntermediateAttrib);
        
        const GA_Topology& topo = geo->getTopology();
        UTparallelFor(groupParser.getVertexSplittableRange(), [this, &topo](const GA_SplittableRange& r)
        {
            GA_PageHandleScalar<GA_Offset>::RWType vtxPrev_ph(vertexVertexPrimPrevAttrib);
            GA_PageHandleScalar<GA_Offset>::RWType vtxNext_ph(vertexVertexPrimNextAttrib);
            GA_Offset vtxoff_prev, vtxoff_next;
            GA_Offset start, end;
            for (GA_PageIterator pit = r.beginPages(); !pit.atEnd(); ++pit)
            {
                for (GA_Iterator it(pit.begin()); it.blockAdvance(start, end); )
                {
                    vtxPrev_ph.setPage(start);
                    vtxNext_ph.setPage(start);
                    for (GA_Offset elemoff = start; elemoff < end; ++elemoff)
                    {
                        topo.getAdjacentBoundaryVertices(elemoff, vtxoff_prev, vtxoff_next);
                        if (calTmpVertexVertexPrimPrevAttrib)
                            vtxPrev_ph.value(elemoff) = vtxoff_prev;
                        if (calTmpVertexVertexPrimNextAttrib)
                            vtxNext_ph.value(elemoff) = vtxoff_next;
                    }
                }
            }
        }, subscribeRatio, minGrainSize);
    }





#if __GFE_MeshTopology_VertexNextEquivDual

    
void vertexNextEquivDualByVertexPointDst()
{
    UT_ASSERT_P(vertexPointDstAttrib);
        
    if (!vertexNextEquivAttrib && !vertexNextEquivGroup && !vertexNextEquivNoLoopAttrib && !vertexNextEquivNoLoopGroup)
        setVertexNextEquiv(!outIntermediateAttrib);

    //int0_wh = vertexNextEquivAttrib;
    //int1_wh = vertexNextEquivNoLoopAttrib;
    int_oh = vertexPointDstAttrib;
    
    const GA_Topology& topo = geo->getTopology();
    //topo.makeVertexRef();
    const GA_ATITopology* const pointRef    = topo.getPointRef();
    const GA_ATITopology* const pointVtxRef = topo.getVertexRef();
    const GA_ATITopology* const vtxNextRef  = topo.getVertexNextRef();

    const GA_EdgeGroup* const edgeGroup = groupParser.classType() == GA_GROUP_EDGE ? groupParser.getEdgeGroup() : nullptr;
    const GA_SplittableRange& vtxSrange = edgeGroup ? GA_SplittableRange(GA_Range(geo->getVertexMap())) : groupParser.getVertexSplittableRange();
        
    UTparallelFor(vtxSrange, [this, pointRef, pointVtxRef, vtxNextRef, edgeGroup](const GA_SplittableRange& r)
    {
        GA_PageHandleScalar<GA_Offset>::RWType equiv_ph(vertexNextEquivAttrib);
        GA_PageHandleScalar<GA_Offset>::RWType equivNoLoop_ph(vertexNextEquivNoLoopAttrib);
        GA_PageHandleScalar<GA_Offset>::ROType dstpt_ph(vertexPointDstAttrib);
        GA_Offset dstpt;
        GA_Offset start, end;
        for (GA_PageIterator pit = r.beginPages(); !pit.atEnd(); ++pit)
        {
            for (GA_Iterator it(pit.begin()); it.blockAdvance(start, end); )
            {
                equiv_ph.setPage(start);
                equivNoLoop_ph.setPage(start);
                dstpt_ph.setPage(start);
                for (GA_Offset elemoff = start; elemoff < end; ++elemoff)
                {
                    dstpt = dstpt_ph.value(elemoff);
                    if (edgeGroup && !edgeGroup->contains(GA_Edge(pointRef->getLink(elemoff), dstpt)))
                        continue;
                    
                    if (gfe::isInvalidOffset(dstpt))
                    {
                        if (calTmpVertexNextEquivAttrib)
                            equiv_ph.value(elemoff) = gfe::INVALID_OFFSET;
                        if (calTmpVertexNextEquivNoLoopAttrib)
                            equivNoLoop_ph.value(elemoff) = gfe::INVALID_OFFSET;
                        continue;
                    }
                    for (GA_Offset vtxoff_next = vtxNextRef->getLink(elemoff); gfe::isValidOffset(vtxoff_next); vtxoff_next = vtxNextRef->getLink(vtxoff_next))
                    {
                        if (int_oh.get(vtxoff_next) != dstpt)
                            continue;
                        dstpt = gfe::INVALID_OFFSET;
                        if (calTmpVertexNextEquivAttrib)
                            equiv_ph.value(elemoff) = vtxoff_next;
                        if (calTmpVertexNextEquivNoLoopAttrib)
                            equivNoLoop_ph.value(elemoff) = vtxoff_next;
                        break;
                    }
                    if (gfe::isInvalidOffset(dstpt))
                        continue;
                    const GA_Offset ptoff = pointRef->getLink(elemoff);
                    for (GA_Offset vtxoff_next = pointVtxRef->getLink(dstpt); ; vtxoff_next = vtxNextRef->getLink(vtxoff_next))
                    {
                        if (gfe::isInvalidOffset(vtxoff_next))
                        {
                            if (calTmpVertexNextEquivGroup)
                                vertexNextEquivGroup->setElement(elemoff, true);
                            if (calTmpVertexNextEquivAttrib)
                                equiv_ph.value(elemoff) = gfe::INVALID_OFFSET;
                            
                            if (calTmpVertexNextEquivNoLoopGroup)
                                vertexNextEquivNoLoopGroup->setElement(elemoff, true);
                            if (calTmpVertexNextEquivNoLoopAttrib)
                                equivNoLoop_ph.value(elemoff) = gfe::INVALID_OFFSET;
                            break;
                        }
                        if (int_oh.get(vtxoff_next) == ptoff)
                        {
                            if (calTmpVertexNextEquivAttrib)
                                equiv_ph.value(elemoff) = vtxoff_next;
                            if (dstpt > ptoff)
                            {
                                if (calTmpVertexNextEquivNoLoopAttrib)
                                    equivNoLoop_ph.value(elemoff) = vtxoff_next;
                            }
                            else
                            {
                                if (calTmpVertexNextEquivNoLoopGroup)
                                    vertexNextEquivNoLoopGroup->setElement(elemoff, true);
                                if (calTmpVertexNextEquivNoLoopAttrib)
                                    equivNoLoop_ph.value(elemoff) = gfe::INVALID_OFFSET;
                            }
                            break;
                        }
                    }
                }
            }
        }

        
    }, subscribeRatio, minGrainSize);

    if (calTmpVertexNextEquivGroup)
        vertexNextEquivGroup->invalidateGroupEntries();
    if (calTmpVertexNextEquivNoLoopGroup)
        vertexNextEquivNoLoopGroup->invalidateGroupEntries();
}



#else

void vertexNextEquiv()
{
    if (!vertexNextEquivAttrib && !vertexNextEquivGroup)
        setVertexNextEquiv(!outIntermediateAttrib);
    
    int0_wh = vertexNextEquivAttrib;
    int_oh = vertexPointDstAttrib;
    
    const GA_Topology& topo = geo->getTopology();
    //topo.makeVertexRef();
    const GA_ATITopology* const pointRef = topo.getPointRef();
    const GA_ATITopology* const pointVtxRef = topo.getVertexRef();
    const GA_ATITopology* const vtxNextRef  = topo.getVertexNextRef();

    const GA_EdgeGroup* const edgeGroup = groupParser.classType() == GA_GROUP_EDGE ? groupParser.getEdgeGroup() : nullptr;
    const GA_SplittableRange& srange = edgeGroup ? GA_SplittableRange(GA_Range(geo->getVertexMap())) : groupParser.getVertexSplittableRange();
    
    UTparallelFor(groupParser.getVertexSplittableRange(), [this, pointRef, pointVtxRef, vtxNextRef, edgeGroup](const GA_SplittableRange& r)
    {
        GA_Offset vtxoff_next, dstpt, ptoff;
        GA_Offset start, end;
        for (GA_Iterator it(r); it.blockAdvance(start, end); )
        {
            for (GA_Offset elemoff = start; elemoff < end; ++elemoff)
            {
                dstpt = int_oh.get(elemoff);
                if (edgeGroup && !edgeGroup->contains(GA_Edge(pointRef->getLink(elemoff), dstpt)))
                    continue;
                
                if (gfe::isInvalidOffset(dstpt))
                {
                    if (calTmpVertexNextEquivAttrib)
                        int0_wh.set(elemoff, gfe::INVALID_OFFSET);
                    continue;
                }
                for (vtxoff_next = vtxNextRef->getLink(elemoff); gfe::isValidOffset(vtxoff_next); vtxoff_next = vtxNextRef->getLink(vtxoff_next))
                {
                    if (int_oh.get(vtxoff_next) != dstpt)
                        continue;
                    dstpt = gfe::INVALID_OFFSET;
                    if (calTmpVertexNextEquivAttrib)
                        int0_wh.set(elemoff, vtxoff_next);
                    break;
                }
                if (gfe::isInvalidOffset(dstpt))
                    continue;
                ptoff = pointRef->getLink(elemoff);
                for (vtxoff_next = pointVtxRef->getLink(dstpt); gfe::isValidOffset(vtxoff_next); vtxoff_next = vtxNextRef->getLink(vtxoff_next))
                {
                    if (int_oh.get(vtxoff_next) != ptoff)
                        continue;
                    dstpt = gfe::INVALID_OFFSET;
                    if (calTmpVertexNextEquivAttrib)
                        int0_wh.set(elemoff, vtxoff_next);
                    break;
                }
                if (gfe::isInvalidOffset(dstpt))
                    continue;
                if (calTmpVertexNextEquivGroup)
                    vertexNextEquivGroup->setElement(elemoff, true);
                if (calTmpVertexNextEquivAttrib)
                    int0_wh.set(elemoff, gfe::INVALID_OFFSET);
            }
        }
    }, subscribeRatio, minGrainSize);
    
    if (calTmpVertexNextEquivGroup)
        vertexNextEquivGroup->invalidateGroupEntries();
}



void vertexNextEquivNoLoop()
{
    if (!vertexNextEquivNoLoopAttrib && !vertexNextEquivNoLoopGroup)
        setVertexNextEquivNoLoop(!outIntermediateAttrib);
    
    int0_wh = vertexNextEquivNoLoopAttrib;
    int_oh = vertexPointDstAttrib;
    
    const GA_Topology& topo = geo->getTopology();
    //topo.makeVertexRef();
    const GA_ATITopology* const pointRef = topo.getPointRef();
    const GA_ATITopology* const pointVtxRef = topo.getVertexRef();
    const GA_ATITopology* const vtxNextRef = topo.getVertexNextRef();

    const GA_EdgeGroup* const edgeGroup = groupParser.classType() == GA_GROUP_EDGE ? groupParser.getEdgeGroup() : nullptr;
    const GA_SplittableRange& srange = edgeGroup ? GA_SplittableRange(GA_Range(geo->getVertexMap())) : groupParser.getVertexSplittableRange();

    UTparallelFor(groupParser.getVertexSplittableRange(), [this, pointRef, pointVtxRef, vtxNextRef, edgeGroup](const GA_SplittableRange& r)
    {
        GA_Offset vtxoff_next, dstpt, ptoff;
        GA_Offset start, end;
        for (GA_Iterator it(r); it.blockAdvance(start, end); )
        {
            for (GA_Offset elemoff = start; elemoff < end; ++elemoff)
            {
                dstpt = int_oh.get(elemoff);
                
                if (edgeGroup && !edgeGroup->contains(GA_Edge(pointRef->getLink(elemoff), dstpt)))
                    continue;
                
                if (gfe::isInvalidOffset(dstpt))
                {
                    if (calTmpVertexNextEquivNoLoopAttrib)
                        int0_wh.set(elemoff, gfe::INVALID_OFFSET);
                    continue;
                }
                for (vtxoff_next = vtxNextRef->getLink(elemoff); gfe::isValidOffset(vtxoff_next); vtxoff_next = vtxNextRef->getLink(vtxoff_next))
                {
                    if (int_oh.get(vtxoff_next) != dstpt)
                        continue;
                    dstpt = gfe::INVALID_OFFSET;
                    if (calTmpVertexNextEquivNoLoopAttrib)
                        int0_wh.set(elemoff, vtxoff_next);
                    break;
                }
                if (gfe::isInvalidOffset(dstpt))
                    continue;
                ptoff = pointRef->getLink(elemoff);
                for (vtxoff_next = pointVtxRef->getLink(dstpt); gfe::isValidOffset(vtxoff_next); vtxoff_next = vtxNextRef->getLink(vtxoff_next))
                {
                    if (int_oh.get(vtxoff_next) != ptoff)
                        continue;
                    if (dstpt > ptoff)
                    {
                        dstpt = gfe::INVALID_OFFSET;
                        if (calTmpVertexNextEquivNoLoopAttrib)
                            int0_wh.set(elemoff, vtxoff_next);
                    }
                    break;
                }
                if (gfe::isInvalidOffset(dstpt))
                    continue;
                if (calTmpVertexNextEquivNoLoopGroup)
                    vertexNextEquivNoLoopGroup->setElement(elemoff, true);
                if (calTmpVertexNextEquivNoLoopAttrib)
                    int0_wh.set(elemoff, gfe::INVALID_OFFSET);
            }
        }
    }, subscribeRatio, minGrainSize);
    
    if (calTmpVertexNextEquivNoLoopGroup)
        vertexNextEquivNoLoopGroup->invalidateGroupEntries();
}

#endif


    

    void pointPointEdgeByVertexPrimIndex()
    {
        UT_ASSERT_P(vertexPrimIndexAttrib);
        
        if (!pointPointEdgeAttrib)
            setPointPointEdge(!outIntermediateAttrib);
        
        intArray_wh = pointPointEdgeAttrib;
        int_oh = vertexPrimIndexAttrib;
        
        const GA_PointGroup* const seamGroup = pointSeamGroup.getPointGroup();
        UTparallelFor(groupParser.getPointSplittableRange(), [this, seamGroup](const GA_SplittableRange& r)
        {
            UT_ValArray<GA_Offset> adjElems;
            GA_Offset start, end;
            for (GA_Iterator it(r); it.blockAdvance(start, end); )
            {
                for (GA_Offset elemoff = start; elemoff < end; ++elemoff)
                {
                    GA_Offset pt_next;
                    for (GA_Offset vtxoff_next = geo->pointVertex(elemoff); gfe::isValidOffset(vtxoff_next); vtxoff_next = geo->vertexToNextVertex(vtxoff_next))
                    {
                        const GA_Offset primoff = geo->vertexPrimitive(vtxoff_next);
                        const GA_OffsetListRef& vertices = geo->getPrimitiveVertexList(primoff);
                        const GA_Size numvtx = vertices.size();
                        
                        const GA_Size vtxpnum = int_oh.get(vtxoff_next);
                        if (vertices.isClosed() || vtxpnum != 0)
                        {
                            pt_next = geo->vertexPoint(vertices[vtxpnum==0 ? numvtx-1 : vtxpnum-1]);
                            gfe::ArrayOp::uniqueAppendSorted(adjElems, pt_next);
                        }

                        const GA_Size vtxpnum_next = vtxpnum+1;
                        if (vertices.isClosed() || vtxpnum_next != numvtx)
                        {
                            pt_next = geo->vertexPoint(vertices[vtxpnum_next==numvtx ? 0 : vtxpnum_next]);
                            gfe::ArrayOp::uniqueAppendSorted(adjElems, pt_next);
                        }
                    }
                    intArray_wh.set(elemoff, adjElems);
                    adjElems.clear();
                }
            }
        }, subscribeRatio, minGrainSize);
    }



        
    template<gfe::OutArrayType _OutArrayType>
    void pointPointEdgeByVertexPrimIndex()
    {
        UT_ASSERT_P(vertexPrimIndexAttrib);

        GU_DetailHandle geoTmp_h;
        GU_Detail* geoTmp;
        if constexpr (_OutArrayType == gfe::OutArrayType::Attrib)
        {
            if (!pointPointEdgeAttrib)
                setPointPointEdge(!outIntermediateAttrib);
            intArray_wh = pointPointEdgeAttrib;
        }
        else
        {
            //const GA_AIFNumericArray* const aIFNumericArray = pointPointEdgeAttrib->getAIFNumericArray();
            geoTmp_h = gfe::GeoBase::newDetail();
            geoTmp = geoTmp_h.gdpNC();
            geoTmp->appendPrimitiveBlock(GEO_PRIMNONE, geo->getNumPoints());
        }
        int_oh = vertexPrimIndexAttrib;
        
        const GA_IndexMap& indexMap = geo->getIndexMap(GA_ATTRIB_POINT);
        
        const GA_PointGroup* const seamGroup = pointSeamGroup.getPointGroup();
        UTparallelFor(groupParser.getPointSplittableRange(), [this, seamGroup, geoTmp, &indexMap](const GA_SplittableRange& r)
        {
            //std::variant<UT_ValArray<GA_Offset>, GA_OffsetList> adjElems;
            
            GA_Offset pt_next;
            GA_Offset start, end;
            if constexpr (_OutArrayType == gfe::OutArrayType::Attrib)
            {
                UT_ValArray<GA_Offset> adjElems;
                for (GA_Iterator it(r); it.blockAdvance(start, end); )
                {
                    for (GA_Offset elemoff = start; elemoff < end; ++elemoff)
                    {
                        for (GA_Offset vtxoff_next = geo->pointVertex(elemoff); gfe::isValidOffset(vtxoff_next); vtxoff_next = geo->vertexToNextVertex(vtxoff_next))
                        {
                            const GA_Offset primoff = geo->vertexPrimitive(vtxoff_next);
                            const GA_OffsetListRef& vertices = geo->getPrimitiveVertexList(primoff);
                            const GA_Size numvtx = vertices.size(); 
                            
                            const GA_Size vtxpnum = int_oh.get(vtxoff_next);
                            if (vertices.isClosed() || vtxpnum != 0)
                            {
                                pt_next = geo->vertexPoint(vertices[vtxpnum==0 ? numvtx-1 : vtxpnum-1]);
                                if constexpr (_OutArrayType != gfe::OutArrayType::Attrib)
                                {
                                    if (!outAsOffset)
                                        pt_next = indexMap.indexFromOffset(pt_next);
                                }
                                gfe::ArrayOp::uniqueAppendSorted(adjElems, pt_next);
                            }

                            const GA_Size vtxpnum_next = vtxpnum+1;
                            if (vertices.isClosed() || vtxpnum_next != numvtx)
                            {
                                pt_next = geo->vertexPoint(vertices[vtxpnum_next==numvtx ? 0 : vtxpnum_next]);
                                if constexpr (_OutArrayType != gfe::OutArrayType::Attrib)
                                {
                                    if (!outAsOffset)
                                        pt_next = indexMap.indexFromOffset(pt_next);
                                }
                                gfe::ArrayOp::uniqueAppendSorted(adjElems, pt_next);
                            }
                        }
                        if constexpr (_OutArrayType == gfe::OutArrayType::Attrib)
                            intArray_wh.set(elemoff, adjElems);
                        else
                            geoTmp->getPrimitiveList().setVertexList(indexMap.indexFromOffset(elemoff), adjElems);
                        adjElems.clear();
                    }
                }
            }
            else
            {
                GA_OffsetList adjElems;
                adjElems.reserve(16);
                for (GA_Iterator it(r); it.blockAdvance(start, end); )
                {
                    for (GA_Offset elemoff = start; elemoff < end; ++elemoff)
                    {
                        for (GA_Offset vtxoff_next = geo->pointVertex(elemoff); gfe::isValidOffset(vtxoff_next); vtxoff_next = geo->vertexToNextVertex(vtxoff_next))
                        {
                            const GA_Offset primoff = geo->vertexPrimitive(vtxoff_next);
                            const GA_OffsetListRef& vertices = geo->getPrimitiveVertexList(primoff);
                            const GA_Size numvtx = vertices.size();
                            
                            const GA_Size vtxpnum = int_oh.get(vtxoff_next);
                            if (vertices.isClosed() || vtxpnum != 0)
                            {
                                pt_next = geo->vertexPoint(vertices[vtxpnum==0 ? numvtx-1 : vtxpnum-1]);
                                if constexpr (_OutArrayType != gfe::OutArrayType::Attrib)
                                {
                                    if (!outAsOffset)
                                        pt_next = indexMap.indexFromOffset(pt_next);
                                }
                                gfe::ArrayOp::uniqueAppendSorted(adjElems, pt_next);
                            }

                            const GA_Size vtxpnum_next = vtxpnum+1;
                            if (vertices.isClosed() || vtxpnum_next != numvtx)
                            {
                                pt_next = geo->vertexPoint(vertices[vtxpnum_next==numvtx ? 0 : vtxpnum_next]);
                                if constexpr (_OutArrayType != gfe::OutArrayType::Attrib)
                                {
                                    if (!outAsOffset)
                                        pt_next = indexMap.indexFromOffset(pt_next);
                                }
                                gfe::ArrayOp::uniqueAppendSorted(adjElems, pt_next);
                            }
                        }
                        if constexpr (_OutArrayType == gfe::OutArrayType::Attrib)
                            intArray_wh.set(elemoff, adjElems);
                        else
                            geoTmp->getPrimitiveList().setVertexList(indexMap.indexFromOffset(elemoff), adjElems);
                        adjElems.clear();
                    }
                }
            }
        }, subscribeRatio, minGrainSize);

        if      constexpr (_OutArrayType == gfe::OutArrayType::Geo)
            geo->replaceWith(*geoTmp);
        else if constexpr (_OutArrayType == gfe::OutArrayType::Packed)
        {
            GU_PackedGeometry::packGeometry(*geo, geoTmp_h);
            //GU_PrimPacked* primPacked = GU_PackedGeometry::packGeometry(*geo, geoTmp_h);
        }
        
        //switch (outArrayType)
        //{
        //case gfe::OutArrayType::Attrib:                                                  break;
        //case gfe::OutArrayType::Geo:    geo->replaceWith(*geoTmp);                       break;
        //case gfe::OutArrayType::Packed: GU_PackedGeometry::packGeometry(*geo, geoTmp_h); break;
        //default: UT_ASSERT_P(0, "Unhandled Out Array Type"); break;
        //}
        
    }

    template<gfe::OutArrayType _OutArrayType>
    void pointPointEdgeByVertexVertexPrim()
    {
        UT_ASSERT_P(vertexVertexPrimPrevAttrib);
        UT_ASSERT_P(vertexVertexPrimNextAttrib);

        GEO_PrimPolySoup* soup;
        GU_DetailHandle geoTmp_h;
        GU_Detail* geoTmp;
        if constexpr (_OutArrayType == gfe::OutArrayType::Attrib)
        {
            if (!pointPointEdgeAttrib)
                setPointPointEdge(!outIntermediateAttrib);
            intArray_wh = pointPointEdgeAttrib;
        }
        else
        {
            //const GA_AIFNumericArray* const aIFNumericArray = pointPointEdgeAttrib->getAIFNumericArray();
            geoTmp_h = gfe::GeoBase::newDetail();
            geoTmp = geoTmp_h.gdpNC();
            //geoTmp->appendPrimitiveBlock(GEO_PRIMNONE, geo->getNumPoints());
            geoTmp->appendPrimitiveBlock(GEO_PRIMPOLYSOUP, 1);
            soup = reinterpret_cast<GEO_PrimPolySoup*>(geoTmp->getPrimitive(0));
        }
        
        const GA_IndexMap& indexMap = geo->getIndexMap(GA_ATTRIB_POINT);
        
        int_oh = vertexVertexPrimPrevAttrib;
        const GA_ROHandleT<GA_Offset> vtxNext_h(vertexVertexPrimNextAttrib);
        
        const GA_PointGroup* const seamGroup = pointSeamGroup.getPointGroup();
        
        const GA_Topology& topo = geo->getTopology();
        //topo.makeFull();
        //topo.makeVertexRef();
        const GA_ATITopology* const pointRef     = topo.getPointRef();
        const GA_ATITopology* const pointVtxRef  = topo.getVertexRef();
        //const GA_ATITopology* const vtxPrevRef = topo.getVertexPrevRef();
        const GA_ATITopology* const vtxNextRef   = topo.getVertexNextRef();

        UTparallelFor(groupParser.getPointSplittableRange(), [this, &vtxNext_h, pointVtxRef, vtxNextRef, pointRef, seamGroup, geoTmp, &indexMap](const GA_SplittableRange& r)
        {
            GA_Offset pt_next;
            GA_Offset start, end;
            if constexpr (_OutArrayType == gfe::OutArrayType::Attrib)
            {
                UT_ValArray<GA_Offset> adjElems;
                for (GA_Iterator it(r); it.blockAdvance(start, end); )
                {
                    for (GA_Offset elemoff = start; elemoff < end; ++elemoff)
                    {
                        for (GA_Offset vtxoff_next = pointVtxRef->getLink(elemoff); gfe::isValidOffset(vtxoff_next); vtxoff_next = vtxNextRef->getLink(vtxoff_next))
                        {
                            const GA_Offset vtxPrev = int_oh.get(vtxoff_next);
                            if (gfe::isValidOffset(vtxPrev))
                            {
                                pt_next = pointRef->getLink(vtxPrev);
                                if constexpr (_OutArrayType != gfe::OutArrayType::Attrib)
                                {
                                    if (!outAsOffset)
                                        pt_next = indexMap.indexFromOffset(pt_next);
                                }
                                gfe::ArrayOp::uniqueAppendSorted(adjElems, pt_next);
                            }
                            const GA_Offset vtxNext = vtxNext_h.get(vtxoff_next);
                            if (gfe::isValidOffset(vtxNext))
                            {
                                pt_next = pointRef->getLink(vtxNext);
                                if constexpr (_OutArrayType != gfe::OutArrayType::Attrib)
                                {
                                    if (!outAsOffset)
                                        pt_next = indexMap.indexFromOffset(pt_next);
                                }
                                gfe::ArrayOp::uniqueAppendSorted(adjElems, pt_next);
                            }
                        }
                        if constexpr (_OutArrayType == gfe::OutArrayType::Attrib)
                            intArray_wh.set(elemoff, adjElems);
                        else
                            geoTmp->getPrimitiveList().setVertexList(indexMap.indexFromOffset(elemoff), adjElems);
                        adjElems.clear();
                    }
                }
            }
            else
            {
                GA_OffsetList adjElems;
                adjElems.reserve(16);
                for (GA_Iterator it(r); it.blockAdvance(start, end); )
                {
                    for (GA_Offset elemoff = start; elemoff < end; ++elemoff)
                    {
                        for (GA_Offset vtxoff_next = pointVtxRef->getLink(elemoff); gfe::isValidOffset(vtxoff_next); vtxoff_next = vtxNextRef->getLink(vtxoff_next))
                        {
                            const GA_Offset vtxPrev = int_oh.get(vtxoff_next);
                            if (gfe::isValidOffset(vtxPrev))
                            {
                                pt_next = pointRef->getLink(vtxPrev);
                                if constexpr (_OutArrayType != gfe::OutArrayType::Attrib)
                                {
                                    if (!outAsOffset)
                                        pt_next = indexMap.indexFromOffset(pt_next);
                                }
                                gfe::ArrayOp::uniqueAppendSorted(adjElems, pt_next);
                            }
                            const GA_Offset vtxNext = vtxNext_h.get(vtxoff_next);
                            if (gfe::isValidOffset(vtxNext))
                            {
                                pt_next = pointRef->getLink(vtxNext);
                                if constexpr (_OutArrayType != gfe::OutArrayType::Attrib)
                                {
                                    if (!outAsOffset)
                                        pt_next = indexMap.indexFromOffset(pt_next);
                                }
                                gfe::ArrayOp::uniqueAppendSorted(adjElems, pt_next);
                            }
                        }
                        if constexpr (_OutArrayType == gfe::OutArrayType::Attrib)
                            intArray_wh.set(elemoff, adjElems);
                        else
                            geoTmp->getPrimitiveList().setVertexList(indexMap.indexFromOffset(elemoff), adjElems);
                        adjElems.clear();
                    }
                }
            }
        }, subscribeRatio, minGrainSize);

        
        if      constexpr (_OutArrayType == gfe::OutArrayType::Geo)
        {
            geo->replaceWith(*geoTmp);
            //geo->bumpAllDataIds();
        }
        else if constexpr (_OutArrayType == gfe::OutArrayType::Packed)
        {
            GU_PackedGeometry::packGeometry(*geo, geoTmp_h);
            //GU_PrimPacked* primPacked = GU_PackedGeometry::packGeometry(*geo, geoTmp_h);
        }
    }


    void pointPointEdge(const GA_Offset ptoff, UT_ValArray<GA_Offset>& adjElems)
    {
        //exint cap = 0;
        GA_Offset pt_next;
        for (GA_Offset vtxoff_next = geo->pointVertex(ptoff); gfe::isValidOffset(vtxoff_next); vtxoff_next = geo->vertexToNextVertex(vtxoff_next))
        {
            const GA_Offset primoff = geo->vertexPrimitive(vtxoff_next);
            
            const GA_OffsetListRef& vertices = geo->getPrimitiveVertexList(primoff);
            const GA_Size numvtx = vertices.size();
            const GA_Size vtxpnum = geo->vertexPrimIndex(vertices, vtxoff_next);

            if (vertices.isClosed() || vtxpnum != 0)
            {
                pt_next = geo->vertexPoint(vertices[vtxpnum==0 ? numvtx-1 : vtxpnum-1]);
                gfe::ArrayOp::uniqueAppendSorted(adjElems, pt_next);
            }

            const GA_Size vtxpnum_next = vtxpnum + 1;
            if (vertices.isClosed() || vtxpnum_next != numvtx)
            {
                pt_next = geo->vertexPoint(vertices[vtxpnum_next==numvtx ? 0 : vtxpnum_next]);
                gfe::ArrayOp::uniqueAppendSorted(adjElems, pt_next);
            }
        }
    }

    //Get all prims neighbours prims with adjacent by edge
    void pointPointEdge()
    {
        if (!pointPointEdgeAttrib)
            setPointPointEdge(!outIntermediateAttrib);
        
        intArray_wh = pointPointEdgeAttrib;
        
        UTparallelFor(groupParser.getPointSplittableRange(), [this](const GA_SplittableRange& r)
        {
            UT_ValArray<GA_Offset> adjElems;
            GA_Offset start, end;
            for (GA_Iterator it(r); it.blockAdvance(start, end); )
            {
                for (GA_Offset elemoff = start; elemoff < end; ++elemoff)
                {
                    pointPointEdge(elemoff, adjElems);
                    intArray_wh.set(elemoff, adjElems);
                    adjElems.clear();
                }
            }
        }, subscribeRatio, minGrainSize);
    }


    //Get all prims neighbours prims with adjacent by edge
    void primPrimEdgeByVertexNextEquiv()
    {
        UT_ASSERT_P(vertexNextEquivAttrib);
        
        if (!primPrimEdgeAttrib)
            setPrimPrimEdge(!outIntermediateAttrib);
        
        intArray_wh = primPrimEdgeAttrib;
        int_oh = vertexNextEquivAttrib;
        
        const GA_VertexGroup* const seamGroup = vertexEdgeSeamGroup.getVertexGroup();
        const GA_Topology& topo = geo->getTopology();
        //topo.makePrimitiveRef();
        const GA_ATITopology* const primRef = topo.getPrimitiveRef();

        UTparallelFor(groupParser.getPrimitiveSplittableRange(), [this, seamGroup, primRef](const GA_SplittableRange& r)
        {
            UT_ValArray<GA_Offset> adjElems;
            GA_Offset start, end;
            for (GA_Iterator it(r); it.blockAdvance(start, end); )
            {
                for (GA_Offset elemoff = start; elemoff < end; ++elemoff)
                {
                    const GA_OffsetListRef& vertices = geo->getPrimitiveVertexList(elemoff);
                    const GA_Size numvtx = vertices.size();
                    for (GA_Size vtxpnum = 0; vtxpnum < numvtx; ++vtxpnum)
                    {
                        const GA_Offset vtxoff_start = vertices[vtxpnum];
                        if (seamGroup && !seamGroup->contains(vtxoff_start))
                            continue;
                        GA_Offset vtxoff_next = int_oh.get(vtxoff_start);
                        if (gfe::isInvalidOffset(vtxoff_next))
                            continue;
                        for (; vtxoff_next != vtxoff_start; vtxoff_next = int_oh.get(vtxoff_next))
                        {
                            const GA_Offset primoff = primRef->getLink(vtxoff_next);
                            gfe::ArrayOp::uniqueAppendSorted(adjElems, primoff);
                        }
                    }
                    intArray_wh.set(elemoff, adjElems);
                    adjElems.clear();
                }
            }
        }, subscribeRatio, minGrainSize);
    }

    
    void primPrimEdgeByVertexPointDst0()
    {
        UT_ASSERT_P(vertexPointDstAttrib);
        
        if (!primPrimEdgeAttrib)
            setPrimPrimEdge(!outIntermediateAttrib);
        
        intArray_wh = primPrimEdgeAttrib;
        int_oh = vertexPointDstAttrib;
        
        const GA_VertexGroup* const seamGroup = vertexEdgeSeamGroup.getVertexGroup();
        UTparallelFor(groupParser.getPrimitiveSplittableRange(), [this, seamGroup](const GA_SplittableRange& r)
        {
            UT_ValArray<GA_Offset> adjElems;
            GA_Offset vtxoff_next;
            GA_Offset start, end;
            if (seamGroup)
            {
                for (GA_Iterator it(r); it.blockAdvance(start, end); )
                {
                    for (GA_Offset elemoff = start; elemoff < end; ++elemoff)
                    {
                        const GA_OffsetListRef& vertices = geo->getPrimitiveVertexList(elemoff);
                        const GA_Size numvtx = vertices.size();
                        for (GA_Size vtxpnum = 0; vtxpnum < numvtx; ++vtxpnum)
                        {
                            const GA_Offset vtxoff = vertices[vtxpnum];
                            const GA_Offset dstpt = int_oh.get(vtxoff);
                            if (gfe::isInvalidOffset(dstpt))
                                continue;
                            const GA_Offset ptoff = geo->vertexPoint(vtxoff);
                            for (vtxoff_next = geo->pointVertex(ptoff); gfe::isValidOffset(vtxoff_next); vtxoff_next = geo->vertexToNextVertex(vtxoff_next))
                            {
                                if (vtxoff_next == vtxoff)
                                    continue;
                                if (int_oh.get(vtxoff_next) != dstpt)
                                    continue;
                                const GA_Offset primoff = geo->vertexPrimitive(vtxoff_next);
                                gfe::ArrayOp::uniqueAppendSorted(adjElems, primoff);
                            }

                            for (vtxoff_next = geo->pointVertex(dstpt); gfe::isValidOffset(vtxoff_next); vtxoff_next = geo->vertexToNextVertex(vtxoff_next))
                            {
                                if (int_oh.get(vtxoff_next) != ptoff)
                                    continue;
                                const GA_Offset primoff = geo->vertexPrimitive(vtxoff_next);
                                gfe::ArrayOp::uniqueAppendSorted(adjElems, primoff);
                            }
                        }
                        intArray_wh.set(elemoff, adjElems);
                        adjElems.clear();
                    }
                }
            }
            else
            {
                for (GA_Iterator it(r); it.blockAdvance(start, end); )
                {
                    for (GA_Offset elemoff = start; elemoff < end; ++elemoff)
                    {
                        const GA_OffsetListRef& vertices = geo->getPrimitiveVertexList(elemoff);
                        const GA_Size numvtx = vertices.size();
                        for (GA_Size vtxpnum = 0; vtxpnum < numvtx; ++vtxpnum)
                        {
                            const GA_Offset vtxoff = vertices[vtxpnum];
                            const GA_Offset dstpt = int_oh.get(vtxoff);
                            if (gfe::isInvalidOffset(dstpt))
                                continue;
                            const GA_Offset ptoff = geo->vertexPoint(vtxoff);
                            for (vtxoff_next = geo->pointVertex(ptoff); gfe::isValidOffset(vtxoff_next); vtxoff_next = geo->vertexToNextVertex(vtxoff_next))
                            {
                                if (vtxoff_next == vtxoff)
                                    continue;
                                if (int_oh.get(vtxoff_next) != dstpt)
                                    continue;
                                const GA_Offset primoff = geo->vertexPrimitive(vtxoff_next);
                                gfe::ArrayOp::uniqueAppendSorted(adjElems, primoff);
                            }

                            for (vtxoff_next = geo->pointVertex(dstpt); gfe::isValidOffset(vtxoff_next); vtxoff_next = geo->vertexToNextVertex(vtxoff_next))
                            {
                                if (int_oh.get(vtxoff_next) != ptoff)
                                    continue;
                                const GA_Offset primoff = geo->vertexPrimitive(vtxoff_next);
                                gfe::ArrayOp::uniqueAppendSorted(adjElems, primoff);
                            }
                        }
                        intArray_wh.set(elemoff, adjElems);
                        adjElems.clear();
                    }
                }
            }
        }, subscribeRatio, minGrainSize);
    }

        
    void primPrimEdgeByVertexPointDst1()
    {
        UT_ASSERT_P(vertexPointDstAttrib);
        
        if (!primPrimEdgeAttrib)
            setPrimPrimEdge(!outIntermediateAttrib);
        
        intArray_wh = primPrimEdgeAttrib;
        int_oh = vertexPointDstAttrib;

        const GA_VertexGroup* const seamGroup = vertexEdgeSeamGroup.getVertexGroup();
        UTparallelFor(groupParser.getPrimitiveSplittableRange(), [this, seamGroup](const GA_SplittableRange& r)
        {
            UT_ValArray<GA_Offset> adjElems;
            GA_Offset vtxoff_next;
            GA_Offset start, end;
            if (seamGroup)
            {
                for (GA_Iterator it(r); it.blockAdvance(start, end); )
                {
                    for (GA_Offset elemoff = start; elemoff < end; ++elemoff)
                    {
                        const GA_OffsetListRef& vertices = geo->getPrimitiveVertexList(elemoff);
                        const GA_Size numvtx = vertices.size();
                        for (GA_Size vtxpnum = 0; vtxpnum < numvtx; ++vtxpnum)
                        {
                            const GA_Offset vtxoff = vertices[vtxpnum];
                            const GA_Offset dstpt = int_oh.get(vtxoff);
                            if (gfe::isInvalidOffset(dstpt))
                                continue;
                            const GA_Offset ptoff = geo->vertexPoint(vtxoff);
                            for (vtxoff_next = geo->pointVertex(ptoff); gfe::isValidOffset(vtxoff_next); vtxoff_next = geo->vertexToNextVertex(vtxoff_next))
                            {
                                if (vtxoff_next == vtxoff)
                                    continue;
                                if (int_oh.get(vtxoff_next) != dstpt)
                                    continue;
                                const GA_Offset primoff = geo->vertexPrimitive(vtxoff_next);
                                gfe::ArrayOp::uniqueAppendSorted(adjElems, primoff);
                            }

                            for (vtxoff_next = geo->pointVertex(dstpt); gfe::isValidOffset(vtxoff_next); vtxoff_next = geo->vertexToNextVertex(vtxoff_next))
                            {
                                if (int_oh.get(vtxoff_next) != ptoff)
                                    continue;
                                const GA_Offset primoff = geo->vertexPrimitive(vtxoff_next);
                                gfe::ArrayOp::uniqueAppendSorted(adjElems, primoff);
                            }
                        }
                        intArray_wh.set(elemoff, adjElems);
                        adjElems.clear();
                    }
                }
            }
            else
            {
                for (GA_Iterator it(r); it.blockAdvance(start, end); )
                {
                    for (GA_Offset elemoff = start; elemoff < end; ++elemoff)
                    {
                        const GA_OffsetListRef& vertices = geo->getPrimitiveVertexList(elemoff);
                        const GA_Size numvtx = vertices.size();
                        for (GA_Size vtxpnum = 0; vtxpnum < numvtx; ++vtxpnum)
                        {
                            const GA_Offset vtxoff = vertices[vtxpnum];
                            const GA_Offset dstpt = int_oh.get(vtxoff);
                            if (gfe::isInvalidOffset(dstpt))
                                continue;
                            const GA_Offset ptoff = geo->vertexPoint(vtxoff);
                            for (vtxoff_next = geo->pointVertex(ptoff); gfe::isValidOffset(vtxoff_next); vtxoff_next = geo->vertexToNextVertex(vtxoff_next))
                            {
                                if (vtxoff_next == vtxoff)
                                    continue;
                                if (int_oh.get(vtxoff_next) != dstpt)
                                    continue;
                                const GA_Offset primoff = geo->vertexPrimitive(vtxoff_next);
                                gfe::ArrayOp::uniqueAppendSorted(adjElems, primoff);
                            }

                            for (vtxoff_next = geo->pointVertex(dstpt); gfe::isValidOffset(vtxoff_next); vtxoff_next = geo->vertexToNextVertex(vtxoff_next))
                            {
                                if (int_oh.get(vtxoff_next) != ptoff)
                                    continue;
                                const GA_Offset primoff = geo->vertexPrimitive(vtxoff_next);
                                gfe::ArrayOp::uniqueAppendSorted(adjElems, primoff);
                            }
                        }
                        intArray_wh.set(elemoff, adjElems);
                        adjElems.clear();
                    }
                }
            }
        }, subscribeRatio, minGrainSize);
    }


    void primPrimEdgeByVertexPointDst2()
    {
        UT_ASSERT_P(vertexPointDstAttrib);
        
        if (!primPrimEdgeAttrib)
            setPrimPrimEdge(outTopoAttrib);
        
        intArray_wh = primPrimEdgeAttrib;
        int_oh = vertexPointDstAttrib;
        
        const GA_VertexGroup* const seamGroup = vertexEdgeSeamGroup.getVertexGroup();
        UTparallelFor(groupParser.getPrimitiveSplittableRange(), [this, seamGroup](const GA_SplittableRange& r)
        {
            UT_ValArray<GA_Offset> adjElems;
            GA_Offset vtxoff_next;
            GA_Offset start, end;
            if (seamGroup)
            {
                for (GA_Iterator it(r); it.blockAdvance(start, end); )
                {
                    for (GA_Offset elemoff = start; elemoff < end; ++elemoff)
                    {
                        const GA_OffsetListRef& vertices = geo->getPrimitiveVertexList(elemoff);
                        const GA_Size numvtx = vertices.size();
                        for (GA_Size vtxpnum = 0; vtxpnum < numvtx; ++vtxpnum)
                        {
                            const GA_Offset vtxoff = vertices[vtxpnum];
                            const GA_Offset dstpt = int_oh.get(vtxoff);
                            if (gfe::isInvalidOffset(dstpt))
                                continue;
                            const GA_Offset ptoff = geo->vertexPoint(vtxoff);
                            for (vtxoff_next = geo->pointVertex(ptoff); gfe::isValidOffset(vtxoff_next); vtxoff_next = geo->vertexToNextVertex(vtxoff_next))
                            {
                                if (vtxoff_next == vtxoff)
                                    continue;
                                if (int_oh.get(vtxoff_next) != dstpt)
                                    continue;
                                const GA_Offset primoff = geo->vertexPrimitive(vtxoff_next);
                                gfe::ArrayOp::uniqueAppendSorted(adjElems, primoff);
                            }

                            for (vtxoff_next = geo->pointVertex(dstpt); gfe::isValidOffset(vtxoff_next); vtxoff_next = geo->vertexToNextVertex(vtxoff_next))
                            {
                                if (int_oh.get(vtxoff_next) != ptoff)
                                    continue;
                                const GA_Offset primoff = geo->vertexPrimitive(vtxoff_next);
                                gfe::ArrayOp::uniqueAppendSorted(adjElems, primoff);
                            }
                        }
                        intArray_wh.set(elemoff, adjElems);
                        adjElems.clear();
                    }
                }
            }
            else
            {
                for (GA_Iterator it(r); it.blockAdvance(start, end); )
                {
                    for (GA_Offset elemoff = start; elemoff < end; ++elemoff)
                    {
                        const GA_OffsetListRef& vertices = geo->getPrimitiveVertexList(elemoff);
                        const GA_Size numvtx = vertices.size();
                        for (GA_Size vtxpnum = 0; vtxpnum < numvtx; ++vtxpnum)
                        {
                            const GA_Offset vtxoff = vertices[vtxpnum];
                            const GA_Offset dstpt = int_oh.get(vtxoff);
                            if (gfe::isInvalidOffset(dstpt))
                                continue;
                            const GA_Offset ptoff = geo->vertexPoint(vtxoff);
                            for (vtxoff_next = geo->pointVertex(ptoff); gfe::isValidOffset(vtxoff_next); vtxoff_next = geo->vertexToNextVertex(vtxoff_next))
                            {
                                if (vtxoff_next == vtxoff)
                                    continue;
                                if (int_oh.get(vtxoff_next) != dstpt)
                                    continue;
                                const GA_Offset primoff = geo->vertexPrimitive(vtxoff_next);
                                gfe::ArrayOp::uniqueAppendSorted(adjElems, primoff);
                            }

                            for (vtxoff_next = geo->pointVertex(dstpt); gfe::isValidOffset(vtxoff_next); vtxoff_next = geo->vertexToNextVertex(vtxoff_next))
                            {
                                if (int_oh.get(vtxoff_next) != ptoff)
                                    continue;
                                const GA_Offset primoff = geo->vertexPrimitive(vtxoff_next);
                                gfe::ArrayOp::uniqueAppendSorted(adjElems, primoff);
                            }
                        }
                        intArray_wh.set(elemoff, adjElems);
                        adjElems.clear();
                    }
                }
            }
        }, subscribeRatio, minGrainSize);
    }


    void primPrimEdge()
    {
        if (!primPrimEdgeAttrib)
            setPrimPrimEdge(!outIntermediateAttrib);
        
        intArray_wh = primPrimEdgeAttrib;
        
        const GEO_Detail* const geoGEO = geo->asGEO_Detail();
        const GA_VertexGroup* const seamGroup = vertexEdgeSeamGroup.getVertexGroup();
        UTparallelFor(groupParser.getPrimitiveSplittableRange(), [this, geoGEO, seamGroup](const GA_SplittableRange& r)
        {
            GEO_Detail::GEO_EdgeAdjArray adjElemStruct;
            UT_ValArray<GA_Offset> adjElems;
            GA_Offset start, end;
            for (GA_Iterator it(r); it.blockAdvance(start, end); )
            {
                if (seamGroup)
                {
                    for (GA_Offset elemoff = start; elemoff < end; ++elemoff)
                    {
                        const GA_Size numAdj = geoGEO->getEdgeAdjacentPolygons(adjElemStruct, elemoff);
                        adjElems.setSizeNoInit(numAdj);
                        for (GA_Size i = 0; i < numAdj; ++i)
                        {
                            adjElems[i] = adjElemStruct[i].myAdjacentPolygon;
                        }
                        intArray_wh.set(elemoff, adjElems);
                    }
                }
                else
                {
                    for (GA_Offset elemoff = start; elemoff < end; ++elemoff)
                    {
                        const GA_Size numAdj = geoGEO->getEdgeAdjacentPolygons(adjElemStruct, elemoff);
                        adjElems.setSizeNoInit(numAdj);
                        for (GA_Size i = 0; i < numAdj; ++i)
                        {
                            adjElems[i] = adjElemStruct[i].myAdjacentPolygon;
                        }
                        intArray_wh.set(elemoff, adjElems);
                    }
                }
            }
        }, subscribeRatio, minGrainSize);
    }

    void primPrimPoint()
    {
        if (!primPrimPointAttrib)
            setPrimPrimPoint(!outIntermediateAttrib);
            
        intArray_wh = primPrimPointAttrib;
        
        const GA_PointGroup* const seamGroup = pointSeamGroup.getPointGroup();
        
        const GA_Topology& topo = geo->getTopology();
        //topo.makeFull();
        //topo.makePrimitiveRef();
        //topo.makeVertexRef();
        const GA_ATITopology* const primRef  = topo.getPrimitiveRef();
        const GA_ATITopology* const pointRef = topo.getPointRef();
        const GA_ATITopology* const pointVtxRef = topo.getVertexRef();
        const GA_ATITopology* const vtxNextRef  = topo.getVertexNextRef();
        
        UTparallelFor(groupParser.getPrimitiveSplittableRange(), [this, seamGroup, vtxNextRef, pointRef, pointVtxRef, primRef](const GA_SplittableRange& r)
        {
            UT_ValArray<GA_Offset> adjElems;
            GA_Offset vtxoff_next;
            GA_Offset start, end;
            if (seamGroup)
            {
                for (GA_Iterator it(r); it.blockAdvance(start, end); )
                {
                    for (GA_Offset elemoff = start; elemoff < end; ++elemoff)
                    {
                        const GA_OffsetListRef& vertices = geo->getPrimitiveVertexList(elemoff);
                        const GA_Size numvtx = vertices.size();
                        for (GA_Size vtxpnum = 0; vtxpnum < numvtx; ++vtxpnum)
                        {
                            const GA_Offset vtxoff = vertices[vtxpnum];
                            const GA_Offset ptoff = pointRef->getLink(vtxoff);
                            if (seamGroup->contains(ptoff))
                                continue;
                            for (vtxoff_next = pointVtxRef->getLink(ptoff); gfe::isValidOffset(vtxoff_next); vtxoff_next = vtxNextRef->getLink(vtxoff_next))
                            {
                                if (vtxoff_next == vtxoff)
                                    continue;
                                const GA_Offset primoff = primRef->getLink(vtxoff_next);
                                gfe::ArrayOp::uniqueAppendSorted(adjElems, primoff);
                            }
                        }
                        intArray_wh.set(elemoff, adjElems);
                        adjElems.clear();
                    }
                }
            }
            else
            {
                for (GA_Iterator it(r); it.blockAdvance(start, end); )
                {
                    for (GA_Offset elemoff = start; elemoff < end; ++elemoff)
                    {
                        const GA_OffsetListRef& vertices = geo->getPrimitiveVertexList(elemoff);
                        const GA_Size numvtx = vertices.size();
                        for (GA_Size vtxpnum = 0; vtxpnum < numvtx; ++vtxpnum)
                        {
                            const GA_Offset vtxoff = vertices[vtxpnum];
                            const GA_Offset ptoff = pointRef->getLink(vtxoff);
                            for (vtxoff_next = pointVtxRef->getLink(ptoff); gfe::isValidOffset(vtxoff_next); vtxoff_next = vtxNextRef->getLink(vtxoff_next))
                            {
                                if (vtxoff_next == vtxoff)
                                    continue;
                                const GA_Offset primoff = primRef->getLink(vtxoff_next);
                                gfe::ArrayOp::uniqueAppendSorted(adjElems, primoff);
                            }
                        }
                        intArray_wh.set(elemoff, adjElems);
                        adjElems.clear();
                    }
                }
            }
        }, subscribeRatio, minGrainSize);
    }




    
public:
    bool outAsOffset = true;
    gfe::OutArrayType outArrayType = gfe::OutArrayType::Attrib;
    bool outIntermediateAttrib = true;

private:
    GA_Attribute*   vertexPrimIndexAttrib       = nullptr;
    GA_Attribute*   vertexVertexPrimPrevAttrib  = nullptr;
    GA_Attribute*   vertexVertexPrimNextAttrib  = nullptr;
    GA_Attribute*   vertexPointDstAttrib        = nullptr;
    GA_Attribute*   vertexNextEquivAttrib       = nullptr;
    GA_Attribute*   vertexNextEquivNoLoopAttrib = nullptr;
    GA_Attribute*   pointPointEdgeAttrib        = nullptr;
    GA_Attribute*   pointPointPrimAttrib        = nullptr;
    GA_Attribute*   primPrimEdgeAttrib          = nullptr;
    GA_Attribute*   primPrimPointAttrib         = nullptr;
    GA_VertexGroup* vertexNextEquivGroup        = nullptr; // unshared vertex
    GA_VertexGroup* vertexNextEquivNoLoopGroup  = nullptr;
    //GA_EdgeGroup*   unsharedEdgeGroup           = nullptr;
    
    bool calVertexPrimIndexAttrib       = true;
    bool calVertexVertexPrimPrevAttrib  = true;
    bool calVertexVertexPrimNextAttrib  = true;
    bool calVertexPointDstAttrib        = true;
    bool calVertexNextEquivAttrib       = true;
    bool calVertexNextEquivNoLoopAttrib = true;
    bool calPointPointEdgeAttrib        = true;
    bool calPointPointPrimAttrib        = true;
    bool calPrimPrimEdgeAttrib          = true;
    bool calPrimPrimPointAttrib         = true;
    bool calVertexNextEquivGroup        = true;
    bool calVertexNextEquivNoLoopGroup  = true;
        
    bool calTmpVertexVertexPrimPrevAttrib;
    bool calTmpVertexVertexPrimNextAttrib;
        
    bool calTmpVertexNextEquivAttrib      ;
    bool calTmpVertexNextEquivNoLoopAttrib;
    bool calTmpVertexNextEquivGroup       ;
    bool calTmpVertexNextEquivNoLoopGroup ;
        
    GA_RWHandleT<GA_Offset> int0_wh;
    GA_RWHandleT<GA_Offset> int1_wh;
        
    GA_ROHandleT<GA_Offset> int_oh;
    GA_RWHandleT<UT_ValArray<GA_Offset>> intArray_wh;

        
    gfe::GroupParser pointSeamGroup;
    gfe::GroupParser vertexEdgeSeamGroup;
    gfe::GroupParser edgeSeamGroup;

    uint8 kernel = 0;
    exint subscribeRatio = 64;
    exint minGrainSize   = 1024;

#undef __GFE_MeshTopology_VertexNextEquivDual
#undef __GFE_MeshTopology_OptimizeTrivialVertices
    
}; // End of class MeshTopology
_GFEL_END
#endif
