
#pragma once

#ifndef __GFE_PointInBBox_h__
#define __GFE_PointInBBox_h__

#include "GFE/GFE_PointInBBox.h"

#include "GFE/GFE_GeoFilter.h"



#include "GFE/GFE_Bound.h"

class GFE_PointInBBox : public GFE_AttribFilterWithRef0 {

public:
	//using GFE_AttribFilterWithRef0::GFE_AttribFilterWithRef0;
	
	GFE_PointInBBox(
		GA_Detail& geo,
		const GA_Detail* const geoRef,
		const SOP_NodeVerb::CookParms* const cookparms = nullptr
	)
		: GFE_AttribFilterWithRef0(geo, geoRef, cookparms)
		, groupParserBound(geoRef ? geoRef : &geo, groupParser.getGOPRef(), cookparms)
	{
	}

	GFE_PointInBBox(
		GA_Detail& geo,
		const GA_Detail* const geoRef,
		const SOP_NodeVerb::CookParms& cookparms
	)
		: GFE_AttribFilterWithRef0(geo, geoRef, cookparms)
		, groupParserBound(geoRef ? geoRef : &geo, groupParser.getGOPRef(), cookparms)
	{
	}


	
    void
        setComputeParm(
			const char numInBoundMin = 1,
            const bool delElement = true,
			exint subscribeRatio = 64,
			exint minGrainSize = 1024
        )
    {
        setHasComputed();
        this->numInBoundMin = numInBoundMin;
        this->doDelGroupElement = delElement;
        this->subscribeRatio = subscribeRatio;
        this->minGrainSize = minGrainSize;
    }


#define GFE_PointInBBox_GETFUN_SPECILIZATION(NAME_LOWER, NAME_UPPER)                \
		public:                                                                     \
			bool NAME_LOWER = false;                                                \
			fpreal NAME_LOWER##Threshold = false;                                   \
		                                                                            \
		public:                                                                     \
			SYS_FORCE_INLINE void set##NAME_UPPER(const fpreal threshold)			\
			{ NAME_LOWER = true; NAME_LOWER##Threshold = threshold; }				\
																					\
			SYS_FORCE_INLINE void set##NAME_UPPER()									\
			{ NAME_LOWER = false; }													\

        
	GFE_PointInBBox_GETFUN_SPECILIZATION(xMin, XMin);
	GFE_PointInBBox_GETFUN_SPECILIZATION(xMax, XMax);
	GFE_PointInBBox_GETFUN_SPECILIZATION(yMin, YMin);
	GFE_PointInBBox_GETFUN_SPECILIZATION(yMax, YMax);
	GFE_PointInBBox_GETFUN_SPECILIZATION(zMin, ZMin);
	GFE_PointInBBox_GETFUN_SPECILIZATION(zMax, ZMax);
    
#undef GFE_PointInBBox_GETFUN_SPECILIZATION



private:

    virtual bool
        computeCore() override
    {
    	if (getOutGroupArray().isEmpty() || !getOutGroupArray()[0])
    		return true;
    	
    	if (!xMin && !xMax && !yMin && !yMax && !zMin && !zMax)
    		return true;

    	if (geo->getNumPoints() <= 0)
    		return true;
    	
    	if (!posAttrib)
    		posAttrib = geo->getP();
    	
    	if (geoRef0)
    	{
    		if (!posRefAttrib)
    			posRefAttrib = geoRef0->getP();
    		bbox = geoRef0->stdBoundingBox<float>(groupParserBound.getElementGroup(), posRefAttrib);
    	}
        else
        {
        	bbox = geo->stdBoundingBox<float>(groupParserBound.getElementGroup(), posAttrib);
        }
    	//GFE_Bound::expandBounds(bbox, enlargeBBox);
    	bbox(0,0) += xMinThreshold;
    	bbox(0,1) += xMaxThreshold;
    	bbox(1,0) += yMinThreshold;
    	bbox(1,1) += yMaxThreshold;
    	bbox(2,0) += zMinThreshold;
    	bbox(2,1) += zMaxThreshold;
    	
    	//bbox.enlargeBounds(enlargeBBox);
    	//bbox.enlargeFloats(enlargeBBox);
    	
    	groupSetter = getOutGroupArray()[0];

    	pointInBBox();
    	
    	
    	if (doDelGroupElement)
    		delGroupElement();

        return true;
    }


    	
	void pointInBBox()
    {
    	auto storageVariant = GFE_Variant::getAttribStorageVariantV3F(*posAttrib);
    	auto xMinVariant = GFE_Variant::getBoolVariant(xMin);
    	auto xMaxVariant = GFE_Variant::getBoolVariant(xMax);
    	auto yMinVariant = GFE_Variant::getBoolVariant(yMin);
    	auto yMaxVariant = GFE_Variant::getBoolVariant(yMax);
    	auto zMinVariant = GFE_Variant::getBoolVariant(zMin);
    	auto zMaxVariant = GFE_Variant::getBoolVariant(zMax);
    		
    	std::visit([&] (auto storageVariant,
						auto xMinVariant,
						auto xMaxVariant,
						auto yMinVariant,
						auto yMaxVariant,
						auto zMinVariant,
						auto zMaxVariant)
		{
			using type = typename GFE_Variant::getAttribStorage_t<storageVariant>;
			pointInBBox<type, xMinVariant, xMaxVariant, yMinVariant, yMaxVariant, zMinVariant, zMaxVariant>();
		}, storageVariant, xMinVariant, xMaxVariant, yMinVariant, yMaxVariant, zMinVariant, zMaxVariant);
    }

	template<typename _Ty, bool xMin, bool xMax, bool yMin, bool yMax, bool zMin, bool zMax>
	void pointInBBox()
    {
    	UTparallelFor(groupParser.getSplittableRange(posAttrib), [this](const GA_SplittableRange& r)
		{
			GFE_ROPageHandleT<_Ty> pos_ph(posAttrib);
			for (GA_PageIterator pit = r.beginPages(); !pit.atEnd(); ++pit)
			{
				GA_Offset start, end;
				for (GA_Iterator it(pit.begin()); it.blockAdvance(start, end); )
				{
					pos_ph.setPage(start);
					for (GA_Offset elemoff = start; elemoff < end; ++elemoff)
					{
						uint8 numInLimit = 0;
						if constexpr (xMin)
							if (pos_ph.value(elemoff)[0] >= bbox.xmin())
								++numInLimit;
						if constexpr (xMax)
							if (xMax && pos_ph.value(elemoff)[0] <= bbox.xmax())
								++numInLimit;
						if constexpr (yMin)
							if (yMin && pos_ph.value(elemoff)[1] >= bbox.ymin())
								++numInLimit;
						if constexpr (yMax)
							if (yMax && pos_ph.value(elemoff)[1] <= bbox.ymax())
								++numInLimit;
						if constexpr (zMin)
							if (zMin && pos_ph.value(elemoff)[2] >= bbox.zmin())
								++numInLimit;
						if constexpr (zMax)
							if (zMax && pos_ph.value(elemoff)[2] <= bbox.zmax())
								++numInLimit;
							
						groupSetter.set(elemoff, numInLimit >= numInBoundMin);
					}
				}
			}
		}, subscribeRatio, minGrainSize);
    }


public:
	GFE_GroupParser groupParserBound;
	
	//UT_BoundingBoxT<float> enlargeBBox = UT_BoundingBoxT<float>(0, 0, 0, 0, 0, 0);
	//
	// bool xMin = false;
	// bool xMax = false;
	// bool yMin = false;
	// bool yMax = false;
	// bool zMin = false;
	// bool zMax = false;

private:
	
	
	//uint8 numInLimit = 0;
	
	uint8 numInBoundMin = 1;
	
	//const GA_Detail* geoSrcTmp;
	//GU_Detail* geoSrcTmpGU;
	
	//GU_DetailHandle geoSrcTmp_h;
	
	UT_BoundingBoxT<float> bbox;
	
	const GA_Attribute* posRefAttrib = nullptr;
	
    exint subscribeRatio = 64;
    exint minGrainSize = 1024;


}; // End of class GFE_PointInBBox


#endif
