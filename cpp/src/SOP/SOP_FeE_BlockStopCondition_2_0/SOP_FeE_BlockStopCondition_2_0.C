
//#define UT_ASSERT_LEVEL 3
#include "SOP_FeE_BlockStopCondition_2_0.h"


#include "SOP_FeE_BlockStopCondition_2_0.proto.h"

#include "GA/GA_Detail.h"
#include "PRM/PRM_TemplateBuilder.h"
#include "UT/UT_Interrupt.h"
#include "UT/UT_DSOVersion.h"


#define __GFE_StopCondition_UseCache 0
#define __GFE_StopCondition_FirstValue 2
#define __GFE_StopCondition_SecondValue 1

using namespace SOP_FeE_BlockStopCondition_2_0_Namespace;


static const char *theDsFile = R"THEDSFILE(
{
    name        parameters

    parm {
        name    "Readme"
        label   " "
        type    label
        size    1
        default { "Remenber to delete Stop Condition Attrib in Detail Class after Block End" }
    }
    parm {
        name    "enable"
        cppname "Enable"
        label   "Enable"
        type    toggle
        default { "1" }
    }
    groupsimple {
        name    "stopCondition_folder"
        label   "Stop Condition"
        hidewhen "{ enable == 0 }"
        grouptag { "group_type" "simple" }

        parm {
            name    "stopConditionAttrib"
            cppname "StopConditionAttrib"
            label   "Stop Condition Attrib"
            type    string
            default { "stopCondition" }
        }
        parm {
            name    "condition"
            cppname "Condition"
            label   "Condition"
            type    toggle
            default { "0" }
        }
    }
}
)THEDSFILE";

PRM_Template*
SOP_FeE_BlockStopCondition_2_0::buildTemplates()
{
    static PRM_TemplateBuilder templ("SOP_FeE_BlockStopCondition_2_0.C"_sh, theDsFile);
    if (templ.justBuilt())
    {
        templ.setChoiceListPtr("stopConditionAttrib"_sh, &SOP_Node::detailAttribReplaceMenu);
    }
    return templ.templates();
}

const UT_StringHolder SOP_FeE_BlockStopCondition_2_0::theSOPTypeName("FeE::blockStopCondition::2.0"_sh);

void
newSopOperator(OP_OperatorTable* table)
{
    OP_Operator* newOp = new OP_Operator(
        SOP_FeE_BlockStopCondition_2_0::theSOPTypeName,
        "FeE Block Stop Condition",
        SOP_FeE_BlockStopCondition_2_0::myConstructor,
        SOP_FeE_BlockStopCondition_2_0::buildTemplates(),
        2,
        3,
        nullptr,
        OP_FLAG_GENERATOR,
        nullptr,
        1,
        "Five elements Elf/Operation");

    newOp->setIconName("SOP_block_end");
    table->addOperator(newOp);

}




#if __GFE_StopCondition_UseCache

class GFE_BlockStopCondition_Cache : public SOP_NodeCache
{
public:
    GFE_BlockStopCondition_Cache()
        : SOP_NodeCache()
    {
    }
    
    ~GFE_BlockStopCondition_Cache() {}
    
public:
    GU_DetailHandle geo_h;
    GU_Detail* geo = nullptr;
    
}; // End of Class GFE_BlockStopCondition_Cache

#endif





class SOP_FeE_BlockStopCondition_2_0Verb : public SOP_NodeVerb
{
public:
    SOP_FeE_BlockStopCondition_2_0Verb() {}
    virtual ~SOP_FeE_BlockStopCondition_2_0Verb() {}

    virtual SOP_NodeParms *allocParms() const { return new SOP_FeE_BlockStopCondition_2_0Parms(); }
#if __GFE_StopCondition_UseCache
    virtual SOP_NodeCache *allocCache() const { return new GFE_BlockStopCondition_Cache(); }
#endif
    virtual UT_StringHolder name() const { return SOP_FeE_BlockStopCondition_2_0::theSOPTypeName; }

    virtual CookMode cookMode(const SOP_NodeParms *parms) const
    {
        auto* sopparms = reinterpret_cast<const SOP_FeE_BlockStopCondition_2_0Parms*>(parms);
        return sopparms->getEnable() ? COOK_GENERIC : COOK_PASSTHROUGH;
    }

    virtual void cook(const CookParms &cookparms) const;

    bool doPartialInputCook() const override
    { return true; }
    
    bool cookInputs(const InputParms& parms) const override
    {
        auto&& sopparms = parms.parms<SOP_FeE_BlockStopCondition_2_0Parms>();
        NodeInputs& inputs = parms.inputs();
        
        if (!sopparms.getEnable())
            return inputs.cookInput(0);

        if (!inputs.cookInput(1))
            return false;
        
        const GA_Detail& inGeo1 = *parms.inputs().inputGeo(1).gdp();
        const GA_Attribute* const stopConditionAttrib = inGeo1.findAttribute(GA_ATTRIB_DETAIL, sopparms.getStopConditionAttrib());
        
        int32 stopCondition = 0;
        if (stopConditionAttrib)
        {
            const GA_AIFTuple* const aifTuple = stopConditionAttrib->getAIFTuple();
            if (aifTuple)
                aifTuple->get(stopConditionAttrib, 0, stopCondition);
        }
        if (stopCondition) // stopCondition == 1 || stopCondition == 2
            return true;
        return inputs.cookInput((inputs.hasInput(2) && sopparms.getCondition()) ? 2 : 0);
    }
    /// This static data member automatically registers
    /// this verb class at library load time.
    static const SOP_NodeVerb::Register<SOP_FeE_BlockStopCondition_2_0Verb> theVerb;
};

// The static member variable definition has to be outside the class definition.
// The declaration is inside the class.
const SOP_NodeVerb::Register<SOP_FeE_BlockStopCondition_2_0Verb> SOP_FeE_BlockStopCondition_2_0Verb::theVerb;

const SOP_NodeVerb *
SOP_FeE_BlockStopCondition_2_0::cookVerb() const 
{ 
    return SOP_FeE_BlockStopCondition_2_0Verb::theVerb.get();
}







void
SOP_FeE_BlockStopCondition_2_0Verb::cook(const SOP_NodeVerb::CookParms &cookparms) const
{
    auto&& sopparms = cookparms.parms<SOP_FeE_BlockStopCondition_2_0Parms>();
    GU_Detail& outGeo0 = *cookparms.gdh().gdpNC();
    //auto sopcache = (GFE_BlockStopCondition_Cache*)cookparms.cache();
    
    //if (!sopparms.getEnable())
    //    return outGeo0.replaceWith(*cookparms.inputGeo(0));
        
    const GU_Detail& inGeo1 = *cookparms.inputGeo(1);
    
    const GA_Attribute* const stopConditionAttrib = inGeo1.findAttribute(GA_ATTRIB_DETAIL, sopparms.getStopConditionAttrib());
        
    int32 stopCondition = 0;
    if (stopConditionAttrib)
    {
        const GA_AIFTuple* const aifTuple = stopConditionAttrib->getAIFTuple();
        if (aifTuple)
        {
            aifTuple->get(stopConditionAttrib, 0, stopCondition);
        }
    }
    // switch (stopCondition)
    // {
    // case 0: break;
    // case 1: return outGeo0.replaceWith(*cookparms.inputGeo(2)); break;
    // case 2: break;
    // }
    if (stopCondition) // stopCondition == 1 || stopCondition == 2
    {
        if (stopCondition==__GFE_StopCondition_SecondValue)
        {
            //GA_Attribute* const outStopConditionAttrib = outGeo0.findAttribute(GA_ATTRIB_DETAIL, sopparms.getStopConditionAttrib());
            //outStopConditionAttrib->bumpDataId();
#ifndef NDEBUG
            const exint dataId0 = outGeo0.getUniqueId();
            const exint dataId1 = inGeo1.getUniqueId();
#endif
            outGeo0.replaceWith(inGeo1);
            //if (outGeo0.getUniqueId() != inGeo1.getUniqueId())
            //    outGeo0.replaceWith(inGeo1);
            //cookparms.gdh().allocateAndSet(const_cast<GU_Detail*>(cookparms.inputGeo(1)));
        }
        else
        {
            UT_ASSERT_MSG(stopCondition == int8(__GFE_StopCondition_FirstValue), "Unhandled Stop Condition");
            
            outGeo0.replaceWith(inGeo1);
            //outGeo0.duplicate(inGeo1);
            GA_Attribute* const outStopConditionAttrib = outGeo0.findAttribute(GA_ATTRIB_DETAIL, sopparms.getStopConditionAttrib());
            
            UT_ASSERT_P(outStopConditionAttrib);
            
            const GA_AIFTuple* const aifTuple = outStopConditionAttrib->getAIFTuple();
            
            UT_ASSERT_P(aifTuple);
            
            aifTuple->set(outStopConditionAttrib, 0, int32(__GFE_StopCondition_SecondValue));
            outStopConditionAttrib->bumpDataId();
#if __GFE_StopCondition_UseCache
            if (!sopcache->geo)
            {
                sopcache->geo = new GU_Detail;
                sopcache->geo->replaceWith(outGeo0);
                sopcache->geo_h.allocateAndSet(sopcache->geo);
            }
#endif
        }
        return;
    }
    if (sopparms.getCondition())
    {
        outGeo0.replaceWith(*cookparms.inputGeo(cookparms.hasInput(2) ? 2 : 0));
        GA_Attribute* outStopConditionAttrib = outGeo0.findAttribute(GA_ATTRIB_DETAIL, sopparms.getStopConditionAttrib());

        if (outStopConditionAttrib)
            outStopConditionAttrib->bumpDataId(); 
        else
            outStopConditionAttrib = outGeo0.getAttributes().createTupleAttribute(GA_ATTRIB_DETAIL, sopparms.getStopConditionAttrib(), GA_STORE_INT8, 1, GA_Defaults(0));
        UT_ASSERT_P(outStopConditionAttrib);
            
        const GA_AIFTuple* const aifTuple = outStopConditionAttrib->getAIFTuple();
            
        UT_ASSERT_P(aifTuple);
            
        aifTuple->set(outStopConditionAttrib, 0, int32(__GFE_StopCondition_FirstValue));
    }
    else
    {
        UT_ASSERT_P(cookparms.inputGeo(0));
        outGeo0.replaceWith(*cookparms.inputGeo(0));
    }
    

}


#undef __GFE_StopCondition_UseCache
#undef __GFE_StopCondition_FirstValue
#undef __GFE_StopCondition_SecondValue
