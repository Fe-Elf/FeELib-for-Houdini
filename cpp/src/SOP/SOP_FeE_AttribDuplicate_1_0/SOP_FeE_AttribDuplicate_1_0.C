//#define UT_ASSERT_LEVEL 3
#include "SOP_FeE_AttribDuplicate_1_0.h"

#include "SOP_FeE_AttribDuplicate_1_0.proto.h"

#include "GA/GA_Detail.h"
#include "PRM/PRM_TemplateBuilder.h"
#include "UT/UT_Interrupt.h"
#include "UT/UT_DSOVersion.h"


#include "GFE/GFE_AttributeDuplicate.h"


using namespace SOP_FeE_AttribDuplicate_1_0_Namespace;


static const char* theDsFile = R"THEDSFILE(
{
    name        parameters
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
        name    "duplicateAttrib"
        cppname "DuplicateAttrib"
        label   "Duplicate Attrib"
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
        name    "groupName"
        cppname "GroupName"
        label   "Group Name"
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
SOP_FeE_AttribDuplicate_1_0::buildTemplates()
{
    static PRM_TemplateBuilder templ("SOP_FeE_AttribDuplicate_1_0.C"_sh, theDsFile);
    if (templ.justBuilt())
    {
        templ.setChoiceListPtr("duplicateAttrib"_sh, &SOP_Node::allAttribMenu);
        templ.setChoiceListPtr("groupName"_sh,       &SOP_Node::allGroupMenu);
    }
    return templ.templates();
}

const UT_StringHolder SOP_FeE_AttribDuplicate_1_0::theSOPTypeName("FeE::attribDuplicate::1.0"_sh);

void
newSopOperator(OP_OperatorTable* table)
{
    OP_Operator* newOp = new OP_Operator(
        SOP_FeE_AttribDuplicate_1_0::theSOPTypeName,
        "FeE Attribute Duplicate",
        SOP_FeE_AttribDuplicate_1_0::myConstructor,
        SOP_FeE_AttribDuplicate_1_0::buildTemplates(),
        1,
        2,
        nullptr,
        OP_FLAG_GENERATOR,
        nullptr,
        1,
        "Five elements Elf/Attribute");

    newOp->setIconName("SOP_attribute");
    table->addOperator(newOp);
}

//class SOP_FeE_AttribDuplicate_1_0Cache : public SOP_NodeCache
//{
//public:
//    SOP_FeE_AttribDuplicate_1_0Cache() : SOP_NodeCache(),
//        myPrevOutputDetailID(-1)
//    {}
//    ~SOP_FeE_AttribDuplicate_1_0Cache() override {}
//
//    int64 myPrevOutputDetailID;
//};


class SOP_FeE_AttribDuplicate_1_0Verb : public SOP_NodeVerb
{
public:
    SOP_FeE_AttribDuplicate_1_0Verb()
    {
    }

    virtual ~SOP_FeE_AttribDuplicate_1_0Verb()
    {
    }

    virtual SOP_NodeParms* allocParms() const { return new SOP_FeE_AttribDuplicate_1_0Parms(); }
    //virtual SOP_NodeCache* allocCache() const { return new SOP_FeE_AttribDuplicate_1_0Cache(); }
    virtual UT_StringHolder name() const { return SOP_FeE_AttribDuplicate_1_0::theSOPTypeName; }

    virtual CookMode cookMode(const SOP_NodeParms* parms) const { return COOK_INPLACE; }

    virtual void cook(const CookParms& cookparms) const;

    /// This static data member automatically registers
    /// this verb class at library ldir0d time.
    static const SOP_NodeVerb::Register<SOP_FeE_AttribDuplicate_1_0Verb> theVerb;
};

// The static member variable definition has to be outside the class definition.
// The declaration is inside the class.
const SOP_NodeVerb::Register<SOP_FeE_AttribDuplicate_1_0Verb> SOP_FeE_AttribDuplicate_1_0Verb::theVerb;

const SOP_NodeVerb*
SOP_FeE_AttribDuplicate_1_0::cookVerb() const
{
    return SOP_FeE_AttribDuplicate_1_0Verb::theVerb.get();
}



static GA_AttributeOwner
sopAttribOwner(const SOP_FeE_AttribDuplicate_1_0Parms::AttribClass parmAttribClass)
{
    using namespace SOP_FeE_AttribDuplicate_1_0Enums;
    switch (parmAttribClass)
    {
    case AttribClass::PRIM:   return GA_ATTRIB_PRIMITIVE; break;
    case AttribClass::POINT:  return GA_ATTRIB_POINT;     break;
    case AttribClass::VERTEX: return GA_ATTRIB_VERTEX;    break;
    case AttribClass::DETAIL: return GA_ATTRIB_DETAIL;    break;
    }
    UT_ASSERT_MSG(0, "Unhandled Geo0 Class type!");
    return GA_ATTRIB_INVALID;
}


void
SOP_FeE_AttribDuplicate_1_0Verb::cook(const SOP_NodeVerb::CookParms& cookparms) const
{
    auto&& sopparms = cookparms.parms<SOP_FeE_AttribDuplicate_1_0Parms>();
    GA_Detail& outGeo0 = *cookparms.gdh().gdpNC();
    //auto sopcache = (SOP_FeE_AttribDuplicate_1_0Cache*)cookparms.cache();

    //const GA_Detail& inGeo0 = *cookparms.inputGeo(0);
    const GA_Detail* const inGeo1 = cookparms.inputGeo(1);

    //outGeo0.replaceWith(inGeo0);

    
    const GA_AttributeOwner attribClass = sopAttribOwner(sopparms.getAttribClass());

    
    UT_AutoInterrupt boss("Processing");
    if (boss.wasInterrupted())
        return;
    
    GFE_AttribDuplicate attribDuplicate(outGeo0, inGeo1, cookparms);
    if (inGeo1)
    {
        attribDuplicate.getRef0AttribArray().set(attribClass, sopparms.getDuplicateAttrib());
        attribDuplicate.getRef0GroupArray() .set(attribClass, sopparms.getGroupName());
    }
    else
    {
        attribDuplicate.getInAttribArray().set(attribClass, sopparms.getDuplicateAttrib());
        attribDuplicate.getInGroupArray() .set(attribClass, sopparms.getGroupName());
    }

    
    if (sopparms.getRenameAttrib())
        attribDuplicate.newAttribNames = sopparms.getNewAttribName();
    if (sopparms.getRenameGroup())
        attribDuplicate.newGroupNames = sopparms.getNewGroupName();
    
    attribDuplicate.computeAndBumpDataId();
    attribDuplicate.visualizeOutGroup();
}
