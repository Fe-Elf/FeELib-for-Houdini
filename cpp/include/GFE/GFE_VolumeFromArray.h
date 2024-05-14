
#pragma once

#ifndef __GFE_Enumerate_h__
#define __GFE_Enumerate_h__

#include "GFE/GFE_VolumeFromArray.h"

#include "GFE/GFE_GeoFilter.h"


#include "GU/GU_PrimVolume.h"


#include "GFE/GFE_AttributeCast.h"
#include "SOP/SOP_Enumerate.proto.h"

/*
    GFE_VolumeFromArray volumeFromArray(geo, cookparms);
    volumeFromArray.findOrCreateTuple(true, GA_ATTRIB_POINT);
    volumeFromArray.compute();
*/
    
class GFE_VolumeFromArray : public GFE_AttribCreateFilterWithRef0 {

public:
    using GFE_AttribCreateFilterWithRef0::GFE_AttribCreateFilterWithRef0;


    void
        setComputeParm(
            const exint subscribeRatio = 64,
            const exint minGrainSize   = 1024
        )
    {
        setHasComputed();
        this->subscribeRatio = subscribeRatio;
        this->minGrainSize   = minGrainSize;
    }

    
    //SYS_FORCE_INLINE GFE_AttributeArray& getInAttribArray()
    //{ return inAttribArray; }
    
private:

    virtual bool
        computeCore() override
    {
        if (geoRef0 ? getRef0AttribArray().isEmpty() : getInAttribArray().isEmpty())
            return false;
        
        
        const size_t size = geoRef0 ? getRef0AttribArray().size() : getInAttribArray().size();
        for (size_t i = 0; i < size; i++)
        {
            attrib = geoRef0 ? getRef0AttribArray()[i] : getInAttribArray()[i];
            const GA_AIFNumericArray* const aifNumericArray = attrib->getAIFTuple();
            if (!aifNumericArray)
                continue;


#if 1
            const GA_Offset primoff = geo->asGA_Detail()->appendPrimitiveBlock(GA_PRIMVOLUME, 1);
            primVolume = geo->getVolumePrimitive(primoff);
#else
            primVolume = reinterpret_cast<GU_PrimVolume*>(geo->asGA_Detail()->appendPrimitive(GA_PRIMVOLUME));
#endif
            switch (aifNumericArray->getStorage(attrib))
            {
            case GA_STORE_INT16:  volumeFromArray<int16>();    break;
            case GA_STORE_INT32:  volumeFromArray<int32>();    break;
            case GA_STORE_INT64:  volumeFromArray<int64>();    break;
            case GA_STORE_REAL16: volumeFromArray<fpreal16>(); break;
            case GA_STORE_REAL32: volumeFromArray<fpreal32>(); break;
            case GA_STORE_REAL64: volumeFromArray<fpreal64>(); break;
            default: break;
            }
        }
        return true;
    }

    template<typename _Ty>
    void volumeFromArray()
    {
        //primVolume->setVoxels();
    }


public:
    
private:
    const GA_Attribute* attrib;
    GU_PrimVolume* primVolume;
    exint subscribeRatio = 64;
    exint minGrainSize   = 1024;

private:
    //GFE_AttributeArray inAttribArray;
    
    
}; // End of class GFE_VolumeFromArray





#endif
