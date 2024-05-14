
//#define UT_ASSERT_LEVEL 3
#include "SOP_FeE_JoinCurve_2_0.h"

#include "SOP_FeE_JoinCurve_2_0.proto.h"

#include "GA/GA_Detail.h"
#include "PRM/PRM_TemplateBuilder.h"
#include "UT/UT_Interrupt.h"
#include "UT/UT_DSOVersion.h"


#include "GFE/GFE_JoinCurve.h"


using namespace SOP_FeE_JoinCurve_2_0_Namespace;

static const char *theDsFile = R"THEDSFILE(
{
    name        parameters
    parm {
        name    "stopPointGroup"
        cppname "StopPointGroup"
        label   "Stop Point Group"
        type    string
        default { "" }
        range   { 0 1 }
        parmtag { "script_action" "import soputils\nkwargs['geometrytype'] = (hou.geometryType.Points,)\nkwargs['inputindex'] = 0\nsoputils.selectGroupParm(kwargs)" }
        parmtag { "script_action_help" "Select geometry from an available viewport." }
        parmtag { "script_action_icon" "BUTTONS_reselect" }
    }
    parm {
        name    "sortPtnum"
        label   "Sort ptnum"
        type    toggle
        default { "0" }
    }


    parm {
        name    "copyAttribNameFromFirstElem"
        label   "Copy Attribute Name from First Elem"
        type    string
        default { "" }
    }
    parm {
        name    "uniqueneb_srcprims"
        label   "Unique Neb Source Prims"
        type    toggle
        default { "1" }
        disablewhen "{ outSrcPrim == 0 }"
    }
    parm {
        name    "keepEdgeGroup"
        label   "Keep Edge Group"
        type    toggle
        default { "1" }
    }
    parm {
        name    "removeDuplicateLoop"
        label   "Remove Duplicate Loop"
        type    toggle
        default { "1" }
    }


    parm {
        name    "primGroup"
        cppname "PrimGroup"
        label   "Prim Group"
        type    string
        default { "" }
        parmtag { "script_action" "import soputils\nkwargs['geometrytype'] = (hou.geometryType.Primitives,)\nkwargs['inputindex'] = 0\nsoputils.selectGroupParm(kwargs)" }
        parmtag { "script_action_help" "Select geometry from an available viewport.\nShift-click to turn on Select Groups." }
        parmtag { "script_action_icon" "BUTTONS_reselect" }
        parmtag { "sop_input" "0" }
    }
    parm {
        name    "pointGroup"
        cppname "PointGroup"
        label   "Point Group"
        type    string
        default { "" }
        parmtag { "script_action" "import soputils\nkwargs['geometrytype'] = (hou.geometryType.Points,)\nkwargs['inputindex'] = 0\nsoputils.selectGroupParm(kwargs)" }
        parmtag { "script_action_help" "Select geometry from an available viewport.\nShift-click to turn on Select Groups." }
        parmtag { "script_action_icon" "BUTTONS_reselect" }
        parmtag { "sop_input" "0" }
    }
    parm {
        name    "vertexGroup"
        cppname "VertexGroup"
        label   "Vertex Group"
        type    string
        default { "" }
        parmtag { "script_action" "import soputils\nkwargs['geometrytype'] = (hou.geometryType.Vertices,)\nkwargs['inputindex'] = 0\nsoputils.selectGroupParm(kwargs)" }
        parmtag { "script_action_help" "Select geometry from an available viewport.\nShift-click to turn on Select Groups." }
        parmtag { "script_action_icon" "BUTTONS_reselect" }
        parmtag { "sop_input" "0" }
    }
    parm {
        name    "edgeGroup"
        cppname "EdgeGroup"
        label   "Edge Group"
        type    string
        default { "" }
        parmtag { "script_action" "import soputils\nkwargs['geometrytype'] = (hou.geometryType.Edges,)\nkwargs['inputindex'] = 0\nsoputils.selectGroupParm(kwargs)" }
        parmtag { "script_action_help" "Select geometry from an available viewport.\nShift-click to turn on Select Groups." }
        parmtag { "script_action_icon" "BUTTONS_reselect" }
        parmtag { "sop_input" "0" }
    }



    parm {
        name    "attribFromVertex"
        cppname "AttribFromVertex"
        label   "Attrib from Vertex"
        type    string
        default { "" }
    }
    parm {
        name    "attribFromPrim"
        cppname "AttribFromPrim"
        label   "Attrib from Prim"
        type    string
        default { "" }
    }
    parm {
        name    "groupFromVertex"
        cppname "GroupFromVertex"
        label   "Group from Vertex"
        type    string
        default { "" }
    }
    parm {
        name    "groupFromPrim"
        cppname "GroupFromPrim"
        label   "Group from Prim"
        type    string
        default { "" }
    }
    parm {
        name    "groupFromEdge"
        cppname "GroupFromEdge"
        label   "Group from Edge"
        type    string
        default { "" }
    }

    parm {
        name    "promoteEdgeGroupToPrim"
        cppname "PromoteEdgeGroupToPrim"
        label   "Promote Edge Group to Prim"
        type    string
        default { "" }
    }



    parm {
        name    "outSrcPrim"
        cppname "OutSrcPrim"
        label   "Output Source Prim"
        type    toggle
        default { 0 }
        nolabel
        joinnext
    }
    parm {
        name    "srcPrimAttribName"
        cppname "SrcPrimAttribName"
        label   "Source Prim Attrib Name"
        type    string
        default { "srcPrim" }
        disablewhen "{ outSrcPrim == 0 }"
    }



    parm {
        name    "primType"
        cppname "PrimType"
        label   "Prim Type"
        type    ordinal
        default { "polyline" }
        menu {
            "polyline"  "Polyline"
            "poly"      "Poly"
        }
    }

    parm {
        name    "keepOrder"
        cppname "KeepOrder"
        label   "Keep Order"
        type    toggle
        default { "0" }
    }
    parm {
       name    "keepLoop"
       cppname "KeepLoop"
       label   "Keep Loop"
       type    toggle
       default { 1 }
    }
    parm {
        name    "closeLoop"
        cppname "CloseLoop"
        label   "Close Loop"
        type    toggle
        default { "0" }
    }


    parm {
       name    "kernel"
       cppname "Kernel"
       label   "Kernel"
       type    integer
       default { 0 }
       range   { 0! 1! }
    }


    parm {
       name    "checkInputError"
       cppname "CheckInputError"
       label   "Check Input Error"
       type    toggle
       default { 0 }
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
SOP_FeE_JoinCurve_2_0::buildTemplates()
{
    static PRM_TemplateBuilder templ("SOP_FeE_JoinCurve_2_0.C"_sh, theDsFile);
    if (templ.justBuilt())
    {
        templ.setChoiceListPtr("stopPointGroup"_sh, &SOP_Node::pointGroupMenu);
    }
    return templ.templates();
}





const UT_StringHolder SOP_FeE_JoinCurve_2_0::theSOPTypeName("FeE::joinCurve::2.0"_sh);

void
newSopOperator(OP_OperatorTable* table)
{
    OP_Operator* newOp = new OP_Operator(
        SOP_FeE_JoinCurve_2_0::theSOPTypeName,
        "FeE Join Curve",
        SOP_FeE_JoinCurve_2_0::myConstructor,
        SOP_FeE_JoinCurve_2_0::buildTemplates(),
        1,
        1,
        nullptr,
        OP_FLAG_GENERATOR,
        nullptr,
        1,
        "Five elements Elf/Topo/Optimize");

    newOp->setIconName("SOP_polypath");
    table->addOperator(newOp);
}




class SOP_FeE_JoinCurve_2_0Verb : public SOP_NodeVerb
{
public:
    SOP_FeE_JoinCurve_2_0Verb() {}
    virtual ~SOP_FeE_JoinCurve_2_0Verb() {}

    virtual SOP_NodeParms *allocParms() const { return new SOP_FeE_JoinCurve_2_0Parms(); }
    virtual UT_StringHolder name() const { return SOP_FeE_JoinCurve_2_0::theSOPTypeName; }

    virtual CookMode cookMode(const SOP_NodeParms *parms) const { return COOK_GENERIC; }

    virtual void cook(const CookParms &cookparms) const;
    
    static const SOP_NodeVerb::Register<SOP_FeE_JoinCurve_2_0Verb> theVerb;
};

const SOP_NodeVerb::Register<SOP_FeE_JoinCurve_2_0Verb> SOP_FeE_JoinCurve_2_0Verb::theVerb;

const SOP_NodeVerb *
SOP_FeE_JoinCurve_2_0::cookVerb() const 
{ 
    return SOP_FeE_JoinCurve_2_0Verb::theVerb.get();
}




static bool
sopPrimPolyIsClosed(SOP_FeE_JoinCurve_2_0Parms::PrimType parmgrouptype)
{
    using namespace SOP_FeE_JoinCurve_2_0Enums;
    switch (parmgrouptype)
    {
    case PrimType::POLYLINE:   return 0;    break;
    case PrimType::POLY:       return 1;    break;
    }
    UT_ASSERT_MSG(0, "Unhandled Prim type!");
    return 0;
}


void
SOP_FeE_JoinCurve_2_0Verb::cook(const SOP_NodeVerb::CookParms& cookparms) const
{
    auto&& sopparms = cookparms.parms<SOP_FeE_JoinCurve_2_0Parms>();
    GA_Detail& outGeo0 = *cookparms.gdh().gdpNC();
    //auto sopcache = (SOP_FeE_JoinCurve_2_0Cache*)cookparms.cache();

    const GA_Detail& inGeo0 = *cookparms.inputGeo(0);

    outGeo0.replaceWith(inGeo0);


    UT_AutoInterrupt boss("Processing");
    if (boss.wasInterrupted())
        return;

    GFE_JoinCurve joinCurve(outGeo0, cookparms);
    
    if (sopparms.getCheckInputError())
    {
        joinCurve.checkInputError();
    }

    joinCurve.groupParser.setPointGroup(sopparms.getStopPointGroup());
    joinCurve.setComputeParm(sopparms.getKeepOrder(), sopparms.getKeepLoop(), sopparms.getCloseLoop());
    if(sopparms.getOutSrcPrim())
        joinCurve.setSrcPrim(false, GA_STORE_INVALID, sopparms.getSrcPrimAttribName());
        
    joinCurve.computeAndBumpDataIdsForAddOrRemove();
    
}


