
//#define UT_ASSERT_LEVEL 3
#include "SOP_FeE_EnumCombine_1_0.h"


#include "SOP_FeE_EnumCombine_1_0.proto.h"

#include "GA/GA_Detail.h"
#include "PRM/PRM_TemplateBuilder.h"
#include "UT/UT_Interrupt.h"
#include "UT/UT_DSOVersion.h"




#include "GFE/GFE_EnumerateCombine.h"

using namespace SOP_FeE_EnumCombine_1_0_Namespace;


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
        name    "class"
        cppname "Class"
        label   "Class"
        type    ordinal
        default { "prim" }
        menu {
            "prim"      "Primitive"
            "point"     "Point"
            "vertex"    "Vertex"
        }
    }
    parm {
        name    "enumAttrib"
        cppname "EnumAttrib"
        label   "Enum Attrib"
        type    string
        default { "index" }
    }
    parm {
        name    "outAttribName"
        cppname "OutAttribName"
        label   "Out Attrib Name"
        type    string
        default { "index" }
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
SOP_FeE_EnumCombine_1_0::buildTemplates()
{
    static PRM_TemplateBuilder templ("SOP_FeE_EnumCombine_1_0.C"_sh, theDsFile);
    if (templ.justBuilt())
    {
        templ.setChoiceListPtr("group"_sh, &SOP_Node::groupMenu);
    }
    return templ.templates();
}

const UT_StringHolder SOP_FeE_EnumCombine_1_0::theSOPTypeName("FeE::enumCombine::1.0"_sh);

void
newSopOperator(OP_OperatorTable* table)
{
    OP_Operator* newOp = new OP_Operator(
        SOP_FeE_EnumCombine_1_0::theSOPTypeName,
        "FeE Enumerate Combine",
        SOP_FeE_EnumCombine_1_0::myConstructor,
        SOP_FeE_EnumCombine_1_0::buildTemplates(),
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





class SOP_FeE_EnumCombine_1_0Verb : public SOP_NodeVerb
{
public:
    SOP_FeE_EnumCombine_1_0Verb() {}
    virtual ~SOP_FeE_EnumCombine_1_0Verb() {}

    virtual SOP_NodeParms *allocParms() const { return new SOP_FeE_EnumCombine_1_0Parms(); }
    virtual UT_StringHolder name() const { return SOP_FeE_EnumCombine_1_0::theSOPTypeName; }

    virtual CookMode cookMode(const SOP_NodeParms *parms) const { return COOK_INPLACE; }

    virtual void cook(const CookParms &cookparms) const;
    
    /// This static data member automatically registers
    /// this verb class at library load time.
    static const SOP_NodeVerb::Register<SOP_FeE_EnumCombine_1_0Verb> theVerb;
};

// The static member variable definition has to be outside the class definition.
// The declaration is inside the class.
const SOP_NodeVerb::Register<SOP_FeE_EnumCombine_1_0Verb> SOP_FeE_EnumCombine_1_0Verb::theVerb;

const SOP_NodeVerb *
SOP_FeE_EnumCombine_1_0::cookVerb() const 
{ 
    return SOP_FeE_EnumCombine_1_0Verb::theVerb.get();
}





static GA_GroupType
sopGroupType(const SOP_FeE_EnumCombine_1_0Parms::GroupType parmGroupType)
{
    using namespace SOP_FeE_EnumCombine_1_0Enums;
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

static GA_AttributeOwner
sopAttribOwner(const SOP_FeE_EnumCombine_1_0Parms::Class parmAttribClass)
{
    using namespace SOP_FeE_EnumCombine_1_0Enums;
    switch (parmAttribClass)
    {
    case Class::PRIM:      return GA_ATTRIB_PRIMITIVE;  break;
    case Class::POINT:     return GA_ATTRIB_POINT;      break;
    case Class::VERTEX:    return GA_ATTRIB_VERTEX;     break;
    }
    UT_ASSERT_MSG(0, "Unhandled Class type!");
    return GA_ATTRIB_INVALID;
}


void
SOP_FeE_EnumCombine_1_0Verb::cook(const SOP_NodeVerb::CookParms &cookparms) const
{
    auto&& sopparms = cookparms.parms<SOP_FeE_EnumCombine_1_0Parms>();
    GA_Detail& outGeo0 = *cookparms.gdh().gdpNC();
    //auto sopcache = (SOP_FeE_EnumCombine_1_0Cache*)cookparms.cache();
    //const GA_Detail& inGeo0 = *cookparms.inputGeo(0);
    //outGeo0.replaceWith(inGeo0);



    const GA_AttributeOwner attribClass = sopAttribOwner(sopparms.getClass());
    const GA_GroupType groupType = sopGroupType(sopparms.getGroupType());


    UT_AutoInterrupt boss("Processing");
    if (boss.wasInterrupted())
        return;
    
    
    GFE_EnumCombine enumerateCombine(outGeo0, cookparms);
    enumerateCombine.setComputeParm(sopparms.getSubscribeRatio(), sopparms.getMinGrainSize());
    
    enumerateCombine.groupParser.setGroup(groupType, sopparms.getGroup());
    enumerateCombine.getInAttribArray().appends(attribClass, sopparms.getEnumAttrib());
    
    enumerateCombine.getOutAttribArray().findOrCreateTuple(false, attribClass, GA_STORECLASS_INT, GA_STORE_INVALID, sopparms.getAttribName());

    enumerateCombine.computeAndBumpDataId();
    
}


