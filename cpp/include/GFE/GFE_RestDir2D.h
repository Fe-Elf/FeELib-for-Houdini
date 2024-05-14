
#pragma once

#ifndef __GFE_RestDir2D_h__
#define __GFE_RestDir2D_h__

#include "GFE/GFE_RestDir2D.h"


#include "UT/UT_OBBox.h"



#include "GFE/GFE_GeoFilter.h"


#include "GFE/GFE_Normal3D.h"


enum class GFE_RestDir2DMethod
{
	AvgNormal,
	HouOBB,
};






class GFE_RestDir2D : public GFE_AttribFilter {

public:
	//using GFE_AttribFilter::GFE_AttribFilter;
	
	GFE_RestDir2D(
		GFE_Detail* const geo,
		const SOP_NodeVerb::CookParms* const cookparms = nullptr
	)
		: GFE_AttribFilter(geo, cookparms)
		, normal3D(geo, cookparms)
	{
	}

	GFE_RestDir2D(
		GA_Detail& geo,
		const SOP_NodeVerb::CookParms* const cookparms = nullptr
	)
		: GFE_AttribFilter(geo, cookparms)
		, normal3D(geo, cookparms)
	{
	}

	~GFE_RestDir2D()
	{
	}

	void
		setComputeParm(
			const GFE_RestDir2DMethod method = GFE_RestDir2DMethod::AvgNormal,
			const exint subscribeRatio = 64,
			const exint minGrainSize = 1024
		)
	{
		setHasComputed();
		//setInAttribBumpDataId(method == GFE_RestDir2DMethod::AvgNormal);
		//getInAttribArray().findOrCreateTuple();
		this->method = method;
		this->subscribeRatio = subscribeRatio;
		this->minGrainSize = minGrainSize;
	}
	
	SYS_FORCE_INLINE void setMatchUpDir(const UT_Vector3D& up)
	{
		matchUpDir = true;
		this->up = up;
	}
	
	SYS_FORCE_INLINE void setMatchUpDir()
	{ matchUpDir = false; }
	
	
	SYS_FORCE_INLINE GA_Attribute* findOrCreateDir(
		const bool detached = false,
		const GA_Storage storage = GA_STORE_INVALID,
		const UT_StringRef& attribName = "")
	{ return getOutAttribArray().findOrCreateDir(detached, GA_ATTRIB_DETAIL, storage, attribName); }
	
	SYS_FORCE_INLINE const UT_Vector3T<fpreal>& getRestDir() const
	{ return restDir; }

	


private:


	virtual bool
		computeCore() override
	{
		if (groupParser.isEmpty())
			return true;
		
		switch (method)
		{
			case GFE_RestDir2DMethod::AvgNormal: restDir2D_avgNormal(); break;
			case GFE_RestDir2DMethod::HouOBB:    restDir2D_houOBB();    break;
		}

		if (matchUpDir && restDir.dot(up) < 0)
			restDir *= -1;
		
		if (!getOutAttribArray().isEmpty())
		{
			GA_Attribute* const outAttrib = getOutAttribArray()[0];
#if 1
			outAttrib->getAIFTuple()->set(outAttrib, 0, restDir[0], 0);
			outAttrib->getAIFTuple()->set(outAttrib, 0, restDir[1], 1);
			outAttrib->getAIFTuple()->set(outAttrib, 0, restDir[2], 2);
#else
			switch (outAttrib->getAIFTuple()->getStorage(outAttrib))
			{
			case GA_STORE_REAL32:
			{
				const GA_RWHandleT<UT_Vector3T<fpreal32>> attrib_h(outAttrib);
				attrib_h.set(0, restDir);
			}
			break;
			case GA_STORE_REAL64:
			{
				const GA_RWHandleT<UT_Vector3T<fpreal64>> attrib_h(outAttrib);
				attrib_h.set(0, restDir);
			}
			break;
			default: UT_ASSERT_MSG(0, "unhandled storage"); break;
			}
#endif
		}
		
		return true;
	}


	void restDir2D_avgNormal()
	{
		normal3D.groupParser.setGroup(groupParser);

		if (!normal3D.getAttrib())
			normal3D.findOrCreateNormal3D();

		normal3D.compute();
		
		const GA_Attribute* const normalAttrib = normal3D.getAttrib();
		const GA_Storage storage = normalAttrib ? normalAttrib->getAIFTuple()->getStorage(normalAttrib) : geo->getPreferredStorageF();;
		switch (storage)
		{
		case GA_STORE_REAL16: restDir2D_avgNormal<fpreal16>(); break;
		case GA_STORE_REAL32: restDir2D_avgNormal<fpreal32>(); break;
		case GA_STORE_REAL64: restDir2D_avgNormal<fpreal64>(); break;
		}
	}

	template<typename FLOAT_T>
	void restDir2D_avgNormal()
	{
		const GA_Attribute* const normalAttrib = normal3D.getAttrib();
		ComputeDir2D_AvgNormal<FLOAT_T> body(geo, normalAttrib);
		UTparallelReduce(groupParser.getSplittableRange(normalAttrib), body, subscribeRatio, minGrainSize);
		restDir = body.getSum();
	}


	
	void restDir2D_houOBB()
	{
		UT_OBBoxD obb;
		geo->asGU_Detail()->getOBBoxForPrims(groupParser.getPrimitiveGroup(), obb);

		//geo->asGU_Detail()->setDetailAttributeI("minAbsAxis", minAbsAxis);
		restDir = obb.getMinAxis();
	}




template<typename FLOAT_T>
class ComputeDir2D_AvgNormal {
public:
	ComputeDir2D_AvgNormal(const GA_Detail* const geo, const GA_Attribute* const normal3DAttrib = nullptr)
		: myGeo(geo)
		, myNormal3DAttrib(normal3DAttrib)
		//, mySum(UT_Vector3T<FLOAT_T>(FLOAT_T(0)))
	{
		mySum = UT_Vector3T<FLOAT_T>(FLOAT_T(0.0));
		myWeight = 0;
	}
	ComputeDir2D_AvgNormal(ComputeDir2D_AvgNormal& src, UT_Split)
		: myGeo(src.myGeo)
		, myNormal3DAttrib(src.myNormal3DAttrib)
		//, mySum(UT_Vector3T<FLOAT_T>(FLOAT_T(0)))
	{
		mySum = UT_Vector3T<FLOAT_T>(FLOAT_T(0.0));
		myWeight = 0;
	}
	void operator()(const GA_SplittableRange& r)
	{
		myWeight = r.getEntries();
		//UT_Vector3T<FLOAT_T> sumTmp(FLOAT_T(0));
		if (myNormal3DAttrib)
		{
			GA_PageHandleT<UT_Vector3T<FLOAT_T>, FLOAT_T, true, false, const GA_Attribute, const GA_ATINumeric, const GA_Detail> normal_ph(myNormal3DAttrib);
			for (GA_PageIterator pit = r.beginPages(); !pit.atEnd(); ++pit)
			{
				GA_Offset start, end;
				for (GA_Iterator it(pit.begin()); it.blockAdvance(start, end); )
				{
					normal_ph.setPage(start);
					for (GA_Offset elemoff = start; elemoff < end; ++elemoff)
					{
						mySum += normal_ph.value(elemoff);
						//mySum += normal_ph.value(elemoff).normalize();
					}
				}
			}
		}
		else
		{
			GA_Offset start, end;
			for (GA_Iterator it(r); it.blockAdvance(start, end); )
			{
				for (GA_Offset elemoff = start; elemoff < end; ++elemoff)
				{
					//sumTmp += myGeo->getPrimitive(elemoff)->computeNormalD();
					if constexpr (std::is_same_v<FLOAT_T, fpreal64>)
						mySum += static_cast<const GEO_Primitive*>(myGeo->getPrimitive(elemoff))->computeNormalD();
					else
						mySum += static_cast<const GEO_Primitive*>(myGeo->getPrimitive(elemoff))->computeNormal();
				}
			}
		}
		mySum /= FLOAT_T(myWeight);
	}
	SYS_FORCE_INLINE void join(const ComputeDir2D_AvgNormal& other)
	{
		const GA_Size weightSum = myWeight += other.myWeight;
		const FLOAT_T weightSumReverse = FLOAT_T(1) / FLOAT_T(weightSum);
		
		mySum = mySum * (myWeight * weightSumReverse) + other.mySum * (other.myWeight * weightSumReverse);
		myWeight = weightSum;
	}
	SYS_FORCE_INLINE const UT_Vector3T<FLOAT_T>& getSum() const
	{ return mySum; }
private:
	UT_Vector3T<FLOAT_T> mySum;
	GA_Size myWeight;
	const GA_Detail* const myGeo;
	const GA_Attribute* const myNormal3DAttrib;
}; // End of Class ComputeDir2D_AvgNormal


public:
	GFE_Normal3D normal3D;
	GFE_RestDir2DMethod method = GFE_RestDir2DMethod::AvgNormal;

	bool matchUpDir = true;
	UT_Vector3D up = UT_Vector3D(0,1,0);
	
private:
	
	UT_Vector3T<fpreal> restDir;
	exint subscribeRatio = 64;
	exint minGrainSize = 1024;
	
	
}; // End of Class GFE_RestDir2D





#endif
