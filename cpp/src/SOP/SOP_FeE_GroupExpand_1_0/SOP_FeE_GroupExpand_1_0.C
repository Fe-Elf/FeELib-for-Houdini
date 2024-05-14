
//#define UT_ASSERT_LEVEL 3
#include "SOP_FeE_GroupExpand_1_0.h"

#include "SOP_FeE_GroupExpand_1_0.proto.h"


#include "GA/GA_Detail.h"
#include "PRM/PRM_TemplateBuilder.h"
#include "UT/UT_Interrupt.h"
#include "UT/UT_DSOVersion.h"



#include "GFE/GFE_GroupExpand.h"


using namespace SOP_FeE_GroupExpand_1_0_Namespace;


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
        name    "outExpandGroup"
        cppname "OutExpandGroup"
        label   "Output Expand Group"
        type    toggle
        default { "0" }
        joinnext
        nolabel
    }
    parm {
        name    "expandGroupName"
        cppname "ExpandGroupName"
        label   "Expand Group Name"
        type    string
        default { "expand" }
        disablewhen "{ outExpandGroup == 0 }"
    }
    parm {
        name    "outBorderGroup"
        cppname "OutBorderGroup"
        label   "Output Border Group"
        type    toggle
        default { "0" }
        joinnext
        nolabel
    }
    parm {
        name    "borderGroupName"
        cppname "BorderGroupName"
        label   "Border Group Name"
        type    string
        default { "border" }
        disablewhen "{ outBorderGroup == 0 }"
    }
    // parm {
    //     name    "prevBorderGroupName"
    //     cppname "PrevBorderGroupName" 
    //     label   "Prev Border Group Name"
    //     type    string
    //     default { "prevBorder" }
    // }

    parm {
        name    "primshareedge"
        label   "Require Primitives Share Edge"
        type    toggle
        default { "0" }
        disablewhen "{ groupType == vertex } { groupType == edge } { groupType == point }"
    }
    parm {
        name    "floodfill"
        label   "Flood Fill to Connected Geometry"
        type    toggle
        default { "0" }
    }
    parm {
        name    "numsteps"
        label   "Steps"
        type    integer
        default { "1" }
        disablewhen "{ floodfill == 1 }"
        range   { -10 10 }
    }
    parm {
        name    "usestepattrib"
        label   "usestepattrib"
        type    toggle
        nolabel
        joinnext
        default { "0" }
    }
    parm {
        name    "stepattrib"
        label   "Step Attribute"
        type    string
        default { "step" }
        disablewhen "{ usestepattrib == 0 }"
    }
    groupsimple {
        name    "constraint_folder"
        label   "Constraints"
        grouptag { "group_type" "simple" }

        groupcollapsible {
            name    "normalfolder"
            label   "Normal Constraints"
            grouptag { "group_type" "collapsible" }

            parm {
                name    "bynormal"
                label   "bynormal"
                type    toggle
                nolabel
                joinnext
                default { "0" }
            }
            parm {
                name    "normalangle"
                label   "Restrict by Normal Spread Angle"
                type    float
                default { "60" }
                disablewhen "{ bynormal == 0 }"
                range   { 0 180 }
            }
            parm {
                name    "overridenormal"
                label   "overridenormal"
                type    toggle
                nolabel
                joinnext
                default { "0" }
                disablewhen "{ bynormal == 0 }"
            }
            parm {
                name    "normalattrib"
                label   "Normal Attribute"
                type    string
                default { "N" }
                disablewhen "{ bynormal == 0 } { overridenormal == 0 }"
            }
        }

        groupcollapsible {
            name    "connectivityfolder"
            label   "Connectivity Constraints"
            grouptag { "group_type" "collapsible" }

            parm {
                name    "useconnectivityattrib"
                label   "useconnectivityattrib"
                type    toggle
                nolabel
                joinnext
                default { "0" }
            }
            parm {
                name    "connectivityattrib"
                label   "Connectivity Attribute"
                type    string
                default { "uv" }
                disablewhen "{ useconnectivityattrib == 0 }"
            }
            parm {
                name    "usecolgroup"
                label   "usecolgroup"
                type    toggle
                nolabel
                joinnext
                default { "0" }
            }
            parm {
                name    "colgroup"
                label   "Collision Group"
                type    string
                default { "" }
                disablewhen "{ usecolgroup == 0 }"
                parmtag { "script_action" "import soputils\nkwargs['geometrytype'] = kwargs['node'].parmTuple('colgrouptype')\nkwargs['inputindex'] = 0\nsoputils.selectGroupParm(kwargs)" }
                parmtag { "script_action_help" "Select geometry from an available viewport.\nShift-click to turn on Select Groups." }
                parmtag { "script_action_icon" "BUTTONS_reselect" }
            }
            parm {
                name    "colgrouptype"
                label   "Collision Group Type"
                type    ordinal
                default { "0" }
                disablewhen "{ usecolgroup == 0 }"
                menu {
                    "auto"      "Guess from Group"
                    "edges"     "Edges"
                    "points"    "Points"
                    "prims"     "Primitives"
                    "vertices"  "Vertices"
                }
            }
            parm {
                name    "colgroupinvert"
                label   "Contain Growth within Collision Group"
                type    toggle
                default { "0" }
                disablewhen "{ usecolgroup == 0 }"
            }
            parm {
                name    "colgroupallowonbound"
                label   "Allow Growth to Collision Group Boundary"
                type    toggle
                default { "0" }
                disablewhen "{ usecolgroup == 0 } { colgroupinvert == 1 }"
            }
        }

    }



    parm {
        name    "largeConnectivity"
        cppname "LargeConnectivity"
        label   "Large Connectivity"
        type    toggle
        default { "0" }
    }


    parm {
        name    "outTopoAttrib"
        cppname "OutTopoAttrib"
        label   "Output Topo Attribute"
        type    toggle
        default { "0" }
    }

    parm {
       name    "subscribeRatio"
       cppname "SubscribeRatio"
       label   "Subscribe Ratio"
       type    integer
       default { 16 }
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
SOP_FeE_GroupExpand_1_0::buildTemplates()
{
    static PRM_TemplateBuilder templ("SOP_FeE_GroupExpand_1_0.C"_sh, theDsFile);
    if (templ.justBuilt())
    {
        templ.setChoiceListPtr("group"_sh, &SOP_Node::allGroupMenu);
    }
    return templ.templates();
}


const UT_StringHolder SOP_FeE_GroupExpand_1_0::theSOPTypeName("FeE::groupExpand::1.0"_sh);

void
newSopOperator(OP_OperatorTable* table)
{
    OP_Operator* newOp = new OP_Operator(
        SOP_FeE_GroupExpand_1_0::theSOPTypeName,
        "FeE Group Expand",
        SOP_FeE_GroupExpand_1_0::myConstructor,
        SOP_FeE_GroupExpand_1_0::buildTemplates(),
        1,
        1,
        nullptr,
        OP_FLAG_GENERATOR,
        nullptr,
        1,
        "Five elements Elf/Group");

    newOp->setIconName("SOP_groupexpand");
    table->addOperator(newOp);

}

//class SOP_FeE_GroupExpand_1_0Cache : public SOP_NodeCache
//{
//public:
//    SOP_FeE_GroupExpand_1_0Cache() : SOP_NodeCache(),
//        myPrevOutputDetailID(-1)
//    {}
//    ~SOP_FeE_GroupExpand_1_0Cache() override {}
//
//    int64 myPrevOutputDetailID;
//};


class SOP_FeE_GroupExpand_1_0Verb : public SOP_NodeVerb
{
public:
    SOP_FeE_GroupExpand_1_0Verb() {}
    virtual ~SOP_FeE_GroupExpand_1_0Verb() {}

    virtual SOP_NodeParms *allocParms() const { return new SOP_FeE_GroupExpand_1_0Parms(); }
    //virtual SOP_NodeCache* allocCache() const { return new SOP_FeE_GroupExpand_1_0Cache(); }
    virtual UT_StringHolder name() const { return SOP_FeE_GroupExpand_1_0::theSOPTypeName; }

    virtual CookMode cookMode(const SOP_NodeParms *parms) const { return COOK_GENERIC; }

    virtual void cook(const CookParms &cookparms) const;

    /// This static data member automatically registers
    /// this verb class at library ldir0d time.
    static const SOP_NodeVerb::Register<SOP_FeE_GroupExpand_1_0Verb> theVerb;
};

// The static member variable definition has to be outside the class definition.
// The declaration is inside the class.
const SOP_NodeVerb::Register<SOP_FeE_GroupExpand_1_0Verb> SOP_FeE_GroupExpand_1_0Verb::theVerb;

const SOP_NodeVerb *
SOP_FeE_GroupExpand_1_0::cookVerb() const 
{ 
    return SOP_FeE_GroupExpand_1_0Verb::theVerb.get();
}








static GA_GroupType
sopGroupType(SOP_FeE_GroupExpand_1_0Parms::GroupType parmgrouptype)
{
    using namespace SOP_FeE_GroupExpand_1_0Enums;
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
SOP_FeE_GroupExpand_1_0Verb::cook(const SOP_NodeVerb::CookParms &cookparms) const
{
    auto &&sopparms = cookparms.parms<SOP_FeE_GroupExpand_1_0Parms>();
    GA_Detail& outGeo0 = *cookparms.gdh().gdpNC();
    //auto sopcache = (SOP_FeE_GroupExpand_1_0Cache*)cookparms.cache();

    const GA_Detail& inGeo0 = *cookparms.inputGeo(0);

    outGeo0.replaceWith(inGeo0);

    if (!sopparms.getOutExpandGroup() && !sopparms.getOutBorderGroup())
        return;
        
    
    const GA_GroupType groupType = sopGroupType(sopparms.getGroupType());

    
    UT_AutoInterrupt boss("Processing");
    if (boss.wasInterrupted())
        return;


    GFE_GroupExpand groupExpand(outGeo0, cookparms);
    
    //groupExpand.groupParser.setGroup(groupType, sopparms.getGroup());
    if (!groupExpand.setBaseGroup(groupType, sopparms.getGroup()))
        return;
    
    groupExpand.setComputeParm(sopparms.getNumsteps(), sopparms.getLargeConnectivity(),
        sopparms.getSubscribeRatio(), sopparms.getMinGrainSize());
    
    groupExpand.outTopoAttrib = sopparms.getOutTopoAttrib();

    if (abs(sopparms.getNumsteps()) == 1)
    {
        if (sopparms.getOutExpandGroup())
            groupExpand.setExpandGroup(false, sopparms.getExpandGroupName());
        if (sopparms.getOutBorderGroup())
            groupExpand.setBorderGroup(false, sopparms.getBorderGroupName());
    }
    else
    {
        groupExpand.setExpandGroup(!sopparms.getOutExpandGroup(), sopparms.getExpandGroupName());
        groupExpand.setBorderGroup(!sopparms.getOutBorderGroup(), sopparms.getBorderGroupName());
    }
    //groupExpand.setPrevBorderGroup(false, sopparms.getPrevBorderGroupName());
    
    groupExpand.computeAndBumpDataId();
    groupExpand.visualizeOutGroup();
    
}


