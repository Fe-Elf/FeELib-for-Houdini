
//#define UT_ASSERT_LEVEL 3
#include "SOP_FeE_Cook_1_0.h"


#include "SOP_FeE_Cook_1_0.proto.h"

#include "GA/GA_Detail.h"
#include "PRM/PRM_TemplateBuilder.h"
#include "UT/UT_Interrupt.h"
#include "UT/UT_DSOVersion.h"




using namespace SOP_FeE_Cook_1_0_Namespace;


static const char *theDsFile = R"THEDSFILE(
{
    name        parameters
    parm {
        name    "cook"
        cppname "Cook"
        label   "Cook"
        type    integer
        default { 0 }
        range   { 0! 256 }
    }
}
)THEDSFILE";

PRM_Template*
SOP_FeE_Cook_1_0::buildTemplates()
{
    static PRM_TemplateBuilder templ("SOP_FeE_Cook_1_0.C"_sh, theDsFile);
    if (templ.justBuilt())
    {
    }
    return templ.templates();
}

const UT_StringHolder SOP_FeE_Cook_1_0::theSOPTypeName("FeE::cook::1.0"_sh);

void
newSopOperator(OP_OperatorTable* table)
{
    OP_Operator* newOp = new OP_Operator(
        SOP_FeE_Cook_1_0::theSOPTypeName,
        "FeE Cook",
        SOP_FeE_Cook_1_0::myConstructor,
        SOP_FeE_Cook_1_0::buildTemplates(),
        1,
        2,
        nullptr,
        OP_FLAG_GENERATOR,
        nullptr,
        1,
        "Five elements Elf/Operation/Update");

    newOp->setIconName("TOP_houdiniserver");
    table->addOperator(newOp);
}





class SOP_FeE_Cook_1_0Verb : public SOP_NodeVerb
{
public:
    SOP_FeE_Cook_1_0Verb() {}
    virtual ~SOP_FeE_Cook_1_0Verb() {}

    virtual SOP_NodeParms *allocParms() const { return new SOP_FeE_Cook_1_0Parms(); }
    virtual UT_StringHolder name() const { return SOP_FeE_Cook_1_0::theSOPTypeName; }

    virtual CookMode cookMode(const SOP_NodeParms *parms) const { return COOK_PASSTHROUGH; }

    virtual void cook(const CookParms &cookparms) const;
    
    /// This static data member automatically registers
    /// this verb class at library load time.
    static const SOP_NodeVerb::Register<SOP_FeE_Cook_1_0Verb> theVerb;
};

// The static member variable definition has to be outside the class definition.
// The declaration is inside the class.
const SOP_NodeVerb::Register<SOP_FeE_Cook_1_0Verb> SOP_FeE_Cook_1_0Verb::theVerb;

const SOP_NodeVerb *
SOP_FeE_Cook_1_0::cookVerb() const 
{ 
    return SOP_FeE_Cook_1_0Verb::theVerb.get();
}







void
SOP_FeE_Cook_1_0Verb::cook(const SOP_NodeVerb::CookParms &cookparms) const
{
    //auto&& sopparms = cookparms.parms<SOP_FeE_Cook_1_0Parms>();

    //std::cout << __cplusplus << std::endl;
    
#ifndef NDEBUG
    //GA_Detail& outGeo0 = *cookparms.gdh().gdpNC();
    ////auto sopcache = (SOP_FeE_Cook_1_0Cache*)cookparms.cache();
    //
    //const GA_Detail& inGeo0 = *cookparms.inputGeo(0);
    //const GA_Detail* const inGeo1 = cookparms.inputGeo(1);
    // 
    //outGeo0.replaceWith(inGeo0);
    //exint uid0 = outGeo0.getUniqueId();
    //exint uid2 = inGeo0.getUniqueId();
    //exint uid1 = inGeo1 ? inGeo1->getUniqueId() : -1;
#endif
    
#if 1
    //outGeo0.replaceWith(inGeo0);
#else
    
    GU_DetailHandle geo_h;
    GU_Detail* const geoTmp = new GU_Detail;
    geo_h.allocateAndSet(geoTmp);
    geoTmp->replaceWith(inGeo0);
    
    outGeo0.replaceWith(*geoTmp);
    geoTmp->clear();
    
    outGeo0.bumpDataIdsForAddOrRemove(1,1,1);
    //geoTmp->bumpDataIdsForAddOrRemove(1,1,1);
#endif
}


