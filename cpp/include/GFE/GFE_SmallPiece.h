
#pragma once

#ifndef __GFE_SmallPiece_h__
#define __GFE_SmallPiece_h__

#include "GFE/GFE_SmallPiece.h"

#include "GFE/GFE_GeoFilter.h"




#include "GFE/GFE_Measure.h"


class GFE_SmallPiece : public GFE_AttribFilter {

public:
    using GFE_AttribFilter::GFE_AttribFilter;
    
    void
        setComputeParm(
            const int8 numConditionMatch = 1,
            const exint subscribeRatio = 64,
            const exint minGrainSize   = 64
        )
    {
        setHasComputed();
        this->numConditionMatch = numConditionMatch;
        this->subscribeRatio = subscribeRatio;
        this->minGrainSize   = minGrainSize;
    }

private:

    virtual bool
        computeCore() override
    {
        if (!(checkAreaPerimeterRatio ||
              checkPerimeter          ||
              checkArea               ||
              checkVolume             ||
              checkOneOfXYZSize       ||
              checkAllXYZSize         ||
              checkXYZSizeSum          ))
            return false;
        
        if (getOutGroupArray().isEmpty() || !getOutGroupArray().isPrimitiveGroup(0) )
            return false;

        if (groupParser.isEmpty())
            return true;
        
        groupSetter = getOutGroupArray()[0];
        
        switch (geo->getPreferredStorageF())
        {
        //case GA_STORE_REAL16: smallPiece<fpreal16>();        break;
        case GA_STORE_REAL32: smallPiece<fpreal32>();        break;
        case GA_STORE_REAL64: smallPiece<fpreal64>();        break;
        default: UT_ASSERT_MSG(0, "unhandled storage");      break;
        }

        return true;
    }

    
#define GFE_SmallPiece_SPECILIZATION_MULTI_CONDITION_FUNC(FuncName)       \
        FuncName(AreaPerimeterRatio)                                      \
        FuncName(Perimeter)                                               \
        FuncName(Area)                                                    \
        FuncName(Volume)                                                  \
        FuncName(OneOfXYZSize)                                            \
        FuncName(AllXYZSize)                                              \
        FuncName(XYZSizeSum)                                              \


    
#define GFE_SmallPiece_SPECILIZATION_CheckAttribType(Name) \
        UT_ASSERT_MSG(!thresholdAttrib##Name || thresholdAttrib##Name->getOwner() == GA_ATTRIB_PRIMITIVE, "must be prim attrib");
        
#define GFE_SmallPiece_SPECILIZATION_PageHandleT(Name) \
        GA_PageHandleT<FLOAT_T, FLOAT_T, true, false, const GA_Attribute, const GA_ATINumeric, const GA_Detail> thresholdAttrib##Name##_ph(thresholdAttrib##Name);

#define GFE_SmallPiece_SPECILIZATION_SetPage(Name) \
        thresholdAttrib##Name##_ph.setPage(start);
        
    
    template<typename FLOAT_T>
    void smallPiece()
    {
        GFE_SmallPiece_SPECILIZATION_MULTI_CONDITION_FUNC(GFE_SmallPiece_SPECILIZATION_CheckAttribType)
        
        GFE_Measure measure(geo, cookparms);
        measure.groupParser.setGroup(groupParser);
        
        if (checkAreaPerimeterRatio || checkArea)
        {
            measure.measureType = GFE_MeasureType::Area;
            areaAttrib = measure.findOrCreateTuple(true);
            measure.compute();
        }
        else
            areaAttrib = nullptr;
        
        if (checkAreaPerimeterRatio || checkPerimeter)
        {
            measure.measureType = GFE_MeasureType::Perimeter;
            perimeterAttrib = measure.findOrCreateTuple(true);
            measure.compute();
        }
        else
            perimeterAttrib = nullptr;
        
        if (checkVolume)
        {
            measure.measureType = GFE_MeasureType::Volume;
            volumeAttrib = measure.findOrCreateTuple(true);
            measure.compute();
        }
        else
            volumeAttrib = nullptr;

        const bool computeBBox = checkOneOfXYZSize || checkAllXYZSize || checkXYZSizeSum;
        
        UTparallelFor(groupParser.getPrimitiveSplittableRange(), [this, computeBBox](const GA_SplittableRange& r)
        {
            int8 numConditionMatchCur = 0;
            
            UT_BoundingBoxT<fpreal32> bbox(GFE_Bound::stdBoundingBoxT<fpreal32>());
            UT_Vector3T<fpreal32> bboxSize;
            
            GA_PageHandleT<FLOAT_T, FLOAT_T, true, false, const GA_Attribute, const GA_ATINumeric, const GA_Detail> area_ph(areaAttrib);
            GA_PageHandleT<FLOAT_T, FLOAT_T, true, false, const GA_Attribute, const GA_ATINumeric, const GA_Detail> perimeter_ph(perimeterAttrib);
            GA_PageHandleT<FLOAT_T, FLOAT_T, true, false, const GA_Attribute, const GA_ATINumeric, const GA_Detail> volume_ph(volumeAttrib);
            
            GFE_SmallPiece_SPECILIZATION_MULTI_CONDITION_FUNC(GFE_SmallPiece_SPECILIZATION_PageHandleT)
            //GA_PageHandleT<FLOAT_T, FLOAT_T, true, false, const GA_Attribute, const GA_ATINumeric, const GA_Detail> thresholdAttribAreaPerimeterRatio_ph(thresholdAttribAreaPerimeterRatio);
            //GA_PageHandleT<FLOAT_T, FLOAT_T, true, false, const GA_Attribute, const GA_ATINumeric, const GA_Detail> thresholdAttribPerimeter_ph(thresholdAttribPerimeter);
            //GA_PageHandleT<FLOAT_T, FLOAT_T, true, false, const GA_Attribute, const GA_ATINumeric, const GA_Detail> thresholdAttribArea_ph(thresholdAttribArea);
            //GA_PageHandleT<FLOAT_T, FLOAT_T, true, false, const GA_Attribute, const GA_ATINumeric, const GA_Detail> thresholdAttribArea_ph(thresholdAttribArea);
            for (GA_PageIterator pit = r.beginPages(); !pit.atEnd(); ++pit)
            {
                GA_Offset start, end;
                for (GA_Iterator it(pit.begin()); it.blockAdvance(start, end); )
                {
                    GFE_SmallPiece_SPECILIZATION_MULTI_CONDITION_FUNC(GFE_SmallPiece_SPECILIZATION_SetPage)
                    
                    area_ph.setPage(start);
                    perimeter_ph.setPage(start);
                    volume_ph.setPage(start);
                    for (GA_Offset elemoff = start; elemoff < end; ++elemoff)
                    {
                        if (checkAreaPerimeterRatio)
                            numConditionMatchCur += area_ph.value(elemoff) / perimeter_ph.value(elemoff) <
                                                    thresholdAreaPerimeterRatio + (thresholdAttribAreaPerimeterRatio ? thresholdAttribAreaPerimeterRatio_ph.value(elemoff) : 0);

                        if (checkArea)
                            numConditionMatchCur += area_ph.value(elemoff) <
                                                    thresholdArea + (thresholdAttribArea ? thresholdAttribArea_ph.value(elemoff) : 0);
                        if (checkPerimeter)
                            numConditionMatchCur += perimeter_ph.value(elemoff) <
                                thresholdPerimeter + (thresholdAttribPerimeter ? thresholdAttribPerimeter_ph.value(elemoff) : 0);
                        if (checkVolume)
                            numConditionMatchCur += volume_ph.value(elemoff) <
                                                    thresholdVolume + (thresholdAttribVolume ? thresholdAttribVolume_ph.value(elemoff) : 0);
                        
                        if (computeBBox)
                        {
                            GFE_Bound::setStd(bbox);
                            geo->getPrimitive(elemoff)->enlargePointBounds(bbox);
                            bboxSize = bbox.size();
                        }
                        if (checkOneOfXYZSize)
                            numConditionMatchCur += bboxSize.minComponent() <
                                                    thresholdOneOfXYZSize + (thresholdAttribOneOfXYZSize ? thresholdAttribOneOfXYZSize_ph.value(elemoff) : 0);
                        if (checkAllXYZSize)
                            numConditionMatchCur += bboxSize.maxComponent() <
                                                    thresholdAllXYZSize + (thresholdAttribAllXYZSize ? thresholdAttribAllXYZSize_ph.value(elemoff) : 0);
                        if (checkXYZSizeSum)
                            numConditionMatchCur += bboxSize.x() + bboxSize.y() + bboxSize.z() <
                                                    thresholdXYZSizeSum + (thresholdAttribXYZSizeSum ? thresholdAttribXYZSizeSum_ph.value(elemoff) : 0);
                        
                        groupSetter.set(elemoff, numConditionMatchCur >= numConditionMatch);
                        //area_ph.value(elemoff)
                    }
                }
            }
        }, subscribeRatio, minGrainSize);
    }

#undef GFE_SmallPiece_SPECILIZATION_CheckAttribType
#undef GFE_SmallPiece_SPECILIZATION_PageHandleT
#undef GFE_SmallPiece_SPECILIZATION_SetPage







    

    
#define GFE_SmallPiece_SPECILIZATION(Name)                                                                     \
private:                                                                                                       \
        bool check##Name = false;                                                                              \
        fpreal threshold##Name = 1e-05;                                                                        \
        const GA_Attribute* thresholdAttrib##Name = nullptr;                                                   \
public:                                                                                                        \
        SYS_FORCE_INLINE void setCheck##Name()                                                                 \
        { check##Name = false; thresholdAttrib##Name = nullptr; }                                              \
                                                                                                               \
        void setCheck##Name(const fpreal threshold, const GA_Attribute* const thresholdAttrib = nullptr)       \
        {                                                                                                      \
            check##Name = true;                                                                                \
            threshold##Name = threshold;                                                                       \
            thresholdAttrib##Name = thresholdAttrib;                                                           \
        }                                                                                                      \
                                                                                                               \
        void setCheck##Name(const fpreal threshold, const UT_StringRef& thresholdAttribName)                   \
        {                                                                                                      \
            check##Name = true;                                                                                \
            threshold##Name = threshold;                                                                       \
            thresholdAttrib##Name = geo->findPrimitiveAttribute(thresholdAttribName);                          \
        }                                                                                                      \
        
GFE_SmallPiece_SPECILIZATION_MULTI_CONDITION_FUNC(GFE_SmallPiece_SPECILIZATION)
    
#undef GFE_SmallPiece_SPECILIZATION



    
    
private:

public:
    int8 numConditionMatch = 1;
    
private:
    //const GA_PrimitiveGroup* outPrimGroup;
    const GA_Attribute* areaAttrib;
    const GA_Attribute* perimeterAttrib;
    const GA_Attribute* volumeAttrib;
    
    exint subscribeRatio = 64;
    exint minGrainSize = 1024;


}; // End of class GFE_SmallPiece


#endif
