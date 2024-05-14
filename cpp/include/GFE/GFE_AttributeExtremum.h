
#pragma once

#ifndef __GFE_AttributeExtremum_h__
#define __GFE_AttributeExtremum_h__

#include "GFE/GFE_AttributeExtremum.h"

#include <GFE/GFE_GeoFilter.h>

#define GFE_AttributeExtremum_UseVariant 0

#if 0
    GFE_AttribExtremum attribExtremum(geo, inGeo1, cookparms);
    
    attribExtremum.outas(sopparms.getOutAsOffset(),
        sopparms.getSubscribeRatio(), sopparms.getMinGrainSize());
        
    attribExtremum.setAttrib(attribClass, sopparms.getAttrib());
        
    attribExtremum.groupParser.setGroup(groupType, sopparms.getGroup());
        
    attribExtremum.computeAndBumpDataId();

#endif


class GFE_AttribExtremum : public GFE_AttribFilterWithRef0 {

public:

    using GFE_AttribFilterWithRef0::GFE_AttribFilterWithRef0;

    
    ~GFE_AttribExtremum()
    {
    }


void
    setComputeParm(
        const bool outAsOffset = true,
        const exint subscribeRatio = 64,
        const exint minGrainSize   = 1024
    )
{
    setHasComputed();
    this->outAsOffset = outAsOffset;
    this->subscribeRatio = subscribeRatio;
    this->minGrainSize   = minGrainSize;
}

    
    const GA_Attribute* setAttrib(const GA_AttributeOwner owner, const UT_StringRef& name)
    { return attrib = geo->findAttribute(owner, name); }
    
    void setAttrib(const GA_Attribute* const inAttrib)
    { attrib = inAttrib; }
    

    
    
#if GFE_AttributeExtremum_UseVariant
    SYS_FORCE_INLINE const auto& getMin() const
    { return min; }
    SYS_FORCE_INLINE const auto& getMax() const
    { return max; }
    SYS_FORCE_INLINE fpreal getMinF() const
    { return std::get_if<fpreal>(&min) ? std::get<fpreal>(min) : std::numeric_limits<fpreal>::max(); }
    SYS_FORCE_INLINE fpreal getMaxF() const
    { return std::get_if<fpreal>(&max) ? std::get<fpreal>(max) : std::numeric_limits<fpreal>::lowest(); }
    SYS_FORCE_INLINE exint getMinI() const
    { return std::get_if<exint>(&min) ? std::get<exint>(min) : std::numeric_limits<exint>::max(); }
    SYS_FORCE_INLINE exint getMaxI() const
    { return std::get_if<exint>(&max) ? std::get<exint>(max) : std::numeric_limits<exint>::lowest(); }
#else
    SYS_FORCE_INLINE auto getMin() const
    { return std::variant<fpreal, exint>(isInt ? min.i : min.f); }
    SYS_FORCE_INLINE auto getMax() const
    { return std::variant<fpreal, exint>(isInt ? max.i : max.f); }
    SYS_FORCE_INLINE fpreal getMinF() const
    { return min.f; }
    SYS_FORCE_INLINE fpreal getMaxF() const
    { return max.f; }
    SYS_FORCE_INLINE exint getMinI() const
    { return min.i; }
    SYS_FORCE_INLINE exint getMaxI() const
    { return max.i; }
#endif
    SYS_FORCE_INLINE GA_Offset getMinElemoff() const
    { return attribMinElemoff; }
    SYS_FORCE_INLINE GA_Offset getMaxElemoff() const
    { return attribMaxElemoff; }
    
private:

    virtual bool
        computeCore() override
    {
        if (!attrib)
            return false;
        
        if (!outAttribMin && !outAttribMax && !outAttribMinElemnum && !outAttribMaxElemnum)
            return false;
            
        if (groupParser.isEmpty())
            return true;

        attribExtremum();

        
        getOutAttribArray().clear();
        
        if (outAttribMin && attribMin)
            getOutAttribArray().append(attribMin);
        if (outAttribMax)
            getOutAttribArray().append(attribMax);
        if (outAttribMinElemnum)
            getOutAttribArray().append(attribMinElemnum);
        if (outAttribMaxElemnum)
            getOutAttribArray().append(attribMaxElemnum);

        
        return true;
    }


    void attribExtremum()
    {
        const GFE_AttribStorage attribStorage = GFE_Type::getAttribStorage(attrib);
        if (!GFE_Variant::isAttribStorageIFV(attribStorage))
            return;
        auto storageVariant = GFE_Variant::getAttribStorageVariantIFV(attribStorage);
        auto outAttribMinVariant = GFE_Variant::getBoolVariant(outAttribMin);
        auto outAttribMaxVariant = GFE_Variant::getBoolVariant(outAttribMax);
        auto outAttribMinElemnumVariant = GFE_Variant::getBoolVariant(outAttribMinElemnum);
        auto outAttribMaxElemnumVariant = GFE_Variant::getBoolVariant(outAttribMaxElemnum);
        std::visit([&] (auto storageVariant,
                        auto outAttribMinVariant,
                        auto outAttribMaxVariant,
                        auto outAttribMinElemnumVariant,
                        auto outAttribMaxElemnumVariant)
        {
            using type = typename GFE_Variant::getAttribStorage_t<storageVariant>;
            using value_type = typename GFE_Type::get_value_type_t<type>;
                
            value_type minVal;
            value_type maxVal;
            GFE_Attribute::computeAttribExtremum<type,
                                                 outAttribMinVariant,
                                                 outAttribMaxVariant,
                                                 outAttribMinElemnumVariant,
                                                 outAttribMaxElemnumVariant>
                (attrib, groupParser.getSplittableRange(attrib), minVal, maxVal, attribMinElemoff, attribMaxElemoff, subscribeRatio, minGrainSize);

#if GFE_AttributeExtremum_UseVariant
            if constexpr (std::is_floating_point_v<type>)
            {
                min = fpreal(minVal);
                max = fpreal(maxVal);
            }
            else
            {
                min = exint(minVal);
                max = exint(maxVal);
            }
#else
            if constexpr (std::is_floating_point_v<type>)
            {
                isInt = false;
                min.f = minVal;
                max.f = maxVal;
            }
            else
            {
                isInt = true;
                min.i = minVal;
                max.i = maxVal;
            }
#endif
            
            if constexpr (outAttribMinVariant)
                setAttribValue(attribMin, minVal);
            if constexpr (outAttribMaxVariant)
                setAttribValue(attribMax, maxVal);
        }, storageVariant, outAttribMinVariant, outAttribMaxVariant, outAttribMinElemnumVariant, outAttribMaxElemnumVariant);

        
        if (outAsOffset)
        {
            //if (GFE_Type::isValidOffset(attribMinElemoff))
            if (outAttribMinElemnum)
            {
                UT_ASSERT_P(GFE_Type::isValidOffset(attrib->getIndexMap(), attribMinElemoff));
                attribMinElemoff = attrib->getIndexMap().indexFromOffset(attribMinElemoff);
            }
            //if (GFE_Type::isValidOffset(attribMaxElemoff))
            if (outAttribMaxElemnum)
            {
                UT_ASSERT_P(GFE_Type::isValidOffset(attrib->getIndexMap(), attribMaxElemoff));
                attribMaxElemoff = attrib->getIndexMap().indexFromOffset(attribMaxElemoff);
            }
        }
        if (outAttribMinElemnum)
            setAttribValue(attribMinElemnum, attribMinElemoff);
        if (outAttribMaxElemnum)
            setAttribValue(attribMaxElemnum, attribMaxElemoff);
        
    }

    template<typename _Ty>
    void setAttribValue(GA_Attribute* const attrib, const _Ty val)
    {
        if (!attrib)
            return;
        const GA_AIFTuple* const aIFTuple = attrib->getAIFTuple();
        if (!aIFTuple)
            return;
        aIFTuple->set(attrib, 0, val);
    }



        
private:
        
#if !GFE_AttributeExtremum_UseVariant
        union minU
        {
            fpreal f;
            exint  i;
        };
        union maxU
        {
            fpreal f;
            exint  i;
        };
#endif



        
public:
    bool outAttribMin = false;
    bool outAttribMax = false;
    bool outAttribMinElemnum = false;
    bool outAttribMaxElemnum = false;
    
    bool outAsOffset = true;

    GA_Attribute* attribMin = nullptr;
    GA_Attribute* attribMax = nullptr;
    GA_Attribute* attribMinElemnum = nullptr;
    GA_Attribute* attribMaxElemnum = nullptr;
    //const char* outAttribMinAttribName = nullptr;
    //const char* outAttribMaxAttribName = nullptr;
    //const char* outAttribMinElemnumAttribName = nullptr;
    //const char* outAttribMaxElemnumAttribName = nullptr;

    
private:
    
#if GFE_AttributeExtremum_UseVariant
    std::variant<fpreal, exint> min;
    std::variant<fpreal, exint> max;
#else
    bool isInt = true;
    minU min;
    maxU max;
#endif
    GA_Offset attribMinElemoff = GFE_INVALID_OFFSET;
    GA_Offset attribMaxElemoff = GFE_INVALID_OFFSET;
    
private:
    const GA_Attribute* attrib = nullptr;
    
    exint subscribeRatio = 64;
    exint minGrainSize   = 1024;



        
    
};








#endif





