
//#define UT_ASSERT_LEVEL 3

#include "SOP_FeE_UVScaletoWorldSize_3_0.h"
#include "SOP_FeE_UVScaletoWorldSize_3_0.proto.h"

#include <GFE/GFE_UVScaletoWorldSize.h>

using namespace SOP_FeE_UVScaletoWorldSize_3_0_Namespace;

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
        name    "uvAttribClass"
        cppname "UVAttribClass"
        label   "UV Attribute Class"
        type    ordinal
        default { "auto" }
        menu {
            "auto"      "Auto"
            "point"     "Point"
            "vertex"    "Vertex"
        }
    }
    parm {
        name    "uvAttrib"
        cppname "UVAttrib"
        label   "UV Attribute"
        type    string
        default { "uv" }
    }
    parm {
        name    "computeUVAreaInPiece"
        cppname "ComputeUVAreaInPiece"
        label   "Compute UV Area In Pieces"
        type    toggle
        default { "1" }
    }



    parm {
        name    "doUVScalex"
        cppname "DoUVScalex"
        label   "Do Scale X"
        type    toggle
        default { "1" }
        nolabel
        joinnext
    }
    parm {
        name    "uvScalex"
        cppname "UVScalex"
        label   "UV Scale X"
        type    float
        default { 1 }
        range   { -1 10 }
        disablewhen "{ doUVScalex == 0 }"
    }



    parm {
        name    "doUVScaley"
        cppname "DoUVScaley"
        label   "Do Scale Y"
        type    toggle
        default { "1" }
        nolabel
        joinnext
    }
    parm {
        name    "uvScaley"
        cppname "UVScaley"
        label   "UV Scale Y"
        type    float
        default { 1 }
        range   { -1 10 }
        disablewhen "{ doUVScaley == 0 }"
    }



    parm {
        name    "doUVScalez"
        cppname "DoUVScalez"
        label   "Do Scale Z"
        type    toggle
        default { "1" }
        nolabel
        joinnext
    }
    parm {
        name    "uvScalez"
        cppname "UVScalez"
        label   "UV Scale Z"
        type    float
        default { 1 }
        range   { -1 10 }
        disablewhen "{ doUVScalez == 0 }"
    }

    parm {
        name    "doUVScalew"
        cppname "DoUVScalew"
        label   "Do Scale W"
        type    toggle
        default { "1" }
        nolabel
        joinnext
    }
    parm {
        name    "uvScalew"
        cppname "UVScalew"
        label   "UV Scale W"
        type    float
        default { 1 }
        range   { -1 10 }
        disablewhen "{ doUVScalew == 0 }"
    }

    parm {
        name    "uvScale"
        cppname "UVScale"
        label   "UV Scale"
        type    float
        default { 1 }
        range   { -10 10 }
        disablewhen "{ doUVScalex == 0 doUVScaley == 0 doUVScalez == 0 doUVScalew == 0 }"
    }



    parm {
        name    "uvSplitDistThreshold"
        cppname "UVSplitDistThreshold"
        label   "UV Split Distance Threshold"
        type    log
        default { 1e-05 }
        range   { 0! 10 }
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
SOP_FeE_UVScaletoWorldSize_3_0::buildTemplates()
{
    static PRM_TemplateBuilder templ("SOP_FeE_UVScaletoWorldSize_3_0.C"_sh, theDsFile);
    if (templ.justBuilt())
    {
        templ.setChoiceListPtr("group"_sh, &SOP_Node::allGroupMenu);
        templ.setChoiceListPtr("uvAttrib"_sh, &SOP_Node::allAttribReplaceMenu);
    }
    return templ.templates();
}


const UT_StringHolder SOP_FeE_UVScaletoWorldSize_3_0::theSOPTypeName("FeE::uvScaletoWorldSize::3.0"_sh);

void
newSopOperator(OP_OperatorTable* table)
{
    OP_Operator* newOp = new OP_Operator(
        SOP_FeE_UVScaletoWorldSize_3_0::theSOPTypeName,
        "FeE UV Scale to World Size",
        SOP_FeE_UVScaletoWorldSize_3_0::myConstructor,
        SOP_FeE_UVScaletoWorldSize_3_0::buildTemplates(),
        1,
        1,
        nullptr,
        OP_FLAG_GENERATOR,
        nullptr,
        1,
        "Five elements Elf/Data/Orient");

    newOp->setIconName("SOP_uvtransform-2.0");
    table->addOperator(newOp);
}



//class SOP_FeE_UVScaletoWorldSize_3_0Cache : public SOP_NodeCache
//{
//public:
//    SOP_FeE_UVScaletoWorldSize_3_0Cache() : SOP_NodeCache(),
//        myPrevOutputDetailID(-1)
//    {}
//    ~SOP_FeE_UVScaletoWorldSize_3_0Cache() override {}
//
//    int64 myPrevOutputDetailID;
//};


class SOP_FeE_UVScaletoWorldSize_3_0Verb : public SOP_NodeVerb
{
public:
    SOP_FeE_UVScaletoWorldSize_3_0Verb() {}
    virtual ~SOP_FeE_UVScaletoWorldSize_3_0Verb() {}

    virtual SOP_NodeParms *allocParms() const { return new SOP_FeE_UVScaletoWorldSize_3_0Parms(); }
    //virtual SOP_NodeCache* allocCache() const { return new SOP_FeE_UVScaletoWorldSize_3_0Cache(); }
    virtual UT_StringHolder name() const { return SOP_FeE_UVScaletoWorldSize_3_0::theSOPTypeName; }

    virtual CookMode cookMode(const SOP_NodeParms *parms) const { return COOK_GENERIC; }

    virtual void cook(const CookParms &cookparms) const;

    static const SOP_NodeVerb::Register<SOP_FeE_UVScaletoWorldSize_3_0Verb> theVerb;
};

const SOP_NodeVerb::Register<SOP_FeE_UVScaletoWorldSize_3_0Verb> SOP_FeE_UVScaletoWorldSize_3_0Verb::theVerb;

const SOP_NodeVerb *
SOP_FeE_UVScaletoWorldSize_3_0::cookVerb() const 
{ 
    return SOP_FeE_UVScaletoWorldSize_3_0Verb::theVerb.get();
}




static GA_AttributeOwner
sopAttribOwner(const SOP_FeE_UVScaletoWorldSize_3_0Parms::UVAttribClass parmAttribClass)
{
    using namespace SOP_FeE_UVScaletoWorldSize_3_0Enums;
    switch (parmAttribClass)
    {
    case UVAttribClass::AUTO:      return GA_ATTRIB_DETAIL;     break;//not detail but means Auto
    case UVAttribClass::POINT:     return GA_ATTRIB_POINT;      break;
    case UVAttribClass::VERTEX:    return GA_ATTRIB_VERTEX;     break;
    }
    UT_ASSERT_MSG(0, "Unhandled Geo0 Class type!");
    return GA_ATTRIB_INVALID;
}

static GA_GroupType
sopGroupType(const SOP_FeE_UVScaletoWorldSize_3_0Parms::GroupType parmGroupType)
{
    using namespace SOP_FeE_UVScaletoWorldSize_3_0Enums;
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





void
SOP_FeE_UVScaletoWorldSize_3_0Verb::cook(const SOP_NodeVerb::CookParms &cookparms) const
{
    auto &&sopparms = cookparms.parms<SOP_FeE_UVScaletoWorldSize_3_0Parms>();
    GA_Detail& outGeo0 = *cookparms.gdh().gdpNC();
    //auto sopcache = (SOP_FeE_UVScaletoWorldSize_3_0Cache*)cookparms.cache();

    //const GA_Detail& inGeo0 = *cookparms.inputGeo(0);

    //outGeo0.replaceWith(inGeo0);


    const bool doUVScalex = sopparms.getDoUVScalex();
    const bool doUVScaley = sopparms.getDoUVScaley();
    const bool doUVScalez = sopparms.getDoUVScalez();
    if (!doUVScalex && !doUVScaley && !doUVScalez)
        return;

    const UT_StringHolder& geo0AttribNames = sopparms.getUVAttrib();
    if (GFE_Type::isInvalid(geo0AttribNames))
        return;

    UT_AutoInterrupt boss("Processing");
    if (boss.wasInterrupted())
        return;





    const GA_GroupType groupType = sopGroupType(sopparms.getGroupType());


#if 1
    UT_Vector4R uvScale(sopparms.getUVScalex(), sopparms.getUVScaley(), sopparms.getUVScalez(), sopparms.getUVScalew());
    uvScale *= sopparms.getUVScale();
#else
    UT_Vector3R uvScale(doUVScalex, doUVScaley, doUVScalez);
    uvScale *= UT_Vector3R(uvScalex, uvScaley, uvScalez) * uniScale;
#endif

    
    const bool computeUVAreaInPiece = sopparms.getComputeUVAreaInPiece();
    const GA_AttributeOwner geo0AttribClass = sopAttribOwner(sopparms.getUVAttribClass());

    const fpreal uvSplitDistThreshold = sopparms.getUVSplitDistThreshold();


    gfe::UVScaletoWorldSize uvScaletoWorldSize(outGeo0, cookparms);
    uvScaletoWorldSize.groupParser.setGroup(groupType, sopparms.getGroup());
    uvScaletoWorldSize.getOutAttribArray().set(geo0AttribClass, geo0AttribNames);
    uvScaletoWorldSize.setComputeParm(computeUVAreaInPiece, uvScale,
        doUVScalex, doUVScaley, doUVScalez,
        sopparms.getOutTopoAttrib(),
        sopparms.getSubscribeRatio(), sopparms.getMinGrainSize());

    uvScaletoWorldSize.computeAndBumpDataId();

}

