
//#define UT_ASSERT_LEVEL 3
#include "SOP_FeE_CurveSetExpand2d_3_0.h"


#include "SOP_FeE_CurveSetExpand2d_3_0.proto.h"

#include "GA/GA_Detail.h"
#include "PRM/PRM_TemplateBuilder.h"
#include "UT/UT_Interrupt.h"
#include "UT/UT_DSOVersion.h"




#include "GFE/GFE_CurveSetExpand2d.h"



using namespace SOP_FeE_CurveSetExpand2d_3_0_Namespace;


static const char *theDsFile = R"THEDSFILE(
{
    name        parameters
    parm {
        name    "group"
        cppname "Group"
        label   "Group"
        type    string
        default { "" }
        parmtag { "script_action" "import soputils\nkwargs['geometrytype'] = (hou.geometryType.Primitives,)\nkwargs['inputindex'] = 0\nsoputils.selectGroupParm(kwargs)" }
        parmtag { "script_action_help" "Select geometry from an available viewport.\nShift-click to turn on Select Groups." }
        parmtag { "script_action_icon" "BUTTONS_reselect" }
    }
    parm {
        name    "distributionType"
        cppname "DistributionType"
        label   "Distribution Type"
        type    ordinal
        default { "arcLength" }
        menu {
            "arcLength" "Arc Length"
            "average"   "Average"
        }
    }
    parm {
        name    "uvAttrib"
        cppname "UVAttrib"
        label   "UV Attribute"
        type    string
        default { "uv" }
        parmtag { "sidefx::attrib_access" "write" }
    }
    parm {
        name    "uvClass"
        cppname "UVClass"
        label   "UV Class"
        type    ordinal
        default { "point" }
        menu {
            "point"     "Point"
            "vertex"    "Vertex"
        }
    }

    parm {
        name    "uniScale"
        cppname "UniScale"
        label   "Uniform Scale"
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
        default { 64 }
        range   { 0! 2048 }
    }
}
)THEDSFILE";

PRM_Template*
SOP_FeE_CurveSetExpand2d_3_0::buildTemplates()
{
    static PRM_TemplateBuilder templ("SOP_FeE_CurveSetExpand2d_3_0.C"_sh, theDsFile);
    if (templ.justBuilt())
    {
        //templ.setChoiceListPtr("group"_sh, &SOP_Node::groupMenu);
        templ.setChoiceListPtr("posAttribName"_sh, &SOP_Node::allTextureCoordMenu);
        
    }
    return templ.templates();
}

const UT_StringHolder SOP_FeE_CurveSetExpand2d_3_0::theSOPTypeName("FeE::curveSetExpand2d::3.0"_sh);

void
newSopOperator(OP_OperatorTable* table)
{
    OP_Operator* newOp = new OP_Operator(
        SOP_FeE_CurveSetExpand2d_3_0::theSOPTypeName,
        "FeE Curve Set Expand 2D",
        SOP_FeE_CurveSetExpand2d_3_0::myConstructor,
        SOP_FeE_CurveSetExpand2d_3_0::buildTemplates(),
        1,
        1,
        nullptr,
        OP_FLAG_GENERATOR,
        nullptr,
        1,
        "Five elements Elf/Generator/Poly Expand");

    newOp->setIconName("SOP_polyexpand2d");
    table->addOperator(newOp);

}





class SOP_FeE_CurveSetExpand2d_3_0Verb : public SOP_NodeVerb
{
public:
    SOP_FeE_CurveSetExpand2d_3_0Verb() {}
    virtual ~SOP_FeE_CurveSetExpand2d_3_0Verb() {}

    virtual SOP_NodeParms *allocParms() const { return new SOP_FeE_CurveSetExpand2d_3_0Parms(); }
    virtual UT_StringHolder name() const { return SOP_FeE_CurveSetExpand2d_3_0::theSOPTypeName; }

    virtual CookMode cookMode(const SOP_NodeParms *parms) const { return COOK_GENERIC; }

    virtual void cook(const CookParms &cookparms) const;
    
    /// This static data member automatically registers
    /// this verb class at library load time.
    static const SOP_NodeVerb::Register<SOP_FeE_CurveSetExpand2d_3_0Verb> theVerb;
};

// The static member variable definition has to be outside the class definition.
// The declaration is inside the class.
const SOP_NodeVerb::Register<SOP_FeE_CurveSetExpand2d_3_0Verb> SOP_FeE_CurveSetExpand2d_3_0Verb::theVerb;

const SOP_NodeVerb *
SOP_FeE_CurveSetExpand2d_3_0::cookVerb() const 
{ 
    return SOP_FeE_CurveSetExpand2d_3_0Verb::theVerb.get();
}





static GA_GroupType
sopGroupType(SOP_FeE_CurveSetExpand2d_3_0Parms::GroupType parmgrouptype)
{
    using namespace SOP_FeE_CurveSetExpand2d_3_0Enums;
    switch (parmgrouptype)
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
SOP_FeE_CurveSetExpand2d_3_0Verb::cook(const SOP_NodeVerb::CookParms &cookparms) const
{
    auto&& sopparms = cookparms.parms<SOP_FeE_CurveSetExpand2d_3_0Parms>();
    GA_Detail& outGeo0 = *cookparms.gdh().gdpNC();
    //auto sopcache = (SOP_FeE_CurveSetExpand2d_3_0Cache*)cookparms.cache();

    const GA_Detail& inGeo0 = *cookparms.inputGeo(0);

    outGeo0.replaceWith(inGeo0);


    const GA_GroupType groupType = sopGroupType(sopparms.getGroupType());
    
    UT_AutoInterrupt boss("Processing");
    if (boss.wasInterrupted())
        return;
    
    GFE_CurveSetExpand2d curveSetExpand2d(outGeo0, cookparms);
    
    curveSetExpand2d.setComputeParm(sopparms.getFirstIndex(), sopparms.getNegativeIndex(), sopparms.getOutAsOffset(),
        sopparms.getSubscribeRatio(), sopparms.getMinGrainSize());

    curveSetExpand2d.groupParser.setGroup(groupType, sopparms.getGroup());
    curveSetExpand2d.computeAndBumpDataId();

}

