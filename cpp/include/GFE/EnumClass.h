
#pragma once

#ifndef __GFE_EnumClass_h__
#define __GFE_EnumClass_h__

#include <GFE/EnumClass.h>

#include <GFE/Core.h>

_GFE_BEGIN
inline namespace EnumClass {


    
#define __GFE_Core_SpecializationVec(NUM)\
        Vector##NUM##I,\
        Vector##NUM##i = Vector##NUM##I,\
        Vector##NUM##L,\
        Vector##NUM##H,\
        Vector##NUM##F,\
        Vector##NUM##D,\

#define __GFE_Core_SpecializationMtx(NUM)\
        Matrix##NUM##F,\
        Matrix##NUM##D,\

#define __GFE_Core_SpecializationVecArray(NUM)\
        Vector##NUM##IArray,\
        Vector##NUM##iArray = Vector##NUM##IArray,\
        Vector##NUM##LArray,\
        Vector##NUM##HArray,\
        Vector##NUM##FArray,\
        Vector##NUM##DArray,\

#define __GFE_Core_SpecializationMtxArray(NUM)\
        Matrix##NUM##FArray,\
        Matrix##NUM##DArray,\
    
enum class AttribStorage
{
    Int8,
    Int16,
    Int32,
    Int64,
    Float16,
    Float32,
    Float64,
    
    __GFE_Core_SpecializationVec(2)
    __GFE_Core_SpecializationVec(3)
    __GFE_Core_SpecializationVec(4)
    
    __GFE_Core_SpecializationMtx(2)
    __GFE_Core_SpecializationMtx(3)
    __GFE_Core_SpecializationMtx(4)
    
    String,
    Dict,
    Int32Array,
    Int64Array,
    Float32Array,
    Float64Array,

    __GFE_Core_SpecializationVecArray(2)
    __GFE_Core_SpecializationVecArray(3)
    __GFE_Core_SpecializationVecArray(4)
    
    __GFE_Core_SpecializationMtxArray(2)
    __GFE_Core_SpecializationMtxArray(3)
    __GFE_Core_SpecializationMtxArray(4)
    
    StringArray,
    DictArray,
    
    Invalid = -1,
    Fpreal16 = Float16,
    Fpreal32 = Float32,
    Fpreal64 = Float64,
    UT_StringHolder = String,
    Fpreal32Array = Float32Array,
    Fpreal64Array = Float64Array,
    UT_StringHolderArray = StringArray,
    DictHolder = Dict,
    UT_OptionsHolderArray = DictArray,
    
    
}; // End of Class Enum Class AttribStorage


#undef __GFE_Core_SpecializationVec
#undef __GFE_Core_SpecializationMtx
#undef __GFE_Core_SpecializationVecArray
#undef __GFE_Core_SpecializationMtxArray



enum class AttribMergeMethod
{
    First,
    Last,
    Min,
    Max,
    Mode,
    Mean,
    Median,
    Sum,
    SumSquare,
    RootMeanSquare,
};

enum class GroupMergeMethod
{
    Replace,
    First,
    Last,
    Min,
    Max,
    Mode,
};


enum class GroupMergeType
{
    Replace,
    Union,
    Intersect,
    Subtract,
};

enum class AttribMergeType
{
    Set,
    Add,
    Sub,
    Mult,
    Min,
    Max,
    Xor,
    Toggle,
    Append,
    Intersect
};


enum class StatisticalFunction
{
    Min,
    Max,
    Mode,
    Mean,
    Median,
    Sum,
    SumSquare,
    RootMeanSquare,
};


enum class ElemTraversingMethod
{
    Custom,
    OneElem,
    SkipNElem,
};

enum class Axis
{
    X,
    Y,
    Z,
    Invalid = -1,
};

enum class CurveEndsType
{
    Ends,
    Start,
    End,
};

#if 0

switch (curveEndsType)
{
case gfe::CurveEndsType::Ends:  break;
case gfe::CurveEndsType::Start: break;
case gfe::CurveEndsType::End:   break;
default: UT_ASSERT_MSG(0, "Unhandled GFE CurveEndsType"); break;
}

#endif


enum class OutArrayType
{
    Attrib,
    Packed,
    Geo,
};



enum class ScaleAxis
{
    X,
    Y,
    Z,
    XYZMin,
    XYZMiddle,
    XYZMax,
    XYMin,
    YZMin,
    ZXMin,
    XYMax,
    YZMax,
    ZXMax,
    Invalid = -1,
};

#if 0
    gfe::GPUComputeMethod gpuComputeMethod = gfe::GPUComputeMethod::Cuda;
    switch(gpuComputeMethod)
    {
    case GFE_GPUComputeMethod::CPU:    ; break;
    case GFE_GPUComputeMethod::Cuda:   ; break;
    case GFE_GPUComputeMethod::OpenCL: ; break;
    }

#endif

enum class GPUComputeMethod
{
    CPU,
    CUDA,
    OpenCL,
    Invalid = -1,
};




} // End of namespace EnumClass
_GFE_END
#endif
