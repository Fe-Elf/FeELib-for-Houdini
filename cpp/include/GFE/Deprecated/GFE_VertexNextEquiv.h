
#pragma once

#ifndef __GFE_VertexNextEquiv_h__
#define __GFE_VertexNextEquiv_h__

//#include "GFE/GFE_VertexNextEquiv.h"


#include "GFE/GFE_TopologyReference.h"

#include "GFE/GFE_GeoFilter.h"





class GFE_VertexNextEquiv : public GFE_AttribFilter {

public:
    using GFE_AttribFilter::GFE_AttribFilter;

    void
        setComputeParm(
            const exint subscribeRatio = 64,
            const exint minGrainSize = 1024
        )
    {
        setHasComputed();
        this->subscribeRatio = subscribeRatio;
        this->minGrainSize = minGrainSize;
    }



private:


    virtual bool
        computeCore() override
    {

        return true;
    }


public:
    
private:
    int subscribeRatio = 8;
    int minGrainSize = 1024;

}; // End of class GFE_VertexNextEquiv







































namespace GFE_VertexNextEquiv_Namespace {



    //Get Vertex Next Equiv Vertex
    static GA_Offset
        vertexNextEquivNoLoop(
            const GA_Detail& geo,
            const GA_Offset vtxoff,
            const GA_Offset dstpt
        )
    {
        const GA_Topology& topo = geo.getTopology();
        const GA_ATITopology* const vtxPointRef = topo.getPointRef();
        const GA_ATITopology* const pointVtxRef = topo.getVertexRef();
        const GA_ATITopology* const vtxNextRef = topo.getVertexNextRef();

        GA_Offset vtxoff_next, ptnum;
        if (dstpt < 0)
        {
            return GA_INVALID_OFFSET;
        }

        for (vtxoff_next = vtxNextRef->getLink(vtxoff); vtxoff_next != GA_INVALID_OFFSET; vtxoff_next = vtxNextRef->getLink(vtxoff_next))
        {
            if (GFE_TopologyReference_Namespace::vertexPointDst(geo, vtxoff_next) != dstpt)
                continue;
            return vtxoff_next;
        }

        ptnum = vtxPointRef->getLink(vtxoff);
        for (vtxoff_next = pointVtxRef->getLink(dstpt); vtxoff_next != GA_INVALID_OFFSET; vtxoff_next = vtxNextRef->getLink(vtxoff_next))
        {
            if (GFE_TopologyReference_Namespace::vertexPointDst(geo, vtxoff_next) != ptnum)
                continue;
            if (dstpt > ptnum)
            {
                return vtxoff_next;
            }
            break;
        }
        return GA_INVALID_OFFSET;
    }


    static GA_Offset
        vertexNextEquivNoLoop(
            const GA_Detail& geo,
            const GA_Offset vtxoff,
            const GA_Offset primoff,
            const GA_Size vtxpnum
        )
    {
        return vertexNextEquivNoLoop(geo, vtxoff, GFE_TopologyReference_Namespace::vertexPointDst(geo, primoff, vtxpnum));
    }

    static GA_Offset
        vertexNextEquivNoLoop(
            const GA_Detail& geo,
            const GA_Offset vtxoff
        )
    {
        return vertexNextEquivNoLoop(geo, vtxoff, GFE_TopologyReference_Namespace::vertexPointDst(geo, vtxoff));
    }







    //Get Vertex Next Equiv Vertex
    static GA_Offset
        vertexNextEquiv(
            const GA_Detail& geo,
            const GA_Offset vtxoff,
            const GA_Offset dstpt
        )
    {
        const GA_Topology& topo = geo.getTopology();
        const GA_ATITopology* const vtxPointRef = topo.getPointRef();
        const GA_ATITopology* const pointVtxRef = topo.getVertexRef();
        const GA_ATITopology* const vtxNextRef = topo.getVertexNextRef();

        GA_Offset vtxoff_next, ptnum;
        if (dstpt < 0)
            return GA_INVALID_OFFSET;

        for (vtxoff_next = vtxNextRef->getLink(vtxoff); vtxoff_next != GA_INVALID_OFFSET; vtxoff_next = vtxNextRef->getLink(vtxoff_next))
        {
            if (GFE_TopologyReference_Namespace::vertexPointDst(geo, vtxoff_next) != dstpt)
                continue;
            return vtxoff_next;
        }

        ptnum = vtxPointRef->getLink(vtxoff);
        for (vtxoff_next = pointVtxRef->getLink(dstpt); vtxoff_next != GA_INVALID_OFFSET; vtxoff_next = vtxNextRef->getLink(vtxoff_next))
        {
            if (GFE_TopologyReference_Namespace::vertexPointDst(geo, vtxoff_next) != ptnum)
                continue;
            return vtxoff_next;
        }
        return GA_INVALID_OFFSET;
    }





    static GA_Offset
        vertexNextEquiv(
            const GA_Detail& geo,
            const GA_Offset vtxoff,
            const GA_Offset primoff,
            const GA_Size vtxpnum
        )
    {
        return vertexNextEquiv(geo, vtxoff, GFE_TopologyReference_Namespace::vertexPointDst(geo, primoff, vtxpnum));
    }

    static GA_Offset
        vertexNextEquiv(
            const GA_Detail& geo,
            const GA_Offset vtxoff
        )
    {
        return vertexNextEquiv(geo, vtxoff, GFE_TopologyReference_Namespace::vertexPointDst(geo, vtxoff));
    }

















    //Get all vertices NextEquiv Vertex
    //template<typename T>
    static void
        vertexNextEquivNoLoop(
            const GA_Detail& geo,
            const GA_RWHandleT<GA_Offset>& attrib_h,
            const GA_VertexGroup* const geoGroup = nullptr,
            const exint subscribeRatio = 64,
            const exint minGrainSize = 64
        )
    {
        const GA_SplittableRange geoSplittableRange0(geo.getVertexRange(geoGroup));
        UTparallelFor(geoSplittableRange0, [&geo, &attrib_h](const GA_SplittableRange& r)
        {
            GA_Offset start, end;
            for (GA_Iterator it(r); it.blockAdvance(start, end); )
            {
                for (GA_Offset elemoff = start; elemoff < end; ++elemoff)
                {
                    attrib_h.set(elemoff, vertexNextEquivNoLoop(geo, elemoff));
                }
            }
        }, subscribeRatio, minGrainSize);
    }




    //Get all vertices NextEquiv Vertex
    //template<typename T>
    static void
        vertexNextEquiv(
            const GA_Detail& geo,
            const GA_RWHandleT<GA_Offset>& attrib_h,
            const GA_VertexGroup* const geoGroup = nullptr,
            const exint subscribeRatio = 64,
            const exint minGrainSize = 64
        )
    {
        const GA_SplittableRange geoSplittableRange0(geo.getVertexRange(geoGroup));
        UTparallelFor(geoSplittableRange0, [&geo, &attrib_h](const GA_SplittableRange& r)
        {
            GA_Offset start, end;
            for (GA_Iterator it(r); it.blockAdvance(start, end); )
            {
                for (GA_Offset elemoff = start; elemoff < end; ++elemoff)
                {
                    attrib_h.set(elemoff, vertexNextEquiv(geo, elemoff));
                }
            }
        }, subscribeRatio, minGrainSize);
    }







    //Get all vertices NextEquiv Vertex
    static void
        vertexNextEquivNoLoop(
            const GA_Detail& geo,
            const GA_RWHandleT<GA_Offset>& attrib_h,
            GA_VertexGroup* const validGroup,
            const GA_ROHandleT<GA_Offset>& dstptAttrib_h,
            const GA_VertexGroup* const geoGroup = nullptr,
            const exint subscribeRatio = 64,
            const exint minGrainSize = 64
        )
    {
        const GA_Topology& topo = geo.getTopology();
        //topo.makeVertexRef();
        const GA_ATITopology* const vtxPointRef = topo.getPointRef();
        const GA_ATITopology* const pointVtxRef = topo.getVertexRef();
        const GA_ATITopology* const vtxNextRef = topo.getVertexNextRef();

        const GA_SplittableRange geoSplittableRange0(geo.getVertexRange(geoGroup));
        UTparallelFor(geoSplittableRange0, 
            [validGroup, &attrib_h, &dstptAttrib_h, vtxPointRef, pointVtxRef, vtxNextRef](const GA_SplittableRange& r)
            {
                GA_Offset vtxoff_next, dstpt, ptnum;
                GA_Offset start, end;
                for (GA_Iterator it(r); it.blockAdvance(start, end); )
                {
                    for (GA_Offset elemoff = start; elemoff < end; ++elemoff)
                    {
                        dstpt = dstptAttrib_h.get(elemoff);
                        if (dstpt < 0)
                        {
                            attrib_h.set(elemoff, GA_INVALID_OFFSET);
                            //validGroup->setElement(elemoff, true);
                            continue;
                        }

                        for (vtxoff_next = vtxNextRef->getLink(elemoff); vtxoff_next != GA_INVALID_OFFSET; vtxoff_next = vtxNextRef->getLink(vtxoff_next))
                        {
                            if (dstptAttrib_h.get(vtxoff_next) != dstpt)
                                continue;
                            dstpt = GA_INVALID_OFFSET;
                            attrib_h.set(elemoff, vtxoff_next);
                            break;
                        }

                        if (dstpt < 0)
                            continue;

                        ptnum = vtxPointRef->getLink(elemoff);
                        for (vtxoff_next = pointVtxRef->getLink(dstpt); vtxoff_next != GA_INVALID_OFFSET; vtxoff_next = vtxNextRef->getLink(vtxoff_next))
                        {
                            if (dstptAttrib_h.get(vtxoff_next) != ptnum)
                                continue;
                            if (dstpt > ptnum)
                            {
                                dstpt = GA_INVALID_OFFSET;
                                attrib_h.set(elemoff, vtxoff_next);
                            }
                            break;
                        }
                        if (dstpt >= 0)
                        {
                            attrib_h.set(elemoff, GA_INVALID_OFFSET);
                            validGroup->setElement(elemoff, true);
                        }
                    }
                }
            }, subscribeRatio, minGrainSize);
        validGroup->invalidateGroupEntries();
    }



    //Get all vertices NextEquiv Vertex
    static void
        vertexNextEquivNoLoop(
            const GA_Detail& geo,
            GA_VertexGroup* const validGroup,
            const GA_ROHandleT<GA_Offset>& dstptAttrib_h,
            const GA_VertexGroup* const geoGroup = nullptr,
            const exint subscribeRatio = 64,
            const exint minGrainSize = 64
        )
    {
        const GA_Topology& topo = geo.getTopology();
        //topo.makeVertexRef();
        const GA_ATITopology* const vtxPointRef = topo.getPointRef();
        const GA_ATITopology* const pointVtxRef = topo.getVertexRef();
        const GA_ATITopology* const vtxNextRef = topo.getVertexNextRef();

        const GA_SplittableRange geoSplittableRange0(geo.getVertexRange(geoGroup));
        UTparallelFor(geoSplittableRange0,
            [validGroup, &dstptAttrib_h, vtxPointRef, pointVtxRef, vtxNextRef](const GA_SplittableRange& r)
        {
            GA_Offset vtxoff_next, dstpt, ptnum;
            GA_Offset start, end;
            for (GA_Iterator it(r); it.blockAdvance(start, end); )
            {
                for (GA_Offset elemoff = start; elemoff < end; ++elemoff)
                {
                    dstpt = dstptAttrib_h.get(elemoff);
                    if (dstpt < 0)
                    {
                        //validGroup->setElement(elemoff, true);
                        continue;
                    }

                    for (vtxoff_next = vtxNextRef->getLink(elemoff); vtxoff_next != GA_INVALID_OFFSET; vtxoff_next = vtxNextRef->getLink(vtxoff_next))
                    {
                        if (dstptAttrib_h.get(vtxoff_next) != dstpt)
                            continue;
                        dstpt = GA_INVALID_OFFSET;
                        break;
                    }

                    if (dstpt < 0)
                        continue;

                    ptnum = vtxPointRef->getLink(elemoff);
                    for (vtxoff_next = pointVtxRef->getLink(dstpt); vtxoff_next != GA_INVALID_OFFSET; vtxoff_next = vtxNextRef->getLink(vtxoff_next))
                    {
                        if (dstptAttrib_h.get(vtxoff_next) != ptnum)
                            continue;
                        if (dstpt > ptnum)
                        {
                            dstpt = GA_INVALID_OFFSET;
                        }
                        break;
                    }
                    if (dstpt >= 0)
                    {
                        validGroup->setElement(elemoff, true);
                    }
                }
            }
        }, subscribeRatio, minGrainSize);
        validGroup->invalidateGroupEntries();
    }


    //Get all vertices NextEquiv Vertex
    static void
        vertexNextEquivNoLoop(
            const GA_Detail& geo,
            const GA_RWHandleT<GA_Offset>& attrib_h,
            const GA_ROHandleT<GA_Offset>& dstptAttrib_h,
            const GA_VertexGroup* const geoGroup = nullptr,
            const exint subscribeRatio = 64,
            const exint minGrainSize = 64
        )
    {
        const GA_Topology& topo = geo.getTopology();
        //topo.makeVertexRef();
        const GA_ATITopology* const vtxPointRef = topo.getPointRef();
        const GA_ATITopology* const pointVtxRef = topo.getVertexRef();
        const GA_ATITopology* const vtxNextRef = topo.getVertexNextRef();

        const GA_SplittableRange geoSplittableRange0(geo.getVertexRange(geoGroup));
        UTparallelFor(geoSplittableRange0,
            [&attrib_h, &dstptAttrib_h, vtxPointRef, pointVtxRef, vtxNextRef](const GA_SplittableRange& r)
        {
            GA_Offset vtxoff_next, dstpt, ptnum;
            GA_Offset start, end;
            for (GA_Iterator it(r); it.blockAdvance(start, end); )
            {
                for (GA_Offset elemoff = start; elemoff < end; ++elemoff)
                {
                    dstpt = dstptAttrib_h.get(elemoff);
                    if (dstpt < 0)
                    {
                        attrib_h.set(elemoff, GA_INVALID_OFFSET);
                        continue;
                    }

                    for (vtxoff_next = vtxNextRef->getLink(elemoff); vtxoff_next != GA_INVALID_OFFSET; vtxoff_next = vtxNextRef->getLink(vtxoff_next))
                    {
                        if (dstptAttrib_h.get(vtxoff_next) != dstpt)
                            continue;
                        dstpt = GA_INVALID_OFFSET;
                        attrib_h.set(elemoff, vtxoff_next);
                        break;
                    }

                    if (dstpt < 0)
                        continue;

                    ptnum = vtxPointRef->getLink(elemoff);
                    for (vtxoff_next = pointVtxRef->getLink(dstpt); vtxoff_next != GA_INVALID_OFFSET; vtxoff_next = vtxNextRef->getLink(vtxoff_next))
                    {
                        if (dstptAttrib_h.get(vtxoff_next) != ptnum)
                            continue;
                        if (dstpt > ptnum)
                        {
                            dstpt = GA_INVALID_OFFSET;
                            attrib_h.set(elemoff, vtxoff_next);
                        }
                        break;
                    }
                    if (dstpt >= 0)
                    {
                        attrib_h.set(elemoff, GA_INVALID_OFFSET);
                    }
                }
            }
        }, subscribeRatio, minGrainSize);
    }







    //Get all vertices NextEquiv Vertex
    static void
        vertexNextEquiv(
            const GA_Detail& geo,
            const GA_RWHandleT<GA_Offset>& attrib_h,
            GA_VertexGroup* unsharedGroup,
            const GA_ROHandleT<GA_Offset>& dstptAttrib_h,
            const GA_VertexGroup* const geoGroup = nullptr,
            const exint subscribeRatio = 64,
            const exint minGrainSize = 64
        )
    {
        const GA_Topology& topo = geo.getTopology();
        //topo.makeVertexRef();
        const GA_ATITopology* const vtxPointRef = topo.getPointRef();
        const GA_ATITopology* const pointVtxRef = topo.getVertexRef();
        const GA_ATITopology* const vtxNextRef = topo.getVertexNextRef();

        const GA_SplittableRange geoSplittableRange0(geo.getVertexRange(geoGroup));
        UTparallelFor(geoSplittableRange0,
            [unsharedGroup, &attrib_h, &dstptAttrib_h, vtxPointRef, pointVtxRef, vtxNextRef](const GA_SplittableRange& r)
        {
            GA_Offset vtxoff_next, dstpt, ptnum;
            GA_Offset start, end;
            for (GA_Iterator it(r); it.blockAdvance(start, end); )
            {
                for (GA_Offset elemoff = start; elemoff < end; ++elemoff)
                {
                    dstpt = dstptAttrib_h.get(elemoff);
                    if (dstpt < 0)
                    {
                        attrib_h.set(elemoff, GA_INVALID_OFFSET);
                        continue;
                    }

                    for (vtxoff_next = vtxNextRef->getLink(elemoff); vtxoff_next != GA_INVALID_OFFSET; vtxoff_next = vtxNextRef->getLink(vtxoff_next))
                    {
                        if (dstptAttrib_h.get(vtxoff_next) != dstpt)
                            continue;
                        dstpt = GA_INVALID_OFFSET;
                        attrib_h.set(elemoff, vtxoff_next);
                        break;
                    }

                    if (dstpt < 0)
                        continue;

                    ptnum = vtxPointRef->getLink(elemoff);
                    for (vtxoff_next = pointVtxRef->getLink(dstpt); vtxoff_next != GA_INVALID_OFFSET; vtxoff_next = vtxNextRef->getLink(vtxoff_next))
                    {
                        if (dstptAttrib_h.get(vtxoff_next) != ptnum)
                            continue;
                        dstpt = GA_INVALID_OFFSET;
                        attrib_h.set(elemoff, vtxoff_next);
                        break;
                    }
                    if (dstpt >= 0)
                    {
                        attrib_h.set(elemoff, GA_INVALID_OFFSET);
                        unsharedGroup->setElement(elemoff, true);
                    }
                }
            }
        }, subscribeRatio, minGrainSize);
    }


    //Get all vertices NextEquiv Vertex
    static void
        vertexNextEquiv(
            const GA_Detail& geo,
            const GA_RWHandleT<GA_Offset>& attrib_h,
            const GA_ROHandleT<GA_Offset>& dstptAttrib_h,
            const GA_VertexGroup* const geoGroup = nullptr,
            const exint subscribeRatio = 64,
            const exint minGrainSize = 64
        )
    {
        const GA_Topology& topo = geo.getTopology();
        //topo.makeVertexRef();
        const GA_ATITopology* const vtxPointRef = topo.getPointRef();
        const GA_ATITopology* const pointVtxRef = topo.getVertexRef();
        const GA_ATITopology* const vtxNextRef = topo.getVertexNextRef();

        const GA_SplittableRange geoSplittableRange0(geo.getVertexRange(geoGroup));
        UTparallelFor(geoSplittableRange0,
            [&attrib_h, &dstptAttrib_h, vtxPointRef, pointVtxRef, vtxNextRef](const GA_SplittableRange& r)
        {
            GA_Offset vtxoff_next, dstpt, ptnum;
            GA_Offset start, end;
            for (GA_Iterator it(r); it.blockAdvance(start, end); )
            {
                for (GA_Offset elemoff = start; elemoff < end; ++elemoff)
                {
                    dstpt = dstptAttrib_h.get(elemoff);
                    if (dstpt < 0)
                    {
                        attrib_h.set(elemoff, GA_INVALID_OFFSET);
                        continue;
                    }

                    for (vtxoff_next = vtxNextRef->getLink(elemoff); vtxoff_next != GA_INVALID_OFFSET; vtxoff_next = vtxNextRef->getLink(vtxoff_next))
                    {
                        if (dstptAttrib_h.get(vtxoff_next) != dstpt)
                            continue;
                        dstpt = GA_INVALID_OFFSET;
                        attrib_h.set(elemoff, vtxoff_next);
                        break;
                    }

                    if (dstpt < 0)
                        continue;

                    ptnum = vtxPointRef->getLink(elemoff);
                    for (vtxoff_next = pointVtxRef->getLink(dstpt); vtxoff_next != GA_INVALID_OFFSET; vtxoff_next = vtxNextRef->getLink(vtxoff_next))
                    {
                        if (dstptAttrib_h.get(vtxoff_next) != ptnum)
                            continue;
                        dstpt = GA_INVALID_OFFSET;
                        attrib_h.set(elemoff, vtxoff_next);
                        break;
                    }
                    if (dstpt >= 0)
                    {
                        attrib_h.set(elemoff, GA_INVALID_OFFSET);
                    }
                }
            }
        }, subscribeRatio, minGrainSize);
    }




    //Get all vertices NextEquiv Vertex
    static void
        vertexNextEquiv(
            const GA_Detail& geo,
            GA_VertexGroup* const unsharedGroup,
            const GA_ROHandleT<GA_Offset>& dstptAttrib_h,
            const GA_VertexGroup* const geoGroup = nullptr,
            const exint subscribeRatio = 64,
            const exint minGrainSize = 64
        )
    {
        const GA_Topology& topo = geo.getTopology();
        //topo.makeVertexRef();
        const GA_ATITopology* const vtxPointRef = topo.getPointRef();
        const GA_ATITopology* const pointVtxRef = topo.getVertexRef();
        const GA_ATITopology* const vtxNextRef = topo.getVertexNextRef();

        const GA_SplittableRange geoSplittableRange0(geo.getVertexRange(geoGroup));
        UTparallelFor(geoSplittableRange0,
            [unsharedGroup, &dstptAttrib_h, vtxPointRef, pointVtxRef, vtxNextRef](const GA_SplittableRange& r)
        {
            GA_Offset vtxoff_next, dstpt, ptnum;
            GA_Offset start, end;
            for (GA_Iterator it(r); it.blockAdvance(start, end); )
            {
                for (GA_Offset elemoff = start; elemoff < end; ++elemoff)
                {
                    dstpt = dstptAttrib_h.get(elemoff);
                    if (dstpt < 0)
                    {
                        continue;
                    }

                    for (vtxoff_next = vtxNextRef->getLink(elemoff); vtxoff_next != GA_INVALID_OFFSET; vtxoff_next = vtxNextRef->getLink(vtxoff_next))
                    {
                        if (dstptAttrib_h.get(vtxoff_next) != dstpt)
                            continue;
                        dstpt = GA_INVALID_OFFSET;
                        break;
                    }

                    if (dstpt < 0)
                        continue;

                    ptnum = vtxPointRef->getLink(elemoff);
                    for (vtxoff_next = pointVtxRef->getLink(dstpt); vtxoff_next != GA_INVALID_OFFSET; vtxoff_next = vtxNextRef->getLink(vtxoff_next))
                    {
                        if (dstptAttrib_h.get(vtxoff_next) != ptnum)
                            continue;
                        dstpt = GA_INVALID_OFFSET;
                        break;
                    }
                    if (dstpt >= 0)
                    {
                        unsharedGroup->setElement(elemoff, true);
                    }
                }
            }
        }, subscribeRatio, minGrainSize);
    }











    //GFE_VertexNextEquiv::addAttribVertexNextEquivNoLoop(geo, name, groupName, name, geoGroup, defaults, creation_args, attribute_options, storage, reuse, subscribeRatio, minGrainSize);

    static GA_Attribute*
        addAttribVertexNextEquivNoLoop(
            GA_Detail& geo,
            GA_VertexGroup*& validGroup,
            const UT_StringHolder& name = "__topo_nextEquivNoLoop",
            const UT_StringHolder& groupName = "__topo_nextEquivValid",
            const GA_VertexGroup* const geoGroup = nullptr,
            const GA_Defaults& defaults = GA_Defaults(-1),
            const GA_Storage storage = GA_STORE_INVALID,
            const UT_Options* const creation_args = nullptr,
            const GA_AttributeOptions* const attribute_options = nullptr,
            const GA_ReuseStrategy& reuse = GA_ReuseStrategy(),
            const exint subscribeRatio = 32,
            const exint minGrainSize = 1024
        )
    {
        GA_Attribute* attribPtr = geo.findVertexAttribute(GFE_TOPO_SCOPE, name);
        if (attribPtr && geo.findVertexGroup(name))
            return attribPtr;
        const GA_Storage finalStorage = storage == GA_STORE_INVALID ? GFE_Type::getPreferredStorageI(geo) : storage;
        attribPtr = geo.getAttributes().createTupleAttribute(GA_ATTRIB_VERTEX, GFE_TOPO_SCOPE, name, finalStorage, 1, defaults, creation_args, attribute_options, reuse);
        //attribPtr = geo.addIntTuple(GA_ATTRIB_VERTEX, GFE_TOPO_SCOPE, name, 1, defaults, creation_args, attribute_options, finalStorage, reuse);

        validGroup = geo.newVertexGroup(groupName);
        const GA_Attribute* const refAttrib = GFE_TopologyReference_Namespace::addAttribVertexPointDst(geo, geoGroup, finalStorage);
        vertexNextEquivNoLoop(geo, attribPtr, validGroup, refAttrib, geoGroup, subscribeRatio, minGrainSize);
        return attribPtr;
    }


    static GA_AttributeUPtr
        addDetachedAttribVertexNextEquivNoLoop(
            GA_Detail& geo,
            GA_VertexGroupUPtr& validGroup,
            const GA_VertexGroup* const geoGroup = nullptr,
            const GA_Defaults& defaults = GA_Defaults(-1),
            const GA_Storage storage = GA_STORE_INVALID,
            const GA_AttributeOptions* const attribute_options = nullptr,
            const exint subscribeRatio = 32,
            const exint minGrainSize = 1024
        )
    {
        const GA_Storage finalStorage = storage == GA_STORE_INVALID ? GFE_Type::getPreferredStorageI(geo) : storage;
        GA_AttributeUPtr attribPtr = geo.getAttributes().createDetachedTupleAttribute(GA_ATTRIB_VERTEX, finalStorage, 1, defaults, attribute_options);
        //attribPtr = geo.addIntTuple(GA_ATTRIB_VERTEX, GFE_TOPO_SCOPE, name, 1, defaults, creation_args, attribute_options, finalStorage, reuse);

        validGroup = geo.createDetachedVertexGroup();
        const GA_AttributeUPtr refAttrib = GFE_TopologyReference_Namespace::addDetachedAttribVertexPointDst(geo, geoGroup, finalStorage);
        vertexNextEquivNoLoop(geo, attribPtr.get(), validGroup.get(), refAttrib.get(), geoGroup, subscribeRatio, minGrainSize);
        return attribPtr;
    }







    //GFE_VertexNextEquiv::addGroupVertexNextEquivNoLoop(geo, name, geoGroup, subscribeRatio, minGrainSize);

    static GA_VertexGroup*
        addGroupVertexNextEquivNoLoop(
            GA_Detail& geo,
            const GA_VertexGroup* const geoGroup = nullptr,
            const GA_Storage storage = GA_STORE_INVALID,
            const UT_StringHolder& name = "__topo_nextEquivValid",
            const exint subscribeRatio = 32,
            const exint minGrainSize = 1024
        )
    {
        GA_VertexGroup* groupPtr = geo.findVertexGroup(name);
        if (groupPtr)
            return groupPtr;

        groupPtr = geo.newVertexGroup(name);
        const GA_Attribute* const refAttrib = GFE_TopologyReference_Namespace::addAttribVertexPointDst(geo, geoGroup, storage);
        vertexNextEquivNoLoop(geo, groupPtr, refAttrib, geoGroup, subscribeRatio, minGrainSize);
        return groupPtr;
    }

    static GA_VertexGroupUPtr
        addDetachedGroupVertexNextEquivNoLoop(
            const GA_Detail& geo,
            const GA_VertexGroup* const geoGroup = nullptr,
            const GA_Storage storage = GA_STORE_INVALID,
            const exint subscribeRatio = 32,
            const exint minGrainSize = 1024
        )
    {
        GA_VertexGroupUPtr groupPtr = geo.createDetachedVertexGroup();
        const GA_AttributeUPtr refAttrib = GFE_TopologyReference_Namespace::addDetachedAttribVertexPointDst(geo, geoGroup, storage);
        vertexNextEquivNoLoop(geo, groupPtr.get(), refAttrib.get(), geoGroup, subscribeRatio, minGrainSize);
        return groupPtr;
    }








    //GFE_VertexNextEquiv::addAttribVertexNextEquiv(geo, name, geoGroup, defaults, creation_args, attribute_options, storage, reuse, subscribeRatio, minGrainSize);

    static GA_Attribute*
        addAttribVertexNextEquivNoLoop(
            GA_Detail& geo,
            const GA_VertexGroup* const geoGroup = nullptr,
            const GA_Storage storage = GA_STORE_INVALID,
            const UT_StringHolder& name = "__topo_nextEquivNoLoop",
            const GA_Defaults& defaults = GA_Defaults(-1),
            const UT_Options* const creation_args = nullptr,
            const GA_AttributeOptions* const attribute_options = nullptr,
            const GA_ReuseStrategy& reuse = GA_ReuseStrategy(),
            const exint subscribeRatio = 32,
            const exint minGrainSize = 1024
        )
    {
        GA_Attribute* attribPtr = geo.findVertexAttribute(name);
        if (attribPtr)
            return attribPtr;
        const GA_Storage finalStorage = storage == GA_STORE_INVALID ? GFE_Type::getPreferredStorageI(geo) : storage;
        attribPtr = geo.getAttributes().createTupleAttribute(GA_ATTRIB_VERTEX, GFE_TOPO_SCOPE, name, finalStorage, 1, defaults, creation_args, attribute_options, reuse);
        const GA_Attribute* const refAttrib = GFE_TopologyReference_Namespace::addAttribVertexPointDst(geo, geoGroup, finalStorage);
        vertexNextEquivNoLoop(geo, attribPtr, refAttrib, geoGroup, subscribeRatio, minGrainSize);
        return attribPtr;
    }

    static GA_AttributeUPtr
        addDetachedAttribVertexNextEquivNoLoop(
            const GA_Detail& geo,
            const GA_VertexGroup* const geoGroup = nullptr,
            const GA_Storage storage = GA_STORE_INVALID,
            const GA_Defaults& defaults = GA_Defaults(-1),
            const GA_AttributeOptions* const attribute_options = nullptr,
            const exint subscribeRatio = 32,
            const exint minGrainSize = 1024
        )
    {
        const GA_Storage finalStorage = storage == GA_STORE_INVALID ? GFE_Type::getPreferredStorageI(geo) : storage;
        GA_AttributeUPtr attribPtr = geo.getAttributes().createDetachedTupleAttribute(GA_ATTRIB_VERTEX, finalStorage, 1, defaults, attribute_options);
        const GA_AttributeUPtr refAttrib = GFE_TopologyReference_Namespace::addDetachedAttribVertexPointDst(geo, geoGroup, finalStorage);
        vertexNextEquivNoLoop(geo, attribPtr.get(), refAttrib.get(), geoGroup, subscribeRatio, minGrainSize);
        return attribPtr;
    }













    //GFE_VertexNextEquiv::addAttribVertexNextEquiv(geo, groupPtr, name, geoGroup, defaults, storage, creation_args, attribute_options, reuse, subscribeRatio, minGrainSize);

    static GA_Attribute*
        addAttribVertexNextEquiv(
            GA_Detail& geo,
            GA_VertexGroup*& groupPtr,
            const GA_VertexGroup* const geoGroup = nullptr,
            const GA_Storage storage = GA_STORE_INVALID,
            const UT_StringHolder& name = "__topo_nextEquiv",
            const UT_StringHolder& groupName = "__topo_unshared",
            const GA_Defaults& defaults = GA_Defaults(-1),
            const UT_Options* const creation_args = nullptr,
            const GA_AttributeOptions* const attribute_options = nullptr,
            const GA_ReuseStrategy& reuse = GA_ReuseStrategy(),
            const exint subscribeRatio = 32,
            const exint minGrainSize = 1024
        )
    {
        GA_Attribute* attribPtr = geo.findVertexAttribute(name);
        if (attribPtr)
            return attribPtr;
        const GA_Storage finalStorage = storage == GA_STORE_INVALID ? GFE_Type::getPreferredStorageI(geo) : storage;
        const GA_Attribute* const refAttrib = GFE_TopologyReference_Namespace::addAttribVertexPointDst(geo, geoGroup, finalStorage);
        attribPtr = geo.getAttributes().createTupleAttribute(GA_ATTRIB_VERTEX, GFE_TOPO_SCOPE, name, finalStorage, 1, defaults, creation_args, attribute_options, reuse);
        groupPtr = geo.newVertexGroup(groupName);
        vertexNextEquiv(geo, attribPtr, groupPtr, refAttrib, geoGroup, subscribeRatio, minGrainSize);
        return attribPtr;
    }


    static GA_AttributeUPtr
        addDetachedAttribVertexNextEquiv(
            const GA_Detail& geo,
            GA_VertexGroupUPtr& groupPtr,
            const GA_VertexGroup* const geoGroup = nullptr,
            const GA_Storage storage = GA_STORE_INVALID,
            const GA_Defaults& defaults = GA_Defaults(-1),
            const GA_AttributeOptions* const attribute_options = nullptr,
            const exint subscribeRatio = 32,
            const exint minGrainSize = 1024
        )
    {
        const GA_Storage finalStorage = storage == GA_STORE_INVALID ? GFE_Type::getPreferredStorageI(geo) : storage;
        const GA_AttributeUPtr refAttribUPtr = GFE_TopologyReference_Namespace::addDetachedAttribVertexPointDst(geo, geoGroup, finalStorage);
        GA_AttributeUPtr attribPtr = geo.getAttributes().createDetachedTupleAttribute(GA_ATTRIB_VERTEX, finalStorage, 1, defaults, attribute_options);
        groupPtr = geo.createDetachedVertexGroup();
        vertexNextEquiv(geo, attribPtr.get(), groupPtr.get(), refAttribUPtr.get(), geoGroup, subscribeRatio, minGrainSize);
        return attribPtr;
    }






    //GFE_VertexNextEquiv::addAttribVertexNextEquiv(geo, name, geoGroup, defaults, storage, nullptr, nullptr, GA_ReuseStrategy(), subscribeRatio, minGrainSize);

    //GA_VertexGroup* unsharedGroup = GFE_VertexNextEquiv::addGroupVertexNextEquiv(outGeo0, geo0VtxGroup, inStorageI, "__topo_unshared", subscribeRatio, minGrainSize);
    static GA_Attribute*
        addAttribVertexNextEquiv(
            GA_Detail& geo,
            const GA_VertexGroup* const geoGroup = nullptr,
            const GA_Storage storage = GA_STORE_INVALID,
            const UT_StringHolder& name = "__topo_nextEquiv",
            const GA_Defaults& defaults = GA_Defaults(-1),
            const UT_Options* const creation_args = nullptr,
            const GA_AttributeOptions* const attribute_options = nullptr,
            const GA_ReuseStrategy& reuse = GA_ReuseStrategy(),
            const exint subscribeRatio = 32,
            const exint minGrainSize = 1024
        )
    {
        GA_Attribute* attribPtr = geo.findVertexAttribute(name);
        if (attribPtr)
            return attribPtr;
        const GA_Storage finalStorage = storage == GA_STORE_INVALID ? GFE_Type::getPreferredStorageI(geo) : storage;
        attribPtr = geo.getAttributes().createTupleAttribute(GA_ATTRIB_VERTEX, GFE_TOPO_SCOPE, name, finalStorage, 1, defaults, creation_args, attribute_options, reuse);
        const GA_Attribute* const refAttrib = GFE_TopologyReference_Namespace::addAttribVertexPointDst(geo, geoGroup, finalStorage);
        vertexNextEquiv(geo, attribPtr, refAttrib, geoGroup, subscribeRatio, minGrainSize);
        return attribPtr;
    }

    //GA_VertexGroup* unsharedGroup = GFE_VertexNextEquiv::addGroupVertexNextEquiv(outGeo0, geo0VtxGroup, inStorageI, "__topo_unshared", subscribeRatio, minGrainSize);
    static GA_AttributeUPtr
        addDetachedAttribVertexNextEquiv(
            const GA_Detail& geo,
            const GA_VertexGroup* const geoGroup = nullptr,
            const GA_Storage storage = GA_STORE_INVALID,
            const GA_Defaults& defaults = GA_Defaults(-1),
            const GA_AttributeOptions* const attribute_options = nullptr,
            const exint subscribeRatio = 32,
            const exint minGrainSize = 1024
        )
    {
        const GA_Storage finalStorage = storage == GA_STORE_INVALID ? GFE_Type::getPreferredStorageI(geo) : storage;
        GA_AttributeUPtr attribPtr = geo.getAttributes().createDetachedTupleAttribute(GA_ATTRIB_VERTEX, finalStorage, 1, defaults, attribute_options);
        const GA_AttributeUPtr refAttribUPtr = GFE_TopologyReference_Namespace::addDetachedAttribVertexPointDst(geo, geoGroup, finalStorage);
        vertexNextEquiv(geo, attribPtr.get(), refAttribUPtr.get(), geoGroup, subscribeRatio, minGrainSize);
        return attribPtr;
    }



    //GFE_VertexNextEquiv::addAttribVertexNextEquiv(geo, name, geoGroup, subscribeRatio, minGrainSize);

    //GA_VertexGroup* unsharedGroup = GFE_VertexNextEquiv::addGroupVertexNextEquiv(outGeo0, geo0VtxGroup, inStorageI, "__topo_unshared", subscribeRatio, minGrainSize);
    static GA_VertexGroup*
    addGroupVertexNextEquiv(
        GA_Detail& geo,
        const GA_VertexGroup* const geoGroup = nullptr,
        const GA_Storage storage = GA_STORE_INVALID,
        const UT_StringHolder& name = "__topo_unshared",
        const exint subscribeRatio = 32,
        const exint minGrainSize = 1024
    )
    {
        GA_VertexGroup* groupPtr = geo.findVertexGroup(name);
        if (groupPtr)
            return groupPtr;
        const GA_Attribute* const refAttrib = GFE_TopologyReference_Namespace::addAttribVertexPointDst(geo, geoGroup, storage);
        groupPtr = geo.newVertexGroup(name);
        vertexNextEquiv(static_cast<const GA_Detail&>(geo), groupPtr, refAttrib, geoGroup, subscribeRatio, minGrainSize);
        return groupPtr;
    }


    //GA_VertexGroup* unsharedGroup = GFE_VertexNextEquiv::addGroupVertexNextEquiv(outGeo0, geo0VtxGroup, inStorageI, "__topo_unshared", subscribeRatio, minGrainSize);
    static GA_VertexGroupUPtr
        addDetachedGroupVertexNextEquiv(
            const GA_Detail& geo,
            const GA_VertexGroup* const geoGroup = nullptr,
            const GA_Storage storage = GA_STORE_INVALID,
            const exint subscribeRatio = 32,
            const exint minGrainSize = 1024
        )
    {
        const GA_AttributeUPtr refAttribUPtr = GFE_TopologyReference_Namespace::addDetachedAttribVertexPointDst(geo, geoGroup, storage);
        GA_VertexGroupUPtr groupPtr = geo.createDetachedVertexGroup();
        vertexNextEquiv(static_cast<const GA_Detail&>(geo), groupPtr.get(), refAttribUPtr.get(), geoGroup, subscribeRatio, minGrainSize);
        return groupPtr;
    }



} // End of namespace GFE_VertexNextEquiv

#endif
