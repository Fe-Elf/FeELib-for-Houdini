
//#define UT_ASSERT_LEVEL 3
#include "SOP_FeE_DelAllAttribGroup_1_0.h"

#include "SOP_FeE_DelAllAttribGroup_1_0.proto.h"

#include "GA/GA_Detail.h"
#include "PRM/PRM_TemplateBuilder.h"
#include "UT/UT_Interrupt.h"
#include "UT/UT_DSOVersion.h"


#include "GFE/GFE_Detail.h"

using namespace SOP_FeE_DelAllAttribGroup_1_0_Namespace;



static const char *theDsFile = R"THEDSFILE(
{
    name        parameters

    parm {
        name    "doKeepPrimAttrib"
        cppname "DoKeepPrimAttrib"
        label   "Do Keep Prim Attrib"
        type    toggle
        default { 1 }
        nolabel
        joinnext
    }
    parm {
        name    "keepPrimAttrib"
        cppname "KeepPrimAttrib"
        label   "Keep Prim Attrib"
        type    string
        default { "" }
        disablewhen "{ doKeepPrimAttrib == 0 }"
    }



    parm {
        name    "doKeepPointAttrib"
        cppname "DoKeepPointAttrib"
        label   "Do Keep Point Attrib"
        type    toggle
        default { 1 }
        nolabel
        joinnext
    }
    parm {
        name    "keepPointAttrib"
        cppname "KeepPointAttrib"
        label   "Keep Point Attrib"
        type    string
        default { "" }
        disablewhen "{ doKeepPointAttrib == 0 }"
    }



    parm {
        name    "doKeepVertexAttrib"
        cppname "DoKeepVertexAttrib"
        label   "Do Keep Vertex Attrib"
        type    toggle
        default { 1 }
        nolabel
        joinnext
    }
    parm {
        name    "keepVertexAttrib"
        cppname "KeepVertexAttrib"
        label   "Keep Vertex Attrib"
        type    string
        default { "" }
        disablewhen "{ doKeepVertexAttrib == 0 }"
    }





    parm {
        name    "doKeepDetailAttrib"
        cppname "DoKeepDetailAttrib"
        label   "Do Keep Detail Attrib"
        type    toggle
        default { 1 }
        nolabel
        joinnext
    }
    parm {
        name    "keepDetailAttrib"
        cppname "KeepDetailAttrib"
        label   "Keep Detail Attrib"
        type    string
        default { "" }
        disablewhen "{ doKeepDetailAttrib == 0 }"
    }



    parm {
        name    "sepparm"
        label   "Separator"
        type    separator
        default { "" }
    }



    parm {
        name    "doKeepPointGroup"
        cppname "DoKeepPointGroup"
        label   "Do Keep Point Group"
        type    toggle
        default { 1 }
        nolabel
        joinnext
    }
    parm {
        name    "keepPointGroup"
        cppname "KeepPointGroup"
        label   "Keep Point Group"
        type    string
        default { "" }
        disablewhen "{ doKeepPointGroup == 0 }"
    }



    parm {
        name    "doKeepVertexGroup"
        cppname "DoKeepVertexGroup"
        label   "Do Keep Vertex Group"
        type    toggle
        default { 1 }
        nolabel
        joinnext
    }
    parm {
        name    "keepVertexGroup"
        cppname "KeepVertexGroup"
        label   "Keep Vertex Group"
        type    string
        default { "" }
        disablewhen "{ doKeepVertexGroup == 0 }"
    }



    parm {
        name    "doKeepPrimGroup"
        cppname "DoKeepPrimGroup"
        label   "Do Keep Prim Group"
        type    toggle
        default { 1 }
        nolabel
        joinnext
    }
    parm {
        name    "keepPrimGroup"
        cppname "KeepPrimGroup"
        label   "Keep Prim Group"
        type    string
        default { "" }
        disablewhen "{ doKeepPrimGroup == 0 }"
    }



    parm {
        name    "doKeepEdgeGroup"
        cppname "DoKeepEdgeGroup"
        label   "Do Keep Edge Group"
        type    toggle
        default { 1 }
        nolabel
        joinnext
    }
    parm {
        name    "keepEdgeGroup"
        cppname "KeepEdgeGroup"
        label   "Keep Edge Group"
        type    string
        default { "" }
        disablewhen "{ doKeepEdgeGroup == 0 }"
    }


}
)THEDSFILE";



PRM_Template*
SOP_FeE_DelAllAttribGroup_1_0::buildTemplates()
{
    static PRM_TemplateBuilder templ("SOP_FeE_DelAllAttribGroup_1_0.C"_sh, theDsFile);
    if (templ.justBuilt())
    {
        templ.setChoiceListPtr("keepPrimAttrib"_sh,    &SOP_Node::primAttribMenu);
        templ.setChoiceListPtr("keepPointAttrib"_sh,   &SOP_Node::pointAttribMenu);
        templ.setChoiceListPtr("keepVertexAttrib"_sh,  &SOP_Node::vertexAttribMenu);
        templ.setChoiceListPtr("keepDetailAttrib"_sh,  &SOP_Node::detailAttribMenu);

        templ.setChoiceListPtr("keepPrimGroup"_sh,     &SOP_Node::primNamedGroupMenu);
        templ.setChoiceListPtr("keepPointGroup"_sh,    &SOP_Node::pointNamedGroupMenu);
        templ.setChoiceListPtr("keepVertexGroup"_sh,   &SOP_Node::vertexNamedGroupMenu);
        templ.setChoiceListPtr("keepEdgeGroup"_sh,     &SOP_Node::edgeNamedGroupMenu);
    }
    return templ.templates();
}


const UT_StringHolder SOP_FeE_DelAllAttribGroup_1_0::theSOPTypeName("FeE::delAllAttribGroup::1.0"_sh);

void
newSopOperator(OP_OperatorTable* table)
{
    OP_Operator* newOp = new OP_Operator(
        SOP_FeE_DelAllAttribGroup_1_0::theSOPTypeName,
        "FeE Delete All Attributes Groups",
        SOP_FeE_DelAllAttribGroup_1_0::myConstructor,
        SOP_FeE_DelAllAttribGroup_1_0::buildTemplates(),
        1,
        1,
        nullptr,
        0,
        nullptr,
        1,
        "Five elements Elf/Attribute");

    newOp->setIconName("SOP_clean");
    table->addOperator(newOp);

}


//class SOP_FeE_DelAllAttribGroup_1_0Cache : public SOP_NodeCache
//{
//public:
//    SOP_FeE_DelAllAttribGroup_1_0Cache() : SOP_NodeCache(),
//        myPrevOutputDetailID(-1)
//    {}
//    ~SOP_FeE_DelAllAttribGroup_1_0Cache() override {}
//
//    int64 myPrevOutputDetailID;
//};


class SOP_FeE_DelAllAttribGroup_1_0Verb : public SOP_NodeVerb
{
public:
    SOP_FeE_DelAllAttribGroup_1_0Verb() {}
    virtual ~SOP_FeE_DelAllAttribGroup_1_0Verb() {}

    virtual SOP_NodeParms *allocParms() const { return new SOP_FeE_DelAllAttribGroup_1_0Parms(); }
    //virtual SOP_NodeCache* allocCache() const { return new SOP_FeE_DelAllAttribGroup_1_0Cache(); }
    virtual UT_StringHolder name() const { return SOP_FeE_DelAllAttribGroup_1_0::theSOPTypeName; }

    virtual CookMode cookMode(const SOP_NodeParms *parms) const { return COOK_INPLACE; }

    virtual void cook(const CookParms &cookparms) const;

    static const SOP_NodeVerb::Register<SOP_FeE_DelAllAttribGroup_1_0Verb> theVerb;
};

const SOP_NodeVerb::Register<SOP_FeE_DelAllAttribGroup_1_0Verb> SOP_FeE_DelAllAttribGroup_1_0Verb::theVerb;

const SOP_NodeVerb *
SOP_FeE_DelAllAttribGroup_1_0::cookVerb() const 
{ 
    return SOP_FeE_DelAllAttribGroup_1_0Verb::theVerb.get();
}





void
SOP_FeE_DelAllAttribGroup_1_0Verb::cook(const SOP_NodeVerb::CookParms &cookparms) const
{
    auto &&sopparms = cookparms.parms<SOP_FeE_DelAllAttribGroup_1_0Parms>();
    GA_Detail& outGeo0 = *cookparms.gdh().gdpNC();
    //auto sopcache = (SOP_FeE_DelAllAttribGroup_1_0Cache*)cookparms.cache();

    //const GA_Detail& inGeo0 = *cookparms.inputGeo(0);

    //outGeo0.replaceWith(inGeo0);
    

    UT_AutoInterrupt boss("Processing");
    if (boss.wasInterrupted())
        return;

    GFE_Detail& outGFE0 = static_cast<GFE_Detail&>(outGeo0);
    
/*
            geo->keepStdAttribute(keepPrimAttrib, keepPointAttrib, keepVertexAttrib, keepDetailAttrib);
            geo->keepStdGroup(keepPrimGroup, keepPointGroup, keepVertexGroup, keepEdgeGroup);
            geo->keepStdAttribute("*", "", "", "");
            geo->keepStdGroup("*", "", "", "");
*/
        

    const char* const allPattern = "*";
    const char* const keepPrimAttrib   = sopparms.getDoKeepPrimAttrib()   ? sopparms.getKeepPrimAttrib()  .c_str() : allPattern;
    const char* const keepPointAttrib  = sopparms.getDoKeepPointAttrib()  ? sopparms.getKeepPointAttrib() .c_str() : allPattern;
    const char* const keepVertexAttrib = sopparms.getDoKeepVertexAttrib() ? sopparms.getKeepVertexAttrib().c_str() : allPattern;
    const char* const keepDetailAttrib = sopparms.getDoKeepDetailAttrib() ? sopparms.getKeepDetailAttrib().c_str() : allPattern;
    outGFE0.keepStdAttribute(keepPrimAttrib, keepPointAttrib, keepVertexAttrib, keepDetailAttrib);
    //GA_AttributeSet& attribSet = outGeo0.getAttributes();
    //GFE_Attribute::keepStdAttribute(attribSet, attribSet, keepPrimAttrib, keepPointAttrib, keepVertexAttrib, keepDetailAttrib);

    const char* const keepPrimGroup   = sopparms.getDoKeepPrimGroup()   ? sopparms.getKeepPrimGroup()  .c_str() : allPattern;
    const char* const keepPointGroup  = sopparms.getDoKeepPointGroup()  ? sopparms.getKeepPointGroup() .c_str() : allPattern;
    const char* const keepVertexGroup = sopparms.getDoKeepVertexGroup() ? sopparms.getKeepVertexGroup().c_str() : allPattern;
    const char* const keepEdgeGroup   = sopparms.getDoKeepEdgeGroup()   ? sopparms.getKeepEdgeGroup()  .c_str() : allPattern;
    outGFE0.keepStdGroup(keepPrimGroup, keepPointGroup, keepVertexGroup, keepEdgeGroup);
    //GFE_Group::keepStdGroup(outGeo0, keepPrimGroup, keepPointGroup, keepVertexGroup, keepEdgeGroup);
}

