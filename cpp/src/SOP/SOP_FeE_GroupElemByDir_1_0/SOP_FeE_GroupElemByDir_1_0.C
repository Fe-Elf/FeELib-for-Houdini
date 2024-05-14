
//#define UT_ASSERT_LEVEL 3
#include "SOP_FeE_GroupElemByDir_1_0.h"


#include "SOP_FeE_GroupElemByDir_1_0.proto.h"

#include "GA/GA_Detail.h"
#include "PRM/PRM_TemplateBuilder.h"
#include "UT/UT_Interrupt.h"
#include "UT/UT_DSOVersion.h"


#include "GFE/GFE_GroupElementByDirection.h"

using namespace SOP_FeE_GroupElemByDir_1_0_Namespace;



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

    parm {
        name    "groupElemByDirMethod"
        cppname "GroupElemByDirMethod"
        label   "Method"
        type    ordinal
        default { "input" }
        menu {
            "input"                "Input"
            "restDir2D_avgNormal"  "RestDir2D AvgNormal"
            "restDir2D_houOBB"     "RestDir2D Hou OBB"
        }
    }
    parm {
        name    "sameDirGroupType"
        cppname "SameDirGroupType"
        label   "Same Dir Group Type"
        type    ordinal
        default { "prim" }
        menu {
            "prim"      "Primitive"
            "point"     "Point"
            "vertex"    "Vertex"
    //         "edge"      "Edge"
        }
    }
    parm {
        name    "sameDirGroupName"
        cppname "SameDirGroupName"
        label   "Same Dir Group Name"
        type    string
        default { "sameDir" }
    }

    parm {
        name    "dirAttrib"
        cppname "DirAttrib"
        label   "Dir Attribute"
        type    string
        default { "N" }
    }
    parm {
        name    "coneAngleThreshold"
        cppname "ConeAngleThreshold"
        label   "Cone Angle Threshold"
        type    angle
        default { "90" }
        range   { 0! 180! }
    }
    parm {
        name    "up"
        cppname "Up"
        label   "Up"
        type    vector
        size    3
        default { "0" "1" "0" }
        disablewhen "{ groupElemByDirMethod != input matchUpDir == 0 }"
    }
    parm {
        name    "normal3DAttribClass"
        cppname "Normal3DAttribClass"
        label   "Normal 3D Attrib Class"
        type    ordinal
        default { "point" }
        disablewhen "{ groupElemByDirMethod != restDir2D_avgNormal }"
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
        disablewhen "{ groupElemByDirMethod != restDir2D_avgNormal }"
    }

    parm {
        name    "matchUpDir"
        cppname "MatchUpDir"
        label   "Match Up Dir"
        type    toggle
        default { "1" }
    }

    parm {
        name    "reverseGroup"
        cppname "ReverseGroup"
        label   "Reverse Group"
        type    toggle
        default { "0" }
    }
    parm {
        name    "delElem"
        cppname "DelElem"
        label   "Delete Element"
        type    toggle
        default { "0" }
    }
    parm {
        name    "reversePrim"
        cppname "ReversePrim"
        label   "Reverse Prim"
        type    toggle
        default { "0" }
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
SOP_FeE_GroupElemByDir_1_0::buildTemplates()
{
    static PRM_TemplateBuilder templ("SOP_FeE_GroupElemByDir_1_0.C"_sh, theDsFile);
    if (templ.justBuilt())
    {
        templ.setChoiceListPtr("group"_sh, &SOP_Node::allGroupMenu);
        templ.setChoiceListPtr("normal3DAttribClass"_sh, &SOP_Node::allAttribReplaceMenu);
    }
    return templ.templates();
}


const UT_StringHolder SOP_FeE_GroupElemByDir_1_0::theSOPTypeName("FeE::groupElemByDir::1.0"_sh);

void
newSopOperator(OP_OperatorTable* table)
{
    OP_Operator* newOp = new OP_Operator(
        SOP_FeE_GroupElemByDir_1_0::theSOPTypeName,
        "FeE Group Element by Direction",
        SOP_FeE_GroupElemByDir_1_0::myConstructor,
        SOP_FeE_GroupElemByDir_1_0::buildTemplates(),
        1,
        2,
        nullptr,
        OP_FLAG_GENERATOR,
        nullptr,
        1,
        "Five elements Elf/Data/Orient");

    newOp->setIconName("SOP_normal");
    table->addOperator(newOp);

}

//class SOP_FeE_GroupElemByDir_1_0Cache : public SOP_NodeCache
//{
//public:
//    SOP_FeE_GroupElemByDir_1_0Cache() : SOP_NodeCache(),
//        myPrevOutputDetailID(-1)
//    {}
//    ~SOP_FeE_GroupElemByDir_1_0Cache() override {}
//
//    int64 myPrevOutputDetailID;
//};


class SOP_FeE_GroupElemByDir_1_0Verb : public SOP_NodeVerb
{
public:
    SOP_FeE_GroupElemByDir_1_0Verb() {}
    virtual ~SOP_FeE_GroupElemByDir_1_0Verb() {}

    virtual SOP_NodeParms *allocParms() const { return new SOP_FeE_GroupElemByDir_1_0Parms(); }
    //virtual SOP_NodeCache* allocCache() const { return new SOP_FeE_GroupElemByDir_1_0Cache(); }
    virtual UT_StringHolder name() const { return SOP_FeE_GroupElemByDir_1_0::theSOPTypeName; }

    virtual CookMode cookMode(const SOP_NodeParms *parms) const { return COOK_GENERIC; }

    virtual void cook(const CookParms &cookparms) const;

    /// This static data member automatically registers
    /// this verb class at library ldir0d time.
    static const SOP_NodeVerb::Register<SOP_FeE_GroupElemByDir_1_0Verb> theVerb;
};

// The static member variable definition has to be outside the class definition.
// The declaration is inside the class.
const SOP_NodeVerb::Register<SOP_FeE_GroupElemByDir_1_0Verb> SOP_FeE_GroupElemByDir_1_0Verb::theVerb;

const SOP_NodeVerb *
SOP_FeE_GroupElemByDir_1_0::cookVerb() const 
{ 
    return SOP_FeE_GroupElemByDir_1_0Verb::theVerb.get();
}






static GFE_NormalSearchOrder
sopAttribSearchOrder(const SOP_FeE_GroupElemByDir_1_0Parms::Normal3DAttribClass attribClass)
{
    using namespace SOP_FeE_GroupElemByDir_1_0Enums;
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

static GA_GroupType
sopGroupType(const SOP_FeE_GroupElemByDir_1_0Parms::SameDirGroupType parmGroupType)
{
    using namespace SOP_FeE_GroupElemByDir_1_0Enums;
    switch (parmGroupType)
    {
    case SameDirGroupType::PRIM:      return GA_GROUP_PRIMITIVE;  break;
    case SameDirGroupType::POINT:     return GA_GROUP_POINT;      break;
    case SameDirGroupType::VERTEX:    return GA_GROUP_VERTEX;     break;
    //case SameDirGroupType::EDGE:      return GA_GROUP_EDGE;       break;
    }
    UT_ASSERT_MSG(0, "Unhandled SameDirGroupType!");
    return GA_GROUP_INVALID;
}

static GA_GroupType
sopGroupType(const SOP_FeE_GroupElemByDir_1_0Parms::GroupType parmGroupType)
{
    using namespace SOP_FeE_GroupElemByDir_1_0Enums;
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

static GFE_GroupElemByDirMethod
sopMethod(const SOP_FeE_GroupElemByDir_1_0Parms::GroupElemByDirMethod parmMethod)
{
    using namespace SOP_FeE_GroupElemByDir_1_0Enums;
    switch (parmMethod)
    {
    case GroupElemByDirMethod::INPUT:                return GFE_GroupElemByDirMethod::Input;               break;
    case GroupElemByDirMethod::RESTDIR2D_AVGNORMAL:  return GFE_GroupElemByDirMethod::RestDir2D_AvgNormal; break;
    case GroupElemByDirMethod::RESTDIR2D_HOUOBB:     return GFE_GroupElemByDirMethod::RestDir2D_HouOBB;    break;
    }
    UT_ASSERT_MSG(0, "Unhandled Group Elem by Dir Method!");
    return GFE_GroupElemByDirMethod::RestDir2D_HouOBB;
}



void
SOP_FeE_GroupElemByDir_1_0Verb::cook(const SOP_NodeVerb::CookParms& cookparms) const
{
    auto&& sopparms = cookparms.parms<SOP_FeE_GroupElemByDir_1_0Parms>();
    GA_Detail& outGeo0 = *cookparms.gdh().gdpNC();
    //auto sopcache = (SOP_FeE_GroupElemByDir_1_0Cache*)cookparms.cache();

    const GA_Detail& inGeo0 = *cookparms.inputGeo(0);
    const GA_Detail* const inGeo1 = cookparms.inputGeo(1);

    outGeo0.replaceWith(inGeo0);

    
    const GA_GroupType groupType = sopGroupType(sopparms.getGroupType());
    const GFE_NormalSearchOrder normalSearchOrder = sopAttribSearchOrder(sopparms.getNormal3DAttribClass());
    const GFE_GroupElemByDirMethod method = sopMethod(sopparms.getGroupElemByDirMethod());
    
    const GA_GroupType outGroupType = sopGroupType(sopparms.getSameDirGroupType());
    
    
    UT_AutoInterrupt boss("Processing");
    if (boss.wasInterrupted())
        return;

    
    
    GFE_GroupElemByDir groupElemByDir(outGeo0, inGeo1, &cookparms);

    
    groupElemByDir.setComputeParm(method, sopparms.getUp(), sopparms.getReversePrim(),
        sopparms.getSubscribeRatio(), sopparms.getMinGrainSize());
    groupElemByDir.setConeAngleThreshold(sopparms.getConeAngleThreshold());
    groupElemByDir.doDelGroupElement = sopparms.getDelElem();
    
    groupElemByDir.groupSetter.setComputeParm(sopparms.getReverseGroup());
    
    groupElemByDir.normal3D.findOrCreateNormal3D(false, true, outGroupType, GA_STORE_INVALID, sopparms.getDirAttrib());
    groupElemByDir.findOrCreateGroup(false, outGroupType, sopparms.getSameDirGroupName());
    
    if (method == GFE_GroupElemByDirMethod::RestDir2D_AvgNormal)
        groupElemByDir.restDir2D.normal3D.findOrCreateNormal3D(false, true, normalSearchOrder, GA_STORE_INVALID, sopparms.getNormal3DAttrib());

    if (sopparms.getMatchUpDir())
        groupElemByDir.restDir2D.setMatchUpDir(sopparms.getUp());
    else
        groupElemByDir.restDir2D.setMatchUpDir();

    

    groupElemByDir.groupParser.setGroup(groupType, sopparms.getGroup());
    
    groupElemByDir.computeAndBumpDataId();
    groupElemByDir.visualizeOutGroup();

}
