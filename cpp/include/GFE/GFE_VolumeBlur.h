
#pragma once

#ifndef __GFE_VolumeBlur_h__
#define __GFE_VolumeBlur_h__

#include "GFE/GFE_VolumeBlur.h"

#include "GFE/GFE_GeoFilter.h"

// https://blog.csdn.net/LinerPhong/article/details/128356166


#define GFE_VolumeBlur_Kernel 1


class GFE_VolumeBlur : public GFE_GeoFilter {

public:
    using GFE_GeoFilter::GFE_GeoFilter;
    
    ~GFE_VolumeBlur()
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

#if GFE_VolumeBlur_Kernel
    SYS_FORCE_INLINE void setKernel(const int8 kernel)
    { this->kernel = kernel; }
#endif
    
private:

    #define CUDA_API extern "C" __declspec(dllimport)
    CUDA_API void boxBlur(const float* idata, float* odata, int radius, int nx, int ny);
    
    __global__ void boxBlurXdir(const float* __restrict__ idata, float* odata, int radius, int nx, int ny)
    {
        int ix = threadIdx.x + blockIdx.x * blockDim.x;
        int iy = threadIdx.y + blockIdx.y * blockDim.y;
        if (ix >= nx || iy >= ny)
            return;
        int idx = ix + iy * nx;
 
        int xStart = ix - radius;
        int xEnd = ix + radius;
 
        if (xStart < 0) xStart = 0; if (xEnd > nx - 1) xEnd = nx - 1;
 
        int num = 0;
        float sum = 0;
 
        for (int i = xStart; i <= xEnd; i++)
        {
            sum += idata[i + nx * iy];
            num++;
        }
 
        odata[idx] = sum / num;
    }
 
    __global__ void boxBlurYdir(const float* __restrict__ idata, float* odata, int radius, int nx, int ny)
    {
        int ix = threadIdx.x + blockIdx.x * blockDim.x;
        int iy = threadIdx.y + blockIdx.y * blockDim.y;
        if (ix >= nx || iy >= ny)
            return;
        int idx = ix + iy * nx;
 
        int yStart = iy - radius;
        int yEnd = iy + radius;
 
        if (yStart < 0) yStart = 0; if (yEnd > ny - 1) yEnd = ny - 1;
 
        int num = 0;
        float sum = 0;
 
        for (int i = yStart; i <= yEnd; i++)
        {
            sum += idata[ix + nx * i];
            num++;
        }
 
        odata[idx] = sum / num;
    }

    #define CUDA_API extern "C" __declspec(dllexport)
 
    CUDA_API void boxBlur(const float* idata, float* odata, int radius, int nx, int ny)
    {
        dim3 block(BDIMX, BDIMY);
        dim3 grid((nx + BDIMX - 1) / BDIMX, (ny + BDIMY - 1) / BDIMY);
        int nBytes = nx * ny * sizeof(float);
 
        float* dIdata, * dTemp, * dOdata;
        cudaMalloc((float**)&dIdata, nBytes);
        cudaMalloc((float**)&dTemp, nBytes);
        cudaMalloc((float**)&dOdata, nBytes);
 
        cudaMemcpy(dIdata, idata, nBytes, cudaMemcpyHostToDevice);
 
        boxBlurXdir << <grid, block >> > (dIdata, dTemp, radius, nx, ny);
        boxBlurYdir << <grid, block >> > (dTemp, dOdata, radius, nx, ny);
 
        cudaMemcpy(odata, dOdata, nBytes, cudaMemcpyDeviceToHost);
 
        cudaFree(dIdata);
        cudaFree(dTemp);
        cudaFree(dOdata);
    }
    

    
    virtual bool
        computeCore() override
    {
        const size_t size = getVolumeArray().size();
        UT_ASSERT_P(voxValues.size() == size);
        
        for (size_t i = 0; i < size; i++)
        {
            vol = getVolumeArray()[i];
            const fpreal32 value = voxValues[i];

            
            switch(gpuComputeMethod)
            {
            case GFE_GPUComputeMethod::CPU:    ; break;
            case GFE_GPUComputeMethod::Cuda:   ; break;
            case GFE_GPUComputeMethod::OpenCL: ; break;
            }
            
            switch (kernel)
            {
            case 0: volumeBlur(value); break;
            case 1: break;
            case 2: break;
            }
#else
            volumeBlur(value);
#endif
        }
        return true;
    }


    void volumeBlur(const fpreal32 value)
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
    GFE_GPUComputeMethod gpuComputeMethod = GFE_GPUComputeMethod::Cuda;
private:
    GU_PrimVolume* vol;
    std::vector<fpreal> voxValues;
    
    exint subscribeRatio = 64;
    exint minGrainSize   = 1024;

#if GFE_VolumeBlur_Kernel
    int8 kernel = 0;
#endif
    
};








#endif





