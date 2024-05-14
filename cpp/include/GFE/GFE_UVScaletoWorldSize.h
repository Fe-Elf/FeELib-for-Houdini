
#pragma once

#ifndef __GFE_UVScaletoWorldSize_h__
#define __GFE_UVScaletoWorldSize_h__

#include <GFE/GFE_UVScaletoWorldSize.h>

#include <GFE/GeoFilter.h>

#include <GU/GU_Promote.h>

#include <GFE/GFE_Connectivity.h>
#include <GFE/GFE_Measure.h>

_GFEL_BEGIN
class UVScaletoWorldSize : public AttribFilter {

//#define GFE_UVScaletoWorldSize_AreaAttribName   "__area_GFE_UVScaletoWorldSize"
//#define GFE_UVScaletoWorldSize_AreaUVAttribName "__areaUV_GFE_UVScaletoWorldSize"

public:

    using AttribFilter::AttribFilter;
    

    void
        setComputeParm(
            const bool computeUVAreaInPiece = true,
            const UT_Vector4D& uvScale = UT_Vector4D(1.0, 1.0, 1.0, 1.0),
            const bool doUVScalex = true,
            const bool doUVScaley = true,
            const bool doUVScalez = true,
            const bool outTopoAttrib = true,
            const exint subscribeRatio = 64,
            const exint minGrainSize   = 1024
        )
    {
        setHasComputed();
        this->outTopoAttrib = outTopoAttrib;
        this->computeUVAreaInPiece = computeUVAreaInPiece;
        this->uvScale = uvScale;
        this->doUVScalex = doUVScalex;
        this->doUVScaley = doUVScaley;
        this->doUVScalez = doUVScalez;
        this->subscribeRatio = subscribeRatio;
        this->minGrainSize   = minGrainSize;
    }





private:


    virtual bool
        computeCore() override
    {
        if (!doUVScalex && !doUVScaley && !doUVScalez)
            return true;

        if (groupParser.isEmpty())
            return true;

        const size_t len = getOutAttribArrayRef().size();
        for (size_t i = 0; i < len; ++i)
        {
            uvAttrib = getOutAttribArrayRef()[i];
            
            const GFE_AttribStorage attribStorage = GFE_Type::getAttribStorage(uvAttrib);
            if (!GFE_Variant::isAttribStorageV(attribStorage))
                continue;
            auto storageVariant = GFE_Variant::getAttribStorageVariantV(attribStorage);
            auto isPointAttribVariant = GFE_Variant::getBoolVariant(uvAttrib->getOwner() == GA_ATTRIB_POINT);
            std::visit([&] (auto storageVariant)
            {
                using type = typename GFE_Variant::getAttribStorage_t<storageVariant>;
                uvScaletoWorldSize<type>();
            }, storageVariant);
        
        }
        return true;
    }



    template<typename _Ty>
    void uvScaletoWorldSize()
    {
        using value_type = typename _Ty::value_type;
        
        const GA_RWHandleT<_Ty>& uv_h(uvAttrib);
        
        GFE_Measure measure(geo, cookparms);
        measure.groupParser.setGroup(groupParser.getPrimitiveGroup());
        //measure.setPositionAttrib();
        //measure.setComputeParm(GFE_MeasureType::Area);

        //GA_Attribute* areaATIPtr = measure.getOutAttribArray().set(GA_ATTRIB_PRIMITIVE, GFE_UVScaletoWorldSize_AreaAttribName);
        GA_Attribute* areaAttrib = measure.findOrCreateTuple(true);
        measure.compute();

        //GA_Attribute* areaUVATIPtr = measure.getOutAttribArray().set(GA_ATTRIB_PRIMITIVE, GFE_UVScaletoWorldSize_AreaUVAttribName);
        GA_Attribute* areaUVAttrib = measure.findOrCreateTuple(true);
        measure.setPositionAttrib(uvAttrib);
        measure.compute();
        

        const GA_AttributeUPtr uvScaleUPtr = geo->createDetachedTupleAttribute(GA_ATTRIB_PRIMITIVE, uv_h->getStorage(), 3);
        GA_Attribute* const uvScaleAttrib = uvScaleUPtr.get();

        GFE_Connectivity connectivity(geo, cookparms);
        if (computeUVAreaInPiece)
        {
            connectivity.groupParser.setGroup(groupParser.getPrimitiveGroup());
            //connectivity.setComputeParm(connectivityConstraint, true);
            connectivity.findOrCreateTuple(true, GA_ATTRIB_PRIMITIVE);
            connectivity.compute();
            const GA_Attribute* const connectivityAttrib = connectivity.getConnectivityAttrib();
#if 0
            GFE_AttribPromote attribPromote(geo, cookparms);

            attribPromote.setSourceAttribute(areaATIPtr);
            attribPromote.setDestinationAttribute(areaATIPtr);
            attribPromote.compute();
            
            attribPromote.setSourceAttribute(GA_ATTRIB_PRIMITIVE, sopparms.getSrcAttrib());
            attribPromote.setDestinationAttribute(dstAttribClass);
            attribPromote.compute();
#else
            areaAttrib   = GU_Promote::promote(*geo->asGU_Detail(), areaAttrib,   GA_ATTRIB_PRIMITIVE, true, GU_Promote::GU_PROMOTE_SUM, nullptr, connectivityAttrib);
            areaUVAttrib = GU_Promote::promote(*geo->asGU_Detail(), areaUVAttrib, GA_ATTRIB_PRIMITIVE, true, GU_Promote::GU_PROMOTE_SUM, nullptr, connectivityAttrib);
#endif
        }


        const _Ty uvScaleTmp = getUVScale<_Ty>();
        
        UTparallelFor(groupParser.getPrimitiveSplittableRange(), [this, uvScaleAttrib, areaAttrib, areaUVAttrib, &uvScaleTmp](const GA_SplittableRange& r)
        {
            GFE_RWPageHandleT<_Ty> uvScale_ph(uvScaleAttrib);
            GFE_ROPageHandleT<value_type> area_ph(areaAttrib);
            GFE_ROPageHandleT<value_type> areaUV_ph(areaUVAttrib);
            _Ty uvS;
            GA_Offset start, end;
            for (GA_PageIterator pit = r.beginPages(); !pit.atEnd(); ++pit)
            {
                for (GA_Iterator it(pit.begin()); it.blockAdvance(start, end); )
                {
                    uvScale_ph.setPage(start);
                    area_ph.setPage(start);
                    areaUV_ph.setPage(start);
                    for (GA_Offset elemoff = start; elemoff < end; ++elemoff)
                    {
                        uvS = std::sqrt(area_ph.value(elemoff) / areaUV_ph.value(elemoff)) * uvScaleTmp;
                        uvS[0] = doUVScalex ? uvS[0] : value_type(1);
                        uvS[1] = doUVScaley ? uvS[1] : value_type(1);
                        uvS[2] = doUVScalez ? uvS[2] : value_type(1);
                        uvScale_ph.value(elemoff) = uvS;
                    }
                }
            }
        }, subscribeRatio, minGrainSize);



        auto isPointAttribVariant = GFE_Variant::getBoolVariant(uvAttrib->getOwner() == GA_ATTRIB_POINT);
        std::visit([&] (auto isPointAttribVariant)
        {
            scaleUV<_Ty, isPointAttribVariant>(uvScaleAttrib);
        }, isPointAttribVariant);

    }

    template<typename _Ty, bool isPointAttrib>
    SYS_FORCE_INLINE void scaleUV(const GA_Attribute* const uvScaleAttrib)
    {
        const GA_ROHandleT<_Ty> uvScaleAttrib_h(uvScaleAttrib);
        UTparallelFor(groupParser.getSplittableRange<isPointAttrib ? GA_ATTRIB_POINT : GA_ATTRIB_VERTEX>(), [this, &uvScaleAttrib_h](const GA_SplittableRange& r)
        {
            GFE_RWPageHandleT<_Ty> uv_ph(uvAttrib);
            GA_Offset primoff;
            GA_Offset start, end;
            for (GA_PageIterator pit = r.beginPages(); !pit.atEnd(); ++pit)
            {
                for (GA_Iterator it(pit.begin()); it.blockAdvance(start, end); )
                {
                    uv_ph.setPage(start);
                    for (GA_Offset elemoff = start; elemoff < end; ++elemoff)
                    {
                        if constexpr (isPointAttrib)
                            primoff = geo->vertexPrimitive(geo->pointVertex(elemoff));
                        else
                            primoff = geo->vertexPrimitive(elemoff);
                        uv_ph.value(elemoff) *= uvScaleAttrib_h.get(primoff);
                    }
                }
            }
        }, subscribeRatio, minGrainSize);
    }
    
    template<typename _Ty>
    SYS_FORCE_INLINE _Ty getUVScale() const
    {
        if constexpr(_Ty::tuple_size == 2)
            return _Ty(uvScale[0], uvScale[1]);
        else if constexpr(_Ty::tuple_size == 3)
            return _Ty(uvScale[0], uvScale[1], uvScale[2]);
        else
            return uvScale;
    }


public:
    bool computeUVAreaInPiece = true;
    UT_Vector4D uvScale = UT_Vector4D(1.0, 1.0, 1.0, 1.0);
    bool doUVScalex = true;
    bool doUVScaley = true;
    bool doUVScalez = true;

private:
    GA_Attribute* uvAttrib;
    
    exint subscribeRatio = 64;
    exint minGrainSize   = 1024;
}; // End of Class UVScaletoWorldSize
_GFEL_END
#endif
