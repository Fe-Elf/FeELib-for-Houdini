
#pragma once

#ifndef __GFE_SplitPoint_h__
#define __GFE_SplitPoint_h__

#include "GFE/GFE_SplitPoint.h"

#include "GFE/GFE_GeoFilter.h"



#include "GA/GA_AIFCompare.h"
#include "GA/GA_ElementWrangler.h"
#include "GU/GU_Promote.h"


#if 0

GFE_SplitPoint splitPoint(geo, cookparms);
splitPoint.groupParser.setGroup(groupParser);
    
splitPoint.getInAttribArray().oappendPrims(   );
splitPoint.getInAttribArray().oappendVertices();
splitPoint.getInGroupArray() .oappendPrims(   );
splitPoint.getInGroupArray() .oappendVertices();
    
splitPoint.getOutGroupArray().appendEdges();
    
splitPoint.setComputeParm(, );
//splitPoint.splitAttribTol = 1e-05;
//splitPoint.promoteAttrib  = true;
splitPoint.outEdgeGroupAsVertex = true;
    
splitPoint.compute();

#endif



class GFE_SplitPoint : public GFE_AttribCreateFilter {

#define __GFE_TEMP_SplitPoint_VertexEdgeGroupName "__GFE_TEMP_SplitPoint_VertexEdgeGroupName%d"

public:
    using GFE_AttribCreateFilter::GFE_AttribCreateFilter;

    ~GFE_SplitPoint()
    {
    }


    void
        setComputeParm(
            const fpreal splitAttribTol = 1e-05,
            const bool promoteAttrib = false
            // ,const exint subscribeRatio = 64,
            // const exint minGrainSize   = 64
        )
    {
        setHasComputed();
        this->splitAttribTol = splitAttribTol;
        this->promoteAttrib  = promoteAttrib;
        // this->subscribeRatio = subscribeRatio;
        // this->minGrainSize   = minGrainSize;
    }
    
    SYS_FORCE_INLINE size_t getNumPointAdded() const
    { return numPointAdded; }

    
    // SYS_FORCE_INLINE void setSplitByAttrib(const GA_Attribute* const attribPtr = nullptr)
    // {
    //     splitAttribPtr = attribPtr;
    // }
    //
    // void setSplitByAttrib(const GA_AttributeOwner owner, const UT_StringRef& name)
    // {
    //     if (owner == GA_ATTRIB_VERTEX)
    //     {
    //         splitAttribPtr = geo->findAttribute(owner, name);
    //         if (!splitAttribPtr)
    //         {
    //             splitAttribPtr = geo->vertexGroups().find(name);
    //         }
    //     }
    //     else if (owner == GA_ATTRIB_PRIMITIVE)
    //     {
    //         splitAttribPtr = geo->findAttribute(owner, name);
    //         if (!splitAttribPtr)
    //         {
    //             splitAttribPtr = geo->primitiveGroups().find(name);
    //         }
    //     }
    //     else
    //     {
    //         splitAttribPtr = geo->findAttribute(GA_ATTRIB_VERTEX, name);
    //         if (!splitAttribPtr)
    //         {
    //             splitAttribPtr = geo->findAttribute(GA_ATTRIB_PRIMITIVE, name);
    //             if (!splitAttribPtr)
    //             {
    //                 splitAttribPtr = geo->vertexGroups().find(name);
    //                 if (!splitAttribPtr)
    //                 {
    //                     splitAttribPtr = geo->primitiveGroups().find(name);
    //                 }
    //             }
    //         }
    //     }
    //     if (!splitAttribPtr && cookparms)
    //     {
    //         cookparms->sopAddWarning(SOP_MESSAGE, "Couldn't find specified vertex or primitive attribute");
    //     }
    // }

    void bumpDataId() const override
    {
        if (numPointAdded <= 0)
            return;
        // If we'd only added/removed points, we could use
        // geo->bumpDataIdsForAddOrRemove(true, false, false),
        // but we also rewired vertices, so we need to bump the
        // linked-list topology attributes.
        geo->getAttributes().bumpAllDataIds(GA_ATTRIB_POINT);
        GA_ATITopology* topo = geo->getTopology().getPointRef();
        if (topo)
            topo->bumpDataId();
        topo = geo->getTopology().getVertexNextRef();
        if (topo)
            topo->bumpDataId();
        topo = geo->getTopology().getVertexPrevRef();
        if (topo)
            topo->bumpDataId();
        // Edge groups might also be affected, if any edges
        // were on points that were split, so we bump their
        // data IDs, just in case.
        geo->edgeGroups().bumpAllDataIds();
    }
    
private:

    virtual bool
        computeCore() override
    {
        if (geo->getNumPoints() == 0)
            return true;

        if (groupParser.isEmpty())
            return true;


        
        GFE_GroupArray edgeGroupArray(geo, cookparms);
        GFE_GroupArray vertexEdgeGroupArray(geo, cookparms);
        const size_t sizeGroup = getOutGroupArray().size();
        for (size_t i = 0; i < sizeGroup; ++i)
        {
            switch (getOutGroupArray()[i]->classType())
            {
            case GA_GROUP_PRIMITIVE: break;
            case GA_GROUP_POINT:     break;
            case GA_GROUP_VERTEX:    break;
            case GA_GROUP_EDGE:
            {
                const size_t len = edgeGroupArray.size();
                edgeGroupArray.append(getOutGroupArray()[i]);
                if (outEdgeGroupAsVertex)
                    vertexEdgeGroupArray.findOrCreateVertex(false, getOutGroupArray()[i]->getName());
                else
                {
                    UT_WorkBuffer name;
                    name.sprintf(__GFE_TEMP_SplitPoint_VertexEdgeGroupName, i);
                    vertexEdgeGroupArray.findOrCreateVertex(false, name);
                }
                GFE_GroupUnion::groupUnion<GA_ATTRIB_OWNER_N>(*vertexEdgeGroupArray.getVertexGroup(len), *edgeGroupArray.getEdgeGroup(len));
            }
            break;
            default: UT_ASSERT_MSG(0, "Unhandled Group Type"); break;
            }
        }
        UT_ASSERT_P(edgeGroupArray.size() == vertexEdgeGroupArray.size());


        
        numPointAdded = 0;

        
        const GA_ElementGroup* const group = groupParser.getElementGroup();
        if (getInAttribArray().isEmpty() && getInGroupArray().isEmpty())
        {
            splitPoint();
        }
        else
        {
            const auto attribSize = getInAttribArray().size();
            for (size_t i = 0; i < attribSize; ++i)
            {
                splitAttrib = getInAttribArray()[i];
                splitPointByAttrib();
            }
            if (promoteAttrib)
            {
                for (size_t i = 0; i < attribSize; ++i)
                {
                    GA_Attribute* const attrib = getInAttribArray()[i];
                    GU_Promote::promote(*geo->asGU_Detail(), attrib, GA_ATTRIB_POINT, true, GU_Promote::GU_PROMOTE_FIRST);
                }
            }
            
            const auto groupSize = getInGroupArray().size();
            for (size_t i = 0; i < groupSize; ++i)
            {
                splitPointByAttrib(getInGroupArray()[i]);
            }
            if (promoteAttrib)
            {
                for (size_t i = 0; i < groupSize; ++i)
                {
                    GA_ElementGroup* const attrib = getInGroupArray().getElementGroup(i);
                    GU_Promote::promote(*geo->asGU_Detail(), attrib, GA_ATTRIB_POINT, true, GU_Promote::GU_PROMOTE_FIRST);
                }
            }
        }

        UT_ASSERT_P(edgeGroupArray.size() == vertexEdgeGroupArray.size());
        const size_t sizeEdgeGroup = vertexEdgeGroupArray.size();
        for (size_t i = 0; i < sizeEdgeGroup; ++i)
        {
            GFE_GroupUnion::groupUnion(*edgeGroupArray.getEdgeGroup(i), *vertexEdgeGroupArray.getVertexGroup(i));
        }
        vertexEdgeGroupArray.destroyAll();
        
        return true;
    }




    void splitPoint()
    {
        UT_AutoInterrupt boss("Making Unique Points");
        if (boss.wasInterrupted())
            return;

        char bcnt = 0;
        
        const GA_GroupType groupType = groupParser.classType();
        UT_UniquePtr<GA_PointWrangler> ptwrangler(nullptr);

        const GA_ElementGroup* const group = groupParser.getElementGroup();
        
        UT_SmallArray<GA_Offset> vtxoffs;
        GA_Offset ptoff_end = geo->getPointMap().lastOffset() + 1;
        
        GA_Offset start, end;
        for (GA_Iterator it(groupParser.getPointSplittableRange()); it.blockAdvance(start, end); )
        {
            if (start >= ptoff_end)
                break;
            if (end > ptoff_end)
                end = ptoff_end;

            for (GA_Offset ptoff = start; ptoff < end; ++ptoff)
            {
                vtxoffs.setSize(0);

                // Make vertex order consistent, regardless of linked list order,
                // by sorting in vertex offset order
                bool hasOther = false;
                if (groupType == GA_GROUP_POINT)
                {
                    for (GA_Offset vtxoff = geo->pointVertex(ptoff); GAisValid(vtxoff); vtxoff = geo->vertexToNextVertex(vtxoff))
                        vtxoffs.append(vtxoff);
                }
                else
                {
                    for (GA_Offset vtxoff = geo->pointVertex(ptoff); GAisValid(vtxoff); vtxoff = geo->vertexToNextVertex(vtxoff))
                    {
                        GA_Offset group_offset = vtxoff;
                        if (groupType == GA_GROUP_PRIMITIVE)
                            group_offset = geo->vertexPrimitive(vtxoff);
                        if (!group || group->contains(group_offset))
                            vtxoffs.append(vtxoff);
                        else
                            hasOther = true;
                    }
                }

                // If there's only one vertex in the group and none outside, nothing to split.
                // If there are no vertices in the group, also nothing to split.
                // Unique all but first vertex if all vertices of the point are in vtxoffs.
                GA_Size nvtx = vtxoffs.size();
                GA_Size points_to_add = nvtx - GA_Size(!hasOther);
                if (points_to_add <= 0)
                    continue;

                numPointAdded += points_to_add;

                if (!ptwrangler)
                    ptwrangler.reset(new GA_PointWrangler(*geo, GA_AttributeFilter::selectPublic()));

                // TODO: This is not perfect; we should be sorting by primitive order
                //       and then vertex order within a primitive, so that results
                //       are the same whether the input was loaded from a file or
                //       computed.
                
#if SYS_VERSION_MAJOR_INT > 19 || ( SYS_VERSION_MAJOR_INT == 19 && SYS_VERSION_MINOR_INT == 5 )
                vtxoffs.sort(std::less<GA_Offset>());
#else
                vtxoffs.stdsort(std::less<GA_Offset>());
#endif
                GA_Offset newptoff = geo->appendPointBlock(points_to_add);
                for (exint j = static_cast<exint>(!hasOther); j < nvtx; ++j, ++newptoff)
                {
                    if (!bcnt++ && boss.wasInterrupted())
                        return;

                    ptwrangler->copyAttributeValues(newptoff, ptoff);
                    geo->setVertexPoint(vtxoffs(j), newptoff);
                }
            }
        }
    }

    template<typename T>
    bool compareVertices(
        const GA_Offset v1,
        const GA_Offset v2,
        const GA_ROHandleT<T>& attrib,
        const GA_Size tupleSize,
        const fpreal tol
    )
    {
        for (GA_Size i = 0; i < tupleSize; i++)
        {
            if (SYSabs(attrib.get(v1, i) - attrib.get(v2, i)) > tol)
                return false;
        }
        return true;
    }

    SYS_FORCE_INLINE void splitPointByAttrib(const GA_ElementGroup* const group)
    { splitAttrib = static_cast<const GA_Attribute*>(group); splitPointByAttrib(); }
    
    SYS_FORCE_INLINE void splitPointByAttrib(const GA_Group* const group)
    { if (group->isElementGroup()) splitPointByAttrib(static_cast<const GA_ElementGroup*>(group)); }
    
    void splitPointByAttrib()
    {
        switch (splitAttrib->getOwner())
        {
            case GA_ATTRIB_PRIMITIVE: splitPointByAttrib<GA_ATTRIB_PRIMITIVE>(); break;
            // case GA_ATTRIB_POINT:     splitPointByAttrib<GA_ATTRIB_POINT>();     break;
            case GA_ATTRIB_VERTEX:    splitPointByAttrib<GA_ATTRIB_VERTEX>();    break;
        }
    }

    
    template<GA_AttributeOwner _Owner>
    void splitPointByAttrib()
    {
        const GA_ElementGroup* const group = groupParser.isPointGroup() ? nullptr : groupParser.getPointGroup();
        // Point groups should be weeded out by the wrappers below.
        // UT_ASSERT(!group || group->classType() == GA_GROUP_PRIMITIVE || group->classType() == GA_GROUP_VERTEX);
        
        const bool isPrimGroup = groupParser.isPrimitiveGroup();
        
        
        const GA_AIFCompare* compare = nullptr;
        GA_ROHandleT<int64> attribi;
        GA_ROHandleR attribf;
        GA_Size tupleSize;
        const GA_ATINumeric* const numeric = dynamic_cast<const GA_ATINumeric*>(splitAttrib);
        if (numeric)
        {
            GA_StorageClass storeclass = splitAttrib->getStorageClass();
            if (storeclass == GA_STORECLASS_INT)
            {
                attribi = splitAttrib;
                UT_ASSERT(attribi.isValid());
                tupleSize = attribi.getTupleSize();
            }
            else if (storeclass == GA_STORECLASS_FLOAT)
            {
                attribf = splitAttrib;
                UT_ASSERT(attribf.isValid());
                tupleSize = attribf.getTupleSize();
            }
            else
            {
                UT_ASSERT_MSG(0, "Why does a GA_ATINumeric have a storage class other than int or float?");
                compare = splitAttrib->getAIFCompare();
            }
        }
        else
        {
            compare = splitAttrib->getAIFCompare();
            if (compare == NULL)
            {
                UT_ASSERT_MSG(0, "Missing an implementation of GA_AIFCompare!");
                return;
            }
        }

        UT_UniquePtr<GA_PointWrangler> ptwrangler(nullptr);

        UT_SmallArray<GA_Offset> vtxlist;
        
        const GA_Size initnumpts = geo->getNumPoints();

        const GA_Range range = groupParser.isPointGroup() ? groupParser.getPointRange() : geo->getPointRange();
        
        GA_Offset start, end;
        for (GA_Iterator it(range); it.fullBlockAdvance(start, end);)
        {
            for (GA_Offset ptoff = start; ptoff < end; ++ptoff)
            {
                if (geo->pointIndex(ptoff) >= initnumpts)
                    break;

                bool splitfound;
                do
                {
                    GA_Offset vtxoff = geo->pointVertex(ptoff);
                    if (!GAisValid(vtxoff))
                        break; // Point doesn't belong to anyone

                    // To avoid the results being dependent on the order of the
                    // linked-list topology attribute, find the highest vertex offset
                    // in the primitive of highest offset to be the base vertex.
                    // Lowest would make more sense, but highest is more compatible with
                    // the previous code.
                    GA_Offset primoff = geo->vertexPrimitive(vtxoff);
                    GA_Offset basevtxoff  = GA_INVALID_OFFSET;
                    GA_Offset baseprimoff = GA_INVALID_OFFSET;
                    if (!group || group->contains(isPrimGroup ? primoff : vtxoff))
                    {
                        basevtxoff = vtxoff;
                        baseprimoff = primoff;
                    }
                    vtxoff = geo->vertexToNextVertex(vtxoff);
                    while (GAisValid(vtxoff))
                    {
                        primoff = geo->vertexPrimitive(vtxoff);

                        if (!group || group->contains(isPrimGroup ? primoff : vtxoff))
                        {
                            if (primoff > baseprimoff || (primoff == baseprimoff && vtxoff > basevtxoff))
                            {
                                basevtxoff = vtxoff;
                                baseprimoff = primoff;
                            }
                        }
                        vtxoff = geo->vertexToNextVertex(vtxoff);
                    }

                    if (!GAisValid(basevtxoff))
                        break; // No vertices in group on current point

                    // We have a vertex on the point that's in the group.
                    // If any vertices mismatch, we make a new point.
                    
                    GA_Offset baseoffset;
                    if constexpr(_Owner == GA_ATTRIB_VERTEX)
                        baseoffset = basevtxoff;
                    else
                        baseoffset = baseprimoff;
                    
                    //GA_Offset baseoffset = _Owner == GA_ATTRIB_VERTEX ? basevtxoff : baseprimoff;
                    
                    vtxlist.setSize(1);
                    vtxlist(0) = basevtxoff;
                    splitfound = false;
                    for (vtxoff = geo->pointVertex(ptoff); GAisValid(vtxoff); vtxoff = geo->vertexToNextVertex(vtxoff))
                    {
                        if (vtxoff == basevtxoff)
                            continue;
                    
                        GA_Offset offset;
                        if constexpr(_Owner == GA_ATTRIB_VERTEX)
                            offset = vtxoff;
                        else
                            offset = geo->vertexPrimitive(vtxoff);
                        //GA_Offset offset = constexpr(_Owner == GA_ATTRIB_VERTEX) ? vtxoff : geo->vertexPrimitive(vtxoff);
                        
                        bool match;
                        if (attribi.isValid())
                            match = compareVertices<int64> (baseoffset, offset, attribi, tupleSize, splitAttribTol);
                        else if (attribf.isValid())
                            match = compareVertices<fpreal>(baseoffset, offset, attribf, tupleSize, splitAttribTol);
                        else
                        {
                            bool success = compare->isEqual(match, *splitAttrib, baseoffset, *splitAttrib, offset);
                            if (!success)
                                match = true;
                        }
                        if (match)
                        {
                            // Only append vertices in the group
                            if (!group || group->contains(isPrimGroup ? geo->vertexPrimitive(vtxoff) : vtxoff))
                            {
                                vtxlist.append(vtxoff);
                            }
                        }
                        else
                        {
                            // Split regardless of whether the unmatched vertex is in the group.
                            // This may or may not be the behaviour people expect,
                            // but it's ambiguous as to what people would expect.
                            // At least, if all vertices in the group have one value
                            // and all vertices out of the group have a different value,
                            // they probably want a new point, and that's accomplished
                            // with this condition.
                            splitfound = true;
                        }
                    }

                    if (splitfound)
                    {
                        // A split has been found! Create a new point
                        // using all the entries of vtxlist.
                        GA_Offset newptoff = geo->appendPoint();
                        ++numPointAdded;
                        if (!ptwrangler)
                            ptwrangler.reset(new GA_PointWrangler(*geo, GA_AttributeFilter::selectPublic()));
                        ptwrangler->copyAttributeValues(newptoff, ptoff);
                        for (exint i = 0; i < vtxlist.size(); i++)
                            geo->setVertexPoint(vtxlist(i), newptoff);
                    }
                } while (splitfound);
            }
        }
    }

public:
    const GA_Attribute* splitAttrib = nullptr;
    fpreal splitAttribTol = 1e-05;
    bool promoteAttrib = false;
    bool outEdgeGroupAsVertex = false;

//private:
//    bool splitByAttrib = false;

private:
    
    size_t numPointAdded;

    //exint subscribeRatio = 64;
    //exint minGrainSize = 64;

#undef __GFE_TEMP_SplitPoint_VertexEdgeGroupName
    
}; // End of class GFE_SplitPoint

#endif
