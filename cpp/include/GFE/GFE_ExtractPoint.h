
#pragma once

#ifndef __GFE_ExtractPoint_h__
#define __GFE_ExtractPoint_h__


#include "GFE/GFE_ExtractPoint.h"



#include "GFE/GFE_GeoFilter.h"



class GFE_ExtractPoint : public GFE_GeoFilter {

public:
    using GFE_GeoFilter::GFE_GeoFilter;
    
    //GFE_ExtractPoint(
    //    GFE_Detail* const inGeo,
    //    const GA_Detail* const inGeoSrc = nullptr,
    //    const SOP_NodeVerb::CookParms* const cookparms = nullptr
    //)
    //    : GFE_GeoFilter(inGeo, inGeoSrc, cookparms)
    //    //, groupParserIn0(geoSrc ? geoSrc : static_cast<const GA_Detail*>(inGeo), groupParser.getGOPRef(), cookparms)
    //{
    //    if (geoSrc)
    //    {
    //        geoSrcGUTmp = new GU_Detail();
    //        geoSrcTmp_h.allocateAndSet(geoSrcGUTmp);
    //        geoSrcTmp = geoSrcGUTmp;
    //        geoSrcTmp->replaceWithPoints(*geoSrc);
    //    }
    //    else
    //    {
    //        geoSrcTmp = geo;
    //    }
    //}
	//
    //GFE_ExtractPoint(
    //    GA_Detail& inGeo,
    //    const GA_Detail* const inGeoSrc = nullptr,
    //    const SOP_NodeVerb::CookParms* const cookparms = nullptr
    //)
    //    : GFE_GeoFilter(inGeo, inGeoSrc, cookparms)
    //    //, groupParserIn0(geoSrc ? geoSrc : &inGeo, groupParser.getGOPRef(), cookparms)
    //{
    //    if (geoSrc)
    //    {
    //        geoSrcGUTmp = new GU_Detail();
    //        geoSrcTmp_h.allocateAndSet(geoSrcGUTmp);
    //        geoSrcTmp = geoSrcGUTmp;
    //        geoSrcTmp->replaceWithPoints(*geoSrc);
    //    }
    //    else
    //    {
    //        geoSrcTmp = geo;
    //    }
    //}
	

    void
        setComputeParm(
            const bool reverseGroup = false,
            const bool delInputGroup = true
            //,const exint subscribeRatio = 64,
            //const exint minGrainSize = 64
        )
    {
        setHasComputed();
        this->reverseGroup = reverseGroup;
        this->delInputGroup = delInputGroup;
        //this->subscribeRatio = subscribeRatio;
        //this->minGrainSize = minGrainSize;
    }
    
    // void setDeleteParm(
    //     const GA_Detail::GA_DestroyPointMode delPointMode = GA_Detail::GA_DESTROY_DEGENERATE_INCOMPATIBLE,
    //     const bool guaranteeNoVertexReference = false
    // )
    // {
    //     this->delPointMode = delPointMode;
    //     this->guaranteeNoVertexReference = guaranteeNoVertexReference;
    // }

    SYS_FORCE_INLINE void setKernel(const uint8 kernel)
    { this->kernel = kernel; }
    

    //SYS_FORCE_INLINE void setGroup(const GA_GroupType groupType, const UT_StringRef& groupName)
    //{ groupParserIn0.setGroup(groupType, groupName); }

    //SYS_FORCE_INLINE void setGroup(const UT_StringRef& groupName)
    //{ setGroup(GA_GROUP_POINT, groupName); }


    SYS_FORCE_INLINE virtual void bumpDataIdsForAddOrRemove() const override
    { geo->bumpDataIdsForAddOrRemove(true, false, false); }
    
    
private:

    void keepStdGroup(const char* const pattern)
    { 
        if (!pattern || pattern == "*")
            return;
        GA_GroupTable& groupTable = *geoSrcTmp->getGroupTable(GA_GROUP_POINT);
        for (GA_GroupTable::iterator<GA_Group> it = groupTable.beginTraverse(); !it.atEnd(); ++it)
        {
            GA_Group* const group = it.group();
            //if (group->isDetached())
            //    continue;
            if (group->getName().multiMatch(pattern))
                continue;
            if (group->getName() == pointGroup->getName())
                continue;
            groupTable.destroy(group);
        }
    }

    SYS_FORCE_INLINE void keepStdGroup(const GA_GroupType groupType, const char* keepGroupPattern)
    { GFE_Group::keepStdGroup(*geoSrcTmp->getGroupTable(groupType), keepGroupPattern); }

    void keepStdGroup(
        const char* const primGroupPattern,
        const char* const pointGroupPattern,
        const char* const vertexGroupPattern,
        const char* const edgeGroupPattern
    )
    {
        keepStdGroup(GA_GROUP_PRIMITIVE, primGroupPattern);
        if (pointGroup && !pointGroup->isDetached())
            keepStdGroup(pointGroupPattern);
        else
            keepStdGroup(GA_GROUP_POINT,     pointGroupPattern);
        keepStdGroup(GA_GROUP_VERTEX,    vertexGroupPattern);
        keepStdGroup(GA_GROUP_EDGE,      edgeGroupPattern);
    }
    
    virtual bool
        computeCore() override
    {
        GU_DetailHandle geoSrcTmp_h;
        if (geoSrc)
        {
            GU_Detail* const geoSrcGUTmp = new GU_Detail();
            geoSrcTmp_h.allocateAndSet(geoSrcGUTmp);
            geoSrcTmp = geoSrcGUTmp;
            geoSrcTmp->replaceWithPoints(*geoSrc);
        }
        else
        {
            geoSrcTmp = geo;
        }
        
        pointGroup = groupParser.isPointDetached() ? groupParser.getPointGroup() : geoSrcTmp->findPointGroup(groupParser.getName());
        
        GFE_AttributeDelete::keepStdAttribute(geoSrcTmp->getAttributes(), keepPrimAttrib, keepPointAttrib, keepVertexAttrib, keepDetailAttrib);
        keepStdGroup(keepPrimAttrib, keepPointAttrib, keepVertexAttrib, keepDetailAttrib);
        
        if (geoSrc)
            extractPointWithSource();
        else
            extractPoint();
        
        if (delInputGroup && pointGroup && !pointGroup->isDetached())
            geo->getGroupTable(GA_GROUP_POINT)->destroy(pointGroup->getName());
        
        //if (delInputGroup && pointGroupNonConst)
        //    geo->destroyGroup(pointGroupNonConst);
        
        return true;
    }

    void extractPointWithSource()
    {
        if (!pointGroup)
        {
            if (reverseGroup)
                geo->clearElement();
            else
                geo->replaceWithPoints(*geoSrcTmp);
            return;
        }
        if (pointGroup->isEmpty())
        {
            if (reverseGroup)
                geo->replaceWithPoints(*geoSrcTmp);
            else
                geo->clearElement();
            return;
        }
        if (pointGroup->entries() == pointGroup->getIndexMap().indexSize())
        {
            if (reverseGroup)
                geo->clearElement();
            else
                geo->replaceWithPoints(*geoSrcTmp);
            return;
        }
        
        switch (kernel)
        {
        case 0:
            //geo->asGEO_Detail()->mergePoints(*srcGeo, pointGroup, false, false);
            geo->asGEO_Detail()->mergePoints(static_cast<const GEO_Detail&>(*geoSrcTmp), GA_Range(geoSrcTmp->getPointMap(), pointGroup, reverseGroup));
            break;
        case 1:
            geo->replaceWithPoints(*geoSrcTmp);
            geo->destroyPointOffsets(GA_Range(geo->getPointMap(), pointGroup, !reverseGroup), GA_Detail::GA_DestroyPointMode::GA_LEAVE_PRIMITIVES, true);
            break;
        case 2:
        {
            geo->replaceWithPoints(*geoSrcTmp);
            GA_OffsetList offList = geo->getOffsetList(pointGroup, !reverseGroup);
            GA_IndexMap& ptmap = geo->getIndexMap(GA_ATTRIB_POINT);

            for (GA_Size arrayi = 0; arrayi < offList.size(); ++arrayi)
            {
                ptmap.destroyOffset(offList[arrayi]);
            }
        }
            break;
        case 3:
        {
            if (reverseGroup)
            {
                if (pointGroup->entries() <= 4096)
                {
                    geo->replaceWithPoints(*geoSrcTmp);
                    geo->destroyPointOffsets(GA_Range(geo->getPointMap(), pointGroup, !reverseGroup), GA_Detail::GA_DestroyPointMode::GA_LEAVE_PRIMITIVES, true);
                }
                else
                {
                    //geo->clearElement();
                    geo->asGEO_Detail()->mergePoints(*static_cast<const GEO_Detail*>(geoSrcTmp), pointGroup, false, false);
                }
            }
            else
            {
                if (pointGroup->entries() >= geo->getNumPoints() - 4096)
                {
                    geo->replaceWithPoints(*geoSrcTmp);
                    geo->destroyPointOffsets(GA_Range(geo->getPointMap(), pointGroup, !reverseGroup), GA_Detail::GA_DestroyPointMode::GA_LEAVE_PRIMITIVES, true);
                }
                else
                {
                    //geo->clearElement();
                    geo->asGEO_Detail()->mergePoints(*static_cast<const GEO_Detail*>(geoSrcTmp), pointGroup, false, false);
                }
            }
        }
            break;
        default:
            break;
        }
        //geo->destroyPointOffsets(GA_Range(geo->getPointMap(), pointGroup), GA_Detail::GA_DestroyPointMode::GA_LEAVE_PRIMITIVES, true);
        //geo->destroyUnusedPoints(pointGroup);
    }



    // can not use in parallel unless for each GA_Detail
    void extractPoint()
    {
        if (!pointGroup)
        {
            if (reverseGroup)
                geo->clearElement();
            // else
            //     geo->replaceWithPoints(*geo);
            return;
        }
        if (pointGroup->isEmpty())
        {
            if (!reverseGroup)
                geo->clearElement();
            // else
            //     geo->replaceWithPoints(*geo);
            return;
        }
        if (pointGroup->entries() == pointGroup->getIndexMap().indexSize())
        {
            if (reverseGroup)
                geo->clearElement();
            // else
            //     geo->replaceWithPoints(*geo);
            return;
        }
        
        switch (kernel)
        {
        case 0:
            geo->destroyPointOffsets(GA_Range(geo->getPointMap(), pointGroup, !reverseGroup), delPointMode, guaranteeNoVertexReference);
        break;
        case 1:
        {
            GA_OffsetList offList = geo->getOffsetList(pointGroup, !reverseGroup);
            GA_IndexMap& ptmap = geo->getIndexMap(GA_ATTRIB_POINT);

            for (GA_Size arrayi = 0; arrayi < offList.size(); ++arrayi)
            {
                ptmap.destroyOffset(offList[arrayi]);
            }
        }
        break;
        case 2:
        {
            if (reverseGroup)
            {
                if (pointGroup->entries() <= 4096)
                {
                    geo->destroyPointOffsets(GA_Range(geo->getPointMap(), pointGroup, !reverseGroup), delPointMode, guaranteeNoVertexReference);
                }
                else
                {
                    //geo->asGEO_Detail()->mergePoints(static_cast<const GEO_Detail&>(*geo->asGEO_Detail()), pointGroup, false, false);
                }
            }
            else
            {
                if (pointGroup->entries() >= geo->getNumPoints() - 4096)
                {
                    geo->destroyPointOffsets(GA_Range(geo->getPointMap(), pointGroup, !reverseGroup), delPointMode, guaranteeNoVertexReference);
                }
                else
                {
                    //geo->asGEO_Detail()->mergePoints(static_cast<const GEO_Detail&>(*geo->asGEO_Detail()), pointGroup, false, false);
                }
            }
        }
            break;
        default:
            break;
        }
        //geo->destroyPointOffsets(GA_Range(geo->getPointMap(), pointGroup), GA_Detail::GA_DestroyPointMode::GA_LEAVE_PRIMITIVES, true);
        //geo->destroyUnusedPoints(pointGroup);
    }

public:
    bool reverseGroup = false;
    bool delInputGroup = true;
    
    const char* keepPrimAttrib   = nullptr;
    const char* keepPointAttrib  = nullptr;
    const char* keepVertexAttrib = nullptr;
    const char* keepDetailAttrib = nullptr;
    const char* keepPrimGroup    = nullptr;
    const char* keepPointGroup   = nullptr;
    const char* keepVertexGroup  = nullptr;
    const char* keepEdgeGroup    = nullptr;
        
private:
    //GFE_GroupParser groupParserIn0;
    
    GA_Detail* geoSrcTmp;
    
    const GA_PointGroup* pointGroup;

    
    GA_Detail::GA_DestroyPointMode delPointMode = GA_Detail::GA_LEAVE_PRIMITIVES;
    bool guaranteeNoVertexReference = true;

    //GA_PointGroup* pointGroupNonConst = nullptr;
    //bool hasSetGroup = false;

    uint8 kernel = 0;

    //exint subscribeRatio = 64;
    //exint minGrainSize = 1024;


}; // End of class GFE_ExtractPoint





#endif
