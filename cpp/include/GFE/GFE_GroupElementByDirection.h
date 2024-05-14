
#pragma once

#ifndef __GFE_GroupElementByDirection_h__
#define __GFE_GroupElementByDirection_h__

#include "GFE_Math.h"
#include "GFE/GFE_GroupElementByDirection.h"


#include "GFE/GFE_GeoFilter.h"



#include "GFE/GFE_RestDir2D.h"

enum class GFE_GroupElemByDirMethod
{
	Input,
	RestDir2D_AvgNormal,
	RestDir2D_HouOBB,
};



class GFE_GroupElemByDir : public GFE_AttribFilterWithRef0 {

public:
	// using GFE_AttribFilterWithRef0::GFE_AttribFilterWithRef0;
	
	GFE_GroupElemByDir(
		GFE_Detail* const geo,
		const GA_Detail* const geoRef,
		const SOP_NodeVerb::CookParms* const cookparms = nullptr
	)
		: GFE_AttribFilterWithRef0(geo, geoRef, cookparms)
		, restDir2D(geo, cookparms)
		, normal3D(geo, cookparms)
	{
	}

	GFE_GroupElemByDir(
		GA_Detail& geo,
		const GA_Detail* const geoRef,
		const SOP_NodeVerb::CookParms* const cookparms = nullptr
	)
		: GFE_AttribFilterWithRef0(geo, geoRef, cookparms)
		, restDir2D(geo, cookparms)
		, normal3D(geo, cookparms)
	{
	}


	void
		setComputeParm(
			const GFE_GroupElemByDirMethod method = GFE_GroupElemByDirMethod::Input,
			//const bool matchUpDir = true,
			const UT_Vector3R& up = UT_Vector3R(0,1,0),
			const bool reversePrim = false,
			const bool delElem = false,
			const exint subscribeRatio = 64,
			const exint minGrainSize = 1024
		)
	{
		setHasComputed();
		this->method = method;
		//this->matchUpDir = matchUpDir;
		this->up = up;
		this->reversePrim = reversePrim;
		this->doDelGroupElement = delElem;
		this->subscribeRatio = subscribeRatio;
		this->minGrainSize   = minGrainSize;
	}
	
	SYS_FORCE_INLINE fpreal setConeAngleThreshold(const fpreal angle)
	{ return coneAngleThreshold = GFE_Math::radians(angle); }
	
    
	virtual void bumpDataId() const override
	{
		getOutAttribArray().bumpDataId();
		getOutGroupArray().bumpDataId();
		if (reversePrim && outElemGroup->classType() == GA_GROUP_PRIMITIVE && (!outElemGroup->isEmpty() ^ groupSetter.reverseGroup))
			bumpDataIdsForAddOrRemove(false, true, false);
	}

private:

	virtual bool
		computeCore() override
	{
		if (getOutGroupArray().isEmpty() || !getOutGroupArray()[0]->isElementGroup())
			return false;
		
		if (groupParser.isEmpty())
			return true;

		if (method == GFE_GroupElemByDirMethod::RestDir2D_AvgNormal || method == GFE_GroupElemByDirMethod::RestDir2D_HouOBB)
		{
			restDir2D.groupParser.setGroup(groupParser);
			
			const GFE_RestDir2DMethod restDir2DMethod = method == GFE_GroupElemByDirMethod::RestDir2D_AvgNormal ? GFE_RestDir2DMethod::AvgNormal : GFE_RestDir2DMethod::HouOBB;
			
			restDir2D.setComputeParm(restDir2DMethod);

			restDir2D.compute();
			restDir = restDir2D.getRestDir();
		}
		else
		{
			restDir = up;
		}

		
		outElemGroup = getOutGroupArray().getElementGroup(0);
		groupSetter = outElemGroup;
		
		if (!dirAttrib)
		{
			normal3D.groupParser.setGroup(groupParser);

			if (!normal3D.getAttrib())
				normal3D.findOrCreateNormal3D(false, true, *outElemGroup, GA_STORE_INVALID, "N");
		
			normal3D.compute();
			
			dirAttrib = normal3D.getAttrib();
		}
		
		// switch (method)
		// {
		// case GFE_GroupElemByDirMethod::Input:               restDir = up;                     break;
		// case GFE_GroupElemByDirMethod::RestDir2D_AvgNormal:
		// case GFE_GroupElemByDirMethod::RestDir2D_HouOBB:    restDir = restDir2D.getRestDir(); break;
		// }
		const GA_Storage storage = dirAttrib->getAIFTuple()->getStorage(dirAttrib);
		switch (dirAttrib->getTupleSize())
		{
		case 2:
			switch (storage)
			{
			//case GA_STORE_REAL16: groupElemByDir<UT_Vector2T<fpreal16>>(); break;
			case GA_STORE_REAL32: groupElemByDir<UT_Vector2T<fpreal32>>(); break;
			case GA_STORE_REAL64: groupElemByDir<UT_Vector2T<fpreal64>>(); break;
			}
			break;
		case 3:
			switch (storage)
			{
			//case GA_STORE_REAL16: groupElemByDir<UT_Vector3T<fpreal16>>(); break;
			case GA_STORE_REAL32: groupElemByDir<UT_Vector3T<fpreal32>>(); break;
			case GA_STORE_REAL64: groupElemByDir<UT_Vector3T<fpreal64>>(); break;
			}
			break;
		}
		
		if (doDelGroupElement)
			delGroupElement();
		else if (reversePrim && outElemGroup->classType() == GA_GROUP_PRIMITIVE)
			geo->asGU_Detail()->reverse(static_cast<const GA_PrimitiveGroup*>(outElemGroup), false);
		
		return true;
	}

	template<typename VECTOR_T>
	void groupElemByDir()
	{
		VECTOR_T restDirTmp;
		if constexpr (VECTOR_T::tuple_size == 2)
			restDirTmp = VECTOR_T(restDir[0], restDir[1]);
		else
			restDirTmp = restDir;
		
		const typename VECTOR_T::value_type cosConeAngleThreshold = cos(coneAngleThreshold);
		
		UTparallelFor(groupParser.getSplittableRange(dirAttrib), [this, &restDirTmp, cosConeAngleThreshold](const GA_SplittableRange& r)
		{
			GA_PageHandleT<VECTOR_T, typename VECTOR_T::value_type, true, false, const GA_Attribute, const GA_ATINumeric, const GA_Detail> dir_ph(dirAttrib);
			for (GA_PageIterator pit = r.beginPages(); !pit.atEnd(); ++pit)
			{
				GA_Offset start, end;
				for (GA_Iterator it(pit.begin()); it.blockAdvance(start, end); )
				{
					dir_ph.setPage(start);
					for (GA_Offset elemoff = start; elemoff < end; ++elemoff)
					{
						groupSetter.set(elemoff, dir_ph.value(elemoff).dot(restDirTmp) > cosConeAngleThreshold);
					}
				}
			}
		}, subscribeRatio, minGrainSize);
	}
	

public:
	GFE_RestDir2D restDir2D;
	GFE_Normal3D normal3D;

	fpreal coneAngleThreshold = PI * 0.5;
	GFE_GroupElemByDirMethod method = GFE_GroupElemByDirMethod::Input;
	//bool matchUpDir = true;
	UT_Vector3R up = UT_Vector3R(0,1,0);
	bool reversePrim = false;
	
	//bool outIntermediateAttrib = false;
	
	
private:
	UT_Vector3R restDir;
	const GA_Attribute* dirAttrib = nullptr;
	GA_ElementGroup* outElemGroup;
	
	exint subscribeRatio = 64;
	exint minGrainSize = 1024;
	
}; // End of class GFE_GroupPolyByDir





#endif
