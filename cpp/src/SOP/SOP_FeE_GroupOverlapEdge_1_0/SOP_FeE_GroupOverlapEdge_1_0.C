
//#define UT_ASSERT_LEVEL 3
#include "SOP_FeE_GroupOverlapEdge_1_0.h"


#include "SOP_FeE_GroupOverlapEdge_1_0.proto.h"

#include "GA/GA_Detail.h"
#include "PRM/PRM_TemplateBuilder.h"
#include "UT/UT_Interrupt.h"
#include "UT/UT_DSOVersion.h"


#include "GFE/GFE_GroupOverlapEdge.h"


using namespace SOP_FeE_GroupOverlapEdge_1_0_Namespace;



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
        name    "outEdgeGroup"
        cppname "OutEdgeGroup"
        label   "Out Edge Group"
        type    toggle
        default { "0" }
        joinnext
        nolabel
        disablewhen "{ delGroupElem == 1 }"
    }
    parm {
        name    "newEdgeGroupName"
        cppname "NewEdgeGroupName"
        label   "New Edge Group Name"
        type    string
        default { "newEdge" }
        parmtag { "script_action" "import soputils\nkwargs['geometrytype'] = (hou.geometryType.Edges,)\nkwargs['inputindex'] = 0\nsoputils.selectGroupParm(kwargs)" }
        parmtag { "script_action_help" "Select geometry from an available viewport." }
        parmtag { "script_action_icon" "BUTTONS_reselect" }
        disablewhen "{ outEdgeGroup == 0 } { delGroupElem == 1 }"
    }
    parm {
        name    "outVertexEdgeGroup"
        cppname "OutVertexEdgeGroup"
        label   "Out Vertex Edge Group"
        type    toggle
        default { "0" }
        joinnext
        nolabel
        disablewhen "{ delGroupElem == 1 }"
    }
    parm {
        name    "newVertexEdgeGroupName"
        cppname "NewVertexEdgeGroupName"
        label   "New Vertex Edge Group Name"
        type    string
        default { "newEdge" }
        parmtag { "script_action" "import soputils\nkwargs['geometrytype'] = (hou.geometryType.Vertices,)\nkwargs['inputindex'] = 0\nsoputils.selectGroupParm(kwargs)" }
        parmtag { "script_action_help" "Select geometry from an available viewport." }
        parmtag { "script_action_icon" "BUTTONS_reselect" }
        disablewhen "{ outVertexEdgeGroup == 0 } { delGroupElem == 1 }"
    }
    
    parm {
        name    "useSnapDist"
        cppname "UseSnapDist"
        label   "Use Snap Dist"
        type    toggle
        default { "1" }
    }
    parm {
        name    "snapDist"
        cppname "SnapDist"
        label   "Snap Dist"
        type    log
        default { "0.001" }
        range   { 0.00001 0.1 }
        disablewhen "{ useSnapDist == 0 }"
    }
    parm {
        name    "runOverGeoRef"
        cppname "RunOverGeoRef"
        label   "Run Over Geo Ref"
        type    toggle
        default { "0" }
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
        label   "Del Group Elem"
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
SOP_FeE_GroupOverlapEdge_1_0::buildTemplates()
{
    static PRM_TemplateBuilder templ("SOP_FeE_GroupOverlapEdge_1_0.C"_sh, theDsFile);
    if (templ.justBuilt())
    {
        templ.setChoiceListPtr("group"_sh, &SOP_Node::allGroupMenu);
        templ.setChoiceListPtr("newEdgeGroupName"_sh, &SOP_Node::edgeNamedGroupMenu);
        templ.setChoiceListPtr("newVertexEdgeGroupName"_sh, &SOP_Node::vertexNamedGroupMenu);
    }
    return templ.templates();
}


const UT_StringHolder SOP_FeE_GroupOverlapEdge_1_0::theSOPTypeName("FeE::groupNewEdge::1.0"_sh);

void
newSopOperator(OP_OperatorTable* table)
{
    OP_Operator* newOp = new OP_Operator(
        SOP_FeE_GroupOverlapEdge_1_0::theSOPTypeName,
        "FeE Group New Edge",
        SOP_FeE_GroupOverlapEdge_1_0::myConstructor,
        SOP_FeE_GroupOverlapEdge_1_0::buildTemplates(),
        2,
        2,
        nullptr,
        OP_FLAG_GENERATOR,
        nullptr,
        1,
        "Five elements Elf/Group/Bound");

    newOp->setIconName("SOP_groupcreate");
    table->addOperator(newOp);

}


//class SOP_FeE_GroupOverlapEdge_1_0Cache : public SOP_NodeCache
//{
//public:
//    SOP_FeE_GroupOverlapEdge_1_0Cache() : SOP_NodeCache(),
//        myPrevOutputDetailID(-1)
//    {}
//    ~SOP_FeE_GroupOverlapEdge_1_0Cache() override {}
//
//    int64 myPrevOutputDetailID;
//};


class SOP_FeE_GroupOverlapEdge_1_0Verb : public SOP_NodeVerb
{
public:
    SOP_FeE_GroupOverlapEdge_1_0Verb() {}
    virtual ~SOP_FeE_GroupOverlapEdge_1_0Verb() {}

    virtual SOP_NodeParms *allocParms() const { return new SOP_FeE_GroupOverlapEdge_1_0Parms(); }
    //virtual SOP_NodeCache* allocCache() const { return new SOP_FeE_GroupOverlapEdge_1_0Cache(); }
    virtual UT_StringHolder name() const { return SOP_FeE_GroupOverlapEdge_1_0::theSOPTypeName; }

    virtual CookMode cookMode(const SOP_NodeParms *parms) const { return COOK_GENERIC; }

    virtual void cook(const CookParms &cookparms) const;

    /// This static data member automatically registers
    /// this verb class at library ldir0d time.
    static const SOP_NodeVerb::Register<SOP_FeE_GroupOverlapEdge_1_0Verb> theVerb;
};

// The static member variable definition has to be outside the class definition.
// The declaration is inside the class.
const SOP_NodeVerb::Register<SOP_FeE_GroupOverlapEdge_1_0Verb> SOP_FeE_GroupOverlapEdge_1_0Verb::theVerb;

const SOP_NodeVerb *
SOP_FeE_GroupOverlapEdge_1_0::cookVerb() const 
{ 
    return SOP_FeE_GroupOverlapEdge_1_0Verb::theVerb.get();
}








static GA_GroupType
sopGroupType(SOP_FeE_GroupOverlapEdge_1_0Parms::GroupType parmGroupType)
{
    using namespace SOP_FeE_GroupOverlapEdge_1_0Enums;
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



void
SOP_FeE_GroupOverlapEdge_1_0Verb::cook(const SOP_NodeVerb::CookParms &cookparms) const
{
    auto &&sopparms = cookparms.parms<SOP_FeE_GroupOverlapEdge_1_0Parms>();
    GA_Detail& outGeo0 = *cookparms.gdh().gdpNC();
    //auto sopcache = (SOP_FeE_GroupOverlapEdge_1_0Cache*)cookparms.cache();

    const GA_Detail& inGeo0 = *cookparms.inputGeo(0);
    const GA_Detail& inGeo1 = *cookparms.inputGeo(1);

    outGeo0.replaceWith(inGeo0);

    const UT_StringHolder& newVertexEdgeGroupName = sopparms.getNewVertexEdgeGroupName();
    const UT_StringHolder& newEdgeGroupName = sopparms.getNewEdgeGroupName();
    
    if (!newVertexEdgeGroupName.isstring() &&
        newVertexEdgeGroupName.length() == 0 &&
        !newEdgeGroupName.isstring() &&
        newEdgeGroupName.length() == 0)
    {
        return;
    }

    const GA_GroupType groupType = sopGroupType(sopparms.getGroupType());


    UT_AutoInterrupt boss("Processing");
    if (boss.wasInterrupted())
        return;

    

    GFE_GroupOverlapEdge groupOverlapEdge(outGeo0, inGeo1, cookparms);
    groupOverlapEdge.groupSetter.setParm(sopparms.getReverseGroup());
    
    groupOverlapEdge.setComputeParm(sopparms.getUseSnapDist(),sopparms.getSnapDist(),sopparms.getRunOverGeoRef(),
        sopparms.getSubscribeRatio(), sopparms.getMinGrainSize());
    
    groupOverlapEdge.doDelGroupElement = sopparms.getDelGroupElem();

    if (sopparms.getDelGroupElem())
        groupOverlapEdge.getOutGroupArray().findOrCreateEdge(true);
    else
    {
        if (sopparms.getOutEdgeGroup())
            groupOverlapEdge.getOutGroupArray().findOrCreateEdge(false, sopparms.getNewEdgeGroupName());
        if (sopparms.getOutVertexEdgeGroup())
            groupOverlapEdge.getOutGroupArray().findOrCreateVertex(false, sopparms.getNewVertexEdgeGroupName());
    }


    groupOverlapEdge.groupParser.setGroup(groupType, sopparms.getGroup());
    groupOverlapEdge.computeAndBumpDataId();
    groupOverlapEdge.delOrVisualizeGroup();

}


