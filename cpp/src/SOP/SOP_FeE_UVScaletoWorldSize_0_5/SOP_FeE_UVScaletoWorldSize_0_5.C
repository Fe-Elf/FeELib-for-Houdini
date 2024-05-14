
//#define UT_ASSERT_LEVEL 3
#include "SOP_FeE_UVScaletoWorldSize_0_5.h"


#include "SOP_FeE_UVScaletoWorldSize_0_5.proto.h"

#include "GA/GA_Detail.h"
#include "PRM/PRM_TemplateBuilder.h"
#include "UT/UT_Interrupt.h"
#include "UT/UT_DSOVersion.h"


#include "GFE/GFE_Attribute.h"
#include "GFE/GFE_Range.h"
#include "GFE/GFE_GroupParser.h"


using namespace SOP_FeE_UVScaletoWorldSize_0_5_Namespace;


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
        parmtag { "sop_input" "0" }
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
        name    "uvAttribClass"
        cppname "UVAttribClass"
        label   "UV Attribute Class"
        type    ordinal
        default { "auto" }
        menu {
            "auto"      "Auto"
            "point"     "Point"
            "vertex"    "Vertex"
        }
    }
    parm {
        name    "uvAttribNames"
        cppname "UVAttribNames"
        label   "UV Attribute Names"
        type    string
        default { "uv" }
    }
    parm {
        name    "computeUVAreaInPiece"
        cppname "ComputeUVAreaInPiece"
        label   "Compute UV Area In Pieces"
        type    toggle
        default { "1" }
    }



    parm {
        name    "areaAttribNames"
        cppname "AreaAttribNames"
        label   "Area Attribute Names"
        type    string
        default { "area" }
    }
    parm {
        name    "areaUVAttribNames"
        cppname "AreaUVAttribNames"
        label   "Area UV Attribute Names"
        type    string
        default { "areaUV" }
    }



    parm {
        name    "doUVScalex"
        cppname "DoUVScalex"
        label   "Do Scale X"
        type    toggle
        default { "1" }
        nolabel
        joinnext
    }
    parm {
        name    "uvScalex"
        cppname "UVScalex"
        label   "UV Scale X"
        type    float
        default { 1 }
        range   { -1 10 }
        disablewhen "{ doUVScalex == 0 }"
    }



    parm {
        name    "doUVScaley"
        cppname "DoUVScaley"
        label   "Do Scale Y"
        type    toggle
        default { "1" }
        nolabel
        joinnext
    }
    parm {
        name    "uvScaley"
        cppname "UVScaley"
        label   "UV Scale Y"
        type    float
        default { 1 }
        range   { -1 10 }
        disablewhen "{ doUVScaley == 0 }"
    }




    parm {
        name    "doUVScalez"
        cppname "DoUVScalez"
        label   "Do Scale Z"
        type    toggle
        default { "1" }
        nolabel
        joinnext
    }
    parm {
        name    "uvScalez"
        cppname "UVScalez"
        label   "UV Scale Z"
        type    float
        default { 1 }
        range   { -1 10 }
        disablewhen "{ doUVScalez == 0 }"
    }
    parm {
        name    "uvScale"
        cppname "UVScale"
        label   "UV Scale"
        type    float
        default { 1 }
        range   { -10 10 }
        disablewhen "{ doUVScalex == 0 doUVScaley == 0 doUVScalez == 0 }"
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
SOP_FeE_UVScaletoWorldSize_0_5::buildTemplates()
{
    static PRM_TemplateBuilder templ("SOP_FeE_UVScaletoWorldSize_0_5.C"_sh, theDsFile);
    if (templ.justBuilt())
    {
        templ.setChoiceListPtr("group"_sh, &SOP_Node::allGroupMenu);
        templ.setChoiceListPtr("uvAttribNames"_sh, &SOP_Node::allAttribReplaceMenu);
        templ.setChoiceListPtr("areaAttribNames"_sh, &SOP_Node::primAttribReplaceMenu);
        templ.setChoiceListPtr("areaUVAttribNames"_sh, &SOP_Node::primAttribReplaceMenu);
    }
    return templ.templates();
}


const UT_StringHolder SOP_FeE_UVScaletoWorldSize_0_5::theSOPTypeName("FeE::uvScaletoWorldSize::0.5"_sh);

void
newSopOperator(OP_OperatorTable* table)
{
    OP_Operator* newOp = new OP_Operator(
        SOP_FeE_UVScaletoWorldSize_0_5::theSOPTypeName,
        "FeE UV Scale to World Size",
        SOP_FeE_UVScaletoWorldSize_0_5::myConstructor,
        SOP_FeE_UVScaletoWorldSize_0_5::buildTemplates(),
        1,
        1,
        nullptr,
        OP_FLAG_GENERATOR,
        nullptr,
        1,
        "Five elements Elf/Data/Orient");

    newOp->setIconName("SOP_uvtransform-2.0");
    table->addOperator(newOp);
}



//class SOP_FeE_UVScaletoWorldSize_0_5Cache : public SOP_NodeCache
//{
//public:
//    SOP_FeE_UVScaletoWorldSize_0_5Cache() : SOP_NodeCache(),
//        myPrevOutputDetailID(-1)
//    {}
//    ~SOP_FeE_UVScaletoWorldSize_0_5Cache() override {}
//
//    int64 myPrevOutputDetailID;
//};


class SOP_FeE_UVScaletoWorldSize_0_5Verb : public SOP_NodeVerb
{
public:
    SOP_FeE_UVScaletoWorldSize_0_5Verb() {}
    virtual ~SOP_FeE_UVScaletoWorldSize_0_5Verb() {}

    virtual SOP_NodeParms *allocParms() const { return new SOP_FeE_UVScaletoWorldSize_0_5Parms(); }
    //virtual SOP_NodeCache* allocCache() const { return new SOP_FeE_UVScaletoWorldSize_0_5Cache(); }
    virtual UT_StringHolder name() const { return SOP_FeE_UVScaletoWorldSize_0_5::theSOPTypeName; }

    virtual CookMode cookMode(const SOP_NodeParms *parms) const { return COOK_GENERIC; }

    virtual void cook(const CookParms &cookparms) const;

    //virtual bool doPartialInputCook() const { return true; }
    virtual bool cookInputs(const InputParms& parms) const
    {
        if (parms.inputs().hasInput(0))
        {
            if (!parms.inputs().cookInput(0))
                return false;
        }
        auto&& sopparms = parms.parms<SOP_FeE_UVScaletoWorldSize_0_5Parms>();
        const bool& computeUVAreaInPiece = sopparms.getComputeUVAreaInPiece();
        const exint refInputnum = computeUVAreaInPiece ? 2 : 1;
        if (parms.inputs().hasInput(refInputnum))
        {
            if (!parms.inputs().cookInput(refInputnum))
                return false;
        }
        return true;
    }

    /// This static data member automatically registers
    /// this verb class at library ldir0d time.
    static const SOP_NodeVerb::Register<SOP_FeE_UVScaletoWorldSize_0_5Verb> theVerb;
};

// The static member variable definition has to be outside the class definition.
// The declaration is inside the class.
const SOP_NodeVerb::Register<SOP_FeE_UVScaletoWorldSize_0_5Verb> SOP_FeE_UVScaletoWorldSize_0_5Verb::theVerb;

const SOP_NodeVerb *
SOP_FeE_UVScaletoWorldSize_0_5::cookVerb() const 
{ 
    return SOP_FeE_UVScaletoWorldSize_0_5Verb::theVerb.get();
}




static GA_AttributeOwner
sopAttribOwner(SOP_FeE_UVScaletoWorldSize_0_5Parms::UVAttribClass attribClass)
{
    using namespace SOP_FeE_UVScaletoWorldSize_0_5Enums;
    switch (attribClass)
    {
    case UVAttribClass::AUTO:      return GA_ATTRIB_DETAIL;     break;//not detail but means Auto
    case UVAttribClass::POINT:     return GA_ATTRIB_POINT;      break;
    case UVAttribClass::VERTEX:    return GA_ATTRIB_VERTEX;     break;
    }
    UT_ASSERT_MSG(0, "Unhandled Geo0 Class type!");
    return GA_ATTRIB_INVALID;
}

static GA_GroupType
sopGroupType(SOP_FeE_UVScaletoWorldSize_0_5Parms::GroupType parmgrouptype)
{
    using namespace SOP_FeE_UVScaletoWorldSize_0_5Enums;
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


void
SOP_FeE_UVScaletoWorldSize_0_5Verb::cook(const SOP_NodeVerb::CookParms& cookparms) const
{
    auto&& sopparms = cookparms.parms<SOP_FeE_UVScaletoWorldSize_0_5Parms>();
    GA_Detail& outGeo0 = *cookparms.gdh().gdpNC();
    //auto sopcache = (SOP_FeE_UVScaletoWorldSize_0_5Cache*)cookparms.cache();

    const bool computeUVAreaInPiece = sopparms.getComputeUVAreaInPiece();
    
    const GA_Detail& inGeo0 = *cookparms.inputGeo(0);
    const GA_Detail* const inGeo1 = cookparms.inputGeo(computeUVAreaInPiece ? 2 : 1);

    if (!inGeo1)
    {
        cookparms.sopAddError(SOP_MESSAGE, "no ref input");
        //outGeo0.clearAndDestroy();
        outGeo0.clearCaches();
        outGeo0.incrementMetaCacheCount();
        outGeo0.clear();
        return;
    }
    if (inGeo0.getNumPrimitives() != inGeo1->getNumPrimitives())
    {
        cookparms.sopAddError(SOP_MESSAGE, "NON MATCH Prim Count");
        //outGeo0.clearAndDestroy();
        outGeo0.clearCaches();
        outGeo0.incrementMetaCacheCount();
        outGeo0.clear();
        return;
    }

    outGeo0.replaceWith(inGeo0);
    // outGeo0.clearAndDestroy();

    //outGeo0 = sopNodeProcess(*inGeo0);

    
    const bool doUVScalex = sopparms.getDoUVScalex();
    const bool doUVScaley = sopparms.getDoUVScaley();
    const bool doUVScalez = sopparms.getDoUVScalez();
    if (!doUVScalex && !doUVScaley && !doUVScalez)
        return;


    const UT_StringHolder& geo0AttribNames = sopparms.getUVAttribNames();
    if (!geo0AttribNames.isstring())
        return;

    const UT_StringHolder& geo1AreaAttribNames = sopparms.getAreaAttribNames();
    if (!geo1AreaAttribNames.isstring())
        return;

    const UT_StringHolder& geo1AreaUVAttribNames = sopparms.getAreaUVAttribNames();
    if (!geo1AreaUVAttribNames.isstring())
        return;







    const GA_GroupType groupType = sopGroupType(sopparms.getGroupType());

    GOP_Manager gop;
    const GA_Group* const geo0Group = GFE_GroupParser_Namespace::findOrParseGroupDetached(cookparms, &outGeo0, groupType, sopparms.getGroup(), gop);
    if (geo0Group && geo0Group->isEmpty())
        return;

    const GA_GroupType geo0finalGroupType = geo0Group ? geo0Group->classType() : GA_GROUP_INVALID;


    UT_AutoInterrupt boss("Processing");
    if (boss.wasInterrupted())
        return;




    const fpreal uniScale = sopparms.getUVScale();
    const fpreal64 uvScalex = sopparms.getUVScalex();
    const fpreal64 uvScaley = sopparms.getUVScaley();
    const fpreal64 uvScalez = sopparms.getUVScalez();

    UT_Vector3T<fpreal64> uvScale(doUVScalex, doUVScaley, doUVScalez);
    //TAttribTypeV uvScale(uvScalex, uvScaley, uvScalez);
    uvScale *= UT_Vector3T<fpreal64>(uvScalex, uvScaley, uvScalez) * uniScale;


    const GA_AttributeOwner geo0AttribClass = sopAttribOwner(sopparms.getUVAttribClass());
    //fpreal uvSplitDistThreshold = sopparms.getUVSplitDistThreshold();

    const exint subscribeRatio = sopparms.getSubscribeRatio();
    const exint minGrainSize = sopparms.getMinGrainSize();

    const GA_Storage inStorageF = GA_STORE_REAL32;


    const UT_StringHolder& geo0AttribNameSub       = geo0AttribNames;
    const UT_StringHolder& geo1AreaAttribNameSub   = geo1AreaAttribNames;
    const UT_StringHolder& geo1AreaUVAttribNameSub = geo1AreaUVAttribNames;



    const GA_Attribute* const areaAttribPtr = inGeo1->getAttributes().findAttribute(GA_ATTRIB_PRIMITIVE, geo1AreaAttribNameSub);
    if (!areaAttribPtr)
        return;

    const GA_Attribute* const areaUVAttribPtr = inGeo1->getAttributes().findAttribute(GA_ATTRIB_PRIMITIVE, geo1AreaUVAttribNameSub);
    if (!areaUVAttribPtr)
        return;

    GA_ROHandleT<fpreal> areaAttribHandle(areaAttribPtr);
    GA_ROHandleT<fpreal> areaUVAttribHandle(areaUVAttribPtr);


    GA_ATINumericUPtr uvScaleATI_deleter = outGeo0.createDetachedTupleAttribute(GA_ATTRIB_PRIMITIVE, inStorageF, 3);
    GA_ATINumeric* const uvScaleATIPtr = uvScaleATI_deleter.get();
    GA_RWHandleT<UT_Vector3T<fpreal64>> uvScaleAttribHandle(uvScaleATIPtr);



    GA_AttributeOwner geo0AttribClassFinal;
    GA_Attribute* const attribPtr = GFE_Attribute::findAttributePointVertex(outGeo0, geo0AttribClass, geo0AttribNameSub, geo0AttribClassFinal);
    if (!attribPtr)
        return;

    GA_RWHandleT<UT_Vector3T<fpreal64>> attribHandle(attribPtr);




    const int attribSize = attribPtr->getTupleSize();
    switch (attribSize)
    {
    case 3:
        break;
    default:
        UT_ASSERT_MSG(0, "Unhandled outGeo0 Attrib Size");
    }



    {
        const GA_SplittableRange geo0PrimSplittableRange(outGeo0.getPrimitiveRange());
        UTparallelFor(geo0PrimSplittableRange, [&outGeo0, &uvScaleAttribHandle, &areaUVAttribHandle, &areaAttribHandle, &uvScale, &doUVScalex, &doUVScaley, &doUVScalez](const GA_SplittableRange& r)
        {
            GA_Offset start, end;
            for (GA_Iterator it(r); it.blockAdvance(start, end); )
            {
                for (GA_Offset elemoff = start; elemoff < end; ++elemoff)
                {
                    const fpreal64 areaUV = areaUVAttribHandle.get(elemoff);
                    const fpreal64 area = areaAttribHandle.get(elemoff);
                    UT_Vector3T<fpreal64> uvS = uvScale * sqrt(area / areaUV);
                    uvS[0] = doUVScalex ? uvS[0] : 1;
                    uvS[1] = doUVScaley ? uvS[1] : 1;
                    uvS[2] = doUVScalez ? uvS[2] : 1;
                    uvScaleAttribHandle.set(elemoff, uvS);
                }
            }
        }, subscribeRatio, minGrainSize);
    }

    {
        //const GA_Range geo0Range = GFE_Group::getRangeByAnyGroup(outGeo0, geo0Group, geo0AttribClassFinal);
        const GA_SplittableRange geo0SplittableRange = GFE_Range::getSplittableRangeByAnyGroup(&outGeo0, geo0Group, geo0AttribClassFinal);
        UTparallelFor(geo0SplittableRange, [&outGeo0, &attribHandle, &uvScaleAttribHandle, &geo0AttribClassFinal](const GA_SplittableRange& r)
        {
            GA_Offset start, end;
            for (GA_Iterator it(r); it.blockAdvance(start, end); )
            {
                for (GA_Offset elemoff = start; elemoff < end; ++elemoff)
                {
                    const GA_Offset& primoff = geo0AttribClassFinal == GA_ATTRIB_POINT ? outGeo0.vertexPrimitive(outGeo0.pointVertex(elemoff)) : outGeo0.vertexPrimitive(elemoff);
                    UT_Vector3T<fpreal64> uv = attribHandle.get(elemoff) * uvScaleAttribHandle.get(primoff);
                    attribHandle.set(elemoff, uv);
                }
            }
        }, subscribeRatio, minGrainSize);
    }


    attribPtr->bumpDataId();
}
