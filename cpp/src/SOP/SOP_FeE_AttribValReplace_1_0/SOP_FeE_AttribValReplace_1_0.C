
//#define UT_ASSERT_LEVEL 3
#include "SOP_FeE_AttribValReplace_1_0.h"


#include "SOP_FeE_AttribValReplace_1_0.proto.h"

#include <GA/GA_Detail.h>
#include <PRM/PRM_TemplateBuilder.h>
#include <UT/UT_Interrupt.h>
#include <UT/UT_DSOVersion.h>




#include <GFE/GFE_AttributeValueReplace.h>




using namespace SOP_FeE_AttribValReplace_1_0_Namespace;


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
        name    "attribClass"
        cppname "AttribClass"
        label   "Attrib Class"
        type    ordinal
        default { "point" }
        menu {
            "prim"      "Primitive"
            "point"     "Point"
            "vertex"    "Vertex"
            "detail"    "Detail"
        }
    }
    parm {
        name    "attrib"
        cppname "Attrib"
        label   "Attrib"
        type    string
        default { "" }
    }
    parm {
        name    "outNewAttrib"
        cppname "OutNewAttrib"
        label   "Out New Attrib"
        type    toggle
        default { 0 }
        nolabel
        joinnext
    }
    parm {
        name    "newAttribName"
        cppname "NewAttribName"
        label   "New Attrib Name"
        type    string
        default { "" }
        disablewhen "{ outNewAttrib == 0 }"
    }
    parm {
        name    "attribType"
        cppname "AttribType"
        label   "Attrib Type"
        type    ordinal
        default { "string" }
        menu {
            "bool"      "Bool"
            "int"       "Int"
            "float"     "Float"
            "string"    "String"
            "vector2"   "Vector2"
            "vector"    "Vector"
            "vector4"   "Vector4"
            "matrix2"   "Matrix2"
            "matrix"    "Matrix"
            "matrix4"   "Matrix4"
        }
        disablewhen "{ outNewAttrib == 0 }"
    }
    parm {
        name    "precision"
        cppname "Precision"
        label   "Precision"
        type    ordinal
        default { "string" }
        menu {
            "auto"    "Auto"
            "1"       "1"
            "8"       "8"
            "16"      "16"
            "32"      "32"
            "64"      "64"
        }
        disablewhen "{ outNewAttrib == 0 }"
    }


    parm {
        name    "delInAttrib"
        cppname "DelInAttrib"
        label   "Del In Attrib"
        type    toggle
        default { 0 }
    }


    multiparm {
        name    "numEntries"
        cppname "NumEntries"
        label   "Number of Entries"
        default 1
        parmtag { "multistartoffset" "0" }
    
        parm {
            name    "enable#"
            label   "Enable"
            type    toggle
            nolabel
            joinnext
            default { "1" }
        }
        parm {
            name    "key#"
            label   "Key"
            type    string
            default { "" }
            disablewhen "{ enable# == 0 }"
        }
    
        parm {
            name    "vals#"
            label   "Value"
            type    string
            default { "" }
            disablewhen "{ enable# == 0 }"
        }
        parm {
            name    "vali#"
            label   "Value"
            type    integer
            default { "0" }
            disablewhen "{ enable# == 0 }"
            range   { 0 10 }
        }
        parm {
            name    "valf#"
            label   "Value"
            type    float
            default { "0" }
            disablewhen "{ enable# == 0 }"
            range   { 0 10 }
        }
        parm {
            name    "val2f#"
            label   "Value"
            type    vector2
            size    2
            default { "0" "0" }
            disablewhen "{ enable# == 0 }"
            range   { -1 1 }
        }
        parm {
            name    "valv#"
            label   "Value"
            type    vector
            size    3
            default { "0" "0" "0" }
            disablewhen "{ enable# == 0 }"
            range   { -1 1 }
        }
        parm {
            name    "val4f#"
            label   "Value"
            type    vector4
            size    4
            default { "0" "0" "0" "0" }
            disablewhen "{ enable# == 0 }"
            range   { -1 1 }
        }
    }

    //multiparm {
    //    name    "numEntries"
    //    cppname "NumEntries"
    //    label   "Number of Entries"
    //    default 1
    //    parmtag { "multistartoffset" "0" }
    //
    //    parm {
    //        name    "enable#"
    //        label   "Enable"
    //        type    toggle
    //        nolabel
    //        joinnext
    //        default { "1" }
    //    }
    //    parm {
    //        name    "key#"
    //        label   "Key"
    //        type    string
    //        default { "" }
    //        disablewhen "{ enable# == 0 }"
    //    }
    //
    //    parm {
    //        name    "vals#"
    //        label   "Value"
    //        type    string
    //        default { "" }
    //        disablewhen "{ attribType != string }" "{ enable# == 0 }"
    //        hidewhen "{ attribType != string }"
    //    }
    //    parm {
    //        name    "vali#"
    //        label   "Value"
    //        type    integer
    //        default { "0" }
    //        disablewhen "{ attribType != int } { enable# == 0 }"
    //        hidewhen "{ attribType != int }"
    //        range   { 0 10 }
    //    }
    //    parm {
    //        name    "valf#"
    //        label   "Value"
    //        type    float
    //        default { "0" }
    //        disablewhen "{ attribType != float } { enable# == 0 }"
    //        hidewhen "{ attribType != float }"
    //        range   { 0 10 }
    //    }
    //    parm {
    //        name    "val2f#"
    //        label   "Value"
    //        type    vector2
    //        size    2
    //        default { "0" "0" }
    //        disablewhen "{ attribType != vector2 } { enable# == 0 }"
    //        hidewhen "{ attribType != vector2 }"
    //        range   { -1 1 }
    //    }
    //    parm {
    //        name    "valv#"
    //        label   "Value"
    //        type    vector
    //        size    3
    //        default { "0" "0" "0" }
    //        disablewhen "{ attribType != vector } { enable# == 0 }"
    //        hidewhen "{ attribType != vector }"
    //        range   { -1 1 }
    //    }
    //    parm {
    //        name    "val4f#"
    //        label   "Value"
    //        type    vector4
    //        size    4
    //        default { "0" "0" "0" "0" }
    //        disablewhen "{ attribType != vector4 } { enable# == 0 }"
    //        hidewhen "{ attribType != vector4 }"
    //        range   { -1 1 }
    //    }
    //}
    


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
SOP_FeE_AttribValReplace_1_0::buildTemplates()
{
    static PRM_TemplateBuilder templ("SOP_FeE_AttribValReplace_1_0.C"_sh, theDsFile);
    if (templ.justBuilt())
    {
        templ.setChoiceListPtr("attrib"_sh, &SOP_Node::allAttribReplaceMenu);
    }
    return templ.templates();
}

const UT_StringHolder SOP_FeE_AttribValReplace_1_0::theSOPTypeName("FeE::attribValReplace::1.0"_sh);

void
newSopOperator(OP_OperatorTable* table)
{
    OP_Operator* newOp = new OP_Operator(
        SOP_FeE_AttribValReplace_1_0::theSOPTypeName,
        "FeE Attribute Value Replace",
        SOP_FeE_AttribValReplace_1_0::myConstructor,
        SOP_FeE_AttribValReplace_1_0::buildTemplates(),
        1,
        1,
        nullptr,
        OP_FLAG_GENERATOR,
        nullptr,
        1,
        "Five elements Elf/Attribute");

    newOp->setIconName("SOP_attribadjustdict");
    table->addOperator(newOp);

}





class SOP_FeE_AttribValReplace_1_0Verb : public SOP_NodeVerb
{
public:
    SOP_FeE_AttribValReplace_1_0Verb() {}
    virtual ~SOP_FeE_AttribValReplace_1_0Verb() {}

    virtual SOP_NodeParms *allocParms() const { return new SOP_FeE_AttribValReplace_1_0Parms(); }
    virtual UT_StringHolder name() const { return SOP_FeE_AttribValReplace_1_0::theSOPTypeName; }

    virtual CookMode cookMode(const SOP_NodeParms *parms) const { return COOK_INPLACE; }

    virtual void cook(const CookParms &cookparms) const;
    
    /// This static data member automatically registers
    /// this verb class at library load time.
    static const SOP_NodeVerb::Register<SOP_FeE_AttribValReplace_1_0Verb> theVerb;
};

// The static member variable definition has to be outside the class definition.
// The declaration is inside the class.
const SOP_NodeVerb::Register<SOP_FeE_AttribValReplace_1_0Verb> SOP_FeE_AttribValReplace_1_0Verb::theVerb;

const SOP_NodeVerb *
SOP_FeE_AttribValReplace_1_0::cookVerb() const 
{ 
    return SOP_FeE_AttribValReplace_1_0Verb::theVerb.get();
}





static GA_GroupType
sopGroupType(const SOP_FeE_AttribValReplace_1_0Parms::GroupType parmGroupType)
{
    using namespace SOP_FeE_AttribValReplace_1_0Enums;
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

static GA_AttributeOwner
sopAttribOwner(const SOP_FeE_AttribValReplace_1_0Parms::AttribClass parmAttribClass)
{
    using namespace SOP_FeE_AttribValReplace_1_0Enums;
    switch (parmAttribClass)
    {
    case AttribClass::PRIM:      return GA_ATTRIB_PRIMITIVE;  break;
    case AttribClass::POINT:     return GA_ATTRIB_POINT;      break;
    case AttribClass::VERTEX:    return GA_ATTRIB_VERTEX;     break;
    case AttribClass::DETAIL:    return GA_ATTRIB_DETAIL;    break;
    }
    UT_ASSERT_MSG(0, "Unhandled Class type!");
    return GA_ATTRIB_INVALID;
}



static GA_StorageClass
sopStorageClass(const SOP_FeE_AttribValReplace_1_0Parms::AttribType parmAttribType)
{
    using namespace SOP_FeE_AttribValReplace_1_0Enums;
    switch (parmAttribType)
    {
    case AttribType::BOOL:   return GA_STORECLASS_OTHER;  break;
    case AttribType::INT:    return GA_STORECLASS_INT;    break;
    case AttribType::FLOAT:  return GA_STORECLASS_FLOAT;  break;
    case AttribType::STRING: return GA_STORECLASS_STRING; break;
    }
    UT_ASSERT_MSG(0, "Unhandled Geo0 Attrib Type!");
    return GA_STORECLASS_INVALID;
}


static GA_Precision
sopPrecision(const SOP_FeE_AttribValReplace_1_0Parms::Precision parmPrecision)
{
    using namespace SOP_FeE_AttribValReplace_1_0Enums;
    switch (parmPrecision)
    {
    case Precision::AUTO: return GA_PRECISION_INVALID; break;
    case Precision::_1:   return GA_PRECISION_1;       break;
    case Precision::_8:   return GA_PRECISION_8;       break;
    case Precision::_16:  return GA_PRECISION_16;      break;
    case Precision::_32:  return GA_PRECISION_32;      break;
    case Precision::_64:  return GA_PRECISION_64;      break;
    }
    UT_ASSERT_MSG(0, "Unhandled Precision!");
    return GA_PRECISION_INVALID;
}



void
SOP_FeE_AttribValReplace_1_0Verb::cook(const SOP_NodeVerb::CookParms &cookparms) const
{
    auto&& sopparms = cookparms.parms<SOP_FeE_AttribValReplace_1_0Parms>();
    GA_Detail& outGeo0 = *cookparms.gdh().gdpNC();
    //auto sopcache = (SOP_FeE_AttribValReplace_1_0Cache*)cookparms.cache();

    //const GA_Detail& inGeo0 = *cookparms.inputGeo(0);

    //outGeo0.replaceWith(inGeo0);



    const GA_AttributeOwner attribClass = sopAttribOwner(sopparms.getAttribClass());
    const GA_GroupType groupType = sopGroupType(sopparms.getGroupType());


    UT_AutoInterrupt boss("Processing");
    if (boss.wasInterrupted())
        return;


    GFE_AttribValReplace attribValReplace(outGeo0, cookparms);
    
    if (sopparms.getOutNewAttrib())
        attribValReplace.getInAttribArray().appends(attribClass, sopparms.getAttrib());
    else
        attribValReplace.getOutAttribArray().appends(attribClass, sopparms.getAttrib());

    attribValReplace.newStorageClass = sopStorageClass(sopparms.getAttribType());
    attribValReplace.newPrecision    = sopPrecision(sopparms.getPrecision());
    
    attribValReplace.setComputeParm(sopparms.getNumEntries(), sopparms.getOutNewAttrib(), sopparms.getNewAttribName(), sopparms.getDelInAttrib(), 
        sopparms.getSubscribeRatio(), sopparms.getMinGrainSize());

    attribValReplace.groupParser.setGroup(groupType, sopparms.getGroup());
    attribValReplace.computeAndBumpDataId();

    
}


