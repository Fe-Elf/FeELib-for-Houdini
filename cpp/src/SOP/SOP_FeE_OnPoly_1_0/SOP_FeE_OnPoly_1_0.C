
//#define UT_ASSERT_LEVEL 3
#include "SOP_FeE_OnPoly_1_0.h"


#include "SOP_FeE_OnPoly_1_0.proto.h"

#include "GA/GA_Detail.h"
#include "PRM/PRM_TemplateBuilder.h"
#include "UT/UT_Interrupt.h"
#include "UT/UT_DSOVersion.h"


#include "GFE/GFE_OnPoly.h"


using namespace SOP_FeE_OnPoly_1_0_Namespace;



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
        name    "collisionGroup"
        cppname "CollisionGroup"
        label   "Collision Group"
        type    string
        default { "" }
        parmtag { "script_action" "import soputils\nkwargs['geometrytype'] = (hou.geometryType.Primitives,)\nkwargs['inputindex'] = 1\nsoputils.selectGroupParm(kwargs)" }
        parmtag { "script_action_help" "Select geometry from an available viewport.\nShift-click to turn on Select Groups." }
        parmtag { "script_action_icon" "BUTTONS_reselect" }
        parmtag { "sop_input" "1" }
    }
    //parm {
    //    name    "class"
    //    cppname "Class"
    //    label   "Piece Elements"
    //    type    ordinal
    //    default { "primitive" }
    //    disablewhen "{ runOverPiece == 0 }"
    //    menu {
    //        "prim"      "Primitive"
    //        "point"     "Point"
    //    }
    //}
    //parm {
    //    name    "classRef"
    //    cppname "ClassRef"
    //    label   "Piece Elements Ref"
    //    type    ordinal
    //    default { "primitive" }
    //    disablewhen "{ runOverPiece == 0 }"
    //    menu {
    //        "prim"      "Primitive"
    //        "point"     "Point"
    //    }
    //}
    parm {
        name    "sepparm2"
        label   "Separator"
        type    separator
        default { "" }
    }
    parm {
        name    "method"
        cppname "Method"
        label   "Method"
        type    ordinal
        default { "minimum" }
        menu {
            "minimum"   "Minimum Distance"
            "project"   "Project Rays"
        }
    }
    parm {
        name    "dirAttrib"
        cppname "DirAttrib"
        label   "Direction Attrib"
        type    string
        default { "N" }
        hidewhen "{ method != project }"
    }
    parm {
        name    "peakDist"
        cppname "PeakDist"
        label   "Peak Distance"
        type    log
        default { "-0.01" }
        hidewhen "{ method != project }"
        range   { 0.0001 0.1 }
    }
    parm {
        name    "checkMaxRayDist"
        cppname "CheckMaxRayDist"
        label   "Check Max Ray Distance"
        type    toggle
        nolabel
        joinnext
        default { "on" }
    }
    parm {
        name    "maxRayDist"
        cppname "MaxRayDist"
        label   "Max Distance"
        type    log
        default { "1e-05" }
        disablewhen "{ checkMaxRayDist == 0 }"
        range   { 0.001 10 }
    }
    parm {
        name    "rayTolerance"
        cppname "RayTolerance"
        label   "Ray Tolerance"
        type    log
        default { "0.01" }
        range   { 0! 1! }
    }
    parm {
        name    "outHitGroup"
        cppname "OutHitGroup"
        label   "Out Hit Group"
        type    toggle
        default { "on" }
    }
    parm {
        name    "hitGroup"
        cppname "HitGroup"
        label   "Hit Group"
        type    string
        default { "onPoly" }
        disablewhen "{ outHitGroup == 0 }"
    }
    parm {
        name    "outHitPrimAttrib"
        cppname "OutHitPrimAttrib"
        label   "Out Hit Prim Attribute"
        type    toggle
        nolabel
        joinnext
        default { "off" }
    }
    parm {
        name    "hitPrimAttrib"
        cppname "HitPrimAttrib"
        label   "Hit Prim Attribute"
        type    string
        default { "hitPrim" }
        disablewhen "{ outHitPrimAttrib == 0 }"
    }
    parm {
        name    "outHitPrimUVAttrib"
        cppname "OutHitPrimUVAttrib"
        label   "Out Hit Prim UV Attribute"
        type    toggle
        nolabel
        joinnext
        default { "off" }
    }
    parm {
        name    "hitPrimUVAttrib"
        cppname "HitPrimUVAttrib"
        label   "Hit Prim UV Attribute"
        type    string
        default { "hitPrimUV" }
        disablewhen "{ outHitPrimUVAttrib == 0 }"
    }
    parm {
        name    "sepparm"
        label   "Separator"
        type    separator
        default { "" }
    }
    parm {
        name    "reverseGroup"
        cppname "reverseGroup"
        label   "Reverse Group"
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
SOP_FeE_OnPoly_1_0::buildTemplates()
{
    static PRM_TemplateBuilder templ("SOP_FeE_OnPoly_1_0.C"_sh, theDsFile);
    if (templ.justBuilt())
    {
        templ.setChoiceListPtr("group"_sh, &SOP_Node::allGroupMenu);
        templ.setChoiceListPtr("combineGroupName"_sh, &SOP_Node::allGroupMenu);
    }
    return templ.templates();
}


const UT_StringHolder SOP_FeE_OnPoly_1_0::theSOPTypeName("FeE::onPoly::1.0"_sh);

void
newSopOperator(OP_OperatorTable* table)
{
    OP_Operator* newOp = new OP_Operator(
        SOP_FeE_OnPoly_1_0::theSOPTypeName,
        "FeE On Poly",
        SOP_FeE_OnPoly_1_0::myConstructor,
        SOP_FeE_OnPoly_1_0::buildTemplates(),
        2,
        2,
        nullptr,
        OP_FLAG_GENERATOR,
        nullptr,
        1,
        "Five elements Elf/Group/Bound");

    newOp->setIconName("SOP_ray");
    table->addOperator(newOp);

}


//class SOP_FeE_OnPoly_1_0Cache : public SOP_NodeCache
//{
//public:
//    SOP_FeE_OnPoly_1_0Cache() : SOP_NodeCache(),
//        myPrevOutputDetailID(-1)
//    {}
//    ~SOP_FeE_OnPoly_1_0Cache() override {}
//
//    int64 myPrevOutputDetailID;
//};


class SOP_FeE_OnPoly_1_0Verb : public SOP_NodeVerb
{
public:
    SOP_FeE_OnPoly_1_0Verb() {}
    virtual ~SOP_FeE_OnPoly_1_0Verb() {}

    virtual SOP_NodeParms *allocParms() const { return new SOP_FeE_OnPoly_1_0Parms(); }
    //virtual SOP_NodeCache* allocCache() const { return new SOP_FeE_OnPoly_1_0Cache(); }
    virtual UT_StringHolder name() const { return SOP_FeE_OnPoly_1_0::theSOPTypeName; }

    virtual CookMode cookMode(const SOP_NodeParms *parms) const { return COOK_GENERIC; }

    virtual void cook(const CookParms &cookparms) const;

    /// This static data member automatically registers
    /// this verb class at library ldir0d time.
    static const SOP_NodeVerb::Register<SOP_FeE_OnPoly_1_0Verb> theVerb;
};

// The static member variable definition has to be outside the class definition.
// The declaration is inside the class.
const SOP_NodeVerb::Register<SOP_FeE_OnPoly_1_0Verb> SOP_FeE_OnPoly_1_0Verb::theVerb;

const SOP_NodeVerb *
SOP_FeE_OnPoly_1_0::cookVerb() const 
{ 
    return SOP_FeE_OnPoly_1_0Verb::theVerb.get();
}







// static GA_AttributeOwner
// sopAttribOwner(SOP_FeE_OnPoly_1_0Parms::Class parmgrouptype)
// {
//     using namespace SOP_FeE_OnPoly_1_0Enums;
//     switch (parmgrouptype)
//     {
//     case Class::PRIM:      return GA_ATTRIB_PRIMITIVE;  break;
//     case Class::POINT:     return GA_ATTRIB_POINT;      break;
//     }
//     UT_ASSERT_MSG(0, "Unhandled geo Class type!");
//     return GA_ATTRIB_PRIMITIVE;
// }
// 
// static GA_AttributeOwner
// sopAttribOwner(SOP_FeE_OnPoly_1_0Parms::ClassRef parmgrouptype)
// {
//     using namespace SOP_FeE_OnPoly_1_0Enums;
//     switch (parmgrouptype)
//     {
//     case ClassRef::PRIM:      return GA_ATTRIB_PRIMITIVE;  break;
//     case ClassRef::POINT:     return GA_ATTRIB_POINT;      break;
//     }
//     UT_ASSERT_MSG(0, "Unhandled geo Class Ref type!");
//     return GA_ATTRIB_PRIMITIVE;
// }

static GA_GroupType
sopGroupType(SOP_FeE_OnPoly_1_0Parms::GroupType parmgrouptype)
{
    using namespace SOP_FeE_OnPoly_1_0Enums;
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
SOP_FeE_OnPoly_1_0Verb::cook(const SOP_NodeVerb::CookParms &cookparms) const
{
    auto &&sopparms = cookparms.parms<SOP_FeE_OnPoly_1_0Parms>();
    GA_Detail& outGeo0 = cookparms.gdh().gdpNC();
    //auto sopcache = (SOP_FeE_OnPoly_1_0Cache*)cookparms.cache();

    const GA_Detail& inGeo0 = cookparms.inputGeo(0);
    const GA_Detail& inGeo1 = cookparms.inputGeo(1);

    outGeo0.replaceWith(inGeo0);


    const UT_StringHolder& geo0AttribNames = sopparms.getCombineGroupName();
    if (!geo0AttribNames.isstring())
        return;

    const GA_GroupType groupType = sopGroupType(sopparms.getGroupType());



    UT_AutoInterrupt boss("Processing");
    if (boss.wasInterrupted())
        return;

    
    GFE_OnPoly onPoly(outGeo0, cookparms);
    onPoly.findOrCreatePointGroup();
    onPoly.setComputeParm(sopparms.getFirstIndex(), sopparms.getNegativeIndex(), sopparms.getOutAsOffset(),
        sopparms.getSubscribeRatio(), sopparms.getMinGrainSize());

    onPoly.groupParser.setGroup(groupType, sopparms.getGroup());
    onPoly.computeAndBumpDataId();

    
    const UT_StringHolder& collisionGroupName = sopparms.getCollisionGroupName();
    const GA_AttributeOwner attribClass = sopparms.getClass();
    const GA_AttributeOwner attribClassRef = sopparms.getClassRef();
    const bool runOverPiece = sopparms.getRunOverPiece();
    const bool outHitGroup = sopparms.getOutHitGroup();
    const bool outHitPrimAttrib = sopparms.getOutHitPrimAttrib();
    const bool outHitPrimUVAttrib = sopparms.getOutHitPrimUVAttrib();
    
    
}

