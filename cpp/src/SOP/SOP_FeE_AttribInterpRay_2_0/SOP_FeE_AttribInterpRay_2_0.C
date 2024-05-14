
//#define UT_ASSERT_LEVEL 3
#include "SOP_FeE_AttribInterpRay_2_0.h"
#include "SOP_FeE_AttribInterpRay_2_0.proto.h"


#include "GA/GA_Detail.h"
#include "PRM/PRM_TemplateBuilder.h"
#include "UT/UT_Interrupt.h"
#include "UT/UT_DSOVersion.h"


#include "GFE/GFE_AttributeInterpolateRay.h"


using namespace SOP_FeE_AttribInterpRay_2_0_Namespace;



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
        name    "minimumDist"
        cppname "MinimumDist"
        label   "Minimum Distance"
        type    toggle
        default { "1" }
    }
    parm {
        name    "maxDist"
        cppname "MaxDist"
        label   "Max Distance"
        type    log
        default { "1" }
        range { 0.001 10 }
    }

    //parm {
    //    name    "dirAttribClass"
    //    cppname "DirAttribClass"
    //    label   "Dir Attrib Class"
    //    type    ordinal
    //    default { "point" }
    //    menu {
    //        "prim"      "Primitive"
    //        "point"     "Point"
    //        "vertex"    "Vertex"
    //        "detail"    "Detail"
    //    }
    //}
    parm {
        name    "dirAttrib"
        cppname "DirAttrib"
        label   "Dir Attrib"
        type    string
        default { "N" }
        disablewhen "{ minimumDist == 1 }"
    }

    parm {
        name    "interpAttrib"
        cppname "InterpAttrib"
        label   "Interp Attrib"
        type    string
        default { "P" }
    }



    parm {
        name    "kernel"
        cppname "Kernel"
        label   "Kernel"
        type    integer
        default { 0 }
        range   { 0! 2 }
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
SOP_FeE_AttribInterpRay_2_0::buildTemplates()
{
    static PRM_TemplateBuilder templ("SOP_FeE_AttribInterpRay_2_0.C"_sh, theDsFile);
    if (templ.justBuilt())
    {
        templ.setChoiceListPtr("group"_sh, &SOP_Node::allGroupMenu);
    }
    return templ.templates();
}

const UT_StringHolder SOP_FeE_AttribInterpRay_2_0::theSOPTypeName("FeE::attribInterp_ray::2.0"_sh);

void
newSopOperator(OP_OperatorTable* table)
{
    OP_Operator* newOp = new OP_Operator(
        SOP_FeE_AttribInterpRay_2_0::theSOPTypeName,
        "FeE Attribute Interpolate Ray",
        SOP_FeE_AttribInterpRay_2_0::myConstructor,
        SOP_FeE_AttribInterpRay_2_0::buildTemplates(),
        2,
        2,
        nullptr,
        OP_FLAG_GENERATOR,
        nullptr,
        1,
        "Five elements Elf/Filter/Interpolate");

    newOp->setIconName("SOP_attribinterpolate");
    table->addOperator(newOp);
}

class SOP_FeE_AttribInterpRay_2_0Verb : public SOP_NodeVerb
{
public:
    SOP_FeE_AttribInterpRay_2_0Verb() {}
    virtual ~SOP_FeE_AttribInterpRay_2_0Verb() {}

    virtual SOP_NodeParms* allocParms() const { return new SOP_FeE_AttribInterpRay_2_0Parms(); }
    virtual UT_StringHolder name() const { return SOP_FeE_AttribInterpRay_2_0::theSOPTypeName; }

    virtual CookMode cookMode(const SOP_NodeParms* parms) const { return COOK_INPLACE; }

    virtual void cook(const CookParms& cookparms) const;

    static const SOP_NodeVerb::Register<SOP_FeE_AttribInterpRay_2_0Verb> theVerb;
};

const SOP_NodeVerb::Register<SOP_FeE_AttribInterpRay_2_0Verb> SOP_FeE_AttribInterpRay_2_0Verb::theVerb;

const SOP_NodeVerb*
SOP_FeE_AttribInterpRay_2_0::cookVerb() const
{
    return SOP_FeE_AttribInterpRay_2_0Verb::theVerb.get();
}




static GA_GroupType
sopGroupType(const SOP_FeE_AttribInterpRay_2_0Parms::GroupType parmGroupType)
{
    using namespace SOP_FeE_AttribInterpRay_2_0Enums;
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
SOP_FeE_AttribInterpRay_2_0Verb::cook(const SOP_NodeVerb::CookParms &cookparms) const
{
    auto &&sopparms = cookparms.parms<SOP_FeE_AttribInterpRay_2_0Parms>();
    GA_Detail& outGeo0 = *cookparms.gdh().gdpNC();

    //const GA_Detail& inGeo0 = *cookparms.inputGeo(0);
    const GA_Detail& inGeo1 = *cookparms.inputGeo(1);

    //outGeo0.replaceWith(inGeo0);

    const GA_GroupType groupType = sopGroupType(sopparms.getGroupType());


    GFE_AttribInterpRay attribInterpRay(outGeo0, &inGeo1, &cookparms);
    attribInterpRay.getOutAttribArray().appendPoints(sopparms.getInterpAttrib());
    attribInterpRay.setComputeParm(sopparms.getMinimumDist(), sopparms.getMaxDist(), sopparms.getSubscribeRatio(), sopparms.getMinGrainSize());

    attribInterpRay.groupParser.setGroup(groupType, sopparms.getGroup());
    attribInterpRay.computeAndBumpDataId();





}

