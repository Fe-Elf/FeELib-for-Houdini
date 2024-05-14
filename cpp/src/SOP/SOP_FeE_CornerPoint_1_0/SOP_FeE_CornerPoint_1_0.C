
//#define UT_ASSERT_LEVEL 3
#include "SOP_FeE_CornerPoint_1_0.h"


#include "SOP_FeE_CornerPoint_1_0.proto.h"

#include "GA/GA_Detail.h"
#include "PRM/PRM_TemplateBuilder.h"
#include "UT/UT_Interrupt.h"
#include "UT/UT_DSOVersion.h"


#include "GFE/GFE_CornerPoint.h"


using namespace SOP_FeE_CornerPoint_1_0_Namespace;



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
    
    


//#if GFE_CornerPoint_ConvexConcave
//
//    parm {
//        name    "outConvexPointGroup"
//        cppname "OutConvexPointGroup"
//        label   "Out Convex Point Group"
//        type    toggle
//        joinnext
//        nolabel
//        default { "0" }
//    }
//    parm {
//        name    "convexPointGroupName"
//        cppname "ConvexPointGroupName"
//        label   "Convex Point Group Name"
//        type    string
//        default { "convex" }
//        disablewhen "{ outConvexPointGroup == 0 }"
//    }
//    parm {
//        name    "thresholdConvexDegrees"
//        cppname "ThresholdConvexDegrees"
//        label   "Threshold Convex Degrees"
//        type    angle
//        default { "180" }
//        range   { 0! 360! }
//        disablewhen "{ outConvexPointGroup == 0 }"
//    }
//
//    parm {
//        name    "outConcavePointGroup"
//        cppname "OutConcavePointGroup"
//        label   "Out Concave Point Group"
//        type    toggle
//        joinnext
//        nolabel
//        default { "1" }
//    }
//    parm {
//        name    "concavePointGroupName"
//        cppname "ConcavePointGroupName"
//        label   "Concave Point Group Name"
//        type    string
//        default { "concave" }
//        disablewhen "{ outConcavePointGroup == 0 }"
//    }
//    parm {
//        name    "thresholdConcaveDegrees"
//        cppname "ThresholdConcaveDegrees"
//        label   "Threshold Concave Degrees"
//        type    angle
//        default { "180" }
//        range   { 0! 360! }
//        disablewhen "{ outConcavePointGroup == 0 }"
//    }
//#else

    parm {
        name    "cornerPointGroupName"
        cppname "CornerPointGroupName"
        label   "Corner Point Group Name"
        type    string
        default { "corner" }
    }
    parm {
        name    "thresholdDegrees"
        cppname "ThresholdDegrees"
        label   "Threshold Degrees"
        type    angle
        default { "180" }
        range   { 0! 360! }
    }

    parm {
        name    "outAsConvexPoint"
        cppname "OutAsConvexPoint"
        label   "Out as Convex Point"
        type    toggle
        default { "0" }
    }


//#endif
        





    parm {
        name    "groupPrimCornerPoint"
        cppname "GroupPrimCornerPoint"
        label   "Group Prim Corner Point"
        type    toggle
        default { "1" }
    }
    parm {
        name    "reverseGroup"
        cppname "ReverseGroup"
        label   "Reverse Group"
        type    toggle
        default { "0" }
    }
    parm {
        name    "outDotValue"
        cppname "OutDotValue"
        label   "Output Dot Value"
        type    toggle
        nolabel
        joinnext
        default { "0" }
    }
    parm {
        name    "dotAttribName"
        cppname "DotAttribName"
        label   "Dot Attrib Name"
        type    string
        default { "dot" }
        disablewhen "{ outDotValue == 0 }"
    }
    parm {
        name    "delCornerPoint"
        cppname "DelCornerPoint"
        label   "Delete Corner Point"
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
SOP_FeE_CornerPoint_1_0::buildTemplates()
{
    static PRM_TemplateBuilder templ("SOP_FeE_CornerPoint_1_0.C"_sh, theDsFile);
    if (templ.justBuilt())
    {
        templ.setChoiceListPtr("group"_sh, &SOP_Node::allGroupMenu);
        templ.setChoiceListPtr("dotAttribName"_sh,        &SOP_Node::pointAttribReplaceMenu);
#if GFE_CornerPoint_ConvexConcave
        templ.setChoiceListPtr("convexPointGroupName"_sh,  &SOP_Node::pointNamedGroupMenu);
        templ.setChoiceListPtr("concavePointGroupName"_sh, &SOP_Node::pointNamedGroupMenu);
#else
        templ.setChoiceListPtr("cornerPointGroupName"_sh,  &SOP_Node::pointNamedGroupMenu);
#endif
    }
    return templ.templates();
}


const UT_StringHolder SOP_FeE_CornerPoint_1_0::theSOPTypeName("FeE::cornerPoint::2.0"_sh);

void
newSopOperator(OP_OperatorTable* table)
{
    OP_Operator* newOp = new OP_Operator(
        SOP_FeE_CornerPoint_1_0::theSOPTypeName,
        "FeE Corner Point",
        SOP_FeE_CornerPoint_1_0::myConstructor,
        SOP_FeE_CornerPoint_1_0::buildTemplates(),
        1,
        1,
        nullptr,
        OP_FLAG_GENERATOR,
        nullptr,
        1,
        "Five elements Elf/Data/Topology");

    newOp->setIconName("SOP_dissolve-2.0");
    table->addOperator(newOp);

}


//class SOP_FeE_CornerPoint_1_0Cache : public SOP_NodeCache
//{
//public:
//    SOP_FeE_CornerPoint_1_0Cache() : SOP_NodeCache(),
//        myPrevOutputDetailID(-1)
//    {}
//    ~SOP_FeE_CornerPoint_1_0Cache() override {}
//
//    int64 myPrevOutputDetailID;
//};


class SOP_FeE_CornerPoint_1_0Verb : public SOP_NodeVerb
{
public:
    SOP_FeE_CornerPoint_1_0Verb() {}
    virtual ~SOP_FeE_CornerPoint_1_0Verb() {}

    virtual SOP_NodeParms *allocParms() const { return new SOP_FeE_CornerPoint_1_0Parms(); }
    //virtual SOP_NodeCache* allocCache() const { return new SOP_FeE_CornerPoint_1_0Cache(); }
    virtual UT_StringHolder name() const { return SOP_FeE_CornerPoint_1_0::theSOPTypeName; }

    virtual CookMode cookMode(const SOP_NodeParms *parms) const { return COOK_INPLACE; }

    virtual void cook(const CookParms &cookparms) const;

    /// This static data member automatically registers
    /// this verb class at library ldir0d time.
    static const SOP_NodeVerb::Register<SOP_FeE_CornerPoint_1_0Verb> theVerb;
};

// The static member variable definition has to be outside the class definition.
// The declaration is inside the class.
const SOP_NodeVerb::Register<SOP_FeE_CornerPoint_1_0Verb> SOP_FeE_CornerPoint_1_0Verb::theVerb;

const SOP_NodeVerb *
SOP_FeE_CornerPoint_1_0::cookVerb() const 
{ 
    return SOP_FeE_CornerPoint_1_0Verb::theVerb.get();
}







static GA_GroupType
sopGroupType(const SOP_FeE_CornerPoint_1_0Parms::GroupType parmGroupType)
{
    using namespace SOP_FeE_CornerPoint_1_0Enums;
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
SOP_FeE_CornerPoint_1_0Verb::cook(const SOP_NodeVerb::CookParms &cookparms) const
{
    auto &&sopparms = cookparms.parms<SOP_FeE_CornerPoint_1_0Parms>();
    GA_Detail& outGeo0 = *cookparms.gdh().gdpNC();
    //auto sopcache = (SOP_FeE_CornerPoint_1_0Cache*)cookparms.cache();

    //const GA_Detail& inGeo0 = *cookparms.inputGeo(0);

    //outGeo0.replaceWith(inGeo0);


    const GA_GroupType groupType = sopGroupType(sopparms.getGroupType());
    
    
    UT_AutoInterrupt boss("Processing");
    if (boss.wasInterrupted())
        return;

    GFE_CornerPoint cornerPoint(outGeo0, cookparms);

    cornerPoint.setComputeParm(sopparms.getGroupPrimCornerPoint(), 
                            sopparms.getSubscribeRatio(), sopparms.getMinGrainSize());

#if GFE_CornerPoint_ConvexConcave
    cornerPoint.setThresholdConvexDegrees(sopparms.getThresholdConvexDegrees());
    cornerPoint.setThresholdConcaveDegrees(sopparms.getThresholdConcaveDegrees());
#else
    cornerPoint.setThresholdDegrees(sopparms.getThresholdDegrees());
    cornerPoint.outAsConvexPoint = sopparms.getOutAsConvexPoint();
#endif
    
    cornerPoint.doDelGroupElement = sopparms.getDelCornerPoint();
    
    cornerPoint.groupSetter.setParm(sopparms.getReverseGroup());
    cornerPoint.groupParser.setGroup(groupType, sopparms.getGroup());
    cornerPoint.findOrCreatePointGroup(false, sopparms.getCornerPointGroupName());
    
    cornerPoint.computeAndBumpDataId();
    cornerPoint.visualizeOutGroup();


}

