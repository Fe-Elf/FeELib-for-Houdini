
//#define UT_ASSERT_LEVEL 3
#include "SOP_FeE_Clip_1_0.h"


#include "SOP_FeE_Clip_1_0.proto.h"

#include "GA/GA_Detail.h"
#include "PRM/PRM_TemplateBuilder.h"
#include "UT/UT_Interrupt.h"
#include "UT/UT_DSOVersion.h"




#include "GFE/GFE_Clip.h"




using namespace SOP_FeE_Clip_1_0_Namespace;


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
        name    "clipMethod"
        cppname "ClipMethod"
        label   "Clip Method"
        type    ordinal
        default { "0" }
        menu {
            "above"  "Primitives Above the Plane"
            "below"  "Primitives Below the Plane"
            "both"   "All Primitives"
        }
    }
    parm {
        name    "origin"
        cppname "Origin"
        label   "Origin"
        type    vector
        size    3
        default { "0" "0" "0" }
        range   { -1 1 }
    }
    parm {
        name    "dist"
        cppname "Dist"
        label   "Distance"
        type    float
        default { "0" }
        range   { -5 5 }
    }
    parm {
        name    "dir"
        cppname "Dir"
        label   "Direction"
        type    direction
        size    3
        default { "0" "1" "0" }
        range   { 0 1 }
    }
    parm {
        name    "normlize"
        cppname "Normlize"
        label   "Normlize"
        type    integer
        default { 0 }
        range   { -5 5 }
    }
    parm {
        name    "outAbovePlaneGroup"
        cppname "OutAbovePlaneGroup"
        label   "Out Above Plane Group"
        type    toggle
        default { "0" }
        nolabel
        joinnext
        disablewhen "{ clipMethod != both }"
    }
    parm {
        name    "abovePlaneGroupName"
        cppname "AbovePlaneGroupName"
        label   "Above Plane Group Name"
        type    string
        default { "abovePlane" }
        disablewhen "{ clipMethod != both } { outAbovePlaneGroup == 0 }"
    }
    parm {
        name    "outBelowPlaneGroup"
        cppname "OutBelowPlaneGroup"
        label   "Out Below Plane Group"
        type    toggle
        default { "0" }
        nolabel
        joinnext
        disablewhen "{ clipMethod != both }"
    }
    parm {
        name    "belowPlaneGroupName"
        cppname "BelowPlaneGroupName"
        label   "Below Plane Group Name"
        type    string
        default { "belowPlane" }
        disablewhen "{ clipMethod != both } { outBelowPlaneGroup == 0 }"
    }
    parm {
        name    "clipPoint"
        cppname "ClipPoint"
        label   "Clip Unconnected Points"
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
SOP_FeE_Clip_1_0::buildTemplates()
{
    static PRM_TemplateBuilder templ("SOP_FeE_Clip_1_0.C"_sh, theDsFile);
    if (templ.justBuilt())
    {
        templ.setChoiceListPtr("primGroup"_sh, &SOP_Node::primGroupMenu);
    }
    return templ.templates();
}

const UT_StringHolder SOP_FeE_Clip_1_0::theSOPTypeName("FeE::clip::1.0"_sh);

void
newSopOperator(OP_OperatorTable* table)
{
    OP_Operator* newOp = new OP_Operator(
        SOP_FeE_Clip_1_0::theSOPTypeName,
        "FeE Clip",
        SOP_FeE_Clip_1_0::myConstructor,
        SOP_FeE_Clip_1_0::buildTemplates(),
        1,
        2,
        nullptr,
        OP_FLAG_GENERATOR,
        nullptr,
        1,
        "Five elements Elf/Fracture/Clip");

    newOp->setIconName("SOP_clip");
    table->addOperator(newOp);

}





class SOP_FeE_Clip_1_0Verb : public SOP_NodeVerb
{
public:
    SOP_FeE_Clip_1_0Verb() {}
    virtual ~SOP_FeE_Clip_1_0Verb() {}

    virtual SOP_NodeParms *allocParms() const { return new SOP_FeE_Clip_1_0Parms(); }
    virtual UT_StringHolder name() const { return SOP_FeE_Clip_1_0::theSOPTypeName; }

    virtual CookMode cookMode(const SOP_NodeParms *parms) const { return COOK_INPLACE; }

    virtual void cook(const CookParms &cookparms) const;
    
    /// This static data member automatically registers
    /// this verb class at library load time.
    static const SOP_NodeVerb::Register<SOP_FeE_Clip_1_0Verb> theVerb;
};

// The static member variable definition has to be outside the class definition.
// The declaration is inside the class.
const SOP_NodeVerb::Register<SOP_FeE_Clip_1_0Verb> SOP_FeE_Clip_1_0Verb::theVerb;

const SOP_NodeVerb *
SOP_FeE_Clip_1_0::cookVerb() const 
{ 
    return SOP_FeE_Clip_1_0Verb::theVerb.get();
}




static GFE_ClipMethod
sopClipMethod(const SOP_FeE_Clip_1_0Parms::ClipMethod clipMethod)
{
    using namespace SOP_FeE_Clip_1_0Enums;
    switch (clipMethod)
    {
    case ClipMethod::ABOVE:       return GFE_ClipMethod::Abode;    break;
    case ClipMethod::BELOW:       return GFE_ClipMethod::Below;    break;
    case ClipMethod::BOTH:        return GFE_ClipMethod::Both;     break;
    }
    UT_ASSERT_MSG(0, "Unhandled Clip Method!");
    return GFE_ClipMethod::Both;
}




void
SOP_FeE_Clip_1_0Verb::cook(const SOP_NodeVerb::CookParms &cookparms) const
{
    auto&& sopparms = cookparms.parms<SOP_FeE_Clip_1_0Parms>();
    GA_Detail& outGeo0 = *cookparms.gdh().gdpNC();
    //auto sopcache = (SOP_FeE_Clip_1_0Cache*)cookparms.cache();

    //const GA_Detail& inGeo0 = *cookparms.inputGeo(0);

    //outGeo0.replaceWith(inGeo0);

    const GFE_ClipMethod clipMethod = sopClipMethod(sopparms.getClipMethod());
    
    
    UT_AutoInterrupt boss("Processing");
    if (boss.wasInterrupted())
        return;

    
    //sopparms.getClipMethod()
    //
    GFE_Clip clip(outGeo0, cookparms);
    clip.setComputeParm(clipMethod, sopparms.getDir(), sopparms.getDist(), sopparms.getOrigin(), sopparms.getNormlize(), sopparms.getClipPoint(),
        sopparms.getSubscribeRatio(), sopparms.getMinGrainSize());

    clip.groupParser.setPrimitiveGroup(sopparms.getPrimGroup());
    
    clip.computeAndBumpDataId();


}



namespace SOP_FeE_Clip_1_0_Namespace {

} // End SOP_FeE_Clip_1_0_Namespace namespace
