
//#define UT_ASSERT_LEVEL 3
#include "SOP_FeE_DelElement_1_0.h"

#include "SOP_FeE_DelElement_1_0.proto.h"

#include "GA/GA_Detail.h"
#include "PRM/PRM_TemplateBuilder.h"
#include "UT/UT_Interrupt.h"
#include "UT/UT_DSOVersion.h"


#include "GFE/GFE_DeleteElement.h"



using namespace SOP_FeE_DelElement_1_0_Namespace;


static const char *theDsFile = R"THEDSFILE(
{
    name	parameters
    parm {
        name    "delElement"
        cppname "DelElement"
        label   "Delete Element"
        type    toggle
        default { "on" }
    }
    groupsimple {
        name    "delElement_folder"
        label   "Delete Element"
        disablewhen "{ delElement == 0 }"
        grouptag { "group_type" "simple" }

        parm {
            name    "delElementGroup"
            cppname "DelElementGroup"
            label   "Group"
            type    string
            default { "" }
            parmtag { "script_action" "import soputils\nkwargs['geometrytype'] = kwargs['node'].parmTuple('delElementGroupType')\nkwargs['inputindex'] = 0\nsoputils.selectGroupParm(kwargs)" }
            parmtag { "script_action_help" "Select geometry from an available viewport.\nShift-click to turn on Select Groups." }
            parmtag { "script_action_icon" "BUTTONS_reselect" }
            parmtag { "sop_input" "0" }
        }
        parm {
            name    "delElementGroupType"
            cppname "DelElementGroupType"
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
            name    "delElementMethod"
            cppname "DelElementMethod"
            label   "Delete Element Method"
            type    ordinal
            default { "auto" }
            menu {
                "delete"     "Delete"
                "leave"      "Leave"
                "auto"       "Auto"
            }
        }
        parm {
            name    "delElementReverseGroup"
            cppname "DelElementReverseGroup"
            label   "Delete Non Selected"
            type    toggle
            default { "off" }
        }
        parm {
            name    "delElementInputGroup"
            cppname "DelElementInputGroup"
            label   "Delete Group"
            type    toggle
            default { "on" }
        }


        parm {
            name    "delElementUnusedPoint"
            cppname "DelElementUnusedPoint"
            label   "Delete Unused Point"
            type    toggle
            default { 1 }
        }
        parm {
            name    "delElementPointMode"
            cppname "DelElementPointMode"
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
            name    "delElementGuaranteeNoVertexReference"
            cppname "DelElementGuaranteeNoVertexReference"
            label   "Guarantee No Vertex Reference"
            type    toggle
            default { 0 }
        }
    }
}
)THEDSFILE";

PRM_Template*
SOP_FeE_DelElement_1_0::buildTemplates()
{
    static PRM_TemplateBuilder templ("SOP_FeE_DelElement_1_0.C"_sh, theDsFile);
    if (templ.justBuilt())
    {
        templ.setChoiceListPtr("delElementGroup"_sh, &SOP_Node::allGroupMenu);
    }
    return templ.templates();
}





const UT_StringHolder SOP_FeE_DelElement_1_0::theSOPTypeName("FeE::delElement::1.0"_sh);

void
newSopOperator(OP_OperatorTable* table)
{
    OP_Operator* newOp = new OP_Operator(
        SOP_FeE_DelElement_1_0::theSOPTypeName,
        "FeE Delete Element",
        SOP_FeE_DelElement_1_0::myConstructor,
        SOP_FeE_DelElement_1_0::buildTemplates(),
        1,
        1,
        nullptr,
        OP_FLAG_GENERATOR,
        nullptr,
        1,
        "Five elements Elf/Operation/Delete");

    newOp->setIconName("SOP_delete");
    table->addOperator(newOp);
}




class SOP_FeE_DelElement_1_0Verb : public SOP_NodeVerb
{
public:
    SOP_FeE_DelElement_1_0Verb() {}
    virtual ~SOP_FeE_DelElement_1_0Verb() {}

    virtual SOP_NodeParms *allocParms() const { return new SOP_FeE_DelElement_1_0Parms(); }
    virtual UT_StringHolder name() const { return SOP_FeE_DelElement_1_0::theSOPTypeName; }

    virtual CookMode cookMode(const SOP_NodeParms *parms) const
    {
        const auto* sopparms = reinterpret_cast<const SOP_FeE_DelElement_1_0Parms*>(parms);
        return sopparms->getDelElement() ? COOK_GENERATOR : COOK_PASSTHROUGH;
    }

    virtual void cook(const CookParms &cookparms) const;
    
    static const SOP_NodeVerb::Register<SOP_FeE_DelElement_1_0Verb> theVerb;
};

const SOP_NodeVerb::Register<SOP_FeE_DelElement_1_0Verb> SOP_FeE_DelElement_1_0Verb::theVerb;

const SOP_NodeVerb *
SOP_FeE_DelElement_1_0::cookVerb() const 
{ 
    return SOP_FeE_DelElement_1_0Verb::theVerb.get();
}









static GA_GroupType
sopGroupType(const SOP_FeE_DelElement_1_0Parms::DelElementGroupType parmGroupType)
{
    using namespace SOP_FeE_DelElement_1_0Enums;
    switch (parmGroupType)
    {
    case DelElementGroupType::GUESS:     return GA_GROUP_INVALID;    break;
    case DelElementGroupType::PRIM:      return GA_GROUP_PRIMITIVE;  break;
    case DelElementGroupType::POINT:     return GA_GROUP_POINT;      break;
    case DelElementGroupType::VERTEX:    return GA_GROUP_VERTEX;     break;
    case DelElementGroupType::EDGE:      return GA_GROUP_EDGE;       break;
    }
    UT_ASSERT_MSG(0, "Unhandled geo0Group type!");
    return GA_GROUP_INVALID;
}

static GA_Detail::GA_DestroyPointMode
sopDelPointMode(const SOP_FeE_DelElement_1_0Parms::DelElementPointMode parmDelPointMode)
{
    using namespace SOP_FeE_DelElement_1_0Enums;
    switch (parmDelPointMode)
    {
    case DelElementPointMode::LEAVEPRIMITIVE:              return GA_Detail::GA_DestroyPointMode::GA_LEAVE_PRIMITIVES;                 break;
    case DelElementPointMode::DELDEGENERATE:               return GA_Detail::GA_DestroyPointMode::GA_DESTROY_DEGENERATE;               break;
    case DelElementPointMode::DELDEGENERATEINCOMPATIBLE:   return GA_Detail::GA_DestroyPointMode::GA_DESTROY_DEGENERATE_INCOMPATIBLE;  break;
    }
    UT_ASSERT_MSG(0, "Unhandled Delete Point Mode!");
    return GA_Detail::GA_DestroyPointMode::GA_DESTROY_DEGENERATE_INCOMPATIBLE;
}

static GFE_DelElementMethod
sopDelElementMethod(const SOP_FeE_DelElement_1_0Parms::DelElementMethod parmDelPointMode)
{
    using namespace SOP_FeE_DelElement_1_0Enums;
    switch (parmDelPointMode)
    {
    case DelElementMethod::DELETE: return GFE_DelElementMethod::Delete;    break;
    case DelElementMethod::LEAVE:  return GFE_DelElementMethod::Leave;     break;
    case DelElementMethod::AUTO:   return GFE_DelElementMethod::Auto;      break;
    }
    UT_ASSERT_MSG(0, "Unhandled Delete Point Mode!");
    return GFE_DelElementMethod::Delete;
}

void
SOP_FeE_DelElement_1_0Verb::cook(const SOP_NodeVerb::CookParms& cookparms) const
{
    auto&& sopparms = cookparms.parms<SOP_FeE_DelElement_1_0Parms>();
    GA_Detail& outGeo0 = *cookparms.gdh().gdpNC();
    //auto sopcache = (SOP_FeE_DelElement_1_0Cache*)cookparms.cache();

    const GA_Detail& inGeo0 = *cookparms.inputGeo(0);

    //if (!sopparms.getDelElement())
    //{
    //    outGeo0.replaceWith(inGeo0);
    //    return;
    //}
    
    outGeo0.clear();
    
    const GFE_DelElementMethod delElementMethod = sopDelElementMethod(sopparms.getDelElementMethod());
    const GA_GroupType groupType = sopGroupType(sopparms.getDelElementGroupType());
    const GA_Detail::GA_DestroyPointMode delPointMode = sopDelPointMode(sopparms.getDelElementPointMode());
    
    UT_AutoInterrupt boss("Processing");
    if (boss.wasInterrupted())
        return;



    GFE_DelElement delElement(outGeo0, inGeo0, cookparms);
    
    delElement.groupParser.setGroup(groupType, sopparms.getDelElementGroup());

    delElement.setComputeParm(
        delElementMethod,
        sopparms.getDelElementReverseGroup(),
        sopparms.getDelElementInputGroup(),
        delPointMode,
        sopparms.getDelElementUnusedPoint(),
        sopparms.getDelElementGuaranteeNoVertexReference());

    delElement.computeAndBumpDataIdsForAddOrRemove();

}


