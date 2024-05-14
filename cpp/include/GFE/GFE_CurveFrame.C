/*
 * Copyright (c) 2022
 *      Side Effects Software Inc.  All rights reserved.
 *
 * Redistribution and use of Houdini Development Kit samples in source and
 * binary forms, with or without modification, are permitted provided that the
 * following conditions are met:
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. The name of Side Effects Software may not be used to endorse or
 *    promote products derived from this software without specific prior
 *    written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY SIDE EFFECTS SOFTWARE `AS IS' AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN
 * NO EVENT SHALL SIDE EFFECTS SOFTWARE BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *----------------------------------------------------------------------------
 * Definitions of functions for computing reference frames
 * for curve vertices.
 */

#include "GFE_CurveFrame.h"

#include <GEO/GEO_Curve.h>
#include <GA/GA_AttributeInstanceMatrix.h>
#include <GA/GA_Iterator.h>
#include <GA/GA_OffsetList.h>
#include <GA/GA_ElementGroup.h>
#include <GA/GA_Range.h>
#include <GA/GA_SplittableRange.h>
#include <GA/GA_Types.h>
#include <UT/UT_Assert.h>
#include <UT/UT_Interrupt.h>
#include <UT/UT_Matrix3.h>
#include <UT/UT_Matrix4.h>
#include <UT/UT_ParallelUtil.h>
#include <UT/UT_Ramp.h>
#include <UT/UT_SmallArray.h>
#include <UT/UT_StringHolder.h>
#include <UT/UT_Vector3.h>
#include <SYS/SYS_Math.h>
#include <SYS/SYS_Types.h>

namespace HDK_Sample {

namespace GU_CurveFrame {

static constexpr float theExtremelySmallLength2 = 1e-37f;
static constexpr float theQuiteSmallRelativeLength = 1e-5f;
static constexpr float theQuiteSmallRelativeLength2 = 1e-10f;

template<typename T>
static void
interpolateTangent(
    const TangentType tangent_type,
    const bool stretch_using_backbone,
    const T max_stretch_scale,
    const T max_stretch_length_threshold, // 2/max_stretch_length
    const UT_Vector3T<T> &prev_edge,
    const UT_Vector3T<T> &next_edge,
    const T prev_length2,
    const T next_length2,
    const bool is_last, // false for first, true for last
    UT_Vector3T<T> *tangent,
    UT_Vector3T<T> *pend_stretch_dir,
    T *pend_stretch_scale,
    bool normalize)
{
    UT_ASSERT_MSG_P(prev_length2 > 0 && next_length2 > 0, "This function doesn't handle zero-length edges.  The caller should.");
    UT_Vector3T<T> next_dir = next_edge;
    if (next_length2 != 1) {
        next_dir /= SYSsqrt(next_length2);
    }
    // first_dir and next_dir are unit vectors.
    // We want to project back what prev_dir would be
    // and compute the tangent from first_dir
    // and prev_dir.
    // Projection back the same angle as
    // the angle between first_dir and next_dir.
    UT_Vector3T<T> prev_dir = prev_edge;
    if (prev_length2 != 1) {
        prev_dir /= SYSsqrt(prev_length2);
    }
    UT_Vector3T<T> mid_dir;
    T mid_length;
    if (tangent_type == TangentType::CIRCULAR || stretch_using_backbone)
    {
        mid_dir = prev_dir + next_dir;
        T mid_length2 = mid_dir.length2();
        mid_length = SYSsqrt(mid_length2);
        if (mid_length < theQuiteSmallRelativeLength) {
            // Almost perfectly backtracking, so just pick the inner one.
            mid_dir = is_last ? prev_dir : next_dir;
        }
        else {
            mid_dir /= mid_length;
        }
    }
    if (stretch_using_backbone) {
        T stretch_scale;
        UT_Vector3T<T> stretch_dir;
        if (mid_length < max_stretch_length_threshold) {
            stretch_scale = max_stretch_scale;

            if (mid_length < theQuiteSmallRelativeLength) {
                // Almost perfectly backtracking, so just pick the inner one.
                stretch_dir = is_last ? prev_dir : next_dir;
            }
            else {
                stretch_dir = next_dir - prev_dir;
                stretch_dir.normalize();
            }
        }
        else {
            // It takes a bit of trig to confirm, but the
            // stretch scale is 1/cos(angle/2), which is 2/L_mid.
            stretch_scale = SYSmin(2.0f / mid_length, max_stretch_scale);
            stretch_dir = next_dir - prev_dir;
            stretch_dir.normalize();
        }
        *pend_stretch_dir = stretch_dir;
        *pend_stretch_scale = stretch_scale;
    }
    if (!tangent)
        return;

    switch (tangent_type) {
        case TangentType::CIRCULAR:
            *tangent = mid_dir;
            break;
        case TangentType::PREV:
            *tangent = normalize ? prev_dir : prev_edge;
            break;
        case TangentType::NEXT:
            *tangent = normalize ? next_dir : next_edge;
            break;
        case TangentType::SUBD:
            // We need to use the unnormalized directions to interpolate in the subd case,
            // even if we're normalizing after.
            mid_dir = prev_edge + next_edge;
            if (normalize)
            {
                T mid_length2 = mid_dir.length2();
                if (mid_length2 < theQuiteSmallRelativeLength2*next_length2) {
                    // Almost perfectly stopping, so just pick the inner one (normalized).
                    mid_dir = is_last ? prev_dir : next_dir;
                }
                else {
                    mid_dir /= SYSsqrt(mid_length2);
                }
            }
            else
                mid_dir *= T(0.5);
            *tangent = mid_dir;
            break;
        case TangentType::NONE:
            UT_ASSERT_MSG(0, "This case should have been excluded earlier.");
            break;
    }
}

template<typename T>
static SYS_FORCE_INLINE UT_Vector3T<T>
extrapolateVectorCircular(const UT_Vector3T<T> &a, const UT_Vector3T<T> &b) {
    // If 'b' is a unit vector, this is a reflection of a through the line that goes through 'b' and the origin,
    // resulting in a vector that's 'a' rotated by twice the rotation that takes 'a' to 'b'.
    return (2*dot(a,b))*b - a;
}

template<typename T>
static SYS_FORCE_INLINE UT_Vector3T<T>
extrapolateVectorLinear(const UT_Vector3T<T> &a, const UT_Vector3T<T> &b) {
    // This is a linear extrapolation as far from 'b' as 'b' is from 'a'.
    return b + (b-a);
}

template<typename T>
static void
extrapolateEndTangent(
    const TangentType tangent_type,
    const bool stretch_using_backbone,
    const T max_stretch_scale,
    const T max_stretch_length_threshold, // 2/max_stretch_length
    const UT_Vector3T<T> &outer_edge,
    const UT_Vector3T<T> &inner_edge,
    const UT_Vector3T<T> &outer_dir, // outer_edge normalized
    const bool is_last, // false for first, true for last
    UT_Vector3T<T> *end_tangent,
    UT_Vector3T<T> *pend_stretch_dir,
    T *pend_stretch_scale,
    const bool outer_edge_is_tangent)
{
    UT_Vector3T<T> inner_dir = inner_edge;
    T inner_length2 = inner_dir.length2();
    if (inner_length2 == 0) {
        if (end_tangent)
            *end_tangent = outer_dir;
        return;
    }

    inner_dir /= SYSsqrt(inner_length2);

    UT_Vector3T<T> extrapolated_edge;
    T extrapolated_length2;
    UT_Vector3T<T> end_edge_or_dir;
    T end_length2;
    if (tangent_type != TangentType::SUBD) {
        extrapolated_edge = extrapolateVectorCircular(inner_dir, outer_dir);
        extrapolated_length2 = 1;
        end_edge_or_dir = outer_edge_is_tangent ? inner_dir : outer_dir;
        end_length2 = 1;
    }
    else {
        // We need to use the unnormalized directions to extrapolate in the subd case.
        extrapolated_edge = extrapolateVectorLinear(inner_edge, outer_edge);
        extrapolated_length2 = extrapolated_edge.length2();
        end_edge_or_dir = outer_edge_is_tangent ? inner_edge : outer_edge;
        end_length2 = outer_edge_is_tangent ? inner_length2 : outer_edge.length2();
    }

    interpolateTangent(tangent_type,
        stretch_using_backbone,
        max_stretch_scale,
        max_stretch_length_threshold,
        is_last ? end_edge_or_dir : extrapolated_edge,
        is_last ? extrapolated_edge : end_edge_or_dir,
        is_last ? end_length2 : extrapolated_length2,
        is_last ? extrapolated_length2 : end_length2,
        is_last,
        end_tangent, pend_stretch_dir, pend_stretch_scale,
        true); // FIXME: Support non-normalized scales!!!
}

template<typename T,typename T2>
static void
computeSingleBackboneFrames(
    bool &rotate_using_backbone,
    bool &stretch_using_backbone,
    bool need_directions,
    UT_Array<UT_Vector3T<T>> &directions,
    UT_Array<UT_Vector3T<T>> &tangents,
    UT_Array<UT_Vector3T<T>> &up_vectors,
    UT_Array<UT_Vector3T<T>> &stretch_dirs,
    UT_Array<T> &stretch_scales,
    T &total_twist_around_loop,
    const GEO_Detail *geo,
    GA_Size nedges,
    GA_Size npoints,
    GA_Size nverts,
    bool closed,
    TangentType tangent_type,
    bool extrapolate_end_tangents,
    const GA_ROHandleV3D &instance_N,
    const GA_Offset primoff,
    const GA_OffsetListRef &vertices,
    const bool use_nurbs_tangents,
    T max_stretch_scale,
    T max_stretch_length_threshold,
    const GA_ROHandleV3D &instance_up,
    UT_Vector3T<T> target_up_vector,
    const bool use_normal_vector_up,
    const bool target_up_vector_at_start,
    const bool continuous_closed_curves,
    const bool use_end_target_up_vector,
    UT_Vector3T<T> end_target_up_vector,
    RotationPer twist_per,
    const GA_ROHandleT<T2> &roll_attrib,
    const int ucomponent)
{
    rotate_using_backbone = (rotate_using_backbone && nedges >= 1);
    stretch_using_backbone = (stretch_using_backbone && npoints >= 3);

    if (!rotate_using_backbone && !stretch_using_backbone && !need_directions)
        return;

    // Compute the backbone frames for all vertices of the curve,
    // as well as the stretch vectors, if applicable.

    directions.setSize(nedges);

    const UT_Vector3T<T> pos0 = geo->getPos3(geo->vertexPoint(vertices(0)));
    UT_Vector3T<T> prev_pos = pos0;
    T max_length_squared = 0;
    for (GA_Size i = 1; i < nverts; ++i)
    {
        const UT_Vector3T<T> cur_pos = geo->getPos3(geo->vertexPoint(vertices(i)));
        UT_Vector3T<T> dir = cur_pos - prev_pos;
        T length2 = dir.length2();
        // Force very short edges to exactly zero, so that length2()
        // will always produce exactly zero for them, even if the
        // compiler pulls some inconsistent shenanigans.
        if (length2 < theExtremelySmallLength2)
        {
            length2 = 0;
            dir.assign(0,0,0);
        }
        directions(i-1) = dir;
        max_length_squared = SYSmax(max_length_squared, length2);
        prev_pos = cur_pos;
    }
    if (closed && nverts == nedges)
    {
        // One last edge if closed and not unrolled
        UT_Vector3T<T> dir = pos0 - prev_pos;
        T length2 = dir.length2();
        // Force very short edges to exactly zero, as above.
        if (length2 < theExtremelySmallLength2)
        {
            length2 = 0;
            dir.assign(0,0,0);
        }
        directions(nverts-1) = dir;
        max_length_squared = SYSmax(max_length_squared, length2);
    }

    if (!rotate_using_backbone && !stretch_using_backbone)
        return;

    if (max_length_squared == 0)
    {
        // If all edges are effectively zero-length, stick
        // with no rotation or stretching.
        rotate_using_backbone = false;
        stretch_using_backbone = false;
        return;
    }

    // We need to specially handle any zero-length edges at the
    // beginning or end of the curve.
    // These loops rely on length2() being consistently
    // zero or non-zero, because we know that there's
    // at least one non-zero-length edge; we just don't
    // want to incorrectly loop all the way past the end.
    GA_Size first_nonzero = 0;
    GA_Size last_nonzero = directions.size()-1;
    while (first_nonzero < last_nonzero && directions(first_nonzero).length2() == 0)
    {
        ++first_nonzero;
    }
    while (first_nonzero < last_nonzero && directions(last_nonzero).length2() == 0)
    {
        --last_nonzero;
    }

    tangents.setSize(npoints);
    up_vectors.setSize(npoints);

    if (instance_N.isValid())
    {
        UT_ASSERT(!instance_up.isValid());
        for (exint i = 0; i < npoints; ++i)
        {
            GA_Offset ptoff = geo->vertexPoint(vertices(i));
            UT_Vector3D normal = instance_N.get(ptoff);
            normal.normalize();
            tangents[i] = normal;
        }
    }
    if (instance_up.isValid())
    {
        UT_ASSERT(!instance_N.isValid());
        for (exint i = 0; i < npoints; ++i)
        {
            GA_Offset ptoff = geo->vertexPoint(vertices(i));
            UT_Vector3D up = instance_up.get(ptoff);
            up.normalize();
            up_vectors[i] = up;
        }
    }

    if (first_nonzero == last_nonzero)
    {
        // Exactly one non-zero-length edge, (usually just one total).
        if (!instance_N.isValid())
        {
            // The single edge length must have length sqrt(max_edge_length_squared).
            UT_Vector3T<T> dir = directions(first_nonzero) / SYSsqrt(max_length_squared);
            tangents.constant(dir);

            if (!instance_up.isValid())
            {
                // No N or up, so same up for all points
                UT_Vector3T<T> other = target_up_vector;
                other -= dot(other,dir)*dir;
                T length2 = other.length2();
                if (length2 >= theQuiteSmallRelativeLength2)
                    other /= SYSsqrt(length2);
                else
                    other.arbitraryPerp(dir);
                up_vectors.constant(other);
            }
            else
            {
                // up attribute, so get closest to up perpendicular to dir
                for (exint i = 0; i < npoints; ++i)
                {
                    UT_Vector3D other = up_vectors[i];
                    other -= dot(other,dir)*dir;
                    T length2 = other.length2();
                    if (length2 >= theQuiteSmallRelativeLength2)
                        other /= SYSsqrt(length2);
                    else
                        other.arbitraryPerp(dir);
                    up_vectors[i] = other;
                }
            }
        }
        else
        {
            // N attribute, but no up, so get closest to target up perpendicular to N
            UT_ASSERT(!instance_up.isValid());

            for (exint i = 0; i < npoints; ++i)
            {
                UT_Vector3D dir = tangents[i];
                UT_Vector3D other = target_up_vector;
                other -= dot(other,dir)*dir;
                T length2 = other.length2();
                if (length2 >= theQuiteSmallRelativeLength2)
                    other /= SYSsqrt(length2);
                else
                    other.arbitraryPerp(dir);
                up_vectors[i] = other;
            }
        }

        // No stretching if only one non-zero-length edge.
        stretch_using_backbone = false;

        // Nothing more to do in this case.
        return;
    }

    // More than one non-zero-length edge.

    if (stretch_using_backbone)
    {
        stretch_dirs.setSizeNoInit(npoints);
        // setSize would initialize to zero, but let's make it explicitly clear
        // that we want to initialize these to a default of zero.
        // A stretch vector being zero will result in no stretch.
        stretch_dirs.constant(UT_Vector3T<T>(0,0,0));
        stretch_scales.setSizeNoInit(npoints);
        stretch_scales.constant(T(1));
    }

    if (instance_N.isValid() && stretch_using_backbone)
    {
        // If instance_N is valid, don't compute tangents, but do still
        // compute stretch_dirs and stretch_scales if stretch_using_backbone,
        // including extrapolating ends as needed.
        if (!closed)
        {
            // Always act as if extrapolating, but based on tangent,
            // since end tangents may not line up with end directions.
            if (first_nonzero == 0)
            {
                extrapolateEndTangent(tangent_type,
                    stretch_using_backbone, max_stretch_scale,
                    max_stretch_length_threshold,
                    tangents[0],
                    directions[0],
                    tangents[0], // Already normalized
                    false, // Not last edge (since first edge)
                    (UT_Vector3T<T>*)nullptr,
                    &stretch_dirs(0),
                    &stretch_scales(0),
                    true);// outer_edge is a tangent
            }
            if (last_nonzero == directions.size()-1)
            {
                extrapolateEndTangent(tangent_type,
                    stretch_using_backbone, max_stretch_scale,
                    max_stretch_length_threshold,
                    tangents.last(),
                    directions.last(),
                    tangents.last(), // Already normalized
                    true, // Last edge
                    (UT_Vector3T<T>*)nullptr,
                    &stretch_dirs(npoints-1),
                    &stretch_scales(npoints-1),
                    true);// outer_edge is a tangent
            }
        }
        else if (first_nonzero == 0 && last_nonzero == directions.size()-1)
        {
            // Closed backbone and no zero-length edges at beginning or end, so
            // it's like a normal case, except with non-adjacent edge indices.

            interpolateTangent(
                tangent_type,
                stretch_using_backbone,
                max_stretch_scale,
                max_stretch_length_threshold,
                directions.last(),
                directions(0),
                directions.last().length2(),
                directions(0).length2(),
                false, // Arbitrarily choose not last
                (UT_Vector3T<T>*)nullptr,
                &stretch_dirs(0),
                &stretch_scales(0),
                true); // FIXME: Support non-normalized scales!!!
        }

        // Middle stretch cases
        GA_Size prev_nonzero = first_nonzero;
        T prev_length2 = directions[first_nonzero].length2();
        for (GA_Size next_edgei = first_nonzero+1; next_edgei <= last_nonzero; ++next_edgei) {
            T next_length2 = directions[next_edgei].length2();
            if (next_length2 != 0)
            {
                // Common case: Two adjacent, non-zero-length edges.
                interpolateTangent(
                    tangent_type,
                    stretch_using_backbone,
                    max_stretch_scale,
                    max_stretch_length_threshold,
                    directions[prev_nonzero],
                    directions[next_edgei],
                    prev_length2,
                    next_length2,
                    false, // Arbitrarily choose not last
                    (UT_Vector3T<T>*)nullptr,
                    &stretch_dirs[next_edgei],
                    &stretch_scales[next_edgei],
                    true); // FIXME: Support non-normalized scales!!!
            }
            else
            {
                // Find next non-zero-length edge.
                do {
                    ++next_edgei;
                    next_length2 = directions(next_edgei).length2();
                } while (next_edgei != last_nonzero && next_length2 == 0);
            }
            prev_nonzero = next_edgei;
            prev_length2 = next_length2;
        }
    }

    if (!instance_N.isValid() && use_nurbs_tangents)
    {
        int primtype = geo->getPrimitiveTypeId(primoff);
        if (primtype == GA_PRIMNURBCURVE || primtype == GA_PRIMBEZCURVE)
        {
            const GEO_Curve *curve = UTverify_cast<const GEO_Curve *>(geo->getPrimitive(primoff));
            const bool curve_closed = curve->isClosed();
            const GA_Basis *basis = curve->getBasis();

            UT_Array<float> greville_us;
            greville_us.setSizeNoInit(nverts);
            for (exint i = 0; i < nverts; ++i)
            {
                float u = basis->getGreville(i, true, curve_closed);
                greville_us(i) = u;
            }

            // Request tangents from curve.
            for (exint i = 0; i < nverts; ++i)
            {
                UT_Vector4 cur_tangent;
                float u = greville_us(i);
                curve->evaluate(u, cur_tangent, /*du=*/1);




            }
        }
    }

    // Handle the beginning and end of the backbone first,
    // so that everything else is just a common middle case.
    UT_Vector3T<T> first_dir = directions(first_nonzero);
    UT_Vector3T<T> last_dir = directions(last_nonzero);
    first_dir.normalize();
    last_dir.normalize();
    if (!instance_N.isValid())
    {
        if (!closed) {
            if (first_nonzero > 0) {
                // If there are zero-length edges at the beginning,
                // set tangents to the first non-zero-length edge direction,
                // up to and including the first point of that edge.
                for (GA_Size i = 0; i <= first_nonzero; ++i) {
                    tangents(i) = first_dir;
                }
            }
            else if (extrapolate_end_tangents) {
                extrapolateEndTangent(tangent_type,
                    stretch_using_backbone, max_stretch_scale,
                    max_stretch_length_threshold,
                    directions(0),
                    directions(1), // We know there are at least two, so this is safe.
                    first_dir,
                    false, // Not last edge (since first edge)
                    &tangents(0),
                    stretch_using_backbone ? &stretch_dirs(0) : nullptr,
                    stretch_using_backbone ? &stretch_scales(0) : nullptr,
                    false);// outer_edge is not a tangent
            }
            else {
                // Not extrapolating end tangents, so just copy.
                tangents(0) = first_dir;
            }

            if (last_nonzero < directions.size()-1) {
                // Same as above for zero-length edges at the end.
                for (GA_Size i = last_nonzero+1; i < npoints; ++i) {
                    tangents(i) = last_dir;
                }
            }
            else if (extrapolate_end_tangents) {
                extrapolateEndTangent(tangent_type,
                    stretch_using_backbone, max_stretch_scale,
                    max_stretch_length_threshold,
                    directions.last(),
                    directions(directions.size()-2), // We know there are at least two, so this is safe.
                    last_dir,
                    true, // Last edge
                    &tangents(npoints-1),
                    stretch_using_backbone ? &stretch_dirs(npoints-1) : nullptr,
                    stretch_using_backbone ? &stretch_scales(npoints-1) : nullptr,
                    false);// outer_edge is not a tangent
            }
            else {
                // Not extrapolating end tangents, so just copy.
                tangents(npoints-1) = last_dir;
            }
        }
        else if (first_nonzero > 0 || last_nonzero < directions.size()-1) {
            // Closed backbone and at least one zero-length edge at beginning or end.

            switch (tangent_type) {
                case TangentType::CIRCULAR:
                {
                    if (first_nonzero == 1 && last_nonzero == directions.size()-1) {
                        tangents(0) = last_dir;
                        tangents(1) = first_dir;
                    }
                    else if (first_nonzero == 0 && last_nonzero == directions.size()-2) {
                        tangents.last() = last_dir;
                        tangents(0) = first_dir;
                    }
                    else {
                        // 3+ coincident, so circularly interpolate tangent
                        T cos_angle = dot(first_dir,last_dir);
                        UT_Vector3T<T> sin_axis = first_dir - cos_angle*last_dir;
                        sin_axis.normalize();
                        // TODO: atan2(sin_angle,cos_angle) is more stable for small angles or half turns.
                        T full_angle = SYSacos(cos_angle);
                        GA_Size nsteps = first_nonzero + (directions.size()-1 - last_nonzero);
                        GA_Size j = 0;
                        for (GA_Size i = last_nonzero+1; i < npoints; ++i, ++j) {
                            T part_angle = j*full_angle/nsteps;
                            tangents(i) = SYScos(part_angle)*last_dir + SYSsin(part_angle)*sin_axis;
                        }
                        for (GA_Size i = 0; i <= first_nonzero; ++i, ++j) {
                            T part_angle = j*full_angle/nsteps;
                            tangents(i) = SYScos(part_angle)*last_dir + SYSsin(part_angle)*sin_axis;
                        }
                    }
                    break;
                }
                case TangentType::SUBD:
                {
                    if (first_nonzero == 1 && last_nonzero == directions.size()-1) {
                        tangents(0) = last_dir;
                        tangents(1) = first_dir;
                    }
                    else if (first_nonzero == 0 && last_nonzero == directions.size()-2) {
                        tangents.last() = last_dir;
                        tangents(0) = first_dir;
                    }
                    else {
                        // 3+ coincident, so linearly interpolate tangent
                        GA_Size nsteps = first_nonzero + (directions.size()-1 - last_nonzero);
                        GA_Size j = 0;
                        // NOTE: This value 0,0,1 should never end up being used.  It should be overwritten first.
                        UT_Vector3T<T> prev_tangent(0,0,1);
                        for (GA_Size i = last_nonzero+1; i < npoints; ++i, ++j) {
                            UT_Vector3T<T> tangent = SYSlerp(directions(last_nonzero), directions(first_nonzero), T(j)/nsteps);
                            tangent.normalize();
                            if (tangent.length2() < 0.5f) {
                                // If didn't normalize, fall back to previous tangent,
                                // (shouldn't happen on first iteration, since tangent should be last_dir).
                                tangent = prev_tangent;
                            }
                            else {
                                prev_tangent = tangent;
                            }
                            tangents(i) = tangent;
                        }
                        for (GA_Size i = 0; i <= first_nonzero; ++i, ++j) {
                            UT_Vector3T<T> tangent = SYSlerp(directions(last_nonzero), directions(first_nonzero), T(j)/nsteps);
                            tangent.normalize();
                            if (tangent.length2() < 0.5f) {
                                // If didn't normalize, fall back to previous tangent,
                                // (shouldn't happen on first iteration, since tangent should be last_dir).
                                tangent = prev_tangent;
                            }
                            else {
                                prev_tangent = tangent;
                            }
                            tangents(i) = tangent;
                        }
                    }
                    break;
                }
                case TangentType::PREV:
                case TangentType::NEXT:
                {
                    UT_Vector3T<T> tangent = (tangent_type == TangentType::PREV) ? last_dir : first_dir;
                    for (GA_Size i = last_nonzero+1; i < npoints; ++i) {
                        tangents(i) = tangent;
                    }
                    for (GA_Size i = 0; i <= first_nonzero; ++i) {
                        tangents(i) = tangent;
                    }
                    break;
                }
                case TangentType::NONE:
                    UT_ASSERT_MSG(0, "This case should have been excluded earlier.");
                    break;
            }
        }
        else {
            // Closed backbone and no zero-length edges at beginning or end, so
            // it's like a normal case, except with non-adjacent edge indices.

            interpolateTangent(
                tangent_type,
                stretch_using_backbone,
                max_stretch_scale,
                max_stretch_length_threshold,
                directions.last(),
                directions(0),
                directions.last().length2(),
                directions(0).length2(),
                false, // Arbitrarily choose not last
                &tangents(0),
                stretch_using_backbone ? &stretch_dirs(0) : nullptr,
                stretch_using_backbone ? &stretch_scales(0) : nullptr,
                true); // FIXME: Support non-normalized scales!!!
        }

        // The beginning and end have been handled, and we have two non-zero-length
        // edges bounding the remaining edges.


        GA_Size prev_nonzero = first_nonzero;
        T prev_length2 = directions(first_nonzero).length2();
        for (GA_Size next_edgei = first_nonzero+1; next_edgei <= last_nonzero; ++next_edgei) {
            T next_length2 = directions(next_edgei).length2();
            if (next_length2 != 0) {
                // Common case: Two adjacent, non-zero-length edges.
                interpolateTangent(
                    tangent_type,
                    stretch_using_backbone,
                    max_stretch_scale,
                    max_stretch_length_threshold,
                    directions(prev_nonzero),
                    directions(next_edgei),
                    prev_length2,
                    next_length2,
                    false, // Arbitrarily choose not last
                    &tangents(next_edgei),
                    stretch_using_backbone ? &stretch_dirs(next_edgei) : nullptr,
                    stretch_using_backbone ? &stretch_scales(next_edgei) : nullptr,
                    true); // FIXME: Support non-normalized scales!!!
                prev_nonzero = next_edgei;
                prev_length2 = next_length2;
                continue;
            }

            // Less-common case: zero-length edge(s)
            // Find next non-zero-length edge.
            do {
                ++next_edgei;
                next_length2 = directions(next_edgei).length2();
            } while (next_edgei != last_nonzero && next_length2 == 0);

            GA_Size nsteps = next_edgei - prev_nonzero - 1;

            UT_Vector3T<T> prev_dir = directions(prev_nonzero) / SYSsqrt(prev_length2);
            UT_Vector3T<T> next_dir = directions(next_edgei) / SYSsqrt(next_length2);
            switch (tangent_type) {
                case TangentType::CIRCULAR:
                {
                    if (nsteps == 1) {
                        tangents(next_edgei-1) = prev_dir;
                        tangents(next_edgei) = next_dir;
                    }
                    else {
                        // 3+ coincident, so circularly interpolate tangent
                        T cos_angle = dot(next_dir,prev_dir);
                        UT_Vector3T<T> sin_axis = next_dir - cos_angle*prev_dir;
                        sin_axis.normalize();
                        // TODO: atan2(sin_angle,cos_angle) is more stable for small angles or half turns.
                        T full_angle = SYSacos(cos_angle);
                        GA_Size j = 0;
                        for (GA_Size i = prev_nonzero+1; i <= next_edgei; ++i, ++j) {
                            T part_angle = j*full_angle/nsteps;
                            tangents(i) = SYScos(part_angle)*prev_dir + SYSsin(part_angle)*sin_axis;
                        }
                    }
                    break;
                }
                case TangentType::SUBD:
                {
                    if (nsteps == 1) {
                        tangents(next_edgei-1) = prev_dir;
                        tangents(next_edgei) = next_dir;
                    }
                    else {
                        // 3+ coincident, so linearly interpolate tangent
                        GA_Size j = 0;
                        // NOTE: This value 0,0,1 should never end up being used.  It should be overwritten first.
                        UT_Vector3T<T> prev_tangent(0,0,1);
                        for (GA_Size i = prev_nonzero+1; i <= next_edgei; ++i, ++j) {
                            UT_Vector3T<T> tangent = SYSlerp(directions(prev_nonzero), directions(next_edgei), T(j)/nsteps);
                            tangent.normalize();
                            if (tangent.length2() < 0.5f) {
                                // If didn't normalize, fall back to previous tangent,
                                // (shouldn't happen on first iteration, since tangent should be last_dir).
                                tangent = prev_tangent;
                            }
                            else {
                                prev_tangent = tangent;
                            }
                            tangents(i) = tangent;
                        }
                    }
                    break;
                }
                case TangentType::PREV:
                case TangentType::NEXT:
                {
                    UT_Vector3T<T> tangent = (tangent_type == TangentType::PREV) ? prev_dir : next_dir;
                    for (GA_Size i = prev_nonzero+1; i <= next_edgei; ++i) {
                        tangents(i) = tangent;
                    }
                    break;
                }
                case TangentType::NONE:
                    UT_ASSERT_MSG(0, "This case should have been excluded earlier.");
                    break;
            }

            prev_nonzero = next_edgei;
            prev_length2 = next_length2;
        }
    }

    // We now have all of the tangents.
    // stretch_dirs and stretch_scales have also been initialized
    // if stretch_using_backbone is true.

    if (instance_up.isValid())
    {
        // up attribute, but we still need to force up_vectors
        // to be perpendicular to tangents.

        for (exint i = 0; i < npoints; ++i)
        {
            UT_Vector3D dir = tangents[i];
            UT_Vector3D other = up_vectors[i];
            other -= dot(other,dir)*dir;
            T length2 = other.length2();
            if (length2 >= theQuiteSmallRelativeLength2)
                other /= SYSsqrt(length2);
            else
                other.arbitraryPerp(dir);
            up_vectors[i] = other;
        }

        return;
    }

    if (use_normal_vector_up) {
        // Compute a face normal for target_up_vector, if possible.
        // This isn't the most efficient way to compute a normal, but we want
        // to try to fall back on the largest normal along the way, so we
        // go one triangle at a time.  We know that we have at least two
        // non-zero-length edges here, so there is at least one triangle,
        // even though it might have zero area.

        // Double-precision for the sum to reduce the risk of catastrophic
        // roundoff error, (e.g. if a curve had more than 16,777,216 vertices,
        // everything after that would effectively be lost in single-precision.)
        UT_Vector3D normal_sum(0,0,0);
        UT_Vector3D max_normal;
        double length2_max_normal = 0;
        T length2_max_chord = 0;
        for (GA_Size i = 1; i < nverts-1; ++i) {
            UT_Vector3T<T> posi = geo->getPos3(geo->vertexPoint(vertices(i)));
            UT_Vector3T<T> chord = posi-pos0;

            // The length of the cross product is 2x the area of the triangle
            // from 0 to i to i+1, but with the direction being the triangle's
            // normal.  When summed, they give a vector whose length is the
            // area of the polygon projected into a plane perpendicular to the
            // direction of the vector.
            normal_sum += cross(directions(i), chord);

            // Record the longest normal along the way, for a fallback.
            double length2 = normal_sum.length2();
            if (length2 > length2_max_normal) {
                length2_max_normal = length2;
                max_normal = normal_sum;
            }
            length2_max_chord = SYSmax(length2_max_chord, chord.length2());
        }
        UT_Vector3T<T> posi = geo->getPos3(geo->vertexPoint(vertices(nverts-1)));
        length2_max_chord = SYSmax(length2_max_chord, (posi-pos0).length2());

        // Double-precision to reduce risk of underflow/overflow below
        double small_rel_length2 = theQuiteSmallRelativeLength2*length2_max_chord;
        double small_rel_length4 = small_rel_length2*small_rel_length2;

        // length2_max_normal is the *square* of 2x the largest *area* along the way,
        // so compare against length-squared *squared*.
        if (length2_max_normal < small_rel_length4) {
            // Even the max length normal along the way was quite small,
            // which happens when the points are all along a line,
            // so stick with default target_up_vector.
        }
        else {
            double length2_normal = normal_sum.length2();
            if (length2_normal < small_rel_length4) {
                // normal_sum ended up being small, but it was large enough
                // partway through, so use that.  This can happen if a
                // curve backtracks along itself to the beginning.
                target_up_vector = max_normal / SYSsqrt(length2_max_normal);
            }
            else {
                // The normal's good, so use that.
                target_up_vector = normal_sum / SYSsqrt(length2_normal);
            }
        }
    }

    // We have all tangents, but we need to compute consistent up vectors.
    // First, we pick an arbitrary starting bitangent; it must be
    // perpendicular to tangents(0) and it might as well be as close to
    // perpendicular to target_up_vector as it can be.
    UT_Vector3T<T> tangent0 = tangents(0);
    UT_Vector3T<T> up_vectors0 = target_up_vector;
    up_vectors0 -= dot(up_vectors0,tangent0)*tangent0;
    T length2 = up_vectors0.length2();
    if (length2 >= theQuiteSmallRelativeLength2) {
        up_vectors0 /= SYSsqrt(length2);
    }
    else {
        up_vectors0.arbitraryPerp(tangent0);
    }
    up_vectors(0) = up_vectors0;

    for (GA_Size i = 1; i < npoints; ++i) {
        // Compute the minimal rotation matrix that takes tangents(i-1) to tangents(i).
        // Vectors in tangents are already normalized, so no need to normalize inside (false).
        // If this is a bottleneck, it's possible to compute the resulting vector
        // without constructing the matrix itself, via:
        // <v|(I-2|a><a|)(I-2|c><c|), where c is (a+b) normalized, and a and b are the tangents.
        // v -= (2*dot(v,a))*a;
        // v -= (2*dot(v,c))*c;
        UT_Matrix3T<T> rotation;
        bool failure = rotation.dihedral(tangents(i-1), tangents(i), false);
        if (failure) {
            // The tangent flipped a half turn, so flip the bitangent a half turn,
            // to keep the up vector the same.
            up_vectors(i) = -up_vectors(i-1);
        }
        else {
            // Rotate bitangent by the rotation matrix.
            up_vectors(i) = up_vectors(i-1) * rotation;
        }

        // Just to prevent roundoff error from getting bad, force up_vectors(i) to be orthogonal to tangents(i).
        up_vectors(i) -= dot(up_vectors(i),tangents(i))*tangents(i);
        // It seems implausible that up_vectors(i) could possibly be problematically small
        // after the subtraction, since it should already have been approximately
        // orthogonal to tangents(i) and unit length, so let's take the risk and just normalize.
        up_vectors(i).normalize();
    }

    // Closed backbone curves need an adjustment to make sure that beginning and end
    // up_vectors line up, without a sudden roll rotation.  In other words,
    // smooth the excess roll out over the whole curve.
    // Open curves with an end target up vector need the same type of adjustment.
    if ((closed && continuous_closed_curves) || use_end_target_up_vector)
    {
        UT_Vector3T<T> final_up_vector;
        UT_Vector3T<T> comparison_tangent;
        bool skip_twist = false;
        if (closed)
        {
            // Same as above regarding not really needing to compute this matrix
            // if it's a bottleneck.
            UT_Matrix3T<T> rotation;
            bool failure = rotation.dihedral(tangents[npoints-1], tangents[0], false);
            if (failure)
            {
                // The tangent flipped a half turn, so keep the up vector the same.
                // The bitangent will implicitly flip a half turn.
                final_up_vector = up_vectors[npoints-1];
            }
            else
            {
                // Rotate up vectors by the rotation matrix.
                final_up_vector = up_vectors[npoints-1] * rotation;
            }
            // Just to prevent roundoff error from getting bad, force final_up_vector to be orthogonal to tangents[0].
            final_up_vector -= dot(final_up_vector,tangents[0])*tangents[0];
            // It seems implausible that final_up_vector could possibly be problematically small
            // after the subtraction, since it should already have been approximately
            // orthogonal to tangents[0] and unit length, so let's take the risk and just normalize.
            final_up_vector.normalize();

            comparison_tangent = tangents[0];
        }
        else
        {
            // Open curve, so final up vector is already in the array.
            final_up_vector = up_vectors[npoints-1];
            comparison_tangent = tangents[npoints-1];
        }

        UT_Vector3T<T> comparison_up_vector;
        if (closed && continuous_closed_curves)
        {
            comparison_up_vector = up_vectors[0];
        }
        else
        {
            // Using an end target up vector.
            if (use_normal_vector_up)
            {
                // Normal vector was written into target_up_vector above.
                comparison_up_vector = target_up_vector;
            }
            else
                comparison_up_vector = end_target_up_vector;
            comparison_up_vector -= comparison_up_vector*dot(comparison_up_vector,comparison_tangent);
            T length2 = comparison_up_vector.length2();
            // If the target up vector is extremely close to the tangent, skip applying a twist.
            if (length2 >= theQuiteSmallRelativeLength2)
                comparison_up_vector /= SYSsqrt(length2);
            else
                skip_twist = true;
        }

        T net_twist = T(0);
        if (!skip_twist)
        {
            // NOTE: The cosine between the start and end up_vectors is
            //       not sufficient to get the direction of the net twist.
            //       We need to consider the direction turned relative to
            //       the tangent.  Thus, we need to use the cross product
            //       between the two dotted against the tangent to get
            //       the correct sine of the angle.
            UT_Vector3T<T> net_twist_vector = cross(final_up_vector, comparison_up_vector);
            T sin_net_twist = dot(net_twist_vector, comparison_tangent);
            T cos_net_twist = dot(final_up_vector, comparison_up_vector);
            net_twist = SYSatan2(sin_net_twist, cos_net_twist); // atan2(y,x) order
        }

        if (!SYSequalZero(net_twist, theQuiteSmallRelativeLength))
        {
            // Apply twist around the loop
            if (twist_per == RotationPer::EDGE || twist_per == RotationPer::FULLEDGES)
            {
                for (GA_Size i = 1; i < npoints; ++i)
                {
                    T reverse_twist = -i*net_twist/nedges;
                    T s = SYSsin(reverse_twist);
                    T c = SYScos(reverse_twist);
                    UT_Vector3T<T> bitangent = cross(up_vectors(i), tangents(i));
                    up_vectors(i) = s*bitangent + c*up_vectors(i);
                }
            }
            else if (twist_per == RotationPer::DISTANCE || twist_per == RotationPer::FULLDISTANCE ||
                (twist_per == RotationPer::ATTRIB && (
                    !roll_attrib.isValid() ||
                    roll_attrib->getOwner() == GA_ATTRIB_PRIMITIVE ||
                    roll_attrib->getOwner() == GA_ATTRIB_DETAIL)))
            {
                double total_length = 0;
                for (GA_Size i = 0; i < nedges; ++i)
                {
                    total_length += directions(i).length();
                }

                // NOTE: total_length shouldn't be zero, since we already ensured
                //       that we have at least two non-zero-length edges.
                double cur_length = 0;
                for (GA_Size i = 1; i < npoints; ++i)
                {
                    cur_length += directions(i-1).length();
                    T reverse_twist = -cur_length*net_twist/total_length;
                    T s = SYSsin(reverse_twist);
                    T c = SYScos(reverse_twist);
                    UT_Vector3T<T> bitangent = cross(up_vectors(i), tangents(i));
                    up_vectors(i) = s*bitangent + c*up_vectors(i);
                }
            }
            else
            { // twist_per == RotationPer::ATTRIB
                if (roll_attrib->getOwner() == GA_ATTRIB_VERTEX)
                {
#if 0
                    T2 ustart = 0;
                    T2 uend = 1;
                    if (nverts == npoints+1)
                    {
                        // We can really only use start and end uvs reliably
                        // if the curve is unrolled, (1 extra vertex).
                        // Otherwise, we default to assuming 0 to 1.
                        ustart = roll_attrib.get(vertices(0), ucomponent);
                        uend = roll_attrib.get(vertices.last(), ucomponent);
                    }
                    T2 uspan = (uend-ustart);
#endif

                    //if (uspan != T2(0))
                    //{
                        for (GA_Size i = 1; i < npoints; ++i)
                        {
                            T2 t = roll_attrib.get(vertices(i), ucomponent);
                            //T2 t = (u-ustart)/uspan;
                            T reverse_twist = -t*net_twist;
                            T s = SYSsin(reverse_twist);
                            T c = SYScos(reverse_twist);
                            UT_Vector3T<T> bitangent = cross(up_vectors(i), tangents(i));
                            up_vectors(i) = s*bitangent + c*up_vectors(i);
                        }
                    //}
                }
                else
                {
                    UT_ASSERT(roll_attrib->getOwner() == GA_ATTRIB_POINT);
                    // Assume 0 to 1, since it won't end correctly and we have to guess.

                    for (GA_Size i = 1; i < npoints; ++i)
                    {
                        T2 u = roll_attrib.get(geo->vertexPoint(vertices(i)), ucomponent);
                        T reverse_twist = -u*net_twist;
                        T s = SYSsin(reverse_twist);
                        T c = SYScos(reverse_twist);
                        UT_Vector3T<T> bitangent = cross(up_vectors(i), tangents(i));
                        up_vectors(i) = s*bitangent + c*up_vectors(i);
                    }
                }
            }

            if (closed && continuous_closed_curves)
            {
                // The twist applied so far is the negative of the original net twist.
                total_twist_around_loop = -net_twist;
            }
        }
    }

    if (target_up_vector_at_start)
    {
        // Nothing else to do if target up vector is okay being at the start,
        // since that's what we used as our initial guess.
        return;
    }

    // Now that we have consistent up_vectors, we make them more stable by
    // making the average closer to perpendicular to the backbone polygon normal,
    // by introducing a global roll rotation.

    // This sounds pretty nebulous, but the math simplifies down nicely.
    // We want to find the z-axis rotation Rz such that
    // [0 1 0] * Rz * M
    // is in the direction of target_up_vector,
    // where M is the current average rotation matrix.
    // (Note that M itself likely isn't a rotation matrix, may not
    // have orthogonal rows, and may even be singular or near-singular.)
    // Depending on which direction we choose as the angle, this is:
    // [sin(theta) cos(theta) 0] * M
    // = [s c 0] * M
    // = s*Mx + c*My
    // If we solve:
    // [Mx.Mx Mx.My][x]   [Mx.up]
    // [Mx.My My.My][y] = [My.up]
    // we'll get the vector that is least-squares closest to up
    // that can be made using Mx and My, as a linear combination of Mx and My.
    // If we normalize [x y], that gives us a valid [s c].  Then, we can apply
    // Rz = [ c -s  0]
    //      [ s  c  0]
    //      [ 0  0  1]
    // on the left of each frame.
    // Of course, if Mx and My are parallel, or one of the two is much shorter
    // than the other, (or equivalently, if the determinant of the 2x2 matrix above
    // is small compared to the length-squared-squared of Mx and My), we need to handle
    // things differently.

    // Find Mx and My, (Mz isn't needed), in double-precision to avoid
    // catastrophic roundoff error in the accumulation.
    UT_Vector3D Mx(0,0,0);
    UT_Vector3D My(0,0,0);
    for (GA_Size i = 0; i < npoints; ++i) {
        My += up_vectors(i);
        UT_Vector3T<T> bitangent = cross(up_vectors(i), tangents(i));
        Mx += bitangent;
    }
    // We don't strictly need to divide by npoints for the average,
    // since we'll be normalizing later, and there's little risk of overflow
    // using double-precision.

    double MxMx = dot(Mx,Mx);
    double MxMy = dot(Mx,My);
    double MyMy = dot(My,My);
    double Mxup = dot(Mx,target_up_vector);
    double Myup = dot(My,target_up_vector);

    // Part of this is effectively the same as UT_Matrix2T::solve
    // Side note: via awesome properties of cross & dot products,
    //            determinant is equal to (Mx x My).(Mx x My),
    //            so it's the length-squared of the cross product.
    double determinant = MxMx*MyMy - MxMy*MxMy;
    double scale2 = SYSmax(MxMx*MxMx, MyMy*MyMy);
    T s; T c;
    // 1e-6 is because if Mx is 1000x longer than My and they're in the ballpark
    // of orthogonal, My is almost certainly due to slightly imbalanced vectors
    // that should really cancel out exactly.  If they're in the ballpark of
    // parallel and the non-parallel component of My is 1000x smaller than Mx,
    // they're effectively the same.
    // In either case, that yields determinant/scale2 ~ 1e-6
    if (!SYSequalZero(determinant, 1e-6*scale2)) {
        // Not near singular, so we can solve it.
        // We'd normally divide by determinant to solve a 2x2 system,
        // but we're normalizing after anyway.
        s = (MyMy*Mxup - MxMy*Myup);
        c = (MxMx*Myup - MxMy*Mxup);
        T normalization = s*s + c*c;
        if (SYSequalZero(normalization, theExtremelySmallLength2)) {
            s = 0;
            c = 1;
        }
        else {
            normalization = T(1) / SYSsqrt(normalization);
            s *= normalization;
            c *= normalization;
        }
    }
    else if (MxMx > MyMy) {
        s = (Mxup >= 0) ? 1 : -1;
        c = 0;
    }
    else {
        s = 0;
        c = (Myup >= 0) ? 1 : -1;
    }

    // New up vector =
    //         [ c -s  0] [<--Mx-->]           [cMx-sMy ]
    // [0 1 0] [ s  c  0] [<--My-->] = [0 1 0] [sMx+cMy ] = [sMx+cMy ]
    //         [ 0  0  1] [<--Mz-->]           [<--Mz-->]

    for (GA_Size i = 0; i < npoints; ++i) {
        UT_Vector3T<T> bitangent = cross(up_vectors(i), tangents(i));
        up_vectors(i) = s*bitangent + c*up_vectors(i);
    }

    // Finally done computing up_vectors!  Yay! :)
}

bool
getPolyProperties(
    const GEO_Detail *geometry,
    const GA_OffsetListRef &vertices,
    exint &nedges,
    bool &closed,
    bool &unrolled)
{
    if (vertices.size() == 0) {
        // Skip cross-sections with zero vertices
        return false;
    }
    if (vertices.size() == 1) {
        // Always treat single-vertex polygons as open,
        // to try to reduce confusion.
        closed = false;
        unrolled = false;
        nedges = 0;
        return true;
    }

    closed = vertices.getExtraFlag();
    unrolled = false;
    nedges = vertices.size();

    if (!closed) {
        --nedges;

        GA_Offset pt0 = geometry->vertexPoint(vertices(0));
        GA_Offset lastpt = geometry->vertexPoint(vertices.last());
        // If first and last point are the same, treat it as if it's closed
        // in most cases, but we want to record that it's an unrolled curve,
        // so that the row and col cases treat it as open with a shared point.
        unrolled = (pt0 == lastpt);
        closed = unrolled;
    }
    return true;
}

template<typename T>
static void
createRotationMatrix(
    UT_Matrix4T<T> &transform,
    const UT_Vector3T<T> &cur_angles,
    const UT_Axis3::axis order[3])
{
    transform.identity();

    for (int i = 0; i < 3; ++i)
    {
        float angle = cur_angles[i];
        if (angle == 0)
            continue;
        transform.rotate(order[i], angle);
    }
}

template<typename T>
void
computeCurveTransforms(
    const GEO_Detail *const geo,
    const GA_PrimitiveGroup *curve_group,
    const GA_RWHandleT<UT_Matrix4T<T>> &transform_attrib,
    const CurveFrameParms<T> &parms)
{
    UT_AutoInterrupt interrupt("Computing curve transforms");
    if (interrupt.wasInterrupted())
        return;

    UT_ASSERT(transform_attrib.isValid());
    if (!transform_attrib.isValid())
        return;

    UT_ASSERT(transform_attrib->getOwner() == GA_ATTRIB_VERTEX);

    // We're going to be writing to tangent_attrib in parallel, and although
    // we're using GA_SplittableRange, we're splitting over primitive pages,
    // not vertex pages, so we have to harden in advance.
    transform_attrib->hardenAllPages();

    bool rotate_using_backbone = parms.myTangentType != TangentType::NONE;
    UT_Vector3T<T> target_up_vector = parms.myTargetUpVector;
    UT_Vector3T<T> end_target_up_vector = parms.myEndTargetUpVector;
    if (rotate_using_backbone)
    {
        // Normalize up_vector
        T length2 = target_up_vector.length2();
        if (length2 < theExtremelySmallLength2)
        {
            // Fall back to 0,1,0 if need be.
            target_up_vector.assign(0,1,0);
        }
        else
        {
            target_up_vector /= SYSsqrt(length2);
        }
        if (parms.myUseEndTargetUpVector)
        {
            // Normalize end up_vector
            length2 = end_target_up_vector.length2();
            if (length2 < theExtremelySmallLength2)
            {
                // Fall back to 0,1,0 if need be.
                end_target_up_vector.assign(0,1,0);
            }
            else
            {
                end_target_up_vector /= SYSsqrt(length2);
            }
        }
    }

    // If we're transforming by instance transform attributes,
    // e.g. N, up, pscale, scale, rot, orient, pivot, trans, xform,
    // set up the cache of those attributes.
    GA_AttributeInstanceMatrix instance_attribs;
    GA_ROHandleV3D instance_N;
    GA_ROHandleV3D instance_up;
    bool transform_by_instance_attribs = parms.myTransformByInstanceAttribs;
    if (transform_by_instance_attribs)
    {
        instance_attribs.initialize(geo->pointAttribs());
        if (parms.myNormalizeScales)
            instance_attribs.resetScales();

        // If there's only one of either N or up, we want to handle those separately.
        bool has_N = instance_attribs.myN.isValid();
        bool has_up = instance_attribs.myUp.isValid();
        if (has_N && !has_up)
        {
            instance_N = instance_attribs.myN;
            instance_attribs.myN.clear();
        }
        else if (has_up && !has_N)
        {
            instance_up = instance_attribs.myUp;
            instance_attribs.myUp.clear();
        }

        if (!instance_attribs.hasAnyAttribs())
            transform_by_instance_attribs = false;

        // We don't want to rotate both based on the backbone curves
        // and based on attributes, but we may still want to use pscale
        // with the backbone curve rotations.
        rotate_using_backbone &= !instance_attribs.hasNonScales();
    }

    const bool use_rotation_attrib[3] =
    {
        parms.myIncAnglePer[0] == RotationPer::ATTRIB && parms.myRotAttribs[0].isValid() && parms.myIncAngles[0] != 0.0,
        parms.myIncAnglePer[1] == RotationPer::ATTRIB && parms.myRotAttribs[1].isValid() && parms.myIncAngles[1] != 0.0,
        parms.myIncAnglePer[2] == RotationPer::ATTRIB && parms.myRotAttribs[2].isValid() && parms.myIncAngles[2] != 0.0
    };
    const GA_AttributeOwner rot_attrib_owner[3] =
    {
        use_rotation_attrib[0] ? parms.myRotAttribs[0]->getOwner() : GA_ATTRIB_INVALID,
        use_rotation_attrib[1] ? parms.myRotAttribs[1]->getOwner() : GA_ATTRIB_INVALID,
        use_rotation_attrib[2] ? parms.myRotAttribs[2]->getOwner() : GA_ATTRIB_INVALID
    };
    const bool varying_attrib[3] =
    {
        rot_attrib_owner[0] == GA_ATTRIB_POINT || rot_attrib_owner[0] == GA_ATTRIB_VERTEX,
        rot_attrib_owner[1] == GA_ATTRIB_POINT || rot_attrib_owner[1] == GA_ATTRIB_VERTEX,
        rot_attrib_owner[2] == GA_ATTRIB_POINT || rot_attrib_owner[2] == GA_ATTRIB_VERTEX
    };

    UT_Axis3::axis order[3];
    switch (parms.myRotationOrder)
    {
        case UT_XformOrder::XYZ: order[0] = UT_Axis3::XAXIS; order[1] = UT_Axis3::YAXIS; order[2] = UT_Axis3::ZAXIS; break;
        case UT_XformOrder::XZY: order[0] = UT_Axis3::XAXIS; order[1] = UT_Axis3::ZAXIS; order[2] = UT_Axis3::YAXIS; break;
        case UT_XformOrder::YXZ: order[0] = UT_Axis3::YAXIS; order[1] = UT_Axis3::XAXIS; order[2] = UT_Axis3::ZAXIS; break;
        case UT_XformOrder::YZX: order[0] = UT_Axis3::YAXIS; order[1] = UT_Axis3::ZAXIS; order[2] = UT_Axis3::XAXIS; break;
        case UT_XformOrder::ZXY: order[0] = UT_Axis3::ZAXIS; order[1] = UT_Axis3::XAXIS; order[2] = UT_Axis3::YAXIS; break;
        case UT_XformOrder::ZYX: order[0] = UT_Axis3::ZAXIS; order[1] = UT_Axis3::YAXIS; order[2] = UT_Axis3::XAXIS; break;
    }

    // Loop over primitives in parallel, using a splittable range and a lambda functor.
    UTparallelFor(GA_SplittableRange(geo->getPrimitiveRange(curve_group)),
        [geo,&parms,transform_by_instance_attribs,&instance_N,&instance_up,
        &instance_attribs,rotate_using_backbone,target_up_vector,end_target_up_vector,
        &transform_attrib,&interrupt,use_rotation_attrib,rot_attrib_owner,varying_attrib,
        &order](const GA_SplittableRange &r)
    {
        // An array to help with computing backbone rotations
        UT_SmallArray<UT_Vector3T<T>, 8*sizeof(UT_Vector3T<T>)> directions;
        UT_SmallArray<UT_Vector3T<T>, 8*sizeof(UT_Vector3T<T>)> tangents;
        UT_SmallArray<UT_Vector3T<T>, 8*sizeof(UT_Vector3T<T>)> up_vectors;
        UT_SmallArray<UT_Vector3T<T>, 8*sizeof(UT_Vector3T<T>)> stretch_dirs;
        UT_SmallArray<T, 8*sizeof(T)> stretch_scales;

        // Shortest length of edgedira+edgedirb before max_stretch_scale must be applied
        const T max_stretch_length_threshold = 2.0f / parms.myMaxStretchScale;

        const bool rotate_using_parameters = !parms.myAngles.isZero() || !parms.myIncAngles.isZero();

        const int curve_u_component(parms.myRotAttribComponent);

        // Inside the functor, we have a sub-range of primitives, so loop over that range.
        // We use blockAdvance, instead of !it.atEnd() and ++it, for less looping overhead.
        GA_Offset start; GA_Offset end;
        for (GA_Iterator it(r); it.blockAdvance(start,end); )
        {
            // We probably don't need to check for interruption on every curve,
            // (unless people put in a curve with millions of vertices), so we
            // can check it once for every contiguous block of up to GA_PAGE_SIZE
            // primitive offsets.
            if (interrupt.wasInterrupted())
                return;

            // Loop over all primitives in this contiguous block of offsets.
            for (GA_Offset primoff = start; primoff < end; ++primoff)
            {
                const GA_OffsetListRef vertices = geo->getPrimitiveVertexList(primoff);
                GA_Size nedges;
                bool closed;
                bool unrolled;
                bool nonempty = getPolyProperties(geo, vertices, nedges, closed, unrolled);
                const GA_Size npoints = nedges + !closed;
                const GA_Size nverts = vertices.size();

                // Nothing to do if no vertices
                if (!nonempty)
                    continue;

                UT_Vector3T<T> local_target_up_vector = target_up_vector;
                UT_Vector3T<T> local_end_target_up_vector = end_target_up_vector;
                if (rotate_using_backbone && parms.myTargetUpVectorAttrib.isValid())
                {
                    UT_ASSERT(parms.myTargetUpVectorAttrib->getOwner() == GA_ATTRIB_PRIMITIVE);
                    local_target_up_vector = parms.myTargetUpVectorAttrib.get(primoff);

                    // Normalize up_vector
                    T length2 = local_target_up_vector.length2();
                    if (length2 < theExtremelySmallLength2)
                    {
                        // Fall back to 0,1,0 if need be.
                        local_target_up_vector.assign(0,1,0);
                    }
                    else
                    {
                        local_target_up_vector /= SYSsqrt(length2);
                    }
                }
                if (rotate_using_backbone && parms.myUseEndTargetUpVector && parms.myEndTargetUpVectorAttrib.isValid())
                {
                    UT_ASSERT(parms.myEndTargetUpVectorAttrib->getOwner() == GA_ATTRIB_PRIMITIVE);
                    local_end_target_up_vector = parms.myEndTargetUpVectorAttrib.get(primoff);

                    // Normalize end up_vector
                    T length2 = local_end_target_up_vector.length2();
                    if (length2 < theExtremelySmallLength2)
                    {
                        // Fall back to 0,1,0 if need be.
                        local_end_target_up_vector.assign(0,1,0);
                    }
                    else
                    {
                        local_end_target_up_vector /= SYSsqrt(length2);
                    }
                }

                bool local_rotate_using_backbone = rotate_using_backbone;
                bool local_stretch_using_backbone = parms.myStretchAroundTurns;

                // If the curve is closed, we may have to introduce a partial twist
                // around the loop, instead of having the twist all at one point.
                // This is used to adjust how much additional twist to apply.
                T total_twist_around_loop = 0;

                const bool continuous_closed = closed && parms.myContinuousClosedCurves;

                // FIXME: Add parameter for use_nurbs_tangents once implemented!!!
                bool use_nurbs_tangents = false;

                RotationPer local_dangle_per[3] =
                {
                    parms.myIncAnglePer[0],
                    parms.myIncAnglePer[1],
                    parms.myIncAnglePer[2]
                };
                UT_Vector3T<T> local_dangles = parms.myIncAngles;

                bool needs_length = false;
                bool needs_total_length = false;
                if (rotate_using_parameters)
                {
                    needs_length =
                        (local_dangle_per[0] == RotationPer::FULLDISTANCE || local_dangle_per[0] == RotationPer::DISTANCE) ||
                        (local_dangle_per[1] == RotationPer::FULLDISTANCE || local_dangle_per[1] == RotationPer::DISTANCE) ||
                        (local_dangle_per[2] == RotationPer::FULLDISTANCE || local_dangle_per[2] == RotationPer::DISTANCE);
                    needs_total_length =
                        (local_dangle_per[0] == RotationPer::FULLDISTANCE || (continuous_closed && local_dangle_per[0] == RotationPer::DISTANCE)) ||
                        (local_dangle_per[1] == RotationPer::FULLDISTANCE || (continuous_closed && local_dangle_per[1] == RotationPer::DISTANCE)) ||
                        (local_dangle_per[2] == RotationPer::FULLDISTANCE || (continuous_closed && local_dangle_per[2] == RotationPer::DISTANCE));
                }
                if (parms.myScaleRamp)
                {
                    needs_length = true;
                    needs_total_length = true;
                }

                computeSingleBackboneFrames(
                    local_rotate_using_backbone,
                    local_stretch_using_backbone,
                    needs_length,
                    directions, tangents, up_vectors,
                    stretch_dirs, stretch_scales,
                    total_twist_around_loop,
                    geo, nedges, npoints, nverts, closed,
                    parms.myTangentType, parms.myExtrapolateEndTangents,
                    instance_N,
                    primoff, vertices, use_nurbs_tangents,
                    parms.myMaxStretchScale, max_stretch_length_threshold,
                    instance_up,
                    local_target_up_vector, parms.myUseCurveNormalAsTargetUp,
                    parms.myTargetUpVectorAtStart, continuous_closed,
                    parms.myUseEndTargetUpVector, local_end_target_up_vector,
                    parms.myIncAnglePer[2],
                    parms.myRotAttribs[2], curve_u_component);

                UT_Vector3T<T> local_angles(0,0,0);
                if (use_rotation_attrib[0] && !varying_attrib[0])
                {
                    GA_Offset offset = (rot_attrib_owner[0] == GA_ATTRIB_PRIMITIVE) ? primoff : GA_DETAIL_OFFSET;
                    local_angles[0] = local_dangles[0]*parms.myRotAttribs[0].get(offset);
                }
                if (use_rotation_attrib[1] && !varying_attrib[1])
                {
                    GA_Offset offset = (rot_attrib_owner[1] == GA_ATTRIB_PRIMITIVE) ? primoff : GA_DETAIL_OFFSET;
                    local_angles[1] = local_dangles[1]*parms.myRotAttribs[1].get(offset);
                }
                if (use_rotation_attrib[2] && !varying_attrib[2])
                {
                    GA_Offset offset = (rot_attrib_owner[2] == GA_ATTRIB_PRIMITIVE) ? primoff : GA_DETAIL_OFFSET;
                    local_angles[2] = local_dangles[2]*parms.myRotAttribs[2].get(offset);
                }

                // Double-precision accumulators to avoid catastrophic roundoff error
                // for curves with > 16,777,216 vertices, (i.e. 2^24).
                double total_length = 0;
                double cur_length = 0;
                if (needs_total_length)
                {
                    for (GA_Size i = 0; i < nedges; ++i)
                    {
                        total_length += directions(i).length();
                    }
                }

                // For closed curves, the total twist must be a multiple of a full turn,
                // so we have to round it, but carefully.
                const bool rotate_using_dangles = !local_dangles.isZero();
                if (rotate_using_parameters && continuous_closed && rotate_using_dangles)
                {
                    for (int axis = 0; axis < 3; ++axis)
                    {
                        if (local_dangle_per[axis] == RotationPer::EDGE)
                        {
                            local_dangles[axis] *= nedges;
                            local_dangle_per[axis] = RotationPer::FULLEDGES;
                        }
                        else if (local_dangle_per[axis] == RotationPer::DISTANCE)
                        {
                            local_dangles[axis] *= total_length;
                            local_dangle_per[axis] = RotationPer::FULLDISTANCE;
                        }
                    }

                    // We have a separate twist due to the base frame needing to line up
                    // with itself after going around the loop, so we want to take into
                    // account that it'll be applied anyway.
                    local_dangles.z() -= total_twist_around_loop;
                    total_twist_around_loop = 0;

                    // Round angles to multiples of a full turn
                    local_dangles /= (2*M_PI);
                    local_dangles.x() = SYSrint(local_dangles.x());
                    local_dangles.y() = SYSrint(local_dangles.y());
                    local_dangles.z() = SYSrint(local_dangles.z());
                    local_dangles *= (2*M_PI);
                }

                // NOTE: Although we're really iterating over vertices,
                //       we don't need to write to the last vertex of an
                //       unrolled curve (open but last point same as first),
                //       and the code above figured out backbone rotations
                //       and stretching for npoints locations.
                for (GA_Size i = 0; i < npoints; ++i)
                {
                    GA_Offset vtxoff = vertices(i);
                    GA_Offset ptoff = geo->vertexPoint(vtxoff);

                    UT_Vector3T<T> cur_angles = parms.myAngles+local_angles;

                    // Apply the parameter rotations before any backbone or
                    // attribute rotations, because they're rotations relative
                    // to those transformed bases, as if the cross section was
                    // rotated before applying this node.
                    if (rotate_using_dangles)
                    {
                        for (int axis = 0; axis < 3; ++axis)
                        {
                            if (axis != 2 && local_dangles[axis] == 0)
                                continue;

                            const RotationPer cur_dangle_per = local_dangle_per[axis];
                            if (cur_dangle_per == RotationPer::EDGE)
                            {
                                cur_angles[axis] += local_dangles[axis]*i;

                                // Remove portion of twist added to make closed curve cycle smoothly.
                                if (axis == 2 && total_twist_around_loop != 0)
                                {
                                    T t = T(i)/nedges;
                                    cur_angles.z() -= total_twist_around_loop*t;
                                }
                            }
                            else if (cur_dangle_per == RotationPer::FULLEDGES)
                            {
                                T t = T(i)/nedges;
                                cur_angles[axis] += local_dangles[axis]*t;

                                // Remove portion of twist added to make closed curve cycle smoothly.
                                if (axis == 2)
                                    cur_angles.z() -= total_twist_around_loop*t;
                            }
                            else if (cur_dangle_per == RotationPer::DISTANCE)
                            {
                                cur_angles[axis] += local_dangles[axis]*cur_length;

                                if (axis == 2 && total_length != 0 && total_twist_around_loop != 0)
                                {
                                    // Remove portion of twist added to make closed curve cycle smoothly.
                                    T t = (cur_length/total_length);
                                    cur_angles.z() -= total_twist_around_loop*t;
                                }
                            }
                            else if (cur_dangle_per == RotationPer::FULLDISTANCE)
                            {
                                // NOTE: total_length could be zero, but cur_length can't be more
                                //       than total_length, so checking for zero exactly suffices.
                                if (total_length != 0)
                                {
                                    T t = (cur_length/total_length);
                                    cur_angles[axis] += t*local_dangles[axis];

                                    // Remove portion of twist added to make closed curve cycle smoothly.
                                    if (axis == 2)
                                        cur_angles.z() -= total_twist_around_loop*t;
                                }
                            }
                            else if (use_rotation_attrib[axis] && varying_attrib[axis])
                            {
                                UT_ASSERT_P(cur_dangle_per == RotationPer::ATTRIB);
                                if (rot_attrib_owner[axis] == GA_ATTRIB_VERTEX)
                                {
#if 0
                                    T ustart = 0;
                                    T uend = 1;
                                    if (!closed || unrolled) {
                                        // We can really only use start and end uvs reliably if the
                                        // curve is open or unrolled, (separate start and end vertices).
                                        // Otherwise, we default to assuming 0 to 1.
                                        ustart = parms.myRotAttribs[axis].get(vertices(0), curve_u_component);
                                        uend = parms.myRotAttribs[axis].get(vertices.last(), curve_u_component);
                                    }
                                    T uspan = (uend-ustart);
#endif

                                    T t = parms.myRotAttribs[axis].get(vtxoff, curve_u_component);

                                    //if (uspan != 0) {
                                    //    T t = (u-ustart)/uspan;
                                        cur_angles[axis] += t*local_dangles[axis];

                                        // Remove portion of twist added to make closed curve cycle smoothly.
                                        if (axis == 2)
                                            cur_angles.z() -= total_twist_around_loop*t;
                                    //}
                                }
                                else
                                {
                                    UT_ASSERT(rot_attrib_owner[axis] == GA_ATTRIB_POINT);
                                    // Assume 0 to 1, since it won't end correctly and we have to guess.

                                    T u = parms.myRotAttribs[axis].get(ptoff, curve_u_component);
                                    cur_angles[axis] += u*local_dangles[axis];

                                    // Remove portion of twist added to make closed curve cycle smoothly.
                                    if (axis == 2)
                                        cur_angles.z() -= total_twist_around_loop*u;
                                }
                            }
                        }
                    }

                    UT_Matrix4T<T> transform;
                    if (rotate_using_parameters)
                    {
                        createRotationMatrix(transform, cur_angles, order);
                    }
                    else
                    {
                        transform.identity();
                    }

                    if (transform_by_instance_attribs)
                    {
                        UT_Matrix4T<T> instance_transform;
                        // Save post-translate P for until after backbone rotation
                        // or scale might be applied.
                        instance_attribs.getMatrix(instance_transform, UT_Vector3T<T>(0,0,0), ptoff);

                        transform *= instance_transform;
                    }

                    if (local_rotate_using_backbone)
                    {
                        UT_Vector3T<T> zaxis = tangents(i);
                        UT_Vector3T<T> yaxis = up_vectors(i);
                        UT_Vector3T<T> xaxis = cross(yaxis, zaxis);
                        UT_Matrix3T<T> rotation(xaxis, yaxis, zaxis);
                        transform *= rotation;
                    }

                    // Apply the stretch vector (to avoid collapsing on turns)
                    // after any rotations, since it should always be applied
                    // in the same final direction.
                    if (local_stretch_using_backbone)
                    {
                        UT_Matrix3T<T> stretch_matrix;
                        stretch_matrix.identity();
                        stretch_matrix.outerproductUpdateT(stretch_scales(i)-1, stretch_dirs(i), stretch_dirs(i));

                        transform *= stretch_matrix;
                    }

                    fpreal scale = parms.myUniformScale;
                    if (parms.myScaleRamp)
                    {
                        const T t = (total_length != 0) ? (cur_length/total_length) : T(i)/nedges;
                        float vals[4];
                        parms.myScaleRamp->rampLookup(t, vals);
                        scale *= vals[0];
                    }

                    // Apply the overall scale
                    if (scale != 1)
                    {
                        // NOTE: We don't want to just do "transform *= scale",
                        //       because we don't want to scale the w components
                        //       of any rows, else translate below will be skewed.
                        transform.scale(scale);
                    }

                    transform.translate(geo->getPos3(ptoff));

                    transform_attrib.set(vtxoff, transform);

                    if (needs_length && i+1 < npoints)
                        cur_length += directions(i).length();
                }
            }
        }
    });
}

#define TEMPLATE_INST2(T) \
template void computeCurveTransforms<T>( \
    const GEO_Detail *const geo, \
    const GA_PrimitiveGroup *curve_group, \
    const GA_RWHandleT<UT_Matrix4T<T>> &transform_attrib, \
    const CurveFrameParms<T> &parms); \
/* end of macro */

TEMPLATE_INST2(fpreal32)
TEMPLATE_INST2(fpreal64)

}

} // End of HDK_Sample namespace
