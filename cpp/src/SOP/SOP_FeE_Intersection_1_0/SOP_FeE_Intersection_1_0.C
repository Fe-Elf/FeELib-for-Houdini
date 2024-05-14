
//#define UT_ASSERT_LEVEL 3
#include "SOP_FeE_Intersection_1_0.h"


#include "SOP_FeE_Intersection_1_0.proto.h"

#include "GA/GA_Detail.h"
#include "PRM/PRM_TemplateBuilder.h"
#include "UT/UT_Interrupt.h"
#include "UT/UT_DSOVersion.h"




#include "GFE/GFE_Intersection.h"




using namespace SOP_FeE_Intersection_1_0_Namespace;


static const char *theDsFile = R"THEDSFILE(
{
    name        parameters
    parm {
        name    "group"
        cppname "Group"
        label   "Group"
        type    string
        default { "" }
        disablewhen "{ hasinput(0) == 0 }"
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
        disablewhen "{ hasinput(0) == 0 }"
        menu {
            "guess"     "Guess from Group"
            "prim"      "Primitive"
            "point"     "Point"
            "vertex"    "Vertex"
            "edge"      "Edge"
        }
    }
    parm {
        name    "groupRef"
        cppname "GroupRef"
        label   "GroupRef"
        type    string
        default { "" }
        disablewhen "{ hasinput(1) == 0 }"
        parmtag { "script_action" "import soputils\nkwargs['geometrytype'] = kwargs['node'].parmTuple('groupTypeRef')\nkwargs['inputindex'] = 0\nsoputils.selectGroupParm(kwargs)" }
        parmtag { "script_action_help" "Select geometry from an available viewport.\nShift-click to turn on Select Groups." }
        parmtag { "script_action_icon" "BUTTONS_reselect" }
    }
    parm {
        name    "groupTypeRef"
        cppname "GroupTypeRef"
        label   "Group Type Ref"
        type    ordinal
        default { "guess" }
        menu {
            "guess"     "Guess from Group"
            "prim"      "Primitive"
            "point"     "Point"
            "vertex"    "Vertex"
            "edge"      "Edge"
        }
        disablewhen "{ hasinput(1) == 0 }"
    }
    parm {
        name    "useTolerance"
        cppname "UseTolerance"
        label   "Use Tolerance"
        type    toggle
        nolabel
        joinnext
        default { "on" }
    }
    parm {
        name    "tolerance"
        cppname "Tolerance"
        label   "Proximity Tolerance"
        type    log
        default { "1e-05" }
        disablewhen "{ useTolerance == 0 }"
        range   { 0.001 1 }
    }

    parm {
        name    "outType"
        cppname "OutType"
        label   "Out Type"
        type    ordinal
        default { "point" }
        menu {
            "point"     "Point"
            "firstGeo"  "FirstGeo"
            "secondGeo" "SecondGeo"
            "dualGeo"   "DualGeo"
        }
    }
    parm {
        name    "detectVertexIntersection"
        cppname "DetectVertexIntersection"
        label   "Detect Vertex Intersections"
        type    toggle
        default { "off" }
        disablewhen "{ outType == point } { hasinput(2) == 1 }"
    }
    parm {
        name    "outIntersectionSegment"
        cppname "OutIntersectionSegment"
        label   "Output Intersection Segment"
        type    toggle
        default { "off" }
        disablewhen "{ outType == point } { hasinput(2) == 1 }"
    }
    parm {
        name    "triangulateMesh"
        cppname "TriangulateMesh"
        label   "Triangulate Mesh"
        type    toggle
        default { "off" }
        disablewhen "{ outType != point } { hasinput(2) == 1 }"
    }

    parm {
        name    "splitCurve"
        cppname "SplitCurve"
        label   "Split Curve"
        type    toggle
        default { "on" }
        disablewhen "{ outType == point hasinput(2) == 0 }"
    }
    parm {
        name    "repairResult"
        cppname "RepairResult"
        label   "Repair Result"
        type    toggle
        default { "on" }
        disablewhen "{ outType == point  hasinput(2) == 0 }"
    }
    group {
        name    "stdswitcher"
        label   "Output Attributes"

        parm {
            name    "outInputnumAttrib"
            cppname "OutInputnumAttrib"
            label   "Out Inputnum Attrib"
            type    toggle
            nolabel
            joinnext
            default { "1" }
            //disablewhen "{ hasinput(2) == 1 }"
        }
        parm {
            name    "inputnumAttrib"
            cppname "InputnumAttrib"
            label   "Input Number"
            type    string
            default { "sourceInput" }
            disablewhen "{ outInputnumAttrib == 0 hasinput(2) == 0 }"
        }

        parm {
            name    "outPrimnumAttrib"
            cppname "OutPrimnumAttrib"
            label   "Out Primnum Attrib"
            type    toggle
            nolabel
            joinnext
            default { "1" }
            //disablewhen "{ hasinput(2) == 1 }"
        }
        parm {
            name    "primnumAttrib"
            cppname "PrimnumAttrib"
            label   "Primitive Number"
            type    string
            default { "sourcePrim" }
            disablewhen "{ outPrimnumAttrib == 0 hasinput(2) == 0 }"
        }
        parm {
            name    "outPrimuvwAttrib"
            cppname "OutPrimuvwAttrib"
            label   "Out Primuvw Attrib"
            type    toggle
            nolabel
            joinnext
            default { "1" }
            //disablewhen "{ hasinput(2) == 1 }"
        }
        parm {
            name    "primuvwAttrib"
            cppname "PrimuvwAttrib"
            label   "Primitive UVW"
            type    string
            default { "sourcePrimuv" }
            disablewhen "{ outPrimuvwAttrib == 0 hasinput(2) == 0 }"
        }
        parm {
            name    "outPtnumAttrib"
            cppname "OutPtnumAttrib"
            label   "Out Ptnum Attrib"
            type    toggle
            nolabel
            joinnext
            default { "0" }
            //disablewhen "{ hasinput(2) == 1 }"
        }
        parm {
            name    "ptnumAttrib"
            cppname "PtnumAttrib"
            label   "Point Num"
            type    string
            default { "sourcePtnum" }
            disablewhen "{ outPtnumAttrib == 0 hasinput(2) == 0 } { hasinput(2) == 1 }"
        }
    }

    parm {
        name    "subscribeRatio"
        cppname "SubscribeRatio"
        label   "Subscribe Ratio"
        type    integer
        default { 64 }
        range   { 0! 256 }
        disablewhen "{ repairResult == 0 } { outType == point hasinput(2) == 0 }"
    }
    parm {
        name    "minGrainSize"
        cppname "MinGrainSize"
        label   "Min Grain Size"
        type    intlog
        default { 1024 }
        range   { 0! 2048 }
        disablewhen "{ repairResult == 0 } { outType == point hasinput(2) == 0 }"
    }
}
)THEDSFILE";

PRM_Template*
SOP_FeE_Intersection_1_0::buildTemplates()
{
    static PRM_TemplateBuilder templ("SOP_FeE_Intersection_1_0.C"_sh, theDsFile);
    if (templ.justBuilt())
    {
        templ.setChoiceListPtr("group"_sh,    &SOP_Node::groupMenu);
        templ.setChoiceListPtr("groupRef"_sh, &SOP_Node::groupMenu);
        templ.setChoiceListPtr("inputnumAttrib"_sh, &SOP_Node::pointAttribReplaceMenu);
        templ.setChoiceListPtr("primnumAttrib"_sh,  &SOP_Node::pointAttribReplaceMenu);
        templ.setChoiceListPtr("primuvwAttrib"_sh,  &SOP_Node::pointAttribReplaceMenu);
        templ.setChoiceListPtr("ptnumAttrib"_sh,    &SOP_Node::pointAttribReplaceMenu);
    }
    return templ.templates();
}

const UT_StringHolder SOP_FeE_Intersection_1_0::theSOPTypeName("FeE::intersection::1.0"_sh);

void
newSopOperator(OP_OperatorTable* table)
{
    OP_Operator* newOp = new OP_Operator(
        SOP_FeE_Intersection_1_0::theSOPTypeName,
        "FeE Intersection Stitch",
        SOP_FeE_Intersection_1_0::myConstructor,
        SOP_FeE_Intersection_1_0::buildTemplates(),
        0,
        3,
        nullptr,
        OP_FLAG_GENERATOR,
        nullptr,
        1,
        "Five elements Elf/Data/Intersection");

    newOp->setIconName("SOP_intersectionstitch");
    table->addOperator(newOp);

}





class SOP_FeE_Intersection_1_0Verb : public SOP_NodeVerb
{
public:
    SOP_FeE_Intersection_1_0Verb() {}
    virtual ~SOP_FeE_Intersection_1_0Verb() {}

    virtual SOP_NodeParms *allocParms() const { return new SOP_FeE_Intersection_1_0Parms(); }
    virtual UT_StringHolder name() const { return SOP_FeE_Intersection_1_0::theSOPTypeName; }

    virtual CookMode cookMode(const SOP_NodeParms *parms) const { return COOK_GENERIC; }

    virtual void cook(const CookParms &cookparms) const;
    
    /// This static data member automatically registers
    /// this verb class at library load time.
    static const SOP_NodeVerb::Register<SOP_FeE_Intersection_1_0Verb> theVerb;
};

// The static member variable definition has to be outside the class definition.
// The declaration is inside the class.
const SOP_NodeVerb::Register<SOP_FeE_Intersection_1_0Verb> SOP_FeE_Intersection_1_0Verb::theVerb;

const SOP_NodeVerb *
SOP_FeE_Intersection_1_0::cookVerb() const 
{ 
    return SOP_FeE_Intersection_1_0Verb::theVerb.get();
}





static GA_GroupType
sopGroupType(SOP_FeE_Intersection_1_0Parms::GroupType parmGroupType)
{
    using namespace SOP_FeE_Intersection_1_0Enums;
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
sopGroupType(SOP_FeE_Intersection_1_0Parms::GroupTypeRef parmGroupType)
{
    using namespace SOP_FeE_Intersection_1_0Enums;
    switch (parmGroupType)
    {
    case GroupTypeRef::GUESS:     return GA_GROUP_INVALID;    break;
    case GroupTypeRef::PRIM:      return GA_GROUP_PRIMITIVE;  break;
    case GroupTypeRef::POINT:     return GA_GROUP_POINT;      break;
    case GroupTypeRef::VERTEX:    return GA_GROUP_VERTEX;     break;
    case GroupTypeRef::EDGE:      return GA_GROUP_EDGE;       break;
    }
    UT_ASSERT_MSG(0, "Unhandled geo0Group type!");
    return GA_GROUP_INVALID;
}

static GFE_Intersection::OutType
sopOutType(SOP_FeE_Intersection_1_0Parms::OutType parmOutType)
{
    using namespace SOP_FeE_Intersection_1_0Enums;
    switch (parmOutType)
    {
    case OutType::POINT:       return GFE_Intersection::OutType::Point;      break;
    case OutType::FIRSTGEO:    return GFE_Intersection::OutType::FirstGeo;   break;
    case OutType::SECONDGEO:   return GFE_Intersection::OutType::SecondGeo;  break;
    case OutType::DUALGEO:     return GFE_Intersection::OutType::DualGeo;    break;
    }
    UT_ASSERT_MSG(0, "Unhandled GFE_Intersection Out Type!");
    return GFE_Intersection::OutType::Point;
}


void
SOP_FeE_Intersection_1_0Verb::cook(const SOP_NodeVerb::CookParms &cookparms) const
{
    auto&& sopparms = cookparms.parms<SOP_FeE_Intersection_1_0Parms>();
    GA_Detail& outGeo0 = *cookparms.gdh().gdpNC();
    //auto sopcache = (SOP_FeE_Intersection_1_0Cache*)cookparms.cache();

    const GA_Detail* const inGeo0 = cookparms.inputGeo(0);
    const GA_Detail* const inGeo1 = cookparms.inputGeo(1);
    const GA_Detail* const inGeo2 = cookparms.inputGeo(2);

    //outGeo0.replaceWith(inGeo0);


    const GA_GroupType groupType     = sopGroupType(sopparms.getGroupType());
    const GA_GroupType groupTypeRef0 = sopGroupType(sopparms.getGroupTypeRef());

    const GFE_Intersection::OutType outType = sopOutType(sopparms.getOutType());

    UT_AutoInterrupt boss("Processing");
    if (boss.wasInterrupted())
        return;
    
/*
    GFE_Intersection intersection(geo, &cookparms);
    intersection.compute();
*/
    
    GFE_Intersection intersection(outGeo0, inGeo0, inGeo1, inGeo2, &cookparms);
    
    intersection.setComputeParm(outType);
    intersection.setIntersectionStitchComputeParm(sopparms.getSplitCurve(), sopparms.getRepairResult(), sopparms.getSubscribeRatio(), sopparms.getMinGrainSize());
    intersection.setIntersectionAnalysisComputeParm(sopparms.getTriangulateMesh(), sopparms.getDetectVertexIntersection(), sopparms.getOutIntersectionSegment());
    
    if (sopparms.getUseTolerance())
        intersection.setTolerance(sopparms.getTolerance());

    intersection.setInputnumAttrib(sopparms.getOutInputnumAttrib(), sopparms.getInputnumAttrib().c_str());
    intersection.setPrimnumAttrib( sopparms.getOutPrimnumAttrib() , sopparms.getPrimnumAttrib(). c_str());
    intersection.setPrimuvwAttrib( sopparms.getOutPrimuvwAttrib() , sopparms.getPrimuvwAttrib(). c_str());
    intersection.setPtnumAttrib(   sopparms.getOutPtnumAttrib()   , sopparms.getPtnumAttrib().   c_str());

    
    intersection.groupParser.setGroup(groupType, sopparms.getGroup());
    intersection.groupParserRef0.setGroup(groupTypeRef0, sopparms.getGroupRef());
    intersection.computeAndBumpDataIdsForAddOrRemove();
    

}


