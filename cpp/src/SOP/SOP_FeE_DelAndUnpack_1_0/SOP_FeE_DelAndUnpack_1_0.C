
//#define UT_ASSERT_LEVEL 3
#include "SOP_FeE_DelAndUnpack_1_0.h"


#include "SOP_FeE_DelAndUnpack_1_0.proto.h"

#include "GA/GA_Detail.h"
#include "PRM/PRM_TemplateBuilder.h"
#include "UT/UT_Interrupt.h"
#include "UT/UT_DSOVersion.h"


#include "GFE/GFE_DeleteAndUnpack.h"



using namespace SOP_FeE_DelAndUnpack_1_0_Namespace;


static const char *theDsFile = R"THEDSFILE(
{
    name	parameters
    parm {
        name    "elemTraversingMethod"
        cppname "ElemTraversingMethod"
        label   "Elem Traversing Method"
        type    ordinal
        default { "oneElem" }
        menu {
            "custom"    "Custom"
            "oneElem"   "One Elem"
            "skipNElem" "Skip N Elem"
        }
    }
    parm {
        name    "group"
        cppname "Group"
        label   "Group"
        type    string
        default { "" }
        hidewhen "{ elemTraversingMethod != custom }"
        parmtag { "script_action" "import soputils\nkwargs['geometrytype'] = kwargs['node'].parmTuple('groupType')\nkwargs['inputindex'] = 0\nsoputils.selectGroupParm(kwargs)" }
        parmtag { "script_action_help" "Select geometry from an available viewport.\nShift-click to turn on Select Groups." }
        parmtag { "script_action_icon" "BUTTONS_reselect" }
    }
    parm {
        name    "groupType"
        cppname "GroupType"
        label   "Group Type"
        type    ordinal
        default { "prim" }
        menu {
            "guess"     "Guess from Group"
            "prim"      "Primitive"
            "point"     "Point"
            "vertex"    "Vertex"
            "edge"      "Edge"
        }
    }
    parm {
        name    "primoff"
        cppname "Primoff"
        label   "Primoff"
        type    integer
        default { "0" }
        hidewhen "{ elemTraversingMethod == custom }"
        range   { 0! 10 }
    }
    parm {
        name    "inputAsOffset"
        cppname "InputAsOffset"
        label   "Input as Offset"
        type    toggle
        default { "0" }
        hidewhen "{ elemTraversingMethod == custom }"
    }
    parm {
        name    "skipNPrim"
        cppname "SkipNPrim"
        label   "Skip N Prim"
        type    integer
        default { "1" }
        hidewhen "{ elemTraversingMethod != skipNElem }"
        range   { 0! 10 }
    }
    parm {
        name    "reverseGroup"
        cppname "ReverseGroup"
        label   "Delete Non Selected"
        type    toggle
        default { "1" }
    }
    parm {
        name    "delGroup"
        cppname "DelGroup"
        label   "Delete Group"
        type    toggle
        default { "1" }
    }

}
)THEDSFILE";

PRM_Template*
SOP_FeE_DelAndUnpack_1_0::buildTemplates()
{
    static PRM_TemplateBuilder templ("SOP_FeE_DelAndUnpack_1_0.C"_sh, theDsFile);
    if (templ.justBuilt())
    {
        templ.setChoiceListPtr("group"_sh, &SOP_Node::allGroupMenu);
    }
    return templ.templates();
}





const UT_StringHolder SOP_FeE_DelAndUnpack_1_0::theSOPTypeName("FeE::delAndUnpack::1.0"_sh);

void
newSopOperator(OP_OperatorTable* table)
{
    OP_Operator* newOp = new OP_Operator(
        SOP_FeE_DelAndUnpack_1_0::theSOPTypeName,
        "FeE Delete and Unpack",
        SOP_FeE_DelAndUnpack_1_0::myConstructor,
        SOP_FeE_DelAndUnpack_1_0::buildTemplates(),
        1,
        1,
        nullptr,
        OP_FLAG_GENERATOR,
        nullptr,
        1,
        "Five elements Elf/Operation/Blast");

    newOp->setIconName("SOP_delete");
    table->addOperator(newOp);
}




class SOP_FeE_DelAndUnpack_1_0Verb : public SOP_NodeVerb
{
public:
    SOP_FeE_DelAndUnpack_1_0Verb() {}
    virtual ~SOP_FeE_DelAndUnpack_1_0Verb() {}

    virtual SOP_NodeParms *allocParms() const { return new SOP_FeE_DelAndUnpack_1_0Parms(); }
    virtual UT_StringHolder name() const { return SOP_FeE_DelAndUnpack_1_0::theSOPTypeName; }

    virtual CookMode cookMode(const SOP_NodeParms *parms) const { return COOK_GENERATOR; }

    virtual void cook(const CookParms &cookparms) const;
    
    static const SOP_NodeVerb::Register<SOP_FeE_DelAndUnpack_1_0Verb> theVerb;
};

const SOP_NodeVerb::Register<SOP_FeE_DelAndUnpack_1_0Verb> SOP_FeE_DelAndUnpack_1_0Verb::theVerb;

const SOP_NodeVerb *
SOP_FeE_DelAndUnpack_1_0::cookVerb() const 
{ 
    return SOP_FeE_DelAndUnpack_1_0Verb::theVerb.get();
}









static GA_GroupType
sopGroupType(const SOP_FeE_DelAndUnpack_1_0Parms::GroupType parmGroupType)
{
    using namespace SOP_FeE_DelAndUnpack_1_0Enums;
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

static GFE_ElemTraversingMethod
sopElemTraversingMethod(const SOP_FeE_DelAndUnpack_1_0Parms::ElemTraversingMethod parmElemTraversingMethod)
{
    using namespace SOP_FeE_DelAndUnpack_1_0Enums;
    switch (parmElemTraversingMethod)
    {
    case ElemTraversingMethod::CUSTOM:           return GFE_ElemTraversingMethod::Custom;     break;
    case ElemTraversingMethod::ONEELEM:          return GFE_ElemTraversingMethod::OneElem;    break;
    case ElemTraversingMethod::SKIPNELEM:        return GFE_ElemTraversingMethod::SkipNElem;  break;
    }
    UT_ASSERT_MSG(0, "Unhandled Delete Type!");
    return GFE_ElemTraversingMethod::OneElem;
}

void
SOP_FeE_DelAndUnpack_1_0Verb::cook(const SOP_NodeVerb::CookParms& cookparms) const
{
    auto&& sopparms = cookparms.parms<SOP_FeE_DelAndUnpack_1_0Parms>();
    GA_Detail& outGeo0 = *cookparms.gdh().gdpNC();
    //auto sopcache = (SOP_FeE_DelAndUnpack_1_0Cache*)cookparms.cache();

    const GA_Detail& inGeo0 = *cookparms.inputGeo(0);
    //outGeo0.replaceWith(inGeo0);


    const GA_GroupType groupType = sopGroupType(sopparms.getGroupType());
    const GFE_ElemTraversingMethod elemTraversingMethod = sopElemTraversingMethod(sopparms.getElemTraversingMethod());
    
    UT_AutoInterrupt boss("Processing");
    if (boss.wasInterrupted())
        return;



    // Replace by GFE_UnpackByGroup Currently
    GFE_DelAndUnpack delAndUnpack(outGeo0, inGeo0, cookparms);
    
    delAndUnpack.setComputeParm(elemTraversingMethod, sopparms.getReverseGroup(), sopparms.getDelGroup());
    
    GA_Offset primoff = sopparms.getPrimoff();
    if(sopparms.getInputAsOffset())
        primoff = outGeo0.primitiveOffset(primoff);

    
    switch (elemTraversingMethod) {
    case GFE_ElemTraversingMethod::Custom:    delAndUnpack.groupParser.setGroup(groupType, sopparms.getGroup());  break;
    case GFE_ElemTraversingMethod::OneElem:   delAndUnpack.setPrimoff(primoff);                                   break;
    case GFE_ElemTraversingMethod::SkipNElem: delAndUnpack.setSkipNPrim(primoff, sopparms.getSkipNPrim());        break;
    default: break;
    }
    

    delAndUnpack.computeAndBumpDataIdsForAddOrRemove();
    
}

