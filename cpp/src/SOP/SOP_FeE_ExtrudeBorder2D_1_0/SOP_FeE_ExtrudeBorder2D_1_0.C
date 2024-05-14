
//#define UT_ASSERT_LEVEL 3
#include "SOP_FeE_ExtrudeBorder2D_1_0.h"


#include "SOP_FeE_ExtrudeBorder2D_1_0.proto.h"

#include "GA/GA_Detail.h"
#include "PRM/PRM_TemplateBuilder.h"
#include "UT/UT_Interrupt.h"
#include "UT/UT_DSOVersion.h"


#include "GFE/GFE_ExtrudeBorder2D.h"



using namespace SOP_FeE_ExtrudeBorder2D_1_0_Namespace;


static const char *theDsFile = R"THEDSFILE(
{
    name	parameters
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
        name    "extrudeBorder2DDist"
        cppname "ExtrudeBorder2DDist"
        label   "Distance"
        type    log
        default { "0.01" }
        range   { 0.0001 1 }
    }
    parm {
        name    "reverseMesh"
        cppname "ReverseMesh"
        label   "Reverse Mesh"
        type    toggle
        default { "0" }
    }
}
)THEDSFILE";

PRM_Template*
SOP_FeE_ExtrudeBorder2D_1_0::buildTemplates()
{
    static PRM_TemplateBuilder templ("SOP_FeE_ExtrudeBorder2D_1_0.C"_sh, theDsFile);
    if (templ.justBuilt())
    {
        templ.setChoiceListPtr("group"_sh, &SOP_Node::allGroupMenu);
    }
    return templ.templates();
}





const UT_StringHolder SOP_FeE_ExtrudeBorder2D_1_0::theSOPTypeName("FeE::extrudeBorder2D::1.0"_sh);

void
newSopOperator(OP_OperatorTable* table)
{
    OP_Operator* newOp = new OP_Operator(
        SOP_FeE_ExtrudeBorder2D_1_0::theSOPTypeName,
        "FeE Extrude Border 2D",
        SOP_FeE_ExtrudeBorder2D_1_0::myConstructor,
        SOP_FeE_ExtrudeBorder2D_1_0::buildTemplates(),
        1,
        1,
        nullptr,
        OP_FLAG_GENERATOR,
        nullptr,
        1,
        "Five elements Elf/Poly Edit");

    newOp->setIconName("SOP_polyextrude-2.0");
    table->addOperator(newOp);
}




class SOP_FeE_ExtrudeBorder2D_1_0Verb : public SOP_NodeVerb
{
public:
    SOP_FeE_ExtrudeBorder2D_1_0Verb() {}
    virtual ~SOP_FeE_ExtrudeBorder2D_1_0Verb() {}

    virtual SOP_NodeParms *allocParms() const { return new SOP_FeE_ExtrudeBorder2D_1_0Parms(); }
    virtual UT_StringHolder name() const { return SOP_FeE_ExtrudeBorder2D_1_0::theSOPTypeName; }

    virtual CookMode cookMode(const SOP_NodeParms *parms) const { return COOK_INPLACE; }

    virtual void cook(const CookParms &cookparms) const;
    
    static const SOP_NodeVerb::Register<SOP_FeE_ExtrudeBorder2D_1_0Verb> theVerb;
};

const SOP_NodeVerb::Register<SOP_FeE_ExtrudeBorder2D_1_0Verb> SOP_FeE_ExtrudeBorder2D_1_0Verb::theVerb;

const SOP_NodeVerb *
SOP_FeE_ExtrudeBorder2D_1_0::cookVerb() const 
{ 
    return SOP_FeE_ExtrudeBorder2D_1_0Verb::theVerb.get();
}










static GA_GroupType
sopGroupType(const SOP_FeE_ExtrudeBorder2D_1_0Parms::GroupType parmGroupType)
{
    using namespace SOP_FeE_ExtrudeBorder2D_1_0Enums;
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
SOP_FeE_ExtrudeBorder2D_1_0Verb::cook(const SOP_NodeVerb::CookParms& cookparms) const
{
    auto&& sopparms = cookparms.parms<SOP_FeE_ExtrudeBorder2D_1_0Parms>();
    GA_Detail& outGeo0 = *cookparms.gdh().gdpNC();
    //auto sopcache = (SOP_FeE_ExtrudeBorder2D_1_0Cache*)cookparms.cache();

    //const GA_Detail& inGeo0 = *cookparms.inputGeo(0);
    //outGeo0.replaceWith(inGeo0);


    const GA_GroupType groupType = sopGroupType(sopparms.getGroupType());

    UT_AutoInterrupt boss("Processing");
    if (boss.wasInterrupted())
        return;

    GFE_ExtrudeBorder2D extrudeBorder2D(outGeo0, cookparms);
    extrudeBorder2D.groupParser.setGroup(groupType, sopparms.getGroup());
    extrudeBorder2D.setComputeParm(sopparms.getExtrudeBorder2DDist(), sopparms.getReverseMesh());
    extrudeBorder2D.computeAndBumpDataIdsForAddOrRemove();
    
}

