
//#define UT_ASSERT_LEVEL 3
#include "SOP_FeE_GroupCurveEnds_2_0.h"

#include "SOP_FeE_GroupCurveEnds_2_0.proto.h"


#include "GA/GA_Detail.h"
#include "PRM/PRM_TemplateBuilder.h"
#include "UT/UT_Interrupt.h"
#include "UT/UT_DSOVersion.h"


#include "GFE/GFE_GroupCurveEnds.h"

using namespace SOP_FeE_GroupCurveEnds_2_0_Namespace;


static const char *theDsFile = R"THEDSFILE(
{
    name        parameters
    parm {
        name    "group"
        cppname "Group"
        label   "Group"
        type    string
        default { "" }
        range   { 0 1 }
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
        name    "outEndsGroup"
        cppname "OutEndsGroup"
        label   "OutEndsGroup"
        type    toggle
        nolabel
        joinnext
        default { "0" }
    }
    parm {
        name    "endsGroupName"
        cppname "EndsGroupName"
        label   "Ends Group Name"
        type    string
        default { "ends" }
        disablewhen "{ outEndsGroup == 0 }"
    }
    parm {
        name    "outEndsInt"
        cppname "OutEndsInt"
        label   "OutEndsInt"
        type    toggle
        nolabel
        joinnext
        default { "0" }
    }
    parm {
        name    "endsIntName"
        cppname "EndsIntName"
        label   "Ends Integer Name"
        type    string
        default { "ends" }
        disablewhen "{ outEndsInt == 0 }"
    }
    parm {
        name    "outStartGroup"
        cppname "OutStartGroup"
        label   "OutStartGroup"
        type    toggle
        nolabel
        joinnext
        default { "0" }
    }
    parm {
        name    "startGroupName"
        cppname "StartGroupName"
        label   "Start Group Name"
        type    string
        default { "start" }
        disablewhen "{ outStartGroup == 0 }"
    }
    parm {
        name    "outStartInt"
        cppname "OutStartInt"
        label   "OutStartInt"
        type    toggle
        nolabel
        joinnext
        default { "0" }
    }
    parm {
        name    "startIntName"
        cppname "StartIntName"
        label   "Start Integer Name"
        type    string
        default { "start" }
        disablewhen "{ outStartInt == 0 }"
    }
    parm {
        name    "outEndGroup"
        cppname "OutEndGroup"
        label   "OutEndGroup"
        type    toggle
        nolabel
        joinnext
        default { "0" }
    }
    parm {
        name    "endGroupName"
        cppname "EndGroupName"
        label   "End Group Name"
        type    string
        default { "end" }
        disablewhen "{ outEndGroup == 0 }"
    }
    parm {
        name    "outEndInt"
        cppname "OutEndInt"
        label   "OutEndInt"
        type    toggle
        nolabel
        joinnext
        default { "0" }
    }
    parm {
        name    "endIntName"
        cppname "EndIntName"
        label   "End Integer Name"
        type    string
        default { "end" }
        disablewhen "{ outEndInt == 0 }"
    }
    parm {
        name    "includeUnused"
        cppname "IncludeUnused"
        label   "Include Unused"
        type    toggle
        default { "0" }
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
        name    "visualize"
        cppname "Visualize"
        label   "Visualize"
        type    ordinal
        default { "ends" }
        menu {
            "ends"  "Ends"
            "start" "Start"
            "end"   "End"
        }
    }
    parm {
        name    "numEnds"
        cppname "NumEnds"
        label   "NumEnds"
        type    integer
        default { "1" }
        range   { 1! 10 }
    }
    parm {
        name    "reverseGroup"
        cppname "ReverseGroup"
        label   "Reverse Group"
        type    toggle
        default { "0" }
    }

    parm {
        name    "delElement"
        cppname "DelElement"
        label   "Del Element"
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
    }
    parm {
        name    "minGrainSize"
        cppname "MinGrainSize"
        label   "Min Grain Size"
        type    intlog
        default { 1024 }
        range   { 0! 2048 }
    }

}
)THEDSFILE";

PRM_Template*
SOP_FeE_GroupCurveEnds_2_0::buildTemplates()
{
    static PRM_TemplateBuilder templ("SOP_FeE_GroupCurveEnds_2_0.C"_sh, theDsFile);
    if (templ.justBuilt())
    {
        templ.setChoiceListPtr("group"_sh, &SOP_Node::allGroupMenu);
    }
    return templ.templates();
}


const UT_StringHolder SOP_FeE_GroupCurveEnds_2_0::theSOPTypeName("FeE::groupCurveEnds::2.0"_sh);

void
newSopOperator(OP_OperatorTable* table)
{
    OP_Operator* newOp = new OP_Operator(
        SOP_FeE_GroupCurveEnds_2_0::theSOPTypeName,
        "FeE Group Curve Ends",
        SOP_FeE_GroupCurveEnds_2_0::myConstructor,
        SOP_FeE_GroupCurveEnds_2_0::buildTemplates(),
        1,
        1,
        nullptr,
        OP_FLAG_GENERATOR,
        nullptr,
        1,
        "Five elements Elf/Data/Topology");

    newOp->setIconName("SOP_ends");
    table->addOperator(newOp);

}

//class SOP_FeE_GroupCurveEnds_2_0Cache : public SOP_NodeCache
//{
//public:
//    SOP_FeE_GroupCurveEnds_2_0Cache() : SOP_NodeCache(),
//        myPrevOutputDetailID(-1)
//    {}
//    ~SOP_FeE_GroupCurveEnds_2_0Cache() override {}
//
//    int64 myPrevOutputDetailID;
//};


class SOP_FeE_GroupCurveEnds_2_0Verb : public SOP_NodeVerb
{
public:
    SOP_FeE_GroupCurveEnds_2_0Verb() {}
    virtual ~SOP_FeE_GroupCurveEnds_2_0Verb() {}

    virtual SOP_NodeParms *allocParms() const { return new SOP_FeE_GroupCurveEnds_2_0Parms(); }
    //virtual SOP_NodeCache* allocCache() const { return new SOP_FeE_GroupCurveEnds_2_0Cache(); }
    virtual UT_StringHolder name() const { return SOP_FeE_GroupCurveEnds_2_0::theSOPTypeName; }

    virtual CookMode cookMode(const SOP_NodeParms *parms) const { return COOK_INPLACE; }

    virtual void cook(const CookParms &cookparms) const;

    /// This static data member automatically registers
    /// this verb class at library ldir0d time.
    static const SOP_NodeVerb::Register<SOP_FeE_GroupCurveEnds_2_0Verb> theVerb;
};

// The static member variable definition has to be outside the class definition.
// The declaration is inside the class.
const SOP_NodeVerb::Register<SOP_FeE_GroupCurveEnds_2_0Verb> SOP_FeE_GroupCurveEnds_2_0Verb::theVerb;

const SOP_NodeVerb *
SOP_FeE_GroupCurveEnds_2_0::cookVerb() const 
{ 
    return SOP_FeE_GroupCurveEnds_2_0Verb::theVerb.get();
}








static GFE_CurveEndsType
sopVisualize(const SOP_FeE_GroupCurveEnds_2_0Parms::Visualize parmVisualize)
{
    using namespace SOP_FeE_GroupCurveEnds_2_0Enums;
    switch (parmVisualize)
    {
    case Visualize::ENDS:    return GFE_CurveEndsType::Ends;    break;
    case Visualize::START:   return GFE_CurveEndsType::Start;   break;
    case Visualize::END:     return GFE_CurveEndsType::End;     break;
    }
    UT_ASSERT_MSG(0, "Unhandled Visualize type!");
    return GFE_CurveEndsType::Ends;
}



static GA_GroupType
sopGroupType(const SOP_FeE_GroupCurveEnds_2_0Parms::GroupType parmGroupType)
{
    using namespace SOP_FeE_GroupCurveEnds_2_0Enums;
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



static GFE_GroupMergeType
sopGroupMergeType(const SOP_FeE_GroupCurveEnds_2_0Parms::GroupMergeType groupMergeType)
{
    using namespace SOP_FeE_GroupCurveEnds_2_0Enums;
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
SOP_FeE_GroupCurveEnds_2_0Verb::cook(const SOP_NodeVerb::CookParms &cookparms) const
{
    auto &&sopparms = cookparms.parms<SOP_FeE_GroupCurveEnds_2_0Parms>();
    GA_Detail& outGeo0 = *cookparms.gdh().gdpNC();
    //auto sopcache = (SOP_FeE_GroupCurveEnds_2_0Cache*)cookparms.cache();

    //const GA_Detail& inGeo0 = *cookparms.inputGeo(0);

    //outGeo0.replaceWith(inGeo0);

    
    const GA_GroupType groupType = sopGroupType(sopparms.getGroupType());
    const GFE_GroupMergeType groupMergeType = sopGroupMergeType(sopparms.getGroupMergeType());
    const GFE_CurveEndsType visualizeType = sopVisualize(sopparms.getVisualize());

    UT_AutoInterrupt boss("Processing");
    if (boss.wasInterrupted())
        return;


    
    GFE_GroupCurveEnds groupCurveEnds(outGeo0, cookparms);
    groupCurveEnds.setComputeParm(sopparms.getNumEnds(),
        sopparms.getSubscribeRatio(), sopparms.getMinGrainSize());
    groupCurveEnds.doDelGroupElement = sopparms.getDelElement();
    
    groupCurveEnds.groupSetter.setParm(groupMergeType, sopparms.getReverseGroup());


    
    groupCurveEnds.setStartGroup(false, sopparms.getStartGroupName());
    groupCurveEnds.setEndGroup  (false, sopparms.getEndGroupName());
    groupCurveEnds.setEndsGroup (false, sopparms.getEndsGroupName());
    
    groupCurveEnds.groupParser.setGroup(groupType, sopparms.getGroup());
    groupCurveEnds.computeAndBumpDataId();
    groupCurveEnds.visualizeOutGroup(visualizeType);
    
    

}


