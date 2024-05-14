
//#define UT_ASSERT_LEVEL 3
#include "SOP_FeE_MatchDirOrient_1_0.h"


#include "SOP_FeE_MatchDirOrient_1_0.proto.h"

#include "GA/GA_Detail.h"
#include "PRM/PRM_TemplateBuilder.h"
#include "UT/UT_Interrupt.h"
#include "UT/UT_DSOVersion.h"




#include "GFE/GFE_MatchDirectionOrient.h"




using namespace SOP_FeE_MatchDirOrient_1_0_Namespace;


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
        name    "dirAttribClass"
        cppname "DirAttribClass"
        label   "Dir Attrib Class"
        type    ordinal
        default { "point" }
        menu {
            "prim"      "Prim"
            "point"     "Point"
            "vertex"    "Vertex"
            "detail"    "Detail"
        }
    }
    parm {
        name    "dirAttrib"
        cppname "DirAttrib"
        label   "Dir Attrib"
        type    string
        default { "N" }
    }
    parm {
        name    "useDirRefAttrib"
        cppname "UseDirRefAttrib"
        label   "Use Dir Ref Attrib"
        type    toggle
        default { "0" }
    }
    parm {
        name    "dirRefAttrib"
        cppname "DirRefAttrib"
        label   "Dir Ref Attrib"
        type    string
        default { "`chs('dirAttrib')`" }
        hidewhen "{ useDirRefAttrib == 0 }"
    }
    parm {
        name    "constDir"
        cppname "ConstDir"
        label   "Const Dir"
        type    vector
        size    3
        default { "0" "1" "0" }
        range   { -1 1 }
        hidewhen "{ useDirRefAttrib == 0 }"
    }
    parm {
        name    "outDir"
        cppname "OutDir"
        label   "Out Direction"
        type    toggle
        default { "1" }
    }
    parm {
        name    "accurate"
        cppname "Accurate"
        label   "Accurate"
        type    toggle
        default { "0" }
        disablewhen "{ outDir == 0 }"
    }
    parm {
        name    "keepBearing"
        cppname "KeepBearing"
        label   "Keep Bearing"
        type    toggle
        default { "0" }
        disablewhen "{ accurate == 0 }"
    }
    parm {
        name    "reverseDir"
        cppname "ReverseDir"
        label   "Reverse Dir"
        type    toggle
        default { "0" }
        disablewhen "{ outDir == 0 }"
    }
    parm {
        name    "outDirReversedGroup"
        cppname "OutDirReversedGroup"
        label   "Out Dir Reversed Group"
        type    toggle
        joinnext
        nolabel
        default { "0" }
    }
    parm {
        name    "dirReversedGroupName"
        cppname "DirReversedGroupName"
        label   "Dir Reversed Group Name"
        type    string
        default { "dirReversed" }
        disablewhen "{ outDirReversedGroup == 0 }"
    }
    parm {
        name    "reverseGroup"
        cppname "ReverseGroup"
        label   "Reverse Group"
        type    toggle
        default { "0" }
        disablewhen "{ outDirReversedGroup == 0 }"
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
SOP_FeE_MatchDirOrient_1_0::buildTemplates()
{
    static PRM_TemplateBuilder templ("SOP_FeE_MatchDirOrient_1_0.C"_sh, theDsFile);
    if (templ.justBuilt())
    {
        templ.setChoiceListPtr("group"_sh, &SOP_Node::allGroupMenu);
        templ.setChoiceListPtr("dirAttrib"_sh,    &SOP_Node::allAttribReplaceMenu);
        templ.setChoiceListPtr("dirRefAttrib"_sh, &SOP_Node::allAttribReplaceMenu);
    }
    return templ.templates();
}

const UT_StringHolder SOP_FeE_MatchDirOrient_1_0::theSOPTypeName("FeE::matchDirOrient::1.0"_sh);

void
newSopOperator(OP_OperatorTable* table)
{
    OP_Operator* newOp = new OP_Operator(
        SOP_FeE_MatchDirOrient_1_0::theSOPTypeName,
        "FeE Match Dir Orient",
        SOP_FeE_MatchDirOrient_1_0::myConstructor,
        SOP_FeE_MatchDirOrient_1_0::buildTemplates(),
        1,
        2,
        nullptr,
        OP_FLAG_GENERATOR,
        nullptr,
        1,
        "Five elements Elf/Data/Orient");

    newOp->setIconName("SOP_matchaxis");
    table->addOperator(newOp);

}





class SOP_FeE_MatchDirOrient_1_0Verb : public SOP_NodeVerb
{
public:
    SOP_FeE_MatchDirOrient_1_0Verb() {}
    virtual ~SOP_FeE_MatchDirOrient_1_0Verb() {}

    virtual SOP_NodeParms *allocParms() const { return new SOP_FeE_MatchDirOrient_1_0Parms(); }
    virtual UT_StringHolder name() const { return SOP_FeE_MatchDirOrient_1_0::theSOPTypeName; }

    virtual CookMode cookMode(const SOP_NodeParms *parms) const { return COOK_INPLACE; }

    virtual void cook(const CookParms &cookparms) const;
    
    /// This static data member automatically registers
    /// this verb class at library load time.
    static const SOP_NodeVerb::Register<SOP_FeE_MatchDirOrient_1_0Verb> theVerb;
};

// The static member variable definition has to be outside the class definition.
// The declaration is inside the class.
const SOP_NodeVerb::Register<SOP_FeE_MatchDirOrient_1_0Verb> SOP_FeE_MatchDirOrient_1_0Verb::theVerb;

const SOP_NodeVerb *
SOP_FeE_MatchDirOrient_1_0::cookVerb() const 
{ 
    return SOP_FeE_MatchDirOrient_1_0Verb::theVerb.get();
}








static GA_AttributeOwner
sopAttribOwner(const SOP_FeE_MatchDirOrient_1_0Parms::DirAttribClass parmAttribClass)
{
    using namespace SOP_FeE_MatchDirOrient_1_0Enums;
    switch (parmAttribClass)
    {
    case DirAttribClass::PRIM:      return GA_ATTRIB_PRIMITIVE;  break;
    case DirAttribClass::POINT:     return GA_ATTRIB_POINT;      break;
    case DirAttribClass::VERTEX:    return GA_ATTRIB_VERTEX;     break;
    case DirAttribClass::DETAIL:    return GA_ATTRIB_DETAIL;     break;
    }
    UT_ASSERT_MSG(0, "Unhandled Class type!");
    return GA_ATTRIB_INVALID;
}


static GA_GroupType
sopGroupType(const SOP_FeE_MatchDirOrient_1_0Parms::GroupType parmGroupType)
{
    using namespace SOP_FeE_MatchDirOrient_1_0Enums;
    switch (parmGroupType)
    {
    case GroupType::GUESS:     return GA_GROUP_INVALID;    break;
    case GroupType::PRIM:      return GA_GROUP_PRIMITIVE;  break;
    case GroupType::POINT:     return GA_GROUP_POINT;      break;
    case GroupType::VERTEX:    return GA_GROUP_VERTEX;     break;
    case GroupType::EDGE:      return GA_GROUP_EDGE;       break;
    }
    UT_ASSERT_MSG(0, "Unhandled Group type!");
    return GA_GROUP_INVALID;
}


void
SOP_FeE_MatchDirOrient_1_0Verb::cook(const SOP_NodeVerb::CookParms &cookparms) const
{
    auto&& sopparms = cookparms.parms<SOP_FeE_MatchDirOrient_1_0Parms>();
    GA_Detail& outGeo0 = *cookparms.gdh().gdpNC();
    //auto sopcache = (SOP_FeE_MatchDirOrient_1_0Cache*)cookparms.cache();

    //const GA_Detail& inGeo0 = *cookparms.inputGeo(0);
    const GA_Detail* const inGeo1 = cookparms.inputGeo(1);

    //outGeo0.replaceWith(inGeo0);




    const GA_GroupType groupType = sopGroupType(sopparms.getGroupType());
    const GA_AttributeOwner attribOwner = sopAttribOwner(sopparms.getDirAttribClass());
    
    UT_AutoInterrupt boss("Processing");
    if (boss.wasInterrupted())
        return;

    
    GFE_MatchDirOrient matchDirOrient(outGeo0, inGeo1, cookparms);
    matchDirOrient.setComputeParm(
        sopparms.getConstDir(),
        sopparms.getOutDir(), sopparms.getAccurate(),
        sopparms.getKeepBearing(), sopparms.getReverseDir(), 
        sopparms.getSubscribeRatio(), sopparms.getMinGrainSize());
    
    matchDirOrient.groupSetter.setParm(sopparms.getReverseGroup());

    matchDirOrient.groupParser.setGroup(groupType, sopparms.getGroup());
    
    matchDirOrient.getOutAttribArray().appends(attribOwner, sopparms.getDirAttrib());
    
    if (sopparms.getUseDirRefAttrib())
        matchDirOrient.appendInRef0Attribs(attribOwner, sopparms.getDirRefAttrib());
    if (sopparms.getOutDirReversedGroup())
        matchDirOrient.findOrCreateGroup(GFE_Type::attributeOwner_groupType(attribOwner), sopparms.getDirReversedGroupName());
    

    matchDirOrient.computeAndBumpDataId();
    matchDirOrient.visualizeOutGroup();
}
