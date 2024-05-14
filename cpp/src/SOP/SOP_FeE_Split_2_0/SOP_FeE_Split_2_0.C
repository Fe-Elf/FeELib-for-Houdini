
//#define UT_ASSERT_LEVEL 3
#include "SOP_FeE_Split_2_0.h"


#include "SOP_FeE_Split_2_0.proto.h"

#include "GA/GA_Detail.h"
#include "PRM/PRM_TemplateBuilder.h"
#include "UT/UT_Interrupt.h"
#include "UT/UT_DSOVersion.h"


#include "GFE/GFE_Split.h"



using namespace SOP_FeE_Split_2_0_Namespace;


static const char *theDsFile = R"THEDSFILE(
{
    name	parameters

    parm {
        name    "splitGroup"
        cppname "SplitGroup"
        label   "Split Group"
        type    string
        default { "" }
        parmtag { "script_action" "import soputils\nkwargs['geometrytype'] = kwargs['node'].parmTuple('splitGroupType')\nkwargs['inputindex'] = 0\nsoputils.selectGroupParm(kwargs)" }
        parmtag { "script_action_help" "Select geometry from an available viewport.\nShift-click to turn on Select Groups." }
        parmtag { "script_action_icon" "BUTTONS_reselect" }
        parmtag { "sop_input" "0" }
    }
    parm {
        name    "splitGroupType"
        cppname "SplitGroupType"
        label   "Split Group Type"
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
        name    "reverseGroup"
        cppname "ReverseGroup"
        label   "Reverse Group"
        type    toggle
        default { "off" }
    }
    parm {
        name    "delInputGroup"
        cppname "DelInputGroup"
        label   "Delete Input Group"
        type    toggle
        default { "on" }
    }

    parm {
        name    "delUnusedPoint"
        cppname "DelUnusedPoint"
        label   "Delete Unused Point"
        type    toggle
        default { 1 }
    }
    parm {
        name    "delPointMode"
        cppname "DelPointMode"
        label   "Delete Point Mode"
        type    ordinal
        default { "delDegenerateIncompatible" }
        menu {
            "leavePrimitive"             "Leave Primitive"
            "delDegenerate"              "Delete Degenerate"
            "delDegenerateIncompatible"  "Delete Degenerate Incompatible"
        }
    }
    parm {
        name    "guaranteeNoVertexReference"
        cppname "GuaranteeNoVertexReference"
        label   "Guarantee No Vertex Reference"
        type    toggle
        default { 0 }
    }
}
)THEDSFILE";

PRM_Template*
SOP_FeE_Split_2_0::buildTemplates()
{
    static PRM_TemplateBuilder templ("SOP_FeE_Split_2_0.C"_sh, theDsFile);
    if (templ.justBuilt())
    {
        templ.setChoiceListPtr("splitGroup"_sh, &SOP_Node::allGroupMenu);
    }
    return templ.templates();
}





const UT_StringHolder SOP_FeE_Split_2_0::theSOPTypeName("FeE::delElement::1.0"_sh);

void
newSopOperator(OP_OperatorTable* table)
{
    OP_Operator* newOp = new OP_Operator(
        SOP_FeE_Split_2_0::theSOPTypeName,
        "FeE Delete Element",
        SOP_FeE_Split_2_0::myConstructor,
        SOP_FeE_Split_2_0::buildTemplates(),
        1,
        1,
        nullptr,
        OP_FLAG_GENERATOR,
        nullptr,
        2,
        "Five elements Elf/Operation/Delete");

    newOp->setIconName("SOP_delete");
    table->addOperator(newOp);
}




class SOP_FeE_Split_2_0Verb : public SOP_NodeVerb
{
public:
    SOP_FeE_Split_2_0Verb() {}
    virtual ~SOP_FeE_Split_2_0Verb() {}

    virtual SOP_NodeParms *allocParms() const { return new SOP_FeE_Split_2_0Parms(); }
    virtual UT_StringHolder name() const { return SOP_FeE_Split_2_0::theSOPTypeName; }

    virtual CookMode cookMode(const SOP_NodeParms *parms) const { return COOK_GENERIC; }

    virtual void cook(const CookParms &cookparms) const;
    
    static const SOP_NodeVerb::Register<SOP_FeE_Split_2_0Verb> theVerb;
};

const SOP_NodeVerb::Register<SOP_FeE_Split_2_0Verb> SOP_FeE_Split_2_0Verb::theVerb;

const SOP_NodeVerb *
SOP_FeE_Split_2_0::cookVerb() const 
{ 
    return SOP_FeE_Split_2_0Verb::theVerb.get();
}









static GA_GroupType
sopGroupType(SOP_FeE_Split_2_0Parms::SplitGroupType parmgrouptype)
{
    using namespace SOP_FeE_Split_2_0Enums;
    switch (parmgrouptype)
    {
    case SplitGroupType::GUESS:     return GA_GROUP_INVALID;    break;
    case SplitGroupType::PRIM:      return GA_GROUP_PRIMITIVE;  break;
    case SplitGroupType::POINT:     return GA_GROUP_POINT;      break;
    case SplitGroupType::VERTEX:    return GA_GROUP_VERTEX;     break;
    case SplitGroupType::EDGE:      return GA_GROUP_EDGE;       break;
    }
    UT_ASSERT_MSG(0, "Unhandled geo0Group type!");
    return GA_GROUP_INVALID;
}

static GA_Detail::GA_DestroyPointMode
sopDelPointMode(SOP_FeE_Split_2_0Parms::DelPointMode delPointMode)
{
    using namespace SOP_FeE_Split_2_0Enums;
    switch (delPointMode)
    {
    case DelPointMode::LEAVEPRIMITIVE:              return GA_Detail::GA_DestroyPointMode::GA_LEAVE_PRIMITIVES;                 break;
    case DelPointMode::DELDEGENERATE:               return GA_Detail::GA_DestroyPointMode::GA_DESTROY_DEGENERATE;               break;
    case DelPointMode::DELDEGENERATEINCOMPATIBLE:   return GA_Detail::GA_DestroyPointMode::GA_DESTROY_DEGENERATE_INCOMPATIBLE;  break;
    }
    UT_ASSERT_MSG(0, "Unhandled Delete Point Mode!");
    return GA_Detail::GA_DestroyPointMode::GA_DESTROY_DEGENERATE_INCOMPATIBLE;
}

void
SOP_FeE_Split_2_0Verb::cook(const SOP_NodeVerb::CookParms& cookparms) const
{
    auto&& sopparms = cookparms.parms<SOP_FeE_Split_2_0Parms>();
    GA_Detail& outGeo0 = *cookparms.gdh().gdpNC();
    //auto sopcache = (SOP_FeE_Split_2_0Cache*)cookparms.cache();

    const GA_Detail& inGeo0 = *cookparms.inputGeo(0);
    outGeo0.replaceWith(inGeo0);



    UT_AutoInterrupt boss("Processing");
    if (boss.wasInterrupted())
        return;



}

