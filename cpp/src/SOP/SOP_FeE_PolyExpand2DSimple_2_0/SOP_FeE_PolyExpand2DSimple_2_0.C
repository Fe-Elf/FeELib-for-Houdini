
//#define UT_ASSERT_LEVEL 3
#include "SOP_FeE_PolyExpand2DSimple_2_0.h"


#include "SOP_FeE_PolyExpand2DSimple_2_0.proto.h"

#include "GA/GA_Detail.h"
#include "PRM/PRM_TemplateBuilder.h"
#include "UT/UT_Interrupt.h"
#include "UT/UT_DSOVersion.h"




#include "GFE/GFE_PolyCut.h"


using namespace SOP_FeE_PolyExpand2DSimple_2_0_Namespace;


static const char *theDsFile = R"THEDSFILE(
{
    name	parameters
    parm {
        name    "primGroup"
        cppname "PrimGroup"
        label   "Prim Group"
        type    string
        default { "" }
        parmtag { "script_action" "import soputils\nkwargs['geometrytype'] = (hou.geometryType.Primitives, )\nkwargs['inputindex'] = 0\nsoputils.selectGroupParm(kwargs)" }
        parmtag { "script_action_help" "Select geometry from an available viewport.\nShift-click to turn on Select Groups." }
        parmtag { "script_action_icon" "BUTTONS_reselect" }
    }
    parm {
        name    "cutPointGroup"
        cppname "CutPointGroup"
        label   "Cut Point Group"
        type    string
        default { "" }
        parmtag { "script_action" "import soputils\nkwargs['geometrytype'] = (hou.geometryType.Points, )\nkwargs['inputindex'] = 0\nsoputils.selectGroupParm(kwargs)" }
        parmtag { "script_action_help" "Select geometry from an available viewport.\nShift-click to turn on Select Groups." }
        parmtag { "script_action_icon" "BUTTONS_reselect" }
    }
    parm {
        name    "mergePrimEndsIfClosed"
        cppname "MergePrimEndsIfClosed"
        label   "Merge Prim Ends if Closed "
        type    toggle
        default { "1" }
    }
    parm {
        name    "cutPoint"
        cppname "CutPoint"
        label   "Cut Point"
        type    toggle
        default { "0" }
    }
    parm {
        name    "polyType"
        cppname "PolyType"
        label   "Prim Type"
        type    ordinal
        default { "auto" }
        menu {
            "auto"          "Auto Detect"
            "polyline"      "Poly Line"
            "poly"          "Poly"
        }
    }

    parm {
        name    "createSrcPrimAttrib"
        cppname "CreateSrcPrimAttrib"
        label   "Create Source Primitive Attribute"
        type    toggle
        nolabel
        joinnext
        default { "0" }
    }
    parm {
        name    "srcPrimAttribName"
        cppname "SrcPrimAttribName"
        label   "Source Primitive Attribute Name"
        type    string
        default { "srcPrim" }
        disablewhen "{ createSrcPrimAttrib == 0 }"
    }
    parm {
        name    "createSrcPointAttrib"
        cppname "CreateSrcPointAttrib"
        label   "Create Source Point Attribute"
        type    toggle
        nolabel
        joinnext
        default { "off" }
        disablewhen "{ cutPoint == 0 }"
    }
    parm {
        name    "srcPointAttribName"
        cppname "SrcPointAttribName"
        label   "Source Point Attribute Name"
        type    string
        default { "srcPoint" }
        disablewhen "{ createSrcPointAttrib == 0 } { cutPoint == 0 }"
    }
    parm {
        name    "sepparm"
        label   "Separator"
        type    separator
        default { "" }
    }

    parm {
        name    "keepPrimAttribName"
        cppname "KeepPrimAttribName"
        label   "Keep Primitive Attribute Name"
        type    string
        default { "" }
    }
    parm {
        name    "keepPointAttribName"
        cppname "KeepPointAttribName"
        label   "Keep Point Attribute Name"
        type    string
        default { "*" }
        disablewhen "{ cutPoint == 0 }"
    }

    parm {
        name    "keepPrimGroupName"
        cppname "KeepPrimGroupName"
        label   "Keep Primitive Group Name"
        type    string
        default { "" }
    }
    parm {
        name    "keepPointGroupName"
        cppname "KeepPointGroupName"
        label   "Keep Point Group Name"
        type    string
        default { "*" }
        disablewhen "{ cutPoint == 0 }"
    }
    parm {
        name    "keepEdgeGroupName"
        cppname "KeepEdgeGroupName"
        label   "Keep Edge Group Name"
        type    string
        default { "" }
        disablewhen "{ cutPoint == 0 }"
    }

    parm {
        name    "delInputPointGroup"
        cppname "DelInputPointGroup"
        label   "Delete Input Point Group"
        type    toggle
        default { "0" }
    }


    //parm {
    //    name    "kernel"
    //    cppname "Kernel"
    //    label   "Kernel"
    //    type    integer
    //    default { 0 }
    //    range   { 0! 1! }
    //}



    //parm {
    //    name    "subscribeRatio"
    //    cppname "SubscribeRatio"
    //    label   "Subscribe Ratio"
    //    type    integer
    //    default { 64 }
    //    range   { 0! 256 }
    //}
    //parm {
    //    name    "minGrainSize"
    //    cppname "MinGrainSize"
    //    label   "Min Grain Size"
    //    type    intlog
    //    default { 64 }
    //    range   { 0! 2048 }
    //}
}
)THEDSFILE";

PRM_Template*
SOP_FeE_PolyExpand2DSimple_2_0::buildTemplates()
{
    static PRM_TemplateBuilder templ("SOP_FeE_PolyExpand2DSimple_2_0.C"_sh, theDsFile);
    if (templ.justBuilt())
    {
        templ.setChoiceListPtr("cutPointGroup"_sh, &SOP_Node::pointGroupMenu);
        templ.setChoiceListPtr("primGroup"_sh, &SOP_Node::primGroupMenu);

        templ.setChoiceListPtr("keepPrimAttribName"_sh, &SOP_Node::primAttribMenu);
        templ.setChoiceListPtr("keepPointAttribName"_sh, &SOP_Node::pointAttribMenu);
        templ.setChoiceListPtr("keepPrimGroupName"_sh, &SOP_Node::primGroupMenu);
        templ.setChoiceListPtr("keepPointGroupName"_sh, &SOP_Node::pointGroupMenu);
        templ.setChoiceListPtr("keepEdgeGroupName"_sh, &SOP_Node::edgeGroupMenu);
    }
    return templ.templates();
}





const UT_StringHolder SOP_FeE_PolyExpand2DSimple_2_0::theSOPTypeName("FeE::polyExpand2d_simple::2.0"_sh);

void
newSopOperator(OP_OperatorTable* table)
{
    OP_Operator* newOp = new OP_Operator(
        SOP_FeE_PolyExpand2DSimple_2_0::theSOPTypeName,
        "FeE Polygon Expand 2D Simple",
        SOP_FeE_PolyExpand2DSimple_2_0::myConstructor,
        SOP_FeE_PolyExpand2DSimple_2_0::buildTemplates(),
        1,
        1,
        nullptr,
        OP_FLAG_GENERATOR,
        nullptr,
        1,
        "Five elements Elf/Generator/Poly Expand");

    newOp->setIconName("SOP_polyexpand2d");
    table->addOperator(newOp);
}




class SOP_FeE_PolyExpand2DSimple_2_0Verb : public SOP_NodeVerb
{
public:
    SOP_FeE_PolyExpand2DSimple_2_0Verb() {}
    virtual ~SOP_FeE_PolyExpand2DSimple_2_0Verb() {}

    virtual SOP_NodeParms *allocParms() const { return new SOP_FeE_PolyExpand2DSimple_2_0Parms(); }
    virtual UT_StringHolder name() const { return SOP_FeE_PolyExpand2DSimple_2_0::theSOPTypeName; }

    virtual CookMode cookMode(const SOP_NodeParms *parms) const { return COOK_GENERIC; }

    virtual void cook(const CookParms &cookparms) const;
    
    static const SOP_NodeVerb::Register<SOP_FeE_PolyExpand2DSimple_2_0Verb> theVerb;
};

const SOP_NodeVerb::Register<SOP_FeE_PolyExpand2DSimple_2_0Verb> SOP_FeE_PolyExpand2DSimple_2_0Verb::theVerb;

const SOP_NodeVerb *
SOP_FeE_PolyExpand2DSimple_2_0::cookVerb() const 
{ 
    return SOP_FeE_PolyExpand2DSimple_2_0Verb::theVerb.get();
}








static GFE_PolyCutType
sopPolyType(SOP_FeE_PolyExpand2DSimple_2_0Parms::PolyType parmgrouptype)
{
    using namespace SOP_FeE_PolyExpand2DSimple_2_0Enums;
    switch (parmgrouptype)
    {
    case PolyType::AUTO:       return GFE_PolyCutType::AUTO;    break;
    case PolyType::POLYLINE:   return GFE_PolyCutType::OPEN;    break;
    case PolyType::POLY:       return GFE_PolyCutType::CLOSE;    break;
    }
    UT_ASSERT_MSG(0, "Unhandled Poly type!");
    return GFE_PolyCutType::AUTO;
}



void
SOP_FeE_PolyExpand2DSimple_2_0Verb::cook(const SOP_NodeVerb::CookParms& cookparms) const
{
    auto&& sopparms = cookparms.parms<SOP_FeE_PolyExpand2DSimple_2_0Parms>();
    GA_Detail& outGeo0 = *cookparms.gdh().gdpNC();
    //auto sopcache = (SOP_FeE_PolyExpand2DSimple_2_0Cache*)cookparms.cache();

    const GA_Detail& inGeo0 = *cookparms.inputGeo(0);

    outGeo0.replaceWith(inGeo0);



    //GA_PointGroup* groupOneNeb = GFE_TopologyReference::addGroupOneNeb(outGeo0, nullptr);


    const bool mergePrimEndsIfClosed = sopparms.getMergePrimEndsIfClosed();
    const bool cutPoint = sopparms.getCutPoint();
    const GFE_PolyCutType polyType = sopPolyType(sopparms.getPolyType());


    //const GA_Storage inStorageI = GFE_Type::getPreferredStorageI(outGeo0);

    UT_AutoInterrupt boss("Processing");
    if (boss.wasInterrupted())
        return;
    

    const UT_StringHolder& keepPrimAttribName  = sopparms.getKeepPrimAttribName();
    const UT_StringHolder& keepPointAttribName = sopparms.getKeepPointAttribName();
    const UT_StringHolder& keepPrimGroupName   = sopparms.getKeepPrimGroupName();
    const UT_StringHolder& keepPointGroupName  = sopparms.getKeepPointGroupName();
    const UT_StringHolder& keepEdgeGroupName   = sopparms.getKeepEdgeGroupName();

    const bool delInputPointGroup = sopparms.getDelInputPointGroup();


    const UT_StringHolder& emptyStr = "";

    const bool createSrcPrimAttrib = sopparms.getCreateSrcPrimAttrib();
    const UT_StringHolder& srcPrimAttribName = createSrcPrimAttrib ? sopparms.getSrcPrimAttribName() : emptyStr;

    const bool createSrcPointAttrib = sopparms.getCreateSrcPointAttrib() && cutPoint;
    const UT_StringHolder& srcPointAttribName = createSrcPointAttrib ? sopparms.getSrcPointAttribName() : emptyStr;

    
    GFE_PolyCut polyCut(outGeo0, &inGeo0, &cookparms);
    polyCut.setComputeParm(cutPoint, mergePrimEndsIfClosed, polyType);

    polyCut.setGroup(sopparms.getCutPointGroup(), sopparms.getPrimGroup());

    if (createSrcPrimAttrib)
        polyCut.createSrcPrimAttrib(srcPrimAttribName);

    if (createSrcPointAttrib)
        polyCut.createSrcPointAttrib(srcPointAttribName);

    polyCut.groupParser_cutPoint.setDelGroup(delInputPointGroup);

    polyCut.computeAndBumpDataIdsForAddOrRemove();

}

