
#include "SOP_FeE_VolumeProject_1_0.h"
#include "SOP_FeE_VolumeProject_1_0.proto.h"

#include "GA/GA_Detail.h"
#include "PRM/PRM_TemplateBuilder.h"
#include "UT/UT_Interrupt.h"
#include "UT/UT_DSOVersion.h"


#include "GFE/GFE_VolumeProject.h"


using namespace SOP_FeE_VolumeProject_1_0_Namespace;

static const char *theDsFile = R"THEDSFILE(
{
    name	volumefft

    parm {
		name	"group"
		cppname	"Group"
		label	"Velocity Volumes"
		type	string
		default	{ "" }
		parmtag	{ "script_action" "import soputils\nkwargs['geometrytype'] = (hou.geometryType.Primitives,)\nkwargs['inputindex'] = 0\nsoputils.selectGroupParm(kwargs)" }
		parmtag	{ "script_action_help" "Select geometry from an available viewport.\nShift-click to turn on Select Groups." }
		parmtag	{ "script_action_icon" "BUTTONS_reselect" }
    }
    parm {
		name	"divGroup"
		cppname	"DivGroup"
		label	"Divergence Volume"
		type	string
		default	{ "" }
		parmtag	{ "script_action" "import soputils\nkwargs['geometrytype'] = (hou.geometryType.Primitives,)\nkwargs['inputindex'] = 1\nsoputils.selectGroupParm(kwargs)" }
		parmtag	{ "script_action_help" "Select geometry from an available viewport.\nShift-click to turn on Select Groups." }
		parmtag	{ "script_action_icon" "BUTTONS_reselect" }
    }
    parm {
		name	"lutGroup"
		cppname	"LUTGroup"
		label	"LUT Volumes"
		type	string
		default	{ "" }
		parmtag	{ "script_action" "import soputils\nkwargs['geometrytype'] = (hou.geometryType.Primitives,)\nkwargs['inputindex'] = 2\nsoputils.selectGroupParm(kwargs)" }
		parmtag	{ "script_action_help" "Select geometry from an available viewport.\nShift-click to turn on Select Groups." }
		parmtag	{ "script_action_icon" "BUTTONS_reselect" }
    }
    parm {
		name	"activeGroup"
		cppname	"ActiveGroup"
		label	"Active Volume"
		type	string
		default	{ "" }
		parmtag	{ "script_action" "import soputils\nkwargs['geometrytype'] = (hou.geometryType.Primitives,)\nkwargs['inputindex'] = 3\nsoputils.selectGroupParm(kwargs)" }
		parmtag	{ "script_action_help" "Select geometry from an available viewport.\nShift-click to turn on Select Groups." }
		parmtag	{ "script_action_icon" "BUTTONS_reselect" }
    }
    parm {
		name	"lutCenter"
		cppname "LUTCenter"
		label	"LUT Center"
		type	integer
		default	{ 10 }
    }
    parm {
		name	"lutRad"
		cppname "LUTRad"
		label	"LUT Rad"
		type	integer
		default	{ 1 }
    }
    parm {
		name	"lutMagic"
		cppname "LUTMagic"
		label	"LUT Magic"
		type	float
		default	{ 1 }
    }
    parm {
		name	"lutRound"
		cppname "LUTRound"
		label	"LUT Round Pattern"
		type	toggle
		default	{ "1" }
    }
    parm {
		name	"doMIP"
		cppname "DoMIP"
		label	"Do MIP MAP"
		type 	toggle
		default	{ "1" }
    }
    parm {
		name	"mipBy4"
		cppname "MipBy4"
		label	"Mip MAP by 4"
		type 	toggle
		default	{ "1" }
    }
    parm {
		name	"mipMagic"
		cppname "MIPMagic"
		label	"MIP Magic"
		type	float
		default	{ 1 }
    }

    parm {
		name	"zeroInactive"
		cppname "ZeroInactive"
		label	"Zero Inactive"
		type	toggle
		default	{ 0 }
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

PRM_Template *
SOP_FeE_VolumeProject_1_0::buildTemplates()
{
    static PRM_TemplateBuilder	templ("SOP_FeE_VolumeProject_1_0.C"_sh, theDsFile);
    if (templ.justBuilt())
    {
	templ.setChoiceListPtr("group", &SOP_Node::primGroupMenu);
	templ.setChoiceListPtr("divGroup", &SOP_Node::primGroupMenu);
    }
    return templ.templates();
}



const UT_StringHolder SOP_FeE_VolumeProject_1_0::theSOPTypeName("FeE::volumeProject::1.0"_sh);

void
newSopOperator(OP_OperatorTable *table)
{
	OP_Operator* newOp = new OP_Operator(
		SOP_FeE_VolumeProject_1_0::theSOPTypeName,
		"FeE Volume Project",
		SOP_FeE_VolumeProject_1_0::myConstructor,
		SOP_FeE_VolumeProject_1_0::buildTemplates(),
		3,
		4,
		nullptr,
		OP_FLAG_GENERATOR,
		nullptr,
		1,
        "Five elements Elf/Filter/Volume");

	newOp->setIconName("volumeanalysis");
	table->addOperator(newOp);
}


class SOP_FeE_VolumeProject_1_0Verb : public SOP_NodeVerb
{
public:
    SOP_FeE_VolumeProject_1_0Verb() 
    {
    }
    virtual ~SOP_FeE_VolumeProject_1_0Verb() {}

    virtual SOP_NodeParms	*allocParms() const { return new SOP_FeE_VolumeProject_1_0Parms(); }
    virtual UT_StringHolder name() const { return SOP_FeE_VolumeProject_1_0::theSOPTypeName; }

    virtual CookMode		 cookMode(const SOP_NodeParms *parms)  const { return COOK_INPLACE; }

    virtual void	cook(const CookParms &cookparms) const;
};


static SOP_NodeVerb::Register<SOP_FeE_VolumeProject_1_0Verb> theSOPVolumeProjectVerb;

const SOP_NodeVerb *
SOP_FeE_VolumeProject_1_0::cookVerb() const 
{ 
    return theSOPVolumeProjectVerb.get();
}


void
SOP_FeE_VolumeProject_1_0Verb::cook(const SOP_NodeVerb::CookParms &cookparms) const
{
    auto&& sopparms = cookparms.parms<SOP_FeE_VolumeProject_1_0Parms>();
    GA_Detail& outGeo0 = *cookparms.gdh().gdpNC();

	const GA_Detail& inGeo1 = *cookparms.inputGeo(1);
	const GA_Detail& inGeo2 = *cookparms.inputGeo(2);
	const GA_Detail* const inGeo3 = cookparms.inputGeo(3);


	
	GFE_VolumeProject volumeProject(outGeo0, inGeo1, inGeo2, inGeo3, cookparms);

	volumeProject.groupParser    .setPrimitiveGroup(sopparms.getGroup());
	volumeProject.groupParserRef0.setPrimitiveGroup(sopparms.getDivGroup());
	volumeProject.groupParserRef1.setPrimitiveGroup(sopparms.getLUTGroup());
	volumeProject.groupParserRef2.setPrimitiveGroup(sopparms.getActiveGroup());
	
	//volumeProject.groupParser.setGroup(groupType, sopparms.getGroup());
	//volumeProject.getOutAttribArray().set(geo0AttribClass, geo0AttribNames);
	volumeProject.setComputeParm(
		sopparms.getSubscribeRatio(), sopparms.getMinGrainSize());

	volumeProject.computeAndBumpDataId();

}
