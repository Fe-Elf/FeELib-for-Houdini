
//#define UT_ASSERT_LEVEL 3
#include "SOP_FeE_EdgeGroupTransfer_3_0.h"


#include "SOP_FeE_EdgeGroupTransfer_3_0.proto.h"

#include "GA/GA_Detail.h"
#include "PRM/PRM_TemplateBuilder.h"
#include "UT/UT_Interrupt.h"
#include "UT/UT_DSOVersion.h"




#include "GFE/GFE_EdgeGroupTransfer.h"



using namespace SOP_FeE_EdgeGroupTransfer_3_0_Namespace;


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
        name    "useSnapDist"
        cppname "UseSnapDist"
        label   "Use Snap Distance"
        type    toggle
        nolabel
        joinnext
        default { "on" }
    }
    parm {
        name    "snapDist"
        cppname "SnapDist"
        label   "Snap Distance"
        type    log
        default { "0.001" }
        disablewhen "{ useSnapDist == 0 }"
        range   { 0.001 10 }
    }


    parm {
        name    "vertexEdgeGroup"
        cppname "VertexEdgeGroup"
        label   "Vertex Edge Group"
        type    string
        default { "" }
        parmtag { "script_action" "import soputils\nkwargs['geometrytype'] = (hou.geometryType.Vertices,)\nkwargs['inputindex'] = 1\nsoputils.selectGroupParm(kwargs)" }
        parmtag { "script_action_help" "Select geometry from an available viewport." }
        parmtag { "script_action_icon" "BUTTONS_reselect" }
    }
    parm {
        name    "renameVertexEdgeGroup"
        cppname "RenameVertexEdgeGroup"
        label   "Rename Vertex Edge Group"
        type    toggle
        nolabel
        joinnext
        default { "off" }
    }
    parm {
        name    "newVertexEdgeGroupName"
        cppname "NewVertexEdgeGroupName"
        label   "New Vertex Edge Group Name"
        type    string
        default { "" }
        disablewhen "{ renameVertexEdgeGroup == 0 }"
    }
    parm {
        name    "edgeGroup"
        cppname "EdgeGroup"
        label   "Edge Group"
        type    string
        default { "" }
        parmtag { "script_action" "import soputils\nkwargs['geometrytype'] = (hou.geometryType.Edges,)\nkwargs['inputindex'] = 1\nsoputils.selectGroupParm(kwargs)" }
        parmtag { "script_action_help" "Select geometry from an available viewport." }
        parmtag { "script_action_icon" "BUTTONS_reselect" }
    }
    parm {
        name    "renameEdgeGroup"
        cppname "RenameEdgeGroup"
        label   "Rename Edge Group"
        type    toggle
        nolabel
        joinnext
        default { "off" }
    }
    parm {
        name    "newEdgeGroupName"
        cppname "NewEdgeGroupName"
        label   "New Edge Group Name"
        type    string
        default { "" }
        disablewhen "{ renameEdgeGroup == 0 }"
    }
    parm {
        name    "outAsVertexGroup"
        cppname "OutAsVertexGroup"
        label   "Output as Vertex Group"
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
        name    "reverseGroup"
        cppname "ReverseGroup"
        label   "Reverse Group"
        type    toggle
        default { "off" }
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
SOP_FeE_EdgeGroupTransfer_3_0::buildTemplates()
{
    static PRM_TemplateBuilder templ("SOP_FeE_EdgeGroupTransfer_3_0.C"_sh, theDsFile);
    if (templ.justBuilt())
    {
        templ.setChoiceListPtr("group"_sh,           &SOP_Node::groupMenu);
        templ.setChoiceListPtr("edgeGroup"_sh,       &SOP_Node::edgeNamedGroupMenu);
        templ.setChoiceListPtr("vertexEdgeGroup"_sh, &SOP_Node::vertexNamedGroupMenu);
    }
    return templ.templates();
}

const UT_StringHolder SOP_FeE_EdgeGroupTransfer_3_0::theSOPTypeName("FeE::edgeGroupTransfer::3.0"_sh);

void
newSopOperator(OP_OperatorTable* table)
{
    OP_Operator* newOp = new OP_Operator(
        SOP_FeE_EdgeGroupTransfer_3_0::theSOPTypeName,
        "FeE Edge Group Transfer",
        SOP_FeE_EdgeGroupTransfer_3_0::myConstructor,
        SOP_FeE_EdgeGroupTransfer_3_0::buildTemplates(),
        2,
        2,
        nullptr,
        OP_FLAG_GENERATOR,
        nullptr,
        1,
        "Five elements Elf/Group");

    newOp->setIconName("SOP_grouptransfer");
    table->addOperator(newOp);

}





class SOP_FeE_EdgeGroupTransfer_3_0Verb : public SOP_NodeVerb
{
public:
    SOP_FeE_EdgeGroupTransfer_3_0Verb() {}
    virtual ~SOP_FeE_EdgeGroupTransfer_3_0Verb() {}

    virtual SOP_NodeParms *allocParms() const { return new SOP_FeE_EdgeGroupTransfer_3_0Parms(); }
    virtual UT_StringHolder name() const { return SOP_FeE_EdgeGroupTransfer_3_0::theSOPTypeName; }

    virtual CookMode cookMode(const SOP_NodeParms *parms) const { return COOK_INPLACE; }

    virtual void cook(const CookParms &cookparms) const;
    
    /// This static data member automatically registers
    /// this verb class at library load time.
    static const SOP_NodeVerb::Register<SOP_FeE_EdgeGroupTransfer_3_0Verb> theVerb;
};

// The static member variable definition has to be outside the class definition.
// The declaration is inside the class.
const SOP_NodeVerb::Register<SOP_FeE_EdgeGroupTransfer_3_0Verb> SOP_FeE_EdgeGroupTransfer_3_0Verb::theVerb;

const SOP_NodeVerb *
SOP_FeE_EdgeGroupTransfer_3_0::cookVerb() const 
{ 
    return SOP_FeE_EdgeGroupTransfer_3_0Verb::theVerb.get();
}




static GA_GroupType
sopGroupType(const SOP_FeE_EdgeGroupTransfer_3_0Parms::GroupType parmGroupType)
{
    using namespace SOP_FeE_EdgeGroupTransfer_3_0Enums;
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
sopGroupMergeType(const SOP_FeE_EdgeGroupTransfer_3_0Parms::GroupMergeType groupMergeType)
{
    using namespace SOP_FeE_EdgeGroupTransfer_3_0Enums;
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
SOP_FeE_EdgeGroupTransfer_3_0Verb::cook(const SOP_NodeVerb::CookParms &cookparms) const
{
    auto&& sopparms = cookparms.parms<SOP_FeE_EdgeGroupTransfer_3_0Parms>();
    GA_Detail& outGeo0 = *cookparms.gdh().gdpNC();
    //auto sopcache = (SOP_FeE_EdgeGroupTransfer_3_0Cache*)cookparms.cache();

    //const GA_Detail& inGeo0 = *cookparms.inputGeo(0);
    const GA_Detail& inGeo1 = *cookparms.inputGeo(1);

    //outGeo0.replaceWith(inGeo0);


    const GA_GroupType groupType = sopGroupType(sopparms.getGroupType());
    const GFE_GroupMergeType groupMergeType = sopGroupMergeType(sopparms.getGroupMergeType());


    UT_AutoInterrupt boss("Processing");
    if (boss.wasInterrupted())
        return;
    
    GFE_EdgeGroupTransfer edgeGroupTransfer(outGeo0, inGeo1, cookparms);
    
    if (sopparms.getRenameEdgeGroup())
        edgeGroupTransfer.newEdgeGroupNames = sopparms.getNewEdgeGroupName();
    
    if (sopparms.getRenameVertexEdgeGroup())
        edgeGroupTransfer.newVertexEdgeGroupNames = sopparms.getNewVertexEdgeGroupName();
    
    edgeGroupTransfer.setComputeParm(
        sopparms.getUseSnapDist(), sopparms.getSnapDist(),
        sopparms.getOutAsVertexGroup(), 
        sopparms.getSubscribeRatio(), sopparms.getMinGrainSize());

    edgeGroupTransfer.groupParser.setGroup(groupType, sopparms.getGroup());
    edgeGroupTransfer.getRef0GroupArray().appends(GA_GROUP_EDGE, sopparms.getEdgeGroup());
    edgeGroupTransfer.getRef0GroupArray().appends(GA_GROUP_VERTEX, sopparms.getVertexEdgeGroup());
    
    edgeGroupTransfer.groupSetter.setComputeParm(groupMergeType, sopparms.getReverseGroup());
    
    edgeGroupTransfer.computeAndBumpDataId();
    edgeGroupTransfer.visualizeOutGroup();


    
}

