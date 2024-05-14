
//#define UT_ASSERT_LEVEL 3
#include "SOP_FeE_SplitPoint_1_0.h"

#include "SOP_FeE_SplitPoint_1_0.proto.h"


#include "GA/GA_Detail.h"
#include "PRM/PRM_TemplateBuilder.h"
#include "UT/UT_Interrupt.h"
#include "UT/UT_DSOVersion.h"

#include "PRM/PRM_ChoiceList.h"

#include "GFE/GFE_SplitPoint.h"

using namespace SOP_FeE_SplitPoint_1_0_Namespace;


static const char *theDsFile = R"THEDSFILE(
{
    name	parameters
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
        default { "0" }
        menu {
            "guess"     "Guess from Group"
            "prim"      "Primitive"
            "point"     "Point"
            "vertex"    "Vertex"
        }
    }
    parm {
        name    "primSplitAttrib"
        cppname "PrimSplitAttrib"
        label   "Primitive Split Attrib"
        type    string
        default { "" }
    }
    parm {
        name    "vertexSplitAttrib"
        cppname "VertexSplitAttrib"
        label   "Vertex Split Attrib"
        type    string
        default { "" }
    }
    parm {
        name    "primSplitGroup"
        cppname "PrimSplitGroup"
        label   "Primitive Split Group"
        type    string
        default { "" }
    }
    parm {
        name    "vertexSplitGroup"
        cppname "VertexSplitGroup"
        label   "Vertex Split Group"
        type    string
        default { "" }
    }
    parm {
        name    "splitByAttribTol"
        cppname "SplitByAttribTol"
        label   "Split by Attrib Tolerance"
        type    log
        default { "0.001" }
        range   { 0! 1 }
    }
    parm {
        name    "promoteSplitAttrib"
        cppname "promoteSplitAttrib"
        label   "Promote Split Attribute"
        type    toggle
        default { "0" }
    }

    parm {
        name    "keepEdgeGroup"
        cppname "KeepEdgeGroup"
        label   "Keep Edge Group"
        type    string
        default { "*" }
    }
    parm {
        name    "outEdgeGroupAsVertex"
        cppname "OutEdgeGroupAsVertex"
        label   "Out Edge Group as Vertex"
        type    toggle
        default { "0" }
    }


    // parm {
    //     name    "subscribeRatio"
    //     cppname "SubscribeRatio"
    //     label   "Subscribe Ratio"
    //     type    integer
    //     default { 64 }
    //     range   { 0! 256 }
    // }
    // parm {
    //     name    "minGrainSize"
    //     cppname "MinGrainSize"
    //     label   "Min Grain Size"
    //     type    intlog
    //     default { 64 }
    //     range   { 0! 2048 }
    // }
}
)THEDSFILE";



static GA_GroupType
sopGroupType(const SOP_FeE_SplitPoint_1_0Parms::GroupType parmGroupType)
{
    using namespace SOP_FeE_SplitPoint_1_0Enums;
    switch (parmGroupType)
    {
    case GroupType::GUESS:      return GA_GROUP_INVALID;    break;
    case GroupType::PRIM:       return GA_GROUP_PRIMITIVE;  break;
    case GroupType::POINT:      return GA_GROUP_POINT;      break;
    case GroupType::VERTEX:     return GA_GROUP_VERTEX;     break;
    }
    UT_ASSERT_MSG(0, "Unhandled group type!");
    return GA_GROUP_INVALID;
}

// static GA_AttributeOwner
// sopSplitAttribOwner(SOP_FeE_SplitPoint_1_0Parms::SplitAttribClass splitAttribClass)
// {
//     using namespace SOP_FeE_SplitPoint_1_0Enums;
//     switch (splitAttribClass)
//     {
//     case SplitAttribClass::GUESS:      return GA_ATTRIB_INVALID;    break;
//     case SplitAttribClass::PRIM:       return GA_ATTRIB_PRIMITIVE;  break;
//     case SplitAttribClass::VERTEX:     return GA_ATTRIB_VERTEX;     break;
//     }
//     UT_ASSERT_MSG(0, "Unhandled Split Attrib Class!");
//     return GA_ATTRIB_INVALID;
// }



OP_ERROR
SOP_FeE_SplitPoint_1_0::cookInputGroups(OP_Context &context, int alone)
{
    UT_ASSERT(alone);

    GA_GroupType groupType = sopGroupType(
        (SOP_FeE_SplitPoint_1_0Enums::GroupType)evalInt("groupType"_sh, 0, context.getTime()));

    const GA_Group *group;
    cookInputAllGroups(
        context,
        group,
        alone != 0,
        true,   // do_selection
        0,      // group parm index
        1,      // group type parm index
        groupType,
        true,   // allow_reference
        false   // is_default_prim
        );
    return error();
}



void
SOP_FeE_SplitPoint_1_0::buildAttribMenu(
    void* data, PRM_Name* entries, int size,
    const PRM_SpareData*, const PRM_Parm*)
{
    SOP_FeE_SplitPoint_1_0* sop = (SOP_FeE_SplitPoint_1_0*)data;
    if (!sop)
        return;

    const int input = 0; // input from which to obtain attribs

    sop->fillAttribNameMenu(entries, size, GA_ATTRIB_VERTEX, input);
}
static PRM_ChoiceList splitPointAttribMenu(
    (PRM_ChoiceListType)(PRM_CHOICELIST_TOGGLE),
    SOP_FeE_SplitPoint_1_0::buildAttribMenu);




PRM_Template*
SOP_FeE_SplitPoint_1_0::buildTemplates()
{
    static PRM_TemplateBuilder templ("SOP_FeE_SplitPoint_1_0.C"_sh, theDsFile);
    if (templ.justBuilt())
    {
        templ.setChoiceListPtr("group", &SOP_Node::allGroupMenu);
        templ.setChoiceListPtr("primSplitAttrib",   &SOP_Node::primAttribMenu);
        templ.setChoiceListPtr("vertexSplitAttrib", &SOP_Node::vertexAttribMenu);
        templ.setChoiceListPtr("primSplitGroup",    &SOP_Node::primGroupMenu);
        templ.setChoiceListPtr("vertexSplitGroup",  &SOP_Node::vertexNamedGroupMenu);
        templ.setChoiceListPtr("keepEdgeGroup",     &SOP_Node::edgeNamedGroupMenu);
        
        //templ.setChoiceListPtr("vertexSplitAttrib", &splitPointAttribMenu);
    }
    return templ.templates();
}





const UT_StringHolder SOP_FeE_SplitPoint_1_0::theSOPTypeName("FeE::splitPoint::1.0"_sh);

void
newSopOperator(OP_OperatorTable* table)
{
    OP_Operator* newOp = new OP_Operator(
        SOP_FeE_SplitPoint_1_0::theSOPTypeName,
        "FeE Split Point",
        SOP_FeE_SplitPoint_1_0::myConstructor,
        SOP_FeE_SplitPoint_1_0::buildTemplates(),
        1,
        1,
        nullptr,
        OP_FLAG_GENERATOR,
        nullptr,
        1,
        "Five elements Elf/Topo/Primitive");

    newOp->setIconName("SOP_splitpoints");
    table->addOperator(newOp);
}




class SOP_FeE_SplitPoint_1_0Verb : public SOP_NodeVerb
{
public:
    SOP_FeE_SplitPoint_1_0Verb() {}
    virtual ~SOP_FeE_SplitPoint_1_0Verb() {}

    virtual SOP_NodeParms *allocParms() const { return new SOP_FeE_SplitPoint_1_0Parms(); }
    virtual UT_StringHolder name() const { return SOP_FeE_SplitPoint_1_0::theSOPTypeName; }

    virtual CookMode cookMode(const SOP_NodeParms *parms) const { return COOK_INPLACE; }

    virtual void cook(const CookParms &cookparms) const;
    
    static const SOP_NodeVerb::Register<SOP_FeE_SplitPoint_1_0Verb> theVerb;
};

const SOP_NodeVerb::Register<SOP_FeE_SplitPoint_1_0Verb> SOP_FeE_SplitPoint_1_0Verb::theVerb;

const SOP_NodeVerb *
SOP_FeE_SplitPoint_1_0::cookVerb() const 
{ 
    return SOP_FeE_SplitPoint_1_0Verb::theVerb.get();
}






void
SOP_FeE_SplitPoint_1_0Verb::cook(const SOP_NodeVerb::CookParms& cookparms) const
{
    auto&& sopparms = cookparms.parms<SOP_FeE_SplitPoint_1_0Parms>();
    GA_Detail& outGeo0 = *cookparms.gdh().gdpNC();
    //auto sopcache = (SOP_FeE_SplitPoint_1_0Cache*)cookparms.cache();

    //const GA_Detail& inGeo0 = *cookparms.inputGeo(0);
    //outGeo0.replaceWith(inGeo0);

    
    const GA_GroupType groupType = sopGroupType(sopparms.getGroupType());
    //const GA_AttributeOwner splitAttribOwner = sopSplitAttribOwner(sopparms.getSplitAttribClass());

    
    UT_AutoInterrupt boss("Processing");
    if (boss.wasInterrupted())
        return;


    
    GFE_SplitPoint splitPoint(outGeo0, cookparms);
    splitPoint.groupParser.setGroup(groupType, sopparms.getGroup());
    
    //if (sopparms.getSplitByAttrib())
    //{
        splitPoint.getInAttribArray().oappendPrims(sopparms.getPrimSplitAttrib());
        splitPoint.getInAttribArray().oappendVertices(sopparms.getVertexSplitAttrib());
        splitPoint.getInGroupArray() .oappendPrims(sopparms.getPrimSplitGroup());
        splitPoint.getInGroupArray() .oappendVertices(sopparms.getVertexSplitGroup());
    
        //splitPoint.splitAttribTol = sopparms.getSplitByAttribTol();
        //splitPoint.promoteAttrib = sopparms.getPromoteSplitAttrib();
    //}
    
    splitPoint.getOutGroupArray().appendEdges(sopparms.getKeepEdgeGroup());
    
    splitPoint.setComputeParm(sopparms.getSplitByAttribTol(), sopparms.getPromoteSplitAttrib()
        //, sopparms.getSubscribeRatio(), sopparms.getMinGrainSize()
        );
    splitPoint.outEdgeGroupAsVertex = sopparms.getOutEdgeGroupAsVertex();
    
    splitPoint.computeAndBumpDataId();
    splitPoint.visualizeOutGroup();
    
}

