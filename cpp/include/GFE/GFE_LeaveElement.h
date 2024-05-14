
#pragma once

#ifndef __GFE_LeaveElement_h__
#define __GFE_LeaveElement_h__



#include "GFE/GFE_LeaveElement.h"






#include "GFE/GFE_GeoFilter.h"



class GFE_LeaveElement : public GFE_GeoFilter {

public:
	//using GFE_GeoFilter::GFE_GeoFilter;
	
	GFE_LeaveElement(
		GFE_Detail* const geo,
		const GA_Detail* const inGeo0 = nullptr,
		const SOP_NodeVerb::CookParms* const cookparms = nullptr
	)
		: GFE_GeoFilter(geo, cookparms)
		, inGeo(inGeo0)
	{
	}

	GFE_LeaveElement(
		GA_Detail& geo,
		const GA_Detail* const inGeo = nullptr,
		const SOP_NodeVerb::CookParms* const cookparms = nullptr
	)
		: GFE_GeoFilter(geo, cookparms)
		, inGeo(inGeo)
	{
	}

	GFE_LeaveElement(
		GA_Detail& geo,
		const GA_Detail* const inGeo,
		const SOP_NodeVerb::CookParms& cookparms
	)
		: GFE_GeoFilter(geo, cookparms)
		, inGeo(inGeo)
	{
	}

	
    void
        setComputeParm(
			const GA_Detail::GA_DestroyPointMode delPointMode = GA_Detail::GA_LEAVE_PRIMITIVES,
            const bool delUnusedPoint = true,
            const bool guaranteeNoVertexReference = false,
            const bool reverseGroup = false,
			const bool delGroup = true
        )
    {
        setHasComputed();
        this->delPointMode = delPointMode;
        this->delUnusedPoint = delUnusedPoint;
        this->guaranteeNoVertexReference = guaranteeNoVertexReference;
        this->reverseGroup = reverseGroup;
        this->delGroup = delGroup;
    }






private:

    virtual bool
        computeCore() override
    {
        if (!reverseGroup && groupParser.isEmpty())
            return true;

    	switch (groupParser.classType())
    	{
    	case GA_GROUP_PRIMITIVE: delElement(groupParser.getPrimitiveGroup()); break;
    	case GA_GROUP_POINT:     delElement(groupParser.getPointGroup());     break;
    	case GA_GROUP_VERTEX:    delElement(groupParser.getVertexGroup());    break;
    	case GA_GROUP_EDGE:      delElement(groupParser.getEdgeGroup());      break;
    	default:                 delElement();                                break;
    	}
    	
    	if (delGroup)
    		groupParser.delGroup();
    	
        return true;
    }


	void delElement()
	{
    	if (inGeo)
    	{
    		if (reverseGroup)
    		{
    			geo->replaceWith(*inGeo);
    			return;
    		}
    	}
    	else
    	{
    		if (reverseGroup)
    			return;
    		
    		if (delPointMode)
    			//geo->destroyPointOffsets(GA_Range(geo->getPointMap(), nullptr), delPointMode, guaranteeNoVertexReference);
    			geo->destroyPrimitives(geo->getPrimitiveRange(), delUnusedPoint);
    		else
    		{
    			GA_Topology& topo = geo->getTopology();
    			GA_Offset start, end;
    			for (GA_Iterator it(geo->getPrimitiveRange()); it.blockAdvance(start, end); )
    			{
    				for (GA_Offset primoff = start; primoff < end; ++primoff)
    				{
    					const GA_OffsetListRef& vertices = geo->getPrimitiveVertexList(primoff);
    					for (GA_Size i = 0; i < vertices.size(); i++)
    					{
    						geo->getPrimitive(primoff)->releaseVertex(vertices[i]);
    						topo.delVertex(vertices[i]);
    					}
    				}
    			}
    			if (delUnusedPoint)
    				geo->destroyUnusedPoints();
    		}
    	}
	}


	void delElement(const GA_PrimitiveGroup* const group)
    {
    	if (inGeo)
    	{
    	}
    	else
    	{
    	}
    	if (!group)
			return delElement();

    	const GA_Range range(geo->getPrimitiveMap(), group, reverseGroup);
    	geo->destroyPrimitives(range, delUnusedPoint);
    	
		if (delPointMode)
			geo->destroyPrimitives(geo->getPrimitiveRange(group, reverseGroup), delUnusedPoint);
		else
		{
			GA_PointGroupUPtr pointGroupUPtr;
			GA_PointGroup* pointGroup;
			if (delUnusedPoint)
			{
				pointGroupUPtr = geo->createDetachedPointGroup();
				pointGroup = pointGroupUPtr.get();
			}

			GA_Topology& topo = geo->getTopology();
			const GA_ATITopology* const vtxPointRef = topo.getPointRef();

			GA_Offset start, end;
			for (GA_Iterator it(geo->getPrimitiveRange(group, reverseGroup)); it.blockAdvance(start, end); )
			{
				for (GA_Offset primoff = start; primoff < end; ++primoff)
				{
					const GA_OffsetListRef& vertices = geo->getPrimitiveVertexList(primoff);
					for (GA_Size i = 0; i < vertices.size(); i++)
					{
						if (delUnusedPoint)
							pointGroup->setElement(vtxPointRef->getLink(vertices[i]), true);
						geo->getPrimitive(primoff)->releaseVertex(vertices[i]);
						topo.delVertex(vertices[i]);
					}
				}
			}
			if (delUnusedPoint)
				geo->destroyUnusedPoints(pointGroup);
		}
	}


	void delElement(const GA_PointGroup* const group)
    {
    	if (inGeo)
    	{
    	}
    	else
    	{
    	}
		if (!group)
			return delElement();

    	geo->destroyPointOffsets(GA_Range(geo->getPointMap(), group, reverseGroup), delPointMode, guaranteeNoVertexReference);

    	
		if (delUnusedPoint)
		{
			geo->destroyPointOffsets(geo->getPointRange(group, reverseGroup), delPointMode);
		}
		else
		{
			GA_PrimitiveGroupUPtr primGroupUPtr;
			GA_PrimitiveGroup* primGroup;
			if (delPointMode)
			{
				primGroupUPtr = geo->createDetachedPrimitiveGroup();
				primGroup = primGroupUPtr.get();
			}

			GA_Topology& topo = geo->getTopology();
			const GA_ATITopology* const pointVtxRef = topo.getVertexRef();
			const GA_ATITopology* const vtxPrimRef = topo.getPrimitiveRef();
			const GA_ATITopology* const vtxNextRef = topo.getVertexNextRef();

			GA_Offset start, end;
			for (GA_Iterator it(geo->getPointRange(group, reverseGroup)); it.blockAdvance(start, end); )
			{
				for (GA_Offset ptoff = start; ptoff < end; ++ptoff)
				{
					for (GA_Offset vtxoff_next = pointVtxRef->getLink(ptoff); vtxoff_next != GA_INVALID_OFFSET; vtxoff_next = vtxNextRef->getLink(vtxoff_next))
					{
						GA_Offset primoff = vtxPrimRef->getLink(vtxoff_next);

						geo->getPrimitive(primoff)->releaseVertex(vtxoff_next);
						topo.delVertex(vtxoff_next);

						if (delPointMode)
							primGroup->setElement(primoff, true);
					}
				}
			}
			if (delPointMode)
				geo->destroyPrimitives(geo->getPrimitiveRange(primGroup));
		}
	}





	void delElement(const GA_VertexGroup* const group)
    {
    	if (inGeo)
    	{
    	}
    	else
    	{
    	}
		if (!group)
			return delElement();

    	geo->destroyVertexOffsets(GA_Range(geo->getVertexMap(), group, reverseGroup));
    	
		GA_Topology& topo = geo->getTopology();
		const GA_ATITopology* const vtxPrimRef = topo.getPrimitiveRef();
		const GA_ATITopology* const vtxPointRef = topo.getPointRef();


		GA_PrimitiveGroupUPtr primGroupUPtr;
		GA_PrimitiveGroup* primGroup;
		if (delPointMode)
		{
			primGroupUPtr = geo->createDetachedPrimitiveGroup();
			primGroup = primGroupUPtr.get();
		}

		GA_PointGroupUPtr pointGroupUPtr;
		GA_PointGroup* pointGroup;
		if (delUnusedPoint)
		{
			pointGroupUPtr = geo->createDetachedPointGroup();
			pointGroup = pointGroupUPtr.get();
		}
    	
		GA_Offset start, end;
		for (GA_Iterator it(geo->getVertexRange(group, reverseGroup)); it.blockAdvance(start, end); )
		{
			for (GA_Offset vtxoff = start; vtxoff < end; ++vtxoff)
			{
				GA_Offset primoff = vtxPrimRef->getLink(vtxoff);

				geo->getPrimitive(primoff)->releaseVertex(vtxoff);
				topo.delVertex(vtxoff);

				if (delPointMode)
					primGroup->setElement(primoff, true);
				if (delUnusedPoint)
					pointGroup->setElement(vtxPointRef->getLink(vtxoff), true);
			}
		}

		if (delPointMode)
			geo->destroyDegeneratePrimitives(primGroup);
		if (delUnusedPoint)
			geo->destroyUnusedPoints(pointGroup);
	}


	void delElement(const GA_EdgeGroup* const group)
    {
    	if (inGeo)
    	{
    	}
    	else
    	{
    	}
		if (!group)
			return delElement();

		const GA_VertexGroupUPtr vtxGroupUPtr = geo->createDetachedVertexGroup();
		GA_VertexGroup* vtxGroup = vtxGroupUPtr.get();
		vtxGroup->combine(group);
		delElement(vtxGroup);
	}


	void leaveElement()
    {
    	geo->clear();
    	if (!group)
    	{
    		if (reverse)
    		{
    			geo->replaceWith(*srcGeo);
    		}
    		return;
    		//geo->destroyPointOffsets(GA_Range(geo->getPointMap(), nullptr));
    		//geo->clearTopologyAttributes();
    		//geo->createTopologyAttributes();
    	}
    	switch (group->classType())
    	{
    	case GA_GROUP_PRIMITIVE:
    		{
    			static_cast<GEO_Detail*>(geo)->merge(*static_cast<const GEO_Detail*>(srcGeo), static_cast<const GA_PrimitiveGroup*>(group));
    		}
    		break;
    	case GA_GROUP_POINT:
    		static_cast<GEO_Detail*>(geo)->mergePoints(*static_cast<const GEO_Detail*>(srcGeo), static_cast<const GA_PointGroup*>(group));
    		break;
    	case GA_GROUP_VERTEX:
    		UT_ASSERT_MSG(0, "not possible");
    		break;
    	case GA_GROUP_EDGE:
    		UT_ASSERT_MSG(0, "not possible");
    		break;
    	default:
    		UT_ASSERT_MSG(0, "not possible");
    		break;
    	}
    	if (delGroup)
    	{
    		geo->destroyGroup(group);
    		group = nullptr;
    	}
    }






public:
    GA_Detail::GA_DestroyPointMode delPointMode = GA_Detail::GA_LEAVE_PRIMITIVES;
    bool delUnusedPoint = true;
    bool guaranteeNoVertexReference = false;
	
    bool reverseGroup = false;
    bool delGroup = true;
	
private:

    //exint subscribeRatio = 64;
    //exint minGrainSize = 1024;


}; // End of class GFE_LeaveElement


#endif
