
//#define UT_ASSERT_LEVEL 3
#include "SOP_FeE_FlatEdge_2_0.h"


#include "SOP_FeE_FlatEdge_2_0.proto.h"

#include "GA/GA_Detail.h"
#include "PRM/PRM_TemplateBuilder.h"
#include "UT/UT_Interrupt.h"
#include "UT/UT_DSOVersion.h"


#include "GFE/GFE_FlatEdge.h"


using namespace SOP_FeE_FlatEdge_2_0_Namespace;



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
    


    parm {
        name    "outFlatVertexEdgeGroup"
        cppname "OutFlatVertexEdgeGroup"
        label   "Out Flat Vertex Edge Group"
        type    toggle
        default { "flatEdge" }
        joinnext
        nolabel
        disablewhen "{ delGroupElement == 1 }"
    }
    parm {
        name    "flatVertexEdgeGroupName"
        cppname "FlatVertexEdgeGroupName"
        label   "Flat Vertex Edge Group Name"
        type    string
        default { "flatEdge" }
        disablewhen "{ outFlatVertexEdgeGroup == 0 } { delGroupElement == 1 }"
    }
    parm {
        name    "outFlatEdgeGroup"
        cppname "OutFlatEdgeGroup"
        label   "Out Flat Edge Group"
        type    toggle
        default { "flatEdge" }
        joinnext
        nolabel
        disablewhen "{ delGroupElement == 1 }"
    }
    parm {
        name    "flatEdgeGroupName"
        cppname "FlatEdgeGroupName"
        label   "Flat Edge Group Name"
        type    string
        default { "flatEdge" }
        disablewhen "{ outFlatEdgeGroup == 0 } { delGroupElement == 1 }"
    }
    parm {
        name    "flatEdgeAngleThreshold"
        cppname "FlatEdgeAngleThreshold"
        label   "Flat Edge Angle Threshold"
        type    log
        default { "0.001" }
        range   { 0.01 180! }
    }
    parm {
        name    "includeUnsharedEdge"
        cppname "IncludeUnsharedEdge"
        label   "Include Unshared Edge"
        type    toggle
        default { "0" }
    }
    parm {
        name    "nonManifoldEdgeOp"
        cppname "NonManifoldEdgeOp"
        label   "Non Manifold Edge Op"
        type    ordinal
        default { "none" }
        menu {
            "none"  "None"
            "all"   "All"
            "min"   "Min"
            "max"   "Max"
        }
    }
    parm {
        name    "absoluteDot"
        cppname "AbsoluteDot"
        label   "Absolute Dot"
        type    toggle
        default { "0" }
    }

    parm {
        name    "findNormal3D"
        cppname "FindNormal3D"
        label   "Find Normal 3D"
        type    toggle
        default { "0" }
    }
    parm {
        name    "normal3DSearchOrder"
        cppname "Normal3DSearchOrder"
        label   "Normal3D Search Order"
        type    ordinal
        default { "prim" }
        menu {
            "prim"       "Primitive"
            "vertex"     "Vertex"
            "primVertex" "PrimVertex"
        }
    }
    parm {
        name    "normal3DAttrib"
        cppname "Normal3DAttrib"
        label   "Normal Attrib"
        type    string
        default { "N" }
    }
    parm {
        name    "weightingMethod"
        cppname "WeightingMethod"
        label   "Weighting Method"
        type    ordinal
        default { "angle" }
        menu {
            "uniform"   "Each Vertex Equally"
            "angle"     "By Vertex Angle"
            "area"      "By Face Area"
        }
        range   { 0! 2! }
    }

    parm {
        name    "reverseGroup"
        cppname "ReverseGroup"
        label   "Reverse Group"
        type    toggle
        default { "0" }
    }
    parm {
        name    "delGroupElement"
        cppname "DelGroupElement"
        label   "Delete Group Element"
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
SOP_FeE_FlatEdge_2_0::buildTemplates()
{
    static PRM_TemplateBuilder templ("SOP_FeE_FlatEdge_2_0.C"_sh, theDsFile);
    if (templ.justBuilt())
    {
        templ.setChoiceListPtr("group"_sh, &SOP_Node::allGroupMenu);
        templ.setChoiceListPtr("normal3DAttrib"_sh, &SOP_Node::allAttribMenu);
    }
    return templ.templates();
}


const UT_StringHolder SOP_FeE_FlatEdge_2_0::theSOPTypeName("FeE::flatEdge::2.0"_sh);

void
newSopOperator(OP_OperatorTable* table)
{
    OP_Operator* newOp = new OP_Operator(
        SOP_FeE_FlatEdge_2_0::theSOPTypeName,
        "FeE Flat Edge",
        SOP_FeE_FlatEdge_2_0::myConstructor,
        SOP_FeE_FlatEdge_2_0::buildTemplates(),
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


//class SOP_FeE_FlatEdge_2_0Cache : public SOP_NodeCache
//{
//public:
//    SOP_FeE_FlatEdge_2_0Cache() : SOP_NodeCache(),
//        myPrevOutputDetailID(-1)
//    {}
//    ~SOP_FeE_FlatEdge_2_0Cache() override {}
//
//    int64 myPrevOutputDetailID;
//};


class SOP_FeE_FlatEdge_2_0Verb : public SOP_NodeVerb
{
public:
    SOP_FeE_FlatEdge_2_0Verb() {}
    virtual ~SOP_FeE_FlatEdge_2_0Verb() {}

    virtual SOP_NodeParms *allocParms() const { return new SOP_FeE_FlatEdge_2_0Parms(); }
    //virtual SOP_NodeCache* allocCache() const { return new SOP_FeE_FlatEdge_2_0Cache(); }
    virtual UT_StringHolder name() const { return SOP_FeE_FlatEdge_2_0::theSOPTypeName; }

    virtual CookMode cookMode(const SOP_NodeParms *parms) const { return COOK_GENERIC; }

    virtual void cook(const CookParms &cookparms) const;

    /// This static data member automatically registers
    /// this verb class at library ldir0d time.
    static const SOP_NodeVerb::Register<SOP_FeE_FlatEdge_2_0Verb> theVerb;
};

// The static member variable definition has to be outside the class definition.
// The declaration is inside the class.
const SOP_NodeVerb::Register<SOP_FeE_FlatEdge_2_0Verb> SOP_FeE_FlatEdge_2_0Verb::theVerb;

const SOP_NodeVerb *
SOP_FeE_FlatEdge_2_0::cookVerb() const 
{ 
    return SOP_FeE_FlatEdge_2_0Verb::theVerb.get();
}







static GA_GroupType
sopGroupType(SOP_FeE_FlatEdge_2_0Parms::GroupType parmGroupType)
{
    using namespace SOP_FeE_FlatEdge_2_0Enums;
    switch (parmGroupType)
    {
    case GroupType::GUESS:     return GA_GROUP_INVALID;    break;
    case GroupType::PRIM:      return GA_GROUP_PRIMITIVE;  break;
    case GroupType::POINT:     return GA_GROUP_POINT;      break;
    case GroupType::VERTEX:    return GA_GROUP_VERTEX;     break;
    case GroupType::EDGE:      return GA_GROUP_EDGE;       break;
    }
    UT_ASSERT_MSG(0, "Unhandled Group type!");
    return GA_GROUP_INVALID;
}


static GFE_FlatEdge::NonManifoldEdgeOp
sopNonManifoldEdgeOp(SOP_FeE_FlatEdge_2_0Parms::NonManifoldEdgeOp parmNonManifoldEdgeOp)
{
    using namespace SOP_FeE_FlatEdge_2_0Enums;
    switch (parmNonManifoldEdgeOp)
    {
    case NonManifoldEdgeOp::NONE:    return GFE_FlatEdge::NonManifoldEdgeOp::None ; break;
    case NonManifoldEdgeOp::ALL:     return GFE_FlatEdge::NonManifoldEdgeOp::All  ; break;
    case NonManifoldEdgeOp::MIN:     return GFE_FlatEdge::NonManifoldEdgeOp::Min  ; break;
    case NonManifoldEdgeOp::MAX:     return GFE_FlatEdge::NonManifoldEdgeOp::Max  ; break;
    }
    UT_ASSERT_MSG(0, "Unhandled Manifold Edge OP!");
    return GFE_FlatEdge::NonManifoldEdgeOp::None;
}


static GFE_NormalSearchOrder
sopNormal3DSearchOrder(SOP_FeE_FlatEdge_2_0Parms::Normal3DSearchOrder parmNormalAttribClass)
{
    using namespace SOP_FeE_FlatEdge_2_0Enums;
    switch (parmNormalAttribClass)
    {
    case Normal3DSearchOrder::PRIM:        return GFE_NormalSearchOrder::Primitive; break;
    case Normal3DSearchOrder::VERTEX:      return GFE_NormalSearchOrder::Vertex;    break;
    case Normal3DSearchOrder::PRIMVERTEX:  return GFE_NormalSearchOrder::PrimVertex;    break;
    }
    UT_ASSERT_MSG(0, "Unhandled Normal Attrib Class!");
    return GFE_NormalSearchOrder::Invalid;
}

static GEO_NormalMethod
sopWeightingMethod(SOP_FeE_FlatEdge_2_0Parms::WeightingMethod parmWeightingMethod)
{
    using namespace SOP_FeE_FlatEdge_2_0Enums;
    switch (parmWeightingMethod)
    {
    case WeightingMethod::UNIFORM:    return GEO_NormalMethod::UNIFORM_WEIGHTED;    break;
    case WeightingMethod::ANGLE:      return GEO_NormalMethod::ANGLE_WEIGHTED;      break;
    case WeightingMethod::AREA:       return GEO_NormalMethod::AREA_WEIGHTED;       break;
    }
    UT_ASSERT_MSG(0, "Unhandled Weighting Method!");
    return GEO_NormalMethod::ANGLE_WEIGHTED;
}

void
SOP_FeE_FlatEdge_2_0Verb::cook(const SOP_NodeVerb::CookParms &cookparms) const
{
    auto &&sopparms = cookparms.parms<SOP_FeE_FlatEdge_2_0Parms>();
    GA_Detail& outGeo0 = *cookparms.gdh().gdpNC();
    //auto sopcache = (SOP_FeE_FlatEdge_2_0Cache*)cookparms.cache();

    const GA_Detail& inGeo0 = *cookparms.inputGeo(0);

    outGeo0.replaceWith(inGeo0);


    const UT_StringHolder& flatEdgeGroupName = sopparms.getFlatEdgeGroupName();
    
    if (!flatEdgeGroupName.isstring() || flatEdgeGroupName.length()==0)
        return;


    const GA_GroupType groupType = sopGroupType(sopparms.getGroupType());
    const GFE_FlatEdge::NonManifoldEdgeOp nonManifoldEdgeOp = sopNonManifoldEdgeOp(sopparms.getNonManifoldEdgeOp());
    const GFE_NormalSearchOrder normalSearchOrder = sopNormal3DSearchOrder(sopparms.getNormal3DSearchOrder());
        
    
    UT_AutoInterrupt boss("Processing");
    if (boss.wasInterrupted())
        return;

    GFE_FlatEdge flatEdge(outGeo0, &cookparms);
    
    flatEdge.setComputeParm(
        sopparms.getIncludeUnsharedEdge(), nonManifoldEdgeOp, sopparms.getAbsoluteDot(),
        sopparms.getSubscribeRatio(), sopparms.getMinGrainSize());
    flatEdge.setAngleThresholdByDegrees(sopparms.getFlatEdgeAngleThreshold());
    
    flatEdge.groupSetter.setParm(sopparms.getReverseGroup());
    flatEdge.doDelGroupElement = sopparms.getDelGroupElement();
    
    flatEdge.normal3D.normalMethod = sopWeightingMethod(sopparms.getWeightingMethod());
    flatEdge.findOrCreateNormal3D(sopparms.getFindNormal3D(), true, normalSearchOrder, GA_STORE_INVALID, sopparms.getNormal3DAttrib());
    
    flatEdge.groupParser.setGroup(groupType, sopparms.getGroup());
    
    if (sopparms.getOutFlatEdgeGroup() || flatEdge.doDelGroupElement)
        flatEdge.findOrCreateEdgeGroup(flatEdge.doDelGroupElement, sopparms.getFlatEdgeGroupName());
    if (sopparms.getOutFlatVertexEdgeGroup())
        flatEdge.findOrCreateVertexGroup(false, sopparms.getFlatVertexEdgeGroupName());
    
    flatEdge.computeAndBumpDataId();
    flatEdge.delOrVisualizeGroup();


}


