
//#define UT_ASSERT_LEVEL 3
#include "SOP_FeE_AttribRemap_1_0.h"
#include "SOP_FeE_AttribRemap_1_0.proto.h"


#include "GA/GA_Detail.h"
#include "PRM/PRM_TemplateBuilder.h"
#include "UT/UT_Interrupt.h"
#include "UT/UT_DSOVersion.h"


#include "GFE/GFE_AttributeRemap.h"


using namespace SOP_FeE_AttribRemap_1_0_Namespace;



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
        name    "clampMin"
        cppname "Clamp Min"
        label   "Clamp Min"
        type    toggle
        default { "off" }
    }
    parm {
        name    "clampMax"
        cppname "Clamp Max"
        label   "Clamp Max"
        type    toggle
        default { "off" }
    }
    parm {
        name    "useextermum_inmin"
        cppname "SubscribeRatio"
        label   "Use Extermum inmin"
        type    toggle
        default { "1" }
    }
    parm {
        name    "inputmin"
        cppname "SubscribeRatio"
        label   "Input Min"
        type    vector
        size    3
        default { "0" "0" "0" }
        disablewhen "{ useextermum_inmin == 1 }"
        range   { -1 1 }
    }
    parm {
        name    "useextermum_inmax"
        cppname "SubscribeRatio"
        label   "Use Extermum inmax"
        type    toggle
        default { "1" }
    }
    parm {
        name    "inputmax"
        cppname "SubscribeRatio"
        label   "Input Max"
        type    vector
        size    3
        default { "1" "1" "1" }
        disablewhen "{ useextermum_inmax == 1 }"
        range   { -1 1 }
    }
    parm {
        name    "useextermum_outmin"
        cppname "SubscribeRatio"
        label   "Use Extermum outmin"
        type    toggle
        default { "0" }
    }
    parm {
        name    "outputmin"
        cppname "SubscribeRatio"
        label   "Output Min"
        type    vector
        size    3
        default { "0" "0" "0" }
        disablewhen "{ useextermum_outmin == 1 }"
        range   { -1 1 }
    }
    parm {
        name    "useextermum_outmax"
        cppname "SubscribeRatio"
        label   "Use Extermum outmax"
        type    toggle
        default { "0" }
    }
    parm {
        name    "outputmax"
        cppname "SubscribeRatio"
        label   "Output Max"
        type    vector
        size    3
        default { "1" "1" "1" }
        disablewhen "{ useextermum_outmax == 1 }"
        range   { -1 1 }
    }
    parm {
        name    "doRemap"
        cppname "DoRemap"
        label   "Do Remap"
        type    toggle
        nolabel
        joinnext
        default { "0" }
    }
    parm {
        name    "remapRamp"
        cppname "RemapRamp"
        label   "Remap Ramp"
        type    ramp_flt
        default { "2" }
        disablewhen "{ ramp_toggle == 0 }"
        range   { 1! 10 }
    }
    parm {
        name    "uniScale"
        cppname "UniScale"
        label   "Uniform Scale"
        type    float
        default { 1 }
        range   { -100 100 }
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
SOP_FeE_AttribRemap_1_0::buildTemplates()
{
    static PRM_TemplateBuilder templ("SOP_FeE_AttribRemap_1_0.C"_sh, theDsFile);
    if (templ.justBuilt())
    {
        templ.setChoiceListPtr("attrib"_sh, &SOP_Node::allAttribMenu);
        templ.setChoiceListPtr("group"_sh, &SOP_Node::allGroupMenu);
    }
    return templ.templates();
}

const UT_StringHolder SOP_FeE_AttribRemap_1_0::theSOPTypeName("FeE::attribRemap::1.0"_sh);

void
newSopOperator(OP_OperatorTable* table)
{
    OP_Operator* newOp = new OP_Operator(
        SOP_FeE_AttribRemap_1_0::theSOPTypeName,
        "FeE Attribute Remap",
        SOP_FeE_AttribRemap_1_0::myConstructor,
        SOP_FeE_AttribRemap_1_0::buildTemplates(),
        1,
        1,
        nullptr,
        OP_FLAG_GENERATOR,
        nullptr,
        1,
        "Five elements Elf/Attribute");

    newOp->setIconName("SOP_attribremap");
    table->addOperator(newOp);
}

class SOP_FeE_AttribRemap_1_0Verb : public SOP_NodeVerb
{
public:
    SOP_FeE_AttribRemap_1_0Verb() {}
    virtual ~SOP_FeE_AttribRemap_1_0Verb() {}

    virtual SOP_NodeParms* allocParms() const { return new SOP_FeE_AttribRemap_1_0Parms(); }
    virtual UT_StringHolder name() const { return SOP_FeE_AttribRemap_1_0::theSOPTypeName; }

    virtual CookMode cookMode(const SOP_NodeParms* parms) const { return COOK_INPLACE; }

    virtual void cook(const CookParms& cookparms) const;

    static const SOP_NodeVerb::Register<SOP_FeE_AttribRemap_1_0Verb> theVerb;
};

const SOP_NodeVerb::Register<SOP_FeE_AttribRemap_1_0Verb> SOP_FeE_AttribRemap_1_0Verb::theVerb;

const SOP_NodeVerb*
SOP_FeE_AttribRemap_1_0::cookVerb() const
{
    return SOP_FeE_AttribRemap_1_0Verb::theVerb.get();
}




static GA_AttributeOwner
sopAttribOwner(const SOP_FeE_AttribRemap_1_0Parms::AttribClass parmAttribClass)
{
    using namespace SOP_FeE_AttribRemap_1_0Enums;
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
sopGroupType(const SOP_FeE_AttribRemap_1_0Parms::GroupType parmGroupType)
{
    using namespace SOP_FeE_AttribRemap_1_0Enums;
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
SOP_FeE_AttribRemap_1_0Verb::cook(const SOP_NodeVerb::CookParms &cookparms) const
{
    auto &&sopparms = cookparms.parms<SOP_FeE_AttribRemap_1_0Parms>();
    GA_Detail& outGeo0 = *cookparms.gdh().gdpNC();

    //const GA_Detail& inGeo0 = *cookparms.inputGeo(0);

    //outGeo0.replaceWith(inGeo0);

    const fpreal64 uniScale = sopparms.getUniScale();
    const bool doNormalize = sopparms.getNormalize();

    if (!doNormalize && uniScale==1.0)
        return;

    const GA_AttributeOwner attribClass = sopAttribOwner(sopparms.getAttribClass());
    const GA_GroupType groupType = sopGroupType(sopparms.getGroupType());


    GFE_AttribRemap attribRemap(outGeo0, cookparms);
    
    attribRemap.groupParser.setGroup(groupType, sopparms.getGroup());
    attribRemap.getOutAttribArray().appends(attribClass, sopparms.getAttribName());
    attribRemap.setComputeParm(doNormalize, uniScale,
        sopparms.getSubscribeRatio(), sopparms.getMinGrainSize());
    attribRemap.computeAndBumpDataId();


}

