
#pragma once

#ifndef __GFE_Measure_h__
#define __GFE_Measure_h__

#include <GFE/GFE_Measure.h>

#include <GFE/GeoFilter.h>

#include <GEO/GEO_Hull.h>

#include <GFE/Bound.h>

#if 0
        gfel::Measure measure(geo, cookparms);
        measure.measureType = measureType;
        measure.groupParser.setGroup(groupParser);
        measureAttrib = measure.findOrCreateTuple(false);
        measure.compute();
#endif

_GFEL_BEGIN
enum class MeasureType
{
    Perimeter,
    Area,
    Volume,
    MeshPerimeter,
    MeshArea,
    MeshVolume,
};

class Measure : public AttribFilter {

public:

    using AttribFilter::AttribFilter;

    void
        setComputeParm(
            const MeasureType measureType = MeasureType::Area,
            const exint subscribeRatio = 64,
            const exint minGrainSize   = 1024
        )
    {
        setHasComputed();
        this->measureType    = measureType;
        this->subscribeRatio = subscribeRatio;
        this->minGrainSize   = minGrainSize;
    }

    GA_Attribute* findOrCreateTuple(
        const bool detached = false,
        const GA_Storage storage = GA_STORE_INVALID,
        const UT_StringRef& attribName = ""
    )
    {
        //getOutAttribArray().clear();
        const GA_AttributeOwner owner = measureType < MeasureType::MeshPerimeter ? GA_ATTRIB_PRIMITIVE : GA_ATTRIB_DETAIL;
        if (!detached && (!attribName.isstring() || attribName.length()==0) )
        {
            const char* measureName;
            switch (measureType)
            {
            case MeasureType::Perimeter:     measureName = "__topo_perimeter";     break;
            case MeasureType::Area:          measureName = "__topo_area";          break;
            case MeasureType::Volume:        measureName = "__topo_volume";        break;
            case MeasureType::MeshPerimeter: measureName = "__topo_meshPerimeter"; break;
            case MeasureType::MeshArea:      measureName = "__topo_meshArea";      break;
            case MeasureType::MeshVolume:    measureName = "__topo_meshVolume";    break;
            default: break;
            }
            return getOutAttribArray().findOrCreateTuple(false,    owner, GA_STORECLASS_FLOAT, storage, measureName);
        }
        else
        {
            return getOutAttribArray().findOrCreateTuple(detached, owner, GA_STORECLASS_FLOAT, storage, attribName);
        }
    }



    
public:

#define GFE_MEASUREMESH_FUNC_SPECIALIZATION(FUNC_NAME);                       \
        fpreal64 FUNC_NAME()                                                  \
        {                                                                     \
            setValidPosAttrib();                                              \
            switch (posAttrib->getAIFTuple()->getStorage(posAttrib))          \
            {                                                                 \
                case GA_STORE_REAL32: return FUNC_NAME<fpreal32>();  break;   \
                case GA_STORE_REAL64: return FUNC_NAME<fpreal64>();  break;   \
                default: UT_ASSERT_MSG(0, "unhandled storage type"); break;   \
            }                                                                 \
            return -1;                                                        \
        }                                                                     \

    
GFE_MEASUREMESH_FUNC_SPECIALIZATION(computeMeshPerimeter)
GFE_MEASUREMESH_FUNC_SPECIALIZATION(computeMeshArea)
GFE_MEASUREMESH_FUNC_SPECIALIZATION(computeMeshVolume)
    
#undef GFE_MEASUREMESH_FUNC_SPECIALIZATION

    
    template<typename _Ty>
    _Ty computeMeshPerimeter()
    {
        ComputePerimeter<_Ty> body(geo, posAttrib);
        UTparallelReduce(groupParser.getPrimitiveSplittableRange(), body, subscribeRatio, minGrainSize);
        return body.getSum();
    }

    template<typename _Ty>
    _Ty computeMeshArea()
    {
        ComputeArea<_Ty> body(geo, posAttrib);
        UTparallelReduce(groupParser.getPrimitiveSplittableRange(), body, subscribeRatio, minGrainSize);
        return body.getSum();
    }

    template<typename _Ty>
    _Ty computeMeshVolume()
    {
        const UT_BoundingBoxT<_Ty>& geoBBox = geo->stdBoundingBox<_Ty>(geo->getPointRange(), posAttrib);
        const UT_Vector3T<_Ty>& bboxCenter = geoBBox.center();
        ComputeVolume<_Ty> body(geo, bboxCenter);
        UTparallelReduce(groupParser.getPrimitiveSplittableRange(), body, subscribeRatio, minGrainSize);
        return body.getSum();
    }





    
private:

    virtual bool
        computeCore() override
    {
        if (getOutAttribArray().isEmpty())
            return false;

        if (groupParser.isEmpty())
            return true;

        setValidPosAttrib();

        measureAttrib = getOutAttribArray().last();
#if 1
        switch (measureType)
        {
        case MeasureType::Perimeter:     computePerimeter();           break;
        case MeasureType::Area:          computeArea();                break;
        case MeasureType::Volume:        computeVolume();              break;
        case MeasureType::MeshPerimeter: computeMeshPerimeterAttrib(); break;
        case MeasureType::MeshArea:      computeMeshAreaAttrib();      break;
        case MeasureType::MeshVolume:    computeMeshVolumeAttrib();    break;
        }
#else
        switch (measureType)
        {
        case MeasureType::Perimeter:
            computePerimeter(geo, measureAttrib, posAttrib, primGroup, subscribeRatio, minGrainSize);
            break;
        case MeasureType::Area:
            computeArea(geo, measureAttrib, posAttrib, primGroup, subscribeRatio, minGrainSize);
            break;
        case MeasureType::Volume:
            computeMeshVolume(geo, posAttrib, primGroup, subscribeRatio, minGrainSize);
            break;
        case MeasureType::MeshPerimeter:
            computeMeshVolume(geo, posAttrib, primGroup, subscribeRatio, minGrainSize);
            break;
        case MeasureType::MeshArea:
            computeMeshVolume(geo, posAttrib, primGroup, subscribeRatio, minGrainSize);
            break;
        case MeasureType::MeshVolume:
            computeMeshVolume(geo, posAttrib, primGroup, subscribeRatio, minGrainSize);
            break;
        default:
            break;
        }
#endif
        return true;
    }















private:



    //GU_Detail::compute3DArea(const GA_Offset primoff)

    template<typename _Ty>
    _Ty computePerimeter(const GA_ROHandleT<UT_Vector3T<_Ty>>& pos_h, const GA_Offset primoff)
    {
        //const GA_OffsetListRef& vertices = geo->getPrimitiveVertexList(primoff);
        //const GA_Size numvtx = vertices.size();
        const GA_Size numvtx = geo->getPrimitiveVertexCount(primoff);
        if (numvtx < 2)
            return 0.0;

        //const bool closed = vertices.getExtraFlag();
        const bool closed = geo->getPrimitiveClosedFlag(primoff);
        _Ty pSum = 0.0;

        switch (geo->getPrimitiveTypeId(primoff))
        {
        case GA_PRIMPOLY:
            {
            }
            break;
        default:
            return 0.0;
        }
        
        UT_Vector3T<_Ty> pos0, pos1, dir;
        switch (pos_h->getOwner())
        {
        case GA_ATTRIB_VERTEX:
            pos0 = pos_h.get(geo->primVertex(primoff, closed ? numvtx-1 : 0));
            for (GA_Size vtxpnum = !closed; vtxpnum < numvtx; ++vtxpnum)
            {
                pos1 = pos_h.get(geo->getPrimitiveVertexOffset(primoff, vtxpnum));
                pos0 = pos1 - pos0;
                pSum += dir.length();
                pos0 = pos1;
            }
            break;
        case GA_ATTRIB_POINT:
            pos0 = pos_h.get(geo->primPoint(primoff, closed ? numvtx-1 : 0));
            for (GA_Size vtxpnum = !closed; vtxpnum < numvtx; ++vtxpnum)
            {
                pos1 = pos_h.get(geo->primPoint(primoff, vtxpnum));
                pos0 = pos1 - pos0;
                pSum += pos0.length();
                pos0 = pos1;
            }
            break;
        default: return 0.0;
        }

        return pSum;
    }

    

    template<typename _Ty>
    _Ty computeArea(const GA_ROHandleT<UT_Vector3T<_Ty>>& pos_h, const GA_Offset primoff)
    {
        //const GA_OffsetListRef& vertices = geo->getPrimitiveVertexList(primoff);

        const bool closed = geo->getPrimitiveClosedFlag(primoff);
        if (!closed)
            return 0.0;

        //const GA_Size numvtx = vertices.size();
        const GA_Size numvtx = geo->getPrimitiveVertexCount(primoff);
        if (numvtx < 3)
            return 0.0;


        switch (geo->getPrimitiveTypeId(primoff))
        {
        case GA_PRIMPOLY:
        {
        }
        break;
        default:
            return 0.0;
        }

        const GA_AttributeOwner attribOwner = pos_h.getAttribute()->getOwner();

        UT_Vector3T<_Ty> pos0, pos1, pos2;

        switch (attribOwner)
        {
        case GA_ATTRIB_VERTEX:
            pos0 = pos_h.get(geo->primVertex(primoff, 0));
            pos1 = pos_h.get(geo->primVertex(primoff, 1));
            pos2 = pos_h.get(geo->primVertex(primoff, 2));
        break;
        case GA_ATTRIB_POINT:
            pos0 = pos_h.get(geo->primPoint(primoff, 0));
            pos1 = pos_h.get(geo->primPoint(primoff, 1));
            pos2 = pos_h.get(geo->primPoint(primoff, 2));
        break;
        default:
            return 0.0;
        }
        //UT_Vector3T<_Ty>& pos1 = geo->getPos3T<_Ty>(geo->vertexPoint(vertices[0]));
        //UT_Vector3T<_Ty>& pos1 = geo->getPos3T<_Ty>(geo->vertexPoint(vertices[1]));
        //UT_Vector3T<_Ty>& pos2 = geo->getPos3T<_Ty>(geo->vertexPoint(vertices[2]));

        //GA_Offset ptoff0 = geo->vertexPoint(geo->getPrimitiveVertexOffset(primoff, 0));

        switch (numvtx)
        {
        case 3: return heronsFormula(pos0, pos1, pos2); break;
        case 4:
        {
            //UT_Vector3T<_Ty> pos3 = geo->getPos3T<_Ty>(geo->vertexPoint(vertices[3]));
            UT_Vector3T<_Ty> pos3;
            switch (attribOwner)
            {
            case GA_ATTRIB_VERTEX: pos3 = pos_h.get(geo->primVertex(primoff, 3)); break;
            case GA_ATTRIB_POINT:  pos3 = pos_h.get(geo->primPoint(primoff, 3));  break;
            default: return 0.0;
            }
            return bretschneidersFormula0(pos0, pos1, pos2, pos3);
        }
        break;
        default: UT_ASSERT_MSG(0, "unhandled numvtx"); break;
        }

        const UT_Vector3T<_Ty>& dir0 = pos1 - pos0;
        const UT_Vector3T<_Ty>& dir1 = pos2 - pos1;
        const UT_Vector3T<_Ty>& crossdir0B = cross(dir0, dir1);
        _Ty lengthdir0B = crossdir0B.length();
        lengthdir0B = 1.0 / lengthdir0B;
        const _Ty cosnx = ((pos1[1] - pos0[1]) * (pos2[2] - pos0[2]) - (pos2[1] - pos0[1]) * (pos1[2] - pos0[2])) * lengthdir0B;
        const _Ty cosny = ((pos2[0] - pos0[0]) * (pos1[2] - pos0[2]) - (pos1[0] - pos0[0]) * (pos2[2] - pos0[2])) * lengthdir0B;
        const _Ty cosnz = ((pos1[0] - pos0[0]) * (pos2[1] - pos0[1]) - (pos2[0] - pos0[0]) * (pos1[1] - pos0[1])) * lengthdir0B;

        _Ty areaSum = 0.0;
        //pos0 = geo->getPos3T<_Ty>(geo->vertexPoint(vertices[numvtx - 1]));
        switch (attribOwner)
        {
        case GA_ATTRIB_VERTEX:
            pos0 = pos_h.get(geo->primVertex(numvtx-1));
            for (GA_Size vtxpnum = 0; vtxpnum < numvtx; ++vtxpnum)
            {
                pos1 = pos_h.get(geo->primVertex(vtxpnum));
                areaSum += cosnz * (pos0[0] * pos1[1] - pos1[0] * pos0[1])
                         + cosnx * (pos0[1] * pos1[2] - pos1[1] * pos0[2])
                         + cosny * (pos0[2] * pos1[0] - pos1[2] * pos0[0]);
                pos0 = pos1;
            }
        break;
        case GA_ATTRIB_POINT:
            pos0 = pos_h.get(geo->primPoint(primoff, numvtx-1));
            for (GA_Size vtxpnum = 0; vtxpnum < numvtx; ++vtxpnum)
            {
                pos1 = pos_h.get(geo->primPoint(primoff, vtxpnum));
                areaSum += cosnz * (pos0[0] * pos1[1] - pos1[0] * pos0[1])
                         + cosnx * (pos0[1] * pos1[2] - pos1[1] * pos0[2])
                         + cosny * (pos0[2] * pos1[0] - pos1[2] * pos0[0]);
                pos0 = pos1;
            }
        break;
        default: return 0.0;
        }

        return abs(0.5 * areaSum);
    }



    
    // template<typename _Ty>
    // _Ty computeVolume(const UT_Vector3T<_Ty>& bboxCenter, const GA_Offset primoff)
    // {
    //     const GEO_Hull* const prim = static_cast<const GEO_Hull*>(geo->getPrimitive(primoff));
    //     return prim->calcVolume(bboxCenter);
    // }


#define GFE_MEASUREPRIM_FUNC_SPECIALIZATION(FUNC_NAME);                                                                              \
        template<typename _Ty, typename POS_FLOAT_T>                                                                             \
        void FUNC_NAME()                                                                                                             \
        {                                                                                                                            \
            const GA_ROHandleT<UT_Vector3T<POS_FLOAT_T>> pos_h(posAttrib);                                                           \
            UTparallelFor(groupParser.getPrimitiveSplittableRange(), [this, &pos_h](const GA_SplittableRange& r)                     \
            {                                                                                                                        \
                GA_PageHandleT<_Ty, _Ty, true, true, GA_Attribute, GA_ATINumeric, GA_Detail> measure_ph(measureAttrib);      \
                GA_Offset start, end;                                                                                                \
                for (GA_PageIterator pit = r.beginPages(); !pit.atEnd(); ++pit)                                                      \
                {                                                                                                                    \
                    for (GA_Iterator it(pit.begin()); it.blockAdvance(start, end); )                                                 \
                    {                                                                                                                \
                        measure_ph.setPage(start);                                                                                   \
                        for (GA_Offset elemoff = start; elemoff < end; ++elemoff)                                                    \
                        {                                                                                                            \
                            measure_ph.value(elemoff) = FUNC_NAME<POS_FLOAT_T>(pos_h, elemoff);                                      \
                        }                                                                                                            \
                    }                                                                                                                \
                }                                                                                                                    \
            }, subscribeRatio, minGrainSize);                                                                                        \
        }                                                                                                                            \


GFE_MEASUREPRIM_FUNC_SPECIALIZATION(computePerimeter)
GFE_MEASUREPRIM_FUNC_SPECIALIZATION(computeArea)
//GFE_MEASUREPRIM_FUNC_SPECIALIZATION(computeVolume)
    
#undef GFE_MEASUREPRIM_FUNC_SPECIALIZATION

    
template<typename _Ty>
void computeVolume()
{
    const UT_BoundingBoxT<_Ty>& geoBBox = geo->stdBoundingBox<_Ty>(geo->getPointRange(), posAttrib);
    const UT_Vector3T<fpreal32> bboxCenter(geoBBox.center());
    UTparallelFor(groupParser.getPrimitiveSplittableRange(), [this, &bboxCenter](const GA_SplittableRange& r)
    {
        GA_PageHandleT<_Ty, _Ty, true, true, GA_Attribute, GA_ATINumeric, GA_Detail> measure_ph(measureAttrib);
        GA_Offset start, end;
        for (GA_PageIterator pit = r.beginPages(); !pit.atEnd(); ++pit)
        {
            for (GA_Iterator it(pit.begin()); it.blockAdvance(start, end); )
            {
                measure_ph.setPage(start);
                for (GA_Offset elemoff = start; elemoff < end; ++elemoff)
                {
                    const GEO_Hull* const prim = static_cast<const GEO_Hull*>(geo->getPrimitive(elemoff));
                    measure_ph.value(elemoff) = prim->calcVolume(bboxCenter);
                    //measure_ph.value(elemoff) = computeVolume<_Ty>(elemoff);
                }
            }
        }
    }, subscribeRatio, minGrainSize);
}




    

#define GFE_MEASUREPRIM_FUNC_SPECIALIZATION(FUNC_NAME);                                       \
        void FUNC_NAME()                                                                      \
        {                                                                                     \
            const GA_Storage posStorage = posAttrib->getAIFTuple()->getStorage(posAttrib);    \
            switch (measureAttrib->getAIFTuple()->getStorage(measureAttrib))                  \
            {                                                                                 \
            case GA_STORE_REAL32:                                                             \
                switch (posStorage)                                                           \
                {                                                                             \
                case GA_STORE_REAL32: FUNC_NAME<fpreal32, fpreal32>(); break;                 \
                case GA_STORE_REAL64: FUNC_NAME<fpreal32, fpreal64>(); break;                 \
                default: UT_ASSERT_MSG(0, "unhandled storage type");   break;                 \
                }                                                                             \
                break;                                                                        \
            case GA_STORE_REAL64:                                                             \
                switch (posStorage)                                                           \
                {                                                                             \
                case GA_STORE_REAL32: FUNC_NAME<fpreal64, fpreal32>(); break;                 \
                case GA_STORE_REAL64: FUNC_NAME<fpreal64, fpreal64>(); break;                 \
                default: UT_ASSERT_MSG(0, "unhandled storage type");   break;                 \
                }                                                                             \
                break;                                                                        \
            default: UT_ASSERT_MSG(0, "unhandled storage type"); break;                       \
            }                                                                                 \
        }                                                                                     \
    
GFE_MEASUREPRIM_FUNC_SPECIALIZATION(computePerimeter)
GFE_MEASUREPRIM_FUNC_SPECIALIZATION(computeArea)
//GFE_MEASUREPRIM_FUNC_SPECIALIZATION(computeVolume)
    
#undef GFE_MEASUREPRIM_FUNC_SPECIALIZATION

    void computeVolume()
    {
        switch (measureAttrib->getAIFTuple()->getStorage(measureAttrib))
        {
        case GA_STORE_REAL32: computeVolume<fpreal32>();     break;
        case GA_STORE_REAL64: computeVolume<fpreal64>();     break;
        default: UT_ASSERT_MSG(0, "unhandled storage type"); break;
        }
    }





    

// #define GFE_MEASUREMESH_FUNC_SPECIALIZATION(FUNC_NAME);             \
//         template<typename _Ty>                                  \
//         void FUNC_NAME()                                            \
//         {                                                           \
//             const GA_RWHandleT<_Ty> measure_h(measureAttrib);   \
//             measure_h.set(0, FUNC_NAME());                          \
//         }                                                           \
//
//     
// GFE_MEASUREMESH_FUNC_SPECIALIZATION(computeMeshPerimeter)
// GFE_MEASUREMESH_FUNC_SPECIALIZATION(computeMeshArea)
// GFE_MEASUREMESH_FUNC_SPECIALIZATION(computeMeshVolume)
//     
// #undef GFE_MEASUREMESH_FUNC_SPECIALIZATION

    

    
#define GFE_MEASUREMESH_FUNC_SPECIALIZATION(FUNC_NAME, FUNC_SUB_NAME);                                        \
        void FUNC_NAME()                                                                                      \
        {                                                                                                     \
            switch (measureAttrib->getAIFTuple()->getStorage(measureAttrib))                                  \
            {                                                                                                 \
            case GA_STORE_REAL32:                                                                             \
		        measureAttrib->getAIFTuple()->set(measureAttrib, 0, static_cast<fpreal32>(FUNC_SUB_NAME()));  \
                break;                                                                                        \
            case GA_STORE_REAL64:                                                                             \
		        measureAttrib->getAIFTuple()->set(measureAttrib, 0, static_cast<fpreal64>(FUNC_SUB_NAME()));  \
                break;                                                                                        \
            default: UT_ASSERT_MSG(0, "unhandled storage type"); break;                                       \
            }                                                                                                 \
        }                                                                                                     \

    
GFE_MEASUREMESH_FUNC_SPECIALIZATION(computeMeshPerimeterAttrib, computeMeshPerimeter)
GFE_MEASUREMESH_FUNC_SPECIALIZATION(computeMeshAreaAttrib,      computeMeshArea)
GFE_MEASUREMESH_FUNC_SPECIALIZATION(computeMeshVolumeAttrib,    computeMeshVolume)
    
#undef GFE_MEASUREMESH_FUNC_SPECIALIZATION








    
    




public:


#if 0


#define computePerimeterMacro(T)                                                                                     \
static fpreal##T                                                                                                     \
computePerimeter##T(                                                                                                 \
    const GA_Detail* const geo,                                                                                      \
    const GA_Offset primoff                                                                                          \
)                                                                                                                    \
{                                                                                                                    \
    const GA_OffsetListRef& vertices = geo->getPrimitiveVertexList(primoff);                                         \
    const GA_Size numvtx = vertices.size();                                                                          \
    if (numvtx < 2)                                                                                                  \
        return 0.0;                                                                                                  \
                                                                                                                     \
    const bool closed = vertices.getExtraFlag();                                                                     \
    fpreal##T pSum = 0.0;                                                                                            \
                                                                                                                     \
    switch (geo->getPrimitiveTypeId(primoff))                                                                        \
    {                                                                                                                \
    case GA_PRIMPOLY:                                                                                                \
    {                                                                                                                \
    }                                                                                                                \
    break;                                                                                                           \
    default:                                                                                                         \
        return 0.0;                                                                                                  \
    }                                                                                                                \
                                                                                                                     \
    UT_Vector3T<fpreal##T> pos0 = geo->getPos3T<fpreal##T>(geo->vertexPoint(vertices[closed ? numvtx-1 : 0]));       \
    for (GA_Size i(!closed); i < numvtx; ++i)                                                                        \
    {                                                                                                                \
        UT_Vector3T<fpreal##T> pos1 = geo->getPos3T<fpreal##T>(geo->vertexPoint(vertices[i]));                       \
        pSum += (pos1 - pos0).length();                                                                              \
        pos0 = pos1;                                                                                                 \
    }                                                                                                                \
                                                                                                                     \
    return pSum;                                                                                                     \
}                                                                                                                    \

    computePerimeterMacro(32);
    computePerimeterMacro(64);



    template<typename T>
    static T computePerimeter(const GA_Detail* const geo, const GA_Offset primoff)
    {
        if constexpr (std::is_same<T, float>::value)
            return computePerimeter32(geo, primoff);
        else
            return computePerimeter64(geo, primoff);
    }

#else

    template<typename T>
    static T computePerimeter(const GA_Detail* const geo, const GA_Offset primoff)
    {
        const GA_OffsetListRef& vertices = geo->getPrimitiveVertexList(primoff);
        const GA_Size numvtx = vertices.size();
        if (numvtx < 2)
            return 0.0;

        const bool closed = vertices.getExtraFlag();
        T pSum = 0.0;

        switch (geo->getPrimitiveTypeId(primoff))
        {
        case GA_PRIMPOLY:
        {
        }
        break;
        default:
            return 0.0;
        }
        
#if SYS_VERSION_MAJOR_INT > 19 || ( SYS_VERSION_MAJOR_INT == 19 && SYS_VERSION_MINOR_INT == 5 )
        UT_Vector3T<T> pos0 = geo->getPos3T<T>(geo->vertexPoint(vertices[closed ? numvtx - 1 : 0]));
#else
        //UT_Vector3T<T> pos0 = static_cast<const GFE_Detail*>(geo)->getPos3T<T>(geo->vertexPoint(vertices[closed ? numvtx - 1 : 0]));
        UT_Vector3T<T> pos0 = geo->getPos3(geo->vertexPoint(vertices[closed ? numvtx - 1 : 0]));
#endif
        
        for (GA_Size i(!closed); i < numvtx; ++i)
        {
#if SYS_VERSION_MAJOR_INT > 19 || ( SYS_VERSION_MAJOR_INT == 19 && SYS_VERSION_MINOR_INT == 5 )
            UT_Vector3T<T> pos1 = geo->getPos3T<T>(geo->vertexPoint(vertices[i]));
#else
            UT_Vector3T<T> pos1 = geo->getPos3(geo->vertexPoint(vertices[i]));
#endif
            pSum += (pos1 - pos0).length();
            pos0 = pos1;
        }

        return pSum;
    }

#endif



    //GU_Detail::compute3DArea(const GA_Offset primoff)

    template<typename T>
    static T computePerimeter(const GA_Detail* const geo, const GA_Offset primoff, const GA_ROHandleT<UT_Vector3T<T>>& pos_h)
    {
        const GA_OffsetListRef& vertices = geo->getPrimitiveVertexList(primoff);
        const GA_Size numvtx = vertices.size();
        if (numvtx < 2)
            return 0.0;

        const bool closed = vertices.getExtraFlag();
        T pSum = 0.0;

        switch (geo->getPrimitiveTypeId(primoff))
        {
        case GA_PRIMPOLY:
        {
        }
        break;
        default:
            return 0.0;
        }
        
        const GA_AttributeOwner attribOwner = pos_h.getAttribute()->getOwner();

        UT_Vector3T<T> pos0, pos1;
        switch (attribOwner)
        {
        case GA_ATTRIB_VERTEX:
        {
            pos0 = pos_h.get(vertices[closed ? numvtx - 1 : 0]);
            for (GA_Size i(!closed); i < numvtx; ++i)
            {
                pos1 = pos_h.get(vertices[i]);
                pSum += (pos1 - pos0).length();
                pos0 = pos1;
            }
        }
        break;
        case GA_ATTRIB_POINT:
        {
            pos0 = pos_h.get(geo->vertexPoint(vertices[closed ? numvtx - 1 : 0]));
            for (GA_Size i(!closed); i < numvtx; ++i)
            {
                pos1 = pos_h.get(geo->vertexPoint(vertices[i]));
                pSum += (pos1 - pos0).length();
                pos0 = pos1;
            }
        }
        break;
        default:
            return 0.0;
        }

        return pSum;
    }



    template<typename _Ty>
    static void
        computePerimeter(
            const GA_Detail* const geo,
            const GA_RWHandleT<_Ty>& p_h,
            const GA_PrimitiveGroup* const geoPrimGroup = nullptr,
            const exint subscribeRatio = 16,
            const exint minGrainSize   = 1024
        )
    {
        //GU_Measure::computePerimeter(*geo, p_h, geoPrimGroup);
        UTparallelFor(GA_SplittableRange(geo->getPrimitiveRange(geoPrimGroup)), [geo, &p_h](const GA_SplittableRange& r)
        {
            GA_PageHandleT<_Ty, _Ty, true, true, GA_Attribute, GA_ATINumeric, GA_Detail> measure_ph(p_h.getAttribute());
            GA_Offset start, end;
            for (GA_PageIterator pit = r.beginPages(); !pit.atEnd(); ++pit)
            {
                for (GA_Iterator it(pit.begin()); it.blockAdvance(start, end); )
                {
                    measure_ph.setPage(start);
                    for (GA_Offset elemoff = start; elemoff < end; ++elemoff)
                    {
                        measure_ph.value(elemoff) = computePerimeter<_Ty>(geo, elemoff);
                    }
                }
            }
        }, subscribeRatio, minGrainSize);
    }

    SYS_FORCE_INLINE static void
        computePerimeter(
            const GA_Detail* const geo,
            GA_Attribute* const pAttrib,
            const GA_PrimitiveGroup* const geoPrimGroup = nullptr,
            const exint subscribeRatio = 16,
            const exint minGrainSize = 1024
        )
    {
        switch (pAttrib->getAIFTuple()->getStorage(pAttrib))
        {
        //case GA_STORE_REAL16: computePerimeter<fpreal16>(geo, pAttrib, geoPrimGroup, subscribeRatio, minGrainSize); break;
        case GA_STORE_REAL32: computePerimeter<fpreal32>(geo, pAttrib, geoPrimGroup, subscribeRatio, minGrainSize); break;
        case GA_STORE_REAL64: computePerimeter<fpreal64>(geo, pAttrib, geoPrimGroup, subscribeRatio, minGrainSize); break;
        default: UT_ASSERT_MSG(0, "unhandled storage type"); break;
        }
    }



    template<typename _Ty, typename T1>
    static void
        computePerimeter(
            const GA_Detail* const geo,
            const GA_RWHandleT<_Ty>& p_h,
            const GA_ROHandleT<UT_Vector3T<T1>>& pos_h,
            const GA_PrimitiveGroup* const geoPrimGroup = nullptr,
            const exint subscribeRatio = 16,
            const exint minGrainSize = 1024
        )
    {
        //GU_Measure::computePerimeter(*geo, p_h, geoPrimGroup);
        UTparallelFor(GA_SplittableRange(geo->getPrimitiveRange(geoPrimGroup)), [geo, &p_h, &pos_h](const GA_SplittableRange& r)
        {
            GA_PageHandleT<_Ty, _Ty, true, true, GA_Attribute, GA_ATINumeric, GA_Detail> measure_ph(p_h.getAttribute());
            GA_Offset start, end;
            for (GA_PageIterator pit = r.beginPages(); !pit.atEnd(); ++pit)
            {
                for (GA_Iterator it(pit.begin()); it.blockAdvance(start, end); )
                {
                    measure_ph.setPage(start);
                    for (GA_Offset elemoff = start; elemoff < end; ++elemoff)
                    {
                        measure_ph.value(elemoff) = computePerimeter(geo, elemoff, pos_h);
                    }
                }
            }
        }, subscribeRatio, minGrainSize);
    }

    
    static void
        computePerimeter(
            const GA_Detail* const geo,
            GA_Attribute* const pAttrib,
            const GA_Attribute* const posAttrib,
            const GA_PrimitiveGroup* const geoPrimGroup = nullptr,
            const exint subscribeRatio = 16,
            const exint minGrainSize = 1024
        )
    {
        switch (pAttrib->getAIFTuple()->getStorage(pAttrib))
        {
        case GA_STORE_REAL16:
            computePerimeter<fpreal16, fpreal16>(geo, pAttrib, posAttrib, geoPrimGroup, subscribeRatio, minGrainSize);
            break;
        case GA_STORE_REAL32:
            computePerimeter<fpreal32, fpreal32>(geo, pAttrib, posAttrib, geoPrimGroup, subscribeRatio, minGrainSize);
            break;
        case GA_STORE_REAL64:
            computePerimeter<fpreal64, fpreal64>(geo, pAttrib, posAttrib, geoPrimGroup, subscribeRatio, minGrainSize);
            break;
        default:
            UT_ASSERT_MSG(0, "unhandled storage type");
            break;
        }
    }








    //////////////////////////// Area ////////////////////////////////



#if 0

#define computeAreaMacro(T)                                                                                                           \
static fpreal##T                                                                                                                      \
computeArea##T(const GA_Detail* const geo, const GA_Offset primoff)                                                                   \
{                                                                                                                                     \
    const GA_OffsetListRef& vertices = geo->getPrimitiveVertexList(primoff);                                                          \
                                                                                                                                      \
    const bool closed = vertices.getExtraFlag();                                                                                      \
    if (!closed)                                                                                                                      \
    return 0.0;                                                                                                                       \
                                                                                                                                      \
    const GA_Size numvtx = vertices.size();                                                                                           \
    if (numvtx < 3)                                                                                                                   \
        return 0.0;                                                                                                                   \
                                                                                                                                      \
                                                                                                                                      \
                                                                                                                                      \
    switch (geo->getPrimitiveTypeId(primoff))                                                                                         \
    {                                                                                                                                 \
    case GA_PRIMPOLY:                                                                                                                 \
    {                                                                                                                                 \
    }                                                                                                                                 \
    break;                                                                                                                            \
    default:                                                                                                                          \
        return 0.0;                                                                                                                   \
    }                                                                                                                                 \
                                                                                                                                      \
                                                                                                                                      \
                                                                                                                                      \
    UT_Vector3T<fpreal##T> pos0 = geo->getPos3T<T>(geo->vertexPoint(vertices[0]));                                                    \
    UT_Vector3T<fpreal##T> pos1 = geo->getPos3T<T>(geo->vertexPoint(vertices[1]));                                                    \
    const UT_Vector3T<fpreal##T> pos2 = geo->getPos3T<T>(geo->vertexPoint(vertices[2]));                                              \
                                                                                                                                      \
    /*GA_Offset ptoff0 = geo->vertexPoint(geo->getPrimitiveVertexOffset(primoff, 0));*/                                               \
                                                                                                                                      \
    switch (numvtx)                                                                                                                   \
    {                                                                                                                                 \
    case 3:                                                                                                                           \
    {                                                                                                                                 \
        return heronsFormula(pos0, pos1, pos2);                                                                                       \
    }                                                                                                                                 \
    break;                                                                                                                            \
    case 4:                                                                                                                           \
    {                                                                                                                                 \
        const UT_Vector3T<fpreal##T> pos3 = geo->getPos3T<T>(geo->vertexPoint(vertices[3]));                                          \
        return bretschneidersFormula0(pos0, pos1, pos2, pos3);                                                                        \
    }                                                                                                                                 \
    break;                                                                                                                            \
    default:                                                                                                                          \
        UT_ASSERT_MSG(0, "unsupport numvtx");                                                                                         \
        break;                                                                                                                        \
    }                                                                                                                                 \
                                                                                                                                      \
                                                                                                                                      \
                                                                                                                                      \
    const UT_Vector3T<fpreal##T> dir0 = pos1 - pos0;                                                                                  \
    const UT_Vector3T<fpreal##T> dir1 = pos2 - pos1;                                                                                  \
    const UT_Vector3T<fpreal##T> crossdir0B = cross(dir0, dir1);                                                                      \
    fpreal##T lengthdir0B = crossdir0B.length();                                                                                      \
    lengthdir0B = 1.0 / lengthdir0B;                                                                                                  \
    const fpreal##T cosnx = ((pos1[1] - pos0[1]) * (pos2[2] - pos0[2]) - (pos2[1] - pos0[1]) * (pos1[2] - pos0[2])) * lengthdir0B;    \
    const fpreal##T cosny = ((pos2[0] - pos0[0]) * (pos1[2] - pos0[2]) - (pos1[0] - pos0[0]) * (pos2[2] - pos0[2])) * lengthdir0B;    \
    const fpreal##T cosnz = ((pos1[0] - pos0[0]) * (pos2[1] - pos0[1]) - (pos2[0] - pos0[0]) * (pos1[1] - pos0[1])) * lengthdir0B;    \
                                                                                                                                      \
    fpreal##T areaSum = 0.0;                                                                                                          \
    pos0 = geo->getPos3T<T>(geo->vertexPoint(vertices[numvtx - 1]));                                                                  \
    for (GA_Size i(0); i < numvtx; ++i)                                                                                               \
    {                                                                                                                                 \
        pos1 = geo->getPos3T<T>(geo->vertexPoint(vertices[i]));                                                                       \
        areaSum += cosnz * (pos0[0] * pos1[1] - pos1[0] * pos0[1])                                                                    \
            + cosnx * (pos0[1] * pos1[2] - pos1[1] * pos0[2])                                                                         \
            + cosny * (pos0[2] * pos1[0] - pos1[2] * pos0[0]);                                                                        \
        pos0 = pos1;                                                                                                                  \
    }                                                                                                                                 \
    return abs(0.5 * areaSum);                                                                                                        \
}                                                                                                                                     \

    computeAreaMacro(32);
    computeAreaMacro(64);



    template<typename T>
    SYS_FORCE_INLINE static T computeArea(const GA_Detail* const geo, const GA_Offset primoff)
    {
        if constexpr (std::is_same<T, float>::value)
            return computeArea32(geo, primoff);
        else
            return computeArea64(geo, primoff);
    }
#else



    template<typename T>
    static T computeArea(const GA_Detail* const geo, const GA_Offset primoff)
    {
        const GA_OffsetListRef& vertices = geo->getPrimitiveVertexList(primoff);

        const bool closed = vertices.getExtraFlag();
        if (!closed)
            return 0.0;

        const GA_Size numvtx = vertices.size();
        if (numvtx < 3)
            return 0.0;


        switch (geo->getPrimitiveTypeId(primoff))
        {
        case GA_PRIMPOLY:
        {
        }
        break;
        default:
            return 0.0;
        }


#if SYS_VERSION_MAJOR_INT > 19 || ( SYS_VERSION_MAJOR_INT == 19 && SYS_VERSION_MINOR_INT == 5 )
        UT_Vector3T<T> pos0 = geo->getPos3T<T>(geo->vertexPoint(vertices[0]));
        UT_Vector3T<T> pos1 = geo->getPos3T<T>(geo->vertexPoint(vertices[1]));
        const UT_Vector3T<T>& pos2 = geo->getPos3T<T>(geo->vertexPoint(vertices[2]));
#else
        UT_Vector3T<T> pos0 = geo->getPos3(geo->vertexPoint(vertices[0]));
        UT_Vector3T<T> pos1 = geo->getPos3(geo->vertexPoint(vertices[1]));
        const UT_Vector3T<T>& pos2 = geo->getPos3(geo->vertexPoint(vertices[2]));
#endif



        /*GA_Offset ptoff0 = geo->vertexPoint(geo->getPrimitiveVertexOffset(primoff, 0));*/

        switch (numvtx)
        {
        case 3:
        {
            return heronsFormula(pos0, pos1, pos2);
        }
        break;
        case 4:
        {
#if SYS_VERSION_MAJOR_INT > 19 || ( SYS_VERSION_MAJOR_INT == 19 && SYS_VERSION_MINOR_INT == 5 )
            const UT_Vector3T<T>& pos3 = geo->getPos3T<T>(geo->vertexPoint(vertices[3]));
#else
            const UT_Vector3T<T>& pos3 = geo->getPos3(geo->vertexPoint(vertices[3]));
#endif
            return bretschneidersFormula0(pos0, pos1, pos2, pos3);
        }
        break;
        default:
            //UT_ASSERT_MSG(0, "unsupport numvtx");
            break;
        }



        const UT_Vector3T<T>& dir0 = pos1 - pos0;
        const UT_Vector3T<T>& dir1 = pos2 - pos1;
        const UT_Vector3T<T>& crossdir0B = cross(dir0, dir1);
        T lengthdir0B = crossdir0B.length();
        lengthdir0B = 1.0 / lengthdir0B;
        const T cosnx = ((pos1[1] - pos0[1]) * (pos2[2] - pos0[2]) - (pos2[1] - pos0[1]) * (pos1[2] - pos0[2])) * lengthdir0B;
        const T cosny = ((pos2[0] - pos0[0]) * (pos1[2] - pos0[2]) - (pos1[0] - pos0[0]) * (pos2[2] - pos0[2])) * lengthdir0B;
        const T cosnz = ((pos1[0] - pos0[0]) * (pos2[1] - pos0[1]) - (pos2[0] - pos0[0]) * (pos1[1] - pos0[1])) * lengthdir0B;

        T areaSum = 0.0;
#if SYS_VERSION_MAJOR_INT > 19 || ( SYS_VERSION_MAJOR_INT == 19 && SYS_VERSION_MINOR_INT == 5 )
        pos0 = geo->getPos3T<T>(geo->vertexPoint(vertices[numvtx - 1]));
#else
        pos0 = geo->getPos3(geo->vertexPoint(vertices[numvtx - 1]));
#endif
        for (GA_Size i(0); i < numvtx; ++i)
        {
#if SYS_VERSION_MAJOR_INT > 19 || ( SYS_VERSION_MAJOR_INT == 19 && SYS_VERSION_MINOR_INT == 5 )
            pos1 = geo->getPos3T<T>(geo->vertexPoint(vertices[i]));
#else
            pos1 = geo->getPos3(geo->vertexPoint(vertices[i]));
#endif
            areaSum += cosnz * (pos0[0] * pos1[1] - pos1[0] * pos0[1])
                + cosnx * (pos0[1] * pos1[2] - pos1[1] * pos0[2])
                + cosny * (pos0[2] * pos1[0] - pos1[2] * pos0[0]);
            pos0 = pos1;
        }
        return abs(0.5 * areaSum);
    }

#endif



    //////////////////////////// Area ////////////////////////////////

    template<typename T>
    static T
        //GU_Detail::compute3DArea(const GA_Offset primoff)
        //computeArea(const GU_Detail* geo, const GA_Offset primoff, GA_ROHandleT<UT_Vector3T<T>&> pos_h, GA_AttributeOwner attribOwner)
        computeArea(
            const GA_Detail* const geo,
            const GA_Offset primoff,
            const GA_ROHandleT<UT_Vector3T<T>>& pos_h
        )
    {
        const GA_OffsetListRef& vertices = geo->getPrimitiveVertexList(primoff);

        const bool closed = vertices.getExtraFlag();
        if (!closed)
            return 0.0;

        const GA_Size numvtx = vertices.size();
        if (numvtx < 3)
            return 0.0;


        switch (geo->getPrimitiveTypeId(primoff))
        {
        case GA_PRIMPOLY:
        {
        }
        break;
        default:
            return 0.0;
        }

        const GA_AttributeOwner attribOwner = pos_h.getAttribute()->getOwner();

        UT_Vector3T<T> pos0, pos1, pos2;

        switch (attribOwner)
        {
        case GA_ATTRIB_VERTEX:
        {
            pos0 = pos_h.get(vertices[0]);
            pos1 = pos_h.get(vertices[1]);
            pos2 = pos_h.get(vertices[2]);
        }
        break;
        case GA_ATTRIB_POINT:
        {
            pos0 = pos_h.get(geo->vertexPoint(vertices[0]));
            pos1 = pos_h.get(geo->vertexPoint(vertices[1]));
            pos2 = pos_h.get(geo->vertexPoint(vertices[2]));
        }
        break;
        default:
            return 0.0;
        }
        //UT_Vector3T<T>& pos1 = geo->getPos3T<T>(geo->vertexPoint(vertices[0]));
        //UT_Vector3T<T>& pos1 = geo->getPos3T<T>(geo->vertexPoint(vertices[1]));
        //UT_Vector3T<T>& pos2 = geo->getPos3T<T>(geo->vertexPoint(vertices[2]));

        //GA_Offset ptoff0 = geo->vertexPoint(geo->getPrimitiveVertexOffset(primoff, 0));

        switch (numvtx)
        {
        case 3:
        {
            return heronsFormula(pos0, pos1, pos2);
        }
        break;
        case 4:
        {
            //UT_Vector3T<T> pos3 = geo->getPos3T<T>(geo->vertexPoint(vertices[3]));
            UT_Vector3T<T> pos3;
            switch (attribOwner)
            {
            case GA_ATTRIB_VERTEX:
            {
                pos3 = pos_h.get(vertices[3]);
            }
            break;
            case GA_ATTRIB_POINT:
            {
                pos3 = pos_h.get(geo->vertexPoint(vertices[3]));
            }
            break;
            default:
                return 0.0;
            }
            return bretschneidersFormula0(pos0, pos1, pos2, pos3);
        }
        break;
        default:
            UT_ASSERT_MSG(0, "unhandled numvtx");
            break;
        }

        const UT_Vector3T<T>& dir0 = pos1 - pos0;
        const UT_Vector3T<T>& dir1 = pos2 - pos1;
        const UT_Vector3T<T>& crossdir0B = cross(dir0, dir1);
        T lengthdir0B = crossdir0B.length();
        lengthdir0B = 1.0 / lengthdir0B;
        const T cosnx = ((pos1[1] - pos0[1]) * (pos2[2] - pos0[2]) - (pos2[1] - pos0[1]) * (pos1[2] - pos0[2])) * lengthdir0B;
        const T cosny = ((pos2[0] - pos0[0]) * (pos1[2] - pos0[2]) - (pos1[0] - pos0[0]) * (pos2[2] - pos0[2])) * lengthdir0B;
        const T cosnz = ((pos1[0] - pos0[0]) * (pos2[1] - pos0[1]) - (pos2[0] - pos0[0]) * (pos1[1] - pos0[1])) * lengthdir0B;

        T areaSum = 0.0;
        //pos0 = geo->getPos3T<T>(geo->vertexPoint(vertices[numvtx - 1]));
        switch (attribOwner)
        {
        case GA_ATTRIB_VERTEX:
        {
            pos0 = pos_h.get(vertices[numvtx - 1]);
            for (GA_Size vtxpnum = 0; vtxpnum < numvtx; ++vtxpnum)
            {
                //pos1 = geo->getPos3T<T>(geo->vertexPoint(vertices[vtxpnum]));
                pos1 = pos_h.get(vertices[vtxpnum]);
                areaSum += cosnz * (pos0[0] * pos1[1] - pos1[0] * pos0[1])
                    + cosnx * (pos0[1] * pos1[2] - pos1[1] * pos0[2])
                    + cosny * (pos0[2] * pos1[0] - pos1[2] * pos0[0]);
                pos0 = pos1;
            }
        }
        break;
        case GA_ATTRIB_POINT:
        {
            pos0 = pos_h.get(geo->vertexPoint(vertices[numvtx - 1]));
            for (GA_Size vtxpnum = 0; vtxpnum < numvtx; ++vtxpnum)
            {
                //pos1 = geo->getPos3T<T>(geo->vertexPoint(vertices[vtxpnum]));
                pos1 = pos_h.get(geo->vertexPoint(vertices[vtxpnum]));
                areaSum += cosnz * (pos0[0] * pos1[1] - pos1[0] * pos0[1])
                    + cosnx * (pos0[1] * pos1[2] - pos1[1] * pos0[2])
                    + cosny * (pos0[2] * pos1[0] - pos1[2] * pos0[0]);
                pos0 = pos1;
            }
        }
        break;
        default:
            return 0.0;
        }

        return abs(0.5 * areaSum);
    }





    template<typename T, typename T1>
    static void
        computeArea(
            const GA_Detail* const geo,
            const GA_RWHandleT<T>& area_h,
            const GA_ROHandleT<UT_Vector3T<T1>>& pos_h,
            const GA_PrimitiveGroup* const geoPrimGroup = nullptr,
            const exint subscribeRatio = 16,
            const exint minGrainSize = 1024
        )
    {
        //GU_Measure::computeArea(*geo, area_h, geoPrimGroup);
        UTparallelFor(GA_SplittableRange(geo->getPrimitiveRange(geoPrimGroup)), [geo, &area_h, &pos_h](const GA_SplittableRange& r)
        {
            GA_PageHandleT<T, T, true, true, GA_Attribute, GA_ATINumeric, GA_Detail> measure_ph(area_h.getAttribute());
            GA_Offset start, end;
            for (GA_PageIterator pit = r.beginPages(); !pit.atEnd(); ++pit)
            {
                for (GA_Iterator it(pit.begin()); it.blockAdvance(start, end); )
                {
                    measure_ph.setPage(start);
                    for (GA_Offset elemoff = start; elemoff < end; ++elemoff)
                    {
                        measure_ph.value(elemoff) = computeArea(geo, elemoff, pos_h);
                    }
                }
            }
        }, subscribeRatio, minGrainSize);
    }



template<typename T>
static void
    computeArea(
        const GA_Detail* const geo,
        const GA_RWHandleT<T>& area_h,
        const GA_PrimitiveGroup* const geoPrimGroup = nullptr,
        const exint subscribeRatio = 16,
        const exint minGrainSize = 1024
    )
{
    //GU_Measure::computeArea(*geo, area_h, geoPrimGroup);
    UTparallelFor(GA_SplittableRange(geo->getPrimitiveRange(geoPrimGroup)), [geo, &area_h](const GA_SplittableRange& r)
    {
        GA_PageHandleT<T, T, true, true, GA_Attribute, GA_ATINumeric, GA_Detail> measure_ph(area_h.getAttribute());
        GA_Offset start, end;
        for (GA_PageIterator pit = r.beginPages(); !pit.atEnd(); ++pit)
        {
            for (GA_Iterator it(pit.begin()); it.blockAdvance(start, end); )
            {
                measure_ph.setPage(start);
                for (GA_Offset elemoff = start; elemoff < end; ++elemoff)
                {
                    measure_ph.value(elemoff) = computeArea<T>(geo, elemoff);
                }
            }
        }
    }, subscribeRatio, minGrainSize);
}

SYS_FORCE_INLINE
    static void
    computeArea(
        const GA_Detail* const geo,
        GA_Attribute* const areaAttribPtr,
        const GA_PrimitiveGroup* const geoPrimGroup = nullptr,
        const exint subscribeRatio = 16,
        const exint minGrainSize = 1024
    )
{
    switch (areaAttribPtr->getAIFTuple()->getStorage(areaAttribPtr))
    {
    case GA_STORE_REAL16: computeArea<fpreal16>(geo, areaAttribPtr, geoPrimGroup, subscribeRatio, minGrainSize); break;
    case GA_STORE_REAL32: computeArea<fpreal32>(geo, areaAttribPtr, geoPrimGroup, subscribeRatio, minGrainSize); break;
    case GA_STORE_REAL64: computeArea<fpreal64>(geo, areaAttribPtr, geoPrimGroup, subscribeRatio, minGrainSize); break;
    default: UT_ASSERT_MSG(0, "unhandled storage type"); break;
    }
}

SYS_FORCE_INLINE
    static void
    computeArea(
        const GA_Detail* const geo,
        GA_Attribute* const areaAttribPtr,
        const GA_Attribute* const posAttribPtr,
        const GA_PrimitiveGroup* const geoPrimGroup = nullptr,
        const exint subscribeRatio = 16,
        const exint minGrainSize = 1024
    )
{
    switch (areaAttribPtr->getAIFTuple()->getStorage(areaAttribPtr))
    {
    case GA_STORE_REAL16: computeArea<fpreal16, fpreal32>(geo, areaAttribPtr, posAttribPtr, geoPrimGroup, subscribeRatio, minGrainSize); break;
    case GA_STORE_REAL32: computeArea<fpreal32, fpreal32>(geo, areaAttribPtr, posAttribPtr, geoPrimGroup, subscribeRatio, minGrainSize); break;
    case GA_STORE_REAL64: computeArea<fpreal64, fpreal64>(geo, areaAttribPtr, posAttribPtr, geoPrimGroup, subscribeRatio, minGrainSize); break;
    default: UT_ASSERT_MSG(0, "unhandled storage type"); break;
    }
}












    //////////////////////////// volume ////////////////////////////////




    static fpreal32
        computeMeshVolume(
            const GA_Detail* const geo,
            const GA_PrimitiveGroup* const geoPrimGroup = nullptr,
            const exint subscribeRatio = 16,
            const exint minGrainSize = 1024
        )
    {
        UT_BoundingBoxT<fpreal32> geoBounds;
        geo->computeQuickBounds(geoBounds);

        const UT_Vector3T<fpreal32>& bboxCenter = geoBounds.center();

        ComputeVolume body(geo, bboxCenter);
        const GA_SplittableRange geoSplittableRange(geo->getPrimitiveRange(geoPrimGroup));
        UTparallelReduce(geoSplittableRange, body, subscribeRatio, minGrainSize);
        return body.getSum();
    }

    template<typename _Ty>
    static _Ty
        computeMeshVolume(
            const GA_Detail* const geo,
            //const GA_AttributeOwner posAttribOwner = GA_ATTRIB_PRIMITIVE,
            const GA_ROHandleT<UT_Vector3T<_Ty>>& pos_h,
            const GA_PrimitiveGroup* const geoPrimGroup = nullptr,
            const exint subscribeRatio = 16,
            const exint minGrainSize = 1024
        )
    {
        const UT_BoundingBoxT<_Ty>& geoBBox = static_cast<const GFE_Detail*>(geo)->stdBoundingBox<_Ty>(geo->getPointRange(), pos_h.getAttribute());
        const UT_Vector3T<_Ty>& bboxCenter = geoBBox.center();

        ComputeVolume body(geo, bboxCenter);
        const GA_SplittableRange geoSplittableRange(geo->getPrimitiveRange(geoPrimGroup));
        UTparallelReduce(geoSplittableRange, body, subscribeRatio, minGrainSize);
        return body.getSum();
    }


    template<typename _Ty>
    static _Ty
        computeMeshVolume(
            const GA_Detail* const geo,
            //const GA_AttributeOwner posAttribOwner = GA_ATTRIB_PRIMITIVE,
            const GA_Attribute* const posAttrib,
            const GA_PrimitiveGroup* const geoPrimGroup = nullptr,
            const exint subscribeRatio = 16,
            const exint minGrainSize = 1024
        )
    {
        const UT_BoundingBoxT<_Ty>& geoBBox = static_cast<const GFE_Detail*>(geo)->stdBoundingBox<_Ty>(geo->getPointRange(), posAttrib);
        const UT_Vector3T<fpreal32>& bboxCenter = geoBBox.center();

        ComputeVolume body(geo, bboxCenter);
        const GA_SplittableRange geoSplittableRange(geo->getPrimitiveRange(geoPrimGroup));
        UTparallelReduce(geoSplittableRange, body, subscribeRatio, minGrainSize);
        return body.getSum();
    }





    
public:

    

    /////////////////////////////////////// Area //////////////////////////////////////////////

    template<typename T>
    static T heronsFormula(const UT_Vector3T<T>& pos0, const UT_Vector3T<T>& pos1, const UT_Vector3T<T>& pos2)
    {
        //Heron's formula
        const T length0 = pos0.distance(pos1);
        const T length1 = pos1.distance(pos2);
        const T length2 = pos2.distance(pos0);
        const T halfP = (length0 + length1 + length2) * 0.5;
        return sqrt(halfP * (halfP - length0) * (halfP - length1) * (halfP - length2));
    }

    //using triangleArea = heronsFormula;
    //typedef heronsFormula triangleArea;



    template<typename T>
    static T
        bretschneidersFormula0(
            const UT_Vector3T<T>& pos0,
            const UT_Vector3T<T>& pos1,
            const UT_Vector3T<T>& pos2,
            const UT_Vector3T<T>& pos3
        )
    {
        //Bretschneider's formula 0
        //https://en.wikipedia.org/wiki/Bretschneider's_formula
        //https://zhuanlan.zhihu.com/p/101219058
        //https://zhuanlan.zhihu.com/p/110025234
        const T length0 = pos0.distance2(pos1);
        const T length1 = pos1.distance2(pos2);
        const T length2 = pos2.distance2(pos3);
        const T length3 = pos3.distance2(pos0);
        T e = pos0.distance2(pos2);
        T f = pos1.distance2(pos3);
        e *= f;//e == (ef)
        f = length1 + length3 - length0 - length2;
        f *= f;

        return sqrt(4 * e - f) * 0.25;
    }


    //typedef bretschneidersFormula0 quadrangleArea;


    template<typename T>
    static T
        bretschneidersFormula1(
            const UT_Vector3T<T>& pos0, const UT_Vector3T<T>& pos1,
            const UT_Vector3T<T>& pos2, const UT_Vector3T<T>& pos3)
    {
        //Bretschneider's formula 1
        const T length0 = pos0.distance(pos1);
        const T length1 = pos1.distance(pos2);
        const T length2 = pos2.distance(pos3);
        const T length3 = pos3.distance(pos0);

        T length4 = pos0.distance(pos2);
        T length5 = pos1.distance(pos3);

        length5 *= length4;
        length4 = length0 * length2 + length1 * length3;

        const T halfP = (length0 + length1 + length2 + length3) * 0.5;
#if 1
        const T area = (halfP - length0) * (halfP - length1) * (halfP - length2) * (halfP - length3)
            - (length4 + length5) * (length4 - length5) * 0.25;
#else
        const T area = sqrt((halfP - length0) * (halfP - length1) * (halfP - length2) * (halfP - length3)
            - length0 * length1 * length2 * length3 * (cos(angle) + 1) * 0.5);
#endif
        return sqrt(area);
    }







    
private:
    
    
    template<typename _Ty>
    class ComputePerimeter
    {
    public:
        ComputePerimeter(const GA_Detail* const a, const GA_ROHandleT<UT_Vector3T<_Ty>>& b)
            : myGeo(a)
            , pos_h(b)
            , mySum(0)
        {}
        ComputePerimeter(ComputePerimeter& src, UT_Split)
            : myGeo(src.myGeo)
            , pos_h(src.pos_h)
            , mySum(0)
        {}
        void operator()(const GA_SplittableRange& r)
        {
            GA_Offset start, end;
            for (GA_Iterator it(r); it.blockAdvance(start, end); )
            {
                for (GA_Offset elemoff = start; elemoff < end; ++elemoff)
                {
                    mySum += computePerimeter<_Ty>(myGeo, elemoff, pos_h);
                }
            }
        }
        SYS_FORCE_INLINE void join(const ComputePerimeter& other)
        { mySum += other.mySum; }
        SYS_FORCE_INLINE _Ty getSum() const
        { return mySum; }
    private:
        _Ty mySum;
        const GA_Detail* const myGeo;
        const GA_ROHandleT<UT_Vector3T<_Ty>>& pos_h;
    }; // End of Class ComputePerimeter



    
    template<typename _Ty>
    class ComputeArea
    {
    public:
        ComputeArea(const GA_Detail* const a, const GA_ROHandleT<UT_Vector3T<_Ty>>& b)
            : myGeo(a)
            , pos_h(b)
            , mySum(0)
        {}
        ComputeArea(ComputeArea& src, UT_Split)
            : myGeo(src.myGeo)
            , pos_h(src.pos_h)
            , mySum(0)
        {}
        void operator()(const GA_SplittableRange& r)
        {
            GA_Offset start, end;
            for (GA_Iterator it(r); it.blockAdvance(start, end); )
            {
                for (GA_Offset elemoff = start; elemoff < end; ++elemoff)
                {
                    mySum += computeArea<_Ty>(myGeo, elemoff, pos_h);
                }
            }
        }
        SYS_FORCE_INLINE void join(const ComputeArea& other)
        { mySum += other.mySum; }
        SYS_FORCE_INLINE _Ty getSum() const
        { return mySum; }
    private:
        _Ty mySum;
        const GA_Detail* const myGeo;
        const GA_ROHandleT<UT_Vector3T<_Ty>>& pos_h;
    }; // End of Class ComputeArea
    
    
    template<typename _Ty>
    class ComputeVolume
    {
    public:
        ComputeVolume(const GA_Detail* const a, const UT_Vector3T<_Ty>& b)
            : myGeo(a)
            , myBBoxCenter(b)
            , mySum(0)
        {}
        ComputeVolume(ComputeVolume& src, UT_Split)
            : myGeo(src.myGeo)
            , myBBoxCenter(src.myBBoxCenter)
            , mySum(0)
        {}
        void operator()(const GA_SplittableRange& r)
        {
            GA_Offset start, end;
            for (GA_Iterator it(r); it.blockAdvance(start, end); )
            {
                for (GA_Offset elemoff = start; elemoff < end; ++elemoff)
                {
                    const GEO_Hull* const prim = static_cast<const GEO_Hull*>(myGeo->getPrimitive(elemoff));
                    mySum += prim->calcVolume(myBBoxCenter);
                }
            }
        }
        SYS_FORCE_INLINE void join(const ComputeVolume& other)
        { mySum += other.mySum; }
        SYS_FORCE_INLINE _Ty getSum() const
        { return mySum; }
    private:
        _Ty mySum;
        const GA_Detail* const myGeo;
        const UT_Vector3T<_Ty>& myBBoxCenter;
    }; // End of Class ComputeVolume



    


public:
    MeasureType measureType = MeasureType::Area;

    
private:
    GA_Attribute* measureAttrib;
    
    exint subscribeRatio = 64;
    exint minGrainSize   = 1024;
    
}; // End of Class Measure
_GFEL_END
#endif
