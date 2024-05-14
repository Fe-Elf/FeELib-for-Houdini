
//#define UT_ASSERT_LEVEL 3
#include "SOP_FeE_Fuse_1_0.h"


#include "SOP_FeE_Fuse_1_0.proto.h"

#include "GA/GA_Detail.h"
#include "PRM/PRM_TemplateBuilder.h"
#include "UT/UT_Interrupt.h"
#include "UT/UT_DSOVersion.h"




#include "GFE/GFE_Fuse.h"




using namespace SOP_FeE_Fuse_1_0_Namespace;


static const char *theDsFile = R"THEDSFILE(
{
    name        parameters
    parm {
        name    "group"
        cppname "Group"
        label   "Group"
        type    string
        default { "" }
        parmtag { "script_action" "import soputils\nkwargs['geometrytype'] = kwargs['node'].parmTuple('groupType')\nkwargs['inputindex'] = 0\nsoputils.selectGroupParm(kwargs)" }
        parmtag { "script_action_help" "Select geometry from an available viewport.\nShift-click to turn on Select Groups." }
        parmtag { "script_action_icon" "BUTTONS_reselect" }
    }
    parm {
        name    "groupType"
        cppname "GroupType"
        label   "Group Type"
        type    ordinal
        default { "guess" }
        menu {
            "guess"     "Guess from Group"
            "prim"      "Primitive"
            "point"     "Point"
            "vertex"    "Vertex"
            "edge"      "Edge"
        }
    }

    parm {
        name    "useTargetGroup"
        cppname "UseTargetGroup"
        label   "Target Group"
        type    toggle
        nolabel
        joinnext
        default { "off" }
        disablewhen "{ snapMethod == grid } { hasinput(1) == 1 }"
    }
    parm {
        name    "targetGroup"
        cppname "TargetGroup"
        label   "Target Group"
        type    string
        default { "" }
        disablewhen "{ snapMethod == grid } { hasinput(1) == 0 useTargetGroup == 0 } { hasinput(1) == 0 }"
    }
    parm {
        name    "modifyTarget"
        cppname "ModifyTarget"
        label   "Modify Target"
        type    toggle
        default { "off" }
        disablewhen "{ snapMethod == grid } { useTargetGroup == 0 }"
    }
    parm {
        name    "posAttrib"
        cppname "PosAttrib"
        label   "Position Attribute"
        type    string
        default { "P" }
        parmtag { "sop_input" "0" }
    }
    parm {
        name    "fuse2D"
        cppname "Fuse2D"
        label   "Fuse 2D"
        type    toggle
        joinnext
        nolabel
        default { "off" }
    }
    parm {
        name    "axis"
        cppname "Axis"
        label   "Axis"
        type    ordinal
        default { "y" }
        menu {
            "x"         "X"
            "y"         "Y"
            "z"         "Z"
        }
        disablewhen "{ fuse2D == 0 }"
    }
    parm {
        name    "fuseRefGeo2D"
        cppname "FuseRefGeo2D"
        label   "Fuse Ref Geo 2D"
        type    toggle
        default { "off" }
        disablewhen "{ fuse2D == 0 }"
    }


    groupsimple {
        name    "snapGroup"
        label   "Snap"
        grouptag { "group_type" "simple" }

        parm {
            name    "snapMethod"
            cppname "SnapMethod"
            label   "Snap Method"
            type    ordinal
            joinnext
            default { "point" }
            menu {
                "point"       "Point"
                "grid"        "Grid"
                "specified"   "Specified Point"
            }
        }
        parm {
            name    "snapAlgorithm"
            cppname "SnapAlgorithm"
            label   "Snap Algorithm"
            type    ordinal
            default { "closest" }
            hidewhen "{ snapMethod != point }"
            menu {
                "lowest"    "Least Target Point Number (for cloud reduction)"
                "closest"   "Closest Target Point (for disjoint pieces)"
            }
        }
        parm {
            name    "useSnapDist"
            cppname "UseSnapDist"
            label   "Use Snap Distance"
            type    toggle
            nolabel
            joinnext
            default { "on" }
            hidewhen "{ snapMethod != point }"
        }
        parm {
            name    "snapDist"
            cppname "SnapDist"
            label   "Snap Distance"
            type    log
            default { "0.00001" }
            disablewhen "{ snapMethod != point } { useSnapDist == 0 }"
            hidewhen "{ snapMethod != point }"
            range   { 0.001 10 }
        }
        parm {
            name    "targetClass"
            cppname "TargetClass"
            label   "Target Class"
            type    ordinal
            default { "point" }
            hidewhen "{ snapMethod != specified }"
            menu {
                "prim"      "Primitive"
                "point"     "Point"
                "vertex"    "Vertex"
                "detail"    "Detail"
            }
        }
        parm {
            name    "targetPointAttrib"
            cppname "TargetPointAttrib"
            label   "Target Point Attribute"
            type    string
            default { "snapTo" }
            hidewhen "{ snapMethod != specified }"
            parmtag { "sop_input" "0" }
        }
        parm {
            name    "usePositionSnapMethod"
            cppname "UsePositionSnapMethod"
            label   "Snap Positions"
            type    toggle
            nolabel
            joinnext
            default { "off" }
            hidewhen "{ snapMethod != point }"
        }
        parm {
            name    "positionSnapMethod"
            cppname "PositionSnapMethod"
            label   "Position Snap Method"
            type    ordinal
            default { "mean" }
            disablewhen "{ snapMethod != point } { usePositionSnapMethod == 0 }"
            hidewhen "{ snapMethod != point }"
            menu {
                "first"     "First Match"
                "last"      "Last Match"
                "min"       "Minimum"
                "max"       "Maximum"
                "mode"      "Mode"
                "mean"      "Average"
                "median"    "Median"
                "sum"       "Sum"
                "sumsquare" "Sum of Squares"
                "rms"       "Root Mean Square"
            }
        }
        parm {
            name    "useRadiusAttrib"
            cppname "UseRadiusAttrib"
            label   "Radius Attribute"
            type    toggle
            nolabel
            joinnext
            default { "off" }
            hidewhen "{ snapMethod != point }"
        }
        parm {
            name    "radiusAttrib"
            cppname "RadiusAttrib"
            label   "Radius Attribute"
            type    string
            default { "pscale" }
            disablewhen "{ useRadiusAttrib == 0 }"
            hidewhen "{ snapMethod != point }"
            parmtag { "sop_input" "1" }
        }
        parm {
            name    "useMatchAttrib"
            cppname "UseMatchAttrib"
            label   "Match Attribute"
            type    toggle
            nolabel
            joinnext
            default { "off" }
            hidewhen "{ snapMethod != point }"
        }
        parm {
            name    "matchAttrib"
            cppname "MatchAttrib"
            label   "Match Attribute"
            type    string
            default { "name" }
            disablewhen "{ useMatchAttrib == 0 }"
            hidewhen "{ snapMethod != point }"
            parmtag { "sop_input" "1" }
        }
        parm {
            name    "matchAttribClass"
            cppname "MatchAttribClass"
            label   "Match Attribute Class"
            type    ordinal
            default { "point" }
            menu {
                "prim"      "Primitive"
                "point"     "Point"
                "vertex"    "Vertex"
            }
        }
        parm {
            name    "mismatchAttrib"
            cppname "MismatchAttrib"
            label   "Mismatch Attribute"
            type    toggle
            default { "off" }
            disablewhen "{ useMatchAttrib == 0 }"
            hidewhen "{ snapMethod != point }"
        }
        parm {
            name    "matchTol"
            cppname "MatchTol"
            label   "Match Tolerance"
            type    log
            default { "0" }
            disablewhen "{ useMatchAttrib == 0 }"
            hidewhen "{ snapMethod != point }"
            range   { 0.001 1 }
        }
        parm {
            name    "gridType"
            cppname "GridType"
            label   "Grid Type"
            type    ordinal
            default { "spacing" }
            hidewhen "{ snapMethod != grid }"
            menu {
                "spacing"   "Grid Spacing"
                "lines"     "Grid Lines"
                "pow2"      "Power of 2 Grid Lines"
            }
        }
        parm {
            name    "gridSpacing"
            cppname "GridSpacing"
            label   "Grid Spacing"
            type    vector
            size    3
            default { "0.1" "0.1" "0.1" }
            disablewhen "{ gridType != spacing }"
            hidewhen "{ snapMethod != grid }"
            range   { -1 1 }
        }
        parm {
            name    "gridLines"
            cppname "GridLines"
            label   "Grid Lines"
            type    vector
            size    3
            default { "10" "10" "10" }
            disablewhen "{ gridType != lines }"
            hidewhen "{ snapMethod != grid }"
            range   { -1 1 }
        }
        parm {
            name    "gridPow2"
            cppname "GridPow2"
            label   "Grid Power 2"
            type    integer
            size    3
            default { "3" "3" "3" }
            disablewhen "{ gridType != pow2 }"
            hidewhen "{ snapMethod != grid }"
            range   { 0 10 }
        }
        parm {
            name    "gridOffset"
            cppname "GridOffset"
            label   "Grid Offset"
            type    vector
            size    3
            default { "0" "0" "0" }
            hidewhen "{ snapMethod != grid }"
            range   { -1 1 }
        }
        parm {
            name    "gridRound"
            cppname "GridRound"
            label   "Grid Round"
            type    ordinal
            default { "nearest" }
            hidewhen "{ snapMethod != grid }"
            menu {
                "nearest"   "Nearest"
                "down"      "Down"
                "up"        "Up"
            }
        }
        parm {
            name    "useGridTol"
            cppname "UseGridTol"
            label   "Use Grid Tolerance"
            type    toggle
            nolabel
            joinnext
            default { "on" }
            hidewhen "{ snapMethod != grid }"
        }
        parm {
            name    "gridTol"
            cppname "GridTol"
            label   "Grid Tolerance"
            type    float
            default { "10" }
            disablewhen "{ snapMethod != grid } { useGridTol == 0 }"
            hidewhen "{ snapMethod != grid }"
            range   { 0.001 10 }
        }
    }

    groupsimple {
        name    "fuse_folder"
        label   "Fuse"
        grouptag { "group_type" "simple" }

        parm {
            name    "fuseSnappedPoint"
            cppname "FuseSnappedPoint"
            label   "Fuse Snapped Points"
            type    toggle
            default { "on" }
        }
        parm {
            name    "keepFusedPoint"
            cppname "KeepFusedPoint"
            label   "Keep Fused Points"
            type    toggle
            default { "off" }
            disablewhen "{ fuseSnappedPoint == 0 }"
        }
        parm {
            name    "delDegenerate"
            cppname "DelDegenerate"
            label   "Remove Repeated Vertices and Degenerate Primitives"
            type    toggle
            default { "on" }
            disablewhen "{ fuseSnappedPoint == 0 }"
        }
        parm {
            name    "delDegenPoint"
            cppname "DelDegenPoint"
            label   "Remove Unused Points from Degenerate Primitives"
            type    toggle
            default { "on" }
            disablewhen "{ fuseSnappedPoint == 0 } { delDegenerate == 0 }"
        }
        parm {
            name    "delUnusedPoint"
            cppname "DelUnusedPoint"
            label   "Remove All Unused Point"
            type    toggle
            default { "off" }
            disablewhen "{ fuseSnappedPoint == 0 }"
        }
    }

    groupsimple {
        name    "outputGroup"
        label   "Output Attributes and Groups"
        grouptag { "group_type" "simple" }

        parm {
            name    "recomputeNormal"
            cppname "RecomputeNormal"
            label   "Recompute Affected Normals"
            type    toggle
            default { "off" }
        }
        parm {
            name    "createSnappedGroup"
            cppname "CreateSnappedGroup"
            label   "Create Snapped Points Group"
            type    toggle
            nolabel
            joinnext
            default { "off" }
        }
        parm {
            name    "snappedGroupName"
            cppname "SnappedGroupName"
            label   "Snapped Point Group"
            type    string
            default { "snapped" }
            disablewhen "{ createSnappedGroup == 0 }"
        }
        parm {
            name    "createSnappedAttrib"
            cppname "CreateSnappedAttrib"
            label   "Create Snapped Destination Attribute"
            type    toggle
            nolabel
            joinnext
            default { "off" }
            disablewhen "{ snapMethod == grid }"
        }
        parm {
            name    "snappedAttribName"
            cppname "SnappedAttribName"
            label   "Snapped Destination Attribute"
            type    string
            default { "snapTo" }
            disablewhen "{ createSnappedAttrib == 0 } { snapMethod == grid }"
        }
        parm {
            name    "keepEdgeGroup"
            cppname "KeepEdgeGroup"
            label   "Keep Edge Group"
            type    string
            default { "*" }
        }
        multiparm {
            name    "numPointAttrib"
            cppname "NumPointAttrib"
            label   "Attributes to Snap"
            default 0

            parm {
                name    "attribMergeMethod#"
                cppname "AttribMergeMethod#"
                label   "Attrib Merge Method"
                type    ordinal
                default { "8" }
                menu {
                    "first"     "First Match"
                    "last"      "Last Match"
                    "min"       "Minimum"
                    "max"       "Maximum"
                    "mode"      "Mode"
                    "mean"      "Average"
                    "median"    "Median"
                    "sum"       "Sum"
                    "sumsquare" "Sum of Squares"
                    "rms"       "Root Mean Square"
                }
            }
            parm {
                name    "pointAttribs#"
                cppname "PointAttribs#"
                label   "Point Attribute"
                type    string
                default { "" }
                parmtag { "sop_input" "1" }
            }
        }

        multiparm {
            name    "numPointGroups"
            cppname "NumPointGroups"
            label   "Groups to Snap"
            default 0

            parm {
                name    "groupMergeMethod#"
                cppname "GroupMergeMethod#"
                label   "Group Merge Method"
                type    ordinal
                default { "0" }
                menu {
                    "first"     "First Match"
                    "last"      "Last Match"
                    "min"       "Minimum"
                    "max"       "Maximum"
                    "mode"      "Mode"
                }
            }
            parm {
                name    "pointGroups#"
                cppname "PointGroups#"
                label   "Point Group"
                type    string
                default { "" }
                parmtag { "sop_input" "1" }
            }
        }
    }

    // parm {
    //     name    "subscribeRatio"
    //     cppname "SubscribeRatio"
    //     label   "Subscribe Ratio"
    //     type    integer
    //     default { 64 }
    //     range   { 0! 256 }
    // }
    // parm {
    //     name    "minGrainSize"
    //     cppname "MinGrainSize"
    //     label   "Min Grain Size"
    //     type    intlog
    //     default { 1024 }
    //     range   { 0! 2048 }
    // }
}
)THEDSFILE";

PRM_Template*
SOP_FeE_Fuse_1_0::buildTemplates()
{
    static PRM_TemplateBuilder templ("SOP_FeE_Fuse_1_0.C"_sh, theDsFile);
    if (templ.justBuilt())
    {
        templ.setChoiceListPtr("group"_sh, &SOP_Node::groupMenu);
        templ.setChoiceListPtr("posAttrib"_sh, &SOP_Node::pointAttribReplaceMenu);
        templ.setChoiceListPtr("pointAttribs#"_sh, &SOP_Node::pointAttribMenu);
        templ.setChoiceListPtr("pointGroups#"_sh,  &SOP_Node::pointGroupMenu);
        templ.setChoiceListPtr("keepEdgeGroup"_sh,  &SOP_Node::edgeGroupMenu);
    }
    return templ.templates();
}

const UT_StringHolder SOP_FeE_Fuse_1_0::theSOPTypeName("FeE::fuse::1.0"_sh);

void
newSopOperator(OP_OperatorTable* table)
{
    OP_Operator* newOp = new OP_Operator(
        SOP_FeE_Fuse_1_0::theSOPTypeName,
        "FeE Fuse",
        SOP_FeE_Fuse_1_0::myConstructor,
        SOP_FeE_Fuse_1_0::buildTemplates(),
        1,
        2,
        nullptr,
        OP_FLAG_GENERATOR,
        nullptr,
        1,
        "Five elements Elf/Topo/Optimize");

    newOp->setIconName("SOP_fuse-2.0");
    table->addOperator(newOp);

}





class SOP_FeE_Fuse_1_0Verb : public SOP_NodeVerb
{
public:
    SOP_FeE_Fuse_1_0Verb() {}
    virtual ~SOP_FeE_Fuse_1_0Verb() {}

    virtual SOP_NodeParms *allocParms() const { return new SOP_FeE_Fuse_1_0Parms(); }
    virtual UT_StringHolder name() const { return SOP_FeE_Fuse_1_0::theSOPTypeName; }

    virtual CookMode cookMode(const SOP_NodeParms *parms) const { return COOK_INPLACE; }

    virtual void cook(const CookParms &cookparms) const;
    
    /// This static data member automatically registers
    /// this verb class at library load time.
    static const SOP_NodeVerb::Register<SOP_FeE_Fuse_1_0Verb> theVerb;
};

// The static member variable definition has to be outside the class definition.
// The declaration is inside the class.
const SOP_NodeVerb::Register<SOP_FeE_Fuse_1_0Verb> SOP_FeE_Fuse_1_0Verb::theVerb;

const SOP_NodeVerb *
SOP_FeE_Fuse_1_0::cookVerb() const 
{ 
    return SOP_FeE_Fuse_1_0Verb::theVerb.get();
}



#if GFE_Fuse_UnderlyingAlgorithm_UseGU

static GU_Snap::AttributeMergeMethod
sopPosMergeMethod(const SOP_FeE_Fuse_1_0Parms::PositionSnapMethod parmPositionSnapMethod)
{
    using namespace SOP_FeE_Fuse_1_0Enums;
    switch (parmPositionSnapMethod)
    {
    case PositionSnapMethod::FIRST:       return GU_Snap::AttributeMergeMethod::MERGE_ATTRIBUTE_FIRST;      break;
    case PositionSnapMethod::LAST:        return GU_Snap::AttributeMergeMethod::MERGE_ATTRIBUTE_LAST;       break;
    case PositionSnapMethod::MIN:         return GU_Snap::AttributeMergeMethod::MERGE_ATTRIBUTE_MIN;        break;
    case PositionSnapMethod::MAX:         return GU_Snap::AttributeMergeMethod::MERGE_ATTRIBUTE_MAX;        break;
    case PositionSnapMethod::MODE:        return GU_Snap::AttributeMergeMethod::MERGE_ATTRIBUTE_MODE;       break;
    case PositionSnapMethod::MEAN:        return GU_Snap::AttributeMergeMethod::MERGE_ATTRIBUTE_MEAN;       break;
    case PositionSnapMethod::MEDIAN:      return GU_Snap::AttributeMergeMethod::MERGE_ATTRIBUTE_MEDIAN;     break;
    case PositionSnapMethod::SUM:         return GU_Snap::AttributeMergeMethod::MERGE_ATTRIBUTE_SUM;        break;
    case PositionSnapMethod::SUMSQUARE:   return GU_Snap::AttributeMergeMethod::MERGE_ATTRIBUTE_SUMSQUARE;  break;
    case PositionSnapMethod::RMS:         return GU_Snap::AttributeMergeMethod::MERGE_ATTRIBUTE_RMS;        break;
    }
    UT_ASSERT_MSG(0, "Unhandled Position Merge Method!");
    return GU_Snap::AttributeMergeMethod::MERGE_ATTRIBUTE_FIRST;
}

static GU_Snap::AttributeMergeMethod
sopAttribMergeMethod(const SOP_FeE_Fuse_1_0Parms::AttribMergeMethod parmAttribMergeMethod)
{
    using namespace SOP_FeE_Fuse_1_0Enums;
    switch (parmAttribMergeMethod)
    {
    case AttribMergeMethod::FIRST:       return GU_Snap::AttributeMergeMethod::MERGE_ATTRIBUTE_FIRST;      break;
    case AttribMergeMethod::LAST:        return GU_Snap::AttributeMergeMethod::MERGE_ATTRIBUTE_LAST;       break;
    case AttribMergeMethod::MIN:         return GU_Snap::AttributeMergeMethod::MERGE_ATTRIBUTE_MIN;        break;
    case AttribMergeMethod::MAX:         return GU_Snap::AttributeMergeMethod::MERGE_ATTRIBUTE_MAX;        break;
    case AttribMergeMethod::MODE:        return GU_Snap::AttributeMergeMethod::MERGE_ATTRIBUTE_MODE;       break;
    case AttribMergeMethod::MEAN:        return GU_Snap::AttributeMergeMethod::MERGE_ATTRIBUTE_MEAN;       break;
    case AttribMergeMethod::MEDIAN:      return GU_Snap::AttributeMergeMethod::MERGE_ATTRIBUTE_MEDIAN;     break;
    case AttribMergeMethod::SUM:         return GU_Snap::AttributeMergeMethod::MERGE_ATTRIBUTE_SUM;        break;
    case AttribMergeMethod::SUMSQUARE:   return GU_Snap::AttributeMergeMethod::MERGE_ATTRIBUTE_SUMSQUARE;  break;
    case AttribMergeMethod::RMS:         return GU_Snap::AttributeMergeMethod::MERGE_ATTRIBUTE_RMS;        break;
    }
    UT_ASSERT_MSG(0, "Unhandled Attrib Merge Method!");
    return GU_Snap::AttributeMergeMethod::MERGE_ATTRIBUTE_FIRST;
}

SYS_FORCE_INLINE static GU_Snap::AttributeMergeMethod
sopAttribMergeMethod(const int64 parmAttribMergeMethod)
{ return sopAttribMergeMethod(static_cast<SOP_FeE_Fuse_1_0Parms::AttribMergeMethod>(parmAttribMergeMethod)); }

static GU_Snap::AttributeMergeMethod
sopGroupMergeMethod(const SOP_FeE_Fuse_1_0Parms::GroupMergeMethod parmGroupMergeMethod)
{
    using namespace SOP_FeE_Fuse_1_0Enums;
    switch (parmGroupMergeMethod)
    {
    case GroupMergeMethod::FIRST:       return GU_Snap::AttributeMergeMethod::MERGE_ATTRIBUTE_FIRST;      break;
    case GroupMergeMethod::LAST:        return GU_Snap::AttributeMergeMethod::MERGE_ATTRIBUTE_LAST;       break;
    case GroupMergeMethod::MIN:         return GU_Snap::AttributeMergeMethod::MERGE_ATTRIBUTE_MIN;        break;
    case GroupMergeMethod::MAX:         return GU_Snap::AttributeMergeMethod::MERGE_ATTRIBUTE_MAX;        break;
    case GroupMergeMethod::MODE:        return GU_Snap::AttributeMergeMethod::MERGE_ATTRIBUTE_MODE;       break;
    }
    UT_ASSERT_MSG(0, "Unhandled Group Merge Method!");
    return GU_Snap::AttributeMergeMethod::MERGE_ATTRIBUTE_FIRST;
}

SYS_FORCE_INLINE static GU_Snap::AttributeMergeMethod
sopGroupMergeMethod(const int64 parmGroupMergeMethod)
{ return sopGroupMergeMethod(static_cast<SOP_FeE_Fuse_1_0Parms::GroupMergeMethod>(parmGroupMergeMethod)); }




static GU_Snap::PointSnapParms::SnapAlgorithm
sopSnapAlgorithm(SOP_FeE_Fuse_1_0Parms::SnapAlgorithm parmSnapAlgorithm)
{
    using namespace SOP_FeE_Fuse_1_0Enums;
    switch (parmSnapAlgorithm)
    {
    case SnapAlgorithm::CLOSEST:     return GU_Snap::PointSnapParms::SnapAlgorithm::ALGORITHM_CLOSEST_POINT;    break;
    case SnapAlgorithm::LOWEST:      return GU_Snap::PointSnapParms::SnapAlgorithm::ALGORITHM_LOWEST_POINT;     break;
    }
    UT_ASSERT_MSG(0, "Unhandled Snap Algorithm!");
    return GU_Snap::PointSnapParms::SnapAlgorithm::ALGORITHM_CLOSEST_POINT;
}

    
static int
sopGridRound(SOP_FeE_Fuse_1_0Parms::GridRound parmGridRound)
{
    using namespace SOP_FeE_Fuse_1_0Enums;
    switch (parmGridRound)
    {
    case GridRound::NEAREST:    return 0; break;
    case GridRound::DOWN:       return 1; break;
    case GridRound::UP:         return 2; break;
    }
    UT_ASSERT_MSG(0, "Unhandled Snap Method!");
    return 0;
}






#else





static SOP_Fuse_2_0Enums::PositionSnapMethod
sopPosMergeMethod(const SOP_FeE_Fuse_1_0Parms::PositionSnapMethod parmPositionSnapMethod)
{
    using namespace SOP_FeE_Fuse_1_0Enums;
    switch (parmPositionSnapMethod)
    {
    case PositionSnapMethod::FIRST:       return SOP_Fuse_2_0Enums::PositionSnapMethod::LOWEST;     break;
    case PositionSnapMethod::LAST:        return SOP_Fuse_2_0Enums::PositionSnapMethod::HIGHEST;    break;
    case PositionSnapMethod::MIN:         return SOP_Fuse_2_0Enums::PositionSnapMethod::MIN;        break;
    case PositionSnapMethod::MAX:         return SOP_Fuse_2_0Enums::PositionSnapMethod::MAX;        break;
    case PositionSnapMethod::MODE:        return SOP_Fuse_2_0Enums::PositionSnapMethod::MODE;       break;
    case PositionSnapMethod::MEAN:        return SOP_Fuse_2_0Enums::PositionSnapMethod::AVERAGE;    break;
    case PositionSnapMethod::MEDIAN:      return SOP_Fuse_2_0Enums::PositionSnapMethod::MEDIAN;     break;
    case PositionSnapMethod::SUM:         return SOP_Fuse_2_0Enums::PositionSnapMethod::SUM;        break;
    case PositionSnapMethod::SUMSQUARE:   return SOP_Fuse_2_0Enums::PositionSnapMethod::SUMSQUARE;  break;
    case PositionSnapMethod::RMS:         return SOP_Fuse_2_0Enums::PositionSnapMethod::RMS;        break;
    }
    UT_ASSERT_MSG(0, "Unhandled Position Merge Method!");
    return SOP_Fuse_2_0Enums::PositionSnapMethod::LOWEST;
}

static SOP_Fuse_2_0Enums::Attribsnapmethod
sopAttribMergeMethod(const SOP_FeE_Fuse_1_0Parms::AttribMergeMethod parmAttribMergeMethod)
{
    using namespace SOP_FeE_Fuse_1_0Enums;
    switch (parmAttribMergeMethod)
    {
    case AttribMergeMethod::FIRST:       return SOP_Fuse_2_0Enums::Attribsnapmethod::FIRST;      break;
    case AttribMergeMethod::LAST:        return SOP_Fuse_2_0Enums::Attribsnapmethod::LAST;       break;
    case AttribMergeMethod::MIN:         return SOP_Fuse_2_0Enums::Attribsnapmethod::MIN;        break;
    case AttribMergeMethod::MAX:         return SOP_Fuse_2_0Enums::Attribsnapmethod::MAX;        break;
    case AttribMergeMethod::MODE:        return SOP_Fuse_2_0Enums::Attribsnapmethod::MODE;       break;
    case AttribMergeMethod::MEAN:        return SOP_Fuse_2_0Enums::Attribsnapmethod::MEAN;       break;
    case AttribMergeMethod::MEDIAN:      return SOP_Fuse_2_0Enums::Attribsnapmethod::MEDIAN;     break;
    case AttribMergeMethod::SUM:         return SOP_Fuse_2_0Enums::Attribsnapmethod::SUM;        break;
    case AttribMergeMethod::SUMSQUARE:   return SOP_Fuse_2_0Enums::Attribsnapmethod::SUMSQUARE;  break;
    case AttribMergeMethod::RMS:         return SOP_Fuse_2_0Enums::Attribsnapmethod::RMS;        break;
    }
    UT_ASSERT_MSG(0, "Unhandled Attrib Merge Method!");
    return SOP_Fuse_2_0Enums::Attribsnapmethod::FIRST;
}

SYS_FORCE_INLINE static SOP_Fuse_2_0Enums::Attribsnapmethod
sopAttribMergeMethod(const int64 parmAttribMergeMethod)
{ return sopAttribMergeMethod(static_cast<SOP_FeE_Fuse_1_0Parms::AttribMergeMethod>(parmAttribMergeMethod)); }

SYS_FORCE_INLINE static int64
sopAttribMergeMethodInt64(const int64 parmAttribMergeMethod)
{ return static_cast<int64>(sopAttribMergeMethod(parmAttribMergeMethod)); }


    
    
static SOP_Fuse_2_0Enums::Grouppropagation
sopGroupMergeMethod(const SOP_FeE_Fuse_1_0Parms::GroupMergeMethod parmGroupMergeMethod)
{
    using namespace SOP_FeE_Fuse_1_0Enums;
    switch (parmGroupMergeMethod)
    {
    case GroupMergeMethod::FIRST:       return SOP_Fuse_2_0Enums::Grouppropagation::LEASTPOINTNUMBER;      break;
    case GroupMergeMethod::LAST:        return SOP_Fuse_2_0Enums::Grouppropagation::GREATESTPOINTNUMBER;   break;
    case GroupMergeMethod::MIN:         return SOP_Fuse_2_0Enums::Grouppropagation::INTERSECT;             break;
    case GroupMergeMethod::MAX:         return SOP_Fuse_2_0Enums::Grouppropagation::UNION;                 break;
    case GroupMergeMethod::MODE:        return SOP_Fuse_2_0Enums::Grouppropagation::MODE;                  break;
    }
    UT_ASSERT_MSG(0, "Unhandled Group Merge Method!");
    return SOP_Fuse_2_0Enums::Grouppropagation::LEASTPOINTNUMBER;
}

SYS_FORCE_INLINE static SOP_Fuse_2_0Enums::Grouppropagation
sopGroupMergeMethod(const int64 parmGroupMergeMethod)
{ return sopGroupMergeMethod(static_cast<SOP_FeE_Fuse_1_0Parms::GroupMergeMethod>(parmGroupMergeMethod)); }

SYS_FORCE_INLINE static int64
sopGroupMergeMethodInt64(const int64 parmAttribMergeMethod)
{ return static_cast<int64>(sopGroupMergeMethod(parmAttribMergeMethod)); }




static SOP_Fuse_2_0Enums::Algorithm
sopSnapAlgorithm(const SOP_FeE_Fuse_1_0Parms::SnapAlgorithm parmSnapAlgorithm)
{
    using namespace SOP_FeE_Fuse_1_0Enums;
    switch (parmSnapAlgorithm)
    {
    case SnapAlgorithm::CLOSEST:     return SOP_Fuse_2_0Enums::Algorithm::CLOSEST;    break;
    case SnapAlgorithm::LOWEST:      return SOP_Fuse_2_0Enums::Algorithm::LOWEST;     break;
    }
    UT_ASSERT_MSG(0, "Unhandled Snap Algorithm!");
    return SOP_Fuse_2_0Enums::Algorithm::CLOSEST;
}

    
static SOP_Fuse_2_0Enums::Gridtype
sopGridType(const SOP_FeE_Fuse_1_0Parms::GridType parmGridType)
{
    using namespace SOP_FeE_Fuse_1_0Enums;
    switch (parmGridType)
    {
    case GridType::SPACING: return SOP_Fuse_2_0Enums::Gridtype::SPACING;  break;
    case GridType::LINES:   return SOP_Fuse_2_0Enums::Gridtype::LINES;    break;
    case GridType::POW2:    return SOP_Fuse_2_0Enums::Gridtype::POW2;     break;
    }
    UT_ASSERT_MSG(0, "Unhandled Grid Type!");
    return SOP_Fuse_2_0Enums::Gridtype::SPACING;
}
    
static SOP_Fuse_2_0Enums::Gridround
sopGridRound(const SOP_FeE_Fuse_1_0Parms::GridRound parmGridRound)
{
    using namespace SOP_FeE_Fuse_1_0Enums;
    switch (parmGridRound)
    {
    case GridRound::NEAREST:    return SOP_Fuse_2_0Enums::Gridround::NEAREST; break;
    case GridRound::DOWN:       return SOP_Fuse_2_0Enums::Gridround::DOWN;    break;
    case GridRound::UP:         return SOP_Fuse_2_0Enums::Gridround::UP;      break;
    }
    UT_ASSERT_MSG(0, "Unhandled Grid Round!");
    return SOP_Fuse_2_0Enums::Gridround::NEAREST;
}

    
#endif

    


static GFE_Fuse::Method
sopMethod(const SOP_FeE_Fuse_1_0Parms::SnapMethod parmMethod)
{
    using namespace SOP_FeE_Fuse_1_0Enums;
    switch (parmMethod)
    {
    case SnapMethod::POINT:      return GFE_Fuse::Method::Point;     break;
    case SnapMethod::GRID:       return GFE_Fuse::Method::Grid;      break;
    case SnapMethod::SPECIFIED:  return GFE_Fuse::Method::Specified; break;
    }
    UT_ASSERT_MSG(0, "Unhandled Snap Method!");
    return GFE_Fuse::Method::Point;
}

static GA_GroupType
sopGroupType(const SOP_FeE_Fuse_1_0Parms::GroupType parmGroupType)
{
    using namespace SOP_FeE_Fuse_1_0Enums;
    switch (parmGroupType)
    {
    case GroupType::GUESS:     return GA_GROUP_INVALID;    break;
    case GroupType::PRIM:      return GA_GROUP_PRIMITIVE;  break;
    case GroupType::POINT:     return GA_GROUP_POINT;      break;
    case GroupType::VERTEX:    return GA_GROUP_VERTEX;     break;
    case GroupType::EDGE:      return GA_GROUP_EDGE;       break;
    }
    UT_ASSERT_MSG(0, "Unhandled Group Type!");
    return GA_GROUP_INVALID;
}

static GA_AttributeOwner
sopAttribOwner(const SOP_FeE_Fuse_1_0Parms::TargetClass parmAttribOwner)
{
    using namespace SOP_FeE_Fuse_1_0Enums;
    switch (parmAttribOwner)
    {
    case TargetClass::PRIM:      return GA_ATTRIB_PRIMITIVE;  break;
    case TargetClass::POINT:     return GA_ATTRIB_POINT;      break;
    case TargetClass::VERTEX:    return GA_ATTRIB_VERTEX;     break;
    case TargetClass::DETAIL:    return GA_ATTRIB_DETAIL;     break;
    }
    UT_ASSERT_MSG(0, "Unhandled Attrib Owner!");
    return GA_ATTRIB_INVALID;
}

static GA_AttributeOwner
sopAttribOwner(const SOP_FeE_Fuse_1_0Parms::MatchAttribClass parmMatchAttribClass)
{
    using namespace SOP_FeE_Fuse_1_0Enums;
    switch (parmMatchAttribClass)
    {
    case MatchAttribClass::PRIM:      return GA_ATTRIB_PRIMITIVE;  break;
    case MatchAttribClass::POINT:     return GA_ATTRIB_POINT;      break;
    case MatchAttribClass::VERTEX:    return GA_ATTRIB_VERTEX;     break;
    }
    UT_ASSERT_MSG(0, "Unhandled Attrib Owner!");
    return GA_ATTRIB_INVALID;
}

    
void
SOP_FeE_Fuse_1_0Verb::cook(const SOP_NodeVerb::CookParms &cookparms) const
{
    auto&& sopparms = cookparms.parms<SOP_FeE_Fuse_1_0Parms>();
    GA_Detail& outGeo0 = *cookparms.gdh().gdpNC();
    //auto sopcache = (SOP_FeE_Fuse_1_0Cache*)cookparms.cache();

    //const GA_Detail& inGeo0 = *cookparms.inputGeo(0);
    const GA_Detail* const inGeo1 = cookparms.inputGeo(1);

    //outGeo0.replaceWith(inGeo0);

    const GA_GroupType groupType = sopGroupType(sopparms.getGroupType());
    const GFE_Fuse::Method method = sopMethod(sopparms.getSnapMethod());
    const GA_AttributeOwner targetClass = sopAttribOwner(sopparms.getTargetClass());
    const GA_AttributeOwner matchAttribClass = sopAttribOwner(sopparms.getMatchAttribClass());
    

    UT_AutoInterrupt boss("Processing");
    if (boss.wasInterrupted())
        return;
    
/*
    GFE_Fuse fuse(geo, nullptr, cookparms);
    GFE_Fuse fuse(geoOriginTmp, nullptr, cookparms);
    fuse.method = GFE_Fuse::Method::Point;
    fuse.setPointComputeParm(true, fuseDist, true);
    fuse.setConsolidate();
    //fuse.setPositionAttrib("P");
    fuse.compute();
*/
    
    GFE_Fuse fuse(outGeo0, inGeo1, &cookparms);

    fuse.setPositionAttrib(sopparms.getPosAttrib());    
    if (inGeo1)
        fuse.setPositionRef0Attrib(sopparms.getPosAttrib());

    if (sopparms.getFuse2D())
        fuse.setFuse2D(static_cast<int8>(sopparms.getAxis()), sopparms.getFuseRefGeo2D());
    

    fuse.setComputeParm(method, sopparms.getModifyTarget());
    fuse.setConsolidate(sopparms.getFuseSnappedPoint(), !sopparms.getKeepFusedPoint(), sopparms.getDelUnusedPoint());
    
    switch (method)
    {
    case GFE_Fuse::Method::Point:
        //fuse.setPointComputeParm(sopparms.getUseSnapDist(), sopparms.getSnapDist(), sopSnapAlgorithm(sopparms.getSnapAlgorithm()));
        fuse.setPointComputeParm(sopparms.getUseSnapDist(), sopparms.getSnapDist(), static_cast<bool>(sopparms.getSnapAlgorithm()));
        if (sopparms.getUseRadiusAttrib())
            fuse.setRadiusAttrib(sopparms.getRadiusAttrib());
    
        if (sopparms.getUseMatchAttrib())
        {
            fuse.setMatchAttrib(matchAttribClass, sopparms.getMatchAttrib());
            fuse.setPointMatchComputeParm(sopparms.getMatchTol(), sopparms.getMismatchAttrib());
        }
        
        if (sopparms.getUsePositionSnapMethod())
            fuse.setPosMergeMethod(sopPosMergeMethod(sopparms.getPositionSnapMethod()));
        
    break;
    case GFE_Fuse::Method::Grid:
        
#if GFE_Fuse_UnderlyingAlgorithm_UseGU
        fuse.setGridComputeParm(sopGridRound(sopparms.getGridRound()),
            sopparms.getGridSpacing(), sopparms.getGridOffset(),
            sopparms.getUseGridTol() ? sopparms.getGridTol() : SYS_FP64_MAX);
#else
        fuse.setGridComputeParm(sopGridType(sopparms.getGridType()), sopGridRound(sopparms.getGridRound()),
            sopparms.getGridSpacing(), sopparms.getGridLines(), sopparms.getGridPow2(),
            sopparms.getUseGridTol(), sopparms.getGridTol());
#endif
    
        
    break;
    case GFE_Fuse::Method::Specified:
        fuse.setSpecifiedComputeParm(targetClass, sopparms.getTargetPointAttrib());
        
        if (sopparms.getUsePositionSnapMethod())
            fuse.setPosMergeMethod(sopPosMergeMethod(sopparms.getPositionSnapMethod()));
        
    break;
    default: UT_ASSERT_MSG(0, "Unhandled Fuse Method"); break;
    }

#if GFE_Fuse_UnderlyingAlgorithm_UseGU
    if (sopparms.getCreateSnappedGroup())
        fuse.setSnappedGroup(false, sopparms.getSnappedGroupName());
    if (sopparms.getCreateSnappedAttrib())
        fuse.setSnapAttrib(false, sopparms.getSnappedAttribName());
#else
    if (sopparms.getCreateSnappedGroup())
        fuse.setSnappedGroup(sopparms.getSnappedGroupName());
    if (sopparms.getCreateSnappedAttrib())
        fuse.setSnapAttrib(sopparms.getSnappedAttribName());
#endif
    





        

#if GFE_Fuse_UnderlyingAlgorithm_UseGU


    GFE_GroupArray&        groupArray     = fuse.getOutGroupArray();
    GFE_RefGroupArray&     refGroupArray  = fuse.getRef0GroupArray();
    GFE_AttributeArray&    attribArray    = fuse.getOutAttribArray();
    GFE_RefAttributeArray& refAttribArray = fuse.getRef0AttribArray();
        
    const UT_Array<SOP_FeE_Fuse_1_0Parms::NumPointAttrib>& numPointAttrib = sopparms.getNumPointAttrib();
    const size_t sizeAttrib = numPointAttrib.size();
    if (fuse.getRef0AttribArray().isValid())
    {
        for (size_t i = 0; i < sizeAttrib; ++i)
        {
            const GU_Snap::AttributeMergeMethod mergeMethod = sopAttribMergeMethod(numPointAttrib[i].attribMergeMethod);

            const size_t sizeStart = refAttribArray.size();
            refAttribArray.appends(GA_ATTRIB_POINT, numPointAttrib[i].pointAttribs);
            const size_t sizeEnd   = refAttribArray.size();
            
            for (size_t j = sizeStart; j < sizeEnd; ++j)
            {
                const GA_Attribute* const srcGroup = refAttribArray[j];
                GA_Attribute* const dstGroup = attribArray.clone(*srcGroup);
                fuse.emplaceMergeAttribs(GU_Snap::AttributeMergeData(mergeMethod, srcGroup, dstGroup));
            }
        }
    }
    else
    {
        for (size_t i = 0; i < sizeAttrib; ++i)
        {
            const GU_Snap::AttributeMergeMethod mergeMethod = sopAttribMergeMethod(numPointAttrib[i].attribMergeMethod);

            const size_t sizeStart = attribArray.size();
            attribArray.appends(GA_ATTRIB_POINT, numPointAttrib[i].pointAttribs);
            const size_t sizeEnd   = attribArray.size();
            
            for (size_t j = sizeStart; j < sizeEnd; ++j)
            {
                GA_Attribute* const dstAttrib = attribArray[j];
                // UT_StringHolder name = dstAttrib->getName();
                // name += "__GFE_Temp_";
                // const GA_Attribute* const srcAttrib = GFE_Attribute::clone(outGeo0, *dstAttrib, name);
                fuse.emplaceMergeAttribs(GU_Snap::AttributeMergeData(mergeMethod, dstAttrib, dstAttrib));
            }
        }
    }


    
    const UT_Array<SOP_FeE_Fuse_1_0Parms::NumPointGroups>& numGroupAttrib = sopparms.getNumPointGroups();
    const size_t sizeGroup = numGroupAttrib.size();
    if (fuse.getRef0AttribArray().isValid())
    {
        for (size_t i = 0; i < sizeGroup; ++i)
        {
            const GU_Snap::AttributeMergeMethod mergeMethod = sopGroupMergeMethod(numGroupAttrib[i].groupMergeMethod);

            const size_t sizeStart = refGroupArray.size();
            refGroupArray.appends(GA_GROUP_POINT, numGroupAttrib[i].pointGroups);
            const size_t sizeEnd   = refGroupArray.size();
            
            for (size_t j = sizeStart; j < sizeEnd; ++j)
            {
                const GA_ElementGroup* const srcGroup = refGroupArray.getElementGroup(j);
                GA_ElementGroup* const dstGroup = groupArray.cloneElement(*srcGroup);
                fuse.emplaceMergeAttribs(GU_Snap::AttributeMergeData(mergeMethod, srcGroup, dstGroup));
            }
        }
    }
    else
    {
        for (size_t i = 0; i < sizeGroup; ++i)
        {
            const GU_Snap::AttributeMergeMethod mergeMethod = sopGroupMergeMethod(numGroupAttrib[i].groupMergeMethod);

            const size_t sizeStart = groupArray.size();
            groupArray.appends(GA_GROUP_POINT, numGroupAttrib[i].pointGroups);
            const size_t sizeEnd   = groupArray.size();
            
            for (size_t j = sizeStart; j < sizeEnd; ++j)
            {
                GA_ElementGroup* const dstGroup = groupArray.getElementGroup(j);
                fuse.emplaceMergeAttribs(GU_Snap::AttributeMergeData(mergeMethod, dstGroup, dstGroup));
            }
        }
    }




        
#else


        
    const UT_Array<SOP_FeE_Fuse_1_0Parms::NumPointAttrib>& numPointAttrib = sopparms.getNumPointAttrib();
    const size_t sizeAttrib = numPointAttrib.size();
    for (size_t i = 0; i < sizeAttrib; ++i)
    {
        SOP_Fuse_2_0Parms::Numpointattribs numAttribs;
        numAttribs.attribsnapmethod = sopAttribMergeMethodInt64(numPointAttrib[i].attribMergeMethod);
        numAttribs.pointattribnames = numPointAttrib[i].pointAttribs;
        fuse.emplaceMergeAttribs(numAttribs);
    }

    
    const UT_Array<SOP_FeE_Fuse_1_0Parms::NumPointGroups>& numGroupAttrib = sopparms.getNumPointGroups();
    const size_t sizeGroup = numGroupAttrib.size();
    for (size_t i = 0; i < sizeGroup; ++i)
    {
        SOP_Fuse_2_0Parms::Numgroups numAttribs;
        numAttribs.grouppropagation = sopGroupMergeMethodInt64(numGroupAttrib[i].groupMergeMethod);
        numAttribs.pointgroupnames = numGroupAttrib[i].pointGroups;
        fuse.emplaceMergeAttribs(numAttribs);
    }


    fuse.setRecomputeNormal(sopparms.getRecomputeNormal());
    
#endif
    

    fuse.setKeepEdgeGroup(sopparms.getKeepEdgeGroup());
    
    fuse.groupParser.setGroup(groupType, sopparms.getGroup());


    fuse.computeAndBumpDataId();
    fuse.visualizeOutGroup();

}


