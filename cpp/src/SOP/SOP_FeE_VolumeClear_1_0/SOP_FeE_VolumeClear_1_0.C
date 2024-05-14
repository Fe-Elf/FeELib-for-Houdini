
//#define UT_ASSERT_LEVEL 3
#include "SOP_FeE_VolumeClear_1_0.h"
#include "SOP_FeE_VolumeClear_1_0.proto.h"


#include "GA/GA_Detail.h"
#include "PRM/PRM_TemplateBuilder.h"
#include "UT/UT_Interrupt.h"
#include "UT/UT_DSOVersion.h"


#include "GFE/GFE_VolumeClear.h"


using namespace SOP_FeE_VolumeClear_1_0_Namespace;



static const char* theDsFile = R"THEDSFILE(
{
    name        parameters
    multiparm {
        name    "numVolumeClear"
        cppname "NumVolumeClear"
        label   "Number of Volume Clear"
        default 1

        parm {
            name    "name#"
            cppname "Name#"
            label   "Name #"
            type    string
            default { "" }
        }
        parm {
            name    "value#"
            cppname "Value#"
            label   "Value #"
            type    float
            default { "0" }
            range   { -1 1 }
        }
    }

    parm {
        name    "kernel"
        cppname "Kernel"
        label   "Kernel"
        type    integer
        default { 0 }
        range   { 0! 1! }
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
SOP_FeE_VolumeClear_1_0::buildTemplates()
{
    static PRM_TemplateBuilder templ("SOP_FeE_VolumeClear_1_0.C"_sh, theDsFile);
    if (templ.justBuilt())
    {
        templ.setChoiceListPtr("name#"_sh, &SOP_Node::namedVolumesMenu);
    }
    return templ.templates();
}

const UT_StringHolder SOP_FeE_VolumeClear_1_0::theSOPTypeName("FeE::volumeClear::1.0"_sh);

void
newSopOperator(OP_OperatorTable* table)
{
    OP_Operator* newOp = new OP_Operator(
        SOP_FeE_VolumeClear_1_0::theSOPTypeName,
        "FeE Volume Clear",
        SOP_FeE_VolumeClear_1_0::myConstructor,
        SOP_FeE_VolumeClear_1_0::buildTemplates(),
        1,
        1,
        nullptr,
        OP_FLAG_GENERATOR,
        nullptr,
        1,
        "Five elements Elf/Filter/Volume");

    newOp->setIconName("SOP_heightfield_layerclear");
    table->addOperator(newOp);
}

class SOP_FeE_VolumeClear_1_0Verb : public SOP_NodeVerb
{
public:
    SOP_FeE_VolumeClear_1_0Verb() {}
    virtual ~SOP_FeE_VolumeClear_1_0Verb() {}

    virtual SOP_NodeParms* allocParms() const { return new SOP_FeE_VolumeClear_1_0Parms(); }
    virtual UT_StringHolder name() const { return SOP_FeE_VolumeClear_1_0::theSOPTypeName; }

    virtual CookMode cookMode(const SOP_NodeParms* parms) const { return COOK_INPLACE; }

    virtual void cook(const CookParms& cookparms) const;

    static const SOP_NodeVerb::Register<SOP_FeE_VolumeClear_1_0Verb> theVerb;
};

const SOP_NodeVerb::Register<SOP_FeE_VolumeClear_1_0Verb> SOP_FeE_VolumeClear_1_0Verb::theVerb;

const SOP_NodeVerb*
SOP_FeE_VolumeClear_1_0::cookVerb() const
{
    return SOP_FeE_VolumeClear_1_0Verb::theVerb.get();
}






void
SOP_FeE_VolumeClear_1_0Verb::cook(const SOP_NodeVerb::CookParms &cookparms) const
{
    auto &&sopparms = cookparms.parms<SOP_FeE_VolumeClear_1_0Parms>();
    GA_Detail& outGeo0 = *cookparms.gdh().gdpNC();

    using NumVolumeClear = SOP_FeE_VolumeClear_1_0Parms::NumVolumeClear;
    
    //const GA_Detail& inGeo0 = *cookparms.inputGeo(0);
    //const GA_Detail* const inGeo1 = cookparms.inputGeo(1);
    
    //outGeo0.replaceWith(inGeo0);



    GFE_VolumeClear volumeClear(outGeo0, cookparms);

#if GFE_VolumeClear_Kernel
    volumeClear.setKernel(sopparms.getKernel());
#endif
    
    volumeClear.setComputeParm(
        sopparms.getSubscribeRatio(), sopparms.getMinGrainSize());
    
    const UT_Array<NumVolumeClear>& numVolumeClears = sopparms.getNumVolumeClear();
    const size_t size = numVolumeClears.size();
    for (size_t i = 0; i < size; ++i)
    {
        if (volumeClear.getVolumeArray().append(numVolumeClears[i].name))
            volumeClear.getVoxelValues().emplace_back(numVolumeClears[i].value);
    }
    
    volumeClear.computeAndBumpDataId();
    

}

