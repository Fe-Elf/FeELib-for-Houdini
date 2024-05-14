
//#define UT_ASSERT_LEVEL 3
#include "SOP_FeE_UVGridify_1_0.h"
#include "SOP_FeE_UVGridify_1_0.proto.h"

#include <GFE/GFE_UVGridify.h>

using namespace SOP_FeE_UVGridify_1_0_Namespace;

static const char *theDsFile = R"THEDSFILE(
{
    name        parameters
    parm {
        name    "group"
        cppname "Group"
        label   "Group"
        type    string
        default { "" }
        range   { 0 1 }
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
        name    "rowsOrColsNumMethod"
        cppname "RowsOrColsNumMethod"
        label   "Rows Or Columns num Method"
        type    ordinal
        default { "uniform" }
        menu {
            "uniform"   "Uniform"
            "rows"      "Rows"
            "cols"      "Columns"
        }
    }
    parm {
        name    "rowsOrColsNum"
        cppname "RowsOrColsNum"
        label   "Rows or Columns num"
        type    intlog
        default { "2" }
        disablewhen "{ rowsOrColsNumMethod == uniform }"
        range   { 2! 30 }
    }
    parm {
        name    "reverseUVu"
        cppname "ReverseUVu"
        label   "Reverse UV U"
        type    toggle
        default { "0" }
    }
    parm {
        name    "reverseUVv"
        cppname "ReverseUVv"
        label   "Reverse UV V"
        type    toggle
        default { "0" }
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
SOP_FeE_UVGridify_1_0::buildTemplates()
{
    static PRM_TemplateBuilder templ("SOP_FeE_UVGridify_1_0.C"_sh, theDsFile);
    if (templ.justBuilt())
    {
        //templ.setChoiceListPtr("group"_sh, &SOP_Node::groupMenu);
        //templ.setChoiceListPtr("posAttribName"_sh, &SOP_Node::allTextureCoordMenu);
        
    }
    return templ.templates();
}

const UT_StringHolder SOP_FeE_UVGridify_1_0::theSOPTypeName("FeE::uvGridify::1.0"_sh);

void
newSopOperator(OP_OperatorTable* table)
{
    OP_Operator* newOp = new OP_Operator(
        SOP_FeE_UVGridify_1_0::theSOPTypeName,
        "FeE UV Gridify",
        SOP_FeE_UVGridify_1_0::myConstructor,
        SOP_FeE_UVGridify_1_0::buildTemplates(),
        1,
        1,
        nullptr,
        OP_FLAG_GENERATOR,
        nullptr,
        1,
        "Five elements Elf/UV");

    newOp->setIconName("SOP_texture");
    table->addOperator(newOp);

}





class SOP_FeE_UVGridify_1_0Verb : public SOP_NodeVerb
{
public:
    SOP_FeE_UVGridify_1_0Verb() {}
    virtual ~SOP_FeE_UVGridify_1_0Verb() {}

    virtual SOP_NodeParms *allocParms() const { return new SOP_FeE_UVGridify_1_0Parms(); }
    virtual UT_StringHolder name() const { return SOP_FeE_UVGridify_1_0::theSOPTypeName; }

    virtual CookMode cookMode(const SOP_NodeParms *parms) const { return COOK_INPLACE; }

    virtual void cook(const CookParms &cookparms) const;
    
    /// This static data member automatically registers
    /// this verb class at library load time.
    static const SOP_NodeVerb::Register<SOP_FeE_UVGridify_1_0Verb> theVerb;
};

// The static member variable definition has to be outside the class definition.
// The declaration is inside the class.
const SOP_NodeVerb::Register<SOP_FeE_UVGridify_1_0Verb> SOP_FeE_UVGridify_1_0Verb::theVerb;

const SOP_NodeVerb *
SOP_FeE_UVGridify_1_0::cookVerb() const 
{ 
    return SOP_FeE_UVGridify_1_0Verb::theVerb.get();
}




static GA_AttributeOwner
sopAttribOwner(const SOP_FeE_UVGridify_1_0Parms::UVAttribClass parmAttribClass)
{
    using namespace SOP_FeE_UVGridify_1_0Enums;
    switch (parmAttribClass)
    {
    case UVAttribClass::AUTO:      return GA_ATTRIB_INVALID;    break;//not detail but means Auto
    case UVAttribClass::POINT:     return GA_ATTRIB_POINT;      break;
    case UVAttribClass::VERTEX:    return GA_ATTRIB_VERTEX;     break;
    }
    UT_ASSERT_MSG(0, "Unhandled Geo0 Class type!");
    return GA_ATTRIB_INVALID;
}


static _gfel::UVGridify::RowColMethod
sopRowsOrColsNumMethod(const SOP_FeE_UVGridify_1_0Parms::RowsOrColsNumMethod parmRowsOrColsNumMethod)
{
    using namespace SOP_FeE_UVGridify_1_0Enums;
    switch (parmRowsOrColsNumMethod)
    {
    case RowsOrColsNumMethod::UNIFORM:     return _gfel::UVGridify::RowColMethod::Uniform;    break;
    case RowsOrColsNumMethod::ROWS:        return _gfel::UVGridify::RowColMethod::Rows;       break;
    case RowsOrColsNumMethod::COLS:        return _gfel::UVGridify::RowColMethod::Columns;    break;
    }
    UT_ASSERT_MSG(0, "Unhandled UVGridify Rows Or Cols Num Method!");
    return _gfel::UVGridify::RowColMethod::Uniform;
}


static GA_GroupType
sopGroupType(const SOP_FeE_UVGridify_1_0Parms::GroupType parmGroupType)
{
    using namespace SOP_FeE_UVGridify_1_0Enums;
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
SOP_FeE_UVGridify_1_0Verb::cook(const SOP_NodeVerb::CookParms &cookparms) const
{
    auto&& sopparms = cookparms.parms<SOP_FeE_UVGridify_1_0Parms>();
    GA_Detail& outGeo0 = *cookparms.gdh().gdpNC();
    //auto sopcache = (SOP_FeE_UVGridify_1_0Cache*)cookparms.cache();

    //const GA_Detail& inGeo0 = *cookparms.inputGeo(0);

    //outGeo0.replaceWith(inGeo0);


    const GA_GroupType groupType = sopGroupType(sopparms.getGroupType());

    const GA_AttributeOwner uvAttribClass = sopAttribOwner(sopparms.getUVAttribClass());
    const _gfel::UVGridify::RowColMethod rowsOrColsNumMethod = sopRowsOrColsNumMethod(sopparms.getRowsOrColsNumMethod());
    

    UT_AutoInterrupt boss("Processing");
    if (boss.wasInterrupted())
        return;
    

    _gfel::UVGridify uvGridify(outGeo0, cookparms);
    uvGridify.groupParser.setGroup(groupType, sopparms.getGroup());
    uvGridify.getOutAttribArray().findOrCreateUV(false, uvAttribClass, GA_STORE_INVALID, sopparms.getUVAttrib());
    uvGridify.setComputeParm(
        rowsOrColsNumMethod, sopparms.getRowsOrColsNum(),
        sopparms.getReverseUVu(), sopparms.getReverseUVv(), sopparms.getUniScale(),
        sopparms.getSubscribeRatio(), sopparms.getMinGrainSize());
    uvGridify.computeAndBumpDataId();

    
}

