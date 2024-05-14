
//#define UT_ASSERT_LEVEL 3
#include "SOP_FeE_ExtractPoint_1_0.h"


#include "SOP_FeE_ExtractPoint_1_0.proto.h"

#include "GA/GA_Detail.h"
#include "PRM/PRM_TemplateBuilder.h"
#include "UT/UT_Interrupt.h"
#include "UT/UT_DSOVersion.h"


#include "GFE/GFE_ExtractPoint.h"


using namespace SOP_FeE_ExtractPoint_1_0_Namespace;



static const char *theDsFile = R"THEDSFILE(
{
    name        parameters
    
    parm {
       name    "extractPoint"
       cppname "ExtractPoint"
       label   "Extract Point"
       type    toggle
       default { 1 }
    }
    groupsimple {
        name    "extractPoint_folder"
        label   "Extract Point"
        disablewhentab "{ extractPoint == 0 }"

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
            default { "point" }
            menu {
                "guess"     "Guess from Group"
                "prim"      "Primitive"
                "point"     "Point"
                "vertex"    "Vertex"
                "edge"      "Edge"
            }
        }
        parm {
            name    "reverseGroup"
            cppname "ReverseGroup"
            label   "Reverse Group"
            type    toggle
            default { 0 }
        }

        parm {
            name    "keepPrimAttrib"
            cppname "KeepPrimAttrib"
            label   "Keep Prim Attributes"
            type    string
            default { "" }
        }
        parm {
            name    "keepPointAttrib"
            cppname "KeepPointAttrib"
            label   "Keep Point Attributes"
            type    string
            default { "*" }
        }
        parm {
            name    "keepVertexAttrib"
            cppname "KeepVertexAttrib"
            label   "Keep Vertex Attributes"
            type    string
            default { "" }
        }
        parm {
            name    "keepDetailAttrib"
            cppname "KeepDetailAttrib"
            label   "Keep Detail Attributes"
            type    string
            default { "" }
        }
        parm {
            name    "keepPrimGroup"
            cppname "KeepPrimGroup"
            label   "Keep Prim Group"
            type    string
            default { "" }
        }
        parm {
            name    "keepPointGroup"
            cppname "KeepPointGroup"
            label   "Keep Point Group"
            type    string
            default { "*" }
        }
        parm {
            name    "keepVertexGroup"
            cppname "KeepVertexGroup"
            label   "Keep Vertex Group"
            type    string
            default { "" }
        }
        parm {
            name    "keepEdgeGroup"
            cppname "KeepEdgeGroup"
            label   "Keep Edge Group"
            type    string
            default { "" }
        }
        parm {
            name    "delInputGroup"
            cppname "DelInputGroup"
            label   "Delete Input Group"
            type    toggle
            default { 1 }
        }


        parm {
            name    "delPointMode"
            cppname "DelPointMode"
            label   "Delete Point Mode"
            type    ordinal
            default { "delDegenerateIncompatible" }
            menu {
                "leavePrimitive"             "Leave Primitive"
                "delDegenerate"              "Delete Degenerate"
                "delDegenerateIncompatible"  "Delete Degenerate Incompatible"
            }
            invisible
        }
        parm {
            name    "guaranteeNoVertexReference"
            cppname "GuaranteeNoVertexReference"
            label   "Guarantee No Vertex Reference"
            type    toggle
            default { 0 }
            invisible
        }
    }

    parm {
        name    "kernel"
        cppname "Kernel"
        label   "Kernel"
        type    integer
        default { 0 }
        invisible
        range   { 0! 3! }
    }
    parm {
        name    "subscribeRatio"
        cppname "SubscribeRatio"
        label   "Subscribe Ratio"
        type    integer
        default { 16 }
        range   { 0! 256 }
        invisible
    }
    parm {
        name    "minGrainSize"
        cppname "MinGrainSize"
        label   "Min Grain Size"
        type    intlog
        default { 1024 }
        range   { 0! 2048 }
        invisible
    }
}
)THEDSFILE";



PRM_Template*
SOP_FeE_ExtractPoint_1_0::buildTemplates()
{
    static PRM_TemplateBuilder templ("SOP_FeE_ExtractPoint_1_0.C"_sh, theDsFile);
    if (templ.justBuilt())
    {
        templ.setChoiceListPtr("group"_sh,            &SOP_Node::pointGroupMenu);

        templ.setChoiceListPtr("keepPrimAttrib"_sh,   &SOP_Node::primAttribMenu);
        templ.setChoiceListPtr("keepPointAttrib"_sh,  &SOP_Node::pointAttribMenu);
        templ.setChoiceListPtr("keepVertexAttrib"_sh, &SOP_Node::vertexAttribMenu);
        templ.setChoiceListPtr("keepDetailAttrib"_sh, &SOP_Node::detailAttribMenu);

        templ.setChoiceListPtr("keepPrimGroup"_sh,    &SOP_Node::primGroupMenu);
        templ.setChoiceListPtr("keepPointGroup"_sh,   &SOP_Node::pointGroupMenu);
        templ.setChoiceListPtr("keepVertexGroup"_sh,  &SOP_Node::vertexNamedGroupMenu);
        templ.setChoiceListPtr("keepEdgeGroup"_sh,    &SOP_Node::edgeGroupMenu);
    }
    return templ.templates();
}


const UT_StringHolder SOP_FeE_ExtractPoint_1_0::theSOPTypeName("FeE::extractPoint::1.0"_sh);

void
newSopOperator(OP_OperatorTable* table)
{
    OP_Operator* newOp = new OP_Operator(
        SOP_FeE_ExtractPoint_1_0::theSOPTypeName,
        "FeE Extract Point",
        SOP_FeE_ExtractPoint_1_0::myConstructor,
        SOP_FeE_ExtractPoint_1_0::buildTemplates(),
        1,
        1,
        nullptr,
        OP_FLAG_GENERATOR,
        nullptr,
        1,
        "Five elements Elf/Group");

    newOp->setIconName("SOP_pointgenerate");
    table->addOperator(newOp);

}

static GA_Detail::GA_DestroyPointMode
sopDelPointMode(SOP_FeE_ExtractPoint_1_0Parms::DelPointMode delPointMode)
{
    using namespace SOP_FeE_ExtractPoint_1_0Enums;
    switch (delPointMode)
    {
    case DelPointMode::LEAVEPRIMITIVE:              return GA_Detail::GA_DestroyPointMode::GA_LEAVE_PRIMITIVES;                 break;
    case DelPointMode::DELDEGENERATE:               return GA_Detail::GA_DestroyPointMode::GA_DESTROY_DEGENERATE;               break;
    case DelPointMode::DELDEGENERATEINCOMPATIBLE:   return GA_Detail::GA_DestroyPointMode::GA_DESTROY_DEGENERATE_INCOMPATIBLE;  break;
    }
    UT_ASSERT_MSG(0, "Unhandled Delete Point Mode!");
    return GA_Detail::GA_DestroyPointMode::GA_DESTROY_DEGENERATE_INCOMPATIBLE;
}


//class SOP_FeE_ExtractPoint_1_0Cache : public SOP_NodeCache
//{
//public:
//    SOP_FeE_ExtractPoint_1_0Cache() : SOP_NodeCache(),
//        myPrevOutputDetailID(-1)
//    {}
//    ~SOP_FeE_ExtractPoint_1_0Cache() override {}
//
//    int64 myPrevOutputDetailID;
//};


class SOP_FeE_ExtractPoint_1_0Verb : public SOP_NodeVerb
{
public:
    SOP_FeE_ExtractPoint_1_0Verb() {}
    virtual ~SOP_FeE_ExtractPoint_1_0Verb() {}

    virtual SOP_NodeParms *allocParms() const { return new SOP_FeE_ExtractPoint_1_0Parms(); }
    //virtual SOP_NodeCache* allocCache() const { return new SOP_FeE_ExtractPoint_1_0Cache(); }
    virtual UT_StringHolder name() const { return SOP_FeE_ExtractPoint_1_0::theSOPTypeName; }

    virtual CookMode cookMode(const SOP_NodeParms *parms) const { return COOK_GENERIC; }

    virtual void cook(const CookParms &cookparms) const;

    /// This static data member automatically registers
    /// this verb class at library ldir0d time.
    static const SOP_NodeVerb::Register<SOP_FeE_ExtractPoint_1_0Verb> theVerb;
};

// The static member variable definition has to be outside the class definition.
// The declaration is inside the class.
const SOP_NodeVerb::Register<SOP_FeE_ExtractPoint_1_0Verb> SOP_FeE_ExtractPoint_1_0Verb::theVerb;

const SOP_NodeVerb *
SOP_FeE_ExtractPoint_1_0::cookVerb() const 
{ 
    return SOP_FeE_ExtractPoint_1_0Verb::theVerb.get();
}






static GA_GroupType
sopGroupType(SOP_FeE_ExtractPoint_1_0Parms::GroupType parmGroupType)
{
    using namespace SOP_FeE_ExtractPoint_1_0Enums;
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
SOP_FeE_ExtractPoint_1_0Verb::cook(const SOP_NodeVerb::CookParms& cookparms) const
{
    auto&& sopparms = cookparms.parms<SOP_FeE_ExtractPoint_1_0Parms>();
    GA_Detail& outGeo0 = *cookparms.gdh().gdpNC();
    //auto sopcache = (SOP_FeE_ExtractPoint_1_0Cache*)cookparms.cache();

    const GA_Detail& inGeo0 = *cookparms.inputGeo(0);


    if (!sopparms.getExtractPoint())
    {
        outGeo0.replaceWith(inGeo0);
        return;
    }
    
    outGeo0.clear();

    const GA_GroupType groupType = sopGroupType(sopparms.getGroupType());

    // const GA_Detail::GA_DestroyPointMode delPointMode = sopDelPointMode(sopparms.getDelPointMode());


    UT_AutoInterrupt boss("Processing");
    if (boss.wasInterrupted())
        return;
/*
    
            GFE_ExtractPoint extractPoint(geo, geoSrc, cookparms);
            extractPoint.groupParser.setGroup(groupParser);
            extractPoint.keepPointAttrib  = keepSrcElemAttribName;
            extractPoint.keepPointGroup   = keepSrcElemGroupName;
            extractPoint.compute();
    
 */

    GFE_ExtractPoint extractPoint(outGeo0, &inGeo0, &cookparms);

    extractPoint.groupParser.setGroup(groupType, sopparms.getGroup());

    extractPoint.keepPrimAttrib   = sopparms.getKeepPrimAttrib();
    extractPoint.keepPointAttrib  = sopparms.getKeepPointAttrib();
    extractPoint.keepVertexAttrib = sopparms.getKeepVertexAttrib();
    extractPoint.keepDetailAttrib = sopparms.getKeepDetailAttrib();

    extractPoint.keepPrimGroup    = sopparms.getKeepPrimGroup();
    extractPoint.keepPointGroup   = sopparms.getKeepPointGroup();
    extractPoint.keepVertexGroup  = sopparms.getKeepVertexGroup();
    extractPoint.keepEdgeGroup    = sopparms.getKeepEdgeGroup();

        
    extractPoint.setComputeParm(
        sopparms.getReverseGroup(), sopparms.getDelInputGroup()
        //,sopparms.getSubscribeRatio(), sopparms.getMinGrainSize()
    );
    extractPoint.setKernel(sopparms.getKernel());

    // extractPoint.setDeleteParm(delPointMode, sopparms.getGuaranteeNoVertexReference());

    extractPoint.computeAndBumpDataIdsForAddOrRemove();


}


