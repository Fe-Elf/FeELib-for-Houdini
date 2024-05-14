
#pragma once

#ifndef __GFE_Bound_h__
#define __GFE_Bound_h__

#include <GFE/Bound.h>

#include <GFE/Core.h>

#include <UT/UT_BoundingBox.h>

_GFE_BEGIN
namespace Bound {

    template<typename _TScalar>
    SYS_FORCE_INLINE static UT_BoundingBoxT<_TScalar> stdBoundingBoxT()
    { return UT_BoundingBoxT<_TScalar>(std::numeric_limits<_TScalar>::max(),
                                       std::numeric_limits<_TScalar>::max(),
                                       std::numeric_limits<_TScalar>::max(),
                                       std::numeric_limits<_TScalar>::lowest(),
                                       std::numeric_limits<_TScalar>::lowest(),
                                       std::numeric_limits<_TScalar>::lowest()); }

    //using stdBoundingBoxH = stdBoundingBoxT<fpreal16>;
    //using stdBoundingBoxF = stdBoundingBoxT<fpreal32>;
    //using stdBoundingBoxD = stdBoundingBoxT<fpreal64>;
    //using stdBoundingBoxR = stdBoundingBoxT<fpreal>  ;
    
    //typedef stdBoundingBoxT<fpreal16> stdBoundingBoxH;
    //typedef stdBoundingBoxT<fpreal32> stdBoundingBoxF;
    //typedef stdBoundingBoxT<fpreal64> stdBoundingBoxD;
    //typedef stdBoundingBoxT<fpreal>   stdBoundingBoxR;


    template<typename _TScalar, typename _TScalar1>
    static void expandBounds(UT_BoundingBoxT<_TScalar>& bound, const UT_BoundingBoxT<_TScalar1>& boundRef)
    {
        bound(0,0) += boundRef(0,0);
        bound(0,1) += boundRef(0,1);
        bound(1,0) += boundRef(1,0);
        bound(1,1) += boundRef(1,1);
        bound(2,0) += boundRef(2,0);
        bound(2,1) += boundRef(2,1);
    }

    template<typename _TScalar>
    SYS_FORCE_INLINE static void setStd(UT_BoundingBoxT<_TScalar>& bbox)
    { bbox.setBounds(std::numeric_limits<_TScalar>::max(),
                     std::numeric_limits<_TScalar>::max(),
                     std::numeric_limits<_TScalar>::max(),
                     std::numeric_limits<_TScalar>::lowest(),
                     std::numeric_limits<_TScalar>::lowest(),
                     std::numeric_limits<_TScalar>::lowest()); }
    
    
    template<typename _TScalar>
    static UT_BoundingBoxT<_TScalar> getBBox(const GA_Detail& geo, const GA_Range& pointRange, const GA_Attribute* posAttrib = nullptr)
    {
        if (gfe::isInvalidPosAttrib(posAttrib))
            posAttrib = geo.getP();
        
        auto bbox = stdBoundingBoxT<fpreal32>();
        geo.enlargeBoundingBox(bbox, pointRange, posAttrib);
        return UT_BoundingBoxT<_TScalar>(bbox);
        // if constexpr (std::is_same_v<_TScalar, float>)
        //     return bbox;
        // else
        //     return UT_BoundingBoxT<_TScalar>(bbox);
    }
    
    template<typename _TScalar>
    static UT_BoundingBoxT<_TScalar> getUnitBBox()
    {
        return UT_BoundingBoxT<_TScalar>(-0.5, -0.5, -0.5, 0.5, 0.5, 0.5);
    }
    
    template<typename _TScalar>
    static void scale(UT_BoundingBoxT<_TScalar>& bbox, const UT_Vector3T<_TScalar>& scale)
    {
        bbox.vals[0][0] *= scale[0]; bbox.vals[0][1] *= scale[0];
        bbox.vals[1][0] *= scale[1]; bbox.vals[1][1] *= scale[1];
        bbox.vals[2][0] *= scale[2]; bbox.vals[2][1] *= scale[2];
    }
    
    
} // End of namespace Bound
_GFE_END
#endif
