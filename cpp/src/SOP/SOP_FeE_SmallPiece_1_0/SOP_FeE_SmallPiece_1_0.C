
//#define UT_ASSERT_LEVEL 3
#include "SOP_FeE_SmallPiece_1_0.h"


#include "SOP_FeE_SmallPiece_1_0.proto.h"

#include "GA/GA_Detail.h"
#include "PRM/PRM_TemplateBuilder.h"
#include "UT/UT_Interrupt.h"
#include "UT/UT_DSOVersion.h"


#include "GFE/GFE_SmallPiece.h"

using namespace SOP_FeE_SmallPiece_1_0_Namespace;

// #define SOP_FeE_SmallPiece_1_0_PARM_SPECILIZATION(Name, Label)  \
// R"THEDSFILE(                                                    
//         parm {                                                  
//             name    "check%s" 
//             label   "Check%s"                           
//             type    toggle                                      
//             nolabel                                             
//             joinnext                                            
//             default { "0" }                                   
//         }                                                       
//         parm {                                                  
//             name    "threshold%s"                         
//             label   "Threshold %s"                        
//             type    log                                         
//             default { "0.1" }                                 
//             disablewhen "{ check%s == 0 }"          
//             range   { 0.01 100 }                                
//         }                                                       
//         parm {                                                  
//             name    "threshold%sAttrib"                   
//             label   "Threshold %s Attrib"                 
//             type    string                                      
//             default { "" }                                    
//             disablewhen "{ check%s == 0 }"                
//         }                                                   
// )THEDSFILE"                                                     


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
        default { "guess" }
        menu {
            "guess"     "Guess from Group"
            "prim"      "Primitive"
            "point"     "Point"
            "vertex"    "Vertex"
            "edge"      "Edge"
        }
    }

)THEDSFILE"

//SOP_FeE_SmallPiece_1_0_PARM_SPECILIZATION(OneOfXYZSize, One of XYZ Size)

R"THEDSFILE(



    parm {
        name    "checkAreaPerimeterRatio"
        cppname "CheckAreaPerimeterRatio"
        label   "Check Area Perimeter Ratio"
        type    toggle
        nolabel
        joinnext
        default { "0" }
    }
    parm {
        name    "thresholdAreaPerimeterRatio"
        cppname "ThresholdAreaPerimeterRatio"
        label   "Threshold Area Perimeter Ratio"
        type    log
        default { "0.1" }
        disablewhen "{ checkAreaPerimeterRatio == 0 }"
        range   { 0.01 100 }
    }
    parm {
        name    "thresholdAttribAreaPerimeterRatio"
        cppname "ThresholdAttribAreaPerimeterRatio"
        label   "Attrib Area Perimeter Ratio"
        type    string
        default { "" }
        disablewhen "{ checkAreaPerimeterRatio == 0 }"
    }



    parm {
        name    "checkPerimeter"
        cppname "CheckPerimeter"
        label   "Check Perimeter"
        type    toggle
        nolabel
        joinnext
        default { "0" }
    }
    parm {
        name    "thresholdPerimeter"
        cppname "ThresholdPerimeter"
        label   "Threshold Perimeter"
        type    log
        default { "0.1" }
        disablewhen "{ checkPerimeter == 0 }"
        range   { 0.01 100 }
    }
    parm {
        name    "thresholdAttribPerimeter"
        cppname "ThresholdAttribPerimeter"
        label   "Attrib Perimeter"
        type    string
        default { "" }
        disablewhen "{ checkPerimeter == 0 }"
    }




    parm {
        name    "checkArea"
        cppname "CheckArea"
        label   "Check Area"
        type    toggle
        nolabel
        joinnext
        default { "0" }
    }
    parm {
        name    "thresholdArea"
        cppname "ThresholdArea"
        label   "Threshold Area"
        type    log
        default { "0.1" }
        disablewhen "{ checkArea == 0 }"
        range   { 0.01 100 }
    }
    parm {
        name    "thresholdAttribArea"
        cppname "ThresholdAttribArea"
        label   "Attrib Area"
        type    string
        default { "" }
        disablewhen "{ checkArea == 0 }"
    }



    parm {
        name    "checkVolume"
        cppname "CheckVolume"
        label   "Check Volume"
        type    toggle
        nolabel
        joinnext
        default { "0" }
    }
    parm {
        name    "thresholdVolume"
        cppname "ThresholdVolume"
        label   "Threshold Volume"
        type    log
        default { "0.1" }
        disablewhen "{ checkVolume == 0 }"
        range   { 0.01 100 }
    }
    parm {
        name    "thresholdAttribVolume"
        cppname "ThresholdAttribVolume"
        label   "Attrib Volume"
        type    string
        default { "" }
        disablewhen "{ checkVolume == 0 }"
    }






    parm {
        name    "checkOneOfXYZSize"
        cppname "CheckOneOfXYZSize"
        label   "Check One of XYZ Size"
        type    toggle
        nolabel
        joinnext
        default { "0" }
    }
    parm {
        name    "thresholdOneOfXYZSize"
        cppname "ThresholdOneOfXYZSize"
        label   "Threshold One of XYZ Size"
        type    log
        default { "0.1" }
        disablewhen "{ checkOneOfXYZSize == 0 }"
        range   { 0.01 100 }
    }
    parm {
        name    "thresholdAttribOneOfXYZSize"
        cppname "ThresholdAttribOneOfXYZSize"
        label   "Attrib One of XYZ Size"
        type    string
        default { "" }
        disablewhen "{ checkOneOfXYZSize == 0 }"
    }




    parm {
        name    "checkAllXYZSize"
        cppname "CheckAllXYZSize"
        label   "Check All XYZ Size"
        type    toggle
        nolabel
        joinnext
        default { "0" }
    }
    parm {
        name    "thresholdAllXYZSize"
        cppname "ThresholdAllXYZSize"
        label   "Threshold All XYZ Size"
        type    log
        default { "0.1" }
        disablewhen "{ checkAllXYZSize == 0 }"
        range   { 0.01 100 }
    }
    parm {
        name    "thresholdAttribAllXYZSize"
        cppname "ThresholdAttribAllXYZSize"
        label   "Attrib All XYZ Size"
        type    string
        default { "" }
        disablewhen "{ checkAllXYZSize == 0 }"
    }





    parm {
        name    "checkXYZSizeSum"
        cppname "CheckXYZSizeSum"
        label   "Check XYZ Size Sum"
        type    toggle
        nolabel
        joinnext
        default { "0" }
    }
    parm {
        name    "thresholdXYZSizeSum"
        cppname "ThresholdXYZSizeSum"
        label   "Threshold XYZ Size Sum"
        type    log
        default { "0.1" }
        disablewhen "{ checkXYZSizeSum == 0 }"
        range   { 0.01 100 }
    }
    parm {
        name    "thresholdAttribXYZSizeSum"
        cppname "ThresholdAttribXYZSizeSum"
        label   "Attrib XYZ Size Sum"
        type    string
        default { "" }
        disablewhen "{ checkXYZSizeSum == 0 }"
    }



)THEDSFILE"
R"THEDSFILE(



    parm {
        name    "numConditionMatch"
        cppname "NumConditionMatch"
        label   "num Condition Match"
        type    integer
        default { "1" }
        range   { 1! 6 }
    }

    parm {
        name    "smallPieceGroupName"
        cppname "SmallPieceGroupName"
        label   "Small Piece Group Name"
        type    string
        default { "small" }
        disablewhen "{ delElem == 1 delGroup == 1 }"
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
        name    "delGroup"
        cppname "DelGroup"
        label   "Delete Group"
        type    toggle
        default { "1" }
        disablewhen "{ delElem == 0 }"
    }
    parm {
        name    "delUnusedPoint"
        cppname "DelUnusedPoint"
        label   "Delete Unused Point"
        type    toggle
        default { "1" }
        disablewhen "{ delElem == 0 }"
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
        default { 1024 }
        range   { 0! 2048 }
    }
}
)THEDSFILE";

#ifdef SOP_FeE_SmallPiece_1_0_PARM_SPECILIZATION
#undef SOP_FeE_SmallPiece_1_0_PARM_SPECILIZATION
#endif




#define SOP_FeE_SmallPiece_1_0_PARM_SPECILIZATION(NAME)                                        \
        templ.setChoiceListPtr("thresholdAttrib"#NAME""_sh, &SOP_Node::primAttribReplaceMenu); \


PRM_Template*
SOP_FeE_SmallPiece_1_0::buildTemplates()
{
    static PRM_TemplateBuilder templ("SOP_FeE_SmallPiece_1_0.C"_sh, theDsFile);
    if (templ.justBuilt())
    {
        templ.setChoiceListPtr("group"_sh,               &SOP_Node::allGroupMenu);
        GFE_SmallPiece_SPECILIZATION_MULTI_CONDITION_FUNC(SOP_FeE_SmallPiece_1_0_PARM_SPECILIZATION)
        templ.setChoiceListPtr("smallPieceGroupName"_sh, &SOP_Node::primGroupMenu);
    }
    return templ.templates();
}


#undef SOP_FeE_SmallPiece_1_0_PARM_SPECILIZATION


const UT_StringHolder SOP_FeE_SmallPiece_1_0::theSOPTypeName("FeE::smallPiece::1.0"_sh);

void
newSopOperator(OP_OperatorTable* table)
{
    OP_Operator* newOp = new OP_Operator(
        SOP_FeE_SmallPiece_1_0::theSOPTypeName,
        "FeE Small Piece",
        SOP_FeE_SmallPiece_1_0::myConstructor,
        SOP_FeE_SmallPiece_1_0::buildTemplates(),
        1,
        1,
        nullptr,
        OP_FLAG_GENERATOR,
        nullptr,
        1,
        "Five elements Elf/Group");

    newOp->setIconName("SOP_measure-2.0");
    table->addOperator(newOp);
}


//class SOP_FeE_SmallPiece_1_0Cache : public SOP_NodeCache
//{
//public:
//    SOP_FeE_SmallPiece_1_0Cache() : SOP_NodeCache(),
//        myPrevOutputDetailID(-1)
//    {}
//    ~SOP_FeE_SmallPiece_1_0Cache() override {}
//
//    int64 myPrevOutputDetailID;
//};


class SOP_FeE_SmallPiece_1_0Verb : public SOP_NodeVerb
{
public:
    SOP_FeE_SmallPiece_1_0Verb() {}
    virtual ~SOP_FeE_SmallPiece_1_0Verb() {}

    virtual SOP_NodeParms *allocParms() const { return new SOP_FeE_SmallPiece_1_0Parms(); }
    //virtual SOP_NodeCache* allocCache() const { return new SOP_FeE_SmallPiece_1_0Cache(); }
    virtual UT_StringHolder name() const { return SOP_FeE_SmallPiece_1_0::theSOPTypeName; }

    virtual CookMode cookMode(const SOP_NodeParms *parms) const { return COOK_INPLACE; }

    virtual void cook(const CookParms &cookparms) const;

    /// This static data member automatically registers
    /// this verb class at library ldir0d time.
    static const SOP_NodeVerb::Register<SOP_FeE_SmallPiece_1_0Verb> theVerb;
};

// The static member variable definition has to be outside the class definition.
// The declaration is inside the class.
const SOP_NodeVerb::Register<SOP_FeE_SmallPiece_1_0Verb> SOP_FeE_SmallPiece_1_0Verb::theVerb;

const SOP_NodeVerb *
SOP_FeE_SmallPiece_1_0::cookVerb() const 
{ 
    return SOP_FeE_SmallPiece_1_0Verb::theVerb.get();
}






static GA_GroupType
sopGroupType(const SOP_FeE_SmallPiece_1_0Parms::GroupType parmGroupType)
{
    using namespace SOP_FeE_SmallPiece_1_0Enums;
    switch (parmGroupType)
    {
    case GroupType::GUESS:     return GA_GROUP_INVALID;    break;
    case GroupType::PRIM:      return GA_GROUP_PRIMITIVE;  break;
    case GroupType::POINT:     return GA_GROUP_POINT;      break;
    case GroupType::VERTEX:    return GA_GROUP_VERTEX;     break;
    case GroupType::EDGE:      return GA_GROUP_EDGE;       break;
    }
    UT_ASSERT_MSG(0, "Unhandled Group Type!");
    return GA_GROUP_INVALID;
}


void
SOP_FeE_SmallPiece_1_0Verb::cook(const SOP_NodeVerb::CookParms &cookparms) const
{
    auto &&sopparms = cookparms.parms<SOP_FeE_SmallPiece_1_0Parms>();
    GA_Detail& outGeo0 = *cookparms.gdh().gdpNC();
    //auto sopcache = (SOP_FeE_SmallPiece_1_0Cache*)cookparms.cache();

    //const GA_Detail& inGeo0 = *cookparms.inputGeo(0);
    //outGeo0.replaceWith(inGeo0);


    const GA_GroupType groupType = sopGroupType(sopparms.getGroupType());

    UT_AutoInterrupt boss("Processing");
    if (boss.wasInterrupted())
        return;


    GFE_SmallPiece smallPiece(outGeo0, cookparms);
    smallPiece.setComputeParm(sopparms.getNumConditionMatch(),
        sopparms.getSubscribeRatio(), sopparms.getMinGrainSize());
    
    smallPiece.findOrCreatePrimitiveGroup(sopparms.getDelElem() && sopparms.getDelGroup(), sopparms.getSmallPieceGroupName());
    smallPiece.groupSetter.setParm(sopparms.getReverseGroup());
    
    smallPiece.doDelGroupElement = sopparms.getDelElem();
    smallPiece.delUnusedPoint = sopparms.getDelUnusedPoint();
    
    smallPiece.groupParser.setGroup(groupType, sopparms.getGroup());

    
#define SOP_FeE_SmallPiece_1_0_SPECILIZATION(Name)                                                          \
        if (sopparms.getCheck##Name())                                                                      \
            smallPiece.setCheck##Name(sopparms.getThreshold##Name(), sopparms.getThresholdAttrib##Name());  \

    
    GFE_SmallPiece_SPECILIZATION_MULTI_CONDITION_FUNC(SOP_FeE_SmallPiece_1_0_SPECILIZATION)
    
#undef SOP_FeE_SmallPiece_1_0_SPECILIZATION
    
    smallPiece.computeAndBumpDataId();
    smallPiece.delOrVisualizeGroup();
    
}

