
//#define UT_ASSERT_LEVEL 3
#include "SOP_FeE_OffsetVertexOrder_2_0.h"
#include "SOP_FeE_OffsetVertexOrder_2_0.proto.h"

#include <GFE/GFE_OffsetVertexOrder.h>

using namespace SOP_FeE_OffsetVertexOrder_2_0_Namespace;

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
    }



    parm {
        name    "offset"
        cppname "Offset"
        label   "Offset"
        type    integer
        default { 1 }
        range   { -10 10 }
    }

    parm {
        name    "useOffsetAttrib"
        cppname "UseOffsetAttrib"
        label   "Use Offset Attrib"
        type    toggle
        nolabel
        joinnext
        default { "0" }
    }
    parm {
        name    "offsetAttrib"
        cppname "OffsetAttrib"
        label   "Offset Attrib"
        type    string
        default { "offset" }
        disablewhen "{ useOffsetAttrib == 0 }"
    }

    parm {
        name    "treatLoopedPrimAsClosed"
        cppname "TreatLoopedPrimAsClosed"
        label   "Treat Looped Prim as Closed"
        type    toggle
        default { "1" }
    }

    parm {
        name    "delOffsetAttrib"
        cppname "DelOffsetAttrib"
        label   "Del Offset Attrib"
        type    toggle
        default { "1" }
        disablewhen "{ useOffsetAttrib == 0 }"
    }

    parm {
        name    "delUnusedPoint"
        cppname "DelUnusedPoint"
        label   "Del Unused Point"
        type    toggle
        default { "1" }
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
SOP_FeE_OffsetVertexOrder_2_0::buildTemplates()
{
    static PRM_TemplateBuilder templ("SOP_FeE_OffsetVertexOrder_2_0.C"_sh, theDsFile);
    if (templ.justBuilt())
    {
        templ.setChoiceListPtr("primGroup"_sh, &SOP_Node::primGroupMenu);
        templ.setChoiceListPtr("offsetAttrib"_sh, &SOP_Node::primAttribReplaceMenu);
    }
    return templ.templates();
}

const UT_StringHolder SOP_FeE_OffsetVertexOrder_2_0::theSOPTypeName("FeE::offsetVertexOrder::2.0"_sh);

void
newSopOperator(OP_OperatorTable* table)
{
    OP_Operator* newOp = new OP_Operator(
        SOP_FeE_OffsetVertexOrder_2_0::theSOPTypeName,
        "FeE Offset Vertex Order",
        SOP_FeE_OffsetVertexOrder_2_0::myConstructor,
        SOP_FeE_OffsetVertexOrder_2_0::buildTemplates(),
        1,
        1,
        nullptr,
        OP_FLAG_GENERATOR,
        nullptr,
        1,
        "Five elements Elf/Filter/Enumerate");

    newOp->setIconName("SOP_sort");
    table->addOperator(newOp);

}





class SOP_FeE_OffsetVertexOrder_2_0Verb : public SOP_NodeVerb
{
public:
    SOP_FeE_OffsetVertexOrder_2_0Verb() {}
    virtual ~SOP_FeE_OffsetVertexOrder_2_0Verb() {}

    virtual SOP_NodeParms *allocParms() const { return new SOP_FeE_OffsetVertexOrder_2_0Parms(); }
    virtual UT_StringHolder name() const { return SOP_FeE_OffsetVertexOrder_2_0::theSOPTypeName; }

    virtual CookMode cookMode(const SOP_NodeParms *parms) const { return COOK_INPLACE; }

    virtual void cook(const CookParms &cookparms) const;
    
    /// This static data member automatically registers
    /// this verb class at library load time.
    static const SOP_NodeVerb::Register<SOP_FeE_OffsetVertexOrder_2_0Verb> theVerb;
};

// The static member variable definition has to be outside the class definition.
// The declaration is inside the class.
const SOP_NodeVerb::Register<SOP_FeE_OffsetVertexOrder_2_0Verb> SOP_FeE_OffsetVertexOrder_2_0Verb::theVerb;

const SOP_NodeVerb *
SOP_FeE_OffsetVertexOrder_2_0::cookVerb() const 
{ 
    return SOP_FeE_OffsetVertexOrder_2_0Verb::theVerb.get();
}






void
SOP_FeE_OffsetVertexOrder_2_0Verb::cook(const SOP_NodeVerb::CookParms &cookparms) const
{
    auto&& sopparms = cookparms.parms<SOP_FeE_OffsetVertexOrder_2_0Parms>();
    GA_Detail& outGeo0 = *cookparms.gdh().gdpNC();
    //auto sopcache = (SOP_FeE_OffsetVertexOrder_2_0Cache*)cookparms.cache();

    //const GA_Detail& inGeo0 = *cookparms.inputGeo(0);

    //outGeo0.replaceWith(inGeo0);

    UT_AutoInterrupt boss("Processing");
    if (boss.wasInterrupted())
        return;
    

    gfe::OffsetVertexOrder offsetVertexOrder(outGeo0, &cookparms);

    offsetVertexOrder.setComputeParm(sopparms.getOffset(),
        sopparms.getSubscribeRatio(), sopparms.getMinGrainSize());

    if (sopparms.getUseOffsetAttrib())
        offsetVertexOrder.setOffsetAttrib(sopparms.getOffsetAttrib());
    
    offsetVertexOrder.setComputeParm(sopparms.getOffset(), sopparms.getTreatLoopedPrimAsClosed(),
        sopparms.getDelOffsetAttrib(), sopparms.getDelUnusedPoint(),
        sopparms.getSubscribeRatio(), sopparms.getMinGrainSize());
    
    offsetVertexOrder.groupParser.setPrimitiveGroup(sopparms.getPrimGroup());
    
    offsetVertexOrder.computeAndBumpDataId();
}


