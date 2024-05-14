
//#define UT_ASSERT_LEVEL 3
#include "SOP_FeE_DirCross_2_0.h"


#include "SOP_FeE_DirCross_2_0.proto.h"

#include "GA/GA_Detail.h"
#include "PRM/PRM_TemplateBuilder.h"
#include "UT/UT_Interrupt.h"
#include "UT/UT_DSOVersion.h"




#include "GFE/GFE_DirectionCross.h"




using namespace SOP_FeE_DirCross_2_0_Namespace;


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
            "prim"      "Prim"
            "point"     "Point"
            "vertex"    "Vertex"
            "detail"    "Detail"
        }
    }
    parm {
        name    "attribName"
        cppname "AttribName"
        label   "Attrib Name"
        type    string
        default { "N" }
    }
    //parm {
    //    name    "renameAttrib"
    //    cppname "RenameAttrib"
    //    label   "Rename Attrib"
    //    type    toggle
    //    nolabel
    //    joinnext
    //    default { "0" }
    //}
    //parm {
    //    name    "newAttribName"
    //    cppname "NewAttribName"
    //    label   "New Attrib Name"
    //    type    string
    //    default { "" }
    //    disablewhen "{ renameAttrib == 0 }"
    //}
    parm {
        name    "useUpAttrib"
        cppname "UseUpAttrib"
        label   "Use Up Attrib"
        type    toggle
        default { "0" }
    }
    parm {
        name    "inputUp"
        cppname "InputUp"
        label   "Input Up"
        type    float
        size    3
        default { "0" "1" "0" }
        hidewhen "{ useUpAttrib == 1 }"
        range   { 0 1 }
    }
    parm {
        name    "upAttribClass"
        cppname "UpAttribClass"
        label   "Up Attrib Class"
        type    ordinal
        default { "point" }
        menu {
            "prim"      "Prim"
            "point"     "Point"
            "vertex"    "Vertex"
            "detail"    "Detail"
        }
        hidewhen "{ useUpAttrib == 0 }"
    }
    parm {
        name    "upAttrib"
        cppname "UpAttrib"
        label   "Up Attrib"
        type    string
        default { "up" }
        hidewhen "{ useUpAttrib == 0 }"
    }
    parm {
        name    "double"
        cppname "Double"
        label   "Double"
        type    toggle
        default { "0" }
    }
    parm {
        name    "crossOrig"
        cppname "CrossOrig"
        label   "Cross Original Direction"
        type    toggle
        default { "1" }
        disablewhen "{ double == 0 }"
    }
    parm {
        name    "normalize"
        cppname "Normalize"
        label   "Normalize"
        type    toggle
        default { "0" }
    }
    parm {
        name    "keepLength"
        cppname "KeepLength"
        label   "Keep Length"
        type    toggle
        default { "0" }
        disablewhen "{ normalize == 1 }"
    }
    parm {
        name    "reverseDir"
        cppname "ReverseDir"
        label   "Reverse Dir"
        type    toggle
        default { "0" }
    }


    parm {
        name    "delUpAttrib"
        cppname "DelUpAttrib"
        label   "Delete up Attrib"
        type    toggle
        default { "0" }
        disablewhen "{ useUpAttrib == 1 }"
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
SOP_FeE_DirCross_2_0::buildTemplates()
{
    static PRM_TemplateBuilder templ("SOP_FeE_DirCross_2_0.C"_sh, theDsFile);
    if (templ.justBuilt())
    {
        templ.setChoiceListPtr("group"_sh, &SOP_Node::groupMenu);
        templ.setChoiceListPtr("upAttrib"_sh, &SOP_Node::allAttribReplaceMenu);
    }
    return templ.templates();
}

const UT_StringHolder SOP_FeE_DirCross_2_0::theSOPTypeName("FeE::dirCross::2.0"_sh);

void
newSopOperator(OP_OperatorTable* table)
{
    OP_Operator* newOp = new OP_Operator(
        SOP_FeE_DirCross_2_0::theSOPTypeName,
        "FeE Direction Cross",
        SOP_FeE_DirCross_2_0::myConstructor,
        SOP_FeE_DirCross_2_0::buildTemplates(),
        1,
        1,
        nullptr,
        OP_FLAG_GENERATOR,
        nullptr,
        1,
        "Five elements Elf/Data/Orient");

    newOp->setIconName("VOP_cross");
    table->addOperator(newOp);

}





class SOP_FeE_DirCross_2_0Verb : public SOP_NodeVerb
{
public:
    SOP_FeE_DirCross_2_0Verb() {}
    virtual ~SOP_FeE_DirCross_2_0Verb() {}

    virtual SOP_NodeParms *allocParms() const { return new SOP_FeE_DirCross_2_0Parms(); }
    virtual UT_StringHolder name() const { return SOP_FeE_DirCross_2_0::theSOPTypeName; }

    virtual CookMode cookMode(const SOP_NodeParms *parms) const { return COOK_INPLACE; }

    virtual void cook(const CookParms &cookparms) const;
    
    /// This static data member automatically registers
    /// this verb class at library load time.
    static const SOP_NodeVerb::Register<SOP_FeE_DirCross_2_0Verb> theVerb;
};

// The static member variable definition has to be outside the class definition.
// The declaration is inside the class.
const SOP_NodeVerb::Register<SOP_FeE_DirCross_2_0Verb> SOP_FeE_DirCross_2_0Verb::theVerb;

const SOP_NodeVerb *
SOP_FeE_DirCross_2_0::cookVerb() const 
{ 
    return SOP_FeE_DirCross_2_0Verb::theVerb.get();
}





static GA_GroupType
sopGroupType(const SOP_FeE_DirCross_2_0Parms::GroupType parmGroupType)
{
    using namespace SOP_FeE_DirCross_2_0Enums;
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
sopAttribOwner(const SOP_FeE_DirCross_2_0Parms::AttribClass parmAttribClass)
{
    using namespace SOP_FeE_DirCross_2_0Enums;
    switch (parmAttribClass)
    {
    case AttribClass::PRIM:      return GA_ATTRIB_PRIMITIVE;  break;
    case AttribClass::POINT:     return GA_ATTRIB_POINT;      break;
    case AttribClass::VERTEX:    return GA_ATTRIB_VERTEX;     break;
    case AttribClass::DETAIL:    return GA_ATTRIB_DETAIL;     break;
    }
    UT_ASSERT_MSG(0, "Unhandled Class type!");
    return GA_ATTRIB_INVALID;
}

static GA_AttributeOwner
sopAttribOwner(const SOP_FeE_DirCross_2_0Parms::UpAttribClass parmAttribClass)
{
    using namespace SOP_FeE_DirCross_2_0Enums;
    switch (parmAttribClass)
    {
    case UpAttribClass::PRIM:      return GA_ATTRIB_PRIMITIVE;  break;
    case UpAttribClass::POINT:     return GA_ATTRIB_POINT;      break;
    case UpAttribClass::VERTEX:    return GA_ATTRIB_VERTEX;     break;
    case UpAttribClass::DETAIL:    return GA_ATTRIB_DETAIL;     break;
    }
    UT_ASSERT_MSG(0, "Unhandled Class type!");
    return GA_ATTRIB_INVALID;
}


void
SOP_FeE_DirCross_2_0Verb::cook(const SOP_NodeVerb::CookParms &cookparms) const
{
    auto&& sopparms = cookparms.parms<SOP_FeE_DirCross_2_0Parms>();
    GA_Detail& outGeo0 = *cookparms.gdh().gdpNC();
    //auto sopcache = (SOP_FeE_DirCross_2_0Cache*)cookparms.cache();

    //const GA_Detail& inGeo0 = *cookparms.inputGeo(0);

    //outGeo0.replaceWith(inGeo0);



    const GA_AttributeOwner upAttribClass = sopAttribOwner(sopparms.getUpAttribClass());
    const GA_AttributeOwner attribClass = sopAttribOwner(sopparms.getAttribClass());
    const GA_GroupType groupType = sopGroupType(sopparms.getGroupType());


    UT_AutoInterrupt boss("Processing");
    if (boss.wasInterrupted())
        return;
    
/*
    GFE_DirCross dirCross(geo, cookparms);
    dirCross.findOrCreateTuple(true, GA_ATTRIB_POINT);
    dirCross.compute();
*/
    
    GFE_DirCross dirCross(outGeo0, cookparms);
    dirCross.setComputeParm(sopparms.getInputUp(),
        sopparms.getNormalize(),sopparms.getKeepLength(),sopparms.getReverseDir(),  sopparms.getDelUpAttrib(),
        sopparms.getSubscribeRatio(), sopparms.getMinGrainSize());

    if (sopparms.getUseUpAttrib())
        dirCross.setUpAttrib(upAttribClass, sopparms.getUpAttrib());
    
    //if (sopparms.getRenameAttrib())
    //    dirCross.newAttribNames = sopparms.getNewAttribName();
        
    dirCross.delUpAttrib = sopparms.getDelUpAttrib();
    
    dirCross.groupParser.setGroup(groupType, sopparms.getGroup());
    
    dirCross.getOutAttribArray().appends(attribClass, sopparms.getAttribName());

    dirCross.computeAndBumpDataId();
}


