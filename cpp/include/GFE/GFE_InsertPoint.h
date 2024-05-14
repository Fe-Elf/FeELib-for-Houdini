
#pragma once

#ifndef __GFE_InsertPoint_h__
#define __GFE_InsertPoint_h__

#include "GFE/GFE_InsertPoint.h"

#include "GFE/GFE_GeoFilter.h"

#include "GFE/GFE_SetVectorComponent.h"

#define __TEMP_GFE_InsertPoint_RestPosCompName "__TEMP_GFE_InsertPoint_RestPosCompName"


/*
    GFE_InsertPoint insertPoint(geo, cookparms);
    insertPoint.findOrCreateTuple(true, GA_ATTRIB_POINT);
    insertPoint.compute();
*/
    
class GFE_InsertPoint : public GFE_GeoFilter {


public:
    using GFE_GeoFilter::GFE_GeoFilter;


    void
        setComputeParm(
            const fpreal threshold = 1e-05,
            const bool insert2D = false,
            const GFE_Axis axis = GFE_Axis::Y,
            
            const exint subscribeRatio = 64,
            const exint minGrainSize   = 1024
        )
    {
        setHasComputed();
        this->threshold = threshold;
        this->subscribeRatio = subscribeRatio;
        this->minGrainSize   = minGrainSize;
    }

    SYS_FORCE_INLINE void setInsert2D()
    { insert2D = false; }

    SYS_FORCE_INLINE void setInsert2D(const GFE_Axis a)
    { insert2D = true; axis = a; }

    
private:

    virtual bool
        computeCore() override
    {
        if (groupParser.isEmpty())
            return true;
        
        setValidPosAttrib();
        
        GFE_SetVectorComponent setVecComp(geo, nullptr, cookparms);
        if (insert2D)
        {
            setVecComp.comp = axis;
            setVecComp.getOutAttribArray().set(posAttribNonConst);
            setVecComp.setRestAttrib(__TEMP_GFE_InsertPoint_RestPosCompName);
            setVecComp.compute();
            UT_ASSERT_P(!setVecComp.getRestAttrib().isEmpty());
        }


        
        pointGroup = groupParser.classType() == GA_GROUP_PRIMITIVE ? nullptr : groupParser.getPointGroup();
        switch (posAttribNonConst->getAIFTuple()->getStorage(posAttribNonConst))
        {
        case GA_STORE_REAL16: insertPoint<fpreal16>();  break;
        case GA_STORE_REAL32: insertPoint<fpreal32>();  break;
        case GA_STORE_REAL64: insertPoint<fpreal64>();  break;
        default: UT_ASSERT_MSG(0, "Unhandled Storage"); break;
        }


        
        if (insert2D)
        {
            GA_Attribute* const restAttrib = geo->findPointAttrib(__TEMP_GFE_InsertPoint_RestPosCompName);
            UT_ASSERT_P(restAttrib);
            setVecComp.setRefAttrib(restAttrib);
            setVecComp.compute();
            geo->destroyAttrib(restAttrib);
        }
        
        return true;
    }


template<typename FLOAT_T>
void insertPoint()
{
    const GA_RWHandleT<UT_Vector3T<FLOAT_T>> pos_h(posAttribNonConst);
    UTparallelFor(groupParser.getPrimitiveSplittableRange(), [this, &pos_h](const GA_SplittableRange& r)
    {
        GA_Offset start, end;
        for (GA_Iterator it(r); it.blockAdvance(start, end); )
        {
            for (GA_Offset primoff = start; primoff < end; ++primoff)
            {
                const GA_OffsetListRef& vertices = geo->getPrimitiveVertexList(elemoff);
                const GA_Size numvtx = vertices.size();
                const GA_Size lastVtxpnum = numvtx-1;
                for (GA_Size vtxpnum = 0; vtxpnum < numvtx; ++vtxpnum)
                {
                    const GA_Offset ptoff = geo->vertexPoint(vertices[vtxpnum]);
                }
            }
        }
    }, subscribeRatio, minGrainSize);
}


public:
    fpreal threshold = 1e-05;
    
private:
    bool insert2D = false;
    GFE_Axis axis = GFE_Axis::Y;
    
private:
    
    const GA_PointGroup* pointGroup;
    
    exint subscribeRatio = 64;
    exint minGrainSize   = 1024;

#undef __TEMP_GFE_InsertPoint_RestPosCompName

}; // End of class GFE_InsertPoint







#endif
