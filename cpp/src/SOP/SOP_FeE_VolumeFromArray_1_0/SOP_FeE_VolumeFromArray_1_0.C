
//#define UT_ASSERT_LEVEL 3
#include "SOP_FeE_VolumeFromArray_1_0.h"


#include "SOP_FeE_VolumeFromArray_1_0.proto.h"

#include "GA/GA_Detail.h"
#include "PRM/PRM_TemplateBuilder.h"
#include "UT/UT_Interrupt.h"
#include "UT/UT_DSOVersion.h"




#include "GFE/GFE_VolumeFromArray.h"




using namespace SOP_FeE_VolumeFromArray_1_0_Namespace;


static const char *theDsFile = R"THEDSFILE(
{
    name        parameters
    parm {
        name    "arrayAttrib"
        cppname "ArrayAttrib"
        label   "Array Attribute"
        type    string
        default { "voxeldata" }
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

PRM_Template*
SOP_FeE_VolumeFromArray_1_0::buildTemplates()
{
    static PRM_TemplateBuilder templ("SOP_FeE_VolumeFromArray_1_0.C"_sh, theDsFile);
    if (templ.justBuilt())
    {
        templ.setChoiceListPtr("group"_sh, &SOP_Node::groupMenu);
        
        templ.setChoiceListPtr("arrayAttrib"_sh, &SOP_Node::detailAttribMenu);
    }
    return templ.templates();
}

const UT_StringHolder SOP_FeE_VolumeFromArray_1_0::theSOPTypeName("FeE::volumeFromArray::1.0"_sh);

void
newSopOperator(OP_OperatorTable* table)
{
    OP_Operator* newOp = new OP_Operator(
        SOP_FeE_VolumeFromArray_1_0::theSOPTypeName,
        "FeE Volume from Array",
        SOP_FeE_VolumeFromArray_1_0::myConstructor,
        SOP_FeE_VolumeFromArray_1_0::buildTemplates(),
        1,
        2,
        nullptr,
        OP_FLAG_GENERATOR,
        nullptr,
        1,
        "Five elements Elf/Filter/Volume");

    newOp->setIconName("SOP_volumerasterize");
    table->addOperator(newOp);

}





class SOP_FeE_VolumeFromArray_1_0Verb : public SOP_NodeVerb
{
public:
    SOP_FeE_VolumeFromArray_1_0Verb() {}
    virtual ~SOP_FeE_VolumeFromArray_1_0Verb() {}

    virtual SOP_NodeParms *allocParms() const { return new SOP_FeE_VolumeFromArray_1_0Parms(); }
    virtual UT_StringHolder name() const { return SOP_FeE_VolumeFromArray_1_0::theSOPTypeName; }

    virtual CookMode cookMode(const SOP_NodeParms *parms) const { return COOK_INPLACE; }

    virtual void cook(const CookParms &cookparms) const;
    
    /// This static data member automatically registers
    /// this verb class at library load time.
    static const SOP_NodeVerb::Register<SOP_FeE_VolumeFromArray_1_0Verb> theVerb;
};

// The static member variable definition has to be outside the class definition.
// The declaration is inside the class.
const SOP_NodeVerb::Register<SOP_FeE_VolumeFromArray_1_0Verb> SOP_FeE_VolumeFromArray_1_0Verb::theVerb;

const SOP_NodeVerb *
SOP_FeE_VolumeFromArray_1_0::cookVerb() const 
{ 
    return SOP_FeE_VolumeFromArray_1_0Verb::theVerb.get();
}





void
SOP_FeE_VolumeFromArray_1_0Verb::cook(const SOP_NodeVerb::CookParms &cookparms) const
{
    auto&& sopparms = cookparms.parms<SOP_FeE_VolumeFromArray_1_0Parms>();
    GA_Detail& outGeo0 = *cookparms.gdh().gdpNC();
    //auto sopcache = (SOP_FeE_VolumeFromArray_1_0Cache*)cookparms.cache();

    //const GA_Detail& inGeo0 = *cookparms.inputGeo(0);
    const GA_Detail* const inGeo1 = cookparms.inputGeo(1);

    //outGeo0.replaceWith(inGeo0);

    
    UT_AutoInterrupt boss("Processing");
    if (boss.wasInterrupted())
        return;

    
    GFE_VolumeFromArray volumeFromArray(outGeo0, inGeo1, cookparms);
    if (inGeo1)
        volumeFromArray.getRef0AttribArray().appends(GA_ATTRIB_DETAIL, sopparms.getArrayAttrib());
    else
        volumeFromArray.getInAttribArray().appends(GA_ATTRIB_DETAIL, sopparms.getArrayAttrib());
    
    volumeFromArray.setComputeParm(
        sopparms.getSubscribeRatio(), sopparms.getMinGrainSize());
    
    volumeFromArray.computeAndBumpDataId();

    

}


