
//#define UT_ASSERT_LEVEL 3

#include "SOP_FeE_MeshTopology_1_0.h"
#include "SOP_FeE_MeshTopology_1_0.proto.h"

#include <GFE/GFE_MeshTopology.h>

using namespace SOP_FeE_MeshTopology_1_0_Namespace;

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
        parmtag { "sop_input" "0" }
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
        name    "pointSeamGroup"
        cppname "PointSeamGroup"
        label   "Point Seam Group"
        type    string
        default { "" }
        parmtag { "script_action" "import soputils\nkwargs['geometrytype'] = (hou.geometryType.Points,)\nkwargs['inputindex'] = 0\nsoputils.selectGroupParm(kwargs)" }
        parmtag { "script_action_help" "Select geometry from an available viewport.\nShift-click to turn on Select Groups." }
        parmtag { "script_action_icon" "BUTTONS_reselect" }
        parmtag { "sop_input" "0" }
    }
    parm {
        name    "vertexEdgeSeamGroup"
        cppname "VertexEdgeSeamGroup"
        label   "Vertex Edge Seam Group"
        type    string
        default { "" }
        parmtag { "script_action" "import soputils\nkwargs['geometrytype'] = (hou.geometryType.Vertices,)\nkwargs['inputindex'] = 0\nsoputils.selectGroupParm(kwargs)" }
        parmtag { "script_action_help" "Select geometry from an available viewport.\nShift-click to turn on Select Groups." }
        parmtag { "script_action_icon" "BUTTONS_reselect" }
        parmtag { "sop_input" "0" }
    }
    parm {
        name    "edgeSeamGroup"
        cppname "EdgeSeamGroup"
        label   "Edge Seam Group"
        type    string
        default { "" }
        parmtag { "script_action" "import soputils\nkwargs['geometrytype'] = (hou.geometryType.Edges,)\nkwargs['inputindex'] = 0\nsoputils.selectGroupParm(kwargs)" }
        parmtag { "script_action_help" "Select geometry from an available viewport.\nShift-click to turn on Select Groups." }
        parmtag { "script_action_icon" "BUTTONS_reselect" }
        parmtag { "sop_input" "0" }
    }





    parm {
        name    "calVertexPrimIndex"
        cppname "CalVertexPrimIndex"
        label   "Calculate Vertex Prim Offset"
        type    toggle
        default { "0" }
        nolabel
        joinnext
    }
    parm {
        name    "vertexPrimIndexAttribName"
        cppname "VertexPrimIndexAttribName"
        label   "Vertex Prim Index Attribute Name"
        type    string
        default { "vtxpnum" }
        disablewhen "{ calVertexPrimIndex == 0 }"
    }

    parm {
        name    "calVertexVertexPrim"
        cppname "CalVertexVertexPrim"
        label   "Calculate Vertex Vertex Prim"
        type    toggle
        default { "0" }
    }
    parm {
        name    "vertexVertexPrimPrevAttribName"
        cppname "VertexVertexPrimPrevAttribName"
        label   "Vertex Vertex Prim Prev Attribute Name"
        type    string
        default { "vtxPrimPrev" }
        disablewhen "{ calVertexVertexPrim == 0 }"
    }
    parm {
        name    "vertexVertexPrimNextAttribName"
        cppname "VertexVertexPrimNextAttribName"
        label   "Vertex Vertex Prim Next Attribute Name"
        type    string
        default { "vtxPrimNext" }
        disablewhen "{ calVertexVertexPrim == 0 }"
    }



    parm {
        name    "calVertexPointDst"
        cppname "CalVertexPointDst"
        label   "Calculate Vertex Point Destination"
        type    toggle
        default { "0" }
        nolabel
        joinnext
    }
    parm {
        name    "vertexPointDstAttribName"
        cppname "VertexPointDstAttribName"
        label   "Vertex Point Destination Attribute Name"
        type    string
        default { "dstpt" }
        disablewhen "{ calVertexPointDst == 0 }"
    }

    parm {
        name    "calVertexNextEquiv"
        cppname "CalVertexNextEquiv"
        label   "Calculate Vertex Next Equiv"
        type    toggle
        default { "0" }
        nolabel
        joinnext
    }
    parm {
        name    "vertexNextEquivAttribName"
        cppname "VertexNextEquivAttribName"
        label   "Vertex Next Equiv Attribute Name"
        type    string
        default { "nextEquiv" }
        disablewhen "{ calVertexNextEquiv == 0 }"
    }
    parm {
        name    "calVertexNextEquivGroup"
        cppname "CalVertexNextEquivGroup"
        label   "Calculate Vertex Next Equiv Group"
        type    toggle
        default { "0" }
        nolabel
        joinnext
    }
    parm {
        name    "vertexNextEquivGroupName"
        cppname "VertexNextEquivGroupName"
        label   "Vertex Next Equiv Group Name"
        type    string
        default { "nextEquiv" }
        disablewhen "{ calVertexNextEquivGroup == 0 }"
    }

    parm {
        name    "calVertexNextEquivNoLoop"
        cppname "CalVertexNextEquivNoLoop"
        label   "Calculate Vertex Next Equiv No Loop"
        type    toggle
        default { "0" }
        nolabel
        joinnext
    }
    parm {
        name    "vertexNextEquivNoLoopAttribName"
        cppname "VertexNextEquivNoLoopAttribName"
        label   "Vertex Next Equiv No Loop Attribute Name"
        type    string
        default { "nextEquivNoLoop" }
        disablewhen "{ calVertexNextEquivNoLoop == 0 }"
    }
    parm {
        name    "calVertexNextEquivNoLoopGroup"
        cppname "CalVertexNextEquivNoLoopGroup"
        label   "Calculate Vertex Next Equiv No Loop Group"
        type    toggle
        default { "0" }
        nolabel
        joinnext
    }
    parm {
        name    "vertexNextEquivNoLoopGroupName"
        cppname "VertexNextEquivNoLoopGroupName"
        label   "Vertex Next Equiv No Loop Group Name"
        type    string
        default { "nextEquivNoLoop" }
        disablewhen "{ calVertexNextEquivNoLoopGroup == 0 }"
    }


    parm {
        name    "calPointPointEdge"
        cppname "CalPointPointEdge"
        label   "Calculate Point Point Edge"
        type    toggle
        default { "0" }
        nolabel
        joinnext
    }
    parm {
        name    "pointPointEdgeAttribName"
        cppname "PointPointEdgeAttribName"
        label   "Point Point Edge Attribute Name"
        type    string
        default { "nebs" }
        disablewhen "{ calPointPointEdge == 0 }"
    }

    parm {
        name    "calPointPointPrim"
        cppname "CalPointPointPrim"
        label   "Calculate Point Point Prim"
        type    toggle
        default { "0" }
        nolabel
        joinnext
    }
    parm {
        name    "pointPointPrimAttribName"
        cppname "PointPointPrimAttribName"
        label   "Point Point Prim Attribute Name"
        type    string
        default { "nebsPrim" }
        disablewhen "{ calPointPointPrim == 0 }"
    }

    parm {
        name    "calPrimPrimEdge"
        cppname "CalPrimPrimEdge"
        label   "Calculate Prim Prim Edge"
        type    toggle
        default { "0" }
        nolabel
        joinnext
    }
    parm {
        name    "primPrimEdgeAttribName"
        cppname "PrimPrimEdgeAttribName"
        label   "Prim Prim Edge Attribute Name"
        type    string
        default { "nebs" }
        disablewhen "{ calPrimPrimEdge == 0 }"
    }

    parm {
        name    "calPrimPrimPoint"
        cppname "CalPrimPrimPoint"
        label   "Calculate Prim Prim Point"
        type    toggle
        default { "0" }
        nolabel
        joinnext
    }
    parm {
        name    "primPrimPointAttribName"
        cppname "PrimPrimPointAttribName"
        label   "Prim Prim Point Attribute Name"
        type    string
        default { "nebsPoint" }
        disablewhen "{ calPrimPrimPoint == 0 }"
    }

    parm {
        name    "outAsOffset"
        cppname "OutAsOffset"
        label   "Output as Offset"
        type    toggle
        default { "1" }
    }
    parm {
        name    "findInputAttrib"
        cppname "FindInputAttrib"
        label   "Find Input Attribute"
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
    parm {
        name    "outArrayType"
        cppname "OutArrayType"
        label   "Output Array Type"
        type    ordinal
        default { "guess" }
        menu {
            "attrib"    "Attrib"
            "packed"    "Packed"
            "geo"       "Geo"
        }
    }
    parm {
       name    "kernel"
       cppname "Kernel"
       label   "Kernel"
       type    integer
       default { 0 }
       range   { 0! 2! }
    }
}
)THEDSFILE";


PRM_Template*
SOP_FeE_MeshTopology_1_0::buildTemplates()
{
    static PRM_TemplateBuilder templ("SOP_FeE_MeshTopology_1_0.C"_sh, theDsFile);
    if (templ.justBuilt())
    {
        templ.setChoiceListPtr("group"_sh, &SOP_Node::allGroupMenu);
    }
    return templ.templates();
}

const UT_StringHolder SOP_FeE_MeshTopology_1_0::theSOPTypeName("FeE::meshTopology::1.0"_sh);

void
newSopOperator(OP_OperatorTable* table)
{
    OP_Operator* newOp = new OP_Operator(
        SOP_FeE_MeshTopology_1_0::theSOPTypeName,
        "FeE Mesh Topology",
        SOP_FeE_MeshTopology_1_0::myConstructor,
        SOP_FeE_MeshTopology_1_0::buildTemplates(),
        1,
        1,
        nullptr,
        OP_FLAG_GENERATOR,
        nullptr,
        1,
        "Five elements Elf/Data/Topology");

    newOp->setIconName("SOP_primitive");
    table->addOperator(newOp);
}

//class SOP_FeE_MeshTopology_1_0Cache : public SOP_NodeCache
//{
//public:
//    SOP_FeE_MeshTopology_1_0Cache() : SOP_NodeCache(),
//        myPrevOutputDetailID(-1)
//    {}
//    ~SOP_FeE_MeshTopology_1_0Cache() override {}
//
//    int64 myPrevOutputDetailID;
//};


class SOP_FeE_MeshTopology_1_0Verb : public SOP_NodeVerb
{
public:
    SOP_FeE_MeshTopology_1_0Verb() {}
    virtual ~SOP_FeE_MeshTopology_1_0Verb() {}

    virtual SOP_NodeParms *allocParms() const { return new SOP_FeE_MeshTopology_1_0Parms(); }
    //virtual SOP_NodeCache* allocCache() const { return new SOP_FeE_MeshTopology_1_0Cache(); }
    virtual UT_StringHolder name() const { return SOP_FeE_MeshTopology_1_0::theSOPTypeName; }

    virtual CookMode cookMode(const SOP_NodeParms *parms) const { return COOK_INPLACE; }

    virtual void cook(const CookParms &cookparms) const;

    /// This static data member automatically registers
    /// this verb class at library ldir0d time.
    static const SOP_NodeVerb::Register<SOP_FeE_MeshTopology_1_0Verb> theVerb;
};

// The static member variable definition has to be outside the class definition.
// The declaration is inside the class.
const SOP_NodeVerb::Register<SOP_FeE_MeshTopology_1_0Verb> SOP_FeE_MeshTopology_1_0Verb::theVerb;

const SOP_NodeVerb *
SOP_FeE_MeshTopology_1_0::cookVerb() const 
{ 
    return SOP_FeE_MeshTopology_1_0Verb::theVerb.get();
}





static gfe::OutArrayType
sopOutArrayType(const SOP_FeE_MeshTopology_1_0Parms::OutArrayType parmGroupType)
{
    using namespace SOP_FeE_MeshTopology_1_0Enums;
    switch (parmGroupType)
    {
    case OutArrayType::ATTRIB:     return gfe::OutArrayType::Attrib; break;
    case OutArrayType::PACKED:     return gfe::OutArrayType::Packed; break;
    case OutArrayType::GEO:        return gfe::OutArrayType::Geo;    break;
    }
    UT_ASSERT_MSG(0, "Unhandled Group Type!");
    return gfe::OutArrayType::Attrib;
}

static GA_GroupType
sopGroupType(const SOP_FeE_MeshTopology_1_0Parms::GroupType parmGroupType)
{
    using namespace SOP_FeE_MeshTopology_1_0Enums;
    switch (parmGroupType)
    {
    case GroupType::GUESS:     return GA_GROUP_INVALID;    break;
    case GroupType::PRIM:      return GA_GROUP_PRIMITIVE;  break;
    case GroupType::POINT:     return GA_GROUP_POINT;      break;
    case GroupType::VERTEX:    return GA_GROUP_VERTEX;     break;
    case GroupType::EDGE:      return GA_GROUP_EDGE;       break;
    }
    UT_ASSERT_MSG(0, "Unhandled Group Type!");
    return GA_GROUP_INVALID;
}

void
SOP_FeE_MeshTopology_1_0Verb::cook(const SOP_NodeVerb::CookParms& cookparms) const
{
    auto&& sopparms = cookparms.parms<SOP_FeE_MeshTopology_1_0Parms>();
    GA_Detail& outGeo0 = *cookparms.gdh().gdpNC();
    //auto sopcache = (SOP_FeE_MeshTopology_1_0Cache*)cookparms.cache();

    //const GA_Detail& inGeo0 = *cookparms.inputGeo(0);

    //outGeo0.replaceWith(inGeo0);

    //outGeo0 = sopNodeProcess(*inGeo0);

    bool calVertexPrimIndex            = sopparms.getCalVertexPrimIndex();
    bool calVertexVertexPrim           = sopparms.getCalVertexVertexPrim();
    bool calVertexPointDst             = sopparms.getCalVertexPointDst();
    bool calVertexNextEquiv            = sopparms.getCalVertexNextEquiv();
    bool calVertexNextEquivNoLoop      = sopparms.getCalVertexNextEquivNoLoop();
    bool calVertexNextEquivGroup       = sopparms.getCalVertexNextEquivGroup();
    bool calVertexNextEquivNoLoopGroup = sopparms.getCalVertexNextEquivNoLoopGroup();
    bool calPointPointEdge             = sopparms.getCalPointPointEdge();
    bool calPointPointPrim             = sopparms.getCalPointPointPrim();
    bool calPrimPrimEdge               = sopparms.getCalPrimPrimEdge();
    bool calPrimPrimPoint              = sopparms.getCalPrimPrimPoint();
    
#if 1
    const UT_StringHolder& vertexPrimIndexAttribName             = sopparms.getVertexPrimIndexAttribName();
    const UT_StringHolder& vertexVertexPrimNextAttribName        = sopparms.getVertexVertexPrimNextAttribName();
    const UT_StringHolder& vertexPointDstAttribName              = sopparms.getVertexPointDstAttribName();
    const UT_StringHolder& vertexNextEquivAttribName             = sopparms.getVertexNextEquivAttribName();
    const UT_StringHolder& vertexNextEquivNoLoopAttribName       = sopparms.getVertexNextEquivNoLoopAttribName();
    const UT_StringHolder& vertexNextEquivGroupName              = sopparms.getVertexNextEquivGroupName();
    const UT_StringHolder& vertexNextEquivNoLoopGroupName        = sopparms.getVertexNextEquivNoLoopGroupName();
    const UT_StringHolder& pointPointEdgeAttribName              = sopparms.getPointPointEdgeAttribName();
    const UT_StringHolder& pointPointPrimAttribName              = sopparms.getPointPointPrimAttribName();
    const UT_StringHolder& primPrimEdgeAttribName                = sopparms.getPrimPrimEdgeAttribName();
    const UT_StringHolder& primPrimPointAttribName               = sopparms.getPrimPrimPointAttribName();
    const UT_StringHolder& vertexVertexPrimPrevAttribName        = sopparms.getVertexVertexPrimPrevAttribName();

    calVertexPrimIndex            = calVertexPrimIndex            && gfe::isValid(vertexPrimIndexAttribName);
    calVertexVertexPrim           = calVertexVertexPrim           && (gfe::isValid(vertexVertexPrimPrevAttribName) || gfe::isValid(vertexVertexPrimNextAttribName));
    calVertexPointDst             = calVertexPointDst             && gfe::isValid(vertexPointDstAttribName);
    calVertexNextEquiv            = calVertexNextEquiv            && gfe::isValid(vertexNextEquivAttribName);
    calVertexNextEquivNoLoop      = calVertexNextEquivNoLoop      && gfe::isValid(vertexNextEquivNoLoopAttribName);
    calVertexNextEquivGroup       = calVertexNextEquivGroup       && gfe::isValid(vertexNextEquivGroupName);
    calVertexNextEquivNoLoopGroup = calVertexNextEquivNoLoopGroup && gfe::isValid(vertexNextEquivNoLoopGroupName);
    calPointPointEdge             = calPointPointEdge             && gfe::isValid(pointPointEdgeAttribName);
    calPointPointPrim             = calPointPointPrim             && gfe::isValid(pointPointPrimAttribName);
    calPrimPrimEdge               = calPrimPrimEdge               && gfe::isValid(primPrimEdgeAttribName);
    calPrimPrimPoint              = calPrimPrimPoint              && gfe::isValid(primPrimPointAttribName);
#else
    UT_StringHolder& pointPointEdgeAttribName;
    UT_StringHolder& pointPointPrimAttribName;
    UT_StringHolder& primPrimEdgeAttribName;
    UT_StringHolder& primPrimPointAttribName;
    UT_StringHolder& vertexNextEquivAttribName;
    if (calPointPointEdge)
    {
        pointPointEdgeAttribName = sopparms.getPointPointEdgeAttribName();
        calPointPointEdge = pointPointEdgeAttribName.isstring();
    }
    if (calPointPointPrim)
    {
        pointPointPrimAttribName = sopparms.getPointPointPrimAttribName();
        calPointPointPrim = pointPointPrimAttribName.isstring();
    }
    if (calPrimPrimEdge)
    {
        primPrimEdgeAttribName = sopparms.getPrimPrimEdgeAttribName();
        calPrimPrimEdge = primPrimEdgeAttribName.isstring();
    }
    if (calPrimPrimPoint)
    {
        primPrimPointAttribName = sopparms.getPrimPrimPointAttribName();
        calPrimPrimPoint = primPrimPointAttribName.isstring();
    }
    if (calVertexNextEquiv)
    {
        calVertexNextEquiv = sopparms.getVertexNextEquivAttribName();
        calVertexNextEquiv = calVertexNextEquiv.isstring();
    }
#endif
    if (
        !calVertexPrimIndex &&
        !calVertexVertexPrim &&
        !calVertexPointDst &&
        !calVertexNextEquiv &&
        !calVertexNextEquivNoLoop &&
        !calVertexNextEquivGroup &&
        !calVertexNextEquivNoLoopGroup &&
        !calPointPointEdge &&
        !calPointPointPrim &&
        !calPrimPrimEdge &&
        !calPrimPrimPoint
        )
        return;




    const UT_StringHolder& pointSeamGroupName = sopparms.getPointSeamGroup();
    //const bool& usepointSeamGroup = pointSeamGroupName.isstring() && (calPointPointEdge || calPointPointPrim || calPrimPrimPoint);

    const UT_StringHolder& vertexEdgeSeamGroupName = sopparms.getVertexEdgeSeamGroup();
    //const bool& useVertexEdgeSeamGroup = vertexEdgeSeamGroupName.isstring() && calPrimPrimEdge;

    const UT_StringHolder& edgeSeamGroupName = sopparms.getEdgeSeamGroup();
    //const bool& useEdgeSeamGroup = edgeSeamGroupName.isstring() && calPrimPrimEdge;


    const GA_GroupType groupType = sopGroupType(sopparms.getGroupType());
    const gfe::OutArrayType outArrayType = sopOutArrayType(sopparms.getOutArrayType());


    UT_AutoInterrupt boss("Processing");
    if (boss.wasInterrupted())
        return;

    
    gfe::MeshTopology meshTopology(outGeo0, &cookparms);

#if __GFE_MeshTopology_TestSpped
    meshTopology.setKernel(sopparms.getKernel());
#endif
    
    meshTopology.outIntermediateAttrib = sopparms.getOutTopoAttrib();
    //meshTopology.outTopoAttrib = sopparms.getOutTopoAttrib();

    meshTopology.setComputeParm(sopparms.getOutAsOffset(), sopparms.getSubscribeRatio(), sopparms.getMinGrainSize());
    meshTopology.outArrayType = outArrayType;
    
    meshTopology.groupParser.setGroup(groupType, sopparms.getGroup());
    //meshTopology.getOutAttribArray().findOrCreateTuple(false, attribClass, storageClass, GA_STORE_INVALID, sopparms.getAttribName());


    if (calVertexPrimIndex)
        meshTopology.setVertexPrimIndex(false, vertexPrimIndexAttribName, GA_STORE_INVALID, sopparms.getFindInputAttrib());
    if (calVertexVertexPrim)
        meshTopology.setVertexVertexPrim(false,
            vertexVertexPrimPrevAttribName, vertexVertexPrimNextAttribName,
            GA_STORE_INVALID, sopparms.getFindInputAttrib());
    if (calVertexPointDst)
        meshTopology.setVertexPointDst(false, vertexPointDstAttribName, GA_STORE_INVALID, sopparms.getFindInputAttrib());

    
    if (calVertexNextEquiv)
        meshTopology.setVertexNextEquiv(false, vertexNextEquivAttribName, GA_STORE_INVALID, sopparms.getFindInputAttrib());
    if (calVertexNextEquivNoLoop)
        meshTopology.setVertexNextEquivNoLoop(false, vertexNextEquivNoLoopAttribName, GA_STORE_INVALID, sopparms.getFindInputAttrib());
    
    if (calVertexNextEquivGroup)
        meshTopology.setVertexNextEquivGroup(false, vertexNextEquivGroupName, sopparms.getFindInputAttrib());
    if (calVertexNextEquivNoLoopGroup)
        meshTopology.setVertexNextEquivNoLoopGroup(false, vertexNextEquivNoLoopGroupName, sopparms.getFindInputAttrib());


    
    if (calPointPointEdge)
        meshTopology.setPointPointEdge(false, pointPointEdgeAttribName, GA_STORE_INVALID, sopparms.getFindInputAttrib());
    if (calPointPointPrim)
        meshTopology.setPointPointPrim(false, pointPointPrimAttribName, GA_STORE_INVALID, sopparms.getFindInputAttrib());
    
    if (calPrimPrimEdge)
        meshTopology.setPrimPrimEdge(false, primPrimEdgeAttribName, GA_STORE_INVALID, sopparms.getFindInputAttrib());
    if (calPrimPrimPoint)
        meshTopology.setPrimPrimPoint(false, primPrimPointAttribName, GA_STORE_INVALID, sopparms.getFindInputAttrib());

    meshTopology.computeAndBumpDataId();
    meshTopology.visualizeOutGroup();
    
}

