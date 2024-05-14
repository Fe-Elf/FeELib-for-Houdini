
#pragma once

#ifndef __GFE_Math_h__
#define __GFE_Math_h__

#include <GFE/Math.h>



#include <GFE/Core.h>
#include <GFE/Type.h>
#include <GFE/TypeTraits.h>


#include <ufe.h>

_GFE_BEGIN
inline namespace Math {

    namespace detail
    {
    
        template<typename _Ty>
        SYS_FORCE_INLINE static _Ty getZeroVector()
        {
            if constexpr(gfe::isVector4<_Ty>)
                return _Ty(0, 0, 0, 0);
            else
                return _Ty(0.0);
        }
    
        template<typename _Ty>
        SYS_FORCE_INLINE static _Ty getZeroScalar()
        {
            if constexpr(std::is_floating_point_v<_Ty>)
                return _Ty(0.0);
            else
                return _Ty(0);
        }
    
    } // End of Namespace detail
    
    
    
    template<typename _Ty>
    SYS_FORCE_INLINE static _Ty getZero()
    {
        if constexpr(gfe::TypeTraits::isVector<_Ty>)
            return detail::getZeroVector<_Ty>();
        else
            return detail::getZeroScalar<_Ty>();
    }
    
    
    
    template<typename _Ty>
    SYS_FORCE_INLINE static gfe::TypeTraits::get_value_type_t<_Ty> length(const _Ty& val)
    {
        using value_type = typename gfe::TypeTraits::get_value_type_t<_Ty>;
        if constexpr (gfe::TypeTraits::isVector<_Ty>)
            return val.length();
        //return detail::lengthVec(val);
        //else if constexpr (GFE_Type::isMatrix<_Ty>)
        //    return detail::lengthMtx(val);
        else if constexpr (gfe::TypeTraits::isScalar<_Ty>)
            return value_type(val);
            
        //return detail::lengthScalar(val);
    }

    
    template<typename _Ty>
    SYS_FORCE_INLINE static gfe::TypeTraits::get_value_type_t<_Ty> length2(const _Ty& val)
    {
        using value_type = typename gfe::TypeTraits::get_value_type_t<_Ty>;
        if constexpr (gfe::TypeTraits::isVector<_Ty>)
            return val.length2();
        else if constexpr (gfe::TypeTraits::isScalar<_Ty>)
            return value_type(val) * value_type(val);
            
        //return detail::lengthScalar(val);
    }


    
    template<typename _TScalar>
    static int8 minidx(const UT_Vector3T<_TScalar>& arg)
    {
        int8 idx = 0;
        _TScalar val = arg[0];
        for (int8 i = 1; i < 3; ++i)
        {
            if (arg[i] >= val)
                continue;
            idx = i;
            val = arg[idx];
        }
        return idx;
    }

    template<typename _TScalar>
    static int8 mididx(const UT_Vector3T<_TScalar>& arg)
    {
        if ( arg[1] > arg[0] )
        {
            if ( arg[2] > arg[1] )
                return 1;
            else if ( arg[2] > arg[0] )
                return 2;
            else
                return 0;
        }
        else
        {
            if ( arg[2] > arg[0] )
                return 0;
            else if ( arg[2] > arg[1] )
                return 2;
            else
                return 1;
        }
    }

    template<typename _TScalar>
    static int8 maxidx(const UT_Vector3T<_TScalar>& arg)
    {
        int8 idx = 0;
        _TScalar val = arg[0];
        for (int i = 1; i < 3; ++i)
        {
            if (arg[i] <= val)
                continue;
            idx = i;
            val = arg[idx];
        }
        return idx;
    }


template<typename FLOAT_T>
static UT_Vector3T<FLOAT_T> blendDir(const UT_Vector3T<FLOAT_T>& dir0, const UT_Vector3T<FLOAT_T>& dir1, const FLOAT_T blend)
{
    UT_QuaternionT<FLOAT_T> q;
    q.updateFromVectors(dir0, dir1);
    q = UT_QuaternionT<FLOAT_T>(0,0,0,1).interpolate(q, blend);
    return q.rotate(dir0);
}

#define GFE_MATH_ReverseROC 1


template<typename _Ty>
SYS_FORCE_INLINE static _Ty distToLine(
        const UT_Vector3T<_Ty>& pos,
        const UT_Vector3T<_Ty>& pos0,
        const UT_Vector3T<_Ty>& pos1)
{ return sqrt(length2(cross(pos0 - pos, pos1 - pos)) / distance2(pos0, pos1)); }



//SYS_FORCE_INLINE static fpreal distToLine(
//     const UT_Vector3T<fpreal>& pos,
//     const UT_Vector3T<fpreal>& pos0,
//     const UT_Vector3T<fpreal>& pos1)
// { return sqrt(length2(cross(pos0 - pos, pos1 - pos)) / distance2(pos0, pos1)); }



template<typename _Ty>
SYS_FORCE_INLINE static _Ty distToLine(
    const UT_Vector3T<_Ty>& dir0,
    const UT_Vector3T<_Ty>& dir1,
    const UT_Vector3T<_Ty>& pos0,
    const UT_Vector3T<_Ty>& pos1)
{ return sqrt(length2(cross(dir0, dir1)) / distance2(pos0, pos1)); }


template<typename _Ty>
static _Ty circleRadius3Point(
    const UT_Vector3T<_Ty>& pos0,
    const UT_Vector3T<_Ty>& pos1,
    const UT_Vector3T<_Ty>& pos2
)
{
    const _Ty x1 = pos0[0],
            x2 = pos1[0],
            x3 = pos2[0],
            y1 = pos0[1],
            y2 = pos1[1],
            y3 = pos2[1],
            z1 = pos0[2],
            z2 = pos1[2],
            z3 = pos2[2];

    const _Ty a1 =  (y1 * z2 - y2 * z1 - y1 * z3 + y3 * z1 + y2 * z3 - y3 * z2),
            b1 = -(x1 * z2 - x2 * z1 - x1 * z3 + x3 * z1 + x2 * z3 - x3 * z2),
            c1 =  (x1 * y2 - x2 * y1 - x1 * y3 + x3 * y1 + x2 * y3 - x3 * y2),
            d1 = -(x1 * y2 * z3 - x1 * y3 * z2 - x2 * y1 * z3 + x2 * y3 * z1 + x3 * y1 * z2 - x3 * y2 * z1);

    const _Ty a2 = 2 * (x2 - x1),
            b2 = 2 * (y2 - y1),
            c2 = 2 * (z2 - z1),
            d2 = x1 * x1 + y1 * y1 + z1 * z1 - x2 * x2 - y2 * y2 - z2 * z2;

    const _Ty a3 = 2 * (x3 - x1),
            b3 = 2 * (y3 - y1),
            c3 = 2 * (z3 - z1),
            d3 = x1 * x1 + y1 * y1 + z1 * z1 - x3 * x3 - y3 * y3 - z3 * z3;

    UT_Vector3T<_Ty> circleCenter;
    circleCenter[0] = -(b1 * c2 * d3 - b1 * c3 * d2 - b2 * c1 * d3 + b2 * c3 * d1 + b3 * c1 * d2 - b3 * c2 * d1)
                     / (a1 * b2 * c3 - a1 * b3 * c2 - a2 * b1 * c3 + a2 * b3 * c1 + a3 * b1 * c2 - a3 * b2 * c1);
    circleCenter[1] = (a1 * c2 * d3 - a1 * c3 * d2 - a2 * c1 * d3 + a2 * c3 * d1 + a3 * c1 * d2 - a3 * c2 * d1)
                     / (a1 * b2 * c3 - a1 * b3 * c2 - a2 * b1 * c3 + a2 * b3 * c1 + a3 * b1 * c2 - a3 * b2 * c1);
    circleCenter[2] = -(a1 * b2 * d3 - a1 * b3 * d2 - a2 * b1 * d3 + a2 * b3 * d1 + a3 * b1 * d2 - a3 * b2 * d1)
                     / (a1 * b2 * c3 - a1 * b3 * c2 - a2 * b1 * c3 + a2 * b3 * c1 + a3 * b1 * c2 - a3 * b2 * c1);

    // addpoint(0, circleCenter);

    // return distance(circleCenter, pos0);

    // #if GFE_MATH_ReverseROC
    //     return 1.0 / distance(circleCenter, pos0);
    // #else
    //     return distance(circleCenter, pos0);
    // #endif
#if GFE_MATH_ReverseROC
    return sqrt(distance2(pos0, pos2) / distance2(circleCenter, pos0));
#else
    return sqrt(distance2(circleCenter, pos0) / distance2(pos0, pos2));
#endif
}


#undef GFE_MATH_ReverseROC





    
template<typename FLOAT_T>
static bool pointInTriangleT(
    const UT_Vector3T<FLOAT_T>& p,
    const UT_Vector3T<FLOAT_T>& a,
    const UT_Vector3T<FLOAT_T>& b,
    const UT_Vector3T<FLOAT_T>& c
)
{
    const UT_Vector3T<FLOAT_T> ba = b - a;
    const UT_Vector3T<FLOAT_T> cb = c - b;
    const UT_Vector3T<FLOAT_T> ac = a - c;
    const UT_Vector3T<FLOAT_T> n = cross(ac, ba);  // Same as n = ba X ca

    UT_Vector3T<FLOAT_T> px = p - a;
    UT_Vector3T<FLOAT_T> nx = cross(ba, px);
    if (nx.dot(n) < 0) return false;
    
    px = p - b;
    nx = cross(cb, px);
    if (nx.dot(n) < 0) return false;
    
    px = p - c;
    nx = cross(ac, px);
    if (nx.dot(n) < 0) return false;
    
    return true;
}

/*
template<typename FLOAT_T>
static bool pointInTriangleT1(
    const UT_Vector3T<FLOAT_T>& pointPos,
    const UT_Vector3T<FLOAT_T>& triPosCenter,
    const UT_Vector3T<FLOAT_T>& triPosLeft,
    const UT_Vector3T<FLOAT_T>& triPosRight
)
{
    const UT_Vector3T<FLOAT_T> cl = triPosLeft   - triPosCenter;
    const UT_Vector3T<FLOAT_T> cr = triPosRight  - triPosCenter;
    const UT_Vector3T<FLOAT_T> cp = pointPos     - triPosCenter;
    const UT_Vector3T<FLOAT_T> lr = triPosRight  - triPosLeft;
    const UT_Vector3T<FLOAT_T> lc = triPosLeft   - triPosCenter;
    const UT_Vector3T<FLOAT_T> lp = triPosLeft   - pointPos;
    const UT_Vector3T<FLOAT_T> rl = triPosLeft   - triPosRight;
    const UT_Vector3T<FLOAT_T> rc = triPosCenter - triPosRight;
    const UT_Vector3T<FLOAT_T> rp = pointPos     - triPosRight;
    
    UT_Vector3T<FLOAT_T> dir0 = cross(cl, cr);
    UT_Vector3T<FLOAT_T> dir1 = cross(cl, cp);
    UT_Vector3T<FLOAT_T> dir2 = cross(lr, lc);
    UT_Vector3T<FLOAT_T> dir3 = cross(lr, lp);
    UT_Vector3T<FLOAT_T> dir4 = cross(rc, rl);
    UT_Vector3T<FLOAT_T> dir5 = cross(rc, rp);
    dir0.normalize();
    dir1.normalize();
    dir2.normalize();
    dir3.normalize();
    dir4.normalize();
    dir5.normalize();
    return dot(dir0, dir1) > 1e-05 &&
           dot(dir2, dir3) > 1e-05 &&
           dot(dir4, dir5) > 1e-05;
}
*/

// typedef pointInTriangleT<fpreal16>   pointInTriangleH;
// typedef pointInTriangleT<fpreal32>   pointInTriangleF;
// typedef pointInTriangleT<fpreal64>   pointInTriangleD;
// typedef pointInTriangleT<fpreal>     pointInTriangleR;

#define pointInTriangleH pointInTriangleT<fpreal16>
#define pointInTriangleF pointInTriangleT<fpreal32>
#define pointInTriangleD pointInTriangleT<fpreal64>
#define pointInTriangleR pointInTriangleT<fpreal>








    
} // End of Namespace Math
_GFE_END


#endif
