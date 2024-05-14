
//#define UT_ASSERT_LEVEL 3
#include "SOP_FeE_AttribExtremum_2_0.h"
#include "SOP_FeE_AttribExtremum_2_0.proto.h"


#include "GA/GA_Detail.h"
#include "PRM/PRM_TemplateBuilder.h"
#include "UT/UT_Interrupt.h"
#include "UT/UT_DSOVersion.h"


#include "GFE/GFE_AttributeExtremum.h"


using namespace SOP_FeE_AttribExtremum_2_0_Namespace;



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
        name    "attribClass"
        cppname "AttribClass"
        label   "Attribute Class"
        type    ordinal
        default { "point" }
        menu {
            "prim"      "Primitive"
            "point"     "Point"
            "vertex"    "Vertex"
            "detail"    "Detail"
        }
    }
    parm {
        name    "attrib"
        cppname "Attrib"
        label   "Attrib"
        type    string
        default { "" }
    }
    parm {
        name    "outAttribMin"
        cppname "OutAttribMin"
        label   "Out Attrib Min"
        type    toggle
        default { "1" }
    }
    parm {
        name    "attribMinName"
        cppname "AttribMinName"
        label   "Attrib Min Name"
        type    string
        default { "min" }
        disablewhen "{ outAttribMin == 0 }"
    }
    parm {
        name    "outAttribMax"
        cppname "OutAttribMax"
        label   "Out Attrib Max"
        type    toggle
        default { "1" }
    }
    parm {
        name    "attribMaxName"
        cppname "AttribMaxName"
        label   "Attrib Min Name"
        type    string
        default { "min" }
        disablewhen "{ outAttribMax == 0 }"
    }

    parm {
        name    "outAttribMinElemnum"
        cppname "OutAttribMinElemnum"
        label   "Out Attrib Min Elemnum"
        type    toggle
        default { "1" }
    }
    parm {
        name    "attribMinElemnumName"
        cppname "AttribMinElemnumName"
        label   "Attrib Min Elemnum Name"
        type    string
        default { "min" }
        disablewhen "{ outAttribMinElemnum == 0 }"
    }
    parm {
        name    "outAttribMaxElemnum"
        cppname "OutAttribMaxElemnum"
        label   "Out Attrib Max Elemnum"
        type    toggle
        default { "1" }
    }
    parm {
        name    "attribMaxElemnumName"
        cppname "AttribMaxElemnumName"
        label   "Attrib Max Elemnum Name"
        type    string
        default { "min" }
        disablewhen "{ outAttribMaxElemnum == 0 }"
    }

    parm {
        name    "outAsOffset"
        cppname "OutAsOffset"
        label   "Out as Offset"
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
        default { 1024 }
        range   { 0! 2048 }
    }
}
)THEDSFILE";

PRM_Template*
SOP_FeE_AttribExtremum_2_0::buildTemplates()
{
    static PRM_TemplateBuilder templ("SOP_FeE_AttribExtremum_2_0.C"_sh, theDsFile);
    if (templ.justBuilt())
    {
        templ.setChoiceListPtr("group"_sh, &SOP_Node::allGroupMenu);
        templ.setChoiceListPtr("attrib"_sh, &SOP_Node::allAttribReplaceMenu);
    }
    return templ.templates();
}

const UT_StringHolder SOP_FeE_AttribExtremum_2_0::theSOPTypeName("FeE::attribExtremum::2.0"_sh);

void
newSopOperator(OP_OperatorTable* table)
{
    OP_Operator* newOp = new OP_Operator(
        SOP_FeE_AttribExtremum_2_0::theSOPTypeName,
        "FeE Attribute Extremum",
        SOP_FeE_AttribExtremum_2_0::myConstructor,
        SOP_FeE_AttribExtremum_2_0::buildTemplates(),
        1,
        2,
        nullptr,
        OP_FLAG_GENERATOR,
        nullptr,
        1,
        "Five elements Elf/Attribute");

    newOp->setIconName("VOP_max");
    table->addOperator(newOp);
}

class SOP_FeE_AttribExtremum_2_0Verb : public SOP_NodeVerb
{
public:
    SOP_FeE_AttribExtremum_2_0Verb() {}
    virtual ~SOP_FeE_AttribExtremum_2_0Verb() {}

    virtual SOP_NodeParms* allocParms() const { return new SOP_FeE_AttribExtremum_2_0Parms(); }
    virtual UT_StringHolder name() const { return SOP_FeE_AttribExtremum_2_0::theSOPTypeName; }

    virtual CookMode cookMode(const SOP_NodeParms* parms) const { return COOK_INPLACE; }

    virtual void cook(const CookParms& cookparms) const;

    static const SOP_NodeVerb::Register<SOP_FeE_AttribExtremum_2_0Verb> theVerb;
};

const SOP_NodeVerb::Register<SOP_FeE_AttribExtremum_2_0Verb> SOP_FeE_AttribExtremum_2_0Verb::theVerb;

const SOP_NodeVerb*
SOP_FeE_AttribExtremum_2_0::cookVerb() const
{
    return SOP_FeE_AttribExtremum_2_0Verb::theVerb.get();
}




static GA_AttributeOwner
sopAttribOwner(const SOP_FeE_AttribExtremum_2_0Parms::AttribClass parmAttribClass)
{
    using namespace SOP_FeE_AttribExtremum_2_0Enums;
    switch (parmAttribClass)
    {
    case AttribClass::PRIM:      return GA_ATTRIB_PRIMITIVE;  break;
    case AttribClass::POINT:     return GA_ATTRIB_POINT;      break;
    case AttribClass::VERTEX:    return GA_ATTRIB_VERTEX;     break;
    case AttribClass::DETAIL:    return GA_ATTRIB_DETAIL;     break;
    }
    UT_ASSERT_MSG(0, "Unhandled Class type!");
    return GA_ATTRIB_INVALID;
}


static GA_GroupType
sopGroupType(const SOP_FeE_AttribExtremum_2_0Parms::GroupType parmGroupType)
{
    using namespace SOP_FeE_AttribExtremum_2_0Enums;
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




void
SOP_FeE_AttribExtremum_2_0Verb::cook(const SOP_NodeVerb::CookParms &cookparms) const
{
    auto &&sopparms = cookparms.parms<SOP_FeE_AttribExtremum_2_0Parms>();
    GA_Detail& outGeo0 = *cookparms.gdh().gdpNC();

    //const GA_Detail& inGeo0 = *cookparms.inputGeo(0);
    const GA_Detail* const inGeo1 = cookparms.inputGeo(1);
    
    //outGeo0.replaceWith(inGeo0);

    const GA_AttributeOwner attribClass = sopAttribOwner(sopparms.getAttribClass());
    const GA_GroupType groupType = sopGroupType(sopparms.getGroupType());


    GFE_AttribExtremum attribExtremum(outGeo0, inGeo1, cookparms);
    attribExtremum.outAttribMin = sopparms.getOutAttribMin();
    attribExtremum.outAttribMax = sopparms.getOutAttribMax();
    attribExtremum.outAttribMinElemnum = sopparms.getOutAttribMinElemnum();
    attribExtremum.outAttribMaxElemnum = sopparms.getOutAttribMaxElemnum();
    attribExtremum.setComputeParm(sopparms.getOutAsOffset(),
        sopparms.getSubscribeRatio(), sopparms.getMinGrainSize());
    
    attribExtremum.setAttrib(attribClass, sopparms.getAttrib());
    
    attribExtremum.groupParser.setGroup(groupType, sopparms.getGroup());
    
    attribExtremum.computeAndBumpDataId();


 
}

