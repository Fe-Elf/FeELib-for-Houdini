
#pragma once

#ifndef __GFE_GroupPolygonByWinding_h__
#define __GFE_GroupPolygonByWinding_h__

#include "GFE_GroupPromote.h"
#include "GFE/GFE_GroupPolygonByWinding.h"


#include "GFE/GFE_GeoFilter.h"
#include "GFE/GFE_Normal3D.h"


#include "GU/GU_RayIntersect.h"
#include "GU/GU_PolyFill.h"

#include "GFE/GFE_RestDir2D.h"


enum class GFE_GroupPolyByWindingMethod
{
	Ray,
};



class GFE_GroupPolyByWinding : public GFE_AttribFilterWithRef0 {

public:
	//using GFE_AttribFilterWithRef0::GFE_AttribFilterWithRef0;
	
	GFE_GroupPolyByWinding(
		GA_Detail& geo,
		const GA_Detail* const geoRef,
		const SOP_NodeVerb::CookParms* const cookparms = nullptr
	)
		: GFE_AttribFilterWithRef0(geo, geoRef, cookparms)
		, normal3D(geo, cookparms)
	{
	}


	void
		setComputeParm(
			const GFE_GroupPolyByWindingMethod method = GFE_GroupPolyByWindingMethod::Ray,
			const bool meshCap = false,
			const bool reversePrim = false
		)
	{
		setHasComputed();
		this->method  = method;
		this->meshCap = meshCap;
		this->reversePrim = reversePrim;
	}

	

	SYS_FORCE_INLINE GA_Attribute* findOrCreateNormal3D(
		const bool detached = false, 
		const GA_Storage storage = GA_STORE_INVALID,
		const UT_StringRef& attribName = "",
		const int tupleSize = 3
		)
	{ return normal3D.findOrCreateNormal3D(true, detached, GFE_NormalSearchOrder::Primitive, storage, attribName, tupleSize); }

	
	
	virtual void bumpDataId() const override
	{
		//getOutAttribArray().bumpDataId();
		getOutGroupArray().bumpDataId();
		if (reversePrim)
			bumpDataIdsForAddOrRemove(false, true, false);
	}

private:

	virtual bool
		computeCore() override
	{
		getOutGroupArray().eraseNonPrimitiveGroup();
		
		if (getOutGroupArray().isEmpty())
		{
			if (reversePrim)
			{
				findOrCreatePrimitiveGroup(true);
			}
			else
			{
				return false;
			}
		}
			
		if (groupParser.isEmpty())
			return true;

		setValidConstPosAttrib();

		GA_PrimitiveGroup* const outPrimGroup = getOutGroupArray().getPrimitiveGroup(0);
		//GA_PointGroupUPtr outPointGroupUPtr = geo->createDetachedPointGroup();
		//setGroup = outPointGroupUPtr.get();
		groupSetter = outPrimGroup;
		
		if (meshCap)
		{
			geoCapTmp = new GU_Detail();
			geoCap_h.allocateAndSet(geoCapTmp);
			geoCapTmp->replaceWith(*geo);
			
			UT_Array<GA_OffsetArray> rings;
			UT_Array<GA_OffsetArray> ringOrigs;
			GU_PolyFill::singlePolys(geoCapTmp, rings, ringOrigs, nullptr);
			
			//geoCapTmp_GA = geoCapTmp;
		}
		else
		{
			geoCapTmp = geo->asGU_Detail();
			//geoCapTmp_GA = geo;
		}

		
		if (!dirAttrib)
		{
			normal3D.groupParser.setGroup(groupParser);

			if (!normal3D.getAttrib())
				normal3D.findOrCreateNormal3D(false, true, GFE_NormalSearchOrder::Primitive, GA_STORE_INVALID, "N");
			
			dirAttrib = normal3D.getAttrib();
			
			if (dirAttrib->isDetached())
				normal3D.compute();
		}
		
		switch (method)
		{
			case GFE_GroupPolyByWindingMethod::Ray: groupPolyByWindingMethod_ray();     break;
		}


		
		
		//GFE_GroupUnion::groupUnionFull(*outPrimGroup, outPointGroupUPtr.get());
		
		//geo->newPointGroup("__GFE_Debug")->combine(outPointGroupUPtr.get());
		
		if (reversePrim)
			geo->asGU_Detail()->reverse(outPrimGroup, false);
			//geo->asGU_Detail()->reversePolys(outPrimGroup);
		
		return true;
	}
	
	void groupPolyByWindingMethod_ray()
	{
		UT_ASSERT_P(dirAttrib->getAIFTuple());
        const GA_Storage storage = dirAttrib->getAIFTuple()->getStorage(dirAttrib);
		switch (storage)
		{
		//case GA_STORE_INT8:   groupPolyByWindingMethod_ray<int8>();     break;
		//case GA_STORE_INT16:  groupPolyByWindingMethod_ray<int16>();    break;
		//case GA_STORE_INT32:  groupPolyByWindingMethod_ray<int32>();    break;
		//case GA_STORE_INT64:  groupPolyByWindingMethod_ray<int64>();    break;
		case GA_STORE_REAL16: groupPolyByWindingMethod_ray<fpreal16>(); break;
		case GA_STORE_REAL32: groupPolyByWindingMethod_ray<fpreal32>(); break;
		case GA_STORE_REAL64: groupPolyByWindingMethod_ray<fpreal64>(); break;
		}
	}
	
	template<typename FLOAT_T>
	void groupPolyByWindingMethod_ray()
	{
		const fpreal posTol = rayTolerance * 10.0;
		
		const GA_ROHandleT<UT_Vector3T<FLOAT_T>> pos_h(posAttrib);
		//const GA_PrimitiveGroup* const geoPrimGroup = groupParser.getPrimitiveGroup();
		const GA_PrimitiveGroup* const geoPrimGroup = nullptr;
		
		UTparallelFor(groupParser.getPrimitiveSplittableRange(), [this, geoPrimGroup, &pos_h, posTol](const GA_SplittableRange& r)
		{
			UT_Vector3T<FLOAT_T> dir;
			
			const GU_RayIntersect rayIntersect(geoCapTmp, geoPrimGroup, true, false, false, false, false);
			
			GA_PageHandleT<UT_Vector3T<FLOAT_T>, FLOAT_T, true, false, const GA_Attribute, const GA_ATINumeric, const GA_Detail> dir_ph(dirAttrib);
			//GA_PageHandleT<UT_Vector3T<FLOAT_T>, FLOAT_T, true, false, const GA_Attribute, const GA_ATINumeric, const GA_Detail> pos_ph(geo->getP());
			for (GA_PageIterator pit = r.beginPages(); !pit.atEnd(); ++pit)
			{
				GA_Offset start, end;
				for (GA_Iterator it(pit.begin()); it.blockAdvance(start, end); )
				{
					dir_ph.setPage(start);
					//pos_ph.setPage(start);
					for (GA_Offset elemoff = start; elemoff < end; ++elemoff)
					{
						//const UT_Vector3T<FLOAT_T>& origPos = geo->getPos3D(geo->vertexPoint(geo->getPrimitiveVertexOffset(elemoff, 0)));
						dir = dir_ph.value(elemoff);
						dir.normalize();
						
						bool flag = false;
						const GA_Size numvtx = geo->getPrimitiveVertexCount(elemoff);
						for (GA_Size vtxpnum = 0; vtxpnum < numvtx; ++vtxpnum)
						{
							GU_RayInfo rayInfo(1e18f, 0.0f, GU_FIND_ALL, rayTolerance);
							const GA_Offset ptoff = geo->vertexPoint(geo->getPrimitiveVertexOffset(elemoff, vtxpnum));
							const int numIntersect = rayIntersect.sendRay(pos_h.get(ptoff) + dir * posTol, dir * 100000, rayInfo);
						
							if (bool(numIntersect % 2))
							{
								flag = true;
								break;
							}
						}
						if (flag)
							groupSetter.set(elemoff, true);
					}
				}
			}
		}, subscribeRatio, minGrainSize);
	}

	
public:
	GFE_Normal3D normal3D;
	
	GFE_GroupPolyByWindingMethod method = GFE_GroupPolyByWindingMethod::Ray;
	fpreal rayTolerance = 1e-05;
	bool meshCap = false;
	bool reversePrim = false;

	
	GA_Attribute* dirAttrib = nullptr;

private:
	GU_DetailHandle geoCap_h;
	GU_Detail* geoCapTmp;
	//GA_Detail* geoCapTmp_GA;

	
	//GA_PrimitiveGroup* outPrimGroup;

	exint subscribeRatio = 64;
	exint minGrainSize = 1024;
	
}; // End of class GFE_GroupPolyByWinding



#endif
