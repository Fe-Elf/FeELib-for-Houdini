
#pragma once

#ifndef __GFE_CurveUV_h__
#define __GFE_CurveUV_h__

#include "GFE/GFE_CurveUV.h"

#include "GFE/GFE_GeoFilter.h"


//#include "GEO/GEO_Curve.h"
//#include "GFE/GFE_Type.h"
//#include "GFE/GFE_Attribute.h"
//#include "GFE/GFE_GroupParser.h"
//#include "GFE/GFE_Measure.h"



enum class GFE_CurveUVMethod
{
    LocalAverage,
    LocalArcLength,
    WorldAverage,
    WorldArcLength,
};

    
class GFE_CurveUV : public GFE_AttribFilter {

public:
    using MethodVariant = std::variant<std::integral_constant<GFE_CurveUVMethod, GFE_CurveUVMethod::LocalAverage  >,
                                       std::integral_constant<GFE_CurveUVMethod, GFE_CurveUVMethod::LocalArcLength>,
                                       std::integral_constant<GFE_CurveUVMethod, GFE_CurveUVMethod::WorldAverage  >,
                                       std::integral_constant<GFE_CurveUVMethod, GFE_CurveUVMethod::WorldArcLength>>;

    static MethodVariant getMethodVariant(const GFE_CurveUVMethod curveUVMethod)
    {
        switch (curveUVMethod)
        {
        case GFE_CurveUVMethod::LocalAverage:   return std::integral_constant<GFE_CurveUVMethod, GFE_CurveUVMethod::LocalAverage  >{}; break;
        case GFE_CurveUVMethod::LocalArcLength: return std::integral_constant<GFE_CurveUVMethod, GFE_CurveUVMethod::LocalArcLength>{}; break;
        case GFE_CurveUVMethod::WorldAverage:   return std::integral_constant<GFE_CurveUVMethod, GFE_CurveUVMethod::WorldAverage  >{}; break;
        case GFE_CurveUVMethod::WorldArcLength: return std::integral_constant<GFE_CurveUVMethod, GFE_CurveUVMethod::WorldArcLength>{}; break;
        }
        UT_ASSERT_MSG(0, "Unhandled Curve UV Method");
        return std::integral_constant<GFE_CurveUVMethod, GFE_CurveUVMethod::LocalAverage>{};
    }



    
    using GFE_AttribFilter::GFE_AttribFilter;

    ~GFE_CurveUV()
    {
    }


    //SYS_FORCE_INLINE void setGroup(const GA_PrimitiveGroup* const geoPrimGroup = nullptr)
    //{ groupParser.setGroup(geoPrimGroup); }

    //SYS_FORCE_INLINE void setGroup(const UT_StringHolder& primGroupName = "")
    //{ groupParser.setPrimitiveGroup(primGroupName); }


    //SYS_FORCE_INLINE GA_Attribute*
    //    findOrCreateUV(
    //        const bool detached = false,
    //        const GA_AttributeOwner attribClass = GA_ATTRIB_VERTEX,
    //        const GA_Storage storage = GA_STORE_INVALID,
    //        const UT_StringHolder& name = "uv",
    //        const int tuple_size = 3
    //    )
    //{ return getOutAttribArray().findOrCreateUV(detached, attribClass, storage, name, tuple_size); }

    void
        setComputeParm(
            const GFE_CurveUVMethod curveUVMethod = GFE_CurveUVMethod::WorldArcLength,
            const exint subscribeRatio = 64,
            const exint minGrainSize   = 1024
        )
    {
        setHasComputed();
        this->curveUVMethod  = curveUVMethod;
        this->subscribeRatio = subscribeRatio;
        this->minGrainSize   = minGrainSize;
    }


private:

    virtual bool
        computeCore() override
    {
        if (getOutAttribArray().isEmpty())
            return false;

        if (groupParser.isEmpty())
            return true;

        setValidConstPosAttrib();
        
        attrib = getOutAttribArray()[0];
        curveUV();

        return true;
    }
    
    void curveUV()
    {
        const GFE_AttribStorage attribStorage = GFE_Type::getAttribStorage(attrib);
        if (!GFE_Variant::isAttribStorageFVF(attribStorage))
            return;
        auto storageVariant = GFE_Variant::getAttribStorageVariantFVF(attribStorage);
        auto methodVariant = getMethodVariant(curveUVMethod);
        //auto outAttribMaxVariant = GFE_Variant::getBoolVariant(outAttribMax);
        std::visit([&] (auto storageVariant,
                        auto methodVariant)
        {
            using type = typename GFE_Variant::getAttribStorage_t<storageVariant>;
            curveUV<type, methodVariant>();
        }, storageVariant, methodVariant);
    }
    
    template<typename _Ty, GFE_CurveUVMethod _TCurveUVMethod>
    void curveUV()
    {
        using value_type = typename GFE_Type::get_value_type_t<_Ty>;
        
        const GA_RWHandleT<_Ty> uv_h(attrib);

        const bool isPointAttrib = attrib->getOwner() == GA_ATTRIB_POINT;
        //const GA_AttributeOwner owner = attrib->getOwner();

        const GA_ROHandleT<UT_Vector3T<value_type>> pos_h(posAttrib);

        UTparallelFor(groupParser.getPrimitiveSplittableRange(), [this, &uv_h, &pos_h, isPointAttrib](const GA_SplittableRange& r)
        {
            constexpr bool isScalar = GFE_Type::isScalar<_Ty>;
            UT_Array<value_type> uvs;
            GA_Size lastIndex;
            value_type dist, p;
            _Ty uv;
            UT_Vector3T<value_type> pos, pos_prev;
            GA_Offset ptoff, vtxoff;
            GA_Offset start, end;
            for (GA_Iterator it(r); it.blockAdvance(start, end); )
            {
                for (GA_Offset primoff = start; primoff < end; ++primoff)
                {
                    const GA_OffsetListRef& vertices = geo->getPrimitiveVertexList(primoff);
                    const GA_Size numvtx = vertices.size();
                    if (numvtx < 2)
                        continue;
                    
                    uv = GFE_Type::getZero<_Ty>();

                    if constexpr (_TCurveUVMethod == GFE_CurveUVMethod::LocalAverage)
                    {
                        dist = 1.0 / (numvtx - 1);
                    }
                    else if constexpr (_TCurveUVMethod == GFE_CurveUVMethod::LocalArcLength)
                    {
                        dist = 0;
                        uvs.setSize(numvtx);
                        ptoff = geo->vertexPoint(vertices[0]);
                        pos_prev = pos_h.get(ptoff);
                    }
                    else if constexpr (_TCurveUVMethod == GFE_CurveUVMethod::WorldAverage)
                    {
                        ptoff = geo->vertexPoint(vertices[0]);
                        pos_prev = pos_h.get(ptoff);
                    }
                    else if constexpr (_TCurveUVMethod == GFE_CurveUVMethod::WorldArcLength)
                    {
                        ptoff = geo->vertexPoint(vertices[0]);
                        pos_prev = pos_h.get(ptoff);
                    }

                    
                    for (GA_Size vtxpnum = 1; vtxpnum < numvtx; vtxpnum++)
                    {
                        if constexpr (_TCurveUVMethod == GFE_CurveUVMethod::LocalAverage)
                        {
                            vtxoff = vertices[vtxpnum];
                            if constexpr (isScalar)
                                uv += dist;
                            else
                                uv[0] += dist;
                            uv_h.set(isPointAttrib ? geo->vertexPoint(vtxoff) : vtxoff, 0, uv);
                        }
                        else if constexpr (_TCurveUVMethod == GFE_CurveUVMethod::LocalArcLength)
                        {
                            for (GA_Size vtxpnum = 1; vtxpnum < numvtx; vtxpnum++)
                            {
                                vtxoff = vertices[vtxpnum];
                                ptoff = geo->vertexPoint(vtxoff);
                                pos = pos_h.get(ptoff);
                                dist += pos.distance(pos_prev);
                                uvs[vtxpnum] = dist;
                                pos_prev = pos;
                            }
                            GA_Size lastIndex = numvtx - 1;
                            value_type p = uvs[lastIndex];
                            for (GA_Size vtxpnum = 1; vtxpnum < lastIndex; vtxpnum++)
                            {
                                vtxoff = vertices[vtxpnum];
                                if constexpr (isScalar)
                                    uv = uvs[vtxpnum] / p;
                                else
                                    uv[0] = uvs[vtxpnum] / p;
                                uv_h.set(isPointAttrib ? geo->vertexPoint(vtxoff) : vtxoff, 0, uv);
                            }
                            vtxoff = vertices[lastIndex];
                            if constexpr (isScalar)
                                uv = 1.0;
                            else
                                uv[0] = 1.0;
                            uv_h.set(isPointAttrib ? geo->vertexPoint(vtxoff) : vtxoff, 0, uv);
                        }
                        else if constexpr (_TCurveUVMethod == GFE_CurveUVMethod::WorldAverage)
                        {
                            vtxoff = vertices[vtxpnum];
                            ptoff = geo->vertexPoint(vtxoff);
                            pos = pos_h.get(ptoff);
                            dist += pos.distance(pos_prev);
                            pos_prev = pos;
                        }
                        else if constexpr (_TCurveUVMethod == GFE_CurveUVMethod::WorldArcLength)
                        {
                            vtxoff = vertices[vtxpnum];
                            ptoff = geo->vertexPoint(vtxoff);
                            pos = pos_h.get(ptoff);
                            if constexpr (isScalar)
                                uv += pos.distance(pos_prev);
                            else
                                uv[0] += pos.distance(pos_prev);
                            uv_h.set(isPointAttrib ? ptoff : vtxoff, 0, uv);
                            pos_prev = pos;
                        }
                    }

                    if constexpr (_TCurveUVMethod == GFE_CurveUVMethod::LocalArcLength)
                    {
                        lastIndex = numvtx - 1;
                        p = uvs[lastIndex];
                    }
                    else if constexpr (_TCurveUVMethod == GFE_CurveUVMethod::WorldAverage)
                    {
                        lastIndex = numvtx;
                        dist /= (numvtx - 1);
                    }
                    else
                    {
                        lastIndex = numvtx;
                    }
                    for (GA_Size vtxpnum = 1; vtxpnum < lastIndex; vtxpnum++)
                    {
                        if constexpr (_TCurveUVMethod == GFE_CurveUVMethod::LocalArcLength)
                        {
                            vtxoff = vertices[vtxpnum];
                            if constexpr (isScalar)
                                uv = uvs[vtxpnum] / p;
                            else
                                uv[0] = uvs[vtxpnum] / p;
                            uv_h.set(isPointAttrib ? geo->vertexPoint(vtxoff) : vtxoff, 0, uv);
                        }
                        else if constexpr (_TCurveUVMethod == GFE_CurveUVMethod::WorldAverage)
                        {
                            vtxoff = vertices[vtxpnum];
                            if constexpr (isScalar)
                                uv += dist;
                            else
                                uv[0] += dist;
                            uv_h.set(isPointAttrib ? geo->vertexPoint(vtxoff) : vtxoff, 0, uv);
                        }
                    }
                    if constexpr (_TCurveUVMethod == GFE_CurveUVMethod::LocalArcLength)
                    {
                        vtxoff = vertices[lastIndex];
                        if constexpr (isScalar)
                            uv = 1.0;
                        else
                            uv[0] = 1.0;
                        uv_h.set(isPointAttrib ? geo->vertexPoint(vtxoff) : vtxoff, 0, uv);
                    }
                    /*
                    if constexpr (_TCurveUVMethod == GFE_CurveUVMethod::LocalAverage)
                    {
                        dist = 1.0 / (numvtx - 1);
                        for (GA_Size vtxpnum = 1; vtxpnum < numvtx; vtxpnum++)
                        {
                            vtxoff = vertices[vtxpnum];
                            if constexpr (isScalar)
                                uv += dist;
                            else
                                uv[0] += dist;
                            uv_h.set(isPointAttrib ? geo->vertexPoint(vtxoff) : vtxoff, 0, uv);
                        }
                    }
                    else if constexpr (_TCurveUVMethod == GFE_CurveUVMethod::LocalArcLength)
                    {
                        uvs.setSize(numvtx);
                        ptoff = geo->vertexPoint(vertices[0]);
                        pos_prev = pos_h.get(ptoff);
                        for (GA_Size vtxpnum = 1; vtxpnum < numvtx; vtxpnum++)
                        {
                            vtxoff = vertices[vtxpnum];
                            ptoff = geo->vertexPoint(vtxoff);
                            pos = pos_h.get(ptoff);
                            dist += pos.distance(pos_prev);
                            uvs[vtxpnum] = dist;
                            pos_prev = pos;
                        }
                        GA_Size lastIndex = numvtx - 1;
                        value_type p = uvs[lastIndex];
                        for (GA_Size vtxpnum = 1; vtxpnum < lastIndex; vtxpnum++)
                        {
                            vtxoff = vertices[vtxpnum];
                            if constexpr (isScalar)
                                uv = uvs[vtxpnum] / p;
                            else
                                uv[0] = uvs[vtxpnum] / p;
                            uv_h.set(isPointAttrib ? geo->vertexPoint(vtxoff) : vtxoff, 0, uv);
                        }
                        vtxoff = vertices[lastIndex];
                        if constexpr (isScalar)
                            uv = 1.0;
                        else
                            uv[0] = 1.0;
                        uv_h.set(isPointAttrib ? geo->vertexPoint(vtxoff) : vtxoff, 0, uv);
                    }
                    else if constexpr (_TCurveUVMethod == GFE_CurveUVMethod::WorldAverage)
                    {
                        ptoff = geo->vertexPoint(vertices[0]);
                        pos_prev = pos_h.get(ptoff);
                        for (GA_Size vtxpnum = 1; vtxpnum < numvtx; vtxpnum++)
                        {
                            vtxoff = vertices[vtxpnum];
                            ptoff = geo->vertexPoint(vtxoff);
                            pos = pos_h.get(ptoff);
                            dist += pos.distance(pos_prev);
                            pos_prev = pos;
                        }
                        dist /= (numvtx - 1);
                        for (GA_Size vtxpnum = 1; vtxpnum < numvtx; vtxpnum++)
                        {
                            vtxoff = vertices[vtxpnum];
                            if constexpr (isScalar)
                                uv += dist;
                            else
                                uv[0] += dist;
                            uv_h.set(isPointAttrib ? geo->vertexPoint(vtxoff) : vtxoff, 0, uv);
                        }
                    }
                    else if constexpr (_TCurveUVMethod == GFE_CurveUVMethod::WorldArcLength)
                    {
                        ptoff = geo->vertexPoint(vertices[0]);
                        pos_prev = pos_h.get(ptoff);
                        for (GA_Size vtxpnum = 1; vtxpnum < numvtx; vtxpnum++)
                        {
                            vtxoff = vertices[vtxpnum];
                            ptoff = geo->vertexPoint(vtxoff);
                            pos = pos_h.get(ptoff);
                            if constexpr (isScalar)
                                uv += pos.distance(pos_prev);
                            else
                                uv[0] += pos.distance(pos_prev);
                            uv_h.set(isPointAttrib ? ptoff : vtxoff, 0, uv);
                            pos_prev = pos;
                        }
                    }
                    */
                }
            }
        }, subscribeRatio, minGrainSize);
    }




public:
    GFE_CurveUVMethod curveUVMethod = GFE_CurveUVMethod::WorldArcLength;

private:

    GA_Attribute* attrib;

    
    exint subscribeRatio = 64;
    exint minGrainSize   = 1024;


}; // End of Class GFE_CurveUV







#endif
