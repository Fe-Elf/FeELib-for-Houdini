
//#define UT_ASSERT_LEVEL 3
#include "SOP_FeE_Normal2D_1_0.h"


#include "SOP_FeE_Normal2D_1_0.proto.h"



#include <GFE/GFE_Normal2D.h>




using namespace SOP_FeE_Normal2D_1_0_Namespace;



static const char *SOP_FeE_Normal2D_1_0_theDsFile = R"THEDSFILE(
{
    name        parameters
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
        name    "posAttrib"
        cppname "PosAttrib"
        label   "Pos Attrib"
        type    string
        default { "P" }
    }



    parm {
        name    "normal2DAttribName"
        cppname "Normal2DAttribName"
        label   "Normal2D Attrib Name"
        type    string
        default { "N" }
    }



    parm {
        name    "extrapolateEnds"
        cppname "ExtrapolateEnds"
        label   "Extrapolate Ends"
        type    toggle
        default { "1" }
    }
    parm {
        name    "scaleByTurns"
        cppname "ScaleByTurns"
        label   "Scale by Turns"
        type    toggle
        default { "1" }
    }
    parm {
        name    "normalize"
        cppname "Normalize"
        label   "Normalize"
        type    toggle
        default { "1" }
        disablewhen "{ scaleByTurns == 1 }"
    }


    parm {
        name    "uniScale"
        cppname "UniScale"
        label   "Uniform Scale"
        type    log
        default { "1" }
        range   { 0.01 100 }
    }
    parm {
        name    "blend"
        cppname "Blend"
        label   "Blend"
        type    float
        default { "0" }
        range   { -1! 1! }
    }







    parm {
        name    "useConstantNormal3D"
        cppname "UseConstantNormal3D"
        label   "Use Constant Normal 3D"
        type    toggle
        default { "0" }
    }
    parm {
        name    "findNormal3D"
        cppname "FindNormal3D"
        label   "Find Normal 3D"
        type    toggle
        default { "0" }
        disablewhen "{ useConstantNormal3D == 1 }"
    }
    parm {
        name    "addNormal3DIfNoFind"
        cppname "AddNormal3DIfNoFind"
        label   "Add Normal 3D If No Find"
        type    toggle
        default { "1" }
        disablewhen "{ useConstantNormal3D == 1 } { findNormal3D == 0 }"
    }

    parm {
        name    "normal3DAttribClass"
        cppname "Normal3DAttribClass"
        label   "Normal 3D Attrib Class"
        type    ordinal
        default { "prim" }
        disablewhen "{ useConstantNormal3D == 1 } { findNormal3D == 0 }"
        menu {
            "prim"          "Primitive"
            "point"         "Point"
            "vertex"        "Vertex"
            "detail"        "Detail"
            "pointvertex"   "PointVertex"
            "all"           "ALL"
        }
    }
    parm {
        name    "normal3DAttrib"
        cppname "Normal3DAttrib"
        label   "Normal3D Attrib"
        type    string
        default { "N" }
        disablewhen "{ useConstantNormal3D == 1 } { findNormal3D == 0 }"
    }
    parm {
        name    "defaultNormal3D"
        cppname "DefaultNormal3D"
        label   "Default Normal 3D"
        type    vector
        size    3
        default { "0" "1" "0" }
        disablewhen "{ useConstantNormal3D == 0 addNormal3DIfNoFind == 1 } { useConstantNormal3D == 0 findNormal3D == 0 }"
        range   { -1 1 }
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
SOP_FeE_Normal2D_1_0::buildTemplates()
{
    static PRM_TemplateBuilder templ("SOP_FeE_Normal2D_1_0.C"_sh, SOP_FeE_Normal2D_1_0_theDsFile);
    if (templ.justBuilt())
    {
        templ.setChoiceListPtr("group"_sh,          &SOP_Node::allGroupMenu);
        templ.setChoiceListPtr("posAttrib"_sh,      &SOP_Node::pointAttribReplaceMenu);
        templ.setChoiceListPtr("normal3DAttrib"_sh, &SOP_Node::allAttribReplaceMenu);
    }
    return templ.templates();
}


//const UT_StringHolder SOP_FeE_Normal2D_1_0::theSOPTypeName("FeE::normal2d::1.0"_sh);

void
newSopOperator(OP_OperatorTable* table)
{
    OP_Operator* newOp = new OP_Operator(
        "FeE::normal2d::1.0",
        //SOP_FeE_Normal2D_1_0::theSOPTypeName,
        "FeE Normal 2D",
        SOP_FeE_Normal2D_1_0::myConstructor,
        SOP_FeE_Normal2D_1_0::buildTemplates(),
        1,
        1,
        nullptr,
        OP_FLAG_GENERATOR,
        nullptr,
        1,
        "Five elements Elf/Data/Orient");

    newOp->setIconName("SOP_normal");
    table->addOperator(newOp);
}


//class SOP_FeE_Normal2D_1_0Cache : public SOP_NodeCache
//{
//public:
//    SOP_FeE_Normal2D_1_0Cache() : SOP_NodeCache(),
//        myPrevOutputDetailID(-1)
//    {}
//    ~SOP_FeE_Normal2D_1_0Cache() override {}
//
//    int64 myPrevOutputDetailID;
//};


class SOP_FeE_Normal2D_1_0Verb : public SOP_NodeVerb
{
public:
    SOP_FeE_Normal2D_1_0Verb() {}
    virtual ~SOP_FeE_Normal2D_1_0Verb() {}

    virtual SOP_NodeParms *allocParms() const { return new SOP_FeE_Normal2D_1_0Parms(); }
    //virtual SOP_NodeCache* allocCache() const { return new SOP_FeE_Normal2D_1_0Cache(); }
    virtual UT_StringHolder name() const { return SOP_FeE_Normal2D_1_0::theSOPTypeName; }

    virtual CookMode cookMode(const SOP_NodeParms *parms) const { return COOK_INPLACE; }

    virtual void cook(const CookParms &cookparms) const;

    static const SOP_NodeVerb::Register<SOP_FeE_Normal2D_1_0Verb> theVerb;
};

const SOP_NodeVerb::Register<SOP_FeE_Normal2D_1_0Verb> SOP_FeE_Normal2D_1_0Verb::theVerb;

const SOP_NodeVerb *
SOP_FeE_Normal2D_1_0::cookVerb() const 
{ 
    return SOP_FeE_Normal2D_1_0Verb::theVerb.get();
}




//static int
//sopAttribSearchOrder(const SOP_FeE_Normal2D_1_0Parms::Normal3DAttribClass attribClass, const GA_AttributeOwner* searchOrder)
//{
//    int size = 0;
//    using namespace SOP_FeE_Normal2D_1_0Enums;
//    switch (attribClass)
//    {
//    case Normal3DAttribClass::PRIM:          searchOrder = { GA_ATTRIB_PRIMITIVE }; size = 1; break;
//    case Normal3DAttribClass::POINT:         return GA_ATTRIB_POINT;      break;
//    case Normal3DAttribClass::VERTEX:        return GA_ATTRIB_VERTEX;     break;
//    case Normal3DAttribClass::DETAIL:        return GA_ATTRIB_DETAIL;     break;
//    case Normal3DAttribClass::POINTVERTEX:   return GA_ATTRIB_INVALID;    break;
//    case Normal3DAttribClass::ALL:           return GA_ATTRIB_OWNER_N;    break;
//    }
//    UT_ASSERT_MSG(0, "Unhandled Geo0 Class type!");
//    return GA_ATTRIB_INVALID;
//}



static GA_GroupType
sopGroupType(const SOP_FeE_Normal2D_1_0Parms::GroupType parmGroupType)
{
    using namespace SOP_FeE_Normal2D_1_0Enums;
    switch (parmGroupType)
    {
    case GroupType::GUESS:     return GA_GROUP_INVALID;    break;
    case GroupType::PRIM:      return GA_GROUP_PRIMITIVE;  break;
    case GroupType::POINT:     return GA_GROUP_POINT;      break;
    case GroupType::VERTEX:    return GA_GROUP_VERTEX;     break;
    case GroupType::EDGE:      return GA_GROUP_EDGE;       break;
    }
    UT_ASSERT_MSG(0, "Unhandled Group type!");
    return GA_GROUP_INVALID;
}



static GFE_NormalSearchOrder
sopAttribSearchOrder(const SOP_FeE_Normal2D_1_0Parms::Normal3DAttribClass parmAttribClass)
{
    using namespace SOP_FeE_Normal2D_1_0Enums;
    switch (parmAttribClass)
    {
    case Normal3DAttribClass::PRIM:          return GFE_NormalSearchOrder::Primitive;   break;
    case Normal3DAttribClass::POINT:         return GFE_NormalSearchOrder::Point;       break;
    case Normal3DAttribClass::VERTEX:        return GFE_NormalSearchOrder::Vertex;      break;
    case Normal3DAttribClass::DETAIL:        return GFE_NormalSearchOrder::Detail;      break;
    case Normal3DAttribClass::POINTVERTEX:   return GFE_NormalSearchOrder::PointVertex; break;
    case Normal3DAttribClass::ALL:           return GFE_NormalSearchOrder::All;         break;
    }
    UT_ASSERT_MSG(0, "Unhandled GFE Normal Search Order!");
    return GFE_NormalSearchOrder::Invalid;
}



void
SOP_FeE_Normal2D_1_0Verb::cook(const SOP_NodeVerb::CookParms &cookparms) const
{
    auto &&sopparms = cookparms.parms<SOP_FeE_Normal2D_1_0Parms>();
    GA_Detail& outGeo0 = *cookparms.gdh().gdpNC();
    //auto sopcache = (SOP_FeE_Normal2D_1_0Cache*)cookparms.cache();

    //const GA_Detail& inGeo0 = *cookparms.inputGeo(0);

    //outGeo0.replaceWith(inGeo0);

    const UT_StringHolder& normal2DAttribName = sopparms.getNormal2DAttribName();
    if (GFE_Type::!normal2DAttribName.isstring() || normal2DAttribName.length() == 0)
        return;

    const GA_GroupType groupType = sopGroupType(sopparms.getGroupType());
    const GFE_NormalSearchOrder geo0Normal3DSearchOrder = sopAttribSearchOrder(sopparms.getNormal3DAttribClass());
    
    UT_AutoInterrupt boss("Processing");
    if (boss.wasInterrupted())
        return;

    

    gfe::GFE_Normal2D normal2D(outGeo0, &cookparms);
    normal2D.groupParser.setGroup(groupType, sopparms.getGroup());
    
    normal2D.setComputeParm(sopparms.getSubscribeRatio(), sopparms.getMinGrainSize());
    normal2D.setComputeParm(sopparms.getExtrapolateEnds(), sopparms.getScaleByTurns());
    normal2D.setComputeParm(sopparms.getNormalize(), sopparms.getUniScale(), sopparms.getBlend());

    
    normal2D.setPositionAttrib(sopparms.getPosAttrib());
    normal2D.setNormal2DAttrib(false, sopparms.getNormal2DAttribName());
    
    const float cuspAngleDegrees = GEO_DEFAULT_ADJUSTED_CUSP_ANGLE;
    const GEO_NormalMethod method = GEO_NormalMethod::ANGLE_WEIGHTED;
    const bool copyOrigIfZero = false;
    normal2D.normal3D.setComputeParm(cuspAngleDegrees, method, copyOrigIfZero);
    normal2D.defaultNormal3D = sopparms.getDefaultNormal3D();
    
    if (!sopparms.getUseConstantNormal3D())
        normal2D.findOrCreateNormal3D(sopparms.getFindNormal3D(), sopparms.getAddNormal3DIfNoFind(),
            geo0Normal3DSearchOrder, sopparms.getNormal3DAttrib());
    
    
    
    normal2D.computeAndBumpDataId();
    normal2D.visualizeOutGroup();
}
