
#pragma once

#ifndef __GFE_MatchDirectionOrient_h__
#define __GFE_MatchDirectionOrient_h__

#include "GFE/GFE_MatchDirectionOrient.h"

#include "GFE/GFE_GeoFilter.h"



class GFE_MatchDirOrient : public GFE_AttribCreateFilterWithRef0 {

public:
    using GFE_AttribCreateFilterWithRef0::GFE_AttribCreateFilterWithRef0;

    ~GFE_MatchDirOrient()
    {
    }

    

    void
    setComputeParm(
        const UT_Vector3T<fpreal64>& constDir = UT_Vector3T<fpreal64>(0,1,0),
        const bool outDir      = true,
        const bool accurate    = false,
        const bool keepBearing = false,
        const bool reverseDir  = false,
        const exint subscribeRatio = 64,
        const exint minGrainSize = 1024
    )
    {
        setHasComputed();
        this->constDir    = constDir;
        this->outDir      = outDir;
        this->accurate    = accurate;
        this->keepBearing = keepBearing;
        this->reverseDir  = reverseDir;
        this->subscribeRatio   = subscribeRatio;
        this->minGrainSize     = minGrainSize;
    }


    
    

private:

virtual bool
    computeCore() override
{
    if (getOutAttribArray().isEmpty())
        return false;

    if (groupParser.isEmpty())
        return true;
    
    const size_t numAttrib = getOutAttribArray().size();
    for (size_t i = 0; i < numAttrib; ++i)
    {
        dirAttrib = getOutAttribArray()[i];
        
        if (getOutGroupArray().isEmpty())
            groupSetter.clear();
        else
            groupSetter = getOutGroupArray()[i % getOutGroupArray().size()];
        
        dirRefAttrib = getRef0AttribArray().isEmpty() ? nullptr : getRef0AttribArray()[i % getRef0AttribArray().size()];
        
        const GA_AIFTuple* const aifTuple = dirAttrib->getAIFTuple();
        if (!aifTuple)
            continue;
        if (dirAttrib->getTupleSize() != 3)
            continue;
        switch (aifTuple->getStorage(dirAttrib))
        {
        case GA_STORE_REAL16: matchDirOrient<fpreal16>(); break;
        case GA_STORE_REAL32: matchDirOrient<fpreal32>(); break;
        case GA_STORE_REAL64: matchDirOrient<fpreal64>(); break;
        default: continue;
        }
    }
    
    return true;
}




template<typename FLOAT_T>
void matchDirOrient()
{
    const GA_AttributeOwner attribOwner = dirAttrib->getOwner();
    useRefAttrib = dirRefAttrib && dirRefAttrib->getOwner() == attribOwner && geoRef0->getNumElements(attribOwner) >= geo->getNumElements(attribOwner);
    const GA_ROHandleT<UT_Vector3T<FLOAT_T>> dirRef_h(dirRefAttrib);
    UTparallelFor(groupParser.getSplittableRange(attribOwner), [this, &dirRef_h](const GA_SplittableRange& r)
    {
        GA_PageHandleT<UT_Vector3T<FLOAT_T>, FLOAT_T, true, true, GA_Attribute, GA_ATINumeric, GA_Detail> dir_ph(dirAttrib);
        for (GA_PageIterator pit = r.beginPages(); !pit.atEnd(); ++pit)
        {
            //UT_Vector3T<FLOAT_T> dir;
            UT_Vector3T<FLOAT_T> dirRef;
            GA_Offset start, end;
            for (GA_Iterator it(r); it.blockAdvance(start, end); )
            {
                dir_ph.setPage(start);
                for (GA_Offset elemoff = start; elemoff < end; ++elemoff)
                {
                    if (useRefAttrib)
                        dirRef = dirRef_h.get(elemoff);
                    else
                        dirRef = constDir;

                    bool condition;
                    if ( ( accurate && keepBearing ) || groupSetter.isElementValid() || !accurate )
                        condition = dot(dir_ph.value(elemoff), dirRef) < 0;

                    if (outDir)
                    {
                        if (accurate)
                        {
                            dirRef.normalize();
                            dir_ph.value(elemoff) = dirRef * dir_ph.value(elemoff).length();
                            if ((keepBearing && condition) ^ reverseDir)
                                dir_ph.value(elemoff) *= -1;
                        }
                        else
                        {
                            if (condition ^ reverseDir)
                                dir_ph.value(elemoff) *= -1;
                        }
                    }
                    
                    if (groupSetter.isElementValid())
                        groupSetter.set(elemoff, condition);

                }
            }
        }
    }, subscribeRatio, minGrainSize);
    
}

public:
    bool outDir      = true;
    bool accurate    = false;
    bool keepBearing = false;
    bool reverseDir  = false;
    UT_Vector3T<fpreal64> constDir = UT_Vector3T<fpreal64>(0,1,0);

private:
    GA_Attribute* dirAttrib;
    const GA_Attribute* dirRefAttrib;
    bool useRefAttrib;
    
    exint subscribeRatio = 64;
    exint minGrainSize = 1024;


}; // End of class GFE_MatchDirOrient

#endif
