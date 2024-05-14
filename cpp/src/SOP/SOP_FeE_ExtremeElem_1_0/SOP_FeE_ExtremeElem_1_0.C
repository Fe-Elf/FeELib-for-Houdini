
//#define UT_ASSERT_LEVEL 3
#include "SOP_FeE_ExtremeElem_1_0.h"


#include "SOP_FeE_ExtremeElem_1_0.proto.h"

#include "GA/GA_Detail.h"
#include "PRM/PRM_TemplateBuilder.h"
#include "UT/UT_Interrupt.h"
#include "UT/UT_DSOVersion.h"


#include "GFE/GFE_ExtremeElement.h"

using namespace SOP_FeE_ExtremeElem_1_0_Namespace;



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
        name    "statisticalFunction"
        cppname "StatisticalFunction"
        label   "Statistical Function"
        type    ordinal
        default { "max" }
        menu {
            "min"   "Minimum"
            "max"   "Maximum"
        }
    }
    parm {
        name    "useMeasureAttrib"
        cppname "UseMeasureAttrib"
        label   "Use Measure Attrib"
        type    toggle
        nolabel
        joinnext
        default { "0" }
    }
    parm {
        name    "measureType"
        cppname "MeasureType"
        label   "MeasureType"
        type    ordinal
        default { "perimeter" }
        menu {
            "perimeter"         "Perimeter"
            "area"              "Area"
            "volume"            "Volume"
            "centroid"          "Centroid"
            "curvature"         "Curvature"
            "gradient"          "Gradient"
            "laplacian"         "Laplacian"
            "boundaryintegral"  "Boundary Integral"
            "surfaceintegral"   "Surface Integral"
        }
        disablewhen "{ useMeasureAttrib == 0 }"
    }


    parm {
        name    "extremeAttribClass"
        cppname "ExtremeAttribClass"
        label   "Extreme Attrib Class"
        type    ordinal
        default { "prim" }
        menu {
            "prim"      "Primitive"
            "point"     "Point"
            "vertex"    "Vertex"
            "detail"    "Detail"
        }
        disablewhen "{ useMeasureAttrib == 1 }"
    }
    parm {
        name    "extremeAttrib"
        cppname "ExtremeAttrib"
        label   "Extreme Attrib"
        type    string
        default { "" }
        disablewhen "{ useMeasureAttrib == 1 }"
    }

    parm {
        name    "outExtremeElemAttrib"
        cppname "OutExtremeElemAttrib"
        label   "Out Extreme Elem Attrib"
        type    toggle
        nolabel
        joinnext
        default { "0" }
    }
    parm {
        name    "extremeElemAttribName"
        cppname "ExtremeElemAttribName"
        label   "Extreme Elem Attrib Name"
        type    string
        default { "extremeElem" }
        disablewhen "{ outExtremeElemAttrib == 0 }"
    }

    parm {
        name    "outExtremeValueAttrib"
        cppname "OutExtremeValueAttrib"
        label   "Out Extreme Value Attrib"
        type    toggle
        nolabel
        joinnext
        default { "0" }
    }
    parm {
        name    "extremeValueAttribName"
        cppname "ExtremeValueAttribName"
        label   "Extreme Value Attrib Name"
        type    string
        default { "extremeValue" }
        disablewhen "{ outExtremeValueAttrib == 0 }"
    }


    parm {
        name    "outExtremeElemGroup"
        cppname "OutExtremeElemGroup"
        label   "Out Extreme Elem Group Name"
        type    toggle
        nolabel
        joinnext
        default { "0" }
        disablewhen "{ delElement == 1 }"
    }
    parm {
        name    "extremeElemGroupName"
        cppname "ExtremeElemGroupName"
        label   "Extreme Elem Group Name"
        type    string
        default { "extremeElem" }
        disablewhen "{ delElement == 1 } { outExtremeElemGroup == 0 }"
        parmtag { "script_action" "import soputils kwargs['geometrytype'] = kwargs['node'].parmTuple('extremeAttribClass') kwargs['inputindex'] = 0 soputils.selectGroupParm(kwargs)" }
        parmtag { "script_action_help" "Select geometry from an available viewport." }
        parmtag { "script_action_icon" "BUTTONS_reselect" }
    }
    parm {
        name    "reverseGroup"
        cppname "ReverseGroup"
        label   "Reverse Group"
        type    toggle
        default { "off" }
        disablewhen "{ delElement == 0 outExtremeElemGroup == 0 }"
    }

    parm {
        name    "delExtremeAttrib"
        cppname "DelExtremeAttrib"
        label   "Delete Extreme Attrib"
        type    toggle
        default { "0" }
    }

    parm {
        name    "delElement"
        cppname "DelElement"
        label   "Delete Element"
        type    toggle
        default { "off" }
    }

    // parm {
    //     name    "usePieceAttrib"
    //     cppname "UsePieceAttrib"
    //     label   "Use Piece Attribute"
    //     type    toggle
    //     nolabel
    //     joinnext
    //     default { "off" }
    // }
    // parm {
    //     name    "pieceAttribClass"
    //     cppname "PieceAttribClass"
    //     label   "Piece Attrib Class"
    //     type    string
    //     default { "class" }
    //     disablewhen "{ usePieceAttrib == 0 }"
    // }
    // parm {
    //     name    "pieceAttrib"
    //     cppname "PieceAttrib"
    //     label   "Piece Attrib"
    //     type    string
    //     default { "class" }
    //     disablewhen "{ usePieceAttrib == 0 }"
    // }

    parm {
       name    "subscribeRatio"
       cppname "SubscribeRatio"
       label   "Subscribe Ratio"
       type    integer
       default { 16 }
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
SOP_FeE_ExtremeElem_1_0::buildTemplates()
{
    static PRM_TemplateBuilder templ("SOP_FeE_ExtremeElem_1_0.C"_sh, theDsFile);
    if (templ.justBuilt())
    {
        templ.setChoiceListPtr("group"_sh, &SOP_Node::allGroupMenu);
        templ.setChoiceListPtr("extremeElemGroupName"_sh, &SOP_Node::allGroupMenu);
    }
    return templ.templates();
}


const UT_StringHolder SOP_FeE_ExtremeElem_1_0::theSOPTypeName("FeE::extremeElem::1.0"_sh);

void
newSopOperator(OP_OperatorTable* table)
{
    OP_Operator* newOp = new OP_Operator(
        SOP_FeE_ExtremeElem_1_0::theSOPTypeName,
        "FeE Extreme Element",
        SOP_FeE_ExtremeElem_1_0::myConstructor,
        SOP_FeE_ExtremeElem_1_0::buildTemplates(),
        1,
        2,
        nullptr,
        OP_FLAG_GENERATOR,
        nullptr,
        1,
        "Five elements Elf/Group");

    newOp->setIconName("SOP_measure-2.0");
    table->addOperator(newOp);
}


//class SOP_FeE_ExtremeElem_1_0Cache : public SOP_NodeCache
//{
//public:
//    SOP_FeE_ExtremeElem_1_0Cache() : SOP_NodeCache(),
//        myPrevOutputDetailID(-1)
//    {}
//    ~SOP_FeE_ExtremeElem_1_0Cache() override {}
//
//    int64 myPrevOutputDetailID;
//};


class SOP_FeE_ExtremeElem_1_0Verb : public SOP_NodeVerb
{
public:
    SOP_FeE_ExtremeElem_1_0Verb() {}
    virtual ~SOP_FeE_ExtremeElem_1_0Verb() {}

    virtual SOP_NodeParms *allocParms() const { return new SOP_FeE_ExtremeElem_1_0Parms(); }
    //virtual SOP_NodeCache* allocCache() const { return new SOP_FeE_ExtremeElem_1_0Cache(); }
    virtual UT_StringHolder name() const { return SOP_FeE_ExtremeElem_1_0::theSOPTypeName; }

    virtual CookMode cookMode(const SOP_NodeParms *parms) const { return COOK_GENERIC; }

    virtual void cook(const CookParms &cookparms) const;

    /// This static data member automatically registers
    /// this verb class at library ldir0d time.
    static const SOP_NodeVerb::Register<SOP_FeE_ExtremeElem_1_0Verb> theVerb;
};

// The static member variable definition has to be outside the class definition.
// The declaration is inside the class.
const SOP_NodeVerb::Register<SOP_FeE_ExtremeElem_1_0Verb> SOP_FeE_ExtremeElem_1_0Verb::theVerb;

const SOP_NodeVerb *
SOP_FeE_ExtremeElem_1_0::cookVerb() const 
{ 
    return SOP_FeE_ExtremeElem_1_0Verb::theVerb.get();
}







static GA_AttributeOwner
sopAttribOwner(SOP_FeE_ExtremeElem_1_0Parms::ExtremeAttribClass parmAttribClass)
{
    using namespace SOP_FeE_ExtremeElem_1_0Enums;
    switch (parmAttribClass)
    {
    case ExtremeAttribClass::PRIM:      return GA_ATTRIB_PRIMITIVE;  break;
    case ExtremeAttribClass::POINT:     return GA_ATTRIB_POINT;      break;
    case ExtremeAttribClass::VERTEX:    return GA_ATTRIB_VERTEX;     break;
    case ExtremeAttribClass::DETAIL:    return GA_ATTRIB_DETAIL;     break;
    }
    UT_ASSERT_MSG(0, "Unhandled Geo0 Class type!");
    return GA_ATTRIB_INVALID;
}


static GA_GroupType
sopGroupType(SOP_FeE_ExtremeElem_1_0Parms::GroupType parmGroupType)
{
    using namespace SOP_FeE_ExtremeElem_1_0Enums;
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



static GFE_StatisticalFunction
sopStatisticalFunction(SOP_FeE_ExtremeElem_1_0Parms::StatisticalFunction parmStatisticalFunction)
{
    using namespace SOP_FeE_ExtremeElem_1_0Enums;
    switch (parmStatisticalFunction)
    {
    case StatisticalFunction::MIN:   return GFE_StatisticalFunction::Min;  break;
    case StatisticalFunction::MAX:   return GFE_StatisticalFunction::Max;  break;
    }
    UT_ASSERT_MSG(0, "Unhandled Statistical Function!");
    return GFE_StatisticalFunction::Min;
}


static GFE_MeasureType
sopMeasureType(SOP_FeE_ExtremeElem_1_0Parms::MeasureType parmMeasureType)
{
    using namespace SOP_FeE_ExtremeElem_1_0Enums;
    switch (parmMeasureType)
    {
    case MeasureType::PERIMETER:          return GFE_MeasureType::Perimeter;       break;
    case MeasureType::AREA:               return GFE_MeasureType::Area;            break;
    case MeasureType::VOLUME:             return GFE_MeasureType::Volume;          break;
    //case MeasureType::MESHPERIMETER:      return GFE_MeasureType::MeshPerimeter;   break;
    //case MeasureType::MESHAREA:           return GFE_MeasureType::MeshArea;        break;
    //case MeasureType::MESHVOLUME:         return GFE_MeasureType::MeshVolume;      break;
    }
    UT_ASSERT_MSG(0, "Unhandled GFE Measure Type!");
    return GFE_MeasureType::Area;
}

void
SOP_FeE_ExtremeElem_1_0Verb::cook(const SOP_NodeVerb::CookParms &cookparms) const
{
    auto &&sopparms = cookparms.parms<SOP_FeE_ExtremeElem_1_0Parms>();
    GA_Detail& outGeo0 = *cookparms.gdh().gdpNC();
    //auto sopcache = (SOP_FeE_ExtremeElem_1_0Cache*)cookparms.cache();

    const GA_Detail& inGeo0 = *cookparms.inputGeo(0);
    const GA_Detail* const inGeo1 = cookparms.inputGeo(1);
    
    outGeo0.replaceWith(inGeo0);



    const GA_GroupType groupType = sopGroupType(sopparms.getGroupType());
    const GFE_StatisticalFunction statisticalFunction = sopStatisticalFunction(sopparms.getStatisticalFunction());
    const GA_AttributeOwner extremeAttribClass = sopAttribOwner(sopparms.getExtremeAttribClass());
    //const GFE_MeasureType measureType = sopMeasureType(sopparms.getMeasure());
    const GFE_MeasureType measureType = sopMeasureType(sopparms.getMeasureType());
    

    
    UT_AutoInterrupt boss("Processing");
    if (boss.wasInterrupted())
        return;



    
    GFE_ExtremeElement extremeElem(outGeo0, inGeo1, &cookparms);
    
    extremeElem.setComputeParm(
        statisticalFunction, sopparms.getDelExtremeAttrib(),
        sopparms.getSubscribeRatio(), sopparms.getMinGrainSize());
    extremeElem.doDelGroupElement = sopparms.getDelElement();
    extremeElem.groupSetter.setParm(sopparms.getReverseGroup());
    
    //extremeElem.measure.measureType = measureType;
    
    // if (sopparms.getUsePieceAttrib())
    // {
    //     extremeElem.setPieceAttrib(GA_ATTRIB_PRIMITIVE, sopparms.getPieceAttrib());
    // }
    
    
    extremeElem.groupParser.setGroup(groupType, sopparms.getGroup());
    
    if (sopparms.getUseMeasureAttrib())
    {
        extremeElem.setMeasureAttrib(true);
        extremeElem.measure.measureType = measureType;
        //extremeElem.measure.setComputeParm(measureType, 1, 9999999);
    }
    else
        extremeElem.getInAttribArray().set(extremeAttribClass, sopparms.getExtremeAttrib());
    
    if (sopparms.getOutExtremeElemGroup() || sopparms.getDelElement())
        extremeElem.findOrCreateGroup(sopparms.getDelElement(), extremeAttribClass, sopparms.getExtremeElemGroupName());
    
    if (sopparms.getOutExtremeElemAttrib())
        extremeElem.setExtremeElemAttrib(false, sopparms.getExtremeElemAttribName());
    
    if (sopparms.getOutExtremeValueAttrib())
        extremeElem.setExtremeValueAttrib(false, sopparms.getExtremeValueAttribName());
    
    extremeElem.computeAndBumpDataId();

    extremeElem.delOrVisualizeGroup();


}


