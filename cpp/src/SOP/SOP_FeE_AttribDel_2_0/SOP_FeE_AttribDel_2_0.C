
//#define UT_ASSERT_LEVEL 3

#include "SOP_FeE_AttribDel_2_0.h"
#include "SOP_FeE_AttribDel_2_0.proto.h"

#include <GFE/GeoFilter.h>

using namespace SOP_FeE_AttribDel_2_0_Namespace;

static const char* theDsFile = R"THEDSFILE(
{
    name        parameters

    parm {
        name    "doDelPrimAttrib"
        cppname "DoDelPrimAttrib"
        label   "Do Del Prim Attrib"
        type    toggle
        default { 1 }
        nolabel
        joinnext
    }
    parm {
        name    "delPrimAttrib"
        cppname "DelPrimAttrib"
        label   "Delete Prim Attrib"
        type    string
        default { "" }
        disablewhen "{ doDelPrimAttrib == 0 }"
    }



    parm {
        name    "doDelPointAttrib"
        cppname "DoDelPointAttrib"
        label   "Do Del Point Attrib"
        type    toggle
        default { 1 }
        nolabel
        joinnext
    }
    parm {
        name    "delPointAttrib"
        cppname "DelPointAttrib"
        label   "Delete Point Attrib"
        type    string
        default { "" }
        disablewhen "{ doDelPointAttrib == 0 }"
    }



    parm {
        name    "doDelVertexAttrib"
        cppname "DoDelVertexAttrib"
        label   "Do Del Vertex Attrib"
        type    toggle
        default { 1 }
        nolabel
        joinnext
    }
    parm {
        name    "delVertexAttrib"
        cppname "DelVertexAttrib"
        label   "Delete Vertex Attrib"
        type    string
        default { "" }
        disablewhen "{ doDelVertexAttrib == 0 }"
    }





    parm {
        name    "doDelDetailAttrib"
        cppname "DoDelDetailAttrib"
        label   "Do Del Detail Attrib"
        type    toggle
        default { 1 }
        nolabel
        joinnext
    }
    parm {
        name    "delDetailAttrib"
        cppname "DelDetailAttrib"
        label   "Delete Detail Attrib"
        type    string
        default { "" }
        disablewhen "{ doDelDetailAttrib == 0 }"
    }



    parm {
        name    "sepparm"
        label   "Separator"
        type    separator
        default { "" }
    }



    parm {
        name    "doDelPointGroup"
        cppname "DoDelPointGroup"
        label   "Do Del Point Group"
        type    toggle
        default { 1 }
        nolabel
        joinnext
    }
    parm {
        name    "delPointGroup"
        cppname "DelPointGroup"
        label   "Delete Point Group"
        type    string
        default { "" }
        disablewhen "{ doDelPointGroup == 0 }"
    }



    parm {
        name    "doDelVertexGroup"
        cppname "DoDelVertexGroup"
        label   "Do Del Vertex Group"
        type    toggle
        default { 1 }
        nolabel
        joinnext
    }
    parm {
        name    "delVertexGroup"
        cppname "DelVertexGroup"
        label   "Delete Vertex Group"
        type    string
        default { "" }
        disablewhen "{ doDelVertexGroup == 0 }"
    }



    parm {
        name    "doDelPrimGroup"
        cppname "DoDelPrimGroup"
        label   "Do Del Prim Group"
        type    toggle
        default { 1 }
        nolabel
        joinnext
    }
    parm {
        name    "delPrimGroup"
        cppname "DelPrimGroup"
        label   "Delete Prim Group"
        type    string
        default { "" }
        disablewhen "{ doDelPrimGroup == 0 }"
    }



    parm {
        name    "doDelEdgeGroup"
        cppname "DoDelEdgeGroup"
        label   "Do Del Edge Group"
        type    toggle
        default { 1 }
        nolabel
        joinnext
    }
    parm {
        name    "delEdgeGroup"
        cppname "DelEdgeGroup"
        label   "Delete Edge Group"
        type    string
        default { "" }
        disablewhen "{ doDelEdgeGroup == 0 }"
    }


}
)THEDSFILE";


PRM_Template*
SOP_FeE_AttribDel_2_0::buildTemplates()
{
    static PRM_TemplateBuilder templ("SOP_FeE_AttribDel_2_0.C"_sh, theDsFile);
    if (templ.justBuilt())
    {
        templ.setChoiceListPtr("delPrimAttrib"_sh,    &SOP_Node::primAttribMenu);
        templ.setChoiceListPtr("delPointAttrib"_sh,   &SOP_Node::pointAttribMenu);
        templ.setChoiceListPtr("delVertexAttrib"_sh,  &SOP_Node::vertexAttribMenu);
        templ.setChoiceListPtr("delDetailAttrib"_sh,  &SOP_Node::detailAttribMenu);

        templ.setChoiceListPtr("delPrimGroup"_sh,     &SOP_Node::primNamedGroupMenu);
        templ.setChoiceListPtr("delPointGroup"_sh,    &SOP_Node::pointNamedGroupMenu);
        templ.setChoiceListPtr("delVertexGroup"_sh,   &SOP_Node::vertexNamedGroupMenu);
        templ.setChoiceListPtr("delEdgeGroup"_sh,     &SOP_Node::edgeNamedGroupMenu);
    }
    return templ.templates();
}

const UT_StringHolder SOP_FeE_AttribDel_2_0::theSOPTypeName("FeE::attribDel::2.0"_sh);

void
newSopOperator(OP_OperatorTable* table)
{
    OP_Operator* newOp = new OP_Operator(
        SOP_FeE_AttribDel_2_0::theSOPTypeName,
        "FeE Attribute Delete",
        SOP_FeE_AttribDel_2_0::myConstructor,
        SOP_FeE_AttribDel_2_0::buildTemplates(),
        1,
        1,
        nullptr,
        OP_FLAG_GENERATOR,
        nullptr,
        1,
        "Five elements Elf/Attribute");

    newOp->setIconName("SOP_attribdelete");
    table->addOperator(newOp);
}

//class SOP_FeE_AttribDel_2_0Cache : public SOP_NodeCache
//{
//public:
//    SOP_FeE_AttribDel_2_0Cache() : SOP_NodeCache(),
//        myPrevOutputDetailID(-1)
//    {}
//    ~SOP_FeE_AttribDel_2_0Cache() override {}
//
//    int64 myPrevOutputDetailID;
//};


class SOP_FeE_AttribDel_2_0Verb : public SOP_NodeVerb
{
public:
    SOP_FeE_AttribDel_2_0Verb()
    {
    }

    virtual ~SOP_FeE_AttribDel_2_0Verb()
    {
    }

    virtual SOP_NodeParms* allocParms() const { return new SOP_FeE_AttribDel_2_0Parms(); }
    //virtual SOP_NodeCache* allocCache() const { return new SOP_FeE_AttribDel_2_0Cache(); }
    virtual UT_StringHolder name() const { return SOP_FeE_AttribDel_2_0::theSOPTypeName; }

    virtual CookMode cookMode(const SOP_NodeParms* parms) const { return COOK_INPLACE; }

    virtual void cook(const CookParms& cookparms) const;

    /// This static data member automatically registers
    /// this verb class at library ldir0d time.
    static const SOP_NodeVerb::Register<SOP_FeE_AttribDel_2_0Verb> theVerb;
};

// The static member variable definition has to be outside the class definition.
// The declaration is inside the class.
const SOP_NodeVerb::Register<SOP_FeE_AttribDel_2_0Verb> SOP_FeE_AttribDel_2_0Verb::theVerb;

const SOP_NodeVerb*
SOP_FeE_AttribDel_2_0::cookVerb() const
{
    return SOP_FeE_AttribDel_2_0Verb::theVerb.get();
}





void
SOP_FeE_AttribDel_2_0Verb::cook(const SOP_NodeVerb::CookParms& cookparms) const
{
    auto&& sopparms = cookparms.parms<SOP_FeE_AttribDel_2_0Parms>();
    GA_Detail& outGeo0 = *cookparms.gdh().gdpNC();
    //auto sopcache = (SOP_FeE_AttribDel_2_0Cache*)cookparms.cache();

    //const GA_Detail& inGeo0 = *cookparms.inputGeo(0);
    //outGeo0.replaceWith(inGeo0);
    
    UT_AutoInterrupt boss("Processing");
    if (boss.wasInterrupted())
        return;

    auto& outGFE0 = static_cast<gfe::GFE_Detail&>(outGeo0);
    
    const UT_StringHolder& allPattern = "*";
    const UT_StringHolder& delPrimAttrib   = sopparms.getDoDelPrimAttrib()   ? sopparms.getDelPrimAttrib()   : allPattern;
    const UT_StringHolder& delPointAttrib  = sopparms.getDoDelPointAttrib()  ? sopparms.getDelPointAttrib()  : allPattern;
    const UT_StringHolder& delVertexAttrib = sopparms.getDoDelVertexAttrib() ? sopparms.getDelVertexAttrib() : allPattern;
    const UT_StringHolder& delDetailAttrib = sopparms.getDoDelDetailAttrib() ? sopparms.getDelDetailAttrib() : allPattern;
    outGFE0.delStdAttribute(delPrimAttrib, delPointAttrib, delVertexAttrib, delDetailAttrib);
    //GA_AttributeSet& attribSet = outGeo0.getAttributes();
    //GFE_Attribute::delStdAttribute(attribSet, attribSet, delPrimAttrib, delPointAttrib, delVertexAttrib, delDetailAttrib);

    const UT_StringHolder& delPrimGroup   = sopparms.getDoDelPrimGroup()   ? sopparms.getDelPrimGroup()   : allPattern;
    const UT_StringHolder& delPointGroup  = sopparms.getDoDelPointGroup()  ? sopparms.getDelPointGroup()  : allPattern;
    const UT_StringHolder& delVertexGroup = sopparms.getDoDelVertexGroup() ? sopparms.getDelVertexGroup() : allPattern;
    const UT_StringHolder& delEdgeGroup   = sopparms.getDoDelEdgeGroup()   ? sopparms.getDelEdgeGroup()   : allPattern;
    outGFE0.delStdGroup(delPrimGroup, delPointGroup, delVertexGroup, delEdgeGroup);
}

