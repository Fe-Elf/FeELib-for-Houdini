
//#define UT_ASSERT_LEVEL 3
#include "SOP_FeE_AttribCopy_4_0.h"


#include "SOP_FeE_AttribCopy_4_0.proto.h"

#include "GA/GA_Detail.h"
#include "PRM/PRM_TemplateBuilder.h"
#include "UT/UT_Interrupt.h"
#include "UT/UT_DSOVersion.h"




#include "GFE/GFE_AttributeCopy.h"



using namespace SOP_FeE_AttribCopy_4_0_Namespace;



static const char *SOP_FeE_AttribCopy_4_0_theDsFile = R"THEDSFILE(
{
    name        parameters

    parm {
        name    "srcGroup"
        cppname "SrcGroup"
        label   "Source Group"
        type    string
        default { "" }
        range   { 0 1 }
        parmtag { "script_action" "import soputils\nkwargs['geometrytype'] = kwargs['node'].parmTuple('sourceClass')\nkwargs['inputindex'] = 1\nsoputils.selectGroupParm(kwargs)" }
        parmtag { "script_action_help" "Select geometry from an available viewport." }
        parmtag { "script_action_icon" "BUTTONS_reselect" }
    }
    parm {
        name    "sourceClass"
        cppname "SourceClass"
        label   "Source Class"
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
        name    "group"
        cppname "Group"
        label   "Group"
        type    string
        default { "" }
        range   { 0 1 }
        parmtag { "script_action" "import soputils\nkwargs['geometrytype'] = kwargs['node'].parmTuple('destinationClass')\nkwargs['inputindex'] = 0\nsoputils.selectGroupParm(kwargs)" }
        parmtag { "script_action_help" "Select geometry from an available viewport." }
        parmtag { "script_action_icon" "BUTTONS_reselect" }
    }
    parm {
        name    "destinationClass"
        cppname "DestinationClass"
        label   "Destination Class"
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
        name    "useIDAttrib"
        cppname "UseIDAttrib"
        label   "Match by Attribute"
        type    toggle
        nolabel
        joinnext
        default { "0" }
    }
    parm {
        name    "iDAttrib"
        cppname "IDAttrib"
        label   "ID Attribute"
        type    string
        joinnext
        default { "id" }
        disablewhen "{ useIDAttrib == 0 }"
    }
    parm {
        name    "iDAttribInput"
        cppname "IDAttribInput"
        label   "ID Attrib Input"
        type    ordinal
        nolabel
        default { "destination" }
        menu {
            "source"        "Source"
            "destination"   "Destination"
        }
    }
    parm {
        name    "iDAttribIsOffset"
        cppname "IDAttribIsOffset"
        label   "ID Attrib is Offset"
        type    toggle
        default { "0" }
        disablewhen "{ useIDAttrib == 0 }"
    }
    parm {
        name    "copyFromSingleOff"
        cppname "CopyFromSingleOff"
        label   "Copy From Single Off"
        type    integer
        default { -1 }
        range   { -1 10 }
        disablewhen "{ useIDAttrib == 1 }"
    }


    groupsimple {
        name    "copyAttrib_folder"
        label   "Copy Attrib"

        parm {
            name    "copyAttrib"
            cppname "CopyAttrib"
            label   "Copy Attrib"
            type    string
            default { "" }
        }
        parm {
            name    "renameAttrib"
            cppname "RenameAttrib"
            label   "Rename Attrib"
            type    toggle
            nolabel
            joinnext
            default { "0" }
        }
        parm {
            name    "newAttribName"
            cppname "NewAttribName"
            label   "New Attrib Name"
            type    string
            default { "" }
            disablewhen "{ renameAttrib == 0 }"
        }
        parm {
            name    "attribMergeType"
            cppname "AttribMergeType"
            label   "Attrib Merge Type"
            type    ordinal
            default { "replace" }
            menu {
                "set"       "Set"
                "add"       "Add"
                "sub"       "Subtract"
                "mult"      "Multiply"
                "min"       "Min"
                "max"       "Max"
                "xor"       "Xor"
                "toggle"    "Toggle"
                "append"    "Append"
                "intersect" "Intersect"
            }
        }
    }
    
    groupsimple {
        name    "copyGroup_folder"
        label   "Copy Group"

        parm {
            name    "copyGroup"
            cppname "CopyGroup"
            label   "Copy Group"
            type    string
            default { "" }
        }
        parm {
            name    "renameGroup"
            cppname "RenameGroup"
            label   "Rename Group"
            type    toggle
            nolabel
            joinnext
            default { "0" }
        }
        parm {
            name    "newGroupName"
            cppname "NewGroupName"
            label   "New Group Name"
            type    string
            default { "" }
            disablewhen "{ renameGroup == 0 }"
        }
        parm {
            name    "groupMergeType"
            cppname "GroupMergeType"
            label   "Group Merge Type"
            type    ordinal
            default { "replace" }
            menu {
                "replace"   "Replace Existing"
                "union"     "Union with Existing"
                "intersect" "Intersect with Existing"
                "subtract"  "Subtract from Existing"
                "toogle"    "Toogle"
            }
        }
    }


    parm {
        name    "subscribeRatio"
        cppname "SubscribeRatio"
        label   "Subscribe Ratio"
        type    integer
        default { 8 }
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
SOP_FeE_AttribCopy_4_0::buildTemplates()
{
    static PRM_TemplateBuilder templ("SOP_FeE_AttribCopy_4_0.C"_sh, SOP_FeE_AttribCopy_4_0_theDsFile);
    if (templ.justBuilt())
    {
        templ.setChoiceListPtr("group"_sh,          &SOP_Node::allGroupMenu);
        templ.setChoiceListPtr("srcGroup"_sh,       &SOP_Node::allGroupMenu);
        templ.setChoiceListPtr("iDAttrib"_sh,   &SOP_Node::allAttribMenu);
        templ.setChoiceListPtr("copyAttrib"_sh, &SOP_Node::allAttribMenu);
        templ.setChoiceListPtr("copyGroup"_sh,  &SOP_Node::allGroupMenu);
    }
    return templ.templates();
}


const UT_StringHolder SOP_FeE_AttribCopy_4_0::theSOPTypeName("FeE::attribCopy::4.0"_sh);

void
newSopOperator(OP_OperatorTable* table)
{
    OP_Operator* newOp = new OP_Operator(
        SOP_FeE_AttribCopy_4_0::theSOPTypeName,
        "FeE Attribute Copy",
        SOP_FeE_AttribCopy_4_0::myConstructor,
        SOP_FeE_AttribCopy_4_0::buildTemplates(),
        2,
        2,
        nullptr,
        0,
        nullptr,
        1,
        "Five elements Elf/Attribute");

    newOp->setIconName("SOP_attribcopy");
    table->addOperator(newOp);

}


//class SOP_FeE_AttribCopy_4_0Cache : public SOP_NodeCache
//{
//public:
//    SOP_FeE_AttribCopy_4_0Cache() : SOP_NodeCache(),
//        myPrevOutputDetailID(-1)
//    {}
//    ~SOP_FeE_AttribCopy_4_0Cache() override {}
//
//    int64 myPrevOutputDetailID;
//};


class SOP_FeE_AttribCopy_4_0Verb : public SOP_NodeVerb
{
public:
    SOP_FeE_AttribCopy_4_0Verb() {}
    virtual ~SOP_FeE_AttribCopy_4_0Verb() {}

    virtual SOP_NodeParms *allocParms() const { return new SOP_FeE_AttribCopy_4_0Parms(); }
    //virtual SOP_NodeCache* allocCache() const { return new SOP_FeE_AttribCopy_4_0Cache(); }
    virtual UT_StringHolder name() const { return SOP_FeE_AttribCopy_4_0::theSOPTypeName; }

    virtual CookMode cookMode(const SOP_NodeParms *parms) const { return COOK_INPLACE; }

    virtual void cook(const CookParms &cookparms) const;

    static const SOP_NodeVerb::Register<SOP_FeE_AttribCopy_4_0Verb> theVerb;
};

const SOP_NodeVerb::Register<SOP_FeE_AttribCopy_4_0Verb> SOP_FeE_AttribCopy_4_0Verb::theVerb;

const SOP_NodeVerb *
SOP_FeE_AttribCopy_4_0::cookVerb() const 
{ 
    return SOP_FeE_AttribCopy_4_0Verb::theVerb.get();
}



static GA_AttributeOwner
sopAttribOwner(const SOP_FeE_AttribCopy_4_0Parms::DestinationClass parmAttribClass)
{
    using namespace SOP_FeE_AttribCopy_4_0Enums;
    switch (parmAttribClass)
    {
    case DestinationClass::PRIM:      return GA_ATTRIB_PRIMITIVE;  break;
    case DestinationClass::POINT:     return GA_ATTRIB_POINT;      break;
    case DestinationClass::VERTEX:    return GA_ATTRIB_VERTEX;     break;
    case DestinationClass::DETAIL:    return GA_ATTRIB_DETAIL;     break;
    }
    UT_ASSERT_MSG(0, "Unhandled Geo0 Class type!");
    return GA_ATTRIB_INVALID;
}

static GA_AttributeOwner
sopAttribOwner(const SOP_FeE_AttribCopy_4_0Parms::SourceClass parmAttribClass)
{
    using namespace SOP_FeE_AttribCopy_4_0Enums;
    switch (parmAttribClass)
    {
    case SourceClass::PRIM:      return GA_ATTRIB_PRIMITIVE;  break;
    case SourceClass::POINT:     return GA_ATTRIB_POINT;      break;
    case SourceClass::VERTEX:    return GA_ATTRIB_VERTEX;     break;
    case SourceClass::DETAIL:    return GA_ATTRIB_DETAIL;     break;
    }
    UT_ASSERT_MSG(0, "Unhandled Geo1 Class type!");
    return GA_ATTRIB_INVALID;
}

static bool
sopIDAttribInput(const SOP_FeE_AttribCopy_4_0Parms::IDAttribInput parmIDAttribInput)
{
    using namespace SOP_FeE_AttribCopy_4_0Enums;
    switch (parmIDAttribInput)
    {
    case IDAttribInput::SOURCE:          return false;     break;
    case IDAttribInput::DESTINATION:     return true;      break;
    }
    UT_ASSERT_MSG(0, "Unhandled Class type!");
    return false;
}

static GFE_AttribMergeType
sopAttribMergeType(const SOP_FeE_AttribCopy_4_0Parms::AttribMergeType parmAttribMergeType)
{
    using namespace SOP_FeE_AttribCopy_4_0Enums;
    switch (parmAttribMergeType)
    {
    case AttribMergeType::SET:          return GFE_AttribMergeType::Set;        break;
    case AttribMergeType::ADD:          return GFE_AttribMergeType::Add;        break;
    case AttribMergeType::SUB:          return GFE_AttribMergeType::Sub;        break;
    case AttribMergeType::MULT:         return GFE_AttribMergeType::Mult;       break;
    case AttribMergeType::MIN:          return GFE_AttribMergeType::Min;        break;
    case AttribMergeType::MAX:          return GFE_AttribMergeType::Max;        break;
    case AttribMergeType::XOR:          return GFE_AttribMergeType::Xor;        break;
    case AttribMergeType::TOGGLE:       return GFE_AttribMergeType::Toggle;     break;
    case AttribMergeType::APPEND:       return GFE_AttribMergeType::Append;     break;
    case AttribMergeType::INTERSECT:    return GFE_AttribMergeType::Intersect;  break;
    }
    UT_ASSERT_MSG(0, "Unhandled Attrib Merge type!");
    return GFE_AttribMergeType::Set;
}


void
SOP_FeE_AttribCopy_4_0Verb::cook(const SOP_NodeVerb::CookParms &cookparms) const
{
    auto &&sopparms = cookparms.parms<SOP_FeE_AttribCopy_4_0Parms>();
    GA_Detail& outGeo0 = *cookparms.gdh().gdpNC();
    //auto sopcache = (SOP_FeE_AttribCopy_4_0Cache*)cookparms.cache();

    //const GA_Detail& inGeo0 = *cookparms.inputGeo(0);
    const GA_Detail& inGeo1 = *cookparms.inputGeo(1);

    //outGeo0.replaceWith(inGeo0);
    
    
    const GA_AttributeOwner sourceClass = sopAttribOwner(sopparms.getSourceClass());
    const GA_AttributeOwner destinationClass = sopAttribOwner(sopparms.getDestinationClass());

    const bool iDAttribInput = sopIDAttribInput(sopparms.getIDAttribInput());
    const GFE_AttribMergeType attribMergeType = sopAttribMergeType(sopparms.getAttribMergeType());
    
    UT_AutoInterrupt boss("Processing");
    if (boss.wasInterrupted())
        return;

    GFE_AttribCopy attribCopy(outGeo0, &inGeo1, &cookparms);

    attribCopy.ownerDst = destinationClass;
    attribCopy.ownerSrc = sourceClass;
    attribCopy.setComputeParm(attribMergeType, iDAttribInput, sopparms.getCopyFromSingleOff(),
        sopparms.getSubscribeRatio(), sopparms.getMinGrainSize());
    
    if (sopparms.getUseIDAttrib())
        attribCopy.setOffsetAttrib(sopparms.getIDAttrib(), sopparms.getIDAttribIsOffset());
    
    if (sopparms.getRenameAttrib())
        attribCopy.newAttribNames = sopparms.getNewAttribName();
    if (sopparms.getRenameGroup())
        attribCopy.newGroupNames  = sopparms.getNewGroupName();
    
    attribCopy.appendGroups(sopparms.getCopyGroup());
    attribCopy.appendAttribs(sopparms.getCopyAttrib());

    attribCopy.computeAndBumpDataId();
    attribCopy.visualizeOutGroup();
    
}

