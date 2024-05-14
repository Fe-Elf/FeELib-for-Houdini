
#pragma once

#ifndef __GFE_SkinIntersectCurve_h__
#define __GFE_SkinIntersectCurve_h__

#include "GFE/GFE_SkinIntersectCurve.h"

#include "GFE/GFE_GeoFilter.h"



#include "GU/GU_IntersectionAnalysis.h"
#include "GU/GU_PrimMesh.h"


class GFE_SkinIntersectCurve : public GFE_AttribFilterWithRef {


public:
    using GFE_AttribFilterWithRef::GFE_AttribFilterWithRef;


    void
        setComputeParm(
            const fpreal tolerance = 1e-05,
            const exint subscribeRatio = 64,
            const exint minGrainSize = 64
        )
    {
        setHasComputed();
        this->tolerance      = tolerance;
        this->subscribeRatio = subscribeRatio;
        this->minGrainSize   = minGrainSize;
    }

    
private:

    virtual bool
        computeCore() override
    {
        if (groupParser.isEmpty())
            return true;
        

        GU_DetailHandle tmpGeoH0;
        GU_Detail* tmpGeo0 = new GU_Detail();
        tmpGeoH0.allocateAndSet(tmpGeo0);
        tmpGeo0->replaceWith(*geo);


        //UT_Array<UT_Int32Array>*    input_num_arr = nullptr;
        //UT_Array<UT_Int64Array>*    prim_num_arr  = nullptr;
        //UT_Array<UT_Fpreal32Array>* prim_uvw_arr  = nullptr;
        UT_Array<UT_Int32Array>    input_num_arr;
        UT_Array<UT_Int64Array>    prim_num_arr;
        UT_Array<UT_Fpreal32Array> prim_uvw_arr;
        UT_Array<UT_Int64Array>*   pt_num_arr     = nullptr;
        UT_Vector3DArray intersectionPoses = GU_IntersectionAnalysis::findIntersectionPoints(
            geo->asGEO_Detail(), nullptr,
            geoRef0->asGEO_Detail(), nullptr,
            threshold,
            //input_num_arr,
            //prim_num_arr,
            //prim_uvw_arr,
            &input_num_arr,
            &prim_num_arr,
            &prim_uvw_arr,
            pt_num_arr);

        for (exint i = intersectionPoses.size() - 1; i >= 0; i--)
        {
            float u = prim_uvw_arr[0][0];
            if (u < 1 && u > 0) continue;
            intersectionPoses.removeIndex(i);
            input_num_arr.removeIndex(i);
            prim_num_arr.removeIndex(i);
            prim_uvw_arr.removeIndex(i);
        }




        tmpGeoH0.deleteGdp();


        return true;
    }


    template<typename T>
    void skinIntersectCurve()
    {
        UT_ASSERT_P(attribPtr);
        const GA_SplittableRange geoSplittableRange0(groupParser.getRange(attribPtr->getOwner()));
        if(outAsOffset)
        {
            UTparallelFor(geoSplittableRange0, [this](const GA_SplittableRange& r)
            {
                GA_PageHandleT<T, T, true, true, GA_Attribute, GA_ATINumeric, GA_Detail> attrib_ph(attribPtr);
                for (GA_PageIterator pit = r.beginPages(); !pit.atEnd(); ++pit)
                {
                    GA_Offset start, end;
                    for (GA_Iterator it(pit.begin()); it.blockAdvance(start, end); )
                    {
                        attrib_ph.setPage(start);
                        for (GA_Offset elemoff = start; elemoff < end; ++elemoff)
                        {
                            attrib_ph.value(elemoff) = firstIndex + elemoff;
                        }
                    }
                }
            }, subscribeRatio, minGrainSize);
        }
        else
        {
            const GA_IndexMap& indexMap = attribPtr->getIndexMap();
            if (negativeIndex)
            {
                UTparallelFor(geoSplittableRange0, [this, &indexMap](const GA_SplittableRange& r)
                {
                    GA_PageHandleT<T, T, true, true, GA_Attribute, GA_ATINumeric, GA_Detail> attrib_ph(attribPtr);
                    for (GA_PageIterator pit = r.beginPages(); !pit.atEnd(); ++pit)
                    {
                        GA_Offset start, end;
                        for (GA_Iterator it(pit.begin()); it.blockAdvance(start, end); )
                        {
                            attrib_ph.setPage(start);
                            for (GA_Offset elemoff = start; elemoff < end; ++elemoff)
                            {
                                attrib_ph.value(elemoff) = firstIndex - indexMap.indexFromOffset(elemoff);
                            }
                        }
                    }
                }, subscribeRatio, minGrainSize);
            }
            else
            {
                UTparallelFor(geoSplittableRange0, [this, &indexMap](const GA_SplittableRange& r)
                {
                    GA_PageHandleT<T, T, true, true, GA_Attribute, GA_ATINumeric, GA_Detail> attrib_ph(attribPtr);
                    for (GA_PageIterator pit = r.beginPages(); !pit.atEnd(); ++pit)
                    {
                        GA_Offset start, end;
                        for (GA_Iterator it(pit.begin()); it.blockAdvance(start, end); )
                        {
                            attrib_ph.setPage(start);
                            for (GA_Offset elemoff = start; elemoff < end; ++elemoff)
                            {
                                attrib_ph.value(elemoff) = firstIndex + indexMap.indexFromOffset(elemoff);
                            }
                        }
                    }
                }, subscribeRatio, minGrainSize);
            }
        }
    }


public:
    
    fpreal tolerance = 1e-05;

private:
    
    GA_Attribute* attribPtr;
    exint subscribeRatio = 64;
    exint minGrainSize = 1024;


}; // End of class GFE_SkinIntersectCurve


















#endif
