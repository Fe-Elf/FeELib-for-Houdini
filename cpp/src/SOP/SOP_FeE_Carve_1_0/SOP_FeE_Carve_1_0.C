
//#define UT_ASSERT_LEVEL 3
#include "SOP_FeE_Carve_1_0.h"
#include "SOP_FeE_Carve_1_0.proto.h"

#include <GFE/GFE_Carve.h>

using namespace SOP_FeE_Carve_1_0_Namespace;

static const char *theDsFile = R"THEDSFILE(
{
    name	parameters
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
        default { "prim" }
        menu {
            "guess"     "Guess from Group"
            "prim"      "Primitive"
            "point"     "Point"
            "vertex"    "Vertex"
            "edge"      "Edge"
        }
    }
    parm {
        name    "carveStartPrimGroup"
        cppname "CarveStartPrimGroup"
        label   "Carve Start Prim Group"
        type    string
        default { "" }
        parmtag { "script_action" "import soputils\nkwargs['geometrytype'] = (hou.geometryType.Primitives,)\nkwargs['inputindex'] = 0\nsoputils.selectGroupParm(kwargs)" }
        parmtag { "script_action_help" "Select geometry from an available viewport.\nShift-click to turn on Select Groups." }
        parmtag { "script_action_icon" "BUTTONS_reselect" }
    }
    parm {
        name    "carveEndPrimGroup"
        cppname "CarveEndPrimGroup"
        label   "Carve End Prim Group"
        type    string
        default { "" }
        parmtag { "script_action" "import soputils\nkwargs['geometrytype'] = (hou.geometryType.Primitives,)\nkwargs['inputindex'] = 0\nsoputils.selectGroupParm(kwargs)" }
        parmtag { "script_action_help" "Select geometry from an available viewport.\nShift-click to turn on Select Groups." }
        parmtag { "script_action_icon" "BUTTONS_reselect" }
    }

    parm {
        name    "carveSpace"
        cppname "CarveSpace"
        label   "Carve Space"
        type    ordinal
        default { "worldArcLength" }
        menu {
            "customAttrib"       "CustomAttrib"
            "localAverage"       "Local Average"
            "localArcLength"     "Local Arc Length"
            "worldAverage"       "World Average"
            "worldArcLength"     "World Arc Length"
        }
    }
    parm {
        name    "customCarveUVAttrib"
        cppname "CustomCarveUVAttrib"
        label   "Custom Carve UV Attrib"
        type    string
        default { "" }
        disablewhen "{ carveSpace != customAttrib }"
    }
    parm {
        name    "sepparm"
        label   "Separator"
        type    separator
        default { "" }
    }
    parm {
        name    "startCarveULocal"
        cppname "StartCarveULocal"
        label   "Start Carve U Local"
        type    float
        default { "0.25" }
        hidewhen "{ carveSpace != localAverage carveSpace != localArcLength }"
        range   { 0! 1! }
    }
    parm {
        name    "startCarveUWorld"
        cppname "StartCarveUWorld"
        label   "Start Carve U Local World"
        type    log
        default { "0.25" }
        hidewhen "{ carveSpace == localAverage } { carveSpace == localArcLength }"
        range   { 0.01 100 }
    }
    parm {
        name    "useCarveUStartAttrib"
        cppname "UseCarveUStartAttrib"
        label   "Use CarveU Start Attrib"
        type    toggle
        default { "0" }
        joinnext
        nolabel
    }
    parm {
        name    "carveUStartAttrib"
        cppname "CarveUStartAttrib"
        label   "CarveU Start Attrib"
        type    string
        default { "startCarveU" }
        disablewhen "{ useCarveUStartAttrib == 0 }"
    }
    parm {
        name    "absCarveUEnd"
        cppname "AbsCarveUEnd"
        label   "Absolute Carve U End"
        type    toggle
        default { "0" }
    }
    parm {
        name    "endCarveULocal"
        cppname "EndCarveULocal"
        label   "End Carve U Local"
        type    float
        default { "0.75" }
        hidewhen "{ carveSpace != localAverage carveSpace != localArcLength }"
        range   { 0! 1! }
    }
    parm {
        name    "endCarveUWorld"
        cppname "EndCarveUWorld"
        label   "End Carve U World"
        type    log
        default { "0.25" }
        hidewhen "{ carveSpace == localAverage } { carveSpace == localArcLength }"
        range   { 0.01 100 }
    }
    parm {
        name    "useCarveUEndAttrib"
        cppname "UseCarveUEndAttrib"
        label   "Use CarveU End Attrib"
        type    toggle
        default { "0" }
        joinnext
        nolabel
    }
    parm {
        name    "carveUEndAttrib"
        cppname "CarveUEndAttrib"
        label   "CarveU End Attrib"
        type    string
        default { "endCarveU" }
        disablewhen "{ useCarveUEndAttrib == 0 }"
    }

    parm {
        name    "sepparm3"
        label   "Separator"
        type    separator
        default { "" }
    }





    parm {
        name    "keepOutsideStart"
        cppname "KeepOutsideStart"
        label   "Keep Outside Start"
        type    toggle
        default { "0" }
    }
    groupsimple {
        name    "outsideStart_folder"
        label   "Outside Start"
        disablewhen "{ keepOutsideStart == 0 }"
        grouptag { "group_type" "simple" }

        parm {
            name    "outOutsideStartPrimGroup"
            cppname "OutOutsideStartPrimGroup"
            label   "Output Outside Start Prim Group"
            type    toggle
            nolabel
            joinnext
            default { "0" }
        }
        parm {
            name    "outsideStartPrimGroup"
            cppname "OutsideStartPrimGroup"
            label   "Outside Start Prim Group"
            type    string
            default { "outside" }
            disablewhen "{ outOutsideStartPrimGroup == 0 }"
        }
        parm {
            name    "reverseOutsideStart"
            cppname "ReverseOutsideStart"
            label   "Reverse Outside Start"
            type    toggle
            default { "0" }
        }
    }

    parm {
        name    "keepOutsideEnd"
        cppname "KeepOutsideEnd"
        label   "Keep Outside End"
        type    toggle
        default { "0" }
    }
    groupsimple {
        name    "outsideEnd_folder"
        label   "Outside End"
        disablewhen "{ keepOutsideEnd == 0 }"
        grouptag { "group_type" "simple" }

        parm {
            name    "outOutsideEndPrimGroup"
            cppname "OutOutsideEndPrimGroup"
            label   "Output Outside End Prim Group"
            type    toggle
            nolabel
            joinnext
            default { "0" }
        }
        parm {
            name    "outsideEndPrimGroup"
            cppname "OutsideEndPrimGroup"
            label   "Outside End Prim Group"
            type    string
            default { "outsideEnd" }
            disablewhen "{ outOutsideEndPrimGroup == 0 }"
        }
        parm {
            name    "reverseOutsideEnd"
            cppname "ReverseOutsideEnd"
            label   "Reverse Outside End"
            type    toggle
            default { "0" }
        }
    }

    parm {
        name    "keepInside"
        cppname "KeepInside"
        label   "Keep Inside"
        type    toggle
        default { "1" }
    }
    groupsimple {
        name    "inside_folder"
        label   "Inside"
        disablewhen "{ keepInside == 0 }"
        grouptag { "group_type" "simple" }

        parm {
            name    "outInsidePrimGroup"
            cppname "OutInsidePrimGroup"
            label   "Output Inside Prim Group"
            type    toggle
            nolabel
            joinnext
            default { "0" }
        }
        parm {
            name    "insidePrimGroup"
            cppname "InsidePrimGroup"
            label   "Inside Prim Group"
            type    string
            default { "inside" }
            disablewhen "{ outInsidePrimGroup == 0 }"
        }
        parm {
            name    "delReversedInsidePrim"
            cppname "DelReversedInsidePrim"
            label   "Delete Reversed Inside Prim"
            type    toggle
            default { "0" }
        }
    }


    parm {
        name    "outCarvedStartPrimGroup"
        cppname "OutCarvedStartPrimGroup"
        label   "Output Carved Start Prim Group"
        type    toggle
        nolabel
        joinnext
        default { "0" }
    }
    parm {
        name    "carvedStartPrimGroup"
        cppname "CarvedStartPrimGroup"
        label   "Carved Start Prim Group"
        type    string
        default { "carvedStart" }
        disablewhen "{ outCarvedStartPrimGroup == 0 }"
    }
    parm {
        name    "outCarvedEndPrimGroup"
        cppname "OutCarvedEndPrimGroup"
        label   "Output Carved End Prim Group"
        type    toggle
        nolabel
        joinnext
        default { "0" }
    }
    parm {
        name    "carvedEndPrimGroup"
        cppname "CarvedEndPrimGroup"
        label   "Carved End Prim Group"
        type    string
        default { "carvedEnd" }
        disablewhen "{ outCarvedEndPrimGroup == 0 }"
    }


    parm {
        name    "outSrcPointAttrib"
        cppname "OutSrcPointAttrib"
        label   "Output Source Point Attrib"
        type    toggle
        nolabel
        joinnext
        default { "0" }
    }
    parm {
        name    "srcPointAttribName"
        cppname "SrcPointAttribName"
        label   "Source Point Attrib Name"
        type    string
        default { "srcPoint" }
        disablewhen "{ outSrcPointAttrib == 0 }"
    }

    parm {
        name    "outSrcPrimAttrib"
        cppname "OutSrcPrimAttrib"
        label   "Output Source Prim Attrib"
        type    toggle
        nolabel
        joinnext
        default { "0" }
    }
    parm {
        name    "srcPrimAttribName"
        cppname "SrcPrimAttribName"
        label   "Source Prim Attrib Name"
        type    string
        default { "srcPrim" }
        disablewhen "{ outSrcPrimAttrib == 0 }"
    }


    parm {
        name    "interpAttrib"
        cppname "InterpAttrib"
        label   "Interp Attrib"
        type    string
        default { "P" }
    }



    parm {
        name    "sortPrimOrder"
        cppname "SortPrimOrder"
        label   "Sort Primitive Order"
        type    toggle
        default { "0" }
    }
    parm {
        name    "sortPointOrder"
        cppname "SortPointOrder"
        label   "Sort Points Order"
        type    toggle
        default { "0" }
    }
    parm {
        name    "delCarveUStartAttrib"
        cppname "DelCarveUStartAttrib"
        label   "Delete CarveU Start Attrib"
        type    toggle
        default { "1" }
    }
    parm {
        name    "delCarveUEndAttrib"
        cppname "DelCarveUEndAttrib"
        label   "Delete CarveU End Attrib"
        type    toggle
        default { "ch('delCarveUStartAttrib')" }
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


}
)THEDSFILE";

PRM_Template*
SOP_FeE_Carve_1_0::buildTemplates()
{
    static PRM_TemplateBuilder templ("SOP_FeE_Carve_1_0.C"_sh, theDsFile);
    if (templ.justBuilt())
    {
        templ.setChoiceListPtr("group"_sh, &SOP_Node::primGroupMenu);
        templ.setChoiceListPtr("carveStartPrimGroup"_sh,  &SOP_Node::primGroupMenu);
        templ.setChoiceListPtr("carveEndPrimGroup"_sh,    &SOP_Node::primGroupMenu);

        templ.setChoiceListPtr("interpAttrib"_sh,         &SOP_Node::allAttribMenu);
        templ.setChoiceListPtr("customCarveUVAttrib"_sh,  &SOP_Node::allAttribReplaceMenu);
    }
    return templ.templates();
}





const UT_StringHolder SOP_FeE_Carve_1_0::theSOPTypeName("FeE::carve::1.0"_sh);

void
newSopOperator(OP_OperatorTable* table)
{
    OP_Operator* newOp = new OP_Operator(
        SOP_FeE_Carve_1_0::theSOPTypeName,
        "FeE Carve",
        SOP_FeE_Carve_1_0::myConstructor,
        SOP_FeE_Carve_1_0::buildTemplates(),
        1,
        1,
        nullptr,
        OP_FLAG_GENERATOR,
        nullptr,
        1,
        "Five elements Elf/Fracture/Clip");

    newOp->setIconName("SOP_carve");
    table->addOperator(newOp);
}




class SOP_FeE_Carve_1_0Verb : public SOP_NodeVerb
{
public:
    SOP_FeE_Carve_1_0Verb() {}
    virtual ~SOP_FeE_Carve_1_0Verb() {}

    virtual SOP_NodeParms *allocParms() const { return new SOP_FeE_Carve_1_0Parms(); }
    virtual UT_StringHolder name() const { return SOP_FeE_Carve_1_0::theSOPTypeName; }

    virtual CookMode cookMode(const SOP_NodeParms *parms) const { return COOK_INPLACE; }

    virtual void cook(const CookParms &cookparms) const;
    
    static const SOP_NodeVerb::Register<SOP_FeE_Carve_1_0Verb> theVerb;
};

const SOP_NodeVerb::Register<SOP_FeE_Carve_1_0Verb> SOP_FeE_Carve_1_0Verb::theVerb;

const SOP_NodeVerb *
SOP_FeE_Carve_1_0::cookVerb() const 
{ 
    return SOP_FeE_Carve_1_0Verb::theVerb.get();
}





 

static GA_GroupType
sopGroupType(const SOP_FeE_Carve_1_0Parms::GroupType parmGroupType)
{
    using namespace SOP_FeE_Carve_1_0Enums;
    switch (parmGroupType)
    {
    case GroupType::GUESS:     return GA_GROUP_INVALID;    break;
    case GroupType::PRIM:      return GA_GROUP_PRIMITIVE;  break;
    case GroupType::POINT:     return GA_GROUP_POINT;      break;
    case GroupType::VERTEX:    return GA_GROUP_VERTEX;     break;
    case GroupType::EDGE:      return GA_GROUP_EDGE;       break;
    }
    UT_ASSERT_MSG(0, "Unhandled geo0Group type!");
    return GA_GROUP_INVALID;
}


static GFE_CarveSpace
sopCarveSpace(const SOP_FeE_Carve_1_0Parms::CarveSpace parmCarveSpace)
{
    using namespace SOP_FeE_Carve_1_0Enums;
    switch (parmCarveSpace)
    {
    case CarveSpace::CUSTOMATTRIB:     return GFE_CarveSpace::CustomAttrib;    break;
    case CarveSpace::LOCALAVERAGE:     return GFE_CarveSpace::LocalAverage;    break;
    case CarveSpace::LOCALARCLENGTH:   return GFE_CarveSpace::LocalArcLength;  break;
    case CarveSpace::WORLDAVERAGE:     return GFE_CarveSpace::WorldAverage;    break;
    case CarveSpace::WORLDARCLENGTH:   return GFE_CarveSpace::WorldArcLength;  break;
    }
    UT_ASSERT_MSG(0, "Unhandled Carve Space!");
    return GFE_CarveSpace::WorldArcLength;
}


void
SOP_FeE_Carve_1_0Verb::cook(const SOP_NodeVerb::CookParms& cookparms) const
{
    auto&& sopparms = cookparms.parms<SOP_FeE_Carve_1_0Parms>();
    GA_Detail& outGeo0 = *cookparms.gdh().gdpNC();
    //auto sopcache = (SOP_FeE_Carve_1_0Cache*)cookparms.cache();

    //const GA_Detail& inGeo0 = *cookparms.inputGeo(0);

    //outGeo0.replaceWith(inGeo0);

    const GFE_CarveSpace carveSpace = sopCarveSpace(sopparms.getCarveSpace());
    const GA_GroupType groupType = sopGroupType(sopparms.getGroupType());
    
    UT_AutoInterrupt boss("Processing");
    if (boss.wasInterrupted())
        return;
    
    _gfel::Carve carve(outGeo0, &cookparms);
    

    carve.setComputeParm(carveSpace,
        sopparms.getKeepOutsideStart(), sopparms.getKeepOutsideEnd(), sopparms.getKeepInside(),
        sopparms.getAbsCarveUEnd(), sopparms.getDelReversedInsidePrim(),
        sopparms.getSubscribeRatio(), sopparms.getMinGrainSize());
    
    carve.reverseOutsideStart = sopparms.getReverseOutsideStart();
    carve.reverseOutsideEnd   = sopparms.getReverseOutsideEnd();

    
    carve.setCarveU<true> (sopparms.getEndCarveULocal(),   sopparms.getEndCarveUWorld());
    carve.setCarveU<false>(sopparms.getStartCarveULocal(), sopparms.getStartCarveUWorld());
    
    carve.setUVAttrib(sopparms.getCustomCarveUVAttrib());

    if (sopparms.getUseCarveUStartAttrib())
        carve.setCarveUStartAttrib(sopparms.getCarveUStartAttrib(), sopparms.getDelCarveUStartAttrib());
    if (sopparms.getUseCarveUEndAttrib())
        carve.setCarveUEndAttrib(  sopparms.getCarveUEndAttrib(), sopparms.getDelCarveUEndAttrib());

    //carve.setCarveStartGroup(sopparms.getCarveStartPrimGroup());
    //carve.setCarveEndGroup(sopparms.getCarveEndPrimGroup());
    carve.groupParser.setGroup(groupType, sopparms.getGroup());
    carve.groupParser_carveStart.setGroup(GA_GROUP_PRIMITIVE, sopparms.getCarveStartPrimGroup());
    carve.groupParser_carveEnd  .setGroup(GA_GROUP_PRIMITIVE, sopparms.getCarveEndPrimGroup());
    
    carve.setCarvedStartPrimGroup(false, sopparms.getCarvedStartPrimGroup());
    carve.setCarvedEndPrimGroup  (false, sopparms.getCarvedEndPrimGroup());
    
    if (sopparms.getOutSrcPrimAttrib())
        carve.enumerate.findOrCreateTuple(false, GA_ATTRIB_PRIMITIVE, sopparms.getSrcPrimAttribName());

    if (sopparms.getOutSrcPointAttrib())
        carve.enumerate.findOrCreateTuple(false, GA_ATTRIB_POINT, sopparms.getSrcPointAttribName());

    //carve.getOutAttribArray().appendPointVertexs();
    carve.interpAttribArray.appends(GA_ATTRIB_POINT, sopparms.getInterpAttrib());
    carve.interpAttribArray.appends(GA_ATTRIB_VERTEX, sopparms.getInterpAttrib());
    
    carve.computeAndBumpDataIdsForAddOrRemove();

    carve.visualizeOutGroup();

}


