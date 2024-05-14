
#pragma once

#ifndef __GFE_DelElement_h__
#define __GFE_DelElement_h__


#include "GFE/GFE_DeleteElement.h"



#include "GFE/GFE_GeoFilter.h"

enum class GFE_DelElementMethod
{
	Delete,
	Leave,
	Auto,
};

class GFE_DelElement : public GFE_GeoFilter {

public:
	using GFE_GeoFilter::GFE_GeoFilter;
	
	//GFE_DelElement(
	//	GFE_Detail* const geo,
	//	const GA_Detail* const geoSrc = nullptr,
	//	const SOP_NodeVerb::CookParms* const cookparms = nullptr
	//)
	//	: GFE_GeoFilter(geo, geoSrc, cookparms)
	//	, groupParserIn0(geoSrc ? geoSrc : static_cast<const GA_Detail*>(geo), groupParser.getGOPRef(), cookparms)
	//{
	//}
	//
	//GFE_DelElement(
	//	GA_Detail& geo,
	//	const GA_Detail* const geoSrc = nullptr,
	//	const SOP_NodeVerb::CookParms* const cookparms = nullptr
	//)
	//	: GFE_GeoFilter(geo, geoSrc, cookparms)
	//	, groupParserIn0(geoSrc ? geoSrc : &geo, groupParser.getGOPRef(), cookparms)
	//{
	//}
	
	
    void
        setComputeParm(
			const GFE_DelElementMethod method = GFE_DelElementMethod::Delete,
			const bool reverseGroup = false,
			const bool delGroup = true,
			const GA_Detail::GA_DestroyPointMode delPointMode = GA_Detail::GA_LEAVE_PRIMITIVES,
            const bool delUnusedPoint = true,
            const bool guaranteeNoVertexReference = false
        )
    {
    	setHasComputed();
    	this->method         = method;
    	this->reverseGroup   = reverseGroup;
    	this->delGroup       = delGroup;
        this->delPointMode   = delPointMode;
        this->delUnusedPoint = delUnusedPoint;
        this->guaranteeNoVertexReference = guaranteeNoVertexReference;
    }

	//SYS_FORCE_INLINE const GA_Group* setGroup(const GA_GroupType groupType, const UT_StringRef& groupName)
    //{ return groupParserIn0.setGroup(groupType, groupName); }






private:

    virtual bool
        computeCore() override
    {
        //if ((leaveElement ^ reverseGroup) ^ groupParserIn0.isEmpty())
        //    return true;
    	
    	switch (method)
    	{
    	case GFE_DelElementMethod::Delete: leaveElement = false; break;
    	case GFE_DelElementMethod::Leave : leaveElement = true;  break;
    	case GFE_DelElementMethod::Auto  :
    		leaveElement = groupParser.entries() > groupParser.getNumElements() / 2;
    		break;
    	}
    	if (leaveElement)
    		reverseGroup = !reverseGroup;
    	
    	if (leaveElement && geoSrc)
    	{
    		geoGU = geo->asGU_Detail();
    		switch (groupParser.classType())
    		{
    		case GA_GROUP_PRIMITIVE: leaveElementFunc(groupParser.getPrimitiveGroup()); break;
    		case GA_GROUP_POINT:     leaveElementFunc(groupParser.getPointGroup());     break;
    		case GA_GROUP_VERTEX:    leaveElementFunc(groupParser.getVertexGroup());    break;
    		case GA_GROUP_EDGE:      leaveElementFunc(groupParser.getEdgeGroup());      break;
    		default:                 leaveElementFunc();                                   break;
    		}
    	}
        else
        {
    		//if (leaveElement)
			//	reverseGroup = !reverseGroup;

        	const GA_GroupType groupType = groupParser.classType();
        	switch (groupType)
        	{
        	case GA_GROUP_PRIMITIVE:
        	case GA_GROUP_POINT:
        	case GA_GROUP_VERTEX:
        	case GA_GROUP_EDGE:
        		if (geoSrc)
        			geo->replaceWith(*geoSrc);
				break;
        	default:
				break;
        	}
        	
        	switch (groupType)
        	{
        	case GA_GROUP_PRIMITIVE: delElement(groupParser.getPrimitiveGroup()); break;
        	case GA_GROUP_POINT:     delElement(groupParser.getPointGroup());     break;
        	case GA_GROUP_VERTEX:    delElement(groupParser.getVertexGroup());    break;
        	case GA_GROUP_EDGE:      delElement(groupParser.getEdgeGroup());      break;
        	default:                 delElement();                                break;
        	}
        	
        	//if (leaveElement)
        	//	reverseGroup = !reverseGroup;
        }
    	
    	if (delGroup)
    		groupParser.delGroup();
    	
    	//if (method == GFE_DelElementMethod::Auto)
    	//	reverseGroup = !reverseGroup;
    	
    	if (leaveElement)
    		reverseGroup = !reverseGroup;
    	
        return true;
    }

	
	void leaveElementFunc()
    {
    	if (reverseGroup)
    		geo->cloneAllMissingAttribGroup(geoSrc);
    	else
    		geo->replaceWith(*geoSrc);
    }
	
	void leaveElementFunc(const GA_PrimitiveGroup* const group)
    { geoGU->mergePrimitives(static_cast<const GEO_Detail&>(*geoSrc), GA_Range(group->getIndexMap(), group, reverseGroup)); }
	
	void leaveElementFunc(const GA_PointGroup* const group)
    {
    	geoGU->mergePoints(static_cast<const GEO_Detail&>(*geoSrc), GA_Range(group->getIndexMap(), groupParser.getPointGroup(), reverseGroup));
    	//const GA_PrimitiveGroupUPtr primGroupUPtr = geo->createDetachedPrimitiveGroup();
    	//GA_PrimitiveGroup* const primGroup = primGroupUPtr.get();
    	//primGroup->combine(group);
    	//leaveElementFunc(primGroup);
    	////geoGU->mergePoints(static_cast<const GEO_Detail&>(*geoSrc), group);
    	//reverseGroup = !reverseGroup;
    	//delElement(group);
    	//reverseGroup = !reverseGroup;
    }
	
	void leaveElementFunc(const GA_VertexGroup* const group)
    {
    	reverseGroup = !reverseGroup;
    	delElement(group);
    	reverseGroup = !reverseGroup;
    }
	
	void leaveElementFunc(const GA_EdgeGroup* const group)
    {
    	reverseGroup = !reverseGroup;
    	delElement(group);
    	reverseGroup = !reverseGroup;
    }
	
	void delElement()
	{
    	if (geoSrc)
    	{
    		if (reverseGroup)
    			geo->replaceWith(*geoSrc);
            else
    			geo->cloneAllMissingAttribGroup(geoSrc);
    		return;
    	}
    	if (reverseGroup)
    		return;

    	reverseGroup = !reverseGroup;
    	if (delPointMode)
    	{
    		//geo->destroyPointOffsets(GA_Range(geo->getPointMap(), nullptr), delPointMode, guaranteeNoVertexReference);
    		//geo->destroyPrimitives(geo->getPrimitiveRange(), delUnusedPoint);
            geo->clearElement();
    		return;
    	}
    	
    	GA_Topology& topo = geo->getTopology();
    	GA_Offset start, end;
    	for (GA_Iterator it(geo->getPrimitiveRange()); it.blockAdvance(start, end); )
    	{
    		for (GA_Offset primoff = start; primoff < end; ++primoff)
    		{
    			const GA_Size numvtx = geo->getPrimitiveVertexCount(primoff);
    			for (GA_Size vtxpnum = 0; vtxpnum < numvtx; ++vtxpnum)
    			{
    				const GA_Offset vtxoff = geo->getPrimitiveVertexOffset(primoff, vtxpnum);
    				geo->getPrimitive(primoff)->releaseVertex(vtxoff);
    				topo.delVertex(vtxoff);
    			}
    		}
    	}
    	if (delUnusedPoint)
    		geo->destroyUnusedPoints();
	}


	void delElement(const GA_PrimitiveGroup* const group)
    {
    	const GA_Range range = geo->getPrimitiveRange(group, reverseGroup);
		if (delPointMode)
		{
			geo->destroyPrimitives(range, delUnusedPoint);
			return;
		}
    	
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
		for (GA_Iterator it(range); it.blockAdvance(start, end); )
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


	void delElement(const GA_PointGroup* const group)
    {
		if (delUnusedPoint)
		{
			GA_Range range;
			
			GA_PointGroupUPtr pointGroupUPtr;
			if (reverseGroup)
			{
				pointGroupUPtr = geo->createDetachedPointGroup();
				GA_PointGroup* const reversedPointGroup = pointGroupUPtr.get();
				reversedPointGroup->combine(group);
				reversedPointGroup->toggleAll(geo->getNumPoints());
				range = geo->getPointRange(reversedPointGroup);
			}
			else
			{
				range = geo->getPointRange(group);
			}
			geo->destroyPointOffsets(range, delPointMode, guaranteeNoVertexReference);
			return;
		}
    	
    	const GA_Range range = geo->getPointRange(group, reverseGroup);
    	
		GA_PrimitiveGroupUPtr primGroupUPtr;
		GA_PrimitiveGroup* primGroup;
		if (delPointMode)
		{
			primGroupUPtr = geo->createDetachedPrimitiveGroup();
			primGroup = primGroupUPtr.get();
		}

		GA_Topology& topo = geo->getTopology();
		const GA_ATITopology* const pointVtxRef = topo.getVertexRef();
		const GA_ATITopology* const vtxPrimRef  = topo.getPrimitiveRef();
		const GA_ATITopology* const vtxNextRef  = topo.getVertexNextRef();

		GA_Offset start, end;
		for (GA_Iterator it(range); it.blockAdvance(start, end); )
		{
			for (GA_Offset ptoff = start; ptoff < end; ++ptoff)
			{
				GA_Offset vtxoff_next;
				for (GA_Offset vtxoff = pointVtxRef->getLink(ptoff); GFE_Type::isValidOffset(vtxoff); vtxoff = vtxoff_next)
				{
					const GA_Offset primoff = vtxPrimRef->getLink(vtxoff);
					
					vtxoff_next = vtxNextRef->getLink(vtxoff);
					
					geo->getPrimitive(primoff)->releaseVertex(vtxoff);
					topo.delVertex(vtxoff);

					vtxoff = vtxoff_next;
					
					if (delPointMode)
						primGroup->setElement(primoff, true);
				}
			}
		}
		if (delPointMode)
			geo->destroyDegeneratePrimitives(primGroup);
	}


	void delElement(const GA_VertexGroup* const group)
    {
    	const GA_Range range = geo->getVertexRange(group, reverseGroup);
    	geo->destroyVertexOffsets(range);
    	
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
		for (GA_Iterator it(range); it.blockAdvance(start, end); )
		{
			for (GA_Offset vtxoff = start; vtxoff < end; ++vtxoff)
			{
				const GA_Offset primoff = vtxPrimRef->getLink(vtxoff);

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
		const GA_VertexGroupUPtr vtxGroupUPtr = geo->createDetachedVertexGroup();
		GA_VertexGroup* const vtxGroup = vtxGroupUPtr.get();
		vtxGroup->combine(group);
		delElement(vtxGroup);
	}


public:
	GFE_DelElementMethod method = GFE_DelElementMethod::Delete;
	
	bool reverseGroup = false;
	bool delGroup = true;
	
    GA_Detail::GA_DestroyPointMode delPointMode = GA_Detail::GA_LEAVE_PRIMITIVES;
    bool delUnusedPoint = true;
    bool guaranteeNoVertexReference = false;
	
private:
	bool leaveElement = false;
	
	//GFE_GroupParser groupParserIn0;
	
	GU_Detail* geoGU;
	
    //exint subscribeRatio = 64;
    //exint minGrainSize = 1024;


}; // End of class GFE_DelElement


#endif
