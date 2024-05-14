
//#define UT_ASSERT_LEVEL 3
#include "SOP_FeE_RestDir2D_1_0.h"


#include "SOP_FeE_RestDir2D_1_0.proto.h"

#include "GA/GA_Detail.h"
#include "PRM/PRM_TemplateBuilder.h"
#include "UT/UT_Interrupt.h"
#include "UT/UT_DSOVersion.h"


#include "GFE/GFE_RestDir2D.h"

using namespace SOP_FeE_RestDir2D_1_0_Namespace;



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
        parmtag { "script_action_help" "Select geometry from an available viewport." }
        parmtag { "script_action_icon" "BUTTONS_reselect" }
    }
    parm {
        name    "groupType"
        cppname "GroupType"
        label   "Group Type"
        type    ordinal
        default { "point" }
        menu {
            "guess"     "Guess from Group"
            "prim"      "Primitive"
            "point"     "Point"
            "vertex"    "Vertex"
            "edge"      "Edge"
        }
    }

    // parm {
    //     name    "runOverPieces"
    //     cppname "RunOverPieces"
    //     label   "Run Over Pieces"
    //     type    toggle
    //     default { "0" }
    // }
    // parm {
    //     name    "findInputPieceAttrib"
    //     cppname "FindInputPieceAttrib"
    //     label   "Find Input Piece Attribute"
    //     type    toggle
    //     default { "0" }
    //     disablewhen "{ runOverPieces == 0 }"
    // }
    // parm {
    //     name    "pieceAttribName"
    //     cppname "PieceAttribName"
    //     label   "Piece Attribute Name"
    //     type    string
    //     default { "class" }
    //     disablewhen "{ runOverPieces == 0 }"
    // }

    parm {
        name    "restDir2DMethod"
        cppname "RestDir2DMethod"
        label   "Method"
        type    ordinal
        default { "avgNormal" }
        menu {
            "avgNormal"  "AvgNormal"
            "houOBB"     "Houdini OBB"
        }
    }




    parm {
        name    "normal3DAttribClass"
        cppname "Normal3DAttribClass"
        label   "Normal 3D Attrib Class"
        type    ordinal
        default { "point" }
        disablewhen "{ restDir2DMethod != avgNormal }"
        menu {
            "prim"          "Prim"
            "point"         "Point"
            "vertex"        "Vertex"
            "detail"        "Detail"
            "pointvertex"   "PointVertex"
            "all"           "ALL"
        }
    }
    parm {
        name    "normal3DAttrib"
        cppname "Normal3DAttrib"
        label   "Normal 3D Attribute"
        type    string
        default { "N" }
        disablewhen "{ restDir2DMethod != avgNormal }"
    }
    parm {
        name    "restDir2DAttribName"
        cppname "RestDir2DAttribName"
        label   "Rest Dir 2D Attribute Name"
        type    string
        default { "restDir2D" }
    }

    // parm {
    //     name    "reverseGroup"
    //     cppname "ReverseGroup"
    //     label   "Reverse Group"
    //     type    toggle
    //     default { "0" }
    // }
    // parm {
    //     name    "reversePrim"
    //     cppname "ReversePrim"
    //     label   "Reverse Prim"
    //     type    toggle
    //     default { "0" }
    // }


    parm {
        name    "matchUpDir"
        cppname "MatchUpDir"
        label   "Match Up Dir"
        type    toggle
        nolabel
        joinnext
        default { "1" }
    }
    parm {
        name    "up"
        cppname "Up"
        label   "Up"
        type    vector
        size    3
        default { "0" "1" "0" }
        disablewhen "{ matchUpDir == 0 }"
    }

    parm {
        name    "subscribeRatio"
        cppname "SubscribeRatio"
        label   "Subscribe Ratio"
        type    integer
        default { 16 }
        range   { 0! 256 }
        invisible
    }
    parm {
        name    "minGrainSize"
        cppname "MinGrainSize"
        label   "Min Grain Size"
        type    intlog
        default { 1024 }
        range   { 0! 2048 }
        invisible
    }
}
)THEDSFILE";



PRM_Template*
SOP_FeE_RestDir2D_1_0::buildTemplates()
{
    static PRM_TemplateBuilder templ("SOP_FeE_RestDir2D_1_0.C"_sh, theDsFile);
    if (templ.justBuilt())
    {
        templ.setChoiceListPtr("group"_sh,          &SOP_Node::allGroupMenu);
        templ.setChoiceListPtr("normal3DAttrib"_sh, &SOP_Node::allAttribReplaceMenu);
    }
    return templ.templates();
}


const UT_StringHolder SOP_FeE_RestDir2D_1_0::theSOPTypeName("FeE::restDir2D::1.0"_sh);

void
newSopOperator(OP_OperatorTable* table)
{
    OP_Operator* newOp = new OP_Operator(
        SOP_FeE_RestDir2D_1_0::theSOPTypeName,
        "FeE Rest Direction 2D",
        SOP_FeE_RestDir2D_1_0::myConstructor,
        SOP_FeE_RestDir2D_1_0::buildTemplates(),
        1,
        1,
        nullptr,
        OP_FLAG_GENERATOR,
        nullptr,
        1,
        "Five elements Elf/Data/Orient");

    newOp->setIconName("SOP_normal");
    table->addOperator(newOp);

}

//class SOP_FeE_RestDir2D_1_0Cache : public SOP_NodeCache
//{
//public:
//    SOP_FeE_RestDir2D_1_0Cache() : SOP_NodeCache(),
//        myPrevOutputDetailID(-1)
//    {}
//    ~SOP_FeE_RestDir2D_1_0Cache() override {}
//
//    int64 myPrevOutputDetailID;
//};


class SOP_FeE_RestDir2D_1_0Verb : public SOP_NodeVerb
{
public:
    SOP_FeE_RestDir2D_1_0Verb() {}
    virtual ~SOP_FeE_RestDir2D_1_0Verb() {}

    virtual SOP_NodeParms *allocParms() const { return new SOP_FeE_RestDir2D_1_0Parms(); }
    //virtual SOP_NodeCache* allocCache() const { return new SOP_FeE_RestDir2D_1_0Cache(); }
    virtual UT_StringHolder name() const { return SOP_FeE_RestDir2D_1_0::theSOPTypeName; }

    virtual CookMode cookMode(const SOP_NodeParms *parms) const { return COOK_GENERIC; }

    virtual void cook(const CookParms &cookparms) const;

    /// This static data member automatically registers
    /// this verb class at library ldir0d time.
    static const SOP_NodeVerb::Register<SOP_FeE_RestDir2D_1_0Verb> theVerb;
};

// The static member variable definition has to be outside the class definition.
// The declaration is inside the class.
const SOP_NodeVerb::Register<SOP_FeE_RestDir2D_1_0Verb> SOP_FeE_RestDir2D_1_0Verb::theVerb;

const SOP_NodeVerb *
SOP_FeE_RestDir2D_1_0::cookVerb() const 
{ 
    return SOP_FeE_RestDir2D_1_0Verb::theVerb.get();
}






static GA_GroupType
sopGroupType(SOP_FeE_RestDir2D_1_0Parms::GroupType parmgrouptype)
{
    using namespace SOP_FeE_RestDir2D_1_0Enums;
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


static GFE_RestDir2DMethod
sopMethod(SOP_FeE_RestDir2D_1_0Parms::RestDir2DMethod parmgrouptype)
{
    using namespace SOP_FeE_RestDir2D_1_0Enums;
    switch (parmgrouptype)
    {
    case RestDir2DMethod::AVGNORMAL:   return GFE_RestDir2DMethod::AvgNormal;   break;
    case RestDir2DMethod::HOUOBB:      return GFE_RestDir2DMethod::HouOBB;   break;
    }
    UT_ASSERT_MSG(0, "Unhandled RestDir2DMethod!");
    return GFE_RestDir2DMethod::AvgNormal;
}


static GFE_NormalSearchOrder
sopAttribSearchOrder(SOP_FeE_RestDir2D_1_0Parms::Normal3DAttribClass attribClass)
{
    using namespace SOP_FeE_RestDir2D_1_0Enums;
    switch (attribClass)
    {
    case Normal3DAttribClass::PRIM:          return GFE_NormalSearchOrder::Primitive;   break;
    case Normal3DAttribClass::POINT:         return GFE_NormalSearchOrder::Point;       break;
    case Normal3DAttribClass::VERTEX:        return GFE_NormalSearchOrder::Vertex;      break;
    case Normal3DAttribClass::DETAIL:        return GFE_NormalSearchOrder::Detail;      break;
    case Normal3DAttribClass::POINTVERTEX:   return GFE_NormalSearchOrder::PointVertex; break;
    case Normal3DAttribClass::ALL:           return GFE_NormalSearchOrder::All;         break;
    }
    UT_ASSERT_MSG(0, "Unhandled GFE Normal Search Order!");
    return GFE_NormalSearchOrder::Invalid;
}


void
SOP_FeE_RestDir2D_1_0Verb::cook(const SOP_NodeVerb::CookParms& cookparms) const
{
    auto&& sopparms = cookparms.parms<SOP_FeE_RestDir2D_1_0Parms>();
    GA_Detail& outGeo0 = *cookparms.gdh().gdpNC();
    //auto sopcache = (SOP_FeE_RestDir2D_1_0Cache*)cookparms.cache();

    const GA_Detail& inGeo0 = *cookparms.inputGeo(0);

    outGeo0.replaceWith(inGeo0);

    const UT_StringHolder& restDir2DAttribName = sopparms.getRestDir2DAttribName();
    if (!restDir2DAttribName.isstring() || restDir2DAttribName.length()==0)
        return;
    
    const GA_GroupType groupType = sopGroupType(sopparms.getGroupType());
    const GFE_NormalSearchOrder normalSearchOrder = sopAttribSearchOrder(sopparms.getNormal3DAttribClass());
    const GFE_RestDir2DMethod method = sopMethod(sopparms.getRestDir2DMethod());

    
    UT_AutoInterrupt boss("Processing");
    if (boss.wasInterrupted())
        return;

    

    GFE_RestDir2D restDir2D(outGeo0, &cookparms);
    restDir2D.groupParser.setGroup(groupType, sopparms.getGroup());

    restDir2D.findOrCreateDir(false, GA_STORE_INVALID, restDir2DAttribName);

    if (method == GFE_RestDir2DMethod::AvgNormal)
        restDir2D.normal3D.findOrCreateNormal3D(true, true, normalSearchOrder, GA_STORE_INVALID, sopparms.getNormal3DAttrib());

    if (sopparms.getMatchUpDir())
        restDir2D.setMatchUpDir(sopparms.getUp());
    else
        restDir2D.setMatchUpDir();
    
    restDir2D.setComputeParm(method,
        sopparms.getSubscribeRatio(), sopparms.getMinGrainSize());

    restDir2D.computeAndBumpDataId();


}

