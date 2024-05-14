
//#define UT_ASSERT_LEVEL 3
#include "SOP_FeE_Triangulate2D_2_0.h"


#include "SOP_FeE_Triangulate2D_2_0.proto.h"

#include "GA/GA_Detail.h"
#include "PRM/PRM_TemplateBuilder.h"
#include "UT/UT_Interrupt.h"
#include "UT/UT_DSOVersion.h"




#include "GFE/GFE_Triangulate2D.h"




using namespace SOP_FeE_Triangulate2D_2_0_Namespace;


static const char *theDsFile = R"THEDSFILE(
{
    name        parameters

    parm {
        name    "axis"
        cppname "Axis"
        label   "Axis"
        type    ordinal
        default { "y" }
        menu {
            "x"         "X"
            "y"         "Y"
            "z"         "Z"
            "custom"    "Custom"
        }
    }
    parm {
        name    "customDir"
        cppname "CustomDir"
        label   "Direction"
        type    direction
        size    3
        default { "0" "-1" "0" }
        disablewhen "{ axis != custom }"
        range   { 0 1 }
    }
    parm {
        name    "keepUnsharedSilhouette"
        cppname "KeepUnsharedSilhouette"
        label   "Keep Unshared Silhouette"
        type    toggle
        default { "0" }
    }
    parm {
        name    "keepOutsidePrim"
        cppname "KeepOutsidePrim"
        label   "Keep Outside Prim"
        type    toggle
        default { "off" }
    }
    parm {
        name    "keepHeight"
        cppname "KeepHeight"
        label   "Keep Height"
        type    toggle
        default { "0" }
    }

    //parm {
    //    name    "copyFirstElemAttrib"
    //    cppname "CopyFirstElemAttrib"
    //    label   "Copy First Elem Attrib"
    //    type    string
    //    default { "" }
    //}

    groupsimple {
        name    "pre"
        label   "Pre"
        grouptag { "group_type" "simple" }


        parm {
            name    "keepSingleSide"
            cppname "KeepSingleSide"
            label   "Keep Single Side"
            type    toggle
            default { "off" }
        }
        parm {
            name    "reverseSide"
            cppname "ReverseSide"
            label   "Reverse Side"
            type    toggle
            default { "off" }
            disablewhen "{ keepSingleSide == 0 }"
        }

        parm {
            name    "preDelSharedEdge"
            cppname "PreDelSharedEdge"
            label   "Pre Delete Shared Edge"
            type    toggle
            default { "0" }
        }
        parm {
            name    "preFuse"
            cppname "PreFuse"
            label   "Pre Fuse"
            type    toggle
            nolabel
            joinnext
            default { "0" }
        }
        parm {
            name    "preFuseDist"
            cppname "PreFuseDist"
            label   "Pre Fuse Dist"
            type    log
            default { "0.001" }
            disablewhen "{ preFuse == 0 }"
            range   { 0.001 10 }
        }
    }

    groupsimple {
        name    "post"
        label   "Post"
        grouptag { "group_type" "simple" }

        parm {
            name    "delSmallPrim"
            cppname "DelSmallPrim"
            label   "Del Small Prim"
            type    toggle
            nolabel
            joinnext
            default { "0" }
        }
        parm {
            name    "thresholdAreaPerimeterRatio"
            cppname "ThresholdAreaPerimeterRatio"
            label   "Threshold Area Perimeter Ratio"
            type    log
            default { "0.001" }
            disablewhen "{ delSmallPrim == 0 }"
            range   { 0.00001 0.1 }
        }
        parm {
            name    "ignoreHole"
            cppname "IgnoreHole"
            label   "Ignore Hole"
            type    toggle
            default { "1" }
        }
        parm {
            name    "postFuse"
            cppname "PostFuse"
            label   "Post Fuse"
            type    toggle
            nolabel
            joinnext
            default { "0" }
        }
        parm {
            name    "postFuseDist"
            cppname "PostFuseDist"
            label   "Post Fuse Distance"
            type    log
            default { "0.001" }
            disablewhen "{ postFuse == 0 }"
            range   { 1e-05 100 }
        }
    }


    //parm {
    //    name    "excludeSharedEdge"
    //    cppname "ExcludeSharedEdge"
    //    label   "Exclude Shared Edge"
    //    type    toggle
    //    default { "0" }
    //}

    // parm {
    //     name    "reverseOutPrim"
    //     label   "Reverse Out Prim"
    //     type    toggle
    //     default { "0" }
    // }



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
SOP_FeE_Triangulate2D_2_0::buildTemplates()
{
    static PRM_TemplateBuilder templ("SOP_FeE_Triangulate2D_2_0.C"_sh, theDsFile);
    if (templ.justBuilt())
    {
        //templ.setChoiceListPtr("primGroup"_sh, &SOP_Node::primGroupMenu);
    }
    return templ.templates();
}

const UT_StringHolder SOP_FeE_Triangulate2D_2_0::theSOPTypeName("FeE::triangulate2d::2.0"_sh);

void
newSopOperator(OP_OperatorTable* table)
{
    OP_Operator* newOp = new OP_Operator(
        SOP_FeE_Triangulate2D_2_0::theSOPTypeName,
        "FeE Triangulate 2D",
        SOP_FeE_Triangulate2D_2_0::myConstructor,
        SOP_FeE_Triangulate2D_2_0::buildTemplates(),
        1,
        1,
        nullptr,
        OP_FLAG_GENERATOR,
        nullptr,
        1,
        "Five elements Elf/Filter/Topo/Artificial Line");

    newOp->setIconName("SOP_triangulate2d-2.0");
    table->addOperator(newOp);

}





class SOP_FeE_Triangulate2D_2_0Verb : public SOP_NodeVerb
{
public:
    SOP_FeE_Triangulate2D_2_0Verb() {}
    virtual ~SOP_FeE_Triangulate2D_2_0Verb() {}

    virtual SOP_NodeParms *allocParms() const { return new SOP_FeE_Triangulate2D_2_0Parms(); }
    virtual UT_StringHolder name() const { return SOP_FeE_Triangulate2D_2_0::theSOPTypeName; }

    virtual CookMode cookMode(const SOP_NodeParms *parms) const { return COOK_GENERIC; }

    virtual void cook(const CookParms &cookparms) const;
    
    /// This static data member automatically registers
    /// this verb class at library load time.
    static const SOP_NodeVerb::Register<SOP_FeE_Triangulate2D_2_0Verb> theVerb;
};

// The static member variable definition has to be outside the class definition.
// The declaration is inside the class.
const SOP_NodeVerb::Register<SOP_FeE_Triangulate2D_2_0Verb> SOP_FeE_Triangulate2D_2_0Verb::theVerb;

const SOP_NodeVerb *
SOP_FeE_Triangulate2D_2_0::cookVerb() const 
{ 
    return SOP_FeE_Triangulate2D_2_0Verb::theVerb.get();
}







static GFE_Axis
sopAxis(const SOP_FeE_Triangulate2D_2_0Parms::Axis parmAxis)
{
    using namespace SOP_FeE_Triangulate2D_2_0Enums;
    switch (parmAxis)
    {
    case Axis::X:       return GFE_Axis::X;       break;
    case Axis::Y:       return GFE_Axis::Y;       break;
    case Axis::Z:       return GFE_Axis::Z;       break;
    case Axis::CUSTOM:  return GFE_Axis::Invalid; break;
    }
    UT_ASSERT_MSG(0, "Unhandled Axis!");
    return GFE_Axis::Invalid;
}


void
SOP_FeE_Triangulate2D_2_0Verb::cook(const SOP_NodeVerb::CookParms &cookparms) const
{
    auto&& sopparms = cookparms.parms<SOP_FeE_Triangulate2D_2_0Parms>();
    GA_Detail& outGeo0 = *cookparms.gdh().gdpNC();
    //auto sopcache = (SOP_FeE_Triangulate2D_2_0Cache*)cookparms.cache();

    const GA_Detail& inGeo0 = *cookparms.inputGeo(0);

    outGeo0.replaceWith(inGeo0);

    
    const GFE_Axis axis = sopAxis(sopparms.getAxis());

    
    UT_AutoInterrupt boss("Processing");
    if (boss.wasInterrupted())
        return;
    
    
    
    GFE_Triangulate2D triangulat2D(outGeo0, cookparms);

    if (axis == GFE_Axis::Invalid)
        triangulat2D.setDir(axis);
    else
        triangulat2D.setDir(sopparms.getCustomDir());

    
    if (sopparms.getKeepSingleSide())
        triangulat2D.setKeepSingleSide(sopparms.getReverseSide());
    
    if (sopparms.getPreFuse())
        triangulat2D.setPreFuse(sopparms.getPreFuseDist());
    
    //triangulat2D.preDelSharedEdge = sopparms.getPreDelSharedEdge();
    
    //triangulat2D.keepUnsharedSilhouette = sopparms.getKeepUnsharedSilhouette()();
    //triangulat2D.keepOutsidePrim = sopparms.getKeepOutsidePrim();
    //triangulat2D.keepHeight = sopparms.getKeepHeight();
    
    if (sopparms.getPostFuse())
        triangulat2D.setPreFuse(sopparms.getPostFuseDist());

    
    triangulat2D.setComputeParm(sopparms.getPreDelSharedEdge(), sopparms.getKeepUnsharedSilhouette(), sopparms.getKeepOutsidePrim(), sopparms.getKeepHeight()
        //,sopparms.getSubscribeRatio(), sopparms.getMinGrainSize()
        );
    
    //triangulat2D.setGroup(sopparms.getGroup());
    
    triangulat2D.computeAndBumpDataId();



}


