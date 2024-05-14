
//#define UT_ASSERT_LEVEL 3
#include "SOP_FeE_GroupReverse_1_0.h"


#include "SOP_FeE_GroupReverse_1_0.proto.h"

#include "GA/GA_Detail.h"
#include "PRM/PRM_TemplateBuilder.h"
#include "UT/UT_Interrupt.h"
#include "UT/UT_DSOVersion.h"


#include "GFE/GFE_Group.h"

using namespace SOP_FeE_GroupReverse_1_0_Namespace;



static const char *theDsFile = R"THEDSFILE(
{
    name        parameters
    
    parm {
        name    "doReverseGroup"
        cppname "DoReverseGroup"
        label   "Do Reverse Group"
        type    toggle
        default { 1 }
    }
    groupsimple {
        name    "reverseGroup_folder"
        label   "Reverse Group"
        disablewhentab "{ doReverseGroup == 0 }"

        parm {
            name    "reverseGroup"
            cppname "ReverseGroup"
            label   "Reverse Group"
            type    string
            default { "" }
            parmtag { "script_action" "import soputils\nkwargs['geometrytype'] = kwargs['node'].parmTuple('groupType')\nkwargs['inputindex'] = 0\nsoputils.selectGroupParm(kwargs)" }
            parmtag { "script_action_help" "Select geometry from an available viewport." }
            parmtag { "script_action_icon" "BUTTONS_reselect" }
        }
        parm {
            name    "reverseGroupType"
            cppname "ReverseGroupType"
            label   "Reverse Group Type"
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
    }
}
)THEDSFILE";



PRM_Template*
SOP_FeE_GroupReverse_1_0::buildTemplates()
{
    static PRM_TemplateBuilder templ("SOP_FeE_GroupReverse_1_0.C"_sh, theDsFile);
    if (templ.justBuilt())
    {
        templ.setChoiceListPtr("reverseGroup"_sh, &SOP_Node::allGroupMenu);
    }
    return templ.templates();
}


const UT_StringHolder SOP_FeE_GroupReverse_1_0::theSOPTypeName("FeE::groupReverse::1.0"_sh);

void
newSopOperator(OP_OperatorTable* table)
{
    OP_Operator* newOp = new OP_Operator(
        SOP_FeE_GroupReverse_1_0::theSOPTypeName,
        "FeE Group Reverse",
        SOP_FeE_GroupReverse_1_0::myConstructor,
        SOP_FeE_GroupReverse_1_0::buildTemplates(),
        1,
        1,
        nullptr,
        OP_FLAG_GENERATOR,
        nullptr,
        1,
        "Five elements Elf/Group");

    newOp->setIconName("SOP_groupcombine");
    table->addOperator(newOp);

}


//class SOP_FeE_GroupReverse_1_0Cache : public SOP_NodeCache
//{
//public:
//    SOP_FeE_GroupReverse_1_0Cache() : SOP_NodeCache(),
//        myPrevOutputDetailID(-1)
//    {}
//    ~SOP_FeE_GroupReverse_1_0Cache() override {}
//
//    int64 myPrevOutputDetailID;
//};


class SOP_FeE_GroupReverse_1_0Verb : public SOP_NodeVerb
{
public:
    SOP_FeE_GroupReverse_1_0Verb() {}
    virtual ~SOP_FeE_GroupReverse_1_0Verb() {}

    virtual SOP_NodeParms *allocParms() const { return new SOP_FeE_GroupReverse_1_0Parms(); }
    //virtual SOP_NodeCache* allocCache() const { return new SOP_FeE_GroupReverse_1_0Cache(); }
    virtual UT_StringHolder name() const { return SOP_FeE_GroupReverse_1_0::theSOPTypeName; }

    virtual CookMode cookMode(const SOP_NodeParms *parms) const { return COOK_GENERIC; }

    virtual void cook(const CookParms &cookparms) const;

    /// This static data member automatically registers
    /// this verb class at library ldir0d time.
    static const SOP_NodeVerb::Register<SOP_FeE_GroupReverse_1_0Verb> theVerb;
};

// The static member variable definition has to be outside the class definition.
// The declaration is inside the class.
const SOP_NodeVerb::Register<SOP_FeE_GroupReverse_1_0Verb> SOP_FeE_GroupReverse_1_0Verb::theVerb;

const SOP_NodeVerb *
SOP_FeE_GroupReverse_1_0::cookVerb() const 
{ 
    return SOP_FeE_GroupReverse_1_0Verb::theVerb.get();
}






static GA_GroupType
sopGroupType(SOP_FeE_GroupReverse_1_0Parms::ReverseGroupType parmGroupType)
{
    using namespace SOP_FeE_GroupReverse_1_0Enums;
    switch (parmGroupType)
    {
    case ReverseGroupType::GUESS:     return GA_GROUP_INVALID;    break;
    case ReverseGroupType::PRIM:      return GA_GROUP_PRIMITIVE;  break;
    case ReverseGroupType::POINT:     return GA_GROUP_POINT;      break;
    case ReverseGroupType::VERTEX:    return GA_GROUP_VERTEX;     break;
    case ReverseGroupType::EDGE:      return GA_GROUP_EDGE;       break;
    }
    UT_ASSERT_MSG(0, "Unhandled geo0Group type!");
    return GA_GROUP_INVALID;
}



void
SOP_FeE_GroupReverse_1_0Verb::cook(const SOP_NodeVerb::CookParms &cookparms) const
{
    auto &&sopparms = cookparms.parms<SOP_FeE_GroupReverse_1_0Parms>();
    GA_Detail& outGeo0 = *cookparms.gdh().gdpNC();
    //auto sopcache = (SOP_FeE_GroupReverse_1_0Cache*)cookparms.cache();

    const GA_Detail& inGeo0 = *cookparms.inputGeo(0);

    outGeo0.replaceWith(inGeo0);
    
    if (!sopparms.getDoReverseGroup())
        return;

    UT_AutoInterrupt boss("Processing");
    if (boss.wasInterrupted())
        return;

    const char* const groupName = sopparms.getReverseGroup().c_str();
    const GA_GroupType groupType = sopGroupType(sopparms.getReverseGroupType());
    //GFE_Group::groupToggle(outGeo0, groupType, sopparms.getGroup());
    
    bool doHighlight = true;
    for (GA_GroupTable::iterator<GA_Group> it = outGeo0.getGroupTable(groupType)->beginTraverse(); !it.atEnd(); ++it)
    {
        GA_Group& group = *it.group();
        if (group.isDetached() || group.isInternal() || !group.getName().multiMatch(groupName))
            continue;
        
#if 1
        if (groupType == GA_GROUP_EDGE)
        {
            auto& elemGroup = static_cast<GA_ElementGroup&>(group);
            GFE_Group::groupToggle(elemGroup);
            elemGroup.bumpDataId();
        }
        else
        {
            auto& elemGroup = static_cast<GA_EdgeGroup&>(group);
            GFE_Group::groupToggle(elemGroup);
            elemGroup.bumpDataId();
        }
#else
        GFE_Group::groupToggle(group);
        GFE_Group::groupBumpDataId(group);
#endif

        if (doHighlight)
        {
            cookparms.select(group);
            doHighlight = false;
        }
    }
}

