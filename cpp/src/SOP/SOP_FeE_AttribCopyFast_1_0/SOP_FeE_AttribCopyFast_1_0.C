
//#define UT_ASSERT_LEVEL 3
#include "SOP_FeE_AttribCopyFast_1_0.h"


#include "SOP_FeE_AttribCopyFast_1_0.proto.h"

#include "GEO/GEO_Detail.h"
#include "PRM/PRM_TemplateBuilder.h"
#include "UT/UT_Interrupt.h"
#include "UT/UT_DSOVersion.h"


#include "GA_FeE/GA_FeE_Group.h"
#include "GA_FeE/GA_FeE_Attribute.h"
#include "GA_FeE/GA_FeE_VertexNextEquiv.h"
#include "GA_FeE/GA_FeE_Detail.h"

using namespace SOP_FeE_AttribCopyFast_1_0_Namespace;



static const char *theDsFile = R"THEDSFILE(
{
    name        parameters

    parm {
        name    "doCopyPrimAttrib"
        cppname "DoCopyPrimAttrib"
        label   "Do Copy Prim Attrib"
        type    toggle
        default { 1 }
        nolabel
        joinnext
    }
    parm {
        name    "copyPrimAttribName"
        cppname "CopyPrimAttribName"
        label   "Copy Prim Attrib Name"
        type    string
        default { "" }
        disablewhen "{ doCopyPrimAttrib == 0 }"
    }



    parm {
        name    "doCopyPointAttrib"
        cppname "DoCopyPointAttrib"
        label   "Do Copy Point Attrib"
        type    toggle
        default { 1 }
        nolabel
        joinnext
    }
    parm {
        name    "copyPointAttribName"
        cppname "CopyPointAttribName"
        label   "Copy Point Attrib Name"
        type    string
        default { "" }
        disablewhen "{ doCopyPointAttrib == 0 }"
    }



    parm {
        name    "doCopyVertexAttrib"
        cppname "DoCopyVertexAttrib"
        label   "Do Copy Vertex Attrib"
        type    toggle
        default { 1 }
        nolabel
        joinnext
    }
    parm {
        name    "copyVertexAttribName"
        cppname "CopyVertexAttribName"
        label   "Copy Vertex Attrib Name"
        type    string
        default { "" }
        disablewhen "{ doCopyVertexAttrib == 0 }"
    }





    parm {
        name    "doCopyDetailAttrib"
        cppname "DoCopyDetailAttrib"
        label   "Do Copy Detail Attrib"
        type    toggle
        default { 1 }
        nolabel
        joinnext
    }
    parm {
        name    "copyDetailAttribName"
        cppname "CopyDetailAttribName"
        label   "Copy Detail Attrib Name"
        type    string
        default { "" }
        disablewhen "{ doCopyDetailAttrib == 0 }"
    }



    parm {
        name    "sepparm"
        label   "Separator"
        type    separator
        default { "" }
    }



    parm {
        name    "doCopyPointGroup"
        cppname "DoCopyPointGroup"
        label   "Do Copy Point Group"
        type    toggle
        default { 1 }
        nolabel
        joinnext
    }
    parm {
        name    "copyPointGroupName"
        cppname "CopyPointGroupName"
        label   "Copy Point Group Name"
        type    string
        default { "" }
        disablewhen "{ doCopyPointGroup == 0 }"
    }



    parm {
        name    "doCopyVertexGroup"
        cppname "DoCopyVertexGroup"
        label   "Do Copy Vertex Group"
        type    toggle
        default { 1 }
        nolabel
        joinnext
    }
    parm {
        name    "copyVertexGroupName"
        cppname "CopyVertexGroupName"
        label   "Copy Vertex Group Name"
        type    string
        default { "" }
        disablewhen "{ doCopyVertexGroup == 0 }"
    }



    parm {
        name    "doCopyPrimGroup"
        cppname "DoCopyPrimGroup"
        label   "Do Copy Prim Group"
        type    toggle
        default { 1 }
        nolabel
        joinnext
    }
    parm {
        name    "copyPrimGroupName"
        cppname "CopyPrimGroupName"
        label   "Copy Prim Group Name"
        type    string
        default { "" }
        disablewhen "{ doCopyPrimGroup == 0 }"
    }



    parm {
        name    "doCopyEdgeGroup"
        cppname "DoCopyEdgeGroup"
        label   "Do Copy Edge Group"
        type    toggle
        default { 1 }
        nolabel
        joinnext
    }
    parm {
        name    "copyEdgeGroupName"
        cppname "CopyEdgeGroupName"
        label   "Copy Edge Group Name"
        type    string
        default { "" }
        disablewhen "{ doCopyEdgeGroup == 0 }"
    }




    parm {
        name    "subscribeRatio"
        cppname "SubscribeRatio"
        label   "Subscribe Ratio"
        type    integer
        default { 16 }
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
SOP_FeE_AttribCopyFast_1_0::buildTemplates()
{
    static PRM_TemplateBuilder templ("SOP_FeE_AttribCopyFast_1_0.C"_sh, theDsFile);
    if (templ.justBuilt())
    {
        templ.setChoiceListPtr("copyPrimAttribName"_sh,    &SOP_Node::primAttribMenu);
        templ.setChoiceListPtr("copyPointAttribName"_sh,   &SOP_Node::pointAttribMenu);
        templ.setChoiceListPtr("copyVertexAttribName"_sh,  &SOP_Node::vertexAttribMenu);
        templ.setChoiceListPtr("copyDetailAttribName"_sh,  &SOP_Node::detailAttribMenu);

        templ.setChoiceListPtr("copyPrimGroupName"_sh,     &SOP_Node::primGroupMenu);
        templ.setChoiceListPtr("copyPointGroupName"_sh,    &SOP_Node::pointGroupMenu);
        templ.setChoiceListPtr("copyVertexGroupName"_sh,   &SOP_Node::vertexNamedGroupMenu);
        templ.setChoiceListPtr("copyEdgeGroupName"_sh,     &SOP_Node::edgeGroupMenu);
    }
    return templ.templates();
}


const UT_StringHolder SOP_FeE_AttribCopyFast_1_0::theSOPTypeName("FeE::attribCopy::4.0"_sh);

void
newSopOperator(OP_OperatorTable* table)
{
    OP_Operator* newOp = new OP_Operator(
        SOP_FeE_AttribCopyFast_1_0::theSOPTypeName,
        "FeE Attribute Copy",
        SOP_FeE_AttribCopyFast_1_0::myConstructor,
        SOP_FeE_AttribCopyFast_1_0::buildTemplates(),
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


//class SOP_FeE_AttribCopyFast_1_0Cache : public SOP_NodeCache
//{
//public:
//    SOP_FeE_AttribCopyFast_1_0Cache() : SOP_NodeCache(),
//        myPrevOutputDetailID(-1)
//    {}
//    ~SOP_FeE_AttribCopyFast_1_0Cache() override {}
//
//    int64 myPrevOutputDetailID;
//};


class SOP_FeE_AttribCopyFast_1_0Verb : public SOP_NodeVerb
{
public:
    SOP_FeE_AttribCopyFast_1_0Verb() {}
    virtual ~SOP_FeE_AttribCopyFast_1_0Verb() {}

    virtual SOP_NodeParms *allocParms() const { return new SOP_FeE_AttribCopyFast_1_0Parms(); }
    //virtual SOP_NodeCache* allocCache() const { return new SOP_FeE_AttribCopyFast_1_0Cache(); }
    virtual UT_StringHolder name() const { return SOP_FeE_AttribCopyFast_1_0::theSOPTypeName; }

    virtual CookMode cookMode(const SOP_NodeParms *parms) const { return COOK_GENERIC; }

    virtual void cook(const CookParms &cookparms) const;

    static const SOP_NodeVerb::Register<SOP_FeE_AttribCopyFast_1_0Verb> theVerb;
};

const SOP_NodeVerb::Register<SOP_FeE_AttribCopyFast_1_0Verb> SOP_FeE_AttribCopyFast_1_0Verb::theVerb;

const SOP_NodeVerb *
SOP_FeE_AttribCopyFast_1_0::cookVerb() const 
{ 
    return SOP_FeE_AttribCopyFast_1_0Verb::theVerb.get();
}





void
SOP_FeE_AttribCopyFast_1_0Verb::cook(const SOP_NodeVerb::CookParms &cookparms) const
{
    auto &&sopparms = cookparms.parms<SOP_FeE_AttribCopyFast_1_0Parms>();
    GA_Detail* outGeo0 = cookparms.gdh().gdpNC();
    //auto sopcache = (SOP_FeE_AttribCopyFast_1_0Cache*)cookparms.cache();

    const GA_Detail* const inGeo0 = cookparms.inputGeo(0);
    const GA_Detail* const inGeo1 = cookparms.inputGeo(1);

    outGeo0->replaceWith(*inGeo0);

    GA_AttributeSet& attribSet = outGeo0->getAttributes();

    //const exint subscribeRatio = sopparms.getSubscribeRatio();
    //const exint minGrainSize = sopparms.getMinGrainSize();

    UT_AutoInterrupt boss("Processing");
    if (boss.wasInterrupted())
        return;



    const UT_StringHolder& allPattern = "";
    const UT_StringHolder& copyPrimAttribName   = sopparms.getDoCopyPrimAttrib()   ? sopparms.getCopyPrimAttribName()   : allPattern;
    const UT_StringHolder& copyPointAttribName  = sopparms.getDoCopyPointAttrib()  ? sopparms.getCopyPointAttribName()  : allPattern;
    const UT_StringHolder& copyVertexAttribName = sopparms.getDoCopyVertexAttrib() ? sopparms.getCopyVertexAttribName() : allPattern;
    const UT_StringHolder& copyDetailAttribName = sopparms.getDoCopyDetailAttrib() ? sopparms.getCopyDetailAttribName() : allPattern;
    GA_FeE_Attribute::copyAttribute(outGeo0, inGeo1, GA_ATTRIB_PRIMITIVE, copyPrimAttribName);
    //GA_FeE_Attribute::copyAttribute(attribSet, copyPrimAttribName, copyPointAttribName, copyVertexAttribName, copyDetailAttribName);

    const UT_StringHolder& copyPrimGroupName   = sopparms.getDoCopyPrimGroup()   ? sopparms.getCopyPrimGroupName()   : allPattern;
    const UT_StringHolder& copyPointGroupName  = sopparms.getDoCopyPointGroup()  ? sopparms.getCopyPointGroupName()  : allPattern;
    const UT_StringHolder& copyVertexGroupName = sopparms.getDoCopyVertexGroup() ? sopparms.getCopyVertexGroupName() : allPattern;
    const UT_StringHolder& copyEdgeGroupName   = sopparms.getDoCopyEdgeGroup()   ? sopparms.getCopyEdgeGroupName()   : allPattern;
    //GA_FeE_Group::copyGroup(outGeo0, copyPrimGroupName, copyPointGroupName, copyVertexGroupName, copyEdgeGroupName);
}



namespace SOP_FeE_AttribCopyFast_1_0_Namespace {

} // End SOP_FeE_AttribCopyFast_1_0_Namespace namespace
