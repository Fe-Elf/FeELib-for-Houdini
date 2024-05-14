
//#define UT_ASSERT_LEVEL 3
#include "SOP_FeE_OffsetAttribToIndex_1_0.h"
#include "SOP_FeE_OffsetAttribToIndex_1_0.proto.h"

#include <GFE/GFE_OffsetAttributeToIndex.h>

using namespace SOP_FeE_OffsetAttribToIndex_1_0_Namespace;

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
        parmtag { "sop_input" "0" }
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
        name    "primAttribName"
        cppname "PrimAttribName"
        label   "Prim Attrib Name"
        type    string
        default { "" }
    }
    parm {
        name    "pointAttribName"
        cppname "PointAttribName"
        label   "Point Attrib Name"
        type    string
        default { "" }
    }
    parm {
        name    "vertexAttribName"
        cppname "VertexAttribName"
        label   "Vertex Attrib Name"
        type    string
        default { "" }
    }


    parm {
       name    "offsetToIndex"
       cppname "OffsetToIndex"
       label   "Offset to Index"
       type    toggle
       default { 1 }
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
SOP_FeE_OffsetAttribToIndex_1_0::buildTemplates()
{
    static PRM_TemplateBuilder templ("SOP_FeE_OffsetAttribToIndex_1_0.C"_sh, theDsFile);
    if (templ.justBuilt())
    {
        templ.setChoiceListPtr("primAttribName"_sh,   &SOP_Node::primAttribMenu);
        templ.setChoiceListPtr("pointAttribName"_sh,  &SOP_Node::pointAttribMenu);
        templ.setChoiceListPtr("vertexAttribName"_sh, &SOP_Node::vertexAttribMenu);
    }
    return templ.templates();
}

const UT_StringHolder SOP_FeE_OffsetAttribToIndex_1_0::theSOPTypeName("FeE::offsetAttribToIndex::1.0"_sh);

void
newSopOperator(OP_OperatorTable* table)
{
    OP_Operator* newOp = new OP_Operator(
        SOP_FeE_OffsetAttribToIndex_1_0::theSOPTypeName,
        "FeE Offset Attrib to Index",
        SOP_FeE_OffsetAttribToIndex_1_0::myConstructor,
        SOP_FeE_OffsetAttribToIndex_1_0::buildTemplates(),
        1,
        1,
        nullptr,
        OP_FLAG_GENERATOR,
        nullptr,
        1,
        "Five elements Elf/Data/Enumerate");

    newOp->setIconName("SOP_enumerate");
    table->addOperator(newOp);
}

//class SOP_FeE_OffsetAttribToIndex_1_0Cache : public SOP_NodeCache
//{
//public:
//    SOP_FeE_OffsetAttribToIndex_1_0Cache() : SOP_NodeCache(),
//        myPrevOutputDetailID(-1)
//    {}
//    ~SOP_FeE_OffsetAttribToIndex_1_0Cache() override {}
//
//    int64 myPrevOutputDetailID;
//};


class SOP_FeE_OffsetAttribToIndex_1_0Verb : public SOP_NodeVerb
{
public:
    SOP_FeE_OffsetAttribToIndex_1_0Verb() {}
    virtual ~SOP_FeE_OffsetAttribToIndex_1_0Verb() {}

    virtual SOP_NodeParms *allocParms() const { return new SOP_FeE_OffsetAttribToIndex_1_0Parms(); }
    //virtual SOP_NodeCache* allocCache() const { return new SOP_FeE_OffsetAttribToIndex_1_0Cache(); }
    virtual UT_StringHolder name() const { return SOP_FeE_OffsetAttribToIndex_1_0::theSOPTypeName; }

    virtual CookMode cookMode(const SOP_NodeParms *parms) const { return COOK_INPLACE; }

    virtual void cook(const CookParms &cookparms) const;

    /// This static data member automatically registers
    /// this verb class at library ldir0d time.
    static const SOP_NodeVerb::Register<SOP_FeE_OffsetAttribToIndex_1_0Verb> theVerb;
};

// The static member variable definition has to be outside the class definition.
// The declaration is inside the class.
const SOP_NodeVerb::Register<SOP_FeE_OffsetAttribToIndex_1_0Verb> SOP_FeE_OffsetAttribToIndex_1_0Verb::theVerb;

const SOP_NodeVerb *
SOP_FeE_OffsetAttribToIndex_1_0::cookVerb() const
{
    return SOP_FeE_OffsetAttribToIndex_1_0Verb::theVerb.get();
}





static GA_GroupType
sopGroupType(const SOP_FeE_OffsetAttribToIndex_1_0Parms::GroupType parmGroupType)
{
    using namespace SOP_FeE_OffsetAttribToIndex_1_0Enums;
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
SOP_FeE_OffsetAttribToIndex_1_0Verb::cook(const SOP_NodeVerb::CookParms& cookparms) const
{
    auto&& sopparms = cookparms.parms<SOP_FeE_OffsetAttribToIndex_1_0Parms>();
    GA_Detail& outGeo0 = *cookparms.gdh().gdpNC();
    //auto sopcache = (SOP_FeE_OffsetAttribToIndex_1_0Cache*)cookparms.cache();

    //const GA_Detail& inGeo0 = *cookparms.inputGeo(0);

    //outGeo0.replaceWith(inGeo0);
    
    const GA_GroupType groupType = sopGroupType(sopparms.getGroupType());
    
    gfe::OffsetAttribToIndex offsetAttribToIndex(outGeo0, cookparms);

    offsetAttribToIndex.groupParser.setGroup(groupType, sopparms.getGroup());
    
    offsetAttribToIndex.setComputeParm(sopparms.getOffsetToIndex(), sopparms.getSubscribeRatio(), sopparms.getMinGrainSize());
    offsetAttribToIndex.getOutAttribArray().appends(GA_ATTRIB_PRIMITIVE, sopparms.getPrimAttribName());
    offsetAttribToIndex.getOutAttribArray().appends(GA_ATTRIB_POINT, sopparms.getPointAttribName());
    offsetAttribToIndex.getOutAttribArray().appends(GA_ATTRIB_VERTEX, sopparms.getVertexAttribName());

    offsetAttribToIndex.computeAndBumpDataId();
    
}


