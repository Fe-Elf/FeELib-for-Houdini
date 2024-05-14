
//#define UT_ASSERT_LEVEL 3
#include "SOP_FeE_SkinIntersectCurve_2_0.h"


#include "SOP_FeE_SkinIntersectCurve_2_0.proto.h"

#include "GA/GA_Detail.h"
#include "PRM/PRM_TemplateBuilder.h"
#include "UT/UT_Interrupt.h"
#include "UT/UT_DSOVersion.h"




#include "GFE/GFE_SkinIntersectCurve.h"



using namespace SOP_FeE_SkinIntersectCurve_2_0_Namespace;


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
        name    "createOrigPrimAttrib"
        cppname "CreateOrigPrimAttrib"
        label   "Create Origin Primitive Attribute"
        type    toggle
        nolabel
        joinnext
        default { "0" }
    }
    parm {
        name    "origPrimAttribName"
        cppname "OrigPrimAttribName"
        label   "Origin Primitive Attribute Name"
        type    string
        default { "origPrim" }
        disablewhen "{ createOrigPrimAttrib == 0 }"
    }
    parm {
        name    "createOrigPointAttrib"
        cppname "CreateOrigPointAttrib"
        label   "Create Origin Point Attribute"
        type    toggle
        nolabel
        joinnext
        default { "off" }
    }
    parm {
        name    "origPointAttribName"
        cppname "OrigPointAttribName"
        label   "Origin Point Attribute Name"
        type    string
        default { "origPoint" }
        disablewhen "{ createOrigPointAttrib == 0 }"
    }
    parm {
        name    "sepparm"
        label   "Separator"
        type    separator
        default { "" }
    }


    parm {
        name    "delInputPrimGroup"
        cppname "DelInputPrimGroup"
        label   "Delete Input Prim Group"
        type    toggle
        default { "0" }
    }

    parm {
        name    "tolerance"
        cppname "Tolerance"
        label   "Tolerance"
        type    log
        default { "1e-05" }
        range { 0! 1 }
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
SOP_FeE_SkinIntersectCurve_2_0::buildTemplates()
{
    static PRM_TemplateBuilder templ("SOP_FeE_SkinIntersectCurve_2_0.C"_sh, theDsFile);
    if (templ.justBuilt())
    {
        templ.setChoiceListPtr("primGroup"_sh, &SOP_Node::primGroupMenu);
    }
    return templ.templates();
}





const UT_StringHolder SOP_FeE_SkinIntersectCurve_2_0::theSOPTypeName("FeE::skinIntersectCurve::2.0"_sh);

void
newSopOperator(OP_OperatorTable* table)
{
    OP_Operator* newOp = new OP_Operator(
        SOP_FeE_SkinIntersectCurve_2_0::theSOPTypeName,
        "FeE Skin Intersect Curve",
        SOP_FeE_SkinIntersectCurve_2_0::myConstructor,
        SOP_FeE_SkinIntersectCurve_2_0::buildTemplates(),
        2,
        2,
        nullptr,
        OP_FLAG_GENERATOR,
        nullptr,
        1,
        "Five elements Elf/Generator/Curve to Poly");

    newOp->setIconName("SOP_skin");
    table->addOperator(newOp);
}




class SOP_FeE_SkinIntersectCurve_2_0Verb : public SOP_NodeVerb
{
public:
    SOP_FeE_SkinIntersectCurve_2_0Verb() {}
    virtual ~SOP_FeE_SkinIntersectCurve_2_0Verb() {}

    virtual SOP_NodeParms *allocParms() const { return new SOP_FeE_SkinIntersectCurve_2_0Parms(); }
    virtual UT_StringHolder name() const { return SOP_FeE_SkinIntersectCurve_2_0::theSOPTypeName; }

    virtual CookMode cookMode(const SOP_NodeParms *parms) const { return COOK_GENERIC; }

    virtual void cook(const CookParms &cookparms) const;
    
    static const SOP_NodeVerb::Register<SOP_FeE_SkinIntersectCurve_2_0Verb> theVerb;
};

const SOP_NodeVerb::Register<SOP_FeE_SkinIntersectCurve_2_0Verb> SOP_FeE_SkinIntersectCurve_2_0Verb::theVerb;

const SOP_NodeVerb *
SOP_FeE_SkinIntersectCurve_2_0::cookVerb() const 
{ 
    return SOP_FeE_SkinIntersectCurve_2_0Verb::theVerb.get();
}


void
SOP_FeE_SkinIntersectCurve_2_0Verb::cook(const SOP_NodeVerb::CookParms& cookparms) const
{
    auto&& sopparms = cookparms.parms<SOP_FeE_SkinIntersectCurve_2_0Parms>();
    GA_Detail& outGeo0 = *cookparms.gdh().gdpNC();
    //auto sopcache = (SOP_FeE_SkinIntersectCurve_2_0Cache*)cookparms.cache();

    const GA_Detail& inGeo0 = *cookparms.inputGeo(0);
    const GA_Detail& inGeo1 = *cookparms.inputGeo(1);

    outGeo0.replaceWith(inGeo0);


    UT_AutoInterrupt boss("Processing");
    if (boss.wasInterrupted())
        return;
    

    

    GFE_SkinIntersectCurve skinIntersectCurve(outGeo0, inGeo1, cookparms);
    skinIntersectCurve.groupParser.setPrimitiveGroup(sopparms.getPrimGroup());
    
    skinIntersectCurve.setComputeParm(sopparms.getTolerance(), 
        sopparms.getSubscribeRatio(), sopparms.getMinGrainSize());

    skinIntersectCurve.computeAndBumpDataId();


}

