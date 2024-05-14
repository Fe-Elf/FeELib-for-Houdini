
//#define UT_ASSERT_LEVEL 3
#include "SOP_FeE_KuhnMunkres_2_0.h"

#include "SOP_FeE_KuhnMunkres_2_0.proto.h"


#include <GA/GA_Detail.h>
#include <PRM/PRM_TemplateBuilder.h>
#include <UT/UT_Interrupt.h>
#include <UT/UT_DSOVersion.h>


#include <gfe.h>
//#include <GFE/GFE_KuhnMunkres.h>

using namespace SOP_FeE_KuhnMunkres_2_0_Namespace;


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
        name    "kmMatchedPointAttribName"
        cppname "KMMatchedPointAttribName"
        label   "KM Matched Point Attrib Name"
        type    string
        default { "kmMatchedPoint" }
    }

    parm {
        name    "posAttrib"
        cppname "PosAttrib"
        label   "Pos Attrib"
        type    string
        default { "P" }
    }
    parm {
        name    "posRefAttrib"
        cppname "PosRefAttrib"
        label   "Pos Ref Attrib"
        type    string
        default { "P" }
    }

    //parm {
    //    name    "subscribeRatio"
    //    cppname "SubscribeRatio"
    //    label   "Subscribe Ratio"
    //    type    integer
    //    default { 16 }
    //    range   { 0! 256 }
    //}
    //parm {
    //    name    "minGrainSize"
    //    cppname "MinGrainSize"
    //    label   "Min Grain Size"
    //    type    intlog
    //    default { 1024 }
    //    range   { 0! 2048 }
    //}

}
)THEDSFILE";

PRM_Template*
SOP_FeE_KuhnMunkres_2_0::buildTemplates()
{
    static PRM_TemplateBuilder templ("SOP_FeE_KuhnMunkres_2_0.C"_sh, theDsFile);
    if (templ.justBuilt())
    {
        templ.setChoiceListPtr("group"_sh, &SOP_Node::allGroupMenu);
        templ.setChoiceListPtr("posAttrib"_sh, &SOP_Node::pointAttribMenu);
        templ.setChoiceListPtr("posRefAttrib"_sh, &SOP_Node::pointAttribMenu);
    }
    return templ.templates();
}


const UT_StringHolder SOP_FeE_KuhnMunkres_2_0::theSOPTypeName("FeE::kuhnMunkres::2.0"_sh);

void
newSopOperator(OP_OperatorTable* table)
{
    OP_Operator* newOp = new OP_Operator(
        SOP_FeE_KuhnMunkres_2_0::theSOPTypeName,
        "FeE Kuhn Munkres",
        SOP_FeE_KuhnMunkres_2_0::myConstructor,
        SOP_FeE_KuhnMunkres_2_0::buildTemplates(),
        2,
        2,
        nullptr,
        OP_FLAG_GENERATOR,
        nullptr,
        1,
        "Five elements Elf/Filter/Match");

    newOp->setIconName("SOP_extracttransform");
    table->addOperator(newOp);

}

//class SOP_FeE_KuhnMunkres_2_0Cache : public SOP_NodeCache
//{
//public:
//    SOP_FeE_KuhnMunkres_2_0Cache() : SOP_NodeCache(),
//        myPrevOutputDetailID(-1)
//    {}
//    ~SOP_FeE_KuhnMunkres_2_0Cache() override {}
//
//    int64 myPrevOutputDetailID;
//};


class SOP_FeE_KuhnMunkres_2_0Verb : public SOP_NodeVerb
{
public:
    SOP_FeE_KuhnMunkres_2_0Verb() {}
    virtual ~SOP_FeE_KuhnMunkres_2_0Verb() {}

    virtual SOP_NodeParms *allocParms() const { return new SOP_FeE_KuhnMunkres_2_0Parms(); }
    //virtual SOP_NodeCache* allocCache() const { return new SOP_FeE_KuhnMunkres_2_0Cache(); }
    virtual UT_StringHolder name() const { return SOP_FeE_KuhnMunkres_2_0::theSOPTypeName; }

    virtual CookMode cookMode(const SOP_NodeParms *parms) const { return COOK_GENERIC; }

    virtual void cook(const CookParms &cookparms) const;

    /// This static data member automatically registers
    /// this verb class at library ldir0d time.
    static const SOP_NodeVerb::Register<SOP_FeE_KuhnMunkres_2_0Verb> theVerb;
};

// The static member variable definition has to be outside the class definition.
// The declaration is inside the class.
const SOP_NodeVerb::Register<SOP_FeE_KuhnMunkres_2_0Verb> SOP_FeE_KuhnMunkres_2_0Verb::theVerb;

const SOP_NodeVerb *
SOP_FeE_KuhnMunkres_2_0::cookVerb() const 
{ 
    return SOP_FeE_KuhnMunkres_2_0Verb::theVerb.get();
}








static GA_GroupType
sopGroupType(const SOP_FeE_KuhnMunkres_2_0Parms::GroupType parmGroupType)
{
    using namespace SOP_FeE_KuhnMunkres_2_0Enums;
    switch (parmgrouptype)
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
SOP_FeE_KuhnMunkres_2_0Verb::cook(const SOP_NodeVerb::CookParms &cookparms) const
{
    auto &&sopparms = cookparms.parms<SOP_FeE_KuhnMunkres_2_0Parms>();
    GA_Detail* const outGeo0 = cookparms.gdh().gdpNC();
    //auto sopcache = (SOP_FeE_KuhnMunkres_2_0Cache*)cookparms.cache();

    const GA_Detail* const inGeo0 = cookparms.inputGeo(0);
    const GA_Detail* const inGeo1 = cookparms.inputGeo(1);

    outGeo0->replaceWith(*inGeo0);


    //const exint subscribeRatio = sopparms.getSubscribeRatio();
    //const exint minGrainSize = sopparms.getMinGrainSize();

    UT_AutoInterrupt boss("Processing");
    if (boss.wasInterrupted())
        return;

    const UT_StringHolder& kmMatchedPointName = sopparms.getKMMatchedPointAttribName();

    GFE_KuhnMunkres::kuhnMunkres(cookparms, outGeo0, inGeo1, kmMatchedPointName, sopparms.getPosAttrib(), sopparms.getPosRefAttrib());

    

}


