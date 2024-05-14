
//#define UT_ASSERT_LEVEL 3
#include "SOP_FeE_PointInBBox_2_0.h"


#include "SOP_FeE_PointInBBox_2_0.proto.h"

#include "GA/GA_Detail.h"
#include "PRM/PRM_TemplateBuilder.h"
#include "UT/UT_Interrupt.h"
#include "UT/UT_DSOVersion.h"




#include "GFE/GFE_PointInBBox.h"




using namespace SOP_FeE_PointInBBox_2_0_Namespace;


static const char *theDsFile = R"THEDSFILE(
{
    name        parameters
    parm {
        name    "group"
        cppname "Group"
        label   "Group"
        type    string
        default { "" }
        range   { 0 1 }
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
        name    "groupRef"
        cppname "GroupRef"
        label   "Group Ref"
        type    string
        default { "" }
        range   { 0 1 }
        parmtag { "script_action" "import soputils\nkwargs['geometrytype'] = kwargs['node'].parmTuple('groupTypeRef')\nkwargs['inputindex'] = 0\nsoputils.selectGroupParm(kwargs)" }
        parmtag { "script_action_help" "Select geometry from an available viewport." }
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
    }
    parm {
        name    "posAttrib"
        cppname "PosAttrib"
        label   "Pos Attrib"
        type    string
        default { "P" }
    }
    parm {
        name    "posAttribRef"
        cppname "PosAttribRef"
        label   "Pos Attrib Ref"
        type    string
        default { "P" }
    }
    parm {
        name    "groupName"
        cppname "GroupName"
        label   "Group Name"
        type    string
        default { "pointInBBox" }
    }
    parm {
        name    "xMin"
        cppname "XMin"
        label   "XMin"
        type    toggle
        nolabel
        joinnext
        default { "0" }
    }
    parm {
        name    "xMinThreshold"
        cppname "XMinThreshold"
        label   "X Min"
        type    log
        default { "0" }
        disablewhen "{ xMin == 0 }"
        range   { 1e-03 10 }
    }
    parm {
        name    "xMax"
        cppname "XMax"
        label   "XMax"
        type    toggle
        nolabel
        joinnext
        default { "0" }
    }
    parm {
        name    "xMaxThreshold"
        cppname "XMaxThreshold"
        label   "X Max"
        type    log
        default { "0" }
        disablewhen "{ xMax == 0 }"
        range   { 1e-03 10 }
    }
    parm {
        name    "yMin"
        cppname "YMin"
        label   "YMin"
        type    toggle
        nolabel
        joinnext
        default { "0" }
    }
    parm {
        name    "yMinThreshold"
        cppname "YMinThreshold"
        label   "Y Min"
        type    log
        default { "0" }
        disablewhen "{ yMin == 0 }"
        range   { 1e-03 10 }
    }
    parm {
        name    "yMax"
        cppname "YMax"
        label   "YMax"
        type    toggle
        nolabel
        joinnext
        default { "0" }
    }
    parm {
        name    "yMaxThreshold"
        cppname "YMaxThreshold"
        label   "Y Max"
        type    log
        default { "0" }
        disablewhen "{ yMax == 0 }"
        range   { 1e-03 10 }
    }
    parm {
        name    "zMin"
        cppname "ZMin"
        label   "ZMin"
        type    toggle
        nolabel
        joinnext
        default { "0" }
    }
    parm {
        name    "zMinThreshold"
        cppname "ZMinThreshold"
        label   "Z Min"
        type    log
        default { "0" }
        disablewhen "{ zMin == 0 }"
        range   { 1e-03 10 }
    }
    parm {
        name    "zMax"
        cppname "ZMax"
        label   "ZMax"
        type    toggle
        nolabel
        joinnext
        default { "0" }
    }
    parm {
        name    "zMaxThreshold"
        cppname "ZMaxThreshold"
        label   "Z Max"
        type    log
        default { "0" }
        disablewhen "{ zMax == 0 }"
        range   { 1e-03 10 }
    }
    parm {
        name    "numInBoundMin"
        cppname "NumInBoundMin"
        label   "Num in Bound Min"
        type    integer
        default { "1" }
        range   { 1! 6! }
    }

    //parm {
    //    name    "intersectNum"
    //    cppname "IntersectNum"
    //    label   "Intersect Num"
    //    type    integer
    //    default { "0" }
    //    range   { 0! 2! }
    //}

    parm {
        name    "groupMergeType"
        cppname "GroupMergeType"
        label   "Group Merge Type"
        type    ordinal
        default { "replace" }
        menu {
            "replace"   "Replace Existing"
            "union"     "Union with Existing"
            "intersect" "Intersect with Existing"
            "subtract"  "Subtract from Existing"
        }
    }
    parm {
        name    "reverseGroup"
        cppname "ReverseGroup"
        label   "Reverse Group"
        type    toggle
        default { "0" }
    }

    parm {
        name    "delElement"
        cppname "DelElement"
        label   "Del Element"
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
        default { 1024 }
        range   { 0! 2048 }
    }
}
)THEDSFILE";

PRM_Template*
SOP_FeE_PointInBBox_2_0::buildTemplates()
{
    static PRM_TemplateBuilder templ("SOP_FeE_PointInBBox_2_0.C"_sh, theDsFile);
    if (templ.justBuilt())
    {
        //templ.setChoiceListPtr("group"_sh,        &SOP_Node::groupMenu);
        templ.setChoiceListPtr("posAttrib"_sh,    &SOP_Node::pointAttribReplaceMenu);
        templ.setChoiceListPtr("posAttribRef"_sh, &SOP_Node::pointAttribReplaceMenu);
    }
    return templ.templates();
}

const UT_StringHolder SOP_FeE_PointInBBox_2_0::theSOPTypeName("FeE::pointInBBox::2.0"_sh);

void
newSopOperator(OP_OperatorTable* table)
{
    OP_Operator* newOp = new OP_Operator(
        SOP_FeE_PointInBBox_2_0::theSOPTypeName,
        "FeE Point in Boundin Box",
        SOP_FeE_PointInBBox_2_0::myConstructor,
        SOP_FeE_PointInBBox_2_0::buildTemplates(),
        1,
        2,
        nullptr,
        OP_FLAG_GENERATOR,
        nullptr,
        1,
        "Five elements Elf/Group/Bound");

    newOp->setIconName("SOP_bound");
    table->addOperator(newOp);

}





class SOP_FeE_PointInBBox_2_0Verb : public SOP_NodeVerb
{
public:
    SOP_FeE_PointInBBox_2_0Verb() {}
    virtual ~SOP_FeE_PointInBBox_2_0Verb() {}

    virtual SOP_NodeParms *allocParms() const { return new SOP_FeE_PointInBBox_2_0Parms(); }
    virtual UT_StringHolder name() const { return SOP_FeE_PointInBBox_2_0::theSOPTypeName; }

    virtual CookMode cookMode(const SOP_NodeParms *parms) const { return COOK_INPLACE; }

    virtual void cook(const CookParms &cookparms) const;
    
    static const SOP_NodeVerb::Register<SOP_FeE_PointInBBox_2_0Verb> theVerb;
};

// The static member variable definition has to be outside the class definition.
// The declaration is inside the class.
const SOP_NodeVerb::Register<SOP_FeE_PointInBBox_2_0Verb> SOP_FeE_PointInBBox_2_0Verb::theVerb;

const SOP_NodeVerb *
SOP_FeE_PointInBBox_2_0::cookVerb() const 
{ 
    return SOP_FeE_PointInBBox_2_0Verb::theVerb.get();
}






static GA_GroupType
sopGroupType(const SOP_FeE_PointInBBox_2_0Parms::GroupType parmGroupType)
{
    using namespace SOP_FeE_PointInBBox_2_0Enums;
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
sopGroupType(const SOP_FeE_PointInBBox_2_0Parms::GroupTypeRef parmGroupType)
{
    using namespace SOP_FeE_PointInBBox_2_0Enums;
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


static GFE_GroupMergeType
sopGroupMergeType(const SOP_FeE_PointInBBox_2_0Parms::GroupMergeType parmGroupMergeType)
{
    using namespace SOP_FeE_PointInBBox_2_0Enums;
    switch (parmGroupMergeType)
    {
    case GroupMergeType::REPLACE:     return GFE_GroupMergeType::Replace;    break;
    case GroupMergeType::UNION:       return GFE_GroupMergeType::Union;      break;
    case GroupMergeType::INTERSECT:   return GFE_GroupMergeType::Intersect;  break;
    case GroupMergeType::SUBTRACT:    return GFE_GroupMergeType::Subtract;   break;
    }
    UT_ASSERT_MSG(0, "Unhandled Group Merge Type!");
    return GFE_GroupMergeType::Replace;
}

void
SOP_FeE_PointInBBox_2_0Verb::cook(const SOP_NodeVerb::CookParms &cookparms) const
{
    auto&& sopparms = cookparms.parms<SOP_FeE_PointInBBox_2_0Parms>();
    GA_Detail& outGeo0 = *cookparms.gdh().gdpNC();
    //auto sopcache = (SOP_FeE_PointInBBox_2_0Cache*)cookparms.cache();

    //const GA_Detail& inGeo0 = *cookparms.inputGeo(0);
    const GA_Detail* const inGeo1 = cookparms.inputGeo(1);

    //outGeo0.replaceWith(inGeo0);

    const GFE_GroupMergeType groupMergeType = sopGroupMergeType(sopparms.getGroupMergeType());

    const GA_GroupType groupType = sopGroupType(sopparms.getGroupType());
    const GA_GroupType groupTypeRef = sopGroupType(sopparms.getGroupTypeRef());
    
    
    UT_AutoInterrupt boss("Processing");
    if (boss.wasInterrupted())
        return;


    

    
    GFE_PointInBBox pointInBBox(outGeo0, inGeo1, cookparms);
    
    
#define SOP_FeE_PointInBBox_2_0_GETFUN_SPECILIZATION(NAME_UPPER)                            \
        if (sopparms.get##NAME_UPPER())                                                     \
            pointInBBox.set##NAME_UPPER(sopparms.get##NAME_UPPER##Threshold());             \

    SOP_FeE_PointInBBox_2_0_GETFUN_SPECILIZATION(XMin)
    SOP_FeE_PointInBBox_2_0_GETFUN_SPECILIZATION(XMax)
    SOP_FeE_PointInBBox_2_0_GETFUN_SPECILIZATION(YMin)
    SOP_FeE_PointInBBox_2_0_GETFUN_SPECILIZATION(YMax)
    SOP_FeE_PointInBBox_2_0_GETFUN_SPECILIZATION(ZMin)
    SOP_FeE_PointInBBox_2_0_GETFUN_SPECILIZATION(ZMax)
    
#undef SOP_FeE_PointInBBox_2_0_GETFUN_SPECILIZATION
    
    // pointInBBox.enlargeBBox.setBounds(
    //     sopparms.getXMinThreshold(),
    //     sopparms.getYMinThreshold(),
    //     sopparms.getZMinThreshold(),
    //     sopparms.getXMaxThreshold(),
    //     sopparms.getYMaxThreshold(),
    //     sopparms.getZMaxThreshold());
    
    pointInBBox.groupParser.setGroup(groupType, sopparms.getGroup());
    pointInBBox.groupParserBound.setGroup(groupTypeRef, sopparms.getGroupRef());
    
    pointInBBox.setComputeParm(sopparms.getNumInBoundMin(), sopparms.getDelElement(),
        sopparms.getSubscribeRatio(), sopparms.getMinGrainSize());
    
    pointInBBox.findOrCreatePointGroup(false, sopparms.getGroupName());
    
    pointInBBox.groupSetter.setComputeParm(groupMergeType, sopparms.getReverseGroup());

    pointInBBox.computeAndBumpDataId();
    pointInBBox.delOrVisualizeGroup();

    
}


