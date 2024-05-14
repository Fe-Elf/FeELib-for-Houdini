
//#define UT_ASSERT_LEVEL 3
#include "SOP_FeE_GroupCombine_1_0.h"


#include "SOP_FeE_GroupCombine_1_0.proto.h"

#include "GA/GA_Detail.h"
#include "PRM/PRM_TemplateBuilder.h"
#include "UT/UT_Interrupt.h"
#include "UT/UT_DSOVersion.h"


#include "GFE/GFE_GroupParser.h"
#include "GFE/GFE_GroupUnion.h"


using namespace SOP_FeE_GroupCombine_1_0_Namespace;



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
        name    "combineGroup"
        cppname "CombineGroup"
        label   "Combine Group"
        type    string
        default { "" }
        parmtag { "script_action" "import soputils\nkwargs['geometrytype'] = kwargs['node'].parmTuple('combineGroupType')\nkwargs['inputindex'] = 0\nsoputils.selectGroupParm(kwargs)" }
        parmtag { "script_action_help" "Select geometry from an available viewport." }
        parmtag { "script_action_icon" "BUTTONS_reselect" }
    }
    parm {
        name    "combineGroupType"
        cppname "CombineGroupType"
        label   "Combine Group Type"
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
        name    "vertexEdgeConnectElemType"
        cppname "VertexEdgeConnectElemType"
        label   "Vertex Edge Connect Elem Type"
        type    ordinal
        default { "point" }
        menu {
            "prim"      "Primitive"
            "point"     "Point"
            "oneVertex" "OneVertex"
            "allVertex" "AllVertex"
        }
        disablewhen "{ combineGroupType != guess combineGroupType != vertex } { groupType != guess groupType != edge }"
    }

    parm {
        name    "reverse"
        cppname "Reverse"
        label   "Reverse"
        type    toggle
        default { 0 }
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
SOP_FeE_GroupCombine_1_0::buildTemplates()
{
    static PRM_TemplateBuilder templ("SOP_FeE_GroupCombine_1_0.C"_sh, theDsFile);
    if (templ.justBuilt())
    {
        templ.setChoiceListPtr("group"_sh, &SOP_Node::allGroupMenu);
        templ.setChoiceListPtr("combineGroup"_sh, &SOP_Node::allGroupMenu);
    }
    return templ.templates();
}


const UT_StringHolder SOP_FeE_GroupCombine_1_0::theSOPTypeName("FeE::groupCombine::1.0"_sh);

void
newSopOperator(OP_OperatorTable* table)
{
    OP_Operator* newOp = new OP_Operator(
        SOP_FeE_GroupCombine_1_0::theSOPTypeName,
        "FeE Group Combine",
        SOP_FeE_GroupCombine_1_0::myConstructor,
        SOP_FeE_GroupCombine_1_0::buildTemplates(),
        1,
        1,
        nullptr,
        OP_FLAG_GENERATOR,
        nullptr,
        1,
        "Five elements Elf/Group");

    newOp->setIconName("SOP_groupcombine");
    table->addOperator(newOp);

}


//class SOP_FeE_GroupCombine_1_0Cache : public SOP_NodeCache
//{
//public:
//    SOP_FeE_GroupCombine_1_0Cache() : SOP_NodeCache(),
//        myPrevOutputDetailID(-1)
//    {}
//    ~SOP_FeE_GroupCombine_1_0Cache() override {}
//
//    int64 myPrevOutputDetailID;
//};


class SOP_FeE_GroupCombine_1_0Verb : public SOP_NodeVerb
{
public:
    SOP_FeE_GroupCombine_1_0Verb() {}
    virtual ~SOP_FeE_GroupCombine_1_0Verb() {}

    virtual SOP_NodeParms *allocParms() const { return new SOP_FeE_GroupCombine_1_0Parms(); }
    //virtual SOP_NodeCache* allocCache() const { return new SOP_FeE_GroupCombine_1_0Cache(); }
    virtual UT_StringHolder name() const { return SOP_FeE_GroupCombine_1_0::theSOPTypeName; }

    virtual CookMode cookMode(const SOP_NodeParms *parms) const { return COOK_INPLACE; }

    virtual void cook(const CookParms &cookparms) const;

    /// This static data member automatically registers
    /// this verb class at library ldir0d time.
    static const SOP_NodeVerb::Register<SOP_FeE_GroupCombine_1_0Verb> theVerb;
};

// The static member variable definition has to be outside the class definition.
// The declaration is inside the class.
const SOP_NodeVerb::Register<SOP_FeE_GroupCombine_1_0Verb> SOP_FeE_GroupCombine_1_0Verb::theVerb;

const SOP_NodeVerb *
SOP_FeE_GroupCombine_1_0::cookVerb() const 
{ 
    return SOP_FeE_GroupCombine_1_0Verb::theVerb.get();
}







static GA_GroupType
sopCombineGroupType(const SOP_FeE_GroupCombine_1_0Parms::CombineGroupType parmCombineGroupType)
{
    using namespace SOP_FeE_GroupCombine_1_0Enums;
    switch (parmCombineGroupType)
    {
    case CombineGroupType::GUESS:     return GA_GROUP_INVALID;    break;
    case CombineGroupType::PRIM:      return GA_GROUP_PRIMITIVE;  break;
    case CombineGroupType::POINT:     return GA_GROUP_POINT;      break;
    case CombineGroupType::VERTEX:    return GA_GROUP_VERTEX;     break;
    case CombineGroupType::EDGE:      return GA_GROUP_EDGE;       break;
    }
    UT_ASSERT_MSG(0, "Unhandled geo0Group type!");
    return GA_GROUP_INVALID;
}




static GA_GroupType
sopGroupType(const SOP_FeE_GroupCombine_1_0Parms::GroupType parmGroupType)
{
    using namespace SOP_FeE_GroupCombine_1_0Enums;
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


static GA_AttributeOwner
sopVertexEdgeConnectElemType(const SOP_FeE_GroupCombine_1_0Parms::VertexEdgeConnectElemType parmVertexEdgeConnectElemType)
{
    using namespace SOP_FeE_GroupCombine_1_0Enums;
    switch (parmVertexEdgeConnectElemType)
    {
    case VertexEdgeConnectElemType::PRIM:      return GA_ATTRIB_PRIMITIVE;  break;
    case VertexEdgeConnectElemType::POINT:     return GA_ATTRIB_POINT;      break;
    case VertexEdgeConnectElemType::ONEVERTEX: return GA_ATTRIB_VERTEX;     break;
    case VertexEdgeConnectElemType::ALLVERTEX: return GA_ATTRIB_OWNER_N;    break;
    }
    UT_ASSERT_MSG(0, "Unhandled Vertex Edge Connect Element Type!");
    return GA_ATTRIB_INVALID;
}


void
SOP_FeE_GroupCombine_1_0Verb::cook(const SOP_NodeVerb::CookParms &cookparms) const
{
    auto &&sopparms = cookparms.parms<SOP_FeE_GroupCombine_1_0Parms>();
    GA_Detail& outGeo0 = *cookparms.gdh().gdpNC();
    //auto sopcache = (SOP_FeE_GroupCombine_1_0Cache*)cookparms.cache();

    //const GA_Detail& inGeo0 = *cookparms.inputGeo(0);

    //outGeo0.replaceWith(inGeo0);


    const UT_StringHolder& combineGroupName = sopparms.getCombineGroup();
    if (!combineGroupName.isstring() || combineGroupName.length() == 0)
        return;
    
    const GA_GroupType combineGroupType = sopCombineGroupType(sopparms.getCombineGroupType());

    GA_GroupTable* groupTable = outGeo0.getGroupTable(combineGroupType);
    GA_Group* combineGroupPtr;
    if (groupTable)
    {
        combineGroupPtr = groupTable->find(combineGroupName);
        if (!combineGroupPtr)
            combineGroupPtr = groupTable->newGroup(combineGroupName);
    }
    else
    {
        combineGroupPtr = outGeo0.getGroupTable(GA_GROUP_PRIMITIVE)->find(combineGroupName);
        if (!combineGroupPtr)
        {
            combineGroupPtr = outGeo0.getGroupTable(GA_GROUP_POINT)->find(combineGroupName);
            if (!combineGroupPtr)
            {
                combineGroupPtr = outGeo0.getGroupTable(GA_GROUP_VERTEX)->find(combineGroupName);
                if (!combineGroupPtr)
                {
                    combineGroupPtr = outGeo0.getGroupTable(GA_GROUP_EDGE)->find(combineGroupName);
                    if (!combineGroupPtr)
                        return;
                }
            }
        }
    }
    GA_Group& combineGroup = *combineGroupPtr;
    
    const GA_GroupType groupType = sopGroupType(sopparms.getGroupType());
    
    GOP_Manager gop;
    GFE_GroupParser groupParser(outGeo0, gop, &cookparms);
    //const GA_Group* const geo0Group = GFE_GroupParser_Namespace::findOrParseGroupDetached(cookparms, &outGeo0, groupType, sopparms.getGroup(), gop);
    const GA_Group* const geo0Group = groupParser.setGroup(groupType, sopparms.getGroup());

    
    UT_AutoInterrupt boss("Processing");
    if (boss.wasInterrupted())
        return;

    if (combineGroup.classType() == GA_GROUP_VERTEX && geo0Group->classType() == GA_GROUP_EDGE)
    {
        const GA_AttributeOwner vertexEdgeConnectElemType = sopVertexEdgeConnectElemType(sopparms.getVertexEdgeConnectElemType());
        GFE_GroupUnion::groupUnion(static_cast<GA_VertexGroup&>(combineGroup), static_cast<const GA_EdgeGroup*>(geo0Group), vertexEdgeConnectElemType, sopparms.getReverse());
    }
    else
        GFE_GroupUnion::groupUnion(combineGroup, geo0Group, sopparms.getReverse(), sopparms.getSubscribeRatio(), sopparms.getMinGrainSize());

    GFE_Group::groupBumpDataId(combineGroup);

    cookparms.select(combineGroup);
    


}

