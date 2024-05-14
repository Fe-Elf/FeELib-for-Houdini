
#pragma once

#ifndef __GFE_PointInMeshWN_h__
#define __GFE_PointInMeshWN_h__

#include "GFE/GFE_PointInMeshWN.h"



// #include "GFE/GFE_GeoFilter.h"
#include "GFE/GFE_WindingNumber.h"
#include "GFE/GFE_AttributeCast.h"



#if 0

class GFE_PointInMeshWN : public GFE_WindingNumber {

public:

    using GFE_WindingNumber::GFE_WindingNumber; 


    void
        setOutGroup(
            const bool outGroupDetached = false,
            const UT_StringHolder& groupName = "pointInMesh_wn",
            const bool groupInGeoPoint = true,
            const bool groupOnGeoPoint = true,
            const fpreal threshold = 1e-05,
            const bool reverseGroup = false,
            const GFE_GroupMergeType groupMergeType = GFE_GroupMergeType::Replace
        )
    {
        hasParm_outGroup = true;
        this->outGroupDetached = outGroupDetached;
        this->groupName = groupName;
        this->groupInGeoPoint = groupInGeoPoint;
        this->groupOnGeoPoint = groupOnGeoPoint;
        this->threshold = threshold;
        this->reverseGroup = reverseGroup;
        this->groupMergeType = groupMergeType;
    }

    GA_PointGroup*
        addOutGroup()
    {
        if (!hasParm_inGroup)
            setInGroup();

        if (!gfeWN.getHasComputed())
            gfeWN.compute();

        if (!hasParm_outGroup)
            setOutGroup();

        if (outGroupDetached)
        {
            pointInMeshGroupUPtr = geoPoint->createDetachedPointGroup();
            pointInMeshGroup = pointInMeshGroupUPtr.get();
        }
        else
        {
            pointInMeshGroup = geoPoint->newPointGroup(groupName);
        }

        switch (gfeWN.getAttrib()->getAIFTuple()->getStorage(gfeWN.getAttrib()))
        {
        case GA_STORE_REAL16:
            computeOutGroup<fpreal16>();
            break;
        case GA_STORE_REAL32:
            computeOutGroup<fpreal32>();
            break;
        case GA_STORE_REAL64:
            computeOutGroup<fpreal64>();
            break;
        default:
            break;
        }

        return pointInMeshGroup;
    }



    void
        setNumericAttrib(
            const bool outNumericDetached = false,
            const GA_Storage numericStorage = GA_STORE_INVALID,
            const UT_StringHolder& numericAttribName = "pointInMesh_wn"
        )
    {
        hasParm_outNumeric = true;
        this->outNumericDetached = outNumericDetached;
        this->numericStorage = numericStorage;
        this->numericAttribName = numericAttribName;
    }

    GA_Attribute*
        addNumericAttrib()
    {
        if (!pointInMeshGroup)
            addOutGroup();

        if (!hasParm_outNumeric)
            setNumericAttrib();

        if (outNumericDetached)
        {
            numericAttribUPtr = geoPoint->createDetachedTupleAttribute(GA_ATTRIB_POINT, numericStorage, 1, GA_Defaults(0));
            numericAttrib = numericAttribUPtr.get();
        }
        else
        {
            numericAttrib = geoPoint->getAttributes().createTupleAttribute(GA_ATTRIB_POINT, numericAttribName, numericStorage, 1, GA_Defaults(0));
        }
        GFE_AttribCast attribCast(geo, cookparms);
        attribCast.getInAttribArray().set(, );
        attribCast.getInGroupArray() .set(, );
        attribCast.newStorageClass = GA_STORECLASS_INT;
        attribCast.compute();
        GFE_AttributeCopy::copyAttribute(numericAttrib, pointInMeshGroup);

        return numericAttrib;
    }


    void
        bumpDataId()
    {
        GFE_WindingNumber::bumpDataId();

        if (pointInMeshGroup && !outGroupDetached)
        {
            pointInMeshGroup->bumpDataId();
        }
        if (numericAttrib && !outNumericDetached)
        {
            numericAttrib->bumpDataId();
        }
    }

    void
        visualizeGroup() const
    {
        if (!outGroupDetached && cookparms && pointInMeshGroup)
        {
            cookparms->getNode()->setHighlight(true);
            cookparms->select(*pointInMeshGroup);
        }
    }


protected:


    virtual bool
        computeCore() override
    {
        addNumericAttrib();
        addOutGroup();
        
        if (!getOutAttribArray().size())
            findOrCreateTuple();
        
        if (groupParser.isEmpty())
            return true;

        for (int i = 0; i < getOutAttribArray().size(); i++)
        {
            attribPtr = getOutAttribArray()[i];
            const GA_AIFTuple* const aifTuple = attribPtr->getAIFTuple();
            if(!aifTuple)
                continue;
            
            switch (aifTuple->getStorage(attribPtr))
            {
            case GA_STORE_REAL16: computeWindingNumber<fpreal32>(); break;
            case GA_STORE_REAL32: computeWindingNumber<fpreal32>(); break;
            case GA_STORE_REAL64: computeWindingNumber<fpreal64>(); break;
            default: break;
            }
        }
        return true;
    }




    
    template<typename FLOAT_T>
    void
        computeOutGroup()
    {
        const GA_SplittableRange geoPointRange(geoPoint->getPointRange(geoPointGroup));
        UTparallelFor(geoPointRange, [this](const GA_SplittableRange& r)
        {
            GA_PageHandleT<FLOAT_T, FLOAT_T, true, false, const GA_Attribute, const GA_ATINumeric, const GA_Detail> attrib_ph(gfeWN.getAttrib());
            for (GA_PageIterator pit = r.beginPages(); !pit.atEnd(); ++pit)
            {
                GA_Offset start, end;
                for (GA_Iterator it(pit.begin()); it.blockAdvance(start, end); )
                {
                    attrib_ph.setPage(start);
                    for (GA_Offset elemoff = start; elemoff < end; ++elemoff)
                    {
                        FLOAT_T windingNumber = attrib_ph.value(elemoff);
                        windingNumber = abs(windingNumber);

                        if (gfeWN.asSolidAngle) {
                            windingNumber /= PI * 2.0;
                        }

                        bool outval;
                        if (groupInGeoPoint && groupOnGeoPoint)
                        {
                            outval = windingNumber > 0.5 - threshold;
                        }
                        else
                        {
                            if (groupInGeoPoint)
                            {
                                outval = windingNumber > 0.5 + threshold;
                            }
                            if (groupOnGeoPoint)
                            {
                                outval = windingNumber > (0.5 - threshold) && windingNumber < (0.5 + threshold);
                            }
                        }

                        outval ^= reverseGroup;

                        switch (groupMergeType)
                        {
                        case GFE_GroupMergeType::Replace:
                            break;
                        case GFE_GroupMerge_Union:
                            outval = outval || pointInMeshGroup->contains(elemoff);
                            break;
                        case GFE_GroupMerge_Intersect:
                            outval = outval && pointInMeshGroup->contains(elemoff);
                            break;
                        case GFE_GroupMerge_Subtract:
                            outval = !outval && pointInMeshGroup->contains(elemoff);
                            break;
                        default:
                            break;
                        }

                        pointInMeshGroup->setElement(elemoff, outval);

                        //if (outIntAttrib && intAttrib) {
                        //    intAttrib_ph.value() = outval;
                        //}
                    }
                }
            }
        }, subscribeRatio, minGrainSize);
    }


private:

    bool hasParm_outGroup = false;
    bool outGroupDetached = false;
    //UT_StringHolder groupName;
    bool groupInGeoPoint = true;
    bool groupOnGeoPoint = true;
    fpreal threshold;
    bool reverseGroup;
    GFE_GroupMergeType groupMergeType;


    bool hasParm_outNumeric = false;
    bool outNumericDetached = false;
    GA_AttributeUPtr numericAttribUPtr;
    GA_Attribute* numericAttrib = nullptr;
    GA_Storage numericStorage;
    UT_StringHolder numericAttribName;




}; // End of class GFE_PointInMeshWN








#else






class GFE_PointInMeshWN : public GFE_AttribFilterWithRef0 {

public:

    GFE_PointInMeshWN(
        GA_Detail& geo,
        const GA_Detail& geoRef0,
        GFE_WindingNumber_Cache* const sopcache = nullptr,
        const SOP_NodeVerb::CookParms* const cookparms = nullptr
    )
        : GFE_AttribFilterWithRef0(geo, &geoRef0, cookparms)
        , gfeWN(geo, geoRef0, sopcache, cookparms)
    {
    }

    ~GFE_PointInMeshWN()
    {
    }
    

    
    void
        setComputeParm(
            const bool groupInGeoPoint = true,
            const bool groupOnGeoPoint = true,
            const fpreal threshold = 1e-05,
            const exint subscribeRatio = 64,
            const exint minGrainSize = 1024
        )
    {
        setHasComputed();
        this->groupInGeoPoint = groupInGeoPoint;
        this->groupOnGeoPoint = groupOnGeoPoint;
        this->threshold       = threshold;
        this->subscribeRatio  = subscribeRatio;
        this->minGrainSize    = minGrainSize;
    }




private:


    virtual bool
        computeCore() override
    {
        if (gfeWN.getOutAttribArray().isEmpty() && getOutGroupArray().isEmpty())
            return false;
        
        if (groupParser.isEmpty())
            return true;

        if (!gfeWN.getHasComputed())
            gfeWN.compute();
        
        switch (gfeWN.getOutAttribArray()[0]->getAIFTuple()->getStorage(gfeWN.getOutAttribArray()[0]))
        {
        case GA_STORE_REAL16: computeOutGroup<fpreal16>(); break;
        case GA_STORE_REAL32: computeOutGroup<fpreal32>(); break;
        case GA_STORE_REAL64: computeOutGroup<fpreal64>(); break;
        default: break;
        }
        
        return true;
    }

    
    template<typename FLOAT_T>
    void computeOutGroup()
    {
        const GA_SplittableRange geoPointRange(groupParser.getPointRange());
        UTparallelFor(geoPointRange, [this](const GA_SplittableRange& r)
        {
            GA_PageHandleT<FLOAT_T, FLOAT_T, true, false, const GA_Attribute, const GA_ATINumeric, const GA_Detail> attrib_ph(gfeWN.getOutAttribArray()[0]);
            for (GA_PageIterator pit = r.beginPages(); !pit.atEnd(); ++pit)
            {
                GA_Offset start, end;
                for (GA_Iterator it(pit.begin()); it.blockAdvance(start, end); )
                {
                    attrib_ph.setPage(start);
                    for (GA_Offset elemoff = start; elemoff < end; ++elemoff)
                    {
                        FLOAT_T windingNumber = attrib_ph.value(elemoff);
                        windingNumber = abs(windingNumber);

                        if (gfeWN.asSolidAngle)
                            windingNumber /= PI * 2.0;

                        bool outval;
                        if (groupInGeoPoint && groupOnGeoPoint)
                        {
                            outval = windingNumber > 0.5 - threshold;
                        }
                        else
                        {
                            if (groupInGeoPoint)
                            {
                                outval = windingNumber > 0.5 + threshold;
                            }
                            if (groupOnGeoPoint)
                            {
                                outval = windingNumber > (0.5 - threshold) && windingNumber < (0.5 + threshold);
                            }
                        }
                        groupSetter.set(elemoff, outval);
                    }
                }
            }
        }, subscribeRatio, minGrainSize);
    }


    
public:
    GFE_WindingNumber gfeWN;
    

    bool groupInGeoPoint = true;
    bool groupOnGeoPoint = true;
    fpreal threshold = 1e-05;
    GFE_GroupMergeType groupMergeType = GFE_GroupMergeType::Replace;


    bool hasParm_outNumeric = false;
    bool outNumericDetached = false;
    GA_AttributeUPtr numericAttribUPtr;
    GA_Attribute* numericAttrib = nullptr;
    GA_Storage numericStorage;
    UT_StringHolder numericAttribName;


private:
    
    exint subscribeRatio = 64;
    exint minGrainSize = 1024;



}; // End of class GFE_PointInMeshWN

#endif















#endif
