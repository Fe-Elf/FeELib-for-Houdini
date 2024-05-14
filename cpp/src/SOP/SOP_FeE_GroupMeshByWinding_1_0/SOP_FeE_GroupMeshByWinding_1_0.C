
//#define UT_ASSERT_LEVEL 3
#include "SOP_FeE_GroupMeshByWinding_1_0.h"


#include "SOP_FeE_GroupMeshByWinding_1_0.proto.h"

#include "GA/GA_Detail.h"
#include "PRM/PRM_TemplateBuilder.h"
#include "UT/UT_Interrupt.h"
#include "UT/UT_DSOVersion.h"



#include "GFE/GFE_GroupMeshByWinding.h"



using namespace SOP_FeE_GroupMeshByWinding_1_0_Namespace;



static const char *theDsFile = R"THEDSFILE(
{
    name        parameters
    
    parm {
        name    "primGroup"
        cppname "PrimGroup"
        label   "Prim Group"
        type    string
        default { "" }
        parmtag { "script_action" "import soputils\nkwargs['geometrytype'] = (hou.geometryType.Primitives,)\nkwargs['inputindex'] = 0\nsoputils.selectGroupParm(kwargs)" }
        parmtag { "script_action_help" "Select geometry from an available viewport.\nShift-click to turn on Select Groups." }
        parmtag { "script_action_icon" "BUTTONS_reselect" }
        parmtag { "sop_input" "0" }
    }
    parm {
        name    "groupMeshByWindingMethod"
        cppname "GroupMeshByWindingMethod"
        label   "Method"
        type    ordinal
        default { "volume" }
        menu {
            "volume"      "Volume"
            "wn_convex"   "Winding Number Convex"
            "wn_bbox"     "Winding Number BBox"
        }
    }
    parm {
        name    "meshCap"
        cppname "MeshCap"
        label   "Mesh Cap"
        type    toggle
        default { "0" }
    }
    parm {
        name    "shrinkAmount"
        cppname "ShrinkAmount"
        label   "Shrink Amount"
        type    log
        default { "0.001" }
        disablewhen "{ groupMeshByWindingMethod != wn_convex }"
        range   { 0.001 1 }
    }


    parm {
        name    "outGroup"
        cppname "OutGroup"
        label   "Out Group"
        type    toggle
        joinnext
        default { "0" }
    }
    parm {
        name    "outGroupType"
        cppname "OutGroupType"
        label   "Out Group Type"
        type    ordinal
        nolabel
        joinnext
        default { "prim" }
        disablewhen "{ outGroup == 0 }"
        menu {
            "prim"      "Primitive"
            "point"     "Point"
            "vertex"    "Vertex"
            "edge"      "Edge"
        }
    }
    parm {
        name    "outGroupName"
        cppname "OutGroupName"
        label   "Out Group Name"
        type    string
        nolabel
        default { "meshWindingCorrect" }
        disablewhen "{ outGroup == 0 }"
    }
    parm {
        name    "outAttrib"
        cppname "OutAttrib"
        label   "Out Attrib"
        type    toggle
        joinnext
        default { "0" }
    }
    parm {
        name    "outAttribName"
        cppname "OutAttribName"
        label   "Out Attrib Name"
        type    string
        nolabel
        default { "meshWindingCorrect" }
        disablewhen "{ outAttrib == 0 }"
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
        name    "reverseGroup"
        cppname "ReverseGroup"
        label   "Reverse Group"
        type    toggle
        default { "0" }
    }
    parm {
        name    "reversePrim"
        cppname "ReversePrim"
        label   "Reverse Prim"
        type    toggle
        default { "0" }
    }

    parm {
        name    "subscribeRatio"
        cppname "SubscribeRatio"
        label   "Subscribe Ratio"
        type    integer
        default { 16 }
        range   { 0! 256 }
        invisible
    }
    parm {
        name    "minGrainSize"
        cppname "MinGrainSize"
        label   "Min Grain Size"
        type    intlog
        default { 1024 }
        range   { 0! 2048 }
        invisible
    }
}
)THEDSFILE";



PRM_Template*
SOP_FeE_GroupMeshByWinding_1_0::buildTemplates()
{
    static PRM_TemplateBuilder templ("SOP_FeE_GroupMeshByWinding_1_0.C"_sh, theDsFile);
    if (templ.justBuilt())
    {
        templ.setChoiceListPtr("primGroup"_sh, &SOP_Node::primGroupMenu);
    }
    return templ.templates();
}


const UT_StringHolder SOP_FeE_GroupMeshByWinding_1_0::theSOPTypeName("FeE::groupMeshByWinding::1.0"_sh);

void
newSopOperator(OP_OperatorTable* table)
{
    OP_Operator* newOp = new OP_Operator(
        SOP_FeE_GroupMeshByWinding_1_0::theSOPTypeName,
        "FeE Group Mesh by Winding",
        SOP_FeE_GroupMeshByWinding_1_0::myConstructor,
        SOP_FeE_GroupMeshByWinding_1_0::buildTemplates(),
        1,
        1,
        nullptr,
        OP_FLAG_GENERATOR,
        nullptr,
        1,
        "Five elements Elf/Data/Orient");

    newOp->setIconName("SOP_normal");
    table->addOperator(newOp);

}



//class SOP_FeE_GroupMeshByWinding_1_0Cache : public SOP_NodeCache
//{
//public:
//    SOP_FeE_GroupMeshByWinding_1_0Cache() : SOP_NodeCache(),
//        myPrevOutputDetailID(-1)
//    {}
//    ~SOP_FeE_GroupMeshByWinding_1_0Cache() override {}
//
//    int64 myPrevOutputDetailID;
//};


class SOP_FeE_GroupMeshByWinding_1_0Verb : public SOP_NodeVerb
{
public:
    SOP_FeE_GroupMeshByWinding_1_0Verb() {}
    virtual ~SOP_FeE_GroupMeshByWinding_1_0Verb() {}

    virtual SOP_NodeParms *allocParms() const { return new SOP_FeE_GroupMeshByWinding_1_0Parms(); }
    //virtual SOP_NodeCache* allocCache() const { return new SOP_FeE_GroupMeshByWinding_1_0Cache(); }
    virtual UT_StringHolder name() const { return SOP_FeE_GroupMeshByWinding_1_0::theSOPTypeName; }

    virtual CookMode cookMode(const SOP_NodeParms *parms) const { return COOK_INPLACE; }

    virtual void cook(const CookParms &cookparms) const;

    /// This static data member automatically registers
    /// this verb class at library ldir0d time.
    static const SOP_NodeVerb::Register<SOP_FeE_GroupMeshByWinding_1_0Verb> theVerb;
};

// The static member variable definition has to be outside the class definition.
// The declaration is inside the class.
const SOP_NodeVerb::Register<SOP_FeE_GroupMeshByWinding_1_0Verb> SOP_FeE_GroupMeshByWinding_1_0Verb::theVerb;

const SOP_NodeVerb *
SOP_FeE_GroupMeshByWinding_1_0::cookVerb() const 
{ 
    return SOP_FeE_GroupMeshByWinding_1_0Verb::theVerb.get();
}







static GA_GroupType
sopGroupType(const SOP_FeE_GroupMeshByWinding_1_0Parms::OutGroupType parmGroupType)
{
    using namespace SOP_FeE_GroupMeshByWinding_1_0Enums;
    switch (parmGroupType)
    {
    case OutGroupType::PRIM:      return GA_GROUP_PRIMITIVE;  break;
    case OutGroupType::POINT:     return GA_GROUP_POINT;      break;
    case OutGroupType::VERTEX:    return GA_GROUP_VERTEX;     break;
    case OutGroupType::EDGE:      return GA_GROUP_EDGE;       break;
    }
    UT_ASSERT_MSG(0, "Unhandled geo0Group type!");
    return GA_GROUP_INVALID;
}


static GFE_GroupMergeType
sopGroupMergeType(const SOP_FeE_GroupMeshByWinding_1_0Parms::GroupMergeType groupMergeType)
{
    using namespace SOP_FeE_GroupMeshByWinding_1_0Enums;
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



static GFE_GroupMeshByWindingMethod
sopMethod(const SOP_FeE_GroupMeshByWinding_1_0Parms::GroupMeshByWindingMethod parmGroupType)
{
    using namespace SOP_FeE_GroupMeshByWinding_1_0Enums;
    switch (parmGroupType)
    {
    case GroupMeshByWindingMethod::VOLUME:     return GFE_GroupMeshByWindingMethod::VOLUME;                break;
    case GroupMeshByWindingMethod::WN_CONVEX:  return GFE_GroupMeshByWindingMethod::WINDINGNUMBER_CONVEX;  break;
    case GroupMeshByWindingMethod::WN_BBOX:    return GFE_GroupMeshByWindingMethod::WINDINGNUMBER_BBOX;    break;
    }
    UT_ASSERT_MSG(0, "Unhandled GFE_groupMeshByWinding METHOD!");
    return GFE_GroupMeshByWindingMethod::VOLUME;
}



void
SOP_FeE_GroupMeshByWinding_1_0Verb::cook(const SOP_NodeVerb::CookParms& cookparms) const
{
    auto&& sopparms = cookparms.parms<SOP_FeE_GroupMeshByWinding_1_0Parms>();
    GA_Detail& outGeo0 = *cookparms.gdh().gdpNC();
    //auto sopcache = (SOP_FeE_GroupMeshByWinding_1_0Cache*)cookparms.cache();

    //const GA_Detail& inGeo0 = *cookparms.inputGeo(0);
    const GA_Detail* const inGeo1 = cookparms.inputGeo(1);
    
    //outGeo0.replaceWith(inGeo0);

    const GFE_GroupMergeType groupMergeType = sopGroupMergeType(sopparms.getGroupMergeType());
    const GFE_GroupMeshByWindingMethod method = sopMethod(sopparms.getGroupMeshByWindingMethod());


    UT_AutoInterrupt boss("Processing");
    if (boss.wasInterrupted())
        return;

    
    GFE_GroupMeshByWinding groupMeshByWinding(outGeo0, inGeo1, cookparms);
    groupMeshByWinding.setComputeParm(method, sopparms.getShrinkAmount(), sopparms.getMeshCap(), sopparms.getReversePrim());
    
    groupMeshByWinding.groupSetter.setParm(groupMergeType, sopparms.getReverseGroup());
    
    if (sopparms.getOutGroup())
    {
        const GA_GroupType outGroupType = sopGroupType(sopparms.getOutGroupType());
        groupMeshByWinding.getOutGroupArray().findOrCreate(false, outGroupType, sopparms.getOutGroupName());
    }
    
    if (sopparms.getOutAttrib())
    {
        groupMeshByWinding.findOrCreateTuple(false, sopparms.getOutAttribName());
    }
    
    groupMeshByWinding.groupParser.setPrimitiveGroup(sopparms.getPrimGroup());
    groupMeshByWinding.computeAndBumpDataId();
    groupMeshByWinding.visualizeOutGroup();

}
