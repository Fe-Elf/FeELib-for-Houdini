
#pragma once

#ifndef __GFE_MatchBBox_h__
#define __GFE_MatchBBox_h__

#include "GFE/GFE_MatchBBox.h"

#include "GFE/GFE_GeoFilter.h"



#include "UT/UT_BoundingBox.h"

#include "GFE/GFE_Bound.h"
#include "GFE/GFE_Math.h"
//#include "GFE/GFE_XformByAttrib.h"

template<typename _TScalar = fpreal>
class GFE_MatchBBoxXform {

public:
    
    GFE_MatchBBoxXform()
    {
    }
    
    ~GFE_MatchBBoxXform()
    {
    }
    
    
    
    SYS_FORCE_INLINE void setRestBBox(const GA_Detail& geo, const GA_Range& pointRange, const GA_Attribute* posAttrib = nullptr)
    { restBBox = GFE_Bound::getBBox<fpreal>(geo, pointRange, posAttrib); }
    
    SYS_FORCE_INLINE void setRefBBox(const GA_Detail& geo, const GA_Range& pointRange, const GA_Attribute* posAttrib = nullptr)
    { refBBox = GFE_Bound::getBBox<fpreal>(geo, pointRange, posAttrib); }
    
    SYS_FORCE_INLINE void setUnitRefBBox()
    { refBBox = GFE_Bound::getUnitBBox<fpreal>(); }
    
    //SYS_FORCE_INLINE UT_BoundingBoxT<fpreal> getRestBBox() const
    //{ return restBBox; }
    //SYS_FORCE_INLINE UT_BoundingBoxT<fpreal> getRefBBox()  const
    //{ return refBBox; }
    
    SYS_FORCE_INLINE UT_Vector3T<_TScalar> computeTranslate(const UT_Vector3T<_TScalar>& biasRest, const UT_Vector3T<_TScalar>& biasRef) const
    {
        return restBBox.maxvec() * (biasRest * 0.5 - 0.5) - restBBox.minvec() * (biasRest * 0.5 + 0.5)
             + refBBox .maxvec() * (biasRef  * 0.5 + 0.5) + refBBox .minvec() * (-biasRef * 0.5 + 0.5);
    }
    
    SYS_FORCE_INLINE UT_Vector3T<_TScalar> computeTranslate(const UT_Vector3T<_TScalar>& bias) const
    { return computeTranslate(bias, bias); }
    
    
    
    UT_Vector3T<_TScalar> computeScale(const UT_Vector3T<_TScalar>& bias, const GFE_ScaleAxis scaleAxis) const
    {
        UT_Vector3T<_TScalar> s = refBBox.size() / restBBox.size();
        if (scaleAxis != GFE_ScaleAxis::Invalid)
        {
            int8 idx;
            switch (scaleAxis)
            {
            case GFE_ScaleAxis::X        : idx = 0;                             break;
            case GFE_ScaleAxis::Y        : idx = 1;                             break;
            case GFE_ScaleAxis::Z        : idx = 2;                             break;
            case GFE_ScaleAxis::XYZMin   : idx = GFE_Math::minidx(bias);        break;
            case GFE_ScaleAxis::XYZMiddle: idx = GFE_Math::mididx(bias);        break;
            case GFE_ScaleAxis::XYZMax   : idx = GFE_Math::maxidx(bias);        break;
            case GFE_ScaleAxis::XYMin    : idx = bias[0] <= bias[1] ? 0 : 1;    break;
            case GFE_ScaleAxis::YZMin    : idx = bias[1] <= bias[2] ? 1 : 2;    break;
            case GFE_ScaleAxis::ZXMin    : idx = bias[2] <= bias[0] ? 2 : 0;    break;
            case GFE_ScaleAxis::XYMax    : idx = bias[0] >= bias[1] ? 0 : 1;    break;
            case GFE_ScaleAxis::YZMax    : idx = bias[1] >= bias[2] ? 1 : 2;    break;
            case GFE_ScaleAxis::ZXMax    : idx = bias[2] >= bias[0] ? 2 : 0;    break;
            default: UT_ASSERT_MSG(0, "Unhandled Scale Axis!"); return {1,1,1}; break;
            }
            s = s[idx];
        }
        s = SYSlerp(UT_Vector3T<_TScalar>(1,1,1), s, bias);
        return s;
    }
    
    SYS_FORCE_INLINE UT_Vector3T<_TScalar> computeCenter() const
    { return restBBox.center(); }
    
    
    void computeScaleTranslate(
        UT_Vector3T<_TScalar>& c,
        UT_Vector3T<_TScalar>& s,
        UT_Vector3T<_TScalar>& t,
        const UT_Vector3T<_TScalar>& sBias,
        const GFE_ScaleAxis scaleAxis,
        const UT_Vector3T<_TScalar>& tBiasRest,
        const UT_Vector3T<_TScalar>& tBiasRef
    ) const
    {
        c = computeCenter();
        s = computeScale(sBias, scaleAxis);
        
        UT_BoundingBoxT<_TScalar> restBBoxScaled(restBBox);
        restBBoxScaled.translate(-c);
        GFE_Bound::scale(restBBoxScaled, s);
        restBBoxScaled.translate(c);
        
        t = restBBoxScaled.maxvec() * (tBiasRest * 0.5 - 0.5) - restBBoxScaled.minvec() * (tBiasRest * 0.5 + 0.5)
          + refBBox       .maxvec() * (tBiasRef  * 0.5 + 0.5) + refBBox       .minvec() * (-tBiasRef * 0.5 + 0.5);
    }
    

    
    UT_Matrix4T<_TScalar> computeXform(
        const UT_Vector3T<_TScalar>& biasRestT, const UT_Vector3T<_TScalar>& biasRefT,
        const UT_Vector3T<_TScalar>& biasRestS, const UT_Vector3T<_TScalar>& biasRefS
    ) const
    {
        UT_Vector3T<_TScalar> c = computeTranslate(biasRestT, biasRefT);
        UT_Vector3T<_TScalar> t = computeTranslate(biasRestT, biasRefT);
        UT_Vector3T<_TScalar> s = computeTranslate(biasRestS, biasRefS);
        UT_Matrix4T<_TScalar> xform;
        xform.translate(-c);
        xform.scale(s);
        xform.translate(c + t);
        return xform;
    }
    
public:
    UT_BoundingBoxT<_TScalar> restBBox;
    UT_BoundingBoxT<_TScalar> refBBox;
    //UT_Vector3T<_TScalar> t;
    //UT_Vector3T<_TScalar> s;
    //UT_Matrix4T<_TScalar> xform;
    
}; // End of class GFE_MatchBBoxXform











class GFE_MatchBBox : public GFE_AttribFilterWithRef1
{
    
public:
    using xform_value_type = fpreal32;
    
    using GFE_AttribFilterWithRef1::GFE_AttribFilterWithRef1;
    

    void
        setComputeParm(
            const exint subscribeRatio = 64,
            const exint minGrainSize   = 1024
        )
    {
        setHasComputed();
        this->subscribeRatio = subscribeRatio;
        this->minGrainSize   = minGrainSize;
    }
    
    //SYS_FORCE_INLINE void setRepairPrecision()
    //{ repairPrecision = false; }
    //
    //SYS_FORCE_INLINE void setRepairPrecision(const fpreal threshold)
    //{ repairPrecision = true; repairPrecisionThreshold = threshold; }

    
    SYS_FORCE_INLINE GA_Attribute* setXformAttrib(GA_Attribute* const inAttrib = nullptr)
    { return xformAttrib = !inAttrib || inAttrib->getTupleSize() != 16 || inAttrib->getStorageClass() != GA_STORECLASS_REAL || !inAttrib->getAIFTuple() ? nullptr : inAttrib; }
    
    SYS_FORCE_INLINE GA_Attribute* setXformAttrib(const bool detached = false, const GA_Storage storage = GA_STORE_INVALID, const UT_StringRef& attribName = "xform")
    { return getOutAttribArray().findOrCreateTuple(detached, GA_ATTRIB_DETAIL, GA_STORECLASS_FLOAT, storage, attribName, 16, GA_Defaults(0.0f), false); }
    
    
    
private:

    virtual bool
        computeCore() override
    {
        if (getOutAttribArray().isEmpty())
            return false;
        
        if (!doTranslate && !doScale)
            return true;

        UT_ASSERT_MSG_P(!xformAttrib ||
                       (xformAttrib->getTupleSize() == 16 &&
                        xformAttrib->getStorageClass() == GA_STORECLASS_REAL &&
                        xformAttrib->getAIFTuple()), "not correct xform attrib type");
        
        if (groupParser.isEmpty())
            return true;

        setValidPosAttrib();
        setValidPosRef0Attrib();
        setValidPosRef1Attrib();
        

        UT_ASSERT_P(GFE_Type::checkTupleAttrib(posAttrib, GA_STORECLASS_FLOAT, GA_STORE_INVALID, 3));
        UT_ASSERT_P(!posRef0Attrib || GFE_Type::checkTupleAttrib(posRef0Attrib, GA_STORECLASS_FLOAT, GA_STORE_INVALID, 3));
        UT_ASSERT_P(!posRef1Attrib || GFE_Type::checkTupleAttrib(posRef1Attrib, GA_STORECLASS_FLOAT, GA_STORE_INVALID, 3));

        bboxXform.setRestBBox(geoRef1 ? *geoRef1 : *geo, usePrimBounding ? groupParser.getPrimitiveRange() : groupParser.getPointRange(), geoRef1 ? posRef1Attrib : posAttrib);
        //bboxXform.setRefBBox(geoRef0 ? *geoRef0 : *geo, (geoRef0 ? groupParserRef0 : groupParser).getPointRange(), geoRef0 ? posRef0Attrib : posAttrib);
        if (geoRef0)
            bboxXform.setRefBBox(*geoRef0, usePrimBounding ? groupParserRef0.getPrimitiveRange() : groupParserRef0.getPointRange(), posRef0Attrib);
        else
            bboxXform.setUnitRefBBox();
        
        if (doTranslate && doScale)
        {
            bboxXform.computeScaleTranslate(c, s, t, sBias, scaleAxis, tBiasRest, tBiasRef);
            t += c + tPost;
            s *= sPost;
        }
        else if (doTranslate)
        {
            t = bboxXform.computeTranslate(tBiasRest, tBiasRef);
            t += tPost;
        }
        else
        {
            UT_ASSERT_P(doScale);
            s = bboxXform.computeScale(sBias, scaleAxis);
            s *= sPost;
        }
        /*
        if (repairPrecision)
        {
            for (int8 i = 0; i < 6; ++i)
            {
                if (SYSabs(bboxXform.restBBox.myFloats[i]) < repairPrecisionThreshold)
                    bboxXform.restBBox.myFloats[i] = 0;
            }
            translate(xform, -(bboxmin0 + bboxmax0));
        }
        */
        const size_t sizeAttrib = getOutAttribArray().size();
        for (size_t i = 0; i < sizeAttrib; ++i)
        {
            attrib = getOutAttribArray()[i];
            if (attrib->getTupleSize() != 3)
                continue;
            
            const GA_AIFTuple* const aifTuple = attrib->getAIFTuple();
            if (aifTuple)
            {
                switch (aifTuple->getStorage(attrib))
                {
                //case GA_STORE_INT8:   matchBBox<int8>();     break;
                //case GA_STORE_INT16:  matchBBox<int16>();    break;
                case GA_STORE_INT32:  matchBBox<int32>();    break;
                case GA_STORE_INT64:  matchBBox<int64>();    break;
                case GA_STORE_REAL16: matchBBox<fpreal16>(); break;
                case GA_STORE_REAL32: matchBBox<fpreal32>(); break;
                case GA_STORE_REAL64: matchBBox<fpreal64>(); break;
                default: UT_ASSERT_MSG(0, "Unhandled Attrib Storage"); break;
                }
            }
            else
            {
                const GA_AIFNumericArray* const aifNumArray = attrib->getAIFNumericArray();
                if (aifNumArray)
                {
                    switch (aifNumArray->getStorage(attrib))
                    {
                    case GA_STORE_INT8:      break;
                    case GA_STORE_INT16:     break;
                    case GA_STORE_INT32:     break;
                    case GA_STORE_INT64:     break;
                    case GA_STORE_REAL16:    break;
                    case GA_STORE_REAL32:    break;
                    case GA_STORE_REAL64:    break;
                    default: UT_ASSERT_MSG(0, "Unhandled Attrib Storage"); break;
                    }
                }
            }
        }
        
        return true;
    }

    template<typename _TScalar>
    void matchBBox()
    {
        if (doTranslate && doScale)
        {
            UT_Vector3T<_TScalar> t = this->t;
            UT_Vector3T<_TScalar> c = this->c;
            UT_Vector3T<_TScalar> s = this->s;
            
            UTparallelFor(groupParser.getSplittableRange(attrib), [this, &c, &s, &t](const GA_SplittableRange& r)
            {
                GA_PageHandleT<UT_Vector3T<_TScalar>, _TScalar, true, true, GA_Attribute, GA_ATINumeric, GA_Detail> attrib_ph(attrib);
                GA_Offset start, end;
                for (GA_PageIterator pit = r.beginPages(); !pit.atEnd(); ++pit)
                {
                    for (GA_Iterator it(pit.begin()); it.blockAdvance(start, end); )
                    {
                        attrib_ph.setPage(start);
                        for (GA_Offset elemoff = start; elemoff < end; ++elemoff)
                        {
                            attrib_ph.value(elemoff) = (attrib_ph.value(elemoff) - c) * s + t;
                        }
                    }
                }
            }, subscribeRatio, minGrainSize);
        }
        else if (doTranslate)
        {
            UT_Vector3T<_TScalar> t = this->t;
            
            UTparallelFor(groupParser.getSplittableRange(attrib), [this, &t](const GA_SplittableRange& r)
            {
                GA_PageHandleT<UT_Vector3T<_TScalar>, _TScalar, true, true, GA_Attribute, GA_ATINumeric, GA_Detail> attrib_ph(attrib);
                GA_Offset start, end;
                for (GA_PageIterator pit = r.beginPages(); !pit.atEnd(); ++pit)
                {
                    for (GA_Iterator it(pit.begin()); it.blockAdvance(start, end); )
                    {
                        attrib_ph.setPage(start);
                        for (GA_Offset elemoff = start; elemoff < end; ++elemoff)
                        {
                            attrib_ph.value(elemoff) += t;
                        }
                    }
                }
            }, subscribeRatio, minGrainSize);
        }
        else
        {
            UT_ASSERT_P(doScale);
                
            UT_Vector3T<_TScalar> c = this->c;
            UT_Vector3T<_TScalar> s = this->s;
            
            UTparallelFor(groupParser.getSplittableRange(attrib), [this, &c, &s](const GA_SplittableRange& r)
            {
                GA_PageHandleT<UT_Vector3T<_TScalar>, _TScalar, true, true, GA_Attribute, GA_ATINumeric, GA_Detail> attrib_ph(attrib);
                GA_Offset start, end;
                for (GA_PageIterator pit = r.beginPages(); !pit.atEnd(); ++pit)
                {
                    for (GA_Iterator it(pit.begin()); it.blockAdvance(start, end); )
                    {
                        attrib_ph.setPage(start);
                        for (GA_Offset elemoff = start; elemoff < end; ++elemoff)
                        {
                            attrib_ph.value(elemoff) = (attrib_ph.value(elemoff)-c) * s + c;
                        }
                    }
                }
            }, subscribeRatio, minGrainSize);
        }
    }

public:
    bool doTranslate = false;
    bool doScale = false;
    bool usePrimBounding = false;
    GFE_ScaleAxis scaleAxis = GFE_ScaleAxis::Invalid;
    
    UT_Vector3T<fpreal> sBias     = {1,1,1};
    UT_Vector3T<fpreal> tBiasRest = {1,1,1};
    UT_Vector3T<fpreal> tBiasRef  = {1,1,1};
    
    UT_Vector3T<xform_value_type> tPost = {0,0,0};
    UT_Vector3T<xform_value_type> sPost = {1,1,1};
    
//private:
//    bool repairPrecision = true;
//    fpreal repairPrecisionThreshold = 1e-05;

    
private:
    GA_Attribute* xformAttrib = nullptr;
    
    exint subscribeRatio = 64;
    exint minGrainSize   = 1024;

private:
    GFE_MatchBBoxXform<xform_value_type> bboxXform;
    UT_Vector3T<xform_value_type> t;
    UT_Vector3T<xform_value_type> c;
    UT_Vector3T<xform_value_type> s;
    
    GA_Attribute* attrib;
    //const GFE_Detail* geoRest;
    //UT_Matrix4T<fpreal> xform;

}; // End of class GFE_MatchBBox


#endif
