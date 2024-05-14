
#pragma once

#ifndef __GFE_PolyReduce2D_h__
#define __GFE_PolyReduce2D_h__

#include "GFE/GFE_PolyReduce2D.h"

#include "GFE/GFE_GeoFilter.h"


#define GFE_PolyReduce2D_ReverseROC 1
#define GFE_PolyReduce2D_UseStd 1
#define GFE_PolyReduce2D_UseDetachedAttrib_ForRelease 1

#define GFE_PolyReduce2D_CoverSourcePoly 0




//#include "GEO/GEO_SplitPoints.h"
#include "GFE/GFE_InlinePoint.h"


//#include "GFE/GFE_Array.h"
#include "GFE/GFE_MeshTopology.h"
#include "GFE/GFE_Math.h"



class GFE_PolyReduce2D : public GFE_AttribFilter {

public:
    enum GeoPropertyType
    {
        ANGLE,
        DIST,
        ROC,
    };

    //using GFE_AttribFilter::GFE_AttribFilter;


    GFE_PolyReduce2D(
        GA_Detail& geo,
        const SOP_NodeVerb::CookParms* const cookparms = nullptr
    )
        : GFE_AttribFilter(geo, cookparms)
        , inlinePoint(geo, cookparms)
    {
        inlinePoint.getOutGroupArray().findOrCreate(GA_GROUP_POINT, true);
        inlinePoint.setComputeParm(1e-05, false, true);
    }

    ~GFE_PolyReduce2D()
    {
    }




    SYS_FORCE_INLINE void setThreshold_maxCosRadians(fpreal threshold_maxAngle)
    { threshold_maxCosRadians = threshold_maxAngle > 0 ? cos(GFE_Math::radians(threshold_maxAngle)) : 1.0; }

    void
    setComputeParm(
        const bool delInLinePoint = true,
        const fpreal threshold_inlineCosRadians = 1e-05,

        const bool limitByGeoProperty = true,
        const GFE_PolyReduce2D::GeoPropertyType geoPropertyType = GFE_PolyReduce2D::GeoPropertyType::ANGLE,
        fpreal threshold_maxCosRadians = cos(GFE_Math::radians(150.0)),
        const fpreal threshold_maxDist = 1e-04,

        const bool limitMinPoint = false,
        const exint minPoint = 10,

        const bool coverSourcePoly = false,
        const bool reverseGroup = false,
        const bool delPoint = false,

        const exint subscribeRatio = 64,
        const exint minGrainSize = 16
    )
    {
        setHasComputed();
        this->delInLinePoint = delInLinePoint;
        inlinePoint.threshold_inlineCosRadians = threshold_inlineCosRadians;

        this->limitByGeoProperty = limitByGeoProperty;
        this->geoPropertyType = geoPropertyType;
        this->threshold_maxCosRadians = threshold_maxCosRadians;
        this->threshold_maxDist = threshold_maxDist;

        this->limitMinPoint = limitMinPoint;
        this->minPoint = minPoint;

        this->coverSourcePoly = coverSourcePoly;
        this->reverseGroup = reverseGroup;
        this->delPoint = delPoint;

        this->subscribeRatio = subscribeRatio;
        this->minGrainSize = minGrainSize;
    }





    virtual void bumpDataId() override
    {
        if (delInLinePoint || coverSourcePoly || delPoint)
            GFE_AttribFilter::bumpDataIdsForAddOrRemove(true, true, true);
        else
            GFE_AttribFilter::bumpDataId();
    }


    SYS_FORCE_INLINE virtual void visualizeOutGroup() override
    {
        if (coverSourcePoly || delPoint)
            return;
        GFE_AttribFilter::visualizeOutGroup();
    }






private:

    virtual bool
        computeCore() override
    {
        if (getOutGroupArray().isEmpty())
            return false;

        if (groupParser.isEmpty())
            return true;

        if (delInLinePoint)
        {
            inlinePoint.groupParser.setGroup(groupParser);
            inlinePoint.getOutGroupArray().findOrCreate(true, GA_GROUP_POINT);
            inlinePoint.compute();
        }

        const size_t len = getOutGroupArray().size();
        for (size_t i = 0; i < len; i++)
        {
            if (getOutGroupArray()[i]->classType() != GA_GROUP_POINT)
            {
                UT_ASSERT_MSG(0, "not correct group type");
                continue;
            }
            polyReduce2DPtGroup = getOutGroupArray().getPointGroup(i);
            switch (geo->getPreferredPrecision())
            {
            default:
            case GA_PRECISION_64: polyReduce2D<fpreal64>(); break;
            case GA_PRECISION_32: polyReduce2D<fpreal32>(); break;
            }
        }
        return true;
    }




    //polyReduce2D(geo, groupName, reverseGroup, delGroup);
    template<typename FLOAT_T>
    void polyReduce2D()
    {
        UT_ASSERT_P(polyReduce2DPtGroup);

        //using value_type = typename VECTOR_T::value_type;

        GA_PrimitiveGroupUPtr fullReducePrimGroupUPtr = geo->createDetachedPrimitiveGroup();
        GA_PrimitiveGroup* const fullReducePrimGroup = fullReducePrimGroupUPtr.get();

        const GA_Precision finalPreferredPrecision = geo->getPreferredPrecision();
        const GA_Storage inStorageI = GFE_Type::getPreferredStorageI(finalPreferredPrecision);
        const GA_Storage inStorageF = GFE_Type::getPreferredStorageF(finalPreferredPrecision);

#if GFE_PolyReduce2D_UseDetachedAttrib_ForRelease
        const GA_AttributeUPtr weightUPtr = geo->getAttributes().createDetachedTupleAttribute(GA_ATTRIB_POINT, inStorageF, 1);
        const GA_AttributeUPtr ndirUPtr   = geo->getAttributes().createDetachedTupleAttribute(GA_ATTRIB_POINT, inStorageF, 3);
        GA_Attribute* const weightPtr = weightUPtr.get();
        GA_Attribute* const ndirPtr   = ndirUPtr.get();
#else
        GA_Attribute* const weightPtr = geo->getAttributes().createTupleAttribute(GA_ATTRIB_POINT, "weight", inStorageF, 1, GA_Defaults(0));
        GA_Attribute* const ndirPtr = geo->getAttributes().createTupleAttribute(GA_ATTRIB_POINT, "ndir", inStorageF, 3, GA_Defaults(0));
#endif

        const GA_RWHandleT<FLOAT_T> weight_h(weightPtr);
        const GA_RWHandleT<UT_Vector3T<FLOAT_T>> ndir_h(ndirPtr);

        
        GFE_MeshTopology meshTopology(geo, cookparms);
        meshTopology.groupParser.setGroup(groupParser);
        const GA_Attribute* const nebsAttribPtr = meshTopology.setPointPointEdge(false);
        meshTopology.compute();
        
        const GA_ROHandleT<UT_ValArray<GA_Offset>> nebsAttrib_h(nebsAttribPtr);

        const GA_ROHandleT<UT_Vector3T<FLOAT_T>> pos_h(geo->getP());


        UTparallelFor(groupParser.getPointSplittableRange(), [this,
            &pos_h, &ndir_h, &weight_h, &nebsAttrib_h](const GA_SplittableRange& r)
        {
            FLOAT_T weight;
            UT_Vector3T<FLOAT_T> pos, ndir, ndir1, posneb0, posneb1;
            UT_ValArray<GA_Offset> nebsArr;

            GA_Offset start, end;
            for (GA_Iterator it(r); it.blockAdvance(start, end); )
            {
                for (GA_Offset elemoff = start; elemoff < end; ++elemoff)
                {
                    pos = pos_h.get(elemoff);
                    nebsAttrib_h.get(elemoff, nebsArr);

                    // UT_ASSERT_MSG(nebsArr.size() <= 2, "not support neighbour count > 2");

                    switch (nebsArr.size())
                    {
                    case 0:
                        ndir = 0;
#if GFE_PolyReduce2D_ReverseROC
                        weight = geoPropertyType == GFE_PolyReduce2D::GeoPropertyType::ANGLE ? 2 : SYS_FP32_MAX;
#else
                        switch (geoPropertyType)
                        {
                        case GFE_PolyReduce2D::GeoPropertyType::ANGLE: weight = 2;            break;
                        case GFE_PolyReduce2D::GeoPropertyType::DIST:  weight = SYS_FP32_MAX; break;
                        case GFE_PolyReduce2D::GeoPropertyType::ROC:   weight = 0;            break;
                        default:                 weight = 2;            break;
                        }
#endif
                        break;
                    case 1:
                        ndir = pos_h.get(nebsArr[0]) - pos;
                        if (geoPropertyType == GFE_PolyReduce2D::GeoPropertyType::ANGLE) {
                            ndir.normalize();
                        }
#if GFE_PolyReduce2D_ReverseROC
                        weight = geoPropertyType == GFE_PolyReduce2D::GeoPropertyType::ANGLE ? 2 : SYS_FP32_MAX;
#else
                        switch (geoPropertyType)
                        {
                        case GFE_PolyReduce2D::GeoPropertyType::ANGLE: weight = 2;            break;
                        case GFE_PolyReduce2D::GeoPropertyType::DIST:  weight = SYS_FP32_MAX; break;
                        case GFE_PolyReduce2D::GeoPropertyType::ROC:   weight = 0;            break;
                        default:                 weight = 2;            break;
                        }
#endif
                        break;
                    case 2:
                        posneb0 = pos_h.get(nebsArr[0]);
                        posneb1 = pos_h.get(nebsArr[1]);
                        switch (geoPropertyType)
                        {
                        case GFE_PolyReduce2D::GeoPropertyType::ANGLE:
                            ndir = posneb1 - pos;
                            ndir.normalize();

                            ndir1 = posneb1 - pos;
                            ndir1.normalize();

                            weight = dot(ndir, ndir1);
                            break;
                        case GFE_PolyReduce2D::GeoPropertyType::DIST:
                            // weight = GFE_Math::distToLine(pos, posneb0, posneb1);
                            ndir = posneb0 - pos;
                            weight = GFE_Math::distToLine(ndir, posneb1 - pos, posneb0, posneb1);
                            break;
                        case GFE_PolyReduce2D::GeoPropertyType::ROC:
                            weight = GFE_Math::circleRadius3Point(posneb0, pos, posneb1);
                            ndir = 0;
                            // #if GFE_PolyReduce2D_ReverseROC
                            //     weight = 1 / weight;
                            // #endif
                            break;
                        default:
                            weight = 0;
                            ndir = 0;
                            UT_ASSERT_MSG(0, "not support geoPropertyType");
                            break;
                        }
                        break;
                    default:
                        weight = 0;
                        ndir = 0;
                        UT_ASSERT_MSG(0, "not support neighbour count > 2");
                        break;
                    }
                    weight_h.set(elemoff, weight);
                    ndir_h.set(elemoff, ndir);
                }
            }
        }, subscribeRatio, minGrainSize);



        const GA_Topology& topo = geo->getTopology();
        const GA_ATITopology& vtxPointRef = *topo.getPointRef();



        const GA_SplittableRange geoPrimSplittableRange(groupParser.getPrimitiveRange());
        UTparallelFor(geoPrimSplittableRange, [this, &vtxPointRef, fullReducePrimGroup,
            &pos_h, &ndir_h, &weight_h](const GA_SplittableRange& r)
        {
            GA_OffsetListRef vertices;
            //FLOAT_T weight;
            //UT_Vector3T<FLOAT_T> pos, ndir, ndir1, posneb0, posneb1;
            //UT_ValArray<GA_Offset> nebsArr;
            UT_Vector3T<FLOAT_T> ndir;

#if GFE_PolyReduce2D_UseStd
            std::vector<GA_Size> idx_prev, idx_next, scaleIdx_prev, scaleIdx_next;
            std::vector<int> argsort;
            std::vector<FLOAT_T> weights;
#else
            UT_ValArray<GA_Size> idx_prev, idx_next, scaleIdx_prev, scaleIdx_next;
            UT_ValArray<int> argsort;
            UT_ValArray<FLOAT_T> weights;
#endif
            UT_ValArray<UT_Vector3T<FLOAT_T>> poses, dirs;


            GA_Offset start, end;
            for (GA_Iterator it(r); it.blockAdvance(start, end); )
            {
                for (GA_Offset elemoff = start; elemoff < end; ++elemoff)
                {
                    idx_prev.clear();
                    idx_next.clear();
                    scaleIdx_prev.clear();
                    scaleIdx_next.clear();
                    poses.clear();
                    dirs.clear();
                    weights.clear();
                    argsort.clear();

                    vertices = geo->getPrimitiveVertexList(elemoff);
                    const GA_Size numvtx = vertices.size();
                    GA_Size lastIndex = numvtx - 1;

                    bool isClosed;
                    {
                        const GA_Offset primpoint0 = vtxPointRef.getLink(vertices[0]);
                        const GA_Offset primpoint1 = vtxPointRef.getLink(vertices[lastIndex]);
                        // isClosed = geo->getPrimitiveClosedFlag(elemoff) || primpoint0 == primpoint1;
                        if (primpoint0 == primpoint1)
                        {
                            --lastIndex;
                            isClosed = true;
                        }
                        else if (geo->getPrimitiveClosedFlag(elemoff))
                            isClosed = true;
                        else
                            isClosed = false;
                    }

                    GA_Size primPointsCount = lastIndex + 1;
                    const GA_Size lastIndex_prev = lastIndex - 1;



                    //////////////////// initial ////////////////////////

#if GFE_PolyReduce2D_UseStd
                    //if (idx_prev.capacity() < primPointsCount)
                    //{
                    //    idx_prev.setCapacity(primPointsCount);
                    //    idx_next.setCapacity(primPointsCount);
                    //    scaleIdx_prev.setCapacity(primPointsCount);
                    //    scaleIdx_next.setCapacity(primPointsCount);
                    //    poses.setCapacity(primPointsCount);
                    //    dirs.setCapacity(primPointsCount);
                    //    weights.setCapacity(primPointsCount);
                    //    argsort.setCapacity(primPointsCount);
                    //}
                    if (idx_prev.size() < primPointsCount)
                    {
                        idx_prev.resize(primPointsCount);
                        idx_next.resize(primPointsCount);
                        scaleIdx_prev.resize(primPointsCount);
                        scaleIdx_next.resize(primPointsCount);
                        poses.setSize(primPointsCount);
                        dirs.setSize(primPointsCount);
                        weights.resize(primPointsCount);
                        argsort.resize(primPointsCount);
                    }
#else
                    //if (idx_prev.capacity() < primPointsCount)
                    //{
                    //    idx_prev.setCapacity(primPointsCount);
                    //    idx_next.setCapacity(primPointsCount);
                    //    scaleIdx_prev.setCapacity(primPointsCount);
                    //    scaleIdx_next.setCapacity(primPointsCount);
                    //    poses.setCapacity(primPointsCount);
                    //    dirs.setCapacity(primPointsCount);
                    //    weights.setCapacity(primPointsCount);
                    //    argsort.setCapacity(primPointsCount);
                    //}
                    if (idx_prev.size() < primPointsCount)
                    {
                        idx_prev.setSize(primPointsCount);
                        idx_next.setSize(primPointsCount);
                        scaleIdx_prev.setSize(primPointsCount);
                        scaleIdx_next.setSize(primPointsCount);
                        poses.setSize(primPointsCount);
                        dirs.setSize(primPointsCount);
                        weights.setSize(primPointsCount);
                        argsort.setSize(primPointsCount);
                    }
#endif


                    for (GA_Size vtxpnum = 0; vtxpnum < primPointsCount; ++vtxpnum) {
                        GA_Offset primpoint = vtxPointRef->getLink(vertices[vtxpnum]);

                        poses[vtxpnum] = pos_h.get(primpoint);
                        dirs[vtxpnum] = ndir_h.get(primpoint);
                        weights[vtxpnum] = weight_h.get(primpoint);
                    }

                    if (geo->getPrimitiveClosedFlag(elemoff))
                    {
                        dirs[lastIndex] = poses[lastIndex_prev] - poses[lastIndex];
                        if (geoPropertyType == GFE_PolyReduce2D::GeoPropertyType::ANGLE)
                            dirs[lastIndex].normalize();
                    }
                    else if (isClosed)
                    {
                        dirs[0] = poses[lastIndex] - poses[0];
                        if (geoPropertyType == GFE_PolyReduce2D::GeoPropertyType::ANGLE)
                            dirs[0].normalize();
                    }
                    else
                        dirs[0] *= -1;


                    const GA_Size NONFoundINT = primPointsCount + 1;// the name can be replace by lastIndex (not value)


                    // cout(isClosed);
                    if (isClosed)
                    {
                        idx_prev[0] = lastIndex;
                        idx_next[lastIndex] = 0;
                    }
                    else
                    {
                        idx_prev[0] = NONFoundINT;
                        idx_next[lastIndex] = NONFoundINT;
                    }
                    idx_next[0] = 1;
                    idx_prev[lastIndex] = lastIndex_prev;



                    GA_Size minidx;
                    {
#if GFE_PolyReduce2D_UseStd
                        argsort = GFE_Array::argsort(weights);
#else
                        argsort = GFE_Array::argsort_UT_Array(weights);
#endif

                        minidx = argsort[lastIndex];
                        scaleIdx_prev[minidx] = argsort[lastIndex_prev];
                        scaleIdx_next[minidx] = NONFoundINT;

                        minidx = argsort[0];
                        scaleIdx_prev[minidx] = NONFoundINT;
                        scaleIdx_next[minidx] = argsort[1];

                        for (GA_Size i = 1, i0 = 0, i1 = 2; i < lastIndex; ++i, ++i0, ++i1)
                        {
                            idx_prev[i] = i0;
                            idx_next[i] = i1;

                            scaleIdx_prev[argsort[i]] = argsort[i0];
                            scaleIdx_next[argsort[i]] = argsort[i1];
                        }
                    }


                    //if (chi("../blastGroup") && primPointsCount < 2)
                    //{
                    //    removeprim(0, @primnum, 1);
                    //    continue;
                    //}


                    if (primPointsCount < 2)
                    {
                        fullReducePrimGroup->setElement(elemoff, true);
                        continue;
                    }



                    // GA_Size loopCount = 0;
                    // for (GA_Size i = 0; i < @Frame-1 && primPointsCount > 0; ++i) {
                    while (primPointsCount > 2)
                    {
                        // ++loopCount;

                        if (limitByGeoProperty)
                        {
                            bool flag = false;
                            //fpreal weight = weights[minidx];
                            switch (geoPropertyType)
                            {
                            case GFE_PolyReduce2D::GeoPropertyType::ANGLE:
                                if (weights[minidx] >= threshold_maxCosRadians)
                                    {
                                    flag = true;
                                    break;
                                }
                                break;
                            case GFE_PolyReduce2D::GeoPropertyType::DIST:
                                if (weights[minidx] >= threshold_maxDist)
                                {
                                    flag = true;
                                    break;
                                }
                                break;
                            case GFE_PolyReduce2D::GeoPropertyType::ROC:
#if GFE_PolyReduce2D_ReverseROC
                                if (weights[minidx] >= threshold_maxDist)
                                {
                                    flag = true;
                                    break;
                                }
#else
                                if (weights[minidx] <= threshold_maxDist)
                                {
                                    flag = true;
                                    break;
                                }
#endif
                            }
                            if (flag)
                                break;
                        }

                        if (limitMinPoint && primPointsCount <= minPoint)
                        {
                            break;
                        }

                        const GA_Size previdx = idx_prev[minidx];
                        const GA_Size nextidx = idx_next[minidx];
                        if (nextidx != NONFoundINT) {
                            // if ( i == @Frame-1 ) {
                            //     printf("\n %d %d", previdx, nextidx);
                            // }

                            switch (geoPropertyType)
                            {
                            case GFE_PolyReduce2D::GeoPropertyType::ANGLE:
                                dirs[nextidx] = poses[previdx] - poses[nextidx];
                                dirs[nextidx].normalize();
                                break;
                            case GFE_PolyReduce2D::GeoPropertyType::DIST:
                                dirs[nextidx] = poses[previdx] - poses[nextidx];
                                break;
                            case GFE_PolyReduce2D::GeoPropertyType::ROC:
                                break;
                            default:
                                dirs[nextidx] = poses[previdx] - poses[nextidx];
                                dirs[nextidx].normalize();
                                break;
                            }

                            const GA_Size next_nextidx = idx_next[nextidx];
                            if (next_nextidx != NONFoundINT)
                            {

                                FLOAT_T scale_min;
                                switch (geoPropertyType)
                                {
                                case GFE_PolyReduce2D::GeoPropertyType::ANGLE: scale_min = -dot(dirs[nextidx], dirs[next_nextidx]);                                                      break;
                                case GFE_PolyReduce2D::GeoPropertyType::DIST:  scale_min = GFE_Math::distToLine(dirs[nextidx], dirs[next_nextidx], poses[previdx], poses[next_nextidx]); break;
                                case GFE_PolyReduce2D::GeoPropertyType::ROC:   scale_min = GFE_Math::circleRadius3Point(poses[previdx], poses[nextidx], poses[next_nextidx]);            break;
                                default:                                      scale_min = -dot(dirs[nextidx], dirs[next_nextidx]);                                                      break;
                                }

                                weights[nextidx] = scale_min;

                                GA_Size prev = scaleIdx_prev[nextidx];
                                GA_Size next = scaleIdx_next[nextidx];
                                if (next != NONFoundINT) scaleIdx_prev[next] = prev;
                                if (prev != NONFoundINT) scaleIdx_next[prev] = next;

                                // GA_Size j = 0;
                                prev = minidx;
                                for (next = scaleIdx_next[prev]; weights[next] < scale_min; next = scaleIdx_next[prev])
                                {
                                    if (next == NONFoundINT) break;
                                    prev = next;
                                    // if ( j < prev ) ++j;
                                }
                                scaleIdx_next[nextidx] = next;
                                if (next != NONFoundINT) scaleIdx_prev[next] = nextidx;
                                scaleIdx_prev[nextidx] = prev;
                                scaleIdx_next[prev] = nextidx;
                            }
                        }

                        if (previdx != NONFoundINT)
                        {
                            const GA_Size next_nextidx = idx_prev[previdx];
                            if (next_nextidx != NONFoundINT)
                            {
                                FLOAT_T scale_min;

                                switch (geoPropertyType)
                                {
                                case GFE_PolyReduce2D::GeoPropertyType::ANGLE: scale_min = -dot(dirs[previdx], dirs[nextidx]);                                                         break;
                                case GFE_PolyReduce2D::GeoPropertyType::DIST:  scale_min = GFE_Math::distToLine(dirs[previdx], dirs[nextidx], poses[next_nextidx], poses[nextidx]); break;
                                case GFE_PolyReduce2D::GeoPropertyType::ROC:   scale_min = GFE_Math::circleRadius3Point(poses[next_nextidx], poses[previdx], poses[nextidx]);       break;
                                default:                 scale_min = -dot(dirs[previdx], dirs[nextidx]);                                                         break;
                                }

                                weights[previdx] = scale_min;

                                GA_Size prev = scaleIdx_prev[previdx];
                                GA_Size next = scaleIdx_next[previdx];
                                if (next != NONFoundINT) scaleIdx_prev[next] = prev;
                                if (prev != NONFoundINT) scaleIdx_next[prev] = next;

                                // GA_Size j = 0;
                                prev = minidx;
                                for (next = scaleIdx_next[prev]; weights[next] < scale_min; next = scaleIdx_next[prev])
                                {
                                    if (next == NONFoundINT) break;
                                    prev = next;
                                    // if ( j < prev ) ++j;
                                }
                                scaleIdx_next[previdx] = next;
                                if (next != NONFoundINT) scaleIdx_prev[next] = previdx;
                                scaleIdx_prev[previdx] = prev;
                                scaleIdx_next[prev] = previdx;
                            }
                        }
                        idx_prev[nextidx] = previdx;
                        idx_next[previdx] = nextidx;

                        --primPointsCount;

                        minidx = scaleIdx_next[minidx];
                    }


                    if (primPointsCount < 2)
                        fullReducePrimGroup->setElement(elemoff, true);

                    if (coverSourcePoly)
                    {
                        // GA_Size primvertices[];
                        // resize(primvertices, primPointsCount);
                        // resize(i[]@primpoints_out, primPointsCount);

                        for (GA_Size k = minidx; k != NONFoundINT; k = scaleIdx_next[k])
                        {
                            const GA_Offset primpoint = vtxPointRef->getLink(vertices[k]);
                            polyReduce2DPtGroup->setElement(primpoint, true);


                            switch (geoPropertyType)
                            {
                            case GFE_PolyReduce2D::GeoPropertyType::ANGLE:
                                ndir_h.set(primpoint, dirs[k]);
                                break;
                            case GFE_PolyReduce2D::GeoPropertyType::DIST:
                                ndir = dirs[k];
                                ndir.normalize();
                                ndir_h.set(primpoint, ndir);
                                break;
                            case GFE_PolyReduce2D::GeoPropertyType::ROC:
                                break;
                            default:
                                ndir_h.set(primpoint, dirs[k]);
                                break;
                            }
                            // weight_h.set(primpoint, weights[k]);

                            // i[]@primpoints_out[i] = primpoint;
                            // primvertices[i++] = k;
                        }
                        // GA_Size argsort[] = argsort(primvertices);
                        // i[]@primpoints_out = reorder(i[]@primpoints_out, argsort);
                    }
                    else
                    {
                        for (GA_Size k = minidx; k != NONFoundINT; k = scaleIdx_next[k])
                        {
                            const GA_Offset primpoint = vtxPointRef->getLink(vertices[k]);
                            polyReduce2DPtGroup->setElement(primpoint, true);
                            //ndir_h.set(primpoint, dirs[k]);
                            //weight_h.set(primpoint, weights[k]);
                        }
                    }

                } // end of for each primitive
            }
        }, subscribeRatio, minGrainSize);

        polyReduce2DPtGroup->invalidateGroupEntries();

        if (!coverSourcePoly)
        {
#if 0
            if (delPoint)
            {
                //polyReduce2DPtGroup->computeGroupEntries();
                geo->destroyPointOffsets(GA_Range(geo->getPointMap(), polyReduce2DPtGroup, !reverseGroup),
                    GA_Detail::GA_DestroyPointMode::GA_DESTROY_DEGENERATE_INCOMPATIBLE);
                if (reverseGroup)
                {
                    polyReduce2DPtGroup->toggleAll(geo->getNumPoints());
                }

                geo->destroyPointOffsets(GA_Range(geo->getPointMap(), polyReduce2DPtGroup),
                    GA_Detail::GA_DestroyPointMode::GA_DESTROY_DEGENERATE_INCOMPATIBLE);
            }
            else if (reverseGroup)
            {
                polyReduce2DPtGroup->toggleAll(geo->getNumPoints());
            }
#else
            //if (reverseGroup ^ delPoint)
            if (reverseGroup && !delPoint)
                polyReduce2DPtGroup->toggleAll(geo->getNumPoints());
        
            if (delPoint)
            {
                geo->destroyPointOffsets(GA_Range(geo->getPointMap(), polyReduce2DPtGroup, reverseGroup),
                    GA_Detail::GA_DestroyPointMode::GA_DESTROY_DEGENERATE_INCOMPATIBLE);
            }
#endif
            return;
        }

        ///////////////////////// Cover Source Poly /////////////////////////////////

#if GFE_PolyReduce2D_CoverSourcePoly



        geo->destroyPrimitiveOffsets(geo->getPrimitiveRange(fullReducePrimGroup), true);

        UTparallelFor(geoPrimSplittableRange, [this, &pos_h,
            polyReduce2DPtGroup](const GA_SplittableRange& r)
        {
            GA_OffsetListRef vertices;

            UT_Vector3T<FLOAT_T> pos, ndir, ndir1, posneb0, posneb1, up;
#if GFE_PolyReduce2D_UseStd
            std::vector<GA_Size> idx_prev, idx_next, dist_prev, dist_next;
#else
            UT_ValArray<GA_Size> idx_prev, idx_next, dist_prev, dist_next;
#endif

            GA_Offset start, end;
            for (GA_Iterator it(r); it.blockAdvance(start, end); )
            {
                for (GA_Offset primoff = start; primoff < end; ++primoff)
                {
                    idx_prev.clear();
                    idx_next.clear();
                    dist_prev.clear();
                    dist_next.clear();

                    up = static_cast<GEO_Primitive*>(geo->getPrimitive(primoff))->computeNormal();
                    //idx_prev.resize(primPointsCount);
                    //idx_next.resize(primPointsCount);
                    //dist_prev.resize(primPointsCount);
                    //dist_next.resize(primPointsCount);

                    vertices = geo->getPrimitiveVertexList(primoff);
                    const GA_Size numvtx = vertices.size();
                    const GA_Size lastIndex = numvtx - 1;

                    GA_Size vtxpnum;
                    GA_Size vtxpnum_last = -1;
                    GA_Offset primpoint_last;
                    for (vtxpnum = numvtx - 1; vtxpnum >= 0; --vtxpnum)
                    {
                        GA_Offset primpoint = vtxPointRef->getLink(vertices[vtxpnum]);
                        if (polyReduce2DPtGroup->containsOffset(primpoint))
                        {
                            vtxpnum_last = vtxpnum;
                            primpoint_last = primpoint;
                            break;
                        }
                    }
                    UT_ASSERT_MSG(primpoint_last != -1, "cant be possible");

                    GA_Size vtxpnum_prev = vtxpnum_last;
                    GA_Offset primpoint_prev = primpoint_last;
                    for (vtxpnum = 0; vtxpnum < numvtx; ++vtxpnum)
                    {
                        GA_Offset primpoint = vtxPointRef->getLink(vertices[vtxpnum]);
                        if (!polyReduce2DPtGroup->containsOffset(primpoint))
                            continue;

                        idx_prev.emplace_back(primpoint_prev);
                        idx_next.emplace_back();
                        vtxpnum_prev = vtxpnum;
                        primpoint_prev = primpoint;

                        pos = pos_h.get(elemoff);

                        int vtxpnum_next = vtxpnum;
                        while (1)
                        {
                            vtxpnum_next = vtxpnum_next == lastIndex ? 0 : vtxpnum_next + 1;
                            int primpoint = primpoint(0, @primnum, vtxpnum_next);
                            if (inpointgroup(0, chs("../tmpGroup_polyReduce2D"), primpoint)) break;
                            setpointattrib(0, chs("../tmpAttrib_lastid"), primpoint, @ptnum);
                        }

                        vtxpnum_next = vtxpnum;
                        while (1)
                        {
                            vtxpnum_next = vtxpnum_next == 0 ? lastIndex : vtxpnum_next - 1;
                            int primpoint = primpoint(0, @primnum, vtxpnum_next);
                            setpointattrib(0, chs("../tmpAttrib_nextid"), primpoint, @ptnum);
                            if (inpointgroup(0, chs("../tmpGroup_polyReduce2D"), primpoint)) break;
                        }
                    }

                    for (GA_Size vtxpnum = 0; vtxpnum < numvtx; ++vtxpnum)
                    {
                        GA_Offset primpoint = vtxPointRef->getLink(vertices[vtxpnum]);

                        if (!polyReduce2DPtGroup->containsOffset(primpoint))
                            continue;

                        vector ndir = point(0, chs("../tmpAttrib_ndir"), i@nextid);// must be normalized

                        vector pos = point(0, 'P', i@nextid);
                        // vector dir = normalize(@P - pos);
                        vector dir = @P - pos;

                        // if ( @ptnum == 109 ) {
                        //     printf('\n%f', dot(dir, ndir));
                        // }

                        vector outDir = cross(ndir, up);
                        if (dot(outDir, dir) < 0) return;

                        float dist2 = distance2(abs(dot(dir, ndir)) * ndir, dir);
                        setpointattrib(0, chs("../tmpAttrib_lastdist"), i@nextid, dist2, 'max');
                        setpointattrib(0, chs("../tmpAttrib_nextdist"), i@lastid, dist2, 'max');

                        //     float dist2 = distance2(dot(dir, ndir) * ndir, dir);
                        //     setpointattrib(0, chs("../tmpAttrib_lastdist"), i@nextid, dist2, 'max');
                        //     setpointattrib(0, chs("../tmpAttrib_nextdist"), i@lastid, dist2, 'max');
                        // }

                        dist_prev.emplace_back(0);
                        dist_next.emplace_back(0);
                    }

                    geo->destroyPointOffsets(GA_Range(geo->getPointMap(), polyReduce2DPtGroup, true),
                        GA_Detail::GA_DestroyPointMode::GA_DESTROY_DEGENERATE_INCOMPATIBLE);

                }
            }
        }, subscribeRatio, minGrainSize);

#endif

    }




public:
    GFE_InlinePoint inlinePoint;

    bool delInLinePoint = true;

    bool limitByGeoProperty = true;
    GFE_PolyReduce2D::GeoPropertyType geoPropertyType = GFE_PolyReduce2D::GeoPropertyType::ANGLE;
    fpreal threshold_maxCosRadians = cos(GFE_Math::radians(150.0));
    fpreal threshold_maxDist = 1e-04;

    bool limitMinPoint = false;
    exint minPoint = 10;

    bool coverSourcePoly = false;
    bool reverseGroup = false;
    bool delPoint = false;

private:

    GA_PointGroup* polyReduce2DPtGroup;
    
    exint subscribeRatio = 64;
    exint minGrainSize = 16;



#undef GFE_PolyReduce2D_ReverseROC
#undef GFE_PolyReduce2D_UseStd
#undef GFE_PolyReduce2D_UseDetachedAttrib_ForRelease

#undef GFE_PolyReduce2D_CoverSourcePoly


    
}; // End of class GFE_PolyReduce2D







#endif
