
//#define UT_ASSERT_LEVEL 3
#include "SOP_FeE_PointInMesh_WN_2_0.h"


#include "SOP_FeE_PointInMesh_WN_2_0.proto.h"

#include "GA/GA_Detail.h"
#include "PRM/PRM_TemplateBuilder.h"
#include "UT/UT_Interrupt.h"
#include "UT/UT_DSOVersion.h"




#include "GFE/GFE_PointInMeshWN.h"




using namespace SOP_FeE_PointInMesh_WN_2_0_Namespace;


static const char *theDsFile = R"THEDSFILE(
{
    name        parameters
    parm {
        name    "wnQueryPointGroup"
        cppname "WNQueryPointGroup"
        label   "Query Point Group"
        type    string
        default { "" }
        parmtag { "script_action" "import soputils\nkwargs['geometrytype'] = (hou.geometryType.Points,)\nkwargs['inputindex'] = 0\nsoputils.selectGroupParm(kwargs)" }
        parmtag { "script_action_help" "Select geometry from an available viewport.\nShift-click to turn on Select Groups." }
        parmtag { "script_action_icon" "BUTTONS_reselect" }
    }
    parm {
        name    "wnMeshPrimGroup"
        cppname "WNMeshPrimGroup"
        label   "Mesh Primitive Group"
        type    string
        default { "" }
        parmtag { "script_action" "import soputils\nkwargs['geometrytype'] = (hou.geometryType.Primitives,)\nkwargs['inputindex'] = 1\nsoputils.selectGroupParm(kwargs)" }
        parmtag { "script_action_help" "Select geometry from an available viewport.\nShift-click to turn on Select Groups." }
        parmtag { "script_action_icon" "BUTTONS_reselect" }
        parmtag { "sop_input" "1" }
    }
    parm {
        name    "group"
        cppname "Group"
        label   "Group"
        type    string
        default { "`chs('wnQueryPointGroup')`" }
        parmtag { "script_action" "import soputils\nkwargs['geometrytype'] = kwargs['node'].parmTuple('groupType')\nkwargs['inputindex'] = 0\nsoputils.selectGroupParm(kwargs)" }
        parmtag { "script_action_help" "Select geometry from an available viewport." }
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
        name    "outWN"
        cppname "OutWN"
        label   "Output Winding Number"
        type    toggle
        nolabel
        joinnext
        default { "0" }
    }
    parm {
        name    "wnAttribName"
        cppname "WNAttribName"
        label   "Winding Number Attrib Name"
        type    string
        default { "windingNumber" }
        disablewhen "{ outWN == 0 }"
    }
    parm {
        name    "outGroup"
        cppname "OutGroup"
        label   "Output Group"
        type    toggle
        nolabel
        joinnext
        default { "1" }
    }
    parm {
        name    "groupName"
        cppname "GroupName"
        label   "Group Name"
        type    string
        default { "`opname('.')`" }
        disablewhen "{ outGroup == 0 }"
    }
    parm {
        name    "outNumericAttrib"
        cppname "OutNumericAttrib"
        label   "Output Integer Attrib"
        type    toggle
        nolabel
        joinnext
        default { "0" }
    }
    parm {
        name    "numericAttribName"
        cppname "NumericAttribName"
        label   "Integer Attrib Name"
        type    string
        default { "`opname('.')`" }
        disablewhen "{ outNumericAttrib == 0 }"
    }
    parm {
        name    "groupMergeType"
        cppname "GroupMergeType"
        label   "Group Merge Type"
        type    ordinal
        default { "replace" }
        menu {
            "replace"   "Replace Existing"
            "union"     "Union with Existing"
            "intersect" "Intersect with Existing"
            "subtract"  "Subtract from Existing"
        }
    }
    parm {
        name    "inGeo"
        cppname "InGeo"
        label   "In Geo"
        type    toggle
        default { "on" }
    }
    parm {
        name    "onGeo"
        cppname "OnGeo"
        label   "On Geo"
        type    toggle
        default { "on" }
    }
    parm {
        name    "threshold"
        cppname "Threshold"
        label   "Threshold"
        type    log
        default { "1e-05" }
        range   { 1e-05 1 }
    }
    groupsimple {
        name    "wn_folder"
        label   "Wingding Number"
        grouptag { "group_type" "simple" }

        parm {
            name    "wnType"
            cppname "WNType"
            label   "Winding Number Type"
            type    ordinal
            default { "xyz" }
            menu {
                "xyz"   "3D"
                "xy"    "2D in XY Plane"
                "yz"    "2D in YZ Plane"
                "zx"    "2D in ZX Plane"
            }
        }
        parm {
            name    "wnAsSolidAngle"
            cppname "WNAsSolidAngle"
            label   "Scale to Solid Angle"
            type    toggle
            default { "off" }
        }
        parm {
            name    "wnNegate"
            cppname "WNNegate"
            label   "Negate Value (Reverse)"
            type    toggle
            default { "off" }
        }
        parm {
            name    "wnFullAccuracy"
            cppname "WNFullAccuracy"
            label   "Full Accuracy (Slow)"
            type    toggle
            default { "off" }
        }
        parm {
            name    "wnAccuracyScale"
            cppname "WNAccuracyScale"
            label   "Accuracy Scale"
            type    float
            default { "2" }
            disablewhen "{ wnFullAccuracy == 1 }"
            range   { 1! 12! }
        }
        parm {
            name    "wnPrecision"
            cppname "WNPrecision"
            label   "Precision"
            type    ordinal
            default { "64" }
            menu {
                "auto"   "Auto"
                "_16"    "16"
                "_32"    "32"
                "_64"    "64"
            }
        }
    }

    parm {
        name    "reverseGroup"
        cppname "ReverseGroup"
        label   "Reverse Group"
        type    toggle
        default { "0" }
    }

    parm {
        name    "subscribeRatio"
        cppname "SubscribeRatio"
        label   "Subscribe Ratio"
        type    integer
        default { 64 }
        range   { 0! 256 }
    }
    parm {
        name    "minGrainSize"
        cppname "MinGrainSize"
        label   "Min Grain Size"
        type    intlog
        default { 64 }
        range   { 0! 2048 }
    }
}
)THEDSFILE";

PRM_Template*
SOP_FeE_PointInMesh_WN_2_0::buildTemplates()
{
    static PRM_TemplateBuilder templ("SOP_FeE_PointInMesh_WN_2_0.C"_sh, theDsFile);
    if (templ.justBuilt())
    {
        templ.setChoiceListPtr("wnQueryPointGroup"_sh, &SOP_Node::pointGroupMenu);
        templ.setChoiceListPtr("wnMeshPrimGroup"_sh, &SOP_Node::primGroupMenu);
        templ.setChoiceListPtr("wnAttribName"_sh, &SOP_Node::pointAttribReplaceMenu);
    }
    return templ.templates();
}

const UT_StringHolder SOP_FeE_PointInMesh_WN_2_0::theSOPTypeName("FeE::pointInMesh_wn::2.0"_sh);

void
newSopOperator(OP_OperatorTable* table)
{
    OP_Operator* newOp = new OP_Operator(
        SOP_FeE_PointInMesh_WN_2_0::theSOPTypeName,
        "FeE Point in Mesh Winding Number",
        SOP_FeE_PointInMesh_WN_2_0::myConstructor,
        SOP_FeE_PointInMesh_WN_2_0::buildTemplates(),
        2,
        2,
        nullptr,
        OP_FLAG_GENERATOR,
        nullptr,
        1,
        "Five elements Elf/Group/Bound");

    newOp->setIconName("SOP_windingnumber");
    table->addOperator(newOp);

}





class SOP_FeE_PointInMesh_WN_2_0Verb : public SOP_NodeVerb
{
public:
    SOP_FeE_PointInMesh_WN_2_0Verb() {}
    virtual ~SOP_FeE_PointInMesh_WN_2_0Verb() {}

    virtual SOP_NodeParms *allocParms() const { return new SOP_FeE_PointInMesh_WN_2_0Parms(); }
    virtual SOP_NodeCache* allocCache() const { return new GFE_WindingNumber_Cache(); }
    virtual UT_StringHolder name() const { return SOP_FeE_PointInMesh_WN_2_0::theSOPTypeName; }

    virtual CookMode cookMode(const SOP_NodeParms *parms) const { return COOK_GENERIC; }

    virtual void cook(const CookParms &cookparms) const;
    
    /// This static data member automatically registers
    /// this verb class at library load time.
    static const SOP_NodeVerb::Register<SOP_FeE_PointInMesh_WN_2_0Verb> theVerb;
};

// The static member variable definition has to be outside the class definition.
// The declaration is inside the class.
const SOP_NodeVerb::Register<SOP_FeE_PointInMesh_WN_2_0Verb> SOP_FeE_PointInMesh_WN_2_0Verb::theVerb;

const SOP_NodeVerb *
SOP_FeE_PointInMesh_WN_2_0::cookVerb() const 
{ 
    return SOP_FeE_PointInMesh_WN_2_0Verb::theVerb.get();
}






static GA_Storage
sopWNStorage(SOP_FeE_PointInMesh_WN_2_0Parms::WNPrecision wnPrecision)
{
    using namespace SOP_FeE_PointInMesh_WN_2_0Enums;
    switch (wnPrecision)
    {
    case WNPrecision::AUTO:     return GA_STORE_INVALID;   break;
    case WNPrecision::_16:      return GA_STORE_REAL16;    break;
    case WNPrecision::_32:      return GA_STORE_REAL32;    break;
    case WNPrecision::_64:      return GA_STORE_REAL64;    break;
    }
    UT_ASSERT_MSG(0, "Unhandled WNPrecision!");
    return GA_STORE_INVALID;
}

static GFE_WNType
sopWNType(SOP_FeE_PointInMesh_WN_2_0Parms::WNType wnType)
{
    using namespace SOP_FeE_PointInMesh_WN_2_0Enums;
    switch (wnType)
    {
    case WNType::XYZ:      return GFE_WNType::XYZ;   break;
    case WNType::XY:       return GFE_WNType::XY;    break;
    case WNType::YZ:       return GFE_WNType::YZ;    break;
    case WNType::ZX:       return GFE_WNType::ZX;    break;
    }
    UT_ASSERT_MSG(0, "Unhandled WNType!");
    return GFE_WNType::XYZ;
}

static GFE_GroupMergeType
sopGroupMergeType(SOP_FeE_PointInMesh_WN_2_0Parms::GroupMergeType groupMergeType)
{
    using namespace SOP_FeE_PointInMesh_WN_2_0Enums;
    switch (groupMergeType)
    {
    case GroupMergeType::REPLACE:     return GFE_GroupMergeType::Replace;    break;
    case GroupMergeType::UNION:       return GFE_GroupMergeType::Union;      break;
    case GroupMergeType::INTERSECT:   return GFE_GroupMergeType::Intersect;  break;
    case GroupMergeType::SUBTRACT:    return GFE_GroupMergeType::Subtract;   break;
    }
    UT_ASSERT_MSG(0, "Unhandled Group Merge Type!");
    return GFE_GroupMergeType::Replace;
}


void
SOP_FeE_PointInMesh_WN_2_0Verb::cook(const SOP_NodeVerb::CookParms &cookparms) const
{
    auto&& sopparms = cookparms.parms<SOP_FeE_PointInMesh_WN_2_0Parms>();
    GA_Detail& outGeo0 = *cookparms.gdh().gdpNC();
    auto sopcache = (GFE_WindingNumber_Cache*)cookparms.cache();

    const GA_Detail& inGeo0 = *cookparms.inputGeo(0);
    const GA_Detail& inGeo1 = *cookparms.inputGeo(1);

    outGeo0.replaceWith(inGeo0);




    const GA_Storage wnStorage = sopWNStorage(sopparms.getWNPrecision());
    const GFE_GroupMergeType groupMergeType = sopGroupMergeType(sopparms.getGroupMergeType());

    UT_AutoInterrupt boss("Processing");
    if (boss.wasInterrupted())
        return;
    
        
    GFE_PointInMeshWN pointInMeshWN(outGeo0, inGeo1, sopcache, &cookparms);

    pointInMeshWN.gfeWN.groupParser.setPointGroup(sopparms.getWNQueryPointGroup());
    pointInMeshWN.gfeWN.groupParserRef0.setPrimitiveGroup(sopparms.getWNMeshPrimGroup());

    if (sopparms.getOutWN() || sopparms.getOutGroup() || sopparms.getOutNumericAttrib())
        pointInMeshWN.gfeWN.findOrCreateTuple(!sopparms.getOutWN(), GA_ATTRIB_POINT, wnStorage, sopparms.getWNAttribName());

    pointInMeshWN.gfeWN.setComputeParm(sopWNType(sopparms.getWNType()),
        sopparms.getWNFullAccuracy(), sopparms.getWNAccuracyScale(),
        sopparms.getWNAsSolidAngle(), sopparms.getWNNegate());

    
    pointInMeshWN.groupSetter.setParm(groupMergeType, sopparms.getReverseGroup());
    //groupMergeType
    
    if (sopparms.getOutGroup())
        pointInMeshWN.findOrCreatePointGroup(false, sopparms.getGroupName());
    
    pointInMeshWN.setComputeParm(
        sopparms.getInGeo(),
        sopparms.getOnGeo(),
        sopparms.getThreshold(),
        sopparms.getSubscribeRatio(), sopparms.getMinGrainSize());
    
    pointInMeshWN.computeAndBumpDataId();
    pointInMeshWN.delOrVisualizeGroup();

}


