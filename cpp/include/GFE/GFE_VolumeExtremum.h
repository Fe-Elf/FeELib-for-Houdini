
#pragma once

#ifndef __GFE_VolumeExtremum_h__
#define __GFE_VolumeExtremum_h__

#include "GFE/GFE_VolumeExtremum.h"

#include "GFE/GFE_GeoFilter.h"


#include "UT/UT_VoxelArray.h"
#include "GU/GU_PrimVolume.h"

class GFE_VolumeExtremum : public GFE_AttribFilterWithRef0 {

public:

    using GFE_AttribFilterWithRef0::GFE_AttribFilterWithRef0;


    ~GFE_VolumeExtremum()
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


    
private:

    virtual bool
        computeCore() override
    {
        if (!getVolumeArray().size())
            return false;

        const size_t size = getVolumeArray().size();
        for (size_t i = 0; i < size; i++)
        {
            primVol = getVolumeArray()[i];
            
            volumeExtremum();
            
        }
        return true;
    }




    
    template<typename _Ty, bool _Min, bool _Max, bool _MinElemoff = false, bool _MaxElemoff = false>
    class ComputeVolumeExtremum
    {
    public:
        ComputeVolumeExtremum(const GU_PrimVolume* const vol)
            : myVol(vol)
            , myMin(GFE_Type::numeric_limits<_Ty>::max())
            , myMax(GFE_Type::numeric_limits<_Ty>::lowest())
            , myMinElemoff(GFE_INVALID_OFFSET)
            , myMaxElemoff(GFE_INVALID_OFFSET)
        {}
        ComputeVolumeExtremum(ComputeVolumeExtremum& src, UT_Split)
            : myVol(src.myVol)
            , myMin(GFE_Type::numeric_limits<_Ty>::max())
            , myMax(GFE_Type::numeric_limits<_Ty>::lowest())
            , myMinElemoff(GFE_INVALID_OFFSET)
            , myMaxElemoff(GFE_INVALID_OFFSET)
        {}
        void operator()(const UT_BlockedRange& r)
        {
            if constexpr (!_Min && !_Max && !_MinElemoff && !_MaxElemoff)
                return;
            
            UT_Vector3i
            UT_VoxelArrayF* vol;
            UTparallelReduce(UT_BlockedRange(0, vol->numTiles()), [&](const UT_BlockedRange<exint> &r)
            {
                const exint curtile = r.begin();
                UT_VoxelTileIteratorF vit;
                vit.setLinearTile(curtile, vol);
                for (vit.rewind(); !vit.atEnd(); vit.advance())
                {
                    float delta = 0;
                    active->getValue(vit.x(), vit.y(), vit.z());
                    if (active &&  < 0.5)
                    {
                        if (sopparms.getZeroInactive())
                            vit.setValue(0);
                        continue;
                    }
                }
            }
        
            UT_ASSERT_MSG(r.getOwner() == myAttrib->getOwner(), "not same owner");
            GFE_ROPageHandleT<_Ty> attrib_ph(myAttrib);
            GA_Offset start, end;
            for (GA_PageIterator pit = r.beginPages(); !pit.atEnd(); ++pit)
            {
                for (GA_Iterator it(pit.begin()); it.blockAdvance(start, end); )
                {
                    attrib_ph.setPage(start);

                    if constexpr ((_Min || _MinElemoff) && (_Max || _MaxElemoff))
                    {
                        if (attrib_ph.value(start) < myMin)
                        {
                            myMin = attrib_ph.value(start);
                            if constexpr (_MinElemoff)
                                myMinElemoff = start;
                        }
                        if (attrib_ph.value(start) > myMax)
                        {
                            myMax = attrib_ph.value(start);
                            if constexpr (_MaxElemoff)
                                myMaxElemoff = start;
                        }
                        for (GA_Offset elemoff = start+1; elemoff < end; ++elemoff)
                        {
                            if (attrib_ph.value(start) < myMin)
                            {
                                myMin = attrib_ph.value(start);
                                if constexpr (_MinElemoff)
                                    myMinElemoff = elemoff;
                            }
                            if (attrib_ph.value(start) > myMax)
                            {
                                myMax = attrib_ph.value(start);
                                if constexpr (_MaxElemoff)
                                    myMaxElemoff = elemoff;
                            }
                        }
                    }
                    else
                    {
                        for (GA_Offset elemoff = start; elemoff < end; ++elemoff)
                        {
                            if constexpr (_Min || _MinElemoff)
                            {
                                if (attrib_ph.value(start) < myMin)
                                {
                                    myMin = attrib_ph.value(start);
                                    if constexpr (_MinElemoff)
                                        myMinElemoff = elemoff;
                                }
                            }
                            if constexpr (_Max || _MaxElemoff)
                            {
                                if (attrib_ph.value(start) > myMax)
                                {
                                    myMax = attrib_ph.value(start);
                                    if constexpr (_MaxElemoff)
                                        myMaxElemoff = elemoff;
                                }
                            }
                        }
                    }
                }
            }
        }
        SYS_FORCE_INLINE void join(const ComputeVolumeExtremum& other)
        {
            if constexpr (_Min || _MinElemoff)
            {
                if (other.myMin < myMin)
                {
                    myMin = other.myMin;
                    if constexpr (_MinElemoff)
                        myMinElemoff = other.myMinElemoff;
                }
            }
            if constexpr (_Max || _MaxElemoff)
            {
                if (other.myMax > myMax)
                {
                    myMax = other.myMax;
                    if constexpr (_MaxElemoff)
                        myMaxElemoff = other.myMaxElemoff;
                }
            }
        }
        SYS_FORCE_INLINE _Ty getMin() const
        { return myMin; }
        SYS_FORCE_INLINE _Ty getMax() const
        { return myMax; }
        //SYS_FORCE_INLINE GA_Offset getMinElemoff() const
        //{ return myMinElemoff; }
        //SYS_FORCE_INLINE GA_Offset getMaxElemoff() const
        //{ return myMaxElemoff; }
    private:
        _Ty myMin;
        _Ty myMax;
        GA_Offset myMinElemoff;
        GA_Offset myMaxElemoff;
        const GU_PrimVolume* const myVol;
    }; // End of Class ComputeVolumeExtremum


    
    template<typename _Ty, bool _Min, bool _Max, bool _MinElemoff = false, bool _MaxElemoff = false>
    static void computeAttribExtremum(
        const GU_PrimVolume* const primVol,
        _Ty& min,
        _Ty& max,
        GA_Offset& minElemoff,
        GA_Offset& maxElemoff,
        const exint subscribeRatio = 64,
        const exint minGrainSize   = 1024
    )
    {
        UT_VoxelArrayHandleF& vol = primVol->getVoxelHandle();
        ComputeVolumeExtremum<_Ty, _Min, _Max, _MinElemoff, _MaxElemoff> body(attrib);
        UTparallelReduce(UT_BlockedRange(0, vol.numTiles()), body, subscribeRatio, minGrainSize);
        min = body.getMin();
        max = body.getMax();
        minElemoff = body.getMinElemoff();
        maxElemoff = body.getMaxElemoff();
    }

    
    void volumeExtremum()
    {
        computeAttribExtremum(primVol, )

        
        UTparallelFor(groupParser.getSplittableRange(attrib), [this](const GA_SplittableRange& r)
        {
            auto tupleTypeVariant = GFE_Variant::getNumericTupleType1vVariant(*attrib);
            auto doNormalizeVariant = GFE_Variant::getBoolVariant(doNormalize);
            auto scaleVariant = GFE_Variant::getBoolVariant(uniScale != 1.0);
            std::visit([&] (auto scaleVariant, auto doNormalizeVariant, auto tupleTypeVariant)
            {
                using type = typename GFE_Variant::get_numeric_tuple_type_t<tupleTypeVariant>;
                GFE_RWPageHandleT<type> attrib_ph(attrib);
                GA_Offset start, end;
                for (GA_PageIterator pit = r.beginPages(); !pit.atEnd(); ++pit)
                {
                    for (GA_Iterator it(pit.begin()); it.blockAdvance(start, end); )
                    {
                        attrib_ph.setPage(start);
                        for (GA_Offset elemoff = start; elemoff < end; ++elemoff)
                        {
                            if constexpr (GFE_Type::isVector<type>)
                            {
                                if constexpr (doNormalizeVariant)
                                    attrib_ph.value(elemoff).normalize();
                            }
                            if constexpr (scaleVariant)
                                attrib_ph.value(elemoff) *= uniScale;
                        }
                    }
                }
            }, scaleVariant, doNormalizeVariant, tupleTypeVariant);
        }, subscribeRatio, minGrainSize);
    }
    
/*
    template<typename _Ty>
    void scaleAttribElement()
    {
        UTparallelFor(groupParser.getSplittableRange(attrib), [this](const GA_SplittableRange& r)
        {
            auto scaleVariant = GFE_Variant::getBoolVariant(uniScale != 1.0);
            auto doNormalizeVariant = GFE_Variant::getBoolVariant(doNormalize);
            std::visit([&] (auto scaleVariant, auto doNormalizeVariant)
            {
                GFE_RWPageHandleT<_Ty> attrib_ph(attrib);
                GA_Offset start, end;
                for (GA_PageIterator pit = r.beginPages(); !pit.atEnd(); ++pit)
                {
                    for (GA_Iterator it(pit.begin()); it.blockAdvance(start, end); )
                    {
                        attrib_ph.setPage(start);
                        for (GA_Offset elemoff = start; elemoff < end; ++elemoff)
                        {
                            if constexpr (GFE_Type::isVector<_Ty>)
                            {
                                if constexpr (doNormalizeVariant)
                                    attrib_ph.value(elemoff).normalize();
                            }
                            if constexpr (scaleVariant)
                                attrib_ph.value(elemoff) *= uniScale;
                        }
                    }
                }
            }, scaleVariant, doNormalizeVariant);
        }, subscribeRatio, minGrainSize);
    }
*/



public:

private:
    GU_PrimVolume* primVol;

    
    exint subscribeRatio = 64;
    exint minGrainSize   = 1024;

    
};








#endif





