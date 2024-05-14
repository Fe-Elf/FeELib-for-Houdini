
#pragma once

#ifndef __GFE_VolumeClear_h__
#define __GFE_VolumeClear_h__

#include "GFE/GFE_VolumeClear.h"

#include "GFE/GFE_GeoFilter.h"


#define GFE_VolumeClear_Kernel 1

    
class GFE_VolumeClear : public GFE_GeoFilter {
    
public:
    using GFE_GeoFilter::GFE_GeoFilter;
    
    ~GFE_VolumeClear()
    {
    }
    
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
    
    SYS_FORCE_INLINE void bumpDataId() const override
    { geo->getPrimitiveList().bumpDataId(); }
    //{ geo->bumpAllDataIds(); }

    
    SYS_FORCE_INLINE std::vector<fpreal>& getVoxelValues()
    { return voxValues; }

    SYS_FORCE_INLINE const std::vector<fpreal>& getVoxelValues() const
    { return voxValues; }

#if GFE_VolumeClear_Kernel
    SYS_FORCE_INLINE void setKernel(const int8 kernel)
    { this->kernel = kernel; }
#endif
    
private:

    virtual bool
        computeCore() override
    {
        const size_t size = getVolumeArray().size();
        UT_ASSERT_P(voxValues.size() == size);
        
        for (size_t i = 0; i < size; i++)
        {
            vol = getVolumeArray()[i];
            const fpreal32 value = voxValues[i];
#if GFE_VolumeClear_Kernel
            switch (kernel)
            {
            case 0: volumeClear(value); break;
            case 1: break;
            case 2: break;
            }
#else
            volumeClear(value);
#endif
        }
        return true;
    }


    void volumeClear(const fpreal32 value)
    {
        UT_ASSERT_P(vol);
    
		UT_VoxelArrayWriteHandleF vx_h = vol->getVoxelWriteHandle();
        UT_VoxelArray<fpreal32>* const vx = &*vx_h;
        UTparallelForEachNumber(vx->numTiles(), [vx, value](const UT_BlockedRange<int> &r)
        {
            //const exint curtile = r.begin();
            UT_VoxelTileIteratorF vit;
            vit.setLinearTile(r.begin(), vx);
            for (vit.rewind(); !vit.atEnd(); vit.advance())
            {
                vit.setValue(value);
            }
        });
        //}, subscribeRatio, minGrainSize);
    }
    


public:
    
private:
    GU_PrimVolume* vol;
    std::vector<fpreal> voxValues;
    
    exint subscribeRatio = 64;
    exint minGrainSize   = 1024;

#if GFE_VolumeClear_Kernel
    int8 kernel = 0;
#endif
    
};








#endif





