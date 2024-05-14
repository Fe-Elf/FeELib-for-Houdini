/*
 * Copyright (c) 2022
 *	Side Effects Software Inc.  All rights reserved.
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
 * The Star SOP
 */

#include "SOP_FeE_Star_1_0.h"

// This is an automatically generated header file based on theDsFile, below,
// to provide SOP_FeE_Star_1_0Parms, an easy way to access parameter values from
// SOP_FeE_Star_1_0Verb::cook with the correct type.
#include "SOP_FeE_Star_1_0.proto.h"

#include <GU/GU_Detail.h>
#include <OP/OP_Operator.h>
#include <OP/OP_OperatorTable.h>
#include <PRM/PRM_Include.h>
#include <PRM/PRM_TemplateBuilder.h>
#include <UT/UT_DSOVersion.h>
#include <UT/UT_Interrupt.h>
#include <UT/UT_StringHolder.h>
#include <SYS/SYS_Math.h>
#include <limits.h>

using namespace SOP_FeE_Star_1_0_Namespace;

//
// Help is stored in a "wiki" style text file.  This text file should be copied
// to $HOUDINI_PATH/help/nodes/sop/star.txt
//
// See the sample_install.sh file for an example.
//

/// This is the internal name of the SOP type.
/// It isn't allowed to be the same as any other SOP's type name.
const UT_StringHolder SOP_FeE_Star_1_0::theSOPTypeName("FeE::star::1.0"_sh);

/// newSopOperator is the hook that Houdini grabs from this dll
/// and invokes to register the SOP.  In this case, we add ourselves
/// to the specified operator table.
void
newSopOperator(OP_OperatorTable *table)
{
    OP_Operator* newOp = new OP_Operator(
        SOP_FeE_Star_1_0::theSOPTypeName,
        "FeE Star",
        SOP_FeE_Star_1_0::myConstructor,
        SOP_FeE_Star_1_0::buildTemplates(),
        0,
        0,
        nullptr,
        OP_FLAG_GENERATOR,
        nullptr,
        1,
        "Five elements Elf/Geo/");

    newOp->setIconName("SOP_Star");
    table->addOperator(newOp);
}

/// This is a multi-line raw string specifying the parameter interface
/// for this SOP.
static const char *theDsFile = R"THEDSFILE(
{
    name        parameters
    parm {
        name    "divs"
        cppname "Divs"
        label   "Divisions"
        type    integer
        default { "5" }
        range   { 2! 50 }
        export all
    }
    parm {
        name    "rad"
        cppname "Rad"
        label   "Radius"
        type    vector2
        size    2
        default { "1" "0.3" }
    }
    parm {
        name    "nRadius"
        cppname "NRadius"
        label   "Allow Negative Radius"
        type    toggle
        default { "0" }
    }
    parm {
        name    "center"
        cppname "Center"
        label   "Center"
        type    vector
        size    3
        default { "0" "0" "0" }
    }
    parm {
        name    "orientation"
        cppname "Orientation"
        label   "Orientation"
        type    ordinal
        default { "0" }
        menu    {
            "xy"    "XY Plane"
            "yz"    "YZ Plane"
            "zx"    "ZX Plane"
        }
    }
}
)THEDSFILE";

PRM_Template*
SOP_FeE_Star_1_0::buildTemplates()
{
    static PRM_TemplateBuilder templ("SOP_FeE_Star_1_0.C"_sh, theDsFile);
    return templ.templates();
}

class SOP_FeE_Star_1_0Verb : public SOP_NodeVerb
{
public:
    SOP_FeE_Star_1_0Verb() {}
    virtual ~SOP_FeE_Star_1_0Verb() {}

    virtual SOP_NodeParms *allocParms() const { return new SOP_FeE_Star_1_0Parms(); }
    virtual UT_StringHolder name() const { return SOP_FeE_Star_1_0::theSOPTypeName; }

    virtual CookMode cookMode(const SOP_NodeParms *parms) const { return COOK_GENERIC; }

    virtual void cook(const CookParms &cookparms) const;
    
    /// This static data member automatically registers
    /// this verb class at library load time.
    static const SOP_NodeVerb::Register<SOP_FeE_Star_1_0Verb> theVerb;
};

// The static member variable definition has to be outside the class definition.
// The declaration is inside the class.
const SOP_NodeVerb::Register<SOP_FeE_Star_1_0Verb> SOP_FeE_Star_1_0Verb::theVerb;

const SOP_NodeVerb *
SOP_FeE_Star_1_0::cookVerb() const 
{ 
    return SOP_FeE_Star_1_0Verb::theVerb.get();
}

/// This is the function that does the actual work.
void
SOP_FeE_Star_1_0Verb::cook(const SOP_NodeVerb::CookParms &cookparms) const
{
    auto &&sopparms = cookparms.parms<SOP_FeE_Star_1_0Parms>();
    GU_Detail& geo = *cookparms.gdh().gdpNC();

    // We need two points per division
    exint npoints = sopparms.getDivs()*2;

    if (npoints < 4)
    {
        // With the range restriction we have on the divisions, this
        // is actually impossible, (except via integer overflow),
        // but it shows how to add an error message or warning to the SOP.
        cookparms.sopAddWarning(SOP_MESSAGE, "There must be at least 2 divisions; defaulting to 2.");
        npoints = 4;
    }

    // If this SOP has cooked before and it wasn't evicted from the cache,
    // its output detail will contain the geometry from the last cook.
    // If it hasn't cooked, or if it was evicted from the cache,
    // the output detail will be empty.
    // This knowledge can save us some effort, e.g. if the number of points on
    // this cook is the same as on the last cook, we can just move the points,
    // (i.e. modifying P), which can also save some effort for the viewport.

    GA_Offset start_ptoff;
    if (geo.getNumPoints() != npoints)
    {
        // Either the SOP hasn't cooked, the detail was evicted from
        // the cache, or the number of points changed since the last cook.

        // This destroys everything except the empty P and topology attributes.
        geo.clearAndDestroy();

        // Build 1 closed polygon (as opposed to a curve),
        // namely that has its closed flag set to true,
        // and the right number of vertices, as a contiguous block
        // of vertex offsets.
        GA_Offset start_vtxoff;
        geo.appendPrimitivesAndVertices(GA_PRIMPOLY, 1, npoints, start_vtxoff, true);

        // Create the right number of points, as a contiguous block
        // of point offsets.
        start_ptoff = geo.appendPointBlock(npoints);

        // Wire the vertices to the points.
        for (exint i = 0; i < npoints; ++i)
        {
            geo.setVertexPoint(start_vtxoff+i,start_ptoff+i);
        }

        // We added points, vertices, and primitives,
        // so this will bump all topology attribute data IDs,
        // P's data ID, and the primitive list data ID.
        geo.bumpDataIdsForAddOrRemove(true, true, true);
    }
    else
    {
        // Same number of points as last cook, and we know that last time,
        // we created a contiguous block of point offsets, so just get the
        // first one.
        start_ptoff = geo.pointOffset(GA_Index(0));

        // We'll only be modifying P, so we only need to bump P's data ID.
        geo.getP()->bumpDataId();
    }

    // Everything after this is just to figure out what to write to P and write it.

    const bool allow_negative_radius = sopparms.getNRadius();

    const UT_Vector3 center = sopparms.getCenter();


    // Start the interrupt scope
    UT_AutoInterrupt boss("Building Star");
    if (boss.wasInterrupted())
        return;

    const fpreal tinc = M_PI * 2.0 / static_cast<fpreal>(npoints);
    const fpreal outer_radius = sopparms.getRad().x();
    const fpreal inner_radius = sopparms.getRad().y();
    
    const SOP_FeE_Star_1_0Parms::Orientation plane = sopparms.getOrientation();
    
    for (exint i = 0; i < npoints; i++)
    {
        const fpreal angle = tinc * static_cast<fpreal>(npoints);
        const bool odd = i & 1;
        fpreal rad = odd ? inner_radius : outer_radius;
        if (!allow_negative_radius && rad < 0)
            rad = 0;

        UT_Vector3T<fpreal> pos;
        switch (plane)
        {
        case SOP_FeE_Star_1_0Parms::Orientation::XY: pos = UT_Vector3T<fpreal>(std::cos(angle), std::sin(angle), 0); break;
        case SOP_FeE_Star_1_0Parms::Orientation::YZ: pos = UT_Vector3T<fpreal>(std::sin(angle), 0, std::cos(angle)); break;
        case SOP_FeE_Star_1_0Parms::Orientation::ZX: pos = UT_Vector3T<fpreal>(std::cos(angle), 0, std::sin(angle)); break;
        }
        pos *= rad;
        pos += center;

        const GA_Offset ptoff = start_ptoff + i;
        geo.setPos3(ptoff, pos);
    }
}
