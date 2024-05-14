//#define UT_ASSERT_LEVEL 3
#include "SOP_FeE_RestVectorComponent_1_0.h"


#include "SOP_FeE_RestVectorComponent_1_0.proto.h"

#include "GA/GA_Detail.h"
#include "PRM/PRM_TemplateBuilder.h"
#include "UT/UT_Interrupt.h"
#include "UT/UT_DSOVersion.h"


#include "GFE/GFE_RestVectorComponent.h"


using namespace SOP_FeE_RestVectorComponent_1_0_Namespace;


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
        default { "P" }
    }
    // parm {
    //     name    "useRefAttrib"
    //     cppname "UseRefAttrib"
    //     label   "Use Ref Attrib"
    //     type    toggle
    //     default { 0 }
    //     nolabel
    //     joinnext
    // }
    // parm {
    //     name    "refAttrib"
    //     cppname "RefAttrib"
    //     label   "Ref Attrib"
    //     type    string
    //     default { "P" }
    //     disablewhen "{ useRefAttrib == 0 }"
    // }
    parm {
        name    "comp"
        cppname "Comp"
        label   "Component"
        type    integer
        default { 1 }
        range   { 0! 4! }
    }

    parm {
        name    "renameAttrib"
        cppname "RenameAttrib"
        label   "Rename Attrib"
        type    toggle
        default { 0 }
        nolabel
        joinnext
    }
    parm {
        name    "restAttribName"
        cppname "RestAttribName"
        label   "Rest Attribute Name"
        type    string
        default { "" }
        disablewhen "{ renameAttrib == 0 }"
    }

    parm {
        name    "delOriginAttrib"
        cppname "DelOriginAttrib"
        label   "Delete Origin Attrib"
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
        default { 64 }
        range   { 0! 2048 }
    }
}
)THEDSFILE";

PRM_Template*
SOP_FeE_RestVectorComponent_1_0::buildTemplates()
{
    static PRM_TemplateBuilder templ("SOP_FeE_RestVectorComponent_1_0.C"_sh, theDsFile);
    if (templ.justBuilt())
    {
        //templ.setChoiceListPtr("group"_sh, &SOP_Node::groupMenu);
        templ.setChoiceListPtr("group"_sh, &SOP_Node::pointGroupMenu);
    }
    return templ.templates();
}

const UT_StringHolder SOP_FeE_RestVectorComponent_1_0::theSOPTypeName("FeE::restVectorComponent::1.0"_sh);

void
newSopOperator(OP_OperatorTable* table)
{
    OP_Operator* newOp = new OP_Operator(
        SOP_FeE_RestVectorComponent_1_0::theSOPTypeName,
        "FeE Rest Vector Component",
        SOP_FeE_RestVectorComponent_1_0::myConstructor,
        SOP_FeE_RestVectorComponent_1_0::buildTemplates(),
        1,
        2,
        nullptr,
        OP_FLAG_GENERATOR,
        nullptr,
        1,
        "Five elements Elf/Data");

    newOp->setIconName("SOP_rest");
    table->addOperator(newOp);
}


class SOP_FeE_RestVectorComponent_1_0Verb : public SOP_NodeVerb
{
public:
    SOP_FeE_RestVectorComponent_1_0Verb()
    {
    }

    virtual ~SOP_FeE_RestVectorComponent_1_0Verb()
    {
    }

    virtual SOP_NodeParms* allocParms() const { return new SOP_FeE_RestVectorComponent_1_0Parms(); }
    virtual UT_StringHolder name() const { return SOP_FeE_RestVectorComponent_1_0::theSOPTypeName; }

    virtual CookMode cookMode(const SOP_NodeParms* parms) const { return COOK_INPLACE; }

    virtual void cook(const CookParms& cookparms) const;

    /// This static data member automatically registers
    /// this verb class at library load time.
    static const SOP_NodeVerb::Register<SOP_FeE_RestVectorComponent_1_0Verb> theVerb;
};

// The static member variable definition has to be outside the class definition.
// The declaration is inside the class.
const SOP_NodeVerb::Register<SOP_FeE_RestVectorComponent_1_0Verb> SOP_FeE_RestVectorComponent_1_0Verb::theVerb;

const SOP_NodeVerb*
SOP_FeE_RestVectorComponent_1_0::cookVerb() const
{
    return SOP_FeE_RestVectorComponent_1_0Verb::theVerb.get();
}


static GA_AttributeOwner
sopAttribOwner(const SOP_FeE_RestVectorComponent_1_0Parms::AttribClass parmAttribClass)
{
    using namespace SOP_FeE_RestVectorComponent_1_0Enums;
    switch (parmAttribClass)
    {
    case AttribClass::PRIM:   return GA_ATTRIB_PRIMITIVE; break;
    case AttribClass::POINT:  return GA_ATTRIB_POINT;     break;
    case AttribClass::VERTEX: return GA_ATTRIB_VERTEX;    break;
    case AttribClass::DETAIL: return GA_ATTRIB_DETAIL;    break;
    }
    UT_ASSERT_MSG(0, "Unhandled Geo0 Class type!");
    return GA_ATTRIB_INVALID;
}

static GA_GroupType
sopGroupType(const SOP_FeE_RestVectorComponent_1_0Parms::GroupType parmGroupType)
{
    using namespace SOP_FeE_RestVectorComponent_1_0Enums;
    switch (parmGroupType)
    {
    case GroupType::GUESS:  return GA_GROUP_INVALID;   break;
    case GroupType::PRIM:   return GA_GROUP_PRIMITIVE; break;
    case GroupType::POINT:  return GA_GROUP_POINT;     break;
    case GroupType::VERTEX: return GA_GROUP_VERTEX;    break;
    case GroupType::EDGE:   return GA_GROUP_EDGE;      break;
    }
    UT_ASSERT_MSG(0, "Unhandled Group Type!");
    return GA_GROUP_INVALID;
}


void
SOP_FeE_RestVectorComponent_1_0Verb::cook(const SOP_NodeVerb::CookParms& cookparms) const
{
    auto&& sopparms = cookparms.parms<SOP_FeE_RestVectorComponent_1_0Parms>();
    GA_Detail& outGeo0 = *cookparms.gdh().gdpNC();
    //auto sopcache = (SOP_FeE_RestVectorComponent_1_0Cache*)cookparms.cache();

    //const GA_Detail& inGeo0 = *cookparms.inputGeo(0);
    const GA_Detail* const inGeo1 = cookparms.inputGeo(1);

    //outGeo0.replaceWith(inGeo0);

    const GA_AttributeOwner geo0AttribClass = sopAttribOwner(sopparms.getAttribClass());

    const GA_GroupType groupType = sopGroupType(sopparms.getGroupType());

    UT_AutoInterrupt boss("Processing");
    if (boss.wasInterrupted())
        return;

/*
            GFE_RestVectorComponent restVectorComponent(geo, nullptr, cookparms);
            restVectorComponent.comp = comp;
            restVectorComponent.setRestAttrib(posAttribNonConst);
            restVectorComponent.newAttribNames = "";
            restVectorComponent.compute();
*/
    
    GFE_RestVectorComponent restVectorComponent(outGeo0, inGeo1, &cookparms);

    restVectorComponent.groupParser.setGroup(groupType, sopparms.getGroup());

    restVectorComponent.setComputeParm(static_cast<int8>(sopparms.getComp()), sopparms.getDelOriginAttrib(),
                                      sopparms.getSubscribeRatio(), sopparms.getMinGrainSize());
    if (sopparms.getRenameAttrib())
        restVectorComponent.newAttribNames = sopparms.getRestAttribName();
    
    restVectorComponent.setRestAttrib(geo0AttribClass, sopparms.getAttrib());
    restVectorComponent.computeAndBumpDataId();

    
}
