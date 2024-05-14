
//#define UT_ASSERT_LEVEL 3
#include "SOP_FeE_PolyReduce2D_4_0.h"


#include "SOP_FeE_PolyReduce2D_4_0.proto.h"

#include "GA/GA_Detail.h"
#include "PRM/PRM_TemplateBuilder.h"
#include "UT/UT_Interrupt.h"
#include "UT/UT_DSOVersion.h"



#include "GFE/GFE_PolyReduce2D.h"



using namespace SOP_FeE_PolyReduce2D_4_0_Namespace;


static const char *theDsFile = R"THEDSFILE(
{
    name	parameters
    parm {
        name    "primGroup"
        cppname "PrimGroup"
        label   "Prim Group"
        type    string
        default { "" }
        range   { 0 1 }
        parmtag { "script_action" "import soputils\nkwargs['geometrytype'] = hou.geometryType.Primitives\nkwargs['inputindex'] = 0\nsoputils.selectGroupParm(kwargs)" }
        parmtag { "script_action_help" "Select geometry from an available viewport." }
        parmtag { "script_action_icon" "BUTTONS_reselect" }
    }
    parm {
        name    "delInLinePoint"
        cppname "DelInLinePoint"
        label   "Delete In-Line Point"
        type    toggle
        default { "1" }
    }
    parm {
        name    "threshold_inlineAngle"
        cppname "Threshold_inlineAngle"
        label   "Threshold In-Line Angle"
        type    angle
        default { "1e-05" }
        disablewhen "{ delInLinePoint == 0 }"
        range   { 0! 180! }
    }
    parm {
        name    "limitByGeoProperty"
        cppname "LimitByGeoProperty"
        label   "Limit by Geometry Property"
        type    toggle
        default { "1" }
    }
    parm {
        name    "geoPropertyType"
        cppname "GeoPropertyType"
        label   "Geometry Property Type"
        type    ordinal
        default { "dist" }
        menu {
            "angle" "Angle"
            "dist"  "Distance"
            "roc"   "Raidus of Curvature"
        }
    }
    parm {
        name    "maxAngle"
        cppname "MaxAngle"
        label   "Max Angle (Dot)"
        type    angle
        default { "150" }
        disablewhen "{ limitByGeoProperty == 0 } { geoPropertyType != angle }"
        range   { 90! 180! }
        parmtag { "units" "m1" }
    }
    parm {
        name    "maxDist"
        cppname "MaxDist"
        label   "Max Distance"
        type    log
        default { "0.0001" }
        disablewhen "{ limitByGeoProperty == 0 } { geoPropertyType != dist geoPropertyType != roc }"
        range   { 0.001 10 }
    }
    parm {
        name    "limitMinPoint"
        cppname "LimitMinPoint"
        label   "Minimum Point"
        type    toggle
        nolabel
        joinnext
        default { "off" }
    }
    parm {
        name    "minPoint"
        cppname "MinPoint"
        label   "Min Point"
        type    integer
        default { "10" }
        disablewhen "{ limitMinPoint == 0 }"
        range   { 1! 50 }
    }
    parm {
        name    "coverSourcePoly"
        cppname "CoverSourcePoly"
        label   "Cover Source Poly"
        type    toggle
        default { "off" }
    }
    parm {
        name    "polyReduce2D_groupName"
        cppname "PolyReduce2D_GroupName"
        label   "Group Name"
        type    string
        default { "polyReduce2D" }
        disablewhen "{ delPoint == 1 }"
        parmtag { "script_action" "import soputils kwargs['geometrytype'] = hou.geometryType.Points kwargs['inputindex'] = 0 soputils.selectGroupParm(kwargs)" }
        parmtag { "script_action_help" "Select geometry from an available viewport." }
        parmtag { "script_action_icon" "BUTTONS_reselect" }
    }

    parm {
        name    "reverseGroup"
        cppname "ReverseGroup"
        label   "Reverse Group"
        type    toggle
        disablewhen "{ coverSourcePoly == 1 }"
        default { "0" }
    }

    parm {
        name    "delPoint"
        cppname "DeletePoint"
        label   "Delete Point"
        type    toggle
        disablewhen "{ coverSourcePoly == 1 }"
        default { "1" }
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
SOP_FeE_PolyReduce2D_4_0::buildTemplates()
{
    static PRM_TemplateBuilder templ("SOP_FeE_PolyReduce2D_4_0.C"_sh, theDsFile);
    if (templ.justBuilt())
    {
        templ.setChoiceListPtr("primGroup"_sh, &SOP_Node::primGroupMenu);
    }
    return templ.templates();
}





const UT_StringHolder SOP_FeE_PolyReduce2D_4_0::theSOPTypeName("FeE::polyReduce2d::4.0"_sh);

void
newSopOperator(OP_OperatorTable* table)
{
    OP_Operator* newOp = new OP_Operator(
        SOP_FeE_PolyReduce2D_4_0::theSOPTypeName,
        "FeE Poly Reduce 2D",
        SOP_FeE_PolyReduce2D_4_0::myConstructor,
        SOP_FeE_PolyReduce2D_4_0::buildTemplates(),
        1,
        1,
        nullptr,
        OP_FLAG_GENERATOR,
        nullptr,
        1,
        "Five elements Elf/Topo/Optimize");

    newOp->setIconName("SOP_edgedivide");
    table->addOperator(newOp);
}




class SOP_FeE_PolyReduce2D_4_0Verb : public SOP_NodeVerb
{
public:
    SOP_FeE_PolyReduce2D_4_0Verb() {}
    virtual ~SOP_FeE_PolyReduce2D_4_0Verb() {}

    virtual SOP_NodeParms *allocParms() const { return new SOP_FeE_PolyReduce2D_4_0Parms(); }
    virtual UT_StringHolder name() const { return SOP_FeE_PolyReduce2D_4_0::theSOPTypeName; }

    virtual CookMode cookMode(const SOP_NodeParms *parms) const { return COOK_GENERIC; }

    virtual void cook(const CookParms &cookparms) const;
    
    static const SOP_NodeVerb::Register<SOP_FeE_PolyReduce2D_4_0Verb> theVerb;
};

const SOP_NodeVerb::Register<SOP_FeE_PolyReduce2D_4_0Verb> SOP_FeE_PolyReduce2D_4_0Verb::theVerb;

const SOP_NodeVerb *
SOP_FeE_PolyReduce2D_4_0::cookVerb() const 
{ 
    return SOP_FeE_PolyReduce2D_4_0Verb::theVerb.get();
}











static GFE_PolyReduce2D_GeoPropertyType
sopGeoPropertyType(SOP_FeE_PolyReduce2D_4_0Parms::GeoPropertyType parmgrouptype)
{
    using namespace SOP_FeE_PolyReduce2D_4_0Enums;
    switch (parmgrouptype)
    {
    case GeoPropertyType::ANGLE:     return GFE_PolyReduce2D_GeoPropertyType::ANGLE;    break;
    case GeoPropertyType::DIST:      return GFE_PolyReduce2D_GeoPropertyType::DIST;     break;
    case GeoPropertyType::ROC:       return GFE_PolyReduce2D_GeoPropertyType::ROC;      break;
    }
    UT_ASSERT_MSG(0, "Unhandled Geo Property Type!");
    return GFE_PolyReduce2D_GeoPropertyType::ANGLE;
}





void
SOP_FeE_PolyReduce2D_4_0Verb::cook(const SOP_NodeVerb::CookParms& cookparms) const
{
    auto&& sopparms = cookparms.parms<SOP_FeE_PolyReduce2D_4_0Parms>();
    GA_Detail& outGeo0 = *cookparms.gdh().gdpNC();
    //auto sopcache = (SOP_FeE_PolyReduce2D_4_0Cache*)cookparms.cache();

    const GA_Detail& inGeo0 = *cookparms.inputGeo(0);

    outGeo0.replaceWith(inGeo0);


    const fpreal minPoint = sopparms.getMinPoint();

    const fpreal threshold_maxDist = sopparms.getMaxDist();

    const fpreal threshold_maxAngle = sopparms.getMaxAngle();
    //const fpreal threshold_maxAngleRadians = GFE_Math::radians(threshold_maxAngle);


    const fpreal threshold_inlineAngle = sopparms.getThreshold_inlineAngle();
    //const fpreal threshold_inlineAngleRadians = GFE_Math::radians(threshold_inlineAngle);


    const GFE_PolyReduce2D_GeoPropertyType geoPropertyType = sopGeoPropertyType(sopparms.getGeoPropertyType());


    
    UT_AutoInterrupt boss("Processing");
    if (boss.wasInterrupted())
        return;
    


    GFE_PolyReduce2D polyReduce2D(outGeo0, cookparms);
    polyReduce2D.groupParser.setPrimitiveGroup(sopparms.getPrimGroup());
    polyReduce2D.getOutGroupArray().findOrCreate(delPoint, GA_GROUP_POINT, sopparms.getPolyReduce2D_GroupName());

    polyReduce2D.setComputeParm(
        sopparms.getDelInLinePoint(), 1e-05,
        limitByGeoProperty, geoPropertyType, 1e-05, threshold_maxDist,
        limitMinPoint, minPoint,
        sopparms.getCoverSourcePoly(), sopparms.getReverseGroup(), sopparms.getDeletePoint(),
        sopparms.getSubscribeRatio(), sopparms.getMinGrainSize());

    polyReduce2D.setLimitByGeoProperty();
    polyReduce2D.setLimitMinPoint();
    const bool limitByGeoProperty = sopparms.getLimitByGeoProperty();
    const bool limitMinPoint = sopparms.getLimitMinPoint();

    polyReduce2D.primInlinePoint.setThreshold_inlineCosRadians(threshold_inlineAngle);
    polyReduce2D.setThreshold_maxCosRadians(threshold_maxAngle);

    polyReduce2D.computeAndBumpDataId();
    polyReduce2D.delOrVisualizeGroup();

    //GA_PointGroup* const polyReduce2DPtGroup = GFE_PolyReduce2D::polyReduce2D(cookparms, outGeo0,
    //    sopparms.getGroup(), sopparms.getPolyReduce2D_GroupName(),
    //    sopparms.getDelInLinePoint(), threshold_inlineAngleRadians,
    //    limitByGeoProperty, geoPropertyType, threshold_maxAngle, threshold_maxDist,
    //    limitMinPoint, minPoint,
    //    coverSourcePoly, reverseGroup, delPoint,
    //    subscribeRatio, minGrainSize);


    //if (delPoint || sopparms.getDelInLinePoint())
    //{
    //    outGeo0->bumpDataIdsForAddOrRemove(1, 1, 1);
    //}
    //
    //if (!delPoint)
    //{
    //    cookparms.getNode()->setHighlight(true);
    //    if (polyReduce2DPtGroup)
    //    {
    //        cookparms.select(*polyReduce2DPtGroup);
    //        polyReduce2DPtGroup->bumpDataId();
    //    }
    //}

    //GFE_TopologyReference::outTopoAttrib(outGeo0, sopparms.getOutTopoAttrib());

    //tmpGeoH0.deleteGdp();

}



namespace SOP_FeE_PolyReduce2D_4_0_Namespace {

} // End SOP_FeE_PolyReduce2D_4_0_Namespace namespace
