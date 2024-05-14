
#pragma once

#ifndef __GFE_ScaleAttributeElement_h__
#define __GFE_ScaleAttributeElement_h__

#include "GFE/GFE_ScaleAttributeElement.h"
#include <GFE/GFE_ScaleAttributeElement.h>

#include <GFE/GFE_GeoFilter.h>



class GFE_ScaleAttribElement : public GFE_AttribFilter {


public:

    using GFE_AttribFilter::GFE_AttribFilter;


    ~GFE_ScaleAttribElement()
    {
    }


void
    setComputeParm(
        const bool doNormalize = true,
        const fpreal uniScale = 1.0,
        const exint subscribeRatio = 64,
        const exint minGrainSize   = 1024
    )
{
    setHasComputed();
    this->doNormalize = doNormalize;
    this->uniScale = uniScale;
    this->subscribeRatio = subscribeRatio;
    this->minGrainSize   = minGrainSize;
}


    
private:

    virtual bool
        computeCore() override
    {
        if (!getOutAttribArray().size())
            return false;

        if (!doNormalize && uniScale==1.0)
            return true;
        
        if (groupParser.isEmpty())
            return true;

        const size_t size = getOutAttribArray().size();
        for (size_t i = 0; i < size; i++)
        {
            attrib = getOutAttribArray()[i];
            
            scaleAttribElement();
            
            const GFE_AttribStorage attribStorage = GFE_Type::getAttribStorage(attrib);
            if (!GFE_Variant::isAttribStorageIFV(attribStorage))
                return;
            auto storageVariant = GFE_Variant::getAttribStorageVariantIFV(attribStorage);
            auto doNormalizeVariant = GFE_Variant::getBoolVariant(doNormalize);
            auto scaleVariant = GFE_Variant::getBoolVariant(uniScale != 1.0);
            std::visit([&] (auto storageVariant, auto scaleVariant, auto doNormalizeVariant)
            {
                using type = typename GFE_Variant::getAttribStorage_t<storageVariant>;
                scaleAttribElement<type, doNormalizeVariant, scaleVariant>();
            }, storageVariant, scaleVariant, doNormalizeVariant);
        }
        return true;
    }

    


    template<typename _Ty, bool _normalize, bool _scale>
    void scaleAttribElement()
    {
        UTparallelFor(groupParser.getSplittableRange(attrib), [this](const GA_SplittableRange& r)
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
                        if constexpr (_normalize && GFE_Type::isVector<_Ty>)
                            attrib_ph.value(elemoff).normalize();
                        if constexpr (_scale)
                            attrib_ph.value(elemoff) *= uniScale;
                    }
                }
            }
        }, subscribeRatio, minGrainSize);
    }
    
/*
    void scaleAttribElement()
    {
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
    */
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
    bool doNormalize = true;
    fpreal uniScale = 1.0;

private:
    GA_Attribute* attrib;
    GA_Attribute* refattrib;

    
    exint subscribeRatio = 64;
    exint minGrainSize   = 1024;

    
};








#endif





