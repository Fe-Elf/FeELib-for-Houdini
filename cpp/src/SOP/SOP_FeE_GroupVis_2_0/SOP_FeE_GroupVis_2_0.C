
//#define UT_ASSERT_LEVEL 3
#include "SOP_FeE_GroupVis_2_0.h"


#include "SOP_FeE_GroupVis_2_0.proto.h"

#include "GA/GA_Detail.h"
#include "PRM/PRM_TemplateBuilder.h"
#include "UT/UT_Interrupt.h"
#include "UT/UT_DSOVersion.h"


#include "GFE/GFE_Group.h"

using namespace SOP_FeE_GroupVis_2_0_Namespace;



static const char *theDsFile = R"THEDSFILE(
{
    name        parameters
    
    parm {
        name    "visGroup"
        cppname "VisGroup"
        label   "Visualize Group"
        type    toggle
        default { 1 }
    }
    groupsimple {
        name    "visGroup_folder"
        cppname "VisGroup_folder"
        label   "Visualize Group"
        disablewhentab "{ visGroup == 0 }"

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
                "guess"       "Guess from Group"
                "prim"        "Primitive"
                "point"       "Point"
                "vertex"      "Vertex"
                "edge"        "Edge"
            }
        }
    }
}
)THEDSFILE";



PRM_Template*
SOP_FeE_GroupVis_2_0::buildTemplates()
{
    static PRM_TemplateBuilder templ("SOP_FeE_GroupVis_2_0.C"_sh, theDsFile);
    if (templ.justBuilt())
    {
        templ.setChoiceListPtr("group"_sh, &SOP_Node::allGroupMenu);
    }
    return templ.templates();
}


const UT_StringHolder SOP_FeE_GroupVis_2_0::theSOPTypeName("FeE::groupVis::2.0"_sh);

void
newSopOperator(OP_OperatorTable* table)
{
    OP_Operator* newOp = new OP_Operator(
        SOP_FeE_GroupVis_2_0::theSOPTypeName,
        "FeE Group Visualize",
        SOP_FeE_GroupVis_2_0::myConstructor,
        SOP_FeE_GroupVis_2_0::buildTemplates(),
        1,
        1,
        nullptr,
        OP_FLAG_GENERATOR,
        nullptr,
        1,
        "Five elements Elf/Group");

    newOp->setIconName("SOP_groupcreate");
    table->addOperator(newOp);
}


//class SOP_FeE_GroupVis_2_0Cache : public SOP_NodeCache
//{
//public:
//    SOP_FeE_GroupVis_2_0Cache() : SOP_NodeCache(),
//        myPrevOutputDetailID(-1)
//    {}
//    ~SOP_FeE_GroupVis_2_0Cache() override {}
//
//    int64 myPrevOutputDetailID;
//};


class SOP_FeE_GroupVis_2_0Verb : public SOP_NodeVerb
{
public:
    SOP_FeE_GroupVis_2_0Verb() {}
    virtual ~SOP_FeE_GroupVis_2_0Verb() {}

    virtual SOP_NodeParms *allocParms() const { return new SOP_FeE_GroupVis_2_0Parms(); }
    //virtual SOP_NodeCache* allocCache() const { return new SOP_FeE_GroupVis_2_0Cache(); }
    virtual UT_StringHolder name() const { return SOP_FeE_GroupVis_2_0::theSOPTypeName; }

    virtual CookMode cookMode(const SOP_NodeParms *parms) const { return COOK_INPLACE; }

    virtual void cook(const CookParms &cookparms) const;

    /// This static data member automatically registers
    /// this verb class at library ldir0d time.
    static const SOP_NodeVerb::Register<SOP_FeE_GroupVis_2_0Verb> theVerb;
};

// The static member variable definition has to be outside the class definition.
// The declaration is inside the class.
const SOP_NodeVerb::Register<SOP_FeE_GroupVis_2_0Verb> SOP_FeE_GroupVis_2_0Verb::theVerb;

const SOP_NodeVerb *
SOP_FeE_GroupVis_2_0::cookVerb() const 
{ 
    return SOP_FeE_GroupVis_2_0Verb::theVerb.get();
}






static GA_GroupType
sopGroupType(const SOP_FeE_GroupVis_2_0Parms::GroupType parmGroupType)
{
    using namespace SOP_FeE_GroupVis_2_0Enums;
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
SOP_FeE_GroupVis_2_0Verb::cook(const SOP_NodeVerb::CookParms &cookparms) const
{
    auto &&sopparms = cookparms.parms<SOP_FeE_GroupVis_2_0Parms>();
    GA_Detail& outGeo0 = *cookparms.gdh().gdpNC();
    //auto sopcache = (SOP_FeE_GroupVis_2_0Cache*)cookparms.cache();

    //const GA_Detail& inGeo0 = *cookparms.inputGeo(0);
    //outGeo0.replaceWith(inGeo0);


    const GA_GroupType groupType = sopGroupType(sopparms.getGroupType());
    const UT_StringHolder& groupName = sopparms.getGroup();
    
    UT_AutoInterrupt boss("Processing");
    if (boss.wasInterrupted())
        return;

    const GA_Group* geo0Group = nullptr;
    switch (groupType)
    {
    case GA_GROUP_INVALID:
        geo0Group = outGeo0.getGroupTable(GA_GROUP_PRIMITIVE)->find(groupName);
        if (!geo0Group)
        {
            geo0Group = outGeo0.getGroupTable(GA_GROUP_POINT)->find(groupName);
            if (!geo0Group)
            {
                geo0Group = outGeo0.getGroupTable(GA_GROUP_VERTEX)->find(groupName);
                if (!geo0Group)
                {
                    geo0Group = outGeo0.getGroupTable(GA_GROUP_EDGE)->find(groupName);
                }
            }
        }
    break;
    case GA_GROUP_PRIMITIVE:
    case GA_GROUP_POINT:
    case GA_GROUP_VERTEX:
    case GA_GROUP_EDGE:
        geo0Group = outGeo0.getGroupTable(groupType)->find(groupName);
    break;
    }

    if (!geo0Group)
        return;

    //if (geo0Group->classType() == GA_GROUP_EDGE)
    //{
    //    const GA_EdgeGroup* geo0EdgeGroup = static_cast<const GA_EdgeGroup*>(geo0Group);
    //    geo0EdgeGroup = geo0EdgeGroup;
    //}

    cookparms.select(*geo0Group);

}
