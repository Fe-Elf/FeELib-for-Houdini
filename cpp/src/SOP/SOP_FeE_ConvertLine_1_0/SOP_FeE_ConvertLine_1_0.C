
//#define UT_ASSERT_LEVEL 3
#include "SOP_FeE_ConvertLine_1_0.h"


#include "SOP_FeE_ConvertLine_1_0.proto.h"

#include "GA/GA_Detail.h"
#include "PRM/PRM_TemplateBuilder.h"
#include "UT/UT_Interrupt.h"
#include "UT/UT_DSOVersion.h"


#include "GFE/GFE_ConvertLine.h"


using namespace SOP_FeE_ConvertLine_1_0_Namespace;


static const char *theDsFile = R"THEDSFILE(
{
    name        parameters
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
        name    "attribFromPrim"
        cppname "AttribFromPrim"
        label   "Attrib from Prim"
        type    string
        default { "" }
    }
    parm {
        name    "attribFromVertex"
        cppname "AttribFromVertex"
        label   "Attrib from Vertex"
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
        name    "groupFromVertex"
        cppname "GroupFromVertex"
        label   "Group from Vertex"
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
        label   "Out Source Prim"
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
        name    "copyPrimAttrib"
        cppname "CopyPrimAttrib"
        label   "Copy Prim Attrib"
        type    toggle
        default { 0 }
        nolabel
        joinnext
    }
    parm {
        name    "copyPrimAttribName"
        cppname "CopyPrimAttribName"
        label   "Copy Prim Attrib Name"
        type    string
        default { "*" }
        disablewhen "{ copyPrimAttrib == 0 }"
    }
    parm {
        name    "copyVertexAttrib"
        cppname "CopyVertexAttrib"
        label   "Copy Vertex Attrib"
        type    toggle
        default { 0 }
        nolabel
        joinnext
    }
    parm {
        name    "copyVertexAttribName"
        cppname "CopyVertexAttribName"
        label   "Copy Vertex Attrib Name"
        type    string
        default { "*" }
        disablewhen "{ copyVertexAttrib == 0 }"
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
        name    "keepSourcePrim"
        cppname "KeepSourcePrim"
        label   "Keep Source Prim"
        type    toggle
        default { "0" }
    }




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
SOP_FeE_ConvertLine_1_0::buildTemplates()
{
    static PRM_TemplateBuilder templ("SOP_FeE_ConvertLine_1_0.C"_sh, theDsFile);
    if (templ.justBuilt())
    {
        templ.setChoiceListPtr("primGroup"_sh, &SOP_Node::primGroupMenu);
        templ.setChoiceListPtr("pointGroup"_sh, &SOP_Node::pointGroupMenu);
        templ.setChoiceListPtr("vertexGroup"_sh, &SOP_Node::vertexNamedGroupMenu);
        templ.setChoiceListPtr("edgeGroup"_sh, &SOP_Node::edgeGroupMenu);

        templ.setChoiceListPtr("attribFromPrim"_sh, &SOP_Node::primAttribMenu);
        templ.setChoiceListPtr("attribFromVertex"_sh, &SOP_Node::vertexAttribMenu);

        templ.setChoiceListPtr("groupFromPrim"_sh, &SOP_Node::primGroupMenu);
        templ.setChoiceListPtr("groupFromVertex"_sh, &SOP_Node::vertexNamedGroupMenu);
        templ.setChoiceListPtr("groupFromEdge"_sh, &SOP_Node::edgeGroupMenu);
        templ.setChoiceListPtr("promoteEdgeGroupToPrim"_sh, &SOP_Node::edgeGroupMenu);
    }
    return templ.templates();
}

const UT_StringHolder SOP_FeE_ConvertLine_1_0::theSOPTypeName("FeE::convertLine::1.0"_sh);

void
newSopOperator(OP_OperatorTable* table)
{
    OP_Operator* newOp = new OP_Operator(
        SOP_FeE_ConvertLine_1_0::theSOPTypeName,
        "FeE Convert Line",
        SOP_FeE_ConvertLine_1_0::myConstructor,
        SOP_FeE_ConvertLine_1_0::buildTemplates(),
        1,
        1,
        nullptr,
        OP_FLAG_GENERATOR,
        nullptr,
        1,
        "Five elements Elf/Convert");

    newOp->setIconName("SOP_convertline");
    table->addOperator(newOp);

}





class SOP_FeE_ConvertLine_1_0Verb : public SOP_NodeVerb
{
public:
    SOP_FeE_ConvertLine_1_0Verb() {}
    virtual ~SOP_FeE_ConvertLine_1_0Verb() {}

    virtual SOP_NodeParms *allocParms() const { return new SOP_FeE_ConvertLine_1_0Parms(); }
    virtual UT_StringHolder name() const { return SOP_FeE_ConvertLine_1_0::theSOPTypeName; }

    virtual CookMode cookMode(const SOP_NodeParms *parms) const { return COOK_GENERIC; }

    virtual void cook(const CookParms &cookparms) const;
    
    /// This static data member automatically registers
    /// this verb class at library load time.
    static const SOP_NodeVerb::Register<SOP_FeE_ConvertLine_1_0Verb> theVerb;
};

// The static member variable definition has to be outside the class definition.
// The declaration is inside the class.
const SOP_NodeVerb::Register<SOP_FeE_ConvertLine_1_0Verb> SOP_FeE_ConvertLine_1_0Verb::theVerb;

const SOP_NodeVerb *
SOP_FeE_ConvertLine_1_0::cookVerb() const 
{ 
    return SOP_FeE_ConvertLine_1_0Verb::theVerb.get();
}




static bool
sopPrimPolyIsClosed(const SOP_FeE_ConvertLine_1_0Parms::PrimType parmIsClosed)
{
    using namespace SOP_FeE_ConvertLine_1_0Enums;
    switch (parmIsClosed)
    {
    case PrimType::POLYLINE:   return 0;    break;
    case PrimType::POLY:       return 1;    break;
    }
    UT_ASSERT_MSG(0, "Unhandled Prim type!");
    return 0;
}





//// Calls functor on every active offset in this index map.
//template<typename FUNCTOR>
//SYS_FORCE_INLINE
//void forEachOffset(GA_IndexMap& idxmap, FUNCTOR&& functor)
//{
//    if (idxmap.isTrivialMap())
//    {
//        const GA_Offset end = GA_Offset(GA_Size(idxmap.indexSize()));
//        for (GA_Offset off(0); off != end; ++off)
//        {
//            functor(off, off);
//        }
//    }
//    else
//    {
//        const GA_Offset veryend(idxmap.myMaxOccupiedOffset + 1);
//        GA_Size idx(0);
//        GA_Offset off(0);
//        while (true)
//        {
//            off = idxmap.findActiveOffset(off, veryend);
//            GA_Offset end = idxmap.findInactiveOffset(off, veryend);
//            if (off == end)
//                break;
//            do
//            {
//                functor(off, idx);
//                ++off;
//                ++idx;
//            } while (off != end);
//        }
//    }
//}




/*
template<typename FUNCTOR>
static void forEachOffset(FUNCTOR&& functor, const GA_IndexMap& index_map, const GA_ElementGroup* group = nullptr, bool complement = false)
{
    // Fall back to regular case if no group.
    //if (!group)
    //{
    //    if (!complement)
    //        index_map.forEachOffset(functor);
    //    return;
    //}

    // Group order is only relevant if not complemented.
    if (!complement)
    {
        const GA_ElementGroupOrder* order = group->getOrdered();
        if (order)
        {
            GA_Size idx(0);
            for (GA_ElementGroupOrderIndex i(0), n(order->entries()); i != n; ++i)
            {
                GA_Offset off = order->getElement(i);
                functor(off, idx);
                ++idx;
            }
            return;
        }
    }

    // We have a group, treated as unordered.
    const GA_Offset veryend = index_map.offsetSize();
    GA_Size idx(0);
    GA_Offset off(0);
    while (true)
    {
        bool value;
        GA_Size span_size;
        group->getConstantSpan(off, veryend, span_size, value);
        if (span_size == 0)
            break;
        if (value == complement)
        {
            off += span_size;
            continue;
        }
        const GA_Offset span_end = off + span_size;
        while (true)
        {
            off = index_map.findActiveOffset(off, span_end);
            GA_Offset end = index_map.findInactiveOffset(off, span_end);
            if (off == end)
                break;
            do
            {
                functor(off, idx);
                ++off;
                ++idx;
            } while (off != end);
        }
    }
}

template<typename FUNCTOR>
SYS_FORCE_INLINE
void forEachPrimitive(GA_Detail* geo, const GA_PrimitiveGroup* group, bool complement, FUNCTOR&& functor)
{
    forEachOffset(functor, geo->getPrimitiveMap(), group, complement);
}

*/


//template<typename FUNCTOR>
//SYS_FORCE_INLINE
//void forEachVertex(GA_Detail* geo, const GA_VertexGroup* group, bool complement, FUNCTOR&& functor)
//{
//    forEachOffset(functor, geo->getVertexMap(), group, complement);
//}






void
SOP_FeE_ConvertLine_1_0Verb::cook(const SOP_NodeVerb::CookParms &cookparms) const
{
    auto&& sopparms = cookparms.parms<SOP_FeE_ConvertLine_1_0Parms>();
    GA_Detail& outGeo0 = *cookparms.gdh().gdpNC();
    //auto sopcache = (SOP_FeE_ConvertLine_1_0Cache*)cookparms.cache();

    const GA_Detail& inGeo0 = *cookparms.inputGeo(0);


    
    const UT_StringHolder& primGroupName   = sopparms.getPrimGroup();
    const UT_StringHolder& pointGroupName  = sopparms.getPointGroup();
    const UT_StringHolder& vertexGroupName = sopparms.getVertexGroup();
    const UT_StringHolder& edgeGroupName   = sopparms.getEdgeGroup();

    const bool isClosed = sopPrimPolyIsClosed(sopparms.getPrimType());


    UT_AutoInterrupt boss("Processing");
    if (boss.wasInterrupted())
        return;


    GFE_ConvertLine convertLine(outGeo0, &inGeo0, &cookparms);

    convertLine.setCopyPrimitiveAttrib(sopparms.getCopyPrimAttrib(), sopparms.getCopyPrimAttribName());
    convertLine.setCopyVertexAttrib(sopparms.getCopyVertexAttrib(), sopparms.getCopyVertexAttribName());

    convertLine.setComputeParm(isClosed, sopparms.getKeepSourcePrim());
    if (sopparms.getOutSrcPrim())
        convertLine.createSrcPrimAttrib(false, GA_STORE_INVALID, sopparms.getSrcPrimAttribName());

    convertLine.groupParser.setPrimitiveGroup(sopparms.getPrimGroup());

    convertLine.computeAndBumpDataIdsForAddOrRemove();
    
}

