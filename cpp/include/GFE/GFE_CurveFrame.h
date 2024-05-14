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
 * Declarations of functions for computing reference frames
 * for curve vertices.
 */

#pragma once

#include <GEO/GEO_Detail.h>
#include <GA/GA_ElementGroup.h>
#include <GA/GA_Handle.h>
#include <GA/GA_Iterator.h>
#include <GA/GA_OffsetList.h>
#include <GA/GA_SplittableRange.h>
#include <GA/GA_Types.h>
#include <UT/UT_Interrupt.h>
#include <UT/UT_ParallelUtil.h>
#include <UT/UT_Matrix4.h>
#include <UT/UT_Vector3.h>
#include <UT/UT_Vector4.h>
#include <SYS/SYS_Inline.h>
#include <SYS/SYS_Types.h>

class GA_PrimitiveGroup;

namespace HDK_Sample {

namespace GU_CurveFrame {

static constexpr int theCurveUVComponent = 0;           // u.  FIXME: Should this be v?
static constexpr int theCrossSectionUVComponent = 1;    // v.  FIXME: Should this be u?

enum TangentType
{
    /// Average of normalized direction vectors of adjacent edges.
    /// Extrapolation rotates tangent from prev/next edge as much
    /// as prev/next tangent rotated to that edge.
    CIRCULAR,

    /// Central difference of (unnormalized) adjacent edges, e.g.
    /// computing velocity from motion trails.
    /// This is the tangent that a subdivision curve would have at each point.
    /// Extrapolation applies half the difference between the two
    /// prev/next edges to the prev/next edge to get the tangent.
    SUBD,

    /// Tangent is the previous edge.
    /// Extrapolation (only back from the beginning of the curve) applies the
    /// negative difference between the two next edges to the next edge.
    PREV,

    /// Tangent is the next edge.
    /// Extrapolation (only forward from the end of the curve) applies the
    /// difference between the two prev edges to the prev edge.
    NEXT,

    /// Tangent is the Z axis (ignoring the curve tangent).
    /// Extrapolation does nothing.
    NONE
};

enum RotationPer
{
    /// Apply angle increment to each edge on top of previous edge's rotation
    EDGE,

    /// Apply angle increment times edge length to each edge on top of previous
    /// edge's rotation.
    DISTANCE,

    /// Apply angle increment times rotation ramp attribute value on top of
    /// starting rotation of the primitive.
    ATTRIB,

    /// Apply (angle increment / num edges) to each edge on top of previous
    /// edge's rotation.
    FULLEDGES,

    /// Apply (angle increment * edge length / total length) to each edge on
    /// top of previous edge's rotation.
    FULLDISTANCE
};

bool
getPolyProperties(
    const GEO_Detail *geometry,
    const GA_OffsetListRef &vertices,
    exint &nedges,
    bool &closed,
    bool &unrolled);

SYS_FORCE_INLINE bool
getPolyProperties(
    const GEO_Detail *geometry,
    const GA_Offset primoff,
    exint &nedges,
    bool &closed,
    bool &unrolled)
{
    const GA_OffsetListRef vertices = geometry->getPrimitiveVertexList(primoff);
    return getPolyProperties(geometry, vertices, nedges, closed, unrolled);
}

template<typename T>
struct CurveFrameParms
{
    TangentType myTangentType;
    bool myExtrapolateEndTangents;

    /// Use incoming N, up, rot, orient, pscale, scale, pivot, trans, transform.
    bool myTransformByInstanceAttribs;

    UT_XformOrder::xyzOrder myRotationOrder;
    UT_Vector3T<T> myAngles = UT_Vector3T<T>(0,0,0);
    UT_Vector3T<T> myIncAngles = UT_Vector3T<T>(0,0,0);
    /// NOTE: myIncAnglePer[2] will also be used for ensuring closed curve continuity.
    RotationPer myIncAnglePer[3];
    /// Component of myRotAttribs being read. (default 0)
    int myRotAttribComponent = 0;
    /// "pitch" (X), "yaw" (Y), and "roll" (Z) attributes to add
    /// to rotation angles, to be multiplied by myIncAngles if myIncAnglePer is ATTRIB,
    /// and they can be vertex, point, primitive, or detail attributes.
    GA_ROHandleT<T> myRotAttribs[3];

    UT_Vector3T<T> myTargetUpVector;
    UT_Vector3T<T> myEndTargetUpVector;
    GA_ROHandleT<UT_Vector3T<T>> myTargetUpVectorAttrib;
    GA_ROHandleT<UT_Vector3T<T>> myEndTargetUpVectorAttrib;
    bool myUseCurveNormalAsTargetUp;
    /// When true, the target up vector will be used as the starting up vector.
    /// When false, it will be used as the *average* up vector.
    bool myTargetUpVectorAtStart;
    /// NOTE: This should only be true if myTargetUpVectorAtStart is true.
    ///       It also only applies to open curves.
    bool myUseEndTargetUpVector = false;

    /// When true, transformations for closed curves (and for "unrolled" curves
    /// that share a first and last point) will have an incremental roll added
    /// to ensure that any net roll introduced by non-planarity of the curve
    /// will be spread out, instead of being all cancelled in the very last
    /// edge.  When false, this roll will be uncorrected, so there can be
    /// a large roll around the last edge.
    ///
    /// This is important to turn off if, for example,
    /// myRotRampAttrib already accounts for this extra roll.
    ///
    /// If this is enabled and myIncAngles[2] is non-zero, the total roll
    /// around the curve, including the inherent roll, will be rounded to
    /// be a multiple of a full turn to ensure continuity.  If myIncAnglePer[2]
    /// is ATTRIB, the full range of the attribute along the curve is assumed to
    /// be 0-1, so if this is not the true range, that roll may need to be
    /// applied separately.
    bool myContinuousClosedCurves = true;

    bool myNormalizeScales;
    T myUniformScale = T(1);
    UT_Ramp *myScaleRamp = nullptr;
    bool myStretchAroundTurns;
    T myMaxStretchScale;
};

/// Computes transforms for vertex attribute transform_attrib based on
/// primitives (assumed to be polygon primitives if not NURBS or Bezier)
/// in geo and based on settings in parms.
///
/// NOTE: End vertices on unrolled curves are not written-to.
///       Use extractAttribFromTransform to extract values to a more permanent
///       vertex attribute.
///
/// This is instantiated for fpreal32 and fpreal64.
template<typename T>
void
computeCurveTransforms(
    const GEO_Detail *const geo,
    const GA_PrimitiveGroup *curve_group,
    const GA_RWHandleT<UT_Matrix4T<T>> &transform_attrib,
    const CurveFrameParms<T> &parms);

template<typename T,typename OUTPUT_T,typename FUNCTOR>
void
extractAttribFromTransform(
    GEO_Detail *geo,
    const GA_PrimitiveGroup *curve_group,
    const GA_ROHandleT<UT_Matrix4T<T>> &transform_attrib,
    const GA_RWHandleT<OUTPUT_T> &output_attrib,
    FUNCTOR &&extract_functor)
{
    // Start an interruptible block.  The message will appear in the status bar
    // if wasInterrupted() is called after about a second of cooking.
    UT_AutoInterrupt interrupt("Writing curve transforms");
    if (interrupt.wasInterrupted()) {
        return;
    }

    // We're going to be writing to axis_attrib in parallel, and although
    // we're using GA_SplittableRange, we're splitting over primitive pages,
    // not vertex pages, so we have to harden in advance.
    output_attrib->hardenAllPages();

    // Loop over primitives in parallel, using a splittable range and a lambda functor.
    UTparallelForLightItems(GA_SplittableRange(geo->getPrimitiveRange(curve_group)),
        [geo,&transform_attrib,&interrupt,&output_attrib,&extract_functor](const GA_SplittableRange &r) {

        // Inside the functor, we have a sub-range of primitives, so loop over that range.
        // We use blockAdvance, instead of !it.atEnd() and ++it, for less looping overhead.
        GA_Offset start; GA_Offset end;
        for (GA_Iterator it(r); it.blockAdvance(start,end); ) {
            // We probably don't need to check for interruption on every curve,
            // (unless people put in a curve with millions of vertices), so we
            // can check it once for every contiguous block of up to GA_PAGE_SIZE
            // primitive offsets.
            if (interrupt.wasInterrupted()) {
                return;
            }

            // Loop over all primitives in this contiguous block of offsets.
            for (GA_Offset primoff = start; primoff < end; ++primoff) {
                const GA_OffsetListRef vertices = geo->getPrimitiveVertexList(primoff);
                GA_Size nedges;
                bool closed;
                bool unrolled;
                bool nonempty = getPolyProperties(geo, vertices, nedges, closed, unrolled);
                const GA_Size npoints = nedges + !closed;

                // Nothing to do if no vertices
                if (!nonempty) {
                    continue;
                }

                // NOTE: Although we're really iterating over vertices,
                //       we didn't write to the last vertex of unrolled curves
                //       (open but last point same as first).
                for (GA_Size i = 0; i < npoints; ++i) {
                    GA_Offset vtxoff = vertices(i);
                    output_attrib.set(vtxoff, extract_functor(transform_attrib.get(vtxoff)));
                }
                if (unrolled) {
                    GA_Offset dest_vtxoff = vertices(npoints);
                    GA_Offset src_vtxoff = vertices(0);
                    output_attrib.set(dest_vtxoff, extract_functor(transform_attrib.get(src_vtxoff)));
                }
            }
        }
    });
}

/// This works for AXIS == 0 (x), 1 (y), 2 (z), and 3 (translation).
template<int AXIS,typename T,typename OUTPUT_T>
void
extractAxisAttrib(
    GEO_Detail *geo,
    const GA_PrimitiveGroup *curve_group,
    const GA_ROHandleT<UT_Matrix4T<T>> &transform_attrib,
    const GA_RWHandleT<UT_Vector3T<OUTPUT_T>> &axis_attrib)
{
    extractAttribFromTransform(geo, curve_group, transform_attrib, axis_attrib,
        [](UT_Matrix4T<T> &&transform) -> UT_Vector3T<OUTPUT_T> {
            return UT_Vector3T<OUTPUT_T>(transform[AXIS]);
        });
}

}

} // End of HDK_Sample namespace
