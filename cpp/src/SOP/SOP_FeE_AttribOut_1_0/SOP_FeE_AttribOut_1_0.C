//#define UT_ASSERT_LEVEL 3
#include "SOP_FeE_AttribOut_1_0.h"

#include "SOP_FeE_AttribOut_1_0.proto.h"

#include <GA/GA_Detail.h>
#include <PRM/PRM_TemplateBuilder.h>
#include <UT/UT_Interrupt.h>
#include <UT/UT_DSOVersion.h>


#include <GFE/GFE_Detail.h>


using namespace SOP_FeE_AttribOut_1_0_Namespace;


static const char* theDsFile = R"THEDSFILE(
{
    name        parameters

    multiparm {
        name    "numAttribOut"
        cppname "NumAttribOut"
        label   "Number of Attrib Out"
        default 0
        parmtag { "multistartoffset" "0" }

        parm {
            name    "outAttrib#"
            cppname "OutAttrib#"
            label   "Out Attrib"
            type    toggle
            default { 1 }
            nolabel
            joinnext
        }
        parm {
            name    "attribClass#"
            cppname "AttribClass#"
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
            name    "attrib#"
            cppname "Attrib#"
            label   "Attrib"
            type    string
            default { "" }
            disablewhen "{ outAttrib# == 0 }"
        }
        parm {
            name    "newAttribName#"
            cppname "NewAttribName#"
            label   "New Attrib Name"
            type    string
            default { "" }
            disablewhen "{ outAttrib# == 1 }"
        }
    }

    multiparm {
        name    "numGroupOut"
        cppname "NumGroupOut"
        label   "Number of Group Out"
        default 0
        parmtag { "multistartoffset" "0" }

        parm {
            name    "outGroup#"
            cppname "OutGroup#"
            label   "Out Group"
            type    toggle
            default { 1 }
            nolabel
            joinnext
        }
        parm {
            name    "groupType#"
            cppname "GroupType#"
            label   "Group Type"
            type    ordinal
            default { "point" }
            menu {
                "prim"      "Primitive"
                "point"     "Point"
                "vertex"    "Vertex"
                "edge"      "Edge"
            }
        }
        parm {
            name    "group#"
            cppname "Group#"
            label   "Group#"
            type    string
            default { "" }
            disablewhen "{ outGroup# == 0 }"
        }
        parm {
            name    "newGroupName#"
            cppname "NewGroupName#"
            label   "New Group Name"
            type    string
            default { "" }
            disablewhen "{ outGroup# == 1 }"
        }
    }



}
)THEDSFILE";


PRM_Template*
SOP_FeE_AttribOut_1_0::buildTemplates()
{
    static PRM_TemplateBuilder templ("SOP_FeE_AttribOut_1_0.C"_sh, theDsFile);
    if (templ.justBuilt())
    {
        templ.setChoiceListPtr("attrib#"_sh,    &SOP_Node::allAttribMenu);
        templ.setChoiceListPtr("group#"_sh,     &SOP_Node::groupNameMenu);
    }
    return templ.templates();
}

const UT_StringHolder SOP_FeE_AttribOut_1_0::theSOPTypeName("FeE::attribOut::1.0"_sh);

void
newSopOperator(OP_OperatorTable* table)
{
    OP_Operator* newOp = new OP_Operator(
        SOP_FeE_AttribOut_1_0::theSOPTypeName,
        "FeE Attribute Output",
        SOP_FeE_AttribOut_1_0::myConstructor,
        SOP_FeE_AttribOut_1_0::buildTemplates(),
        1,
        1,
        nullptr,
        OP_FLAG_GENERATOR,
        nullptr,
        1,
        "Five elements Elf/Attribute");

    newOp->setIconName("SOP_attribute");
    table->addOperator(newOp);
}

//class SOP_FeE_AttribOut_1_0Cache : public SOP_NodeCache
//{
//public:
//    SOP_FeE_AttribOut_1_0Cache() : SOP_NodeCache(),
//        myPrevOutputDetailID(-1)
//    {}
//    ~SOP_FeE_AttribOut_1_0Cache() override {}
//
//    int64 myPrevOutputDetailID;
//};


class SOP_FeE_AttribOut_1_0Verb : public SOP_NodeVerb
{
public:
    SOP_FeE_AttribOut_1_0Verb()
    {
    }

    virtual ~SOP_FeE_AttribOut_1_0Verb()
    {
    }

    virtual SOP_NodeParms* allocParms() const { return new SOP_FeE_AttribOut_1_0Parms(); }
    //virtual SOP_NodeCache* allocCache() const { return new SOP_FeE_AttribOut_1_0Cache(); }
    virtual UT_StringHolder name() const { return SOP_FeE_AttribOut_1_0::theSOPTypeName; }

    virtual CookMode cookMode(const SOP_NodeParms* parms) const { return COOK_GENERIC; }

    virtual void cook(const CookParms& cookparms) const;

    /// This static data member automatically registers
    /// this verb class at library ldir0d time.
    static const SOP_NodeVerb::Register<SOP_FeE_AttribOut_1_0Verb> theVerb;
};

// The static member variable definition has to be outside the class definition.
// The declaration is inside the class.
const SOP_NodeVerb::Register<SOP_FeE_AttribOut_1_0Verb> SOP_FeE_AttribOut_1_0Verb::theVerb;

const SOP_NodeVerb*
SOP_FeE_AttribOut_1_0::cookVerb() const
{
    return SOP_FeE_AttribOut_1_0Verb::theVerb.get();
}



#if 1


static GA_AttributeOwner
sopAttribOwner(const int64 attribClass)
{
    using namespace SOP_FeE_AttribOut_1_0Enums;
    switch (attribClass)
    {
    default:
    case 0:      return GA_ATTRIB_PRIMITIVE;  break;
    case 1:      return GA_ATTRIB_POINT;      break;
    case 2:      return GA_ATTRIB_VERTEX;     break;
    case 3:      return GA_ATTRIB_DETAIL;     break;
    }
    UT_ASSERT_MSG(0, "Unhandled Geo0 Class type!");
    return GA_ATTRIB_INVALID;
}

static GA_GroupType
sopGroupType(const int64 parmGroupType)
{
    using namespace SOP_FeE_AttribOut_1_0Enums;
    switch (parmGroupType)
    {
    default:
    case 0:      return GA_GROUP_PRIMITIVE;  break;
    case 1:      return GA_GROUP_POINT;      break;
    case 2:      return GA_GROUP_VERTEX;     break;
    case 3:      return GA_GROUP_EDGE;       break;
    }
    UT_ASSERT_MSG(0, "Unhandled geo0Group type!");
    return GA_GROUP_INVALID;
}


#else



static GA_AttributeOwner
sopAttribOwner(SOP_FeE_AttribOut_1_0Parms::AttribClass attribClass)
{
    using namespace SOP_FeE_AttribOut_1_0Enums;
    switch (attribClass)
    {
    default:
    case AttribClass::PRIM:      return GA_ATTRIB_PRIMITIVE;  break;
    case AttribClass::POINT:     return GA_ATTRIB_POINT;      break;
    case AttribClass::VERTEX:    return GA_ATTRIB_VERTEX;     break;
    case AttribClass::DETAIL:    return GA_ATTRIB_DETAIL;     break;
    }
    UT_ASSERT_MSG(0, "Unhandled Geo0 Class type!");
    return GA_ATTRIB_INVALID;
}

static GA_GroupType
sopGroupType(SOP_FeE_AttribOut_1_0Parms::GroupClass parmGroupClass)
{
    using namespace SOP_FeE_AttribOut_1_0Enums;
    switch (parmGroupClass)
    {
    default:
    case GroupClass::PRIM:      return GA_GROUP_PRIMITIVE;  break;
    case GroupClass::POINT:     return GA_GROUP_POINT;      break;
    case GroupClass::VERTEX:    return GA_GROUP_VERTEX;     break;
    case GroupClass::EDGE:      return GA_GROUP_EDGE;       break;
    }
    UT_ASSERT_MSG(0, "Unhandled geo0Group type!");
    return GA_GROUP_INVALID;
}

#endif

    

void
SOP_FeE_AttribOut_1_0Verb::cook(const SOP_NodeVerb::CookParms& cookparms) const
{
    auto&& sopparms = cookparms.parms<SOP_FeE_AttribOut_1_0Parms>();
    GA_Detail& outGeo0 = *cookparms.gdh().gdpNC();
    //auto sopcache = (SOP_FeE_AttribOut_1_0Cache*)cookparms.cache();

    const GA_Detail& inGeo0 = *cookparms.inputGeo(0);

    outGeo0.replaceWith(inGeo0);


    UT_AutoInterrupt boss("Processing");
    if (boss.wasInterrupted())
        return;


    //GFE_Detail& outGFE0 = static_cast<GFE_Detail&>(outGeo0);

    const UT_Array<SOP_FeE_AttribOut_1_0Parms::NumAttribOut>& numAttribOut = sopparms.getNumAttribOut();
    const size_t size = numAttribOut.size();
    for (size_t i = 0; i < size; i++)
    {
        const SOP_FeE_AttribOut_1_0Parms::NumAttribOut& attribOut = numAttribOut[i];

        const GA_AttributeOwner attribClass = sopAttribOwner(attribOut.attribClass);
        if (attribOut.outAttrib)
        {
            outGeo0.renameAttribute(attribClass, GA_SCOPE_PUBLIC, attribOut.attrib, attribOut.newAttribName);
        }
        else
        {
            outGeo0.destroyAttribute(attribClass, attribOut.attrib);
        }
    }

    bool doVisualize = true;
    const UT_Array<SOP_FeE_AttribOut_1_0Parms::NumGroupOut>& numGroupOut = sopparms.getNumGroupOut();
    const size_t groupSize = numGroupOut.size();
    for (size_t i = 0; i < groupSize; i++)
    {
        const SOP_FeE_AttribOut_1_0Parms::NumGroupOut& groupOut = numGroupOut[i];

        const GA_GroupType groupClass = sopGroupType(groupOut.groupType);
        if (groupOut.outGroup)
        {
            outGeo0.getGroupTable(groupClass)->renameGroup(groupOut.group, groupOut.newGroupName);
            if (doVisualize)
            {
                const GA_Group* const outGroup = outGeo0.getGroupTable(groupClass)->find(groupOut.newGroupName);
                cookparms.select(*outGroup);
                doVisualize = false;
            }
        }
        else
        {
            outGeo0.getGroupTable(groupClass)->destroy(groupOut.group);
        }
    }

}
