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
 * This SOP computes orientations along curves, including treating closed
 * polygons as closed loop curves.
 */

// A .proto.h file is an automatically generated header file based on theDsFile,
// below, to provide SOP_FeE_OrientAlongCurve_1_0Parms, an easy way to access parameter
// values from SOP_FeE_OrientAlongCurve_1_0Verb::cook with the correct type, and
// SOP_FeE_OrientAlongCurve_1_0Enums, a namespace containing enum types for any ordinal
// menu parameters.
#include "SOP_FeE_OrientAlongCurve_1_0.proto.h"

#include "GU/GU_CurveFrame.h"

#include <SOP/SOP_Node.h>
#include <SOP/SOP_NodeVerb.h>
#include <GU/GU_Detail.h>
#include <GOP/GOP_Manager.h>
#include <GA/GA_Handle.h>
#include <GA/GA_Iterator.h>
#include <GA/GA_Names.h>
#include <GA/GA_OffsetList.h>
#include <GA/GA_SplittableRange.h>
#include <GA/GA_Types.h>
#include <PRM/PRM_TemplateBuilder.h>
#include <UT/UT_DSOVersion.h>
#include <UT/UT_Interrupt.h>
#include <UT/UT_PageArray.h>
#include <UT/UT_PageArrayImpl.h>
#include <UT/UT_ParallelUtil.h>
#include <UT/UT_StringHolder.h>
#include <UT/UT_UniquePtr.h>
#include <UT/UT_Vector3.h>
#include <SYS/SYS_Math.h>


namespace SOP_FeE_OrientAlongCurve_1_0_Namespace {

//******************************************************************************
//*                 Setup                                                      *
//******************************************************************************

class SOP_FeE_OrientAlongCurve_1_0Verb : public SOP_NodeVerb
{
public:
    SOP_NodeParms *allocParms() const override { return new SOP_FeE_OrientAlongCurve_1_0Parms(); }
    //SOP_NodeCache *allocCache() const override { return new SOP_FeE_OrientAlongCurve_1_0Cache(); }
    UT_StringHolder name() const override { return theSOPTypeName; }

    CookMode cookMode(const SOP_NodeParms *parms) const override { return COOK_GENERIC; }

    void cook(const CookParms &cookparms) const override;

    /// This is the internal name of the SOP type.
    /// It isn't allowed to be the same as any other SOP's type name.
    static const UT_StringHolder theSOPTypeName;

    /// This static data member automatically registers
    /// this verb class at library load time.
    static const SOP_NodeVerb::Register<SOP_FeE_OrientAlongCurve_1_0Verb> theVerb;

    /// This is the parameter interface string, below.
    static const char *const theDsFile;
};

// The static member variable definitions have to be outside the class definition.
// The declarations are inside the class.
const UT_StringHolder SOP_FeE_OrientAlongCurve_1_0Verb::theSOPTypeName("FeE::orientAlongCurve::1.0"_sh);
const SOP_NodeVerb::Register<SOP_FeE_OrientAlongCurve_1_0Verb> SOP_FeE_OrientAlongCurve_1_0Verb::theVerb;

/// This is the SOP class definition.
class SOP_FeE_OrientAlongCurve_1_0 : public SOP_Node
{
public:
    static PRM_Template *buildTemplates();

    static OP_Node *myConstructor(OP_Network *net, const char *name, OP_Operator *op)
    {
        OP_Node* newOp = new SOP_FeE_OrientAlongCurve_1_0(net, name, op);
        //newOp->setColor(UT_Color(UT_ColorType::UT_RGB, 0.8, 0.5, 0.5));
        newOp->setNodeShape("squared");
        //newOp->setUserData("nodeshape", "tilted", false);
        return newOp;
    }

protected:
    const SOP_NodeVerb *cookVerb() const override;

    SOP_FeE_OrientAlongCurve_1_0(OP_Network *net, const char *name, OP_Operator *op)
        : SOP_Node(net, name, op)
    {
        // All verb SOPs must manage data IDs, to track what's changed
        // from cook to cook.
        mySopFlags.setManagesDataIDs(true);
    }

    ~SOP_FeE_OrientAlongCurve_1_0() override {}

    /// Since this SOP implements a verb, cookMySop just delegates to the verb.
    OP_ERROR cookMySop(OP_Context &context) override
    {
        return cookMyselfAsVerb(context);
    }

    /// These are the labels that appear when hovering over the inputs.
    const char *inputLabel(unsigned idx) const override
    {
        switch (idx)
        {
            case 0:     return "Curves";
            default:    return "Invalid Source";
        }
    }

    /// This just indicates whether an input wire gets drawn with a dotted line
    /// in the network editor.  If something is usually copied directly
    /// into the output, a solid line (false) is used, but this SOP very often
    /// doesn't do that for either input.
    int isRefInput(unsigned i) const override
    {
        // The curves are passed through
        return false;
    }
};

PRM_Template *SOP_FeE_OrientAlongCurve_1_0::buildTemplates()
{
    static PRM_TemplateBuilder templ("SOP_FeE_OrientAlongCurve_1_0.C"_sh, SOP_FeE_OrientAlongCurve_1_0Verb::theDsFile);
    if (templ.justBuilt())
    {
        templ.setChoiceListPtr("group", &SOP_Node::primGroupMenu);
    }
    return templ.templates();
}

const SOP_NodeVerb *SOP_FeE_OrientAlongCurve_1_0::cookVerb() const
{
    return SOP_FeE_OrientAlongCurve_1_0Verb::theVerb.get();
}

} // End of SOP_FeE_OrientAlongCurve_1_0_Namespace namespace

/// newSopOperator is the hook that Houdini grabs from this dll
/// and invokes to register the SOP.  In this case, we add ourselves
/// to the specified operator table.
void newSopOperator(OP_OperatorTable *table)
{
    OP_Operator* newOp = new OP_Operator(
        SOP_FeE_OrientAlongCurve_1_0_Namespace::SOP_FeE_OrientAlongCurve_1_0Verb::theSOPTypeName,
        "FeE Orientation Along Curve",
        SOP_FeE_OrientAlongCurve_1_0_Namespace::SOP_FeE_OrientAlongCurve_1_0::myConstructor,
        SOP_FeE_OrientAlongCurve_1_0_Namespace::SOP_FeE_OrientAlongCurve_1_0::buildTemplates(),
        1,
        1,
        nullptr,
        0,    // No flags: not a generator, no merge input, not an output
        nullptr,
        1,
        "Five elements Elf/Data/Orient");

    newOp->setIconName("SOP_orientalongcurve");
    table->addOperator(newOp);
}

namespace SOP_FeE_OrientAlongCurve_1_0_Namespace {

//******************************************************************************
//*                 Parameters                                                 *
//******************************************************************************

/// This is a multi-line raw string specifying the parameter interface for this SOP.
const char *const SOP_FeE_OrientAlongCurve_1_0Verb::theDsFile = R"THEDSFILE(
{
    name        parameters
    parm {
        name    "group"
        cppname "CurveGroup"
        label   "Curve Group"
        type    string
        default { "" }
        parmtag { "script_action" "import soputils\nkwargs['geometrytype'] = (hou.geometryType.Primitives,)\nkwargs['inputindex'] = 0\nsoputils.selectGroupParm(kwargs)" }
        parmtag { "script_action_help" "Select geometry from an available viewport.\nShift-click to turn on Select Groups." }
        parmtag { "script_action_icon" "BUTTONS_reselect" }
    }
    groupsimple {
        name    "frame_folder"
        label   "Frame"

        parm {
            name    "tangenttype"
            cppname "TangentType"
            label   "Tangent Type"
            type    ordinal
            default { "0" } // Default to first entry in menu, "avgdir"
            menu {
                "avgdir"    "Average of Edge Directions"
                "diff"      "Central Difference"
                "prev"      "Previous Edge"
                "next"      "Next Edge"
                "none"      "Z Axis (Ignore Curve)"
            }
        }
        parm {
            name    "continuousclosed"
            cppname "ContinuousClosed"
            label   "Make Closed Curve Orientations Continuous"
            type    toggle
            default { "1" }
            disablewhen "{ tangenttype == none }"
        }
        parm {
            name    "extrapolateendtangents"
            cppname "ExtrapolateEndTangents"
            label   "Extrapolate End Tangents"
            type    toggle
            default { "0" }
            disablewhen "{ tangenttype == none }"
        }
        parm {
            name    "transformbyattribs"
            cppname "TransformByAttribs"
            label   "Transform Using Point Attributes"
            type    toggle
            default { "0" }
        }
        //parm {
        //    name    "orienttype"
        //    cppname "OrientType"
        //    label   "Orientation Type"
        //    type    ordinal
        //    default { "0" } // Default to first entry in menu, "minrot"
        //    menu {
        //        "minrot"    "Minimal Rotation"
        //        "curvature" "Curvature"
        //        "bank"      "Bank Around Turns"
        //    }
        //    disablewhen "{ tangenttype == none }"
        //}
        parm {
            name    "sepparm"
            label   ""
            type    separator
            default { "" }
        }
        parm {
            name    "upvectortype"
            cppname "UpVectorType"
            label   "Target Up Vector"
            type    ordinal
            default { "0" } // Default to first entry in menu, "normal"
            menu {
                "normal"    "Curve Normal"
                "x"         "X Axis"
                "y"         "Y Axis"
                "z"         "Z Axis"
                "attrib"    "Attribute"
                "custom"    "Custom"
            }
            disablewhen "{ tangenttype == none }"
        }
        //parm {
        //    name    "usenormalup"
        //    cppname "UseNormalUp"
        //    label   "Use Curve Normal as Up Vector (When Valid)"
        //    type    toggle
        //    default { "1" }
        //    disablewhen "{ tangenttype == none }"
        //}
        parm {
            name    "upvectoratstart"
            cppname "UpVectorAtStart"
            label   "Target Up Vector at Start (else Average)"
            type    toggle
            default { "1" }
            disablewhen "{ tangenttype == none }"
        }
        parm {
            name    "useendupvector"
            cppname "UseEndUpVector"
            label   "Use Target End Up Vector"
            type    toggle
            default { "0" }
            disablewhen "{ tangenttype == none } { upvectoratstart == 0 }"
        }
        parm {
            name    "upvectorattrib"
            cppname "UpVectorAttrib"
            label   "Start Up Attribute"
            type    string
            default { "target_up" }
            disablewhen "{ tangenttype == none } { upvectortype != attrib }"
            hidewhen "{ tangenttype == none } { upvectortype != attrib }"
        }
        parm {
            name    "endupvectorattrib"
            cppname "EndUpVectorAttrib"
            label   "End Up Attribute"
            type    string
            default { "target_up_end" }
            disablewhen "{ tangenttype == none } { upvectortype != attrib } { useendupvector == 0 } { upvectoratstart == 0 }"
            hidewhen "{ tangenttype == none } { upvectortype != attrib } { useendupvector == 0 } { upvectoratstart == 0 }"
        }
        parm {
            name    "upvector"
            cppname "UpVector"
            label   "Start Up Vector"
            type    vector
            size    3
            default { "0" "1" "0" }
            disablewhen "{ tangenttype == none } { upvectortype != custom }"
            hidewhen "{ tangenttype == none } { upvectortype != custom }"
        }
        parm {
            name    "endupvector"
            cppname "EndUpVector"
            label   "End Up Vector"
            type    vector
            size    3
            default { "0" "1" "0" }
            disablewhen "{ tangenttype == none } { upvectortype != custom } { useendupvector == 0 } { upvectoratstart == 0 }"
            hidewhen "{ tangenttype == none } { upvectortype != custom } { useendupvector == 0 } { upvectoratstart == 0 }"
        }
    }
)THEDSFILE"
// ==== This is necessary because MSVC++ has a limit of 16380 character per
// ==== string literal
R"THEDSFILE(
    groupcollapsible {
        name    "rotation_folder"
        label   "Additional Rotations"
        grouptag { "group_type" "collapsible" }
        parmtag { "group_default" "0" }

        parm {
            name    "rOrd"
            cppname "ROrd"
            label   "Rotate Order"
            type    ordinal
            // NOTE: The default rotation order X,Y,Z is semi-arbitrary, but Z
            //       should probably be last, since it always needs to twist
            //       around the curve tangent.  The X and Y rotations may have
            //       just been to reorient a cross-section before copying.
            default { "xyz" }
            menu {
                "xyz"   "Pitch, Yaw, Roll"
                "xzy"   "Pitch, Roll, Yaw"
                "yxz"   "Yaw, Pitch, Roll"
                "yzx"   "Yaw, Roll, Pitch"
                "zxy"   "Roll, Pitch, Yaw"
                "zyx"   "Roll, Yaw, Pitch"
            }
        }
        parm {
            name    "applyroll"
            cppname "ApplyRoll"
            label   "Apply Roll or Twist"
            type    toggle
            default { "0" }
        }
        parm {
            name    "roll"
            label   "Roll"
            type    float
            default { "0" }
            range   { -180 180 }
            hidewhen "{ applyroll == 0 }"
        }
        parm {
            name    "rollper"
            cppname "RollPer"
            label   "Twist Per"
            type    ordinal
            default { "4" }     // Default to "fulldistance" entry in menu
            menu {
                "edge"          "Per Edge"
                "distance"      "Per Unit Distance"
                "attrib"        "Scale by Attribute"
                "fulledges"     "Per Full Curve by Edges"
                "fulldistance"  "Per Full Curve by Distance"
            }
            hidewhen "{ applyroll == 0 }"
        }
        parm {
            name    "fulltwists"
            cppname "FullTwists"
            label   "Full Twists"
            type    integer
            default { "0" }
            range   { -10 10 }
            hidewhen "{ applyroll == 0 }"
        }
        parm {
            name    "incroll"
            cppname "IncRoll"
            label   "Partial Twist"
            type    float
            default { "0" }
            range   { -180 180 }
            hidewhen "{ applyroll == 0 }"
        }
        parm {
            name    "rollattrib"
            cppname "RollAttrib"
            label   "Twist Ramp Attribute"
            type    string
            default { "roll" }
            disablewhen "{ applyroll == 0 } { applyroll == 1 rollper != attrib }"
            hidewhen    "{ applyroll == 0 } { applyroll == 1 rollper != attrib }"
        }
        parm {
            name    "sepparmroll"
            label   ""
            type    separator
            default { "" }
            hidewhen "{ applyroll == 0 }"
        }
        parm {
            name    "applyyaw"
            cppname "ApplyYaw"
            label   "Apply Yaw"
            type    toggle
            default { "0" }
        }
        parm {
            name    "yaw"
            label   "Yaw"
            type    float
            default { "0" }
            range   { -180 180 }
            hidewhen "{ applyyaw == 0 }"
        }
        parm {
            name    "yawper"
            cppname "YawPer"
            label   "Yaw Per"
            type    ordinal
            default { "4" }     // Default to "fulldistance" entry in menu
            menu {
                "edge"          "Per Edge"
                "distance"      "Per Unit Distance"
                "attrib"        "Scale By Attribute"
                "fulledges"     "Per Full Curve by Edges"
                "fulldistance"  "Per Full Curve by Distance"
            }
            hidewhen "{ applyyaw == 0 }"
        }
        parm {
            name    "incyaw"
            cppname "IncYaw"
            label   "Incremental Yaw"
            type    float
            default { "0" }
            range   { -180 180 }
            hidewhen "{ applyyaw == 0 }"
        }
        parm {
            name    "yawattrib"
            cppname "YawAttrib"
            label   "Yaw Ramp Attribute"
            type    string
            default { "yaw" }
            disablewhen "{ applyyaw == 0 } { applyyaw == 1 yawper != attrib }"
            hidewhen    "{ applyyaw == 0 } { applyyaw == 1 yawper != attrib }"
        }
        parm {
            name    "sepparmyaw"
            label   ""
            type    separator
            default { "" }
            hidewhen "{ applyyaw == 0 }"
        }
        parm {
            name    "applypitch"
            cppname "ApplyPitch"
            label   "Apply Pitch"
            type    toggle
            default { "0" }
        }
        parm {
            name    "pitch"
            label   "Pitch"
            type    float
            default { "0" }
            range   { -180 180 }
            hidewhen "{ applypitch == 0 }"
        }
        parm {
            name    "pitchper"
            cppname "PitchPer"
            label   "Pitch Per"
            type    ordinal
            default { "4" }     // Default to "fulldistance" entry in menu
            menu {
                "edge"          "Per Edge"
                "distance"      "Per Unit Distance"
                "attrib"        "Scale By Attribute"
                "fulledges"     "Per Full Curve by Edges"
                "fulldistance"  "Per Full Curve by Distance"
            }
            hidewhen "{ applypitch == 0 }"
        }
        parm {
            name    "incpitch"
            cppname "IncPitch"
            label   "Incremental Pitch"
            type    float
            default { "0" }
            range   { -180 180 }
            hidewhen "{ applypitch == 0 }"
        }
        parm {
            name    "pitchattrib"
            cppname "PitchAttrib"
            label   "Pitch Ramp Attribute"
            type    string
            default { "pitch" }
            disablewhen "{ applypitch == 0 } { applypitch == 1 pitchper != attrib }"
            hidewhen    "{ applypitch == 0 } { applypitch == 1 pitchper != attrib }"
        }
    }
)THEDSFILE"
// ==== This is necessary because MSVC++ has a limit of 16380 character per
// ==== string literal
R"THEDSFILE(
    groupcollapsible {
        name    "scales_folder"
        label   "Scales and Shears"
        grouptag { "group_type" "collapsible" }
        parmtag { "group_default" "0" }

        parm {
            name    "normalize"
            label   "Normalize Scales"
            type    toggle
            default { "1" }
        }
        parm {
            name    "scale"
            label   "Uniform Scale"
            type    float
            default { "1" }
            range   { 0 4 }
        }
        parm {
            name    "stretcharoundturns"
            cppname "StretchAroundTurns"
            label   "Stretch Around Turns"
            type    toggle
            default { "0" }
        }
        parm {
            name    "maxstretcharoundturns"
            cppname "MaxStretchAroundTurns"
            label   "Max Stretch"
            type    log
            default { "10" }
            range   { 1! 100 }
            disablewhen "{ stretcharoundturns == 0 }"
        }
    }
)THEDSFILE"
// ==== This is necessary because MSVC++ has a limit of 16380 character per
// ==== string literal
R"THEDSFILE(
    groupcollapsible {
        name    "output_folder"
        label   "Output Attributes"
        grouptag { "group_type" "collapsible" }
        parmtag { "group_default" "1" }

        parm {
            name    "class"
            cppname "Class"
            label   "Class"
            type    ordinal
            default { "0" }     // Default to first entry in menu, "point"
            menu {
                "point"     "Point"
                "vertex"    "Vertex"
            }
            //joinnext
        }
        //parm {
        //    name    "precision"
        //    cppname "Precision"
        //    label   "Precision"
        //    type    ordinal
        //    default { "1" }     // Default to first entry in menu, "32"
        //    menu {
        //        "16"    "16-bit"
        //        "32"    "32-bit"
        //        "64"    "64-bit"
        //    }
        //}
        parm {
            name    "outputxaxis"
            cppname "OutputXAxis"
            label   "Output X Axis"
            type    toggle
            default { "0" }
            nolabel
            joinnext
        }
        parm {
            name    "xaxisname"
            cppname "XAxisName"
            label   "X Axis"
            type    string
            default { "out" }
            disablewhen "{ outputxaxis == 0 }"
        }
        parm {
            name    "outputyaxis"
            cppname "OutputYAxis"
            label   "Output Y Axis"
            type    toggle
            default { "1" }
            nolabel
            joinnext
        }
        parm {
            name    "yaxisname"
            cppname "YAxisName"
            label   "Y Axis"
            type    string
            default { "up" }
            disablewhen "{ outputyaxis == 0 }"
        }
        parm {
            name    "outputzaxis"
            cppname "OutputZAxis"
            label   "Output Z Axis"
            type    toggle
            default { "1" }
            nolabel
            joinnext
        }
        parm {
            name    "zaxisname"
            cppname "ZAxisName"
            label   "Tangent (Z Axis)"
            type    string
            default { "N" }
            disablewhen "{ outputzaxis == 0 }"
        }
        parm {
            name    "sepparmattrib"
            label   ""
            type    separator
            default { "" }
        }
        parm {
            name    "outputtranslation"
            cppname "OutputTranslation"
            label   "Output Translation"
            type    toggle
            default { "0" }
            nolabel
            joinnext
        }
        parm {
            name    "translationname"
            cppname "TranslationName"
            label   "Translation"
            type    string
            default { "P" }
            disablewhen "{ outputtranslation == 0 }"
        }
        parm {
            name    "outputquaternion"
            cppname "OutputQuaternion"
            label   "Output Quaternion"
            type    toggle
            default { "0" }
            nolabel
            joinnext
        }
        parm {
            name    "quaternionname"
            cppname "QuaternionName"
            label   "Quaternion"
            type    string
            default { "orient" }
            disablewhen "{ outputquaternion == 0 }"
        }
        parm {
            name    "outputtransform3"
            cppname "OutputTransform3"
            label   "Output 3x3 Transform"
            type    toggle
            default { "0" }
            nolabel
            joinnext
        }
        parm {
            name    "transform3name"
            cppname "Transform3Name"
            label   "3x3 Transform"
            type    string
            default { "transform" }
            disablewhen "{ outputtransform3 == 0 }"
        }
        parm {
            name    "outputtransform4"
            cppname "OutputTransform4"
            label   "Output 4x4 Transform"
            type    toggle
            default { "0" }
            nolabel
            joinnext
        }
        parm {
            name    "transform4name"
            cppname "Transform4Name"
            label   "4x4 Transform"
            type    string
            default { "transform" }
            disablewhen "{ outputtransform4 == 0 }"
        }
    }
}
)THEDSFILE";

//******************************************************************************
//*                 Cooking                                                    *
//******************************************************************************

using namespace SOP_FeE_OrientAlongCurve_1_0Enums;
using namespace GU_CurveFrame;

template<typename T>
static void
sopSetupTransformAttribs(
    GEO_Detail *output_geo,
    const SOP_NodeVerb::CookParms &cookparms,
    const UT_StringHolder &axis_attrib_name,
    const GA_AttributeOwner output_owner,
    const GA_Storage output_storage,
    const exint tuple_size,
    GA_ATINumeric *&output_attrib,
    GA_ATINumeric *&intermediate_attrib,
    GA_ATINumericUPtr &intermediate_attrib_deleter,
    GA_TypeInfo type_info)
{
    if (output_owner == GA_ATTRIB_VERTEX && axis_attrib_name == GA_Names::P)
    {
        cookparms.sopAddWarning(SOP_ErrorCodes::SOP_MESSAGE, "Unable to create a vertex attribute named \"P\".");
        output_attrib = nullptr;
        return;
    }
    // Use existing attribute if present, in case we're just writing to part of it.
    output_attrib = GA_ATINumeric::cast(output_geo->findAttribute(output_owner, axis_attrib_name));
    if (!output_attrib)
    {
        output_attrib = UTverify_cast<GA_ATINumeric*>(output_geo->createTupleAttribute(output_owner, axis_attrib_name, output_storage, tuple_size));
        if (!output_attrib)
        {
            cookparms.sopAddWarning(SOP_ErrorCodes::SOP_ERR_INVALID_ATTRIBUTE_NAME, axis_attrib_name.c_str());
            return;
        }
    }
    else
    {
        if (output_attrib->getTupleSize() < tuple_size)
            output_attrib->setTupleSize(tuple_size);
        if (output_attrib->getStorage() != output_storage)
            output_attrib->setStorage(output_storage);
    }
    output_attrib->bumpDataId();
    output_attrib->setTypeInfo((axis_attrib_name == GA_Names::N && type_info == GA_TYPE_VECTOR) ? GA_TYPE_NORMAL : type_info);

    if (output_owner != GA_ATTRIB_VERTEX)
    {
        const GA_Storage intermediate_storage = SYSisSame<T,fpreal32>() ? GA_STORE_REAL32 : GA_STORE_REAL64;
        intermediate_attrib_deleter = output_geo->createDetachedTupleAttribute(GA_ATTRIB_VERTEX, intermediate_storage, tuple_size);
        intermediate_attrib = intermediate_attrib_deleter.get();
    }
    else
    {
        intermediate_attrib = output_attrib;
    }
}

template<typename T>
static void
sopCreateQuaternionAttrib(
    GEO_Detail *output_geo,
    const GA_PrimitiveGroup *curve_group,
    const SOP_NodeVerb::CookParms &cookparms,
    const GA_ROHandleM4D &transform_attrib,
    const UT_StringHolder &axis_attrib_name,
    const GA_AttributeOwner output_owner,
    const GA_Storage output_storage)
{
    GA_ATINumeric *output_attrib;
    GA_ATINumeric *intermediate_attrib;
    GA_ATINumericUPtr intermediate_attrib_deleter;
    sopSetupTransformAttribs<T>(
        output_geo, cookparms, axis_attrib_name,
        output_owner, output_storage, 4, output_attrib,
        intermediate_attrib, intermediate_attrib_deleter,
        GA_TYPE_QUATERNION);

    if (!output_attrib)
        return;

    GA_RWHandleT<UT_QuaternionT<T>> quat_attrib(intermediate_attrib);
    UT_ASSERT(quat_attrib.isValid());
    extractAttribFromTransform(output_geo, curve_group, transform_attrib, quat_attrib,
        [](UT_Matrix4D &&transform) -> UT_QuaternionT<T> {
            UT_QuaternionT<T> quaternion;
            quaternion.updateFromArbitraryMatrix(UT_Matrix3D(transform));
            return quaternion;
        });

    if (output_owner == GA_ATTRIB_VERTEX)
        return;

    UT_ASSERT(output_owner == GA_ATTRIB_POINT);

    // Convert from vertex to point by averaging "directions" of the quaternions.
    // Averaging quaternions linearly isn't as perfect as slerping, but if we
    // normalize, it's usually pretty close in practice.  If it's only 2 of them
    // and they're averaged uniformly, it's exact.

    GA_RWHandleT<UT_QuaternionT<T>> output_handle(output_attrib);
    UT_ASSERT(output_handle.isValid());

    auto &&functor = [output_geo,curve_group,&output_handle,&quat_attrib](const GA_SplittableRange &r)
    {
        GA_Offset start;
        GA_Offset end;
        for (GA_Iterator it(r); it.blockAdvance(start, end); )
        {
            for (GA_Offset ptoff = start; ptoff < end; ++ptoff)
            {
                // Sum up all of the values from vertices
                UT_QuaternionT<T> direction_sum;
                exint num_vertices = 0;

                for (GA_Offset vtxoff = output_geo->pointVertex(ptoff); GAisValid(vtxoff); vtxoff = output_geo->vertexToNextVertex(vtxoff))
                {
                    if (curve_group)
                    {
                        // Exclude vertices outside curve_group
                        GA_Offset primoff = output_geo->vertexPrimitive(vtxoff);
                        if (!curve_group->contains(primoff))
                            continue;
                    }

                    UT_QuaternionT<T> direction = quat_attrib.get(vtxoff);
                    if (num_vertices == 0)
                    {
                        // Just copy first, since often only one
                        direction_sum = direction;
                    }
                    else
                    {
                        // NOTE: Don't need to normalize each one,
                        //       since they should already be normalized.
                        // Avoid quaternions that represent rotations more than 180 degrees apart
                        if (dot(direction_sum, direction) < 0)
                            direction.negate();
                        direction_sum += direction;
                    }
                    ++num_vertices;
                }

                if (num_vertices == 1)
                    output_handle.set(ptoff, direction_sum);
                else if (num_vertices > 1)
                {
                    // Normalize the direction sum.
                    direction_sum.normalize();
                    output_handle.set(ptoff, direction_sum);
                }
            }
        }
    };
    if (curve_group)
    {
        GA_PointGroup point_group(*output_geo);
        point_group.combine(curve_group);

        UTparallelFor(GA_SplittableRange(output_geo->getPointRange(&point_group)), functor);
    }
    else
    {
        UTparallelFor(GA_SplittableRange(output_geo->getPointRange()), functor);
    }
}

template<typename T,bool include_translates>
static void
sopCreateTransformAttrib(
    GEO_Detail *output_geo,
    const GA_PrimitiveGroup *curve_group,
    const SOP_NodeVerb::CookParms &cookparms,
    const GA_ROHandleM4D &transform_attrib,
    const UT_StringHolder &axis_attrib_name,
    const GA_AttributeOwner output_owner,
    const GA_Storage output_storage)
{
    const int tuple_size = include_translates ? 16 : 9;
    GA_ATINumeric *output_attrib;
    GA_ATINumeric *intermediate_attrib;
    GA_ATINumericUPtr intermediate_attrib_deleter;
    sopSetupTransformAttribs<T>(
        output_geo, cookparms, axis_attrib_name,
        output_owner, output_storage, tuple_size, output_attrib,
        intermediate_attrib, intermediate_attrib_deleter,
        GA_TYPE_TRANSFORM);

    if (!output_attrib)
        return;

    GA_RWHandleT<UT_Matrix3T<T>> transform3_attrib(include_translates ? nullptr : intermediate_attrib);
    GA_RWHandleT<UT_Matrix4T<T>> transform4_attrib(include_translates ? intermediate_attrib : nullptr);
    if (!include_translates)
    {
        UT_ASSERT(transform3_attrib.isValid());
        extractAttribFromTransform(output_geo, curve_group, transform_attrib, transform3_attrib,
            [](UT_Matrix4D &&transform) -> UT_Matrix3T<T> {
            return UT_Matrix3T<T>(transform);
        });
    }
    else
    {
        UT_ASSERT(transform4_attrib.isValid());
        extractAttribFromTransform(output_geo, curve_group, transform_attrib, transform4_attrib,
            [](UT_Matrix4D &&transform) -> UT_Matrix4T<T> {
            return UT_Matrix4T<T>(transform);
        });
    }

    if (output_owner == GA_ATTRIB_VERTEX)
        return;

    UT_ASSERT(output_owner == GA_ATTRIB_POINT);

    // Convert from vertex to point by averaging "directions" of the quaternions,
    // averaging the scales, averaging the shears, and averaging the translates.
    // Averaging quaternions linearly isn't as perfect as slerping, but if we
    // normalize, it's usually pretty close in practice.  If it's only 2 of them
    // and they're averaged uniformly, it's exact.

    GA_RWHandleT<UT_Matrix3T<T>> output3_handle(include_translates ? nullptr : output_attrib);
    GA_RWHandleT<UT_Matrix4T<T>> output4_handle(include_translates ? output_attrib : nullptr);
    UT_ASSERT(output3_handle.isValid() || output4_handle.isValid());

    auto &&functor = [output_geo,curve_group,&output3_handle,&output4_handle,&transform3_attrib,&transform4_attrib](const GA_SplittableRange &r)
    {
        GA_Offset start;
        GA_Offset end;
        for (GA_Iterator it(r); it.blockAdvance(start, end); )
        {
            for (GA_Offset ptoff = start; ptoff < end; ++ptoff)
            {
                UT_Matrix3T<T> first_matrix3;
                UT_Matrix4T<T> first_matrix4;

                // Sum up all of the values from vertices
                UT_QuaternionT<T> direction_sum;
                UT_Matrix3T<T> stretch_matrix_sum;
                UT_Vector3T<T> translate_sum;

                exint num_vertices = 0;

                for (GA_Offset vtxoff = output_geo->pointVertex(ptoff); GAisValid(vtxoff); vtxoff = output_geo->vertexToNextVertex(vtxoff))
                {
                    if (curve_group)
                    {
                        // Exclude vertices outside curve_group
                        GA_Offset primoff = output_geo->vertexPrimitive(vtxoff);
                        if (!curve_group->contains(primoff))
                            continue;
                    }

                    if (!include_translates)
                    {
                        UT_Matrix3T<T> matrix3 = transform3_attrib.get(vtxoff);
                        if (num_vertices == 0)
                        {
                            // Just copy first, since often only one
                            first_matrix3 = matrix3;
                        }
                        else
                        {
                            if (num_vertices == 1)
                            {
                                first_matrix3.makeRotationMatrix(&stretch_matrix_sum);
                                direction_sum.updateFromRotationMatrix(first_matrix3);
                            }
                            UT_Matrix3T<T> stretch_matrix;
                            matrix3.makeRotationMatrix(&stretch_matrix);
                            stretch_matrix_sum += stretch_matrix;
                            UT_QuaternionT<T> direction;
                            direction.updateFromRotationMatrix(matrix3);
                            // Avoid quaternions that represent rotations more than 180 degrees apart
                            if (dot(direction_sum, direction) < 0)
                                direction.negate();
                            direction_sum += direction;
                        }
                    }
                    else
                    {
                        UT_Matrix4T<T> matrix4 = transform4_attrib.get(vtxoff);
                        if (num_vertices == 0)
                        {
                            // Just copy first, since often only one
                            first_matrix4 = matrix4;
                        }
                        else
                        {
                            if (num_vertices == 1)
                            {
                                first_matrix3 = UT_Matrix3T<T>(first_matrix4);
                                first_matrix3.makeRotationMatrix(&stretch_matrix_sum);
                                direction_sum.updateFromRotationMatrix(first_matrix3);
                                first_matrix4.getTranslates(translate_sum);
                            }
                            UT_Matrix3T<T> matrix3(matrix4);
                            UT_Matrix3T<T> stretch_matrix;
                            matrix3.makeRotationMatrix(&stretch_matrix);
                            stretch_matrix_sum += stretch_matrix;
                            UT_QuaternionT<T> direction;
                            direction.updateFromRotationMatrix(matrix3);
                            // Avoid quaternions that represent rotations more than 180 degrees apart
                            if (dot(direction_sum, direction) < 0)
                                direction.negate();
                            direction_sum += direction;
                            UT_Vector3T<T> translate;
                            first_matrix4.getTranslates(translate);
                            translate_sum += translate;
                        }
                    }
                    ++num_vertices;
                }

                if (num_vertices == 1)
                {
                    if (!include_translates)
                        output3_handle.set(ptoff, first_matrix3);
                    else
                        output4_handle.set(ptoff, first_matrix4);
                }
                else if (num_vertices > 1)
                {
                    // Normalize the direction sum.
                    direction_sum.normalize();
                    stretch_matrix_sum /= num_vertices;

                    UT_Matrix3T<T> rotation_matrix;
                    direction_sum.getRotationMatrix(rotation_matrix);
                    // NOTE: The order matters: check the comment on UT_Matrix3T::polarDecompose()
                    UT_Matrix3T<T> matrix3 = stretch_matrix_sum * rotation_matrix;
                    if (!include_translates)
                    {
                        output3_handle.set(ptoff, matrix3);
                    }
                    else
                    {
                        translate_sum /= num_vertices;
                        UT_Matrix4T<T> matrix4(matrix3);
                        matrix4.setTranslates(translate_sum);
                        output4_handle.set(ptoff, matrix4);
                    }
                }
            }
        }
    };
    if (curve_group)
    {
        GA_PointGroup point_group(*output_geo);
        point_group.combine(curve_group);

        UTparallelFor(GA_SplittableRange(output_geo->getPointRange(&point_group)), functor);
    }
    else
    {
        UTparallelFor(GA_SplittableRange(output_geo->getPointRange()), functor);
    }
}

template<int AXIS,typename T>
static void
sopCreateAxisAttrib(
    GEO_Detail *output_geo,
    const GA_PrimitiveGroup *curve_group,
    const SOP_NodeVerb::CookParms &cookparms,
    const GA_ROHandleM4D &transform_attrib,
    const UT_StringHolder &axis_attrib_name,
    const GA_AttributeOwner output_owner,
    const GA_Storage output_storage)
{
    GA_ATINumeric *output_attrib;
    GA_ATINumeric *intermediate_attrib;
    GA_ATINumericUPtr intermediate_attrib_deleter;
    sopSetupTransformAttribs<T>(
        output_geo, cookparms, axis_attrib_name,
        output_owner, output_storage, 3, output_attrib,
        intermediate_attrib, intermediate_attrib_deleter,
        (AXIS == 3) ? GA_TYPE_POINT : GA_TYPE_VECTOR);

    if (!output_attrib)
        return;

    GA_RWHandleT<UT_Vector3T<T>> axis_attrib(intermediate_attrib);
    UT_ASSERT(axis_attrib.isValid());
    extractAxisAttrib<AXIS>(output_geo, curve_group, transform_attrib, axis_attrib);

    if (output_owner == GA_ATTRIB_VERTEX)
        return;

    UT_ASSERT(output_owner == GA_ATTRIB_POINT);

    // Convert from vertex to point by averaging directions and lengths.

    GA_RWHandleT<UT_Vector3T<T>> output_handle(output_attrib);
    UT_ASSERT(output_handle.isValid());

    auto &&functor = [output_geo,curve_group,&output_handle,&axis_attrib](const GA_SplittableRange &r)
    {
        GA_Offset start;
        GA_Offset end;
        for (GA_Iterator it(r); it.blockAdvance(start, end); )
        {
            for (GA_Offset ptoff = start; ptoff < end; ++ptoff)
            {
                // Sum up all of the values from vertices
                UT_Vector3T<T> direction_sum;
                T length_sum;
                exint num_vertices = 0;

                for (GA_Offset vtxoff = output_geo->pointVertex(ptoff); GAisValid(vtxoff); vtxoff = output_geo->vertexToNextVertex(vtxoff))
                {
                    if (curve_group)
                    {
                        // Exclude vertices outside curve_group
                        GA_Offset primoff = output_geo->vertexPrimitive(vtxoff);
                        if (!curve_group->contains(primoff))
                            continue;
                    }

                    UT_Vector3T<T> direction = axis_attrib.get(vtxoff);
                    if (num_vertices == 0)
                    {
                        // Just copy first, since often only one
                        direction_sum = direction;
                    }
                    else
                    {
                        if (num_vertices == 1)
                            length_sum = direction_sum.normalize();

                        length_sum += direction.normalize();
                        // Avoid quaternions that represent rotations more than 180 degrees apart
                        if (dot(direction_sum, direction) < 0)
                            direction.negate();
                        direction_sum += direction;
                    }
                    ++num_vertices;
                }

                if (num_vertices == 1)
                    output_handle.set(ptoff, direction_sum);
                else if (num_vertices > 1)
                {
                    // Normalize the direction sum and use the average length.
                    direction_sum.normalize();
                    output_handle.set(ptoff, direction_sum*(length_sum/num_vertices));
                }
            }
        }
    };
    if (curve_group)
    {
        GA_PointGroup point_group(*output_geo);
        point_group.combine(curve_group);

        UTparallelFor(GA_SplittableRange(output_geo->getPointRange(&point_group)), functor);
    }
    else
    {
        UTparallelFor(GA_SplittableRange(output_geo->getPointRange()), functor);
    }
}

/// This is the function that does the actual work.
void SOP_FeE_OrientAlongCurve_1_0Verb::cook(const CookParms &cookparms) const
{
    ///////////////////////////////////////////////////////////////////
    //                                                               //
    //          0. PREP                                              //
    //                                                               //
    ///////////////////////////////////////////////////////////////////

    // This gives easy access to all of the current parameter values
    auto &&sopparms = cookparms.parms<SOP_FeE_OrientAlongCurve_1_0Parms>();
    //auto sopcache = (SOP_FeE_OrientAlongCurve_1_0Cache *)cookparms.cache();

    // The output detail
    GEO_Detail *output_geo = cookparms.gdh().gdpNC();

    const GEO_Detail *const curve_input = cookparms.inputGeo(0);
    UT_ASSERT(curve_input);

    // Copy the input geometry if we're not surfacing the curves.
    // Most data will be referenced instead of deep-copied.
    output_geo->replaceWith(*curve_input);

    // GOP_Manager will own any temporary groups it creates
    // and automatically destroy them when it goes out of scope.
    GOP_Manager group_manager;
    const GA_PrimitiveGroup *curve_group = nullptr;
    if (sopparms.getCurveGroup().isstring()) {
        // Parse curve group on input detail if surfacing.
        // Parse curve group on output detail if keeping curves.
        curve_group = group_manager.parsePrimitiveGroups(sopparms.getCurveGroup(),
                        GOP_Manager::GroupCreator(output_geo));
    }
    
    // Warn if we know there are going to be non-polygon primitives, since
    // the results may be a little to very unusual for non-polygon primitives.
    if (!curve_group && curve_input->getNumPrimitives() != curve_input->countPrimitiveType(GA_PRIMPOLY)) {
        cookparms.sopAddWarning(SOP_MESSAGE, "Input geometry contains non-polygon primitives, so results may not be as expected");
    }

    ///////////////////////////////////////////////////////////////////
    //                                                               //
    //          3. TRANSFORMS                                        //
    //                                                               //
    ///////////////////////////////////////////////////////////////////

    // Compute transforms on the curves in a detached attribute,
    // since we won't be keeping it, and the index map won't be changing
    // while it's alive.
    GA_ATINumericUPtr transform_deleter = output_geo->createDetachedTupleAttribute(GA_ATTRIB_VERTEX, GA_STORE_REAL64, 16);
    GA_ATINumeric *detached_transform_attrib = transform_deleter.get();
    GA_RWHandleM4D transform_attrib(detached_transform_attrib);

    CurveFrameParms<double> parms;

    parms.myTangentType = GU_CurveFrame::TangentType(int(sopparms.getTangentType()));
    parms.myExtrapolateEndTangents = sopparms.getExtrapolateEndTangents();
    parms.myTransformByInstanceAttribs = sopparms.getTransformByAttribs();

    parms.myRotationOrder = OP_Node::getRotOrder(int(sopparms.getROrd()));

    const bool applypitch = sopparms.getApplyPitch();
    const bool applyyaw = sopparms.getApplyYaw();
    const bool applyroll = sopparms.getApplyRoll();
    const UT_Vector3D angles(
        applypitch ? SYSdegToRad(sopparms.getPitch()) : 0.0,
        applyyaw   ? SYSdegToRad(sopparms.getYaw())   : 0.0,
        applyroll  ? SYSdegToRad(sopparms.getRoll())  : 0.0);
    parms.myAngles = angles;
    const UT_Vector3D incangles(
        applypitch ? SYSdegToRad(sopparms.getIncPitch()) : 0.0,
        applyyaw   ? SYSdegToRad(sopparms.getIncYaw())   : 0.0,
        applyroll  ? SYSdegToRad(sopparms.getIncRoll() + 360.0*sopparms.getFullTwists()) : 0.0);
    parms.myIncAngles = incangles;
    parms.myIncAnglePer[0] = GU_CurveFrame::RotationPer(int(sopparms.getPitchPer()));
    parms.myIncAnglePer[1] = GU_CurveFrame::RotationPer(int(sopparms.getYawPer()));
    parms.myIncAnglePer[2] = GU_CurveFrame::RotationPer(int(sopparms.getRollPer()));

    const GA_AttributeOwner search_order[4] = {
        GA_ATTRIB_VERTEX,
        GA_ATTRIB_POINT,
        GA_ATTRIB_PRIMITIVE,
        GA_ATTRIB_DETAIL
    };
    if (parms.myIncAngles[0] != 0.0 && parms.myIncAnglePer[0] == GU_CurveFrame::RotationPer::ATTRIB)
        parms.myRotAttribs[0].bind(output_geo->findAttribute(sopparms.getPitchAttrib(), search_order, 4));
    if (parms.myIncAngles[1] != 0.0 && parms.myIncAnglePer[1] == GU_CurveFrame::RotationPer::ATTRIB)
        parms.myRotAttribs[1].bind(output_geo->findAttribute(sopparms.getYawAttrib(), search_order, 4));
    if (parms.myIncAngles[2] != 0.0 && parms.myIncAnglePer[2] == GU_CurveFrame::RotationPer::ATTRIB)
        parms.myRotAttribs[2].bind(output_geo->findAttribute(sopparms.getRollAttrib(), search_order, 4));

    using UpVectorType = SOP_FeE_OrientAlongCurve_1_0Enums::UpVectorType;

    switch (sopparms.getUpVectorType())
    {
        case UpVectorType::X: parms.myTargetUpVector = UT_Vector3D(1,0,0); break;
        case UpVectorType::NORMAL: SYS_FALLTHROUGH; // Fallback up vector for curve normal case is Y Axis.
        case UpVectorType::Y: parms.myTargetUpVector = UT_Vector3D(0,1,0); break;
        case UpVectorType::Z: parms.myTargetUpVector = UT_Vector3D(0,0,1); break;
        case UpVectorType::ATTRIB:
        {
            parms.myTargetUpVectorAttrib.bind(output_geo, GA_ATTRIB_PRIMITIVE, sopparms.getUpVectorAttrib());
            if (!parms.myTargetUpVectorAttrib.isValid())
            {
                parms.myTargetUpVectorAttrib.bind(output_geo, GA_ATTRIB_DETAIL, sopparms.getUpVectorAttrib());
            }
            if (parms.myTargetUpVectorAttrib.isValid() && parms.myTargetUpVectorAttrib->getOwner() == GA_ATTRIB_DETAIL)
            {
                // If it's a detail attribute, just read the value here.
                parms.myTargetUpVector = parms.myTargetUpVectorAttrib.get(GA_DETAIL_OFFSET);
                parms.myTargetUpVectorAttrib.clear();
            }
            else
            {
                if (!parms.myTargetUpVectorAttrib.isValid())
                {
                    cookparms.sopAddWarning(SOP_MESSAGE, "Target up vector attribute not found; defaulting to y axis.");
                }
                parms.myTargetUpVector = UT_Vector3D(0,1,0);
            }
            break;
        }
        case UpVectorType::CUSTOM: parms.myTargetUpVector = sopparms.getUpVector(); break;
    }
    parms.myUseCurveNormalAsTargetUp = (sopparms.getUpVectorType() == UpVectorType::NORMAL);
    parms.myTargetUpVectorAtStart = sopparms.getUpVectorAtStart();
    parms.myContinuousClosedCurves = sopparms.getContinuousClosed();

    if (parms.myTargetUpVectorAtStart && sopparms.getUseEndUpVector())
    {
        parms.myUseEndTargetUpVector = true;
        if (sopparms.getUpVectorType() == UpVectorType::ATTRIB)
        {
            parms.myEndTargetUpVectorAttrib.bind(output_geo, GA_ATTRIB_PRIMITIVE, sopparms.getEndUpVectorAttrib());
            if (!parms.myEndTargetUpVectorAttrib.isValid())
            {
                parms.myEndTargetUpVectorAttrib.bind(output_geo, GA_ATTRIB_DETAIL, sopparms.getEndUpVectorAttrib());
            }
            if (parms.myEndTargetUpVectorAttrib.isValid() && parms.myEndTargetUpVectorAttrib->getOwner() == GA_ATTRIB_DETAIL)
            {
                // If it's a detail attribute, just read the value here.
                parms.myEndTargetUpVector = parms.myEndTargetUpVectorAttrib.get(GA_DETAIL_OFFSET);
                parms.myEndTargetUpVectorAttrib.clear();
            }
            else
            {
                if (!parms.myEndTargetUpVectorAttrib.isValid())
                {
                    cookparms.sopAddWarning(SOP_MESSAGE, "End target up vector attribute not found; defaulting to no end target up vector.");
                    parms.myUseEndTargetUpVector = false;
                }
                parms.myEndTargetUpVector = UT_Vector3D(0,1,0);
            }
        }
        else if (sopparms.getUpVectorType() == UpVectorType::CUSTOM)
        {
            parms.myEndTargetUpVector = sopparms.getEndUpVector();
        }
        else
        {
            parms.myEndTargetUpVector = parms.myTargetUpVector;
        }
    }

    parms.myNormalizeScales = sopparms.getNormalize();
    //parms.myMakeOrthogonal = sopparms.getOrthogonal();
    parms.myUniformScale = sopparms.getScale();
    parms.myStretchAroundTurns = sopparms.getStretchAroundTurns();
    parms.myMaxStretchScale = sopparms.getMaxStretchAroundTurns();

    computeCurveTransforms(output_geo, curve_group, transform_attrib, parms);

    ///////////////////////////////////////////////////////////////////
    //                                                               //
    //          4. ATTRIBUTES                                        //
    //                                                               //
    ///////////////////////////////////////////////////////////////////

    // Output any frame attributes
    GA_AttributeOwner output_owner = (sopparms.getClass() == Class::POINT) ? GA_ATTRIB_POINT : GA_ATTRIB_VERTEX;
    GA_Storage output_storage = (
#if 1
        (UTverify_cast<const GA_ATINumeric*>(output_geo->getP())->getStorage() == GA_STORE_REAL64) ? GA_STORE_REAL64 : GA_STORE_REAL32);
#else
        (sopparms.getPrecision() == Precision::_16)
            ? GA_STORE_REAL16
            : ((sopparms.getPrecision() == Precision::_32)
                ? GA_STORE_REAL32
                : GA_STORE_REAL64));
#endif
    GA_Storage intermediate_storage = (output_storage == GA_STORE_REAL16) ? GA_STORE_REAL32 : output_storage;

    if (sopparms.getOutputXAxis() && sopparms.getXAxisName().isstring())
    {

        if (intermediate_storage == GA_STORE_REAL32)
        {
            sopCreateAxisAttrib<0,fpreal32>(
                output_geo, curve_group, cookparms, transform_attrib,
                sopparms.getXAxisName(), output_owner, output_storage);
        }
        else
        {
            sopCreateAxisAttrib<0,fpreal64>(
                output_geo, curve_group, cookparms, transform_attrib,
                sopparms.getXAxisName(), output_owner, output_storage);
        }
    }
    if (sopparms.getOutputYAxis() && sopparms.getYAxisName().isstring())
    {
        if (intermediate_storage == GA_STORE_REAL32)
        {
            sopCreateAxisAttrib<1,fpreal32>(
                output_geo, curve_group, cookparms, transform_attrib,
                sopparms.getYAxisName(), output_owner, output_storage);
        }
        else
        {
            sopCreateAxisAttrib<1,fpreal64>(
                output_geo, curve_group, cookparms, transform_attrib,
                sopparms.getYAxisName(), output_owner, output_storage);
        }
    }
    if (sopparms.getOutputZAxis() && sopparms.getZAxisName().isstring())
    {
        if (intermediate_storage == GA_STORE_REAL32)
        {
            sopCreateAxisAttrib<2,fpreal32>(
                output_geo, curve_group, cookparms, transform_attrib,
                sopparms.getZAxisName(), output_owner, output_storage);
        }
        else
        {
            sopCreateAxisAttrib<2,fpreal64>(
                output_geo, curve_group, cookparms, transform_attrib,
                sopparms.getZAxisName(), output_owner, output_storage);
        }
    }
    if (sopparms.getOutputTranslation() && sopparms.getTranslationName().isstring())
    {
        if (intermediate_storage == GA_STORE_REAL32)
        {
            sopCreateAxisAttrib<3,fpreal32>(
                output_geo, curve_group, cookparms, transform_attrib,
                sopparms.getTranslationName(), output_owner, output_storage);
        }
        else
        {
            sopCreateAxisAttrib<3,fpreal64>(
                output_geo, curve_group, cookparms, transform_attrib,
                sopparms.getTranslationName(), output_owner, output_storage);
        }
    }
    if (sopparms.getOutputQuaternion() && sopparms.getQuaternionName().isstring())
    {
        if (intermediate_storage == GA_STORE_REAL32)
        {
            sopCreateQuaternionAttrib<fpreal32>(
                output_geo, curve_group, cookparms, transform_attrib,
                sopparms.getQuaternionName(), output_owner, output_storage);
        }
        else
        {
            sopCreateQuaternionAttrib<fpreal64>(
                output_geo, curve_group, cookparms, transform_attrib,
                sopparms.getQuaternionName(), output_owner, output_storage);
        }
    }
    if (sopparms.getOutputTransform3() && sopparms.getTransform3Name().isstring())
    {
        if (intermediate_storage == GA_STORE_REAL32)
        {
            sopCreateTransformAttrib<fpreal32,false>(
                output_geo, curve_group, cookparms, transform_attrib,
                sopparms.getTransform3Name(), output_owner, output_storage);
        }
        else
        {
            sopCreateTransformAttrib<fpreal64,false>(
                output_geo, curve_group, cookparms, transform_attrib,
                sopparms.getTransform3Name(), output_owner, output_storage);
        }
    }
    if (sopparms.getOutputTransform4() && sopparms.getTransform4Name().isstring())
    {
        if (intermediate_storage == GA_STORE_REAL32)
        {
            sopCreateTransformAttrib<fpreal32,true>(
                output_geo, curve_group, cookparms, transform_attrib,
                sopparms.getTransform4Name(), output_owner, output_storage);
        }
        else
        {
            sopCreateTransformAttrib<fpreal64,true>(
                output_geo, curve_group, cookparms, transform_attrib,
                sopparms.getTransform4Name(), output_owner, output_storage);
        }
    }
}

} // End of SOP_FeE_OrientAlongCurve_1_0_Namespace namespace
