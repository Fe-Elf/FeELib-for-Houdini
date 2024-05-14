
//#define UT_ASSERT_LEVEL 3
#include "SOP_FeE_MatchAxis_1_0.h"


#include "SOP_FeE_MatchAxis_1_0.proto.h"

#include "GA/GA_Detail.h"
#include "PRM/PRM_TemplateBuilder.h"
#include "UT/UT_Interrupt.h"
#include "UT/UT_DSOVersion.h"




#include "GFE/GFE_MatchAxis.h"




using namespace SOP_FeE_MatchAxis_1_0_Namespace;


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
        name    "primAttribToXform"
        cppname "PrimAttribToXform"
        label   "Prim Attrib to Transform"
        type    string
        default { "" }
    }
    parm {
        name    "pointAttribToXform"
        cppname "PointAttribToXform"
        label   "Point Attrib to Transform"
        type    string
        default { "P" }
    }
    parm {
        name    "vertexAttribToXform"
        cppname "VertexAttribToXform"
        label   "Vertex Attrib to Transform"
        type    string
        default { "" }
    }
    parm {
        name    "detailAttribToXform"
        cppname "DetailAttribToXform"
        label   "Detail Attrib to Transform"
        type    string
        default { "" }
    }
    parm {
        name    "fromVec"
        cppname "FromVec"
        label   "From Vector"
        type    direction
        size    3
        default { "0" "0" "1" }
        range   { -1 1 }
    }
    parm {
        name    "toVec"
        cppname "ToVec"
        label   "To Vector"
        type    direction
        size    3
        default { "0" "1" "0" }
        range   { -1 1 }
    }
    parm {
        name    "upVec"
        cppname "UpVec"
        label   "Up Vector"
        type    direction
        size    3
        default { "0" "1" "0" }
        range   { -1 1 }
    }
    parm {
        name    "center"
        cppname "Center"
        label   "Center"
        type    vector
        size    3
        default { "0" "0" "0" }
        disablewhen "{ hasinput(1) == 1 }"
        range   { -1 1 }
    }
    parm {
        name    "bias"
        cppname "Bias"
        label   "Bias"
        type    float
        default { "1" }
        range   { 0! 1! }
    }
    
    parm {
        name    "outXformAttrib"
        cppname "OutXformAttrib"
        label   "Out Xform Attrib"
        type    toggle
        default { "0" }
        joinnext
        nolabel
    }
    parm {
        name    "xformAttribName"
        cppname "XformAttribName"
        label   "Xform Attrib Name"
        type    string
        default { "xform" }
        disablewhen "{ outXformAttrib == 0 }"
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
SOP_FeE_MatchAxis_1_0::buildTemplates()
{
    static PRM_TemplateBuilder templ("SOP_FeE_MatchAxis_1_0.C"_sh, theDsFile);
    if (templ.justBuilt())
    {
        templ.setChoiceListPtr("group"_sh, &SOP_Node::allGroupMenu);
        templ.setChoiceListPtr("primAttribToXform"_sh,   &SOP_Node::primAttribMenu);
        templ.setChoiceListPtr("pointAttribToXform"_sh,  &SOP_Node::pointAttribMenu);
        templ.setChoiceListPtr("vertexAttribToXform"_sh, &SOP_Node::vertexAttribMenu);
        templ.setChoiceListPtr("detailAttribToXform"_sh, &SOP_Node::detailAttribMenu);
    }
    return templ.templates();
}

const UT_StringHolder SOP_FeE_MatchAxis_1_0::theSOPTypeName("FeE::matchAxis::1.0"_sh);

void
newSopOperator(OP_OperatorTable* table)
{
    OP_Operator* newOp = new OP_Operator(
        SOP_FeE_MatchAxis_1_0::theSOPTypeName,
        "FeE Match Axis",
        SOP_FeE_MatchAxis_1_0::myConstructor,
        SOP_FeE_MatchAxis_1_0::buildTemplates(),
        1,
        3,
        nullptr,
        OP_FLAG_GENERATOR,
        nullptr,
        1,
        "Five elements Elf/Filter/Match");

    newOp->setIconName("SOP_matchaxis");
    table->addOperator(newOp);

}





class SOP_FeE_MatchAxis_1_0Verb : public SOP_NodeVerb
{
public:
    SOP_FeE_MatchAxis_1_0Verb() {}
    virtual ~SOP_FeE_MatchAxis_1_0Verb() {}

    virtual SOP_NodeParms *allocParms() const { return new SOP_FeE_MatchAxis_1_0Parms(); }
    virtual UT_StringHolder name() const { return SOP_FeE_MatchAxis_1_0::theSOPTypeName; }

    virtual CookMode cookMode(const SOP_NodeParms *parms) const { return COOK_INPLACE; }

    virtual void cook(const CookParms &cookparms) const;
    
    /// This static data member automatically registers
    /// this verb class at library load time.
    static const SOP_NodeVerb::Register<SOP_FeE_MatchAxis_1_0Verb> theVerb;
};

// The static member variable definition has to be outside the class definition.
// The declaration is inside the class.
const SOP_NodeVerb::Register<SOP_FeE_MatchAxis_1_0Verb> SOP_FeE_MatchAxis_1_0Verb::theVerb;

const SOP_NodeVerb *
SOP_FeE_MatchAxis_1_0::cookVerb() const 
{ 
    return SOP_FeE_MatchAxis_1_0Verb::theVerb.get();
}







static GA_GroupType
sopGroupType(const  SOP_FeE_MatchAxis_1_0Parms::GroupType parmGroupType)
{
    using namespace SOP_FeE_MatchAxis_1_0Enums;
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
SOP_FeE_MatchAxis_1_0Verb::cook(const SOP_NodeVerb::CookParms &cookparms) const
{
    auto&& sopparms = cookparms.parms<SOP_FeE_MatchAxis_1_0Parms>();
    GA_Detail& outGeo0 = *cookparms.gdh().gdpNC();
    //auto sopcache = (SOP_FeE_MatchAxis_1_0Cache*)cookparms.cache();

    //const GA_Detail& inGeo0 = *cookparms.inputGeo(0);

    //outGeo0.replaceWith(inGeo0);
    
    const GA_GroupType groupType = sopGroupType(sopparms.getGroupType());

    UT_AutoInterrupt boss("Processing");
    if (boss.wasInterrupted())
        return;

    GFE_MatchAxis matchAxis(outGeo0, cookparms);
    
    matchAxis.getOutAttribArray().appendPrimitives(sopparms.getPrimAttribToXform());
    matchAxis.getOutAttribArray().appendPoints    (sopparms.getPointAttribToXform());
    matchAxis.getOutAttribArray().appendVertices  (sopparms.getVertexAttribToXform());
    matchAxis.getOutAttribArray().appendDetails   (sopparms.getDetailAttribToXform());

    
    matchAxis.setComputeParm(sopparms.getSubscribeRatio(), sopparms.getMinGrainSize());
    
    matchAxis.setXformVector(sopparms.getFromVec(), sopparms.getToVec(), sopparms.getUpVec(), sopparms.getCenter());

    if (sopparms.getOutXformAttrib())
        matchAxis.setXformAttrib(false, GA_STORE_INVALID, sopparms.getXformAttribName());
    

    matchAxis.groupParser.setGroup(groupType, sopparms.getGroup());
    matchAxis.computeAndBumpDataId();


}


    