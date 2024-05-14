
#include "SOP_FeE_WindingNumber_1_0.h"

#include "SOP_FeE_WindingNumber_1_0.proto.h"

#include "GA/GA_Detail.h"
#include "PRM/PRM_TemplateBuilder.h"
#include "UT/UT_Interrupt.h"
#include "UT/UT_DSOVersion.h"

#include "GFE/GFE_WindingNumber.h"




using namespace SOP_FeE_WindingNumber_1_0_Namespace;


class SOP_FeE_WindingNumber_1_0Verb : public SOP_NodeVerb
{
public:
    virtual SOP_NodeParms *allocParms() const { return new SOP_FeE_WindingNumber_1_0Parms(); }
    virtual SOP_NodeCache *allocCache() const { return new GFE_WindingNumber_Cache(); }
    virtual UT_StringHolder name() const { return theSOPTypeName; }

    /// This SOP wouldn't get any benefit from the results of the previous cook,
    /// (except for what's stored in SOP_FeE_WindingNumber_1_0Cache), and it would
    /// always duplicate its input at the start of the cook anyway, so it might
    /// as well use COOK_INPLACE, (which always either steals the first input's
    /// detail for the output detail or duplicates it into the output detail),
    /// instead of COOK_GENERIC, (which would let us have an output detail
    /// that's separate from the input detail and might be the same output
    /// detail as on a previous cook).
    virtual CookMode cookMode(const SOP_NodeParms *parms) const { return COOK_INPLACE; }

    virtual void cook(const CookParms &cookparms) const;

    /// This is the internal name of the SOP type.
    /// It isn't allowed to be the same as any other SOP's type name.
    static const UT_StringHolder theSOPTypeName;

    /// This static data member automatically registers
    /// this verb class at library load time.
    static const SOP_NodeVerb::Register<SOP_FeE_WindingNumber_1_0Verb> theVerb;

    /// This is the parameter interface string, below.
    static const char *const theDsFile;
};

// The static member variable definitions have to be outside the class definition.
// The declarations are inside the class.
const UT_StringHolder SOP_FeE_WindingNumber_1_0Verb::theSOPTypeName("FeE::windingNumber::1.0"_sh);
const SOP_NodeVerb::Register<SOP_FeE_WindingNumber_1_0Verb> SOP_FeE_WindingNumber_1_0Verb::theVerb;


static const char* theDsFile = R"THEDSFILE(
{
    name        parameters
    parm {
        name    "group"
        cppname "Group"
        label   "Group"
        type    string
        default { "" }
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
        name    "groupRef"
        cppname "GroupRef"
        label   "GroupRef"
        type    string
        default { "" }
        parmtag { "script_action" "import soputils\nkwargs['geometrytype'] = kwargs['node'].parmTuple('groupTypeRef')\nkwargs['inputindex'] = 0\nsoputils.selectGroupParm(kwargs)" }
        parmtag { "script_action_help" "Select geometry from an available viewport." }
        parmtag { "script_action_icon" "BUTTONS_reselect" }
    }
    parm {
        name    "groupTypeRef"
        cppname "GroupTypeRef"
        label   "Group Type Ref"
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
        name    "wnType"
        cppname "WNType"
        label   "Winding Number Type"
        type    ordinal
        default { "0" }
        menu {
            "xyz"   "3D"
            "xy"    "2D in XY Plane"
            "yz"    "2D in YZ Plane"
            "zx"    "2D in ZX Plane"
        }
    }
    parm {
        name    "outPrimWNAttrib"
        cppname "OutPrimWNAttrib"
        label   "Output Prim Winding Number"
        type    toggle
        nolabel
        joinnext
        default { "1" }
    }
    parm {
        name    "primWNAttribName"
        cppname "PrimWNAttribName"
        label   "Prim WN Attrib Name"
        type    string
        default { "windingNumber" }
        disablewhen "{ outPrimWNAttrib == 0 }"
    }
    parm {
        name    "outPointWNAttrib"
        cppname "OutPointWNAttrib"
        label   "Output Point Winding Number"
        type    toggle
        nolabel
        joinnext
        default { "0" }
    }
    parm {
        name    "pointWNAttribName"
        cppname "PointWNAttribName"
        label   "Point WN Attrib Name"
        type    string
        default { "windingNumber" }
        disablewhen "{ outPointWNAttrib == 0 }"
    }
    parm {
        name    "outVertexEdgeWNAttrib"
        cppname "OutVertexEdgeWNAttrib"
        label   "Output Vertex Edge Winding Number"
        type    toggle
        nolabel
        joinnext
        default { "0" }
    }
    parm {
        name    "vertexEdgeWNAttribName"
        cppname "VertexEdgeWNAttribName"
        label   "Vertex Edge WN Attrib Name"
        type    string
        default { "windingNumber" }
        disablewhen "{ outVertexEdgeWNAttrib == 0 }"
    }
    parm {
        name    "outPrimInMeshGroup"
        cppname "OutPrimInMeshGroup"
        label   "Output Prim Group"
        type    toggle
        nolabel
        joinnext
        default { "off" }
    }
    parm {
        name    "primInMeshGroupName"
        cppname "PrimInMeshGroupName"
        label   "Prim Group Name"
        type    string
        default { "inMesh" }
        disablewhen "{ outPrimInMeshGroup == 0 }"
    }
    parm {
        name    "outPointInMeshGroup"
        cppname "OutPointInMeshGroup"
        label   "Output Point Group"
        type    toggle
        nolabel
        joinnext
        default { "off" }
    }
    parm {
        name    "pointInMeshGroupName"
        cppname "PointInMeshGroupName"
        label   "Point Group Name"
        type    string
        default { "inMesh" }
        disablewhen "{ outPointInMeshGroup == 0 }"
    }
    parm {
        name    "outVertexEdgeInMeshGroup"
        cppname "OutVertexEdgeInMeshGroup"
        label   "Output Vertex Edge Group"
        type    toggle
        nolabel
        joinnext
        default { "off" }
    }
    parm {
        name    "vertexEdgeInMeshGroupName"
        cppname "VertexEdgeInMeshGroupName"
        label   "Vertex Edge Group Name"
        type    string
        default { "inMesh" }
        disablewhen "{ outVertexEdgeInMeshGroup == 0 }"
    }
    parm {
        name    "outEdgeInMeshGroup"
        cppname "OutEdgeInMeshGroup"
        label   "Output Edge Group"
        type    toggle
        nolabel
        joinnext
        default { "off" }
    }
    parm {
        name    "edgeInMeshGroupName"
        cppname "EdgeInMeshGroupName"
        label   "Edge Group Name"
        type    string
        default { "inMesh" }
        disablewhen "{ outEdgeInMeshGroup == 0 }"
    }
    parm {
        name    "wnAsSolidAngle"
        cppname "WNAsSolidAngle"
        label   "Scale to Solid Angle"
        type    toggle
        default { "0" }
        joinnext
    }
    parm {
        name    "wnNegate"
        cppname "WNNegate"
        label   "Negate Value (Reverse)"
        type    toggle
        default { "0" }
    }
    parm {
        name    "wnFullAccuracy"
        cppname "WNFullAccuracy"
        label   "Full Accuracy (Slow)"
        type    toggle
        default { "0" }
    }
    parm {
        name    "wnAccuracyScale"
        cppname "WNAccuracyScale"
        label   "Accuracy Scale"
        type    float
        default { "2" }
        range   { 1! 20 }
        disablewhen "{ wnFullAccuracy == 1 }"
    }
    parm {
        name    "wnPrecision"
        cppname "WNPrecision"
        label   "Precision"
        type    ordinal
        default { "auto" }
        menu {
            "auto"   "Auto"
            "_16"    "16"
            "_32"    "32"
            "_64"    "64"
        }
    }

    groupsimple {
        name    "inMeshGroup_folder"
        label   "In Mesh Group"
        grouptag { "group_type" "simple" }
        disablewhen "{ outPrimInMeshGroup == 0  outVertexEdgeInMeshGroup == 0  outPointInMeshGroup == 0  outEdgeInMeshGroup == 0 }"

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
            name    "groupInMesh"
            cppname "GroupInMesh"
            label   "In Geo"
            type    toggle
            default { "on" }
        }
        parm {
            name    "groupOnMesh"
            cppname "GroupOnMesh"
            label   "On Geo"
            type    toggle
            default { "on" }
        }
        parm {
            name    "pointInMeshThreshold"
            cppname "PointInMeshThreshold"
            label   "Point in Mesh Threshold"
            type    log
            default { "1e-05" }
            range   { 1e-05 1 }
        }
        parm {
            name    "reverseGroup"
            cppname "ReverseGroup"
            label   "Reverse Group"
            type    toggle
            default { "0" }
        }
        parm {
            name    "delGroupElem"
            cppname "DelGroupElem"
            label   "Delete Group Element"
            type    toggle
            default { "0" }
        }
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
SOP_FeE_WindingNumber_1_0::buildTemplates()
{
    static PRM_TemplateBuilder templ("SOP_FeE_WindingNumber_1_0.C"_sh, theDsFile);
    if (templ.justBuilt())
    {
        templ.setChoiceListPtr("group"_sh,    &SOP_Node::groupMenu);
        templ.setChoiceListPtr("groupRef"_sh, &SOP_Node::groupMenu);
    }
    return templ.templates();
}

const SOP_NodeVerb *SOP_FeE_WindingNumber_1_0::cookVerb() const
{
    return SOP_FeE_WindingNumber_1_0Verb::theVerb.get();
}

/// newSopOperator is the hook that Houdini grabs from this dll
/// and invokes to register the SOP.  In this case, we add ourselves
/// to the specified operator table.
void
newSopOperator(OP_OperatorTable *table)
{
    OP_Operator* newOp = new OP_Operator(
        SOP_FeE_WindingNumber_1_0Verb::theSOPTypeName,
        "FeE Winding Number",
        SOP_FeE_WindingNumber_1_0::myConstructor,
        SOP_FeE_WindingNumber_1_0::buildTemplates(),
        2,
        2,
        nullptr,
        0,  // No flags: not a generator, no merge input, not an output
        nullptr,
        1,
        "Five elements Elf/Data");

    newOp->setIconName("SOP_windingnumber");
    table->addOperator(newOp);
}




static GA_Storage
sopWNStorage(const SOP_FeE_WindingNumber_1_0Parms::WNPrecision wnPrecision)
{
    using namespace SOP_FeE_WindingNumber_1_0Enums;
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
sopWNType(const SOP_FeE_WindingNumber_1_0Parms::WNType wnType)
{
    using namespace SOP_FeE_WindingNumber_1_0Enums;
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


//static GA_AttributeOwner
//sopAttribOwner(const SOP_FeE_WindingNumber_1_0Parms::WNAttribClass parmAttribClass)
//{
//    using namespace SOP_FeE_WindingNumber_1_0Enums;
//    switch (parmAttribClass)
//    {
//    case WNAttribClass::PRIM:      return GA_ATTRIB_PRIMITIVE;  break;
//    case WNAttribClass::POINT:     return GA_ATTRIB_POINT;      break;
//    case WNAttribClass::VERTEX:    return GA_ATTRIB_VERTEX;     break;
//    }
//    UT_ASSERT_MSG(0, "Unhandled Geo0 Class type!");
//    return GA_ATTRIB_INVALID;
//}

//static GA_GroupType
//sopGroupType(const SOP_FeE_WindingNumber_1_0Parms::PointInMeshGroupType parmGroupType)
//{
//    using namespace SOP_FeE_WindingNumber_1_0Enums;
//    switch (parmGroupType)
//    {
//    case PointInMeshGroupType::PRIM:      return GA_GROUP_PRIMITIVE;  break;
//    case PointInMeshGroupType::POINT:     return GA_GROUP_POINT;      break;
//    case PointInMeshGroupType::VERTEX:    return GA_GROUP_VERTEX;     break;
//    case PointInMeshGroupType::EDGE:      return GA_GROUP_EDGE;       break;
//    }
//    UT_ASSERT_MSG(0, "Unhandled Point in Mesh Group Type!");
//    return GA_GROUP_INVALID;
//}


static GA_GroupType
sopGroupType(const SOP_FeE_WindingNumber_1_0Parms::GroupType parmGroupType)
{
    using namespace SOP_FeE_WindingNumber_1_0Enums;
    switch (parmGroupType)
    {
    case GroupType::GUESS:     return GA_GROUP_INVALID;    break;
    case GroupType::PRIM:      return GA_GROUP_PRIMITIVE;  break;
    case GroupType::POINT:     return GA_GROUP_POINT;      break;
    case GroupType::VERTEX:    return GA_GROUP_VERTEX;     break;
    case GroupType::EDGE:      return GA_GROUP_EDGE;       break;
    }
    UT_ASSERT_MSG(0, "Unhandled geo0Group type!");
    return GA_GROUP_INVALID;
}



static GA_GroupType
sopGroupType(const SOP_FeE_WindingNumber_1_0Parms::GroupTypeRef parmGroupType)
{
    using namespace SOP_FeE_WindingNumber_1_0Enums;
    switch (parmGroupType)
    {
    case GroupTypeRef::GUESS:     return GA_GROUP_INVALID;    break;
    case GroupTypeRef::PRIM:      return GA_GROUP_PRIMITIVE;  break;
    case GroupTypeRef::POINT:     return GA_GROUP_POINT;      break;
    case GroupTypeRef::VERTEX:    return GA_GROUP_VERTEX;     break;
    case GroupTypeRef::EDGE:      return GA_GROUP_EDGE;       break;
    }
    UT_ASSERT_MSG(0, "Unhandled geo0Group type!");
    return GA_GROUP_INVALID;
}




static GFE_GroupMergeType
sopGroupMergeType(const SOP_FeE_WindingNumber_1_0Parms::GroupMergeType groupMergeType)
{
    using namespace SOP_FeE_WindingNumber_1_0Enums;
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


void SOP_FeE_WindingNumber_1_0Verb::cook(const SOP_NodeVerb::CookParms& cookparms) const
{
    auto &&sopparms = cookparms.parms<SOP_FeE_WindingNumber_1_0Parms>();
    auto sopcache = (GFE_WindingNumber_Cache*)cookparms.cache();

    GA_Detail& geoPoint = *cookparms.gdh().gdpNC();
    const GA_Detail& geoRefMesh = *cookparms.inputGeo(1);
    

    const GA_Storage wnStorage = sopWNStorage(sopparms.getWNPrecision());
    const GFE_GroupMergeType groupMergeType = sopGroupMergeType(sopparms.getGroupMergeType());
    
    const GA_GroupType groupType = sopGroupType(sopparms.getGroupType());
    const GA_GroupType groupTypeRef = sopGroupType(sopparms.getGroupTypeRef());
    //const GA_AttributeOwner attribOwner = sopAttribOwner(sopparms.getWNAttribClass());
            


    
    GFE_WindingNumber wn(geoPoint, geoRefMesh, sopcache, &cookparms);
    
    wn.groupParser.setGroup(groupType, sopparms.getGroup());
    wn.groupParserRef0.setGroup(groupTypeRef, sopparms.getGroupRef());
    
    //wn.setInGroup(sopparms.getWNQueryPointGroup(), sopparms.getWNMeshPrimGroup());

    wn.groupSetter.setParm(groupMergeType, sopparms.getReverseGroup());
    if (sopparms.getOutPointInMeshGroup())
        wn.findOrCreatePointGroup(false, sopparms.getPointInMeshGroupName());
    if (sopparms.getOutPrimInMeshGroup())
        wn.findOrCreatePrimitiveGroup(false, sopparms.getPrimInMeshGroupName());
    if (sopparms.getOutVertexEdgeInMeshGroup())
        wn.findOrCreateVertexGroup(false, sopparms.getVertexEdgeInMeshGroupName());
    if (sopparms.getOutEdgeInMeshGroup())
        wn.findOrCreateEdgeGroup(false, sopparms.getEdgeInMeshGroupName());
    
    if (sopparms.getOutPrimWNAttrib())
        wn.findOrCreateTuple(false, GA_ATTRIB_PRIMITIVE, wnStorage, sopparms.getPrimWNAttribName());
    if (sopparms.getOutPointWNAttrib())
        wn.findOrCreateTuple(false, GA_ATTRIB_POINT,     wnStorage, sopparms.getPointWNAttribName());
    if (sopparms.getOutVertexEdgeWNAttrib())
        wn.findOrCreateTuple(false, GA_ATTRIB_VERTEX,    wnStorage, sopparms.getVertexEdgeWNAttribName());

    //if (sopparms.getOutPointInMeshGroup())
    //{
    //    wn.setPointInMeshComputeParm(
    //        sopparms.getGroupInGeoPoint(),
    //        sopparms.getGroupOnGeoPoint(),
    //        sopparms.getPointInMeshThreshold());
    //    
    //    const GA_GroupType groupType = sopGroupType(sopparms.getPointInMeshGroupType());
    //        
    //    wn.findOrCreateGroup(false, groupType, sopparms.getPointInMeshGroupName());
    //}

    wn.setComputeParm(sopWNType(sopparms.getWNType()),
        sopparms.getWNFullAccuracy(), sopparms.getWNAccuracyScale(),
        sopparms.getWNAsSolidAngle(), sopparms.getWNNegate());
    wn.doDelGroupElement = sopparms.getDelGroupElem();
    
    wn.computeAndBumpDataId();
    wn.visualizeOutGroup();

}
