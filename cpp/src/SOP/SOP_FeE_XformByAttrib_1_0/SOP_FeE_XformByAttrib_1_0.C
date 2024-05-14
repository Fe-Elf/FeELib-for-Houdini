
//#define UT_ASSERT_LEVEL 3
#include "SOP_FeE_XformByAttrib_1_0.h"


#include "SOP_FeE_XformByAttrib_1_0.proto.h"

#include "GA/GA_Detail.h"
#include "PRM/PRM_TemplateBuilder.h"
#include "UT/UT_Interrupt.h"
#include "UT/UT_DSOVersion.h"




#include "GFE/GFE_XformByAttrib.h"




using namespace SOP_FeE_XformByAttrib_1_0_Namespace;


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
        name    "xformAttribClass"
        cppname "XformAttribClass"
        label   "Transform Attribute Class"
        type    ordinal
        default { "point" }
        menu {
            "prim"        "Primitive"
            "point"       "Point"
            "vertex"      "Vertex"
            "detail"      "Detail"
        //    "pointVertex" "PointVertex"
        //    "all"         "All"
        }
    }
    parm {
        name    "xformAttrib"
        cppname "XformAttrib"
        label   "Transform Attribute"
        type    string
        default { "xform" }
    }
    parm {
        name    "primAttribToXform"
        cppname "PrimAttribToXform"
        label   "Prim Attrib to Transform"
        type    string
        default { "" }
    }
    parm {
        name    "pointAttribToXform"
        cppname "PointAttribToXform"
        label   "Point Attrib to Transform"
        type    string
        default { "" }
    }
    parm {
        name    "vertexAttribToXform"
        cppname "VertexAttribToXform"
        label   "Vertex Attrib to Transform"
        type    string
        default { "" }
    }
    parm {
        name    "detailAttribToXform"
        cppname "DetailAttribToXform"
        label   "Detail Attrib to Transform"
        type    string
        default { "" }
    }


    parm {
        name    "invertXform"
        cppname "InvertXform"
        label   "Invert Transform"
        type    toggle
        default { "off" }
    }
    parm {
        name    "preserveLength"
        cppname "PreserveLength"
        label   "Preserve Length"
        type    toggle
        default { "0" }
    }
    parm {
        name    "delXformAttrib"
        cppname "DelXformAttrib"
        label   "Delete Transform Attribute"
        type    toggle
        default { "on" }
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
SOP_FeE_XformByAttrib_1_0::buildTemplates()
{
    static PRM_TemplateBuilder templ("SOP_FeE_XformByAttrib_1_0.C"_sh, theDsFile);
    if (templ.justBuilt())
    {
        templ.setChoiceListPtr("group"_sh,               &SOP_Node::allGroupMenu);
        
        templ.setChoiceListPtr("xformAttrib"_sh,         &SOP_Node::allAttribMenu);
        templ.setChoiceListPtr("primAttribToXform"_sh,   &SOP_Node::primAttribMenu);
        templ.setChoiceListPtr("pointAttribToXform"_sh,  &SOP_Node::pointAttribMenu);
        templ.setChoiceListPtr("vertexAttribToXform"_sh, &SOP_Node::vertexAttribMenu);
        templ.setChoiceListPtr("detailAttribToXform"_sh, &SOP_Node::detailAttribMenu);
    }
    return templ.templates();
}

const UT_StringHolder SOP_FeE_XformByAttrib_1_0::theSOPTypeName("FeE::xformByAttrib::1.0"_sh);

void
newSopOperator(OP_OperatorTable* table)
{
    OP_Operator* newOp = new OP_Operator(
        SOP_FeE_XformByAttrib_1_0::theSOPTypeName,
        "FeE Transform by Attribute",
        SOP_FeE_XformByAttrib_1_0::myConstructor,
        SOP_FeE_XformByAttrib_1_0::buildTemplates(),
        1,
        2,
        nullptr,
        OP_FLAG_GENERATOR,
        nullptr,
        1,
        "Five elements Elf/Filter/Match");

    newOp->setIconName("SOP_xformbyattrib");
    table->addOperator(newOp);

}





class SOP_FeE_XformByAttrib_1_0Verb : public SOP_NodeVerb
{
public:
    SOP_FeE_XformByAttrib_1_0Verb() {}
    virtual ~SOP_FeE_XformByAttrib_1_0Verb() {}

    virtual SOP_NodeParms *allocParms() const { return new SOP_FeE_XformByAttrib_1_0Parms(); }
    virtual UT_StringHolder name() const { return SOP_FeE_XformByAttrib_1_0::theSOPTypeName; }

    virtual CookMode cookMode(const SOP_NodeParms *parms) const { return COOK_INPLACE; }

    virtual void cook(const CookParms &cookparms) const;
    
    /// This static data member automatically registers
    /// this verb class at library load time.
    static const SOP_NodeVerb::Register<SOP_FeE_XformByAttrib_1_0Verb> theVerb;
};

// The static member variable definition has to be outside the class definition.
// The declaration is inside the class.
const SOP_NodeVerb::Register<SOP_FeE_XformByAttrib_1_0Verb> SOP_FeE_XformByAttrib_1_0Verb::theVerb;

const SOP_NodeVerb *
SOP_FeE_XformByAttrib_1_0::cookVerb() const 
{ 
    return SOP_FeE_XformByAttrib_1_0Verb::theVerb.get();
}







static GA_AttributeOwner
sopAttribOwner(const SOP_FeE_XformByAttrib_1_0Parms::XformAttribClass parmAttribClass)
{
    using namespace SOP_FeE_XformByAttrib_1_0Enums;
    switch (parmAttribClass)
    {
    case XformAttribClass::PRIM:      return GA_ATTRIB_PRIMITIVE;  break;
    case XformAttribClass::POINT:     return GA_ATTRIB_POINT;      break;
    case XformAttribClass::VERTEX:    return GA_ATTRIB_VERTEX;     break;
    case XformAttribClass::DETAIL:    return GA_ATTRIB_DETAIL;     break;
    }
    UT_ASSERT_MSG(0, "Unhandled Geo0 Class type!");
    return GA_ATTRIB_INVALID;
}


static GA_GroupType
sopGroupType(const SOP_FeE_XformByAttrib_1_0Parms::GroupType parmGroupType)
{
    using namespace SOP_FeE_XformByAttrib_1_0Enums;
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
SOP_FeE_XformByAttrib_1_0Verb::cook(const SOP_NodeVerb::CookParms &cookparms) const
{
    auto&& sopparms = cookparms.parms<SOP_FeE_XformByAttrib_1_0Parms>();
    GA_Detail& outGeo0 = *cookparms.gdh().gdpNC();
    //auto sopcache = (SOP_FeE_XformByAttrib_1_0Cache*)cookparms.cache();

    //const GA_Detail& inGeo0 = *cookparms.inputGeo(0);
    const GA_Detail* const inGeo1 = cookparms.inputGeo(1);

    //outGeo0.replaceWith(inGeo0);


    
    const GA_GroupType groupType = sopGroupType(sopparms.getGroupType());

    const GA_AttributeOwner owner = sopAttribOwner(sopparms.getXformAttribClass());

    
    UT_AutoInterrupt boss("Processing");
    if (boss.wasInterrupted())
        return;

    GFE_XformByAttrib xformByAttrib(outGeo0, inGeo1, cookparms);
    
    xformByAttrib.getOutAttribArray().appendPrimitives(sopparms.getPrimAttribToXform());
    xformByAttrib.getOutAttribArray().appendPoints    (sopparms.getPointAttribToXform());
    xformByAttrib.getOutAttribArray().appendVertices  (sopparms.getVertexAttribToXform());
    xformByAttrib.getOutAttribArray().appendDetails   (sopparms.getDetailAttribToXform());

    xformByAttrib.setXformAttrib(owner, sopparms.getXformAttrib());

    xformByAttrib.setComputeParm(sopparms.getPreserveLength(), sopparms.getInvertXform(), sopparms.getDelXformAttrib(),
        sopparms.getSubscribeRatio(), sopparms.getMinGrainSize());

    xformByAttrib.groupParser.setGroup(groupType, sopparms.getGroup());
    
    xformByAttrib.computeAndBumpDataId();

}

