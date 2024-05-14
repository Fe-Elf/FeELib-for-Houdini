
//#define UT_ASSERT_LEVEL 3
#include "SOP_FeE_PointGenPerElem_1_0.h"


#include "SOP_FeE_PointGenPerElem_1_0.proto.h"

#include "GA/GA_Detail.h"
#include "PRM/PRM_TemplateBuilder.h"
#include "UT/UT_Interrupt.h"
#include "UT/UT_DSOVersion.h"




#include "GFE/GFE_PointGeneratePerElement.h"




using namespace SOP_FeE_PointGenPerElem_1_0_Namespace;


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
        name    "elementClass"
        cppname "ElementClass"
        label   "Element Class"
        type    ordinal
        default { "prim" }
        menu {
            "prim"      "Primitive"
            "point"     "Point"
            "edge"      "Edge"
        }
    }
    
    
    parm {
        name    "setPositionOnElem"
        cppname "SetPositionOnElem"
        label   "Set Position on Elem"
        type    toggle
        default { "1" }
    }
    parm {
        name    "useNumPointAttrib"
        cppname "UseNumPointAttrib"
        label   "Use Num Point Attrib"
        type    toggle
        default { "0" }
        nolabel
        joinnext
        disablewhen "{ elementClass == edge }"
    }
    parm {
        name    "numPointAttrib"
        cppname "NumPointAttrib"
        label   "Num Point Attrib"
        type    string
        default { "numpt" }
        disablewhen "{ useNumPointAttrib == 0 } { elementClass == edge }"
    }
    parm {
        name    "outSrcElemoffAttrib"
        cppname "OutSrcElemoffAttrib"
        label   "Out Source Elemoff Attrib"
        type    toggle
        default { "0" }
        nolabel
        joinnext
    }
    parm {
        name    "srcElemoffAttribName"
        cppname "SrcElemoffAttribName"
        label   "Source Elemoff Attrib Name"
        type    string
        default { "index" }
        disablewhen "{ outSrcElemoffAttrib == 0 }"
    }
    parm {
        name    "outAsOffset"
        cppname "OutAsOffset"
        label   "Out as Offset"
        type    toggle
        default { "off" }
        disablewhen "{ outSrcElemoffAttrib == 0 }"
    }
    parm {
        name    "keepSrcElemAttrib"
        cppname "KeepSrcElemAttrib"
        label   "Keep Src Elem Attrib"
        type    string
        default { "*" }
    }
    parm {
        name    "keepSrcElemGroup"
        cppname "KeepSrcElemGroup"
        label   "Keep Src Elem Group"
        type    string
        default { "*" }
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
SOP_FeE_PointGenPerElem_1_0::buildTemplates()
{
    static PRM_TemplateBuilder templ("SOP_FeE_PointGenPerElem_1_0.C"_sh, theDsFile);
    if (templ.justBuilt())
    {
        templ.setChoiceListPtr("group"_sh, &SOP_Node::allGroupMenu);
        templ.setChoiceListPtr("numPointAttrib"_sh,       &SOP_Node::allAttribReplaceMenu);
        templ.setChoiceListPtr("srcElemoffAttribName"_sh, &SOP_Node::pointAttribReplaceMenu);
        templ.setChoiceListPtr("keepSrcElemAttrib"_sh, &SOP_Node::allAttribMenu);
        templ.setChoiceListPtr("keepSrcElemGroup"_sh, &SOP_Node::allGroupMenu);
    }
    return templ.templates();
}

const UT_StringHolder SOP_FeE_PointGenPerElem_1_0::theSOPTypeName("FeE::pointGenPerElem::1.0"_sh);

void
newSopOperator(OP_OperatorTable* table)
{
    OP_Operator* newOp = new OP_Operator(
        SOP_FeE_PointGenPerElem_1_0::theSOPTypeName,
        "FeE Point Generate per Element",
        SOP_FeE_PointGenPerElem_1_0::myConstructor,
        SOP_FeE_PointGenPerElem_1_0::buildTemplates(),
        1,
        1,
        nullptr,
        OP_FLAG_GENERATOR,
        nullptr,
        1,
        "Five elements Elf/Generator/Point");

    newOp->setIconName("SOP_pointgenerate");
    table->addOperator(newOp);
}





class SOP_FeE_PointGenPerElem_1_0Verb : public SOP_NodeVerb
{
public:
    SOP_FeE_PointGenPerElem_1_0Verb() {}
    virtual ~SOP_FeE_PointGenPerElem_1_0Verb() {}

    virtual SOP_NodeParms *allocParms() const { return new SOP_FeE_PointGenPerElem_1_0Parms(); }
    virtual UT_StringHolder name() const { return SOP_FeE_PointGenPerElem_1_0::theSOPTypeName; }

    virtual CookMode cookMode(const SOP_NodeParms *parms) const { return COOK_GENERIC; }

    virtual void cook(const CookParms &cookparms) const;
    
    /// This static data member automatically registers
    /// this verb class at library load time.
    static const SOP_NodeVerb::Register<SOP_FeE_PointGenPerElem_1_0Verb> theVerb;
};

// The static member variable definition has to be outside the class definition.
// The declaration is inside the class.
const SOP_NodeVerb::Register<SOP_FeE_PointGenPerElem_1_0Verb> SOP_FeE_PointGenPerElem_1_0Verb::theVerb;

const SOP_NodeVerb *
SOP_FeE_PointGenPerElem_1_0::cookVerb() const 
{ 
    return SOP_FeE_PointGenPerElem_1_0Verb::theVerb.get();
}





static GA_GroupType
sopGroupType(const SOP_FeE_PointGenPerElem_1_0Parms::GroupType parmGroupType)
{
    using namespace SOP_FeE_PointGenPerElem_1_0Enums;
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

static GA_GroupType
sopGroupType(const SOP_FeE_PointGenPerElem_1_0Parms::ElementClass parmElementClass)
{
    using namespace SOP_FeE_PointGenPerElem_1_0Enums;
    switch (parmElementClass)
    {
    case ElementClass::PRIM:      return GA_GROUP_PRIMITIVE;  break;
    case ElementClass::POINT:     return GA_GROUP_POINT;      break;
    case ElementClass::EDGE:      return GA_GROUP_EDGE;       break;
    }
    UT_ASSERT_MSG(0, "Unhandled Class type!");
    return GA_GROUP_INVALID;
}





void
SOP_FeE_PointGenPerElem_1_0Verb::cook(const SOP_NodeVerb::CookParms &cookparms) const
{
    auto&& sopparms = cookparms.parms<SOP_FeE_PointGenPerElem_1_0Parms>();
    GA_Detail& outGeo0 = *cookparms.gdh().gdpNC();
    //auto sopcache = (SOP_FeE_PointGenPerElem_1_0Cache*)cookparms.cache();

    const GA_Detail& inGeo0 = *cookparms.inputGeo(0);

    //outGeo0.replaceWith(inGeo0);


    const GA_GroupType elementClass = sopGroupType(sopparms.getElementClass());
    const GA_GroupType groupType = sopGroupType(sopparms.getGroupType());

    UT_AutoInterrupt boss("Processing");
    if (boss.wasInterrupted())
        return;

/*
        GFE_PointGenPerElem pointGenPerElem(geoPoint, geo, cookparms);
        pointGenPerElem.elementClass = GA_GROUP_PRIMITIVE;
        pointGenPerElem.srcElemoffAttribName = "";
        pointGenPerElem.groupParser.setGroup(groupParser);
        pointGenPerElem.compute();
 */

    GFE_PointGenPerElem pointGenPerElem(outGeo0, inGeo0, cookparms);
    
    pointGenPerElem.setComputeParm(elementClass, sopparms.getSetPositionOnElem(), sopparms.getKeepSrcElemAttrib(), sopparms.getKeepSrcElemGroup(),
        sopparms.getSubscribeRatio(), sopparms.getMinGrainSize());

    if (sopparms.getUseNumPointAttrib())
        pointGenPerElem.setNumPointAttrib(sopparms.getNumPointAttrib());
    
    if (sopparms.getOutSrcElemoffAttrib())
    {
        pointGenPerElem.srcElemoffAttribName = sopparms.getSrcElemoffAttribName();
        pointGenPerElem.outAsOffset = sopparms.getOutAsOffset();
        //pointGenPerElem.setSrcElemoffAttrib(sopparms.getSrcElemoffAttribName());
    }
    
    pointGenPerElem.groupParser.setGroup(groupType, sopparms.getGroup());
    pointGenPerElem.computeAndBumpDataId();

    

}


