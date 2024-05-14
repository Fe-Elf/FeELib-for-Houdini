
#pragma once

#ifndef __GFE_XformByAttrib_h__
#define __GFE_XformByAttrib_h__

#include "GFE/GFE_XformByAttrib.h"

#include "GFE/GFE_GeoFilter.h"




#include "GA/GA_AttributeTransformer.h"



class GFE_XformByAttrib : public GFE_AttribFilterWithRef0
{
public:
    using GFE_AttribFilterWithRef0::GFE_AttribFilterWithRef0;
    
    void
        setComputeParm(
            const bool preserveLength  = false,
            const bool invertXform     = false,
            const bool delXformAttrib  = true,
            const exint subscribeRatio = 64,
            const exint minGrainSize   = 1024
        )
    {
        setHasComputed();
        this->preserveLength = preserveLength;
        this->invertXform    = invertXform;
        this->delXformAttrib = delXformAttrib;
        this->subscribeRatio = subscribeRatio;
        this->minGrainSize   = minGrainSize;
    }

    template<typename _TScalar>
    SYS_FORCE_INLINE void setXform4(const UT_Matrix4T<_TScalar>& xform)
    { xform4 = xform; xformByXform4 = true; }
    
    template<typename _TScalar>
    SYS_FORCE_INLINE void setXform4(UT_Matrix4T<_TScalar>&& xform)
    { xform4 = xform; xformByXform4 = true; }


    
    template<typename _TScalar>
    SYS_FORCE_INLINE void setXform3(const UT_Matrix4T<_TScalar>& xform)
    { xform3 = xform; xformByXform4 = false; }
    
    template<typename _TScalar>
    SYS_FORCE_INLINE void setXform3(const UT_Matrix3T<_TScalar>& xform)
    { xform3 = xform; xformByXform4 = false; }
    
    template<typename _TScalar>
    SYS_FORCE_INLINE void setXform3(UT_Matrix3T<_TScalar>&& xform)
    { xform3 = xform; xformByXform4 = false; }


    
    void setXformAttrib(GA_Attribute* const attrib)
    {
        xformAttrib = attrib;
        xformAttribNonConst = attrib;
        checkXformAttrib();
    }

    void setXformAttrib(const GA_Attribute* const attrib = nullptr)
    {
        xformAttrib = attrib;
        xformAttribNonConst = nullptr;
        checkXformAttrib();
    }

    void setXformAttrib(const GA_AttributeOwner owner, const UT_StringRef& name)
    {
        if(geoRef0)
        {
            xformAttribNonConst = nullptr;
            xformAttrib = geoRef0->findAttribute(owner, name);
        }
        else
        {
            xformAttribNonConst = geo->findAttribute(owner, name);
            xformAttrib = xformAttribNonConst;
        }
        checkXformAttrib();
    }

    
private:

    virtual bool
        computeCore() override
    {
        if (getOutAttribArray().isEmpty())
            return false;
        
        if (geo->getNumPoints()==0)
            return true;
        
        if (groupParser.isEmpty())
            return true;

        const size_t size = getOutAttribArray().size();
        for (size_t i = 0; i < size; ++i)
        {
            attribPtr = getOutAttribArray()[i];
            const GA_AIFTuple* const aifTuple = attribPtr->getAIFTuple();
            if (!aifTuple)
                continue;
            
            const GA_Storage storage = aifTuple->getStorage(attribPtr);
            // const GA_Storage storage = attribPtr->getAIFTuple()->getStorage(attribPtr);
            switch (aifTuple->getTupleSize(attribPtr))
            {
            // case 2:
            //     switch (storage)
            //     {
            //     case GA_STORE_INT16:  xform<UT_Vector2T<int16>>();           break;
            //     case GA_STORE_INT32:  xform<UT_Vector2T<int32>>();           break;
            //     case GA_STORE_INT64:  xform<UT_Vector2T<int64>>();           break;
            //     case GA_STORE_REAL16: xform<UT_Vector2T<fpreal16>>();        break;
            //     case GA_STORE_REAL32: xform<UT_Vector2T<fpreal32>>();        break;
            //     case GA_STORE_REAL64: xform<UT_Vector2T<fpreal64>>();        break;
            //     default: break;
            //     }
            //     break;
            case 3:
                switch (storage)
                {
                //case GA_STORE_INT16:  xform<UT_Vector3T<int16>>();           break;
                //case GA_STORE_INT32:  xform<UT_Vector3T<int32>>();           break;
                //case GA_STORE_INT64:  xform<UT_Vector3T<int64>>();           break;
                //case GA_STORE_REAL16: xform<UT_Vector3T<fpreal16>>();        break;
                case GA_STORE_REAL32: xform<UT_Vector3T<fpreal32>>();        break;
                case GA_STORE_REAL64: xform<UT_Vector3T<fpreal64>>();        break;
                default: break;
                }
            break;
            // case 4:
            //     switch (storage)
            //     {
            //     case GA_STORE_INT16:  xform<UT_Vector4T<int16>>();           break;
            //     case GA_STORE_INT32:  xform<UT_Vector4T<int32>>();           break;
            //     case GA_STORE_INT64:  xform<UT_Vector4T<int64>>();           break;
            //     case GA_STORE_REAL16: xform<UT_Vector4T<fpreal16>>();        break;
            //     case GA_STORE_REAL32: xform<UT_Vector4T<fpreal32>>();        break;
            //     case GA_STORE_REAL64: xform<UT_Vector4T<fpreal64>>();        break;
            //     default: break;
            //     }
            //break;
            case 9:
                switch (storage)
                {
                //case GA_STORE_INT16:  xform<UT_Matrix3T<int16>>();           break;
                //case GA_STORE_INT32:  xform<UT_Matrix3T<int32>>();           break;
                //case GA_STORE_INT64:  xform<UT_Matrix3T<int64>>();           break;
                //case GA_STORE_REAL16: xform<UT_Matrix3T<fpreal16>>();        break;
                case GA_STORE_REAL32: xform<UT_Matrix3T<fpreal32>>();        break;
                case GA_STORE_REAL64: xform<UT_Matrix3T<fpreal64>>();        break;
                default: break;
                }
            break;
            case 16:
                switch (storage)
                {
                //case GA_STORE_INT16:  xform<UT_Matrix4T<int16>>();           break;
                //case GA_STORE_INT32:  xform<UT_Matrix4T<int32>>();           break;
                //case GA_STORE_INT64:  xform<UT_Matrix4T<int64>>();           break;
                //case GA_STORE_REAL16: xform<UT_Matrix4T<fpreal16>>();        break;
                case GA_STORE_REAL32: xform<UT_Matrix4T<fpreal32>>();        break;
                case GA_STORE_REAL64: xform<UT_Matrix4T<fpreal64>>();        break;
                default: break;
                }
            break;
            default: break;
            }
        }
        
        if (delXformAttrib && xformAttribNonConst)
            geo->destroyAttrib(xformAttribNonConst);

        return true;
    }






    template <GA_AttributeOwner FROM, GA_AttributeOwner TO, typename _TMatrix, typename MATRIX_T>
    void xformMatrix3ByAttrib()
    {
        const GA_ROHandleT<MATRIX_T> xform_h(xformAttrib);
        if (&xformAttrib->getDetail() == geo)
        {
            UTparallelFor(groupParser.getSplittableRange(attribPtr), [this, &xform_h](const GA_SplittableRange& r)
            {
                GA_PageHandleT<_TMatrix, typename _TMatrix::value_type, true, true, GA_Attribute, GA_ATINumeric, GA_Detail> attrib_ph(attribPtr);
                GA_Offset start, end;
                for (GA_PageIterator pit = r.beginPages(); !pit.atEnd(); ++pit)
                {
                    for (GA_Iterator it(pit.begin()); it.blockAdvance(start, end); )
                    {
                        attrib_ph.setPage(start);
                        for (GA_Offset elemoff = start; elemoff < end; ++elemoff)
                        {
                            const GA_Offset elemoff_ref = geo->offsetPromote<FROM, TO>(elemoff);
                            if (invertXform)
                            {
                                //UT_Matrix4D xform = xform_h.get(elemoff_ref);
                                //xform *=
                                UT_Matrix3T<typename _TMatrix::value_type> xform(xform_h.get(elemoff_ref));
                                xform.invert();
                                attrib_ph.value(elemoff) *= xform;
                            }
                            else
                            {
                                attrib_ph.value(elemoff) *= UT_Matrix3T<typename _TMatrix::value_type>(xform_h.get(elemoff_ref));
                            }
                        }
                    }
                }
            }, subscribeRatio, minGrainSize);
        }
        else
        {
            const GA_IndexMap& indexMap = xformAttrib->getIndexMap();
            UTparallelFor(groupParser.getSplittableRange(attribPtr), [this, &xform_h, &indexMap](const GA_SplittableRange& r)
            {
                GA_PageHandleT<_TMatrix, typename _TMatrix::value_type, true, true, GA_Attribute, GA_ATINumeric, GA_Detail> attrib_ph(attribPtr);
                GA_Offset start, end;
                for (GA_PageIterator pit = r.beginPages(); !pit.atEnd(); ++pit)
                {
                    for (GA_Iterator it(pit.begin()); it.blockAdvance(start, end); )
                    {
                        attrib_ph.setPage(start);
                        for (GA_Offset elemoff = start; elemoff < end; ++elemoff)
                        {
                            const GA_Offset elemoff_ref = geo->offsetPromote<FROM, TO>(elemoff);
                            
                            if (!indexMap.isOffsetInRange(elemoff_ref))
                                continue;
                            
                            if (invertXform)
                            {
                                UT_Matrix3T<typename _TMatrix::value_type> xform(xform_h.get(elemoff_ref));
                                xform.invert();
                                attrib_ph.value(elemoff) *= xform;
                            }
                            else
                            {
                                attrib_ph.value(elemoff) *= UT_Matrix3T<typename _TMatrix::value_type>(xform_h.get(elemoff_ref));
                            }
                        }
                    }
                }
            }, subscribeRatio, minGrainSize);
        }
    }

    
    template <GA_AttributeOwner FROM, GA_AttributeOwner TO, typename _TMatrix, typename MATRIX_T>
    void xformMatrixByAttrib()
    {
        const GA_ROHandleT<MATRIX_T> xform_h(xformAttrib);
        if (&xformAttrib->getDetail() == geo)
        {
            UTparallelFor(groupParser.getSplittableRange(attribPtr), [this, &xform_h](const GA_SplittableRange& r)
            {
                GA_PageHandleT<_TMatrix, typename _TMatrix::value_type, true, true, GA_Attribute, GA_ATINumeric, GA_Detail> attrib_ph(attribPtr);
                GA_Offset start, end;
                for (GA_PageIterator pit = r.beginPages(); !pit.atEnd(); ++pit)
                {
                    for (GA_Iterator it(pit.begin()); it.blockAdvance(start, end); )
                    {
                        attrib_ph.setPage(start);
                        for (GA_Offset elemoff = start; elemoff < end; ++elemoff)
                        {
                            const GA_Offset elemoff_ref = geo->offsetPromote<FROM, TO>(elemoff);
                            if (invertXform)
                            {
                                //UT_Matrix4D xform = xform_h.get(elemoff_ref);
                                //xform *=
                                MATRIX_T xform = xform_h.get(elemoff_ref);
                                xform.invert();
                                attrib_ph.value(elemoff) *= xform;
                            }
                            else
                            {
                                attrib_ph.value(elemoff) *= xform_h.get(elemoff_ref);
                            }
                        }
                    }
                }
            }, subscribeRatio, minGrainSize);
        }
        else
        {
            const GA_IndexMap& indexMap = xformAttrib->getIndexMap();
            UTparallelFor(groupParser.getSplittableRange(attribPtr), [this, &xform_h, &indexMap](const GA_SplittableRange& r)
            {
                GA_PageHandleT<_TMatrix, typename _TMatrix::value_type, true, true, GA_Attribute, GA_ATINumeric, GA_Detail> attrib_ph(attribPtr);
                GA_Offset start, end;
                for (GA_PageIterator pit = r.beginPages(); !pit.atEnd(); ++pit)
                {
                    for (GA_Iterator it(pit.begin()); it.blockAdvance(start, end); )
                    {
                        attrib_ph.setPage(start);
                        for (GA_Offset elemoff = start; elemoff < end; ++elemoff)
                        {
                            const GA_Offset elemoff_ref = geo->offsetPromote<FROM, TO>(elemoff);
                            
                            if (!indexMap.isOffsetInRange(elemoff_ref))
                                continue;
                            
                            if (invertXform)
                            {
                                MATRIX_T xform = xform_h.get(elemoff_ref);
                                xform.invert();
                                attrib_ph.value(elemoff) *= xform;
                            }
                            else
                            {
                                attrib_ph.value(elemoff) *= xform_h.get(elemoff_ref);
                            }
                        }
                    }
                }
            }, subscribeRatio, minGrainSize);
        }
    }


    template <GA_AttributeOwner FROM, GA_AttributeOwner TO, typename _TMatrix, typename MATRIX_T>
    void xformVectorByAttrib()
    {
        const GA_ROHandleT<MATRIX_T> xform_h(xformAttrib);
        if (&xformAttrib->getDetail() == geo)
        {
            UTparallelFor(groupParser.getSplittableRange(attribPtr), [this, &xform_h](const GA_SplittableRange& r)
            {
                GA_PageHandleT<_TMatrix, typename _TMatrix::value_type, true, true, GA_Attribute, GA_ATINumeric, GA_Detail> attrib_ph(attribPtr);
                GA_Offset start, end;
                for (GA_PageIterator pit = r.beginPages(); !pit.atEnd(); ++pit)
                {
                    for (GA_Iterator it(pit.begin()); it.blockAdvance(start, end); )
                    {
                        attrib_ph.setPage(start);
                        for (GA_Offset elemoff = start; elemoff < end; ++elemoff)
                        {
                            const GA_Offset elemoff_ref = geo->offsetPromote<FROM, TO>(elemoff);
                            if (invertXform)
                            {
                                //UT_Matrix4D xform = xform_h.get(elemoff_ref);
                                //xform *=
                                MATRIX_T xform = xform_h.get(elemoff_ref);
                                xform.invert();
                                if (preserveLength)
                                {
                                    const typename _TMatrix::value_type length = attrib_ph.value(elemoff).length();
                                    attrib_ph.value(elemoff) *= xform;
                                    attrib_ph.value(elemoff).normalize();
                                    attrib_ph.value(elemoff) *= length;
                                }
                                else
                                {
                                    attrib_ph.value(elemoff) *= xform;
                                }
                            }
                            else
                            {
                                if (preserveLength)
                                {
                                    const typename _TMatrix::value_type length = attrib_ph.value(elemoff).length();
                                    attrib_ph.value(elemoff) *= xform_h.get(elemoff_ref);
                                    attrib_ph.value(elemoff).normalize();
                                    attrib_ph.value(elemoff) *= length;
                                }
                                else
                                {
                                    attrib_ph.value(elemoff) *= xform_h.get(elemoff_ref);
                                }
                            }
                        }
                    }
                }
            }, subscribeRatio, minGrainSize);
        }
        else
        {
            const GA_IndexMap& indexMap = xformAttrib->getIndexMap();
            UTparallelFor(groupParser.getSplittableRange(attribPtr), [this, &xform_h, &indexMap](const GA_SplittableRange& r)
            {
                GA_PageHandleT<_TMatrix, typename _TMatrix::value_type, true, true, GA_Attribute, GA_ATINumeric, GA_Detail> attrib_ph(attribPtr);
                GA_Offset start, end;
                for (GA_PageIterator pit = r.beginPages(); !pit.atEnd(); ++pit)
                {
                    for (GA_Iterator it(pit.begin()); it.blockAdvance(start, end); )
                    {
                        attrib_ph.setPage(start);
                        for (GA_Offset elemoff = start; elemoff < end; ++elemoff)
                        {
                            const GA_Offset elemoff_ref = geo->offsetPromote<FROM, TO>(elemoff);
                            
                            if (!indexMap.isOffsetInRange(elemoff_ref))
                                continue;
                            
                            if (invertXform)
                            {
                                MATRIX_T xform = xform_h.get(elemoff_ref);
                                xform.invert();
                                if (preserveLength)
                                {
                                    const typename _TMatrix::value_type length = attrib_ph.value(elemoff).length();
                                    attrib_ph.value(elemoff) *= xform;
                                    attrib_ph.value(elemoff).normalize();
                                    attrib_ph.value(elemoff) *= length;
                                }
                                else
                                {
                                    attrib_ph.value(elemoff) *= xform;
                                }
                            }
                            else
                            {
                                if (preserveLength)
                                {
                                    const typename _TMatrix::value_type length = attrib_ph.value(elemoff).length();
                                    attrib_ph.value(elemoff) *= xform_h.get(elemoff_ref);
                                    attrib_ph.value(elemoff).normalize();
                                    attrib_ph.value(elemoff) *= length;
                                }
                                else
                                {
                                    attrib_ph.value(elemoff) *= xform_h.get(elemoff_ref);
                                }
                            }
                        }
                    }
                }
            }, subscribeRatio, minGrainSize);
        }
    }

    
    template <typename _TMatrix, typename MATRIX_T>
    void xformVectorByAttrib()
    {
        if(attribPtr->getOwner() == xformAttrib->getOwner())
        {
            if(&xformAttrib->getDetail() == geo)
            {
                UTparallelFor(groupParser.getSplittableRange(attribPtr), [this](const GA_SplittableRange& r)
                {
                    GA_PageHandleT<_TMatrix, typename _TMatrix::value_type, true, true, GA_Attribute, GA_ATINumeric, GA_Detail> attrib_ph(attribPtr);
                    GA_PageHandleT<MATRIX_T, typename MATRIX_T::value_type, true, false, const GA_Attribute, const GA_ATINumeric, const GA_Detail> xform_ph(xformAttrib);
                    GA_Offset start, end;
                    for (GA_PageIterator pit = r.beginPages(); !pit.atEnd(); ++pit)
                    {
                        for (GA_Iterator it(pit.begin()); it.blockAdvance(start, end); )
                        {
                            attrib_ph.setPage(start);
                            xform_ph.setPage(start);
                            for (GA_Offset elemoff = start; elemoff < end; ++elemoff)
                            {
                                if (invertXform)
                                {
                                    MATRIX_T xform = xform_ph.value(elemoff);
                                    xform.invert();
                                    if (preserveLength)
                                    {
                                        const typename _TMatrix::value_type length = attrib_ph.value(elemoff).length();
                                        attrib_ph.value(elemoff) *= xform;
                                        attrib_ph.value(elemoff).normalize();
                                        attrib_ph.value(elemoff) *= length;
                                    }
                                    else
                                    {
                                        attrib_ph.value(elemoff) *= xform;
                                    }
                                }
                                else
                                {
                                    if (preserveLength)
                                    {
                                        const typename _TMatrix::value_type length = attrib_ph.value(elemoff).length();
                                        attrib_ph.value(elemoff) *= xform_ph.value(elemoff);
                                        attrib_ph.value(elemoff).normalize();
                                        attrib_ph.value(elemoff) *= length;
                                    }
                                    else
                                    {
                                        attrib_ph.value(elemoff) *= xform_ph.value(elemoff);
                                    }
                                }
                            }
                        }
                    }
                }, subscribeRatio, minGrainSize);
            }
            else
            {
                const GA_IndexMap& indexMap = xformAttrib->getIndexMap();
                const GA_ROHandleT<MATRIX_T> xform_h(xformAttrib);
                UTparallelFor(groupParser.getSplittableRange(attribPtr), [this, &xform_h, &indexMap](const GA_SplittableRange& r)
                {
                    GA_PageHandleT<_TMatrix, typename _TMatrix::value_type, true, true, GA_Attribute, GA_ATINumeric, GA_Detail> attrib_ph(attribPtr);
                    GA_Offset start, end;
                    for (GA_PageIterator pit = r.beginPages(); !pit.atEnd(); ++pit)
                    {
                        for (GA_Iterator it(pit.begin()); it.blockAdvance(start, end); )
                        {
                            attrib_ph.setPage(start);
                            for (GA_Offset elemoff = start; elemoff < end; ++elemoff)
                            {
                                if (!indexMap.isOffsetInRange(elemoff))
                                    continue;
                                if (invertXform)
                                {
                                    MATRIX_T xform = xform_h.get(elemoff);
                                    xform.invert();
                                    if (preserveLength)
                                    {
                                        const typename _TMatrix::value_type length = attrib_ph.value(elemoff).length();
                                        attrib_ph.value(elemoff) *= xform;
                                        attrib_ph.value(elemoff).normalize();
                                        attrib_ph.value(elemoff) *= length;
                                    }
                                    else
                                    {
                                        attrib_ph.value(elemoff) *= xform;
                                    }
                                }
                                else
                                {
                                    if (preserveLength)
                                    {
                                        const typename _TMatrix::value_type length = attrib_ph.value(elemoff).length();
                                        attrib_ph.value(elemoff) *= xform_h.get(elemoff);
                                        attrib_ph.value(elemoff).normalize();
                                        attrib_ph.value(elemoff) *= length;
                                    }
                                    else
                                    {
                                        attrib_ph.value(elemoff) *= xform_h.get(elemoff);
                                    }
                                }
                            }
                        }
                    }
                }, subscribeRatio, minGrainSize);
            }
        }
        else
        {
            switch (attribPtr->getOwner())
            {
            case GA_ATTRIB_PRIMITIVE:
                switch (xformAttrib->getOwner())
                {
                    case GA_ATTRIB_PRIMITIVE: xformVectorByAttrib<GA_ATTRIB_PRIMITIVE, GA_ATTRIB_PRIMITIVE, _TMatrix, MATRIX_T>(); break;
                    case GA_ATTRIB_POINT:     xformVectorByAttrib<GA_ATTRIB_PRIMITIVE, GA_ATTRIB_POINT,     _TMatrix, MATRIX_T>(); break;
                    case GA_ATTRIB_VERTEX:    xformVectorByAttrib<GA_ATTRIB_PRIMITIVE, GA_ATTRIB_VERTEX,    _TMatrix, MATRIX_T>(); break;
                    default: UT_ASSERT_MSG(0, "unsupport owner"); break;
                }
                break;
            case GA_ATTRIB_POINT:
                switch (xformAttrib->getOwner())
                {
                    case GA_ATTRIB_PRIMITIVE: xformVectorByAttrib<GA_ATTRIB_POINT, GA_ATTRIB_PRIMITIVE, _TMatrix, MATRIX_T>(); break;
                    case GA_ATTRIB_POINT:     xformVectorByAttrib<GA_ATTRIB_POINT, GA_ATTRIB_POINT,     _TMatrix, MATRIX_T>(); break;
                    case GA_ATTRIB_VERTEX:    xformVectorByAttrib<GA_ATTRIB_POINT, GA_ATTRIB_VERTEX,    _TMatrix, MATRIX_T>(); break;
                    default: UT_ASSERT_MSG(0, "unsupport owner"); break;
                }
                break;
            case GA_ATTRIB_VERTEX:
                switch (xformAttrib->getOwner())
                {
                    case GA_ATTRIB_PRIMITIVE: xformVectorByAttrib<GA_ATTRIB_VERTEX, GA_ATTRIB_PRIMITIVE, _TMatrix, MATRIX_T>(); break;
                    case GA_ATTRIB_POINT:     xformVectorByAttrib<GA_ATTRIB_VERTEX, GA_ATTRIB_POINT,     _TMatrix, MATRIX_T>(); break;
                    case GA_ATTRIB_VERTEX:    xformVectorByAttrib<GA_ATTRIB_VERTEX, GA_ATTRIB_VERTEX,    _TMatrix, MATRIX_T>(); break;
                    default: UT_ASSERT_MSG(0, "unsupport owner"); break;
                }
                break;
                
            default: UT_ASSERT_MSG(0, "unsupport owner"); break;
            }
        }
    }



    template <typename _TMatrix, typename MATRIX_T>
    void xformMatrix3ByAttrib()
    {
        if(attribPtr->getOwner() == xformAttrib->getOwner())
        {
            if(&xformAttrib->getDetail() == geo)
            {
                UTparallelFor(groupParser.getSplittableRange(attribPtr), [this](const GA_SplittableRange& r)
                {
                    GA_PageHandleT<_TMatrix, typename _TMatrix::value_type, true, true, GA_Attribute, GA_ATINumeric, GA_Detail> attrib_ph(attribPtr);
                    GA_PageHandleT<MATRIX_T, typename MATRIX_T::value_type, true, false, const GA_Attribute, const GA_ATINumeric, const GA_Detail> xform_ph(xformAttrib);
                    GA_Offset start, end;
                    for (GA_PageIterator pit = r.beginPages(); !pit.atEnd(); ++pit)
                    {
                        for (GA_Iterator it(pit.begin()); it.blockAdvance(start, end); )
                        {
                            attrib_ph.setPage(start);
                            xform_ph.setPage(start);
                            for (GA_Offset elemoff = start; elemoff < end; ++elemoff)
                            {
                                if (invertXform)
                                {
                                    UT_Matrix3T<typename _TMatrix::value_type> xform(xform_ph.value(elemoff));
                                    xform.invert();
                                    attrib_ph.value(elemoff) *= xform;
                                }
                                else
                                {
                                    attrib_ph.value(elemoff) *= UT_Matrix3T<typename _TMatrix::value_type>(xform_ph.value(elemoff));
                                }
                            }
                        }
                    }
                }, subscribeRatio, minGrainSize);
            }
            else
            {
                const GA_IndexMap& indexMap = xformAttrib->getIndexMap();
                const GA_ROHandleT<MATRIX_T> xform_h(xformAttrib);
                UTparallelFor(groupParser.getSplittableRange(attribPtr), [this, &xform_h, &indexMap](const GA_SplittableRange& r)
                {
                    GA_PageHandleT<_TMatrix, typename _TMatrix::value_type, true, true, GA_Attribute, GA_ATINumeric, GA_Detail> attrib_ph(attribPtr);
                    GA_Offset start, end;
                    for (GA_PageIterator pit = r.beginPages(); !pit.atEnd(); ++pit)
                    {
                        for (GA_Iterator it(pit.begin()); it.blockAdvance(start, end); )
                        {
                            attrib_ph.setPage(start);
                            for (GA_Offset elemoff = start; elemoff < end; ++elemoff)
                            {
                                if (!indexMap.isOffsetInRange(elemoff))
                                    continue;
                                if (invertXform)
                                {
                                    UT_Matrix3T<typename _TMatrix::value_type> xform(xform_h.get(elemoff));
                                    xform.invert();
                                    attrib_ph.value(elemoff) *= xform;
                                }
                                else
                                {
                                    attrib_ph.value(elemoff) *= UT_Matrix3T<typename _TMatrix::value_type>(xform_h.get(elemoff));
                                }
                            }
                        }
                    }
                }, subscribeRatio, minGrainSize);
            }
        }
        else
        {
            switch (attribPtr->getOwner())
            {
            case GA_ATTRIB_PRIMITIVE:
                switch (xformAttrib->getOwner())
                {
                    case GA_ATTRIB_PRIMITIVE: xformMatrix3ByAttrib<GA_ATTRIB_PRIMITIVE, GA_ATTRIB_PRIMITIVE, _TMatrix, MATRIX_T>(); break;
                    case GA_ATTRIB_POINT:     xformMatrix3ByAttrib<GA_ATTRIB_PRIMITIVE, GA_ATTRIB_POINT,     _TMatrix, MATRIX_T>(); break;
                    case GA_ATTRIB_VERTEX:    xformMatrix3ByAttrib<GA_ATTRIB_PRIMITIVE, GA_ATTRIB_VERTEX,    _TMatrix, MATRIX_T>(); break;
                    default: UT_ASSERT_MSG(0, "unsupport owner"); break;
                }
                break;
            case GA_ATTRIB_POINT:
                switch (xformAttrib->getOwner())
                {
                    case GA_ATTRIB_PRIMITIVE: xformMatrix3ByAttrib<GA_ATTRIB_POINT, GA_ATTRIB_PRIMITIVE, _TMatrix, MATRIX_T>(); break;
                    case GA_ATTRIB_POINT:     xformMatrix3ByAttrib<GA_ATTRIB_POINT, GA_ATTRIB_POINT,     _TMatrix, MATRIX_T>(); break;
                    case GA_ATTRIB_VERTEX:    xformMatrix3ByAttrib<GA_ATTRIB_POINT, GA_ATTRIB_VERTEX,    _TMatrix, MATRIX_T>(); break;
                    default: UT_ASSERT_MSG(0, "unsupport owner"); break;
                }
                break;
            case GA_ATTRIB_VERTEX:
                switch (xformAttrib->getOwner())
                {
                    case GA_ATTRIB_PRIMITIVE: xformMatrix3ByAttrib<GA_ATTRIB_VERTEX, GA_ATTRIB_PRIMITIVE, _TMatrix, MATRIX_T>(); break;
                    case GA_ATTRIB_POINT:     xformMatrix3ByAttrib<GA_ATTRIB_VERTEX, GA_ATTRIB_POINT,     _TMatrix, MATRIX_T>(); break;
                    case GA_ATTRIB_VERTEX:    xformMatrix3ByAttrib<GA_ATTRIB_VERTEX, GA_ATTRIB_VERTEX,    _TMatrix, MATRIX_T>(); break;
                    default: UT_ASSERT_MSG(0, "unsupport owner"); break;
                }
                break;
                
            default: UT_ASSERT_MSG(0, "unsupport owner"); break;
            }
        }
    }



    
    template <typename _TMatrix, typename MATRIX_T>
    void xformMatrixByAttrib()
    {
        if (attribPtr->getOwner() == xformAttrib->getOwner())
        {
            if (&xformAttrib->getDetail() == geo)
            {
                UTparallelFor(groupParser.getSplittableRange(attribPtr), [this](const GA_SplittableRange& r)
                {
                    GFE_RWPageHandleT<_TMatrix> attrib_ph(attribPtr);
                    GFE_ROPageHandleT<_TMatrix> xform_ph(xformAttrib);
                    GA_Offset start, end;
                    for (GA_PageIterator pit = r.beginPages(); !pit.atEnd(); ++pit)
                    {
                        for (GA_Iterator it(pit.begin()); it.blockAdvance(start, end); )
                        {
                            attrib_ph.setPage(start);
                            xform_ph.setPage(start);
                            for (GA_Offset elemoff = start; elemoff < end; ++elemoff)
                            {
                                if (invertXform)
                                {
                                    MATRIX_T xform = xform_ph.value(elemoff);
                                    xform.invert();
                                    attrib_ph.value(elemoff) *= xform;
                                }
                                else
                                {
                                    attrib_ph.value(elemoff) *= xform_ph.value(elemoff);
                                }
                            }
                        }
                    }
                }, subscribeRatio, minGrainSize);
            }
            else
            {
                const GA_IndexMap& indexMap = xformAttrib->getIndexMap();
                const GA_ROHandleT<MATRIX_T> xform_h(xformAttrib);
                UTparallelFor(groupParser.getSplittableRange(attribPtr), [this, &xform_h, &indexMap](const GA_SplittableRange& r)
                {
                    GFE_RWPageHandleT<_TMatrix> attrib_ph(attribPtr);
                    GA_Offset start, end;
                    for (GA_PageIterator pit = r.beginPages(); !pit.atEnd(); ++pit)
                    {
                        for (GA_Iterator it(pit.begin()); it.blockAdvance(start, end); )
                        {
                            attrib_ph.setPage(start);
                            for (GA_Offset elemoff = start; elemoff < end; ++elemoff)
                            {
                                if (!indexMap.isOffsetInRange(elemoff))
                                    continue;
                                if (invertXform)
                                {
                                    MATRIX_T xform = xform_h.get(elemoff);
                                    xform.invert();
                                    attrib_ph.value(elemoff) *= xform;
                                }
                                else
                                {
                                    attrib_ph.value(elemoff) *= xform_h.get(elemoff);
                                }
                            }
                        }
                    }
                }, subscribeRatio, minGrainSize);
            }
        }
        else
        {
            switch (attribPtr->getOwner())
            {
            case GA_ATTRIB_PRIMITIVE:
                switch (xformAttrib->getOwner())
                {
                    case GA_ATTRIB_PRIMITIVE: xformMatrixByAttrib<GA_ATTRIB_PRIMITIVE, GA_ATTRIB_PRIMITIVE, _TMatrix, MATRIX_T>(); break;
                    case GA_ATTRIB_POINT:     xformMatrixByAttrib<GA_ATTRIB_PRIMITIVE, GA_ATTRIB_POINT,     _TMatrix, MATRIX_T>(); break;
                    case GA_ATTRIB_VERTEX:    xformMatrixByAttrib<GA_ATTRIB_PRIMITIVE, GA_ATTRIB_VERTEX,    _TMatrix, MATRIX_T>(); break;
                    default: UT_ASSERT_MSG(0, "unsupport owner"); break;
                }
                break;
            case GA_ATTRIB_POINT:
                switch (xformAttrib->getOwner())
                {
                    case GA_ATTRIB_PRIMITIVE: xformMatrixByAttrib<GA_ATTRIB_POINT, GA_ATTRIB_PRIMITIVE, _TMatrix, MATRIX_T>(); break;
                    case GA_ATTRIB_POINT:     xformMatrixByAttrib<GA_ATTRIB_POINT, GA_ATTRIB_POINT,     _TMatrix, MATRIX_T>(); break;
                    case GA_ATTRIB_VERTEX:    xformMatrixByAttrib<GA_ATTRIB_POINT, GA_ATTRIB_VERTEX,    _TMatrix, MATRIX_T>(); break;
                    default: UT_ASSERT_MSG(0, "unsupport owner"); break;
                }
                break;
            case GA_ATTRIB_VERTEX:
                switch (xformAttrib->getOwner())
                {
                    case GA_ATTRIB_PRIMITIVE: xformMatrixByAttrib<GA_ATTRIB_VERTEX, GA_ATTRIB_PRIMITIVE, _TMatrix, MATRIX_T>(); break;
                    case GA_ATTRIB_POINT:     xformMatrixByAttrib<GA_ATTRIB_VERTEX, GA_ATTRIB_POINT,     _TMatrix, MATRIX_T>(); break;
                    case GA_ATTRIB_VERTEX:    xformMatrixByAttrib<GA_ATTRIB_VERTEX, GA_ATTRIB_VERTEX,    _TMatrix, MATRIX_T>(); break;
                    default: UT_ASSERT_MSG(0, "unsupport owner"); break;
                }
                break;
                
            default: UT_ASSERT_MSG(0, "unsupport owner"); break;
            }
        }
    }







    
    template <typename _TMatrix, typename MATRIX_T>
    void xformVectorByMatrix(const MATRIX_T& xform)
    {
        UTparallelFor(groupParser.getSplittableRange(attribPtr), [this, &xform](const GA_SplittableRange& r)
        {
            GFE_RWPageHandleT<_TMatrix> attrib_ph(attribPtr);
            GA_Offset start, end;
            for (GA_PageIterator pit = r.beginPages(); !pit.atEnd(); ++pit)
            {
                for (GA_Iterator it(pit.begin()); it.blockAdvance(start, end); )
                {
                    attrib_ph.setPage(start);
                    for (GA_Offset elemoff = start; elemoff < end; ++elemoff)
                    {
                        if (invertXform)
                        {
                            MATRIX_T xform_inv(xform);
                            xform_inv.invert();
                            if (preserveLength)
                            {
                                const typename _TMatrix::value_type length = attrib_ph.value(elemoff).length();
                                attrib_ph.value(elemoff) *= xform_inv;
                                attrib_ph.value(elemoff).normalize();
                                attrib_ph.value(elemoff) *= length;
                            }
                            else
                            {
                                attrib_ph.value(elemoff) *= xform_inv;
                            }
                        }
                        else
                        {
                            if (preserveLength)
                            {
                                const typename _TMatrix::value_type length = attrib_ph.value(elemoff).length();
                                attrib_ph.value(elemoff) *= xform;
                                attrib_ph.value(elemoff).normalize();
                                attrib_ph.value(elemoff) *= length;
                            }
                            else
                            {
                                attrib_ph.value(elemoff) *= xform;
                            }
                        }
                    }
                }
            }
        }, subscribeRatio, minGrainSize);
    }

    
    template <typename _TMatrix, typename MATRIX_T>
    void xformMatrixByMatrix(const MATRIX_T& xform)
    {
        UTparallelFor(groupParser.getSplittableRange(attribPtr), [this, &xform](const GA_SplittableRange& r)
        {
            GFE_RWPageHandleT<_TMatrix> attrib_ph(attribPtr);
            GA_Offset start, end;
            for (GA_PageIterator pit = r.beginPages(); !pit.atEnd(); ++pit)
            {
                for (GA_Iterator it(pit.begin()); it.blockAdvance(start, end); )
                {
                    attrib_ph.setPage(start);
                    for (GA_Offset elemoff = start; elemoff < end; ++elemoff)
                    {
                        if (invertXform)
                        {
                            MATRIX_T xform_inv(xform);
                            xform_inv.invert();
                            attrib_ph.value(elemoff) *= xform_inv;
                        }
                        else
                        {
                            attrib_ph.value(elemoff) *= xform;
                        }
                    }
                }
            }
        }, subscribeRatio, minGrainSize);
    }

    

    template <typename _TMatrix>
    void xformMatrixByMatrix()
    {
        if(xformByXform4)
            xformMatrixByMatrix<_TMatrix, UT_Matrix4T<GFE_Type::get_value_type_t<_TMatrix>>>(xform4);
            //xformMatrixByMatrix<_TMatrix, UT_Matrix4T<typename _TMatrix::value_type>>(xform4);
        else
            xformMatrixByMatrix<_TMatrix, UT_Matrix3T<typename _TMatrix::value_type>>(xform3);
    }


    
    template <typename _TMatrix>
    void xformVectorByAttrib()
    {
        if (xformAttrib->getAIFTuple()->getTupleSize(xformAttrib) == 16)
        {
            switch (xformAttrib->getAIFTuple()->getStorage(xformAttrib))
            {
            case GA_STORE_REAL32: xformVectorByAttrib<_TMatrix, UT_Matrix4T<fpreal32>>();        break;
            case GA_STORE_REAL64: xformVectorByAttrib<_TMatrix, UT_Matrix4T<fpreal64>>();        break;
            default: break;
            }
        }
        else
            switch (xformAttrib->getAIFTuple()->getStorage(xformAttrib))
            {
            case GA_STORE_REAL32: xformVectorByAttrib<_TMatrix, UT_Matrix3T<fpreal32>>();        break;
            case GA_STORE_REAL64: xformVectorByAttrib<_TMatrix, UT_Matrix3T<fpreal64>>();        break;
            default: break;
            }
    }

    template <typename _TMatrix>
    void xformMatrixByAttrib()
    {
        if (xformAttrib->getAIFTuple()->getTupleSize(xformAttrib) == 16)
        {
            if constexpr(GFE_Type::isMatrix3<_TMatrix>)
            {
                switch (xformAttrib->getAIFTuple()->getStorage(xformAttrib))
                {
                    //case GA_STORE_INT16:  xformMatrixByAttrib<_TMatrix, UT_Matrix4T<int16>>();           break;
                    //case GA_STORE_INT32:  xformMatrixByAttrib<_TMatrix, UT_Matrix4T<int32>>();           break;
                    //case GA_STORE_INT64:  xformMatrixByAttrib<_TMatrix, UT_Matrix4T<int64>>();           break;
                    //case GA_STORE_REAL16: xformMatrixByAttrib<_TMatrix, UT_Matrix4T<fpreal16>>();        break;
                    case GA_STORE_REAL32: xformMatrix3ByAttrib<_TMatrix, UT_Matrix4T<fpreal32>>();        break;
                    case GA_STORE_REAL64: xformMatrix3ByAttrib<_TMatrix, UT_Matrix4T<fpreal64>>();        break;
                default: break;
                }
            }
            else
            {
                UT_ASSERT_P(GFE_Type::isMatrix4<_TMatrix>);
                switch (xformAttrib->getAIFTuple()->getStorage(xformAttrib))
                {
                    //case GA_STORE_INT16:  xformMatrixByAttrib<_TMatrix, UT_Matrix4T<int16>>();           break;
                    //case GA_STORE_INT32:  xformMatrixByAttrib<_TMatrix, UT_Matrix4T<int32>>();           break;
                    //case GA_STORE_INT64:  xformMatrixByAttrib<_TMatrix, UT_Matrix4T<int64>>();           break;
                    //case GA_STORE_REAL16: xformMatrixByAttrib<_TMatrix, UT_Matrix4T<fpreal16>>();        break;
                    case GA_STORE_REAL32: xformMatrixByAttrib<_TMatrix, UT_Matrix4T<fpreal32>>();        break;
                    case GA_STORE_REAL64: xformMatrixByAttrib<_TMatrix, UT_Matrix4T<fpreal64>>();        break;
                default: break;
                }
            }
        }
        else
        {
            switch (xformAttrib->getAIFTuple()->getStorage(xformAttrib))
            {
            case GA_STORE_REAL32: xformMatrixByAttrib<_TMatrix, UT_Matrix3T<fpreal32>>();        break;
            case GA_STORE_REAL64: xformMatrixByAttrib<_TMatrix, UT_Matrix3T<fpreal64>>();        break;
            default: break;
            }
        }
    }
    

    template <typename _TMatrix>
    void xform()
    {
        using value_type = typename _TMatrix::value_type;
        if (xformAttrib && attribPtr->getOwner() != GA_ATTRIB_DETAIL && xformAttrib->getOwner() != GA_ATTRIB_DETAIL)
        {
            if constexpr(GFE_Type::isMatrix34<_TMatrix>)
            {
                xformMatrixByAttrib<_TMatrix>();
            }
            else
            {
                xformVectorByAttrib<_TMatrix>();
            }
            return;
        }
        
        if (xformAttrib)
        {
            UT_ASSERT_P(xformAttrib->getAIFTuple());
            
            xformByXform4 = xformAttrib->getAIFTuple()->getTupleSize(xformAttrib) == 16;
            if (xformByXform4)
            {
                UT_ASSERT_MSG(attribPtr->getAIFTuple()->getTupleSize(xformAttrib) == 16, "can not be possible");
                
                const GA_ROHandleM4D xform_h(xformAttrib);
                xform4 = xform_h.get(0);
            }
            else
            {
                UT_ASSERT_MSG(attribPtr->getAIFTuple()->getTupleSize(xformAttrib) == 9, "can not be possible");
                
                const GA_ROHandleM3D xform_h(xformAttrib);
                xform3 = xform_h.get(0);
            }
        }

        if (attribPtr->getOwner() == GA_ATTRIB_DETAIL)
        {
            const GA_RWHandleT<_TMatrix> attrib_h(attribPtr);
            
            _TMatrix value = attrib_h.get(0);
            
            if (xformByXform4)
            {
                if constexpr (GFE_Type::isMatrix4<_TMatrix>);
                    value *= UT_Matrix4T<value_type>(xform4);
            }
            else
                value *= UT_Matrix3T<value_type>(xform3);
            
            attrib_h.set(0, value);
        }
        else
        {
            if constexpr(GFE_Type::isMatrix<_TMatrix>)
            {
                if (xformByXform4)
                {
                    
                    if constexpr (GFE_Type::isMatrix3<_TMatrix>)
                    {
                        xformMatrixByMatrix<_TMatrix, UT_Matrix3T<typename _TMatrix::value_type>>(UT_Matrix3T<typename _TMatrix::value_type>(xform4));
                    }
                    else
                    {
                        xformMatrixByMatrix<_TMatrix, UT_Matrix4T<typename _TMatrix::value_type>>(UT_Matrix4T<typename _TMatrix::value_type>(xform4));
                    }
                }
                else
                    xformMatrixByMatrix<_TMatrix, UT_Matrix3T<typename _TMatrix::value_type>>(UT_Matrix3T<typename _TMatrix::value_type>(xform3));
            }
            else
            {
                if (xformByXform4)
                    xformVectorByMatrix<_TMatrix, UT_Matrix4T<typename _TMatrix::value_type>>(UT_Matrix4T<typename _TMatrix::value_type>(xform4));
                else
                    xformVectorByMatrix<_TMatrix, UT_Matrix3T<typename _TMatrix::value_type>>(UT_Matrix3T<typename _TMatrix::value_type>(xform3));
            }
        }
    }



    
    void checkXformAttrib()
    {
        if (!xformAttrib)
            return;
    
        const GA_AIFTuple* const aifTuple = xformAttrib->getAIFTuple();

        if (!aifTuple)
        {
            xformAttrib = nullptr;
            xformAttribNonConst = nullptr;
            return;
        }
        
        const int tupleSize = aifTuple->getTupleSize(xformAttrib);
        if (tupleSize != 9 && tupleSize != 16)
        {
            xformAttrib = nullptr;
            xformAttribNonConst = nullptr;
            return;
        }
    }



public:

    
    UT_Matrix3T<fpreal64> xform3;
    UT_Matrix4T<fpreal64> xform4;

    
    bool preserveLength = false;
    bool invertXform    = false;
    bool delXformAttrib = true;
    

private:
    
    const GA_Attribute* xformAttrib = nullptr;
    GA_Attribute* xformAttribNonConst = nullptr;
    
    GA_Attribute* attribPtr;
    bool xformByXform4 = false;
    
    exint subscribeRatio = 64;
    exint minGrainSize   = 1024;


}; // End of class GFE_XformByAttrib



#endif
