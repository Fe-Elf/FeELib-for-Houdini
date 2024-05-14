
#pragma once

#ifndef __GFE_GroupAttribBoundary_h__
#define __GFE_GroupAttribBoundary_h__

#include "GFE_Math.h"
#include "GFE_MeshTopology.h"
#include "GFE/GFE_GroupAttributeBoundary.h"


#include "GFE/GFE_GeoFilter.h"



class GFE_GroupAttribBoundary : public GFE_AttribFilter {



public:
    using GFE_AttribFilter::GFE_AttribFilter;

    ~GFE_GroupAttribBoundary()
    {
    }


    void
        setComputeParm(
            const fpreal threshold = 1e-07,
            const bool includeUnhsaredEdge = true,
            const exint subscribeRatio = 64,
            const exint minGrainSize = 1024
        )
    {
        setHasComputed();
        this->threshold = threshold;
        this->includeUnhsaredEdge = includeUnhsaredEdge;
        this->subscribeRatio      = subscribeRatio;
        this->minGrainSize        = minGrainSize;
    }


private:

    virtual bool
        computeCore() override
    {
        if (getOutAttribArray().isEmpty() || getOutGroupArray().isEmpty())
            return false;

        
        outVtxGroup  = nullptr;
        outEdgeGroup = nullptr;
        const size_t sizeGroup = getOutGroupArray().size();
        for (size_t i = 0; i < sizeGroup; ++i)
        {
            switch (getOutGroupArray()[i]->classType())
            {
            case GA_GROUP_PRIMITIVE:
            case GA_GROUP_POINT:     continue; break;
            case GA_GROUP_VERTEX:    UT_ASSERT_MSG(0, "Already Has Vtx  Group"); outVtxGroup  = getOutGroupArray().getVertexGroup(i); break;
            case GA_GROUP_EDGE:      UT_ASSERT_MSG(0, "Already Has Edge Group"); outEdgeGroup = getOutGroupArray().getEdgeGroup(i);   break;
            default: UT_ASSERT_MSG(0, "Unhandled Group Type"); break;
            }
        }
        
        GA_VertexGroupUPtr vtxGroupUPtr;
        if (!outVtxGroup)
        {
            if (outEdgeGroup)
            {
                vtxGroupUPtr = geo->createDetachedVertexGroup();
                outVtxGroup = vtxGroupUPtr.get();
            }
            else
                return false;
        }
            
        if (groupParser.isEmpty())
            return true;

        groupSetter = outVtxGroup;
        
        const size_t sizeAttrib = getOutAttribArray().size();
        for (size_t i = 0; i < sizeAttrib; ++i)
        {
            attrib = getOutAttribArray()[i];
            owner = attrib->getOwner();
            
            switch (owner)
            {
            case GA_ATTRIB_PRIMITIVE:
            case GA_ATTRIB_VERTEX:              break;
            case GA_ATTRIB_POINT:
            case GA_ATTRIB_DETAIL:    continue; break;
            default: UT_ASSERT_MSG(0, "Unhandled Attrib Owner"); break;
            }
            
            const GA_AIFTuple* const aifTuple = attrib->getAIFTuple();
            if (!aifTuple)
                continue;
            
            const bool isPointAttrib = owner == GA_ATTRIB_POINT;
            
            switch (aifTuple->getTupleSize(attrib))
            {
            case 1:
                switch (aifTuple->getStorage(attrib))
                {
                case GA_STORE_INT16:  groupAttribBoundary<int16>();    break;
                case GA_STORE_INT32:  groupAttribBoundary<int32>();    break;
                case GA_STORE_INT64:  groupAttribBoundary<int64>();    break;
                case GA_STORE_REAL16: groupAttribBoundary<fpreal16>(); break;
                case GA_STORE_REAL32: groupAttribBoundary<fpreal32>(); break;
                case GA_STORE_REAL64: groupAttribBoundary<fpreal64>(); break;
                default: break;
                }
            case 2:
                switch (aifTuple->getStorage(attrib))
                {
                case GA_STORE_REAL16: groupAttribBoundary<UT_Vector2T<fpreal16>>(); break;
                case GA_STORE_REAL32: groupAttribBoundary<UT_Vector2T<fpreal32>>(); break;
                case GA_STORE_REAL64: groupAttribBoundary<UT_Vector2T<fpreal64>>(); break;
                default: break;
                }
            break;
            case 3:
                switch (aifTuple->getStorage(attrib))
                {
                case GA_STORE_REAL16: groupAttribBoundary<UT_Vector3T<fpreal16>>(); break;
                case GA_STORE_REAL32: groupAttribBoundary<UT_Vector3T<fpreal32>>(); break;
                case GA_STORE_REAL64: groupAttribBoundary<UT_Vector3T<fpreal64>>(); break;
                default: break;
                }
            break;
            case 4:
                switch (aifTuple->getStorage(attrib))
                {
                case GA_STORE_REAL16: groupAttribBoundary<UT_Vector4T<fpreal16>>(); break;
                case GA_STORE_REAL32: groupAttribBoundary<UT_Vector4T<fpreal32>>(); break;
                case GA_STORE_REAL64: groupAttribBoundary<UT_Vector4T<fpreal64>>(); break;
                default: break;
                }
            default: break;
            }

        }
        
        if (outEdgeGroup)
            GFE_GroupUnion::groupUnion(*outEdgeGroup, *outVtxGroup);

        return true;
    }


    template<typename T>
    void groupAttribBoundary()
    {
        
        UT_ASSERT_P(outVtxGroup);
        UT_ASSERT_P(owner == GA_ATTRIB_PRIMITIVE || owner == GA_ATTRIB_VERTEX);

        
        // if constexpr(::std::is_same_v<T, int8> ||
        //              ::std::is_same_v<T, int16> ||
        //              ::std::is_same_v<T, int32> ||
        //              ::std::is_same_v<T, int64> ||
        //              ::std::is_same_v<T, fpreal16> ||
        //              ::std::is_same_v<T, fpreal32> ||
        //              ::std::is_same_v<T, fpreal64> )
        // {
        //     using value_type = T;
        // }
        // else
        //     using value_type = typename T::value_type;
        // 
        // using value_type = constexpr(::std::is_same_v<T, int8> ||
        //              ::std::is_same_v<T, int16> ||
        //              ::std::is_same_v<T, int32> ||
        //              ::std::is_same_v<T, int64> ||
        //              ::std::is_same_v<T, fpreal16> ||
        //              ::std::is_same_v<T, fpreal32> ||
        //              ::std::is_same_v<T, fpreal64>)
        //     ? T
        //     : typename T::value_type;
        
        const fpreal threshold2 = threshold * threshold;
        
        GFE_MeshTopology meshTopology(geo, cookparms);
        meshTopology.groupParser.setGroup(groupParser);
        const GA_Attribute*   const vtxNextEquivAttrib = meshTopology.setVertexNextEquiv(true);
        const GA_VertexGroup* const vtxNextEquivNoLoopGroup = meshTopology.setVertexNextEquivNoLoopGroup(true);
        meshTopology.compute();

        const GA_ROHandleT<T> attrib_h(attrib);
        if (owner == GA_ATTRIB_PRIMITIVE)
        {
            const GA_ATITopology* const vtxPrimRef = geo->getTopology().getPrimitiveRef();
            UTparallelFor(geo->getVertexSplittableRange(vtxNextEquivNoLoopGroup), [this, &attrib_h, threshold2, vtxNextEquivAttrib, vtxPrimRef](const GA_SplittableRange& r)
            {
                T val0, val1;
                GA_PageHandleScalar<GA_Offset>::ROType nextEquiv_ph(vtxNextEquivAttrib);
                for (GA_PageIterator pit = r.beginPages(); !pit.atEnd(); ++pit)
                {
                    GA_Offset start, end;
                    for (GA_Iterator it(pit.begin()); it.blockAdvance(start, end); )
                    {
                        nextEquiv_ph.setPage(start);
                        for (GA_Offset elemoff = start; elemoff < end; ++elemoff)
                        {
                            if (GFE_Type::isInvalidOffset(nextEquiv_ph.value(elemoff)))
                            {
                                groupSetter.set(elemoff, includeUnhsaredEdge);
                                continue;
                            }
                                
                            const GA_Offset primoff0 = vtxPrimRef->getLink(elemoff);
                            const GA_Offset primoff1 = vtxPrimRef->getLink(nextEquiv_ph.value(elemoff));
                            val0 = attrib_h.get(primoff0);
                            val1 = attrib_h.get(primoff1);
                            groupSetter.set(elemoff, GFE_Math::tupleDistance2(val0, val1) < threshold2);
                        }
                    }
                }
            }, subscribeRatio, minGrainSize);
        }
        else
        {
            //const GA_ROHandleT<T> nextEquiv_h(vtxNextEquivAttrib);
            const GA_ATITopology* const nextRef = geo->getTopology().getVertexNextRef();
            const GA_ATITopology* const pointRef = geo->getTopology().getPointRef();
            UTparallelFor(geo->getVertexSplittableRange(vtxNextEquivNoLoopGroup), [this, &attrib_h, threshold2, vtxNextEquivAttrib, nextRef, pointRef](const GA_SplittableRange& r)
            {
                T val0, val1;
                GA_PageHandleScalar<GA_Offset>::ROType nextEquiv_ph(vtxNextEquivAttrib);
                for (GA_PageIterator pit = r.beginPages(); !pit.atEnd(); ++pit)
                {
                    GA_Offset start, end;
                    for (GA_Iterator it(pit.begin()); it.blockAdvance(start, end); )
                    {
                        nextEquiv_ph.setPage(start);
                        for (GA_Offset elemoff = start; elemoff < end; ++elemoff)
                        {
                            if (GFE_Type::isInvalidOffset(nextEquiv_ph.value(elemoff)))
                            {
                                groupSetter.set(elemoff, includeUnhsaredEdge);
                                continue;
                            }
                                
                            const GA_Offset vtxoff2 = nextEquiv_ph.value(elemoff);
                            const GA_Offset vtxoff3 = nextRef->getLink(vtxoff2);
                            
                            const GA_Offset ptoff0 = pointRef->getLink(elemoff);
                            const GA_Offset ptoff2 = pointRef->getLink(vtxoff2);
                            const GA_Offset ptoff3 = pointRef->getLink(vtxoff3);

                            bool flag = false;
                            
                            GA_Offset targetVtxoff = ptoff0 == ptoff2 ? vtxoff2 : (ptoff0 == ptoff3 ? vtxoff3 : GFE_INVALID_OFFSET);
                            if (GFE_Type::isValidOffset(targetVtxoff))
                            {
                                val0 = attrib_h.get(elemoff);
                                val1 = attrib_h.get(targetVtxoff);
                                flag = GFE_Math::tupleDistance2(val0, val1) < threshold2;
                            }
                            if (!flag)
                            {
                                const GA_Offset vtxoff1 = nextRef->getLink(elemoff);
                                const GA_Offset ptoff1 = pointRef->getLink(vtxoff1);
                                targetVtxoff = ptoff1 == ptoff2 ? vtxoff2 : (ptoff1 == ptoff3 ? vtxoff3 : GFE_INVALID_OFFSET);
                                if (GFE_Type::isValidOffset(targetVtxoff))
                                {
                                    val0 = attrib_h.get(vtxoff1);
                                    val1 = attrib_h.get(targetVtxoff);
                                    flag = GFE_Math::tupleDistance2(val0, val1) < threshold2;
                                }
                                else
                                {
                                    UT_ASSERT_MSG(0, "Impossible");
                                }
                            }
                            groupSetter.set(elemoff, flag);
                        }
                    }
                }
            }, subscribeRatio, minGrainSize);
        }
    }




    
public:
    fpreal threshold = 1e-07;
    bool includeUnhsaredEdge = true;
    
private:
    GA_Attribute* attrib;
    GA_AttributeOwner owner;
    GA_VertexGroup* outVtxGroup;
    GA_EdgeGroup*   outEdgeGroup;
    
    exint subscribeRatio = 64;
    exint minGrainSize = 64;


}; // End of Class GFE_GroupAttribBoundary









#endif
