
#pragma once

#ifndef __GFE_SetVectorComponent_h__
#define __GFE_SetVectorComponent_h__

#include "GFE/GFE_SetVectorComponent.h"

#include "GFE/GFE_GeoFilter.h"



#include "GFE/GFE_RestVectorComponent.h"


    
#if 0
        GFE_SetVectorComponent setVectorComponent(geo, nullptr, cookparms);
        setVectorComponent.comp = comp;
        setVectorComponent.getOutAttribArray().set(posAttribNonConst);
        setVectorComponent.setRefAttrib();
        //setVectorComponent.setRestAttrib("");
        setVectorComponent.compute();
        UT_ASSERT_P(!setVectorComponent.getRestAttrib().isEmpty());
#endif

class GFE_SetVectorComponent : public GFE_AttribFilterWithRef0 {

public:

    //using GFE_AttribFilterWithRef0::GFE_AttribFilterWithRef0;

    GFE_SetVectorComponent(
        GFE_Detail* const geo,
        const GA_Detail* const geoRef,
        const SOP_NodeVerb::CookParms* const cookparms = nullptr
    )
        : GFE_AttribFilterWithRef0(geo, geoRef, cookparms)
        , restVecComp(geo, nullptr, cookparms)
    {
    }

    GFE_SetVectorComponent(
        GA_Detail& geo,
        const GA_Detail* const geoRef,
        const SOP_NodeVerb::CookParms* const cookparms = nullptr
    )
        : GFE_AttribFilterWithRef0(geo, geoRef, cookparms)
        , restVecComp(geo, nullptr, cookparms)
    {
    }
    
    void
        setComputeParm(
            const int8 comp = 1,
            const fpreal attribValF = 0,
            const bool delRefAttrib = true,
            const exint subscribeRatio = 64,
            const exint minGrainSize   = 1024
        )
    {
        setHasComputed();
        this->comp = comp;
        this->attribValF = attribValF;
        this->delRefAttrib = delRefAttrib;
        this->subscribeRatio = subscribeRatio;
        this->minGrainSize   = minGrainSize;
    }
    
    SYS_FORCE_INLINE void setComp(const int8 v = 1)
    { comp = v; }
    
    SYS_FORCE_INLINE void setComp(const GFE_Axis axis = GFE_Axis::Y)
    { comp = static_cast<int8>(axis); }
    
    void setRefAttrib(const GA_Attribute* const attrib)
    {
        if (getOutAttribArray().isEmpty() || attrib->getOwner() != getOutAttribArray()[0]->getOwner())
        {
            if (cookparms)
                cookparms->sopAddWarning(0, "must add out attrib before add ref attrib");
            return;
        }
        attribRef = attrib;
        refAttribNames = "";
        UT_ASSERT(getOutAttribArray().isEmpty() ? 1 : attrib->getOwner() == getOutAttribArray()[0]->getOwner());
    }
    
    SYS_FORCE_INLINE void setRefAttrib(GA_Attribute* const attrib)
    {
        attribRefPtrNonConst = attrib;
        setRefAttrib(static_cast<const GA_Attribute*>(attrib));
    }
    
    // SYS_FORCE_INLINE GA_Attribute* setRefAttrib(const UT_StringHolder& attribPattern)
    // {
    //     // if (!attribPattern.isstring() || attribPattern.length() == 0)
    //     //     attribRef = nullptr;
    //     if (getOutAttribArray().isEmpty())
    //         return nullptr;
    //     const GFE_Detail* const inGeo = geoRef0 ? geoRef0 : geo;
    //     attribRef = inGeo->findAttribute(getOutAttribArray()[0]->getOwner(), attribPattern);
    //     return attribRef;
    // }
    
    SYS_FORCE_INLINE void setRefAttrib(const char* const attribPattern)
    { refAttribNames = attribPattern; }

    //SYS_FORCE_INLINE void setRefAttrib(const UT_StringRef& attribPattern)
    //{ refAttribNames = attribPattern; }


    
    // SYS_FORCE_INLINE void setRestAttrib(GA_Attribute* const attrib)
    // {
    //     attribRestPtr = attrib;
    //     UT_ASSERT(getOutAttribArray().isEmpty() ? 1 : attribRestPtr->getOwner() == getOutAttribArray()[0]->getOwner());
    // }
    // SYS_FORCE_INLINE GA_Attribute* setRestAttrib(const UT_StringHolder& attribPattern)
    // {
    //     // if (!attribPattern.isstring() || attribPattern.length() == 0)
    //     //     attribRef = nullptr;
    //     
    //     // UT_ASSERT(!getOutAttribArray().isEmpty());
    //     
    //     if (getOutAttribArray().isEmpty())
    //         return nullptr;
    //     attribRestPtr = geo->findAttribute(getOutAttribArray()[0]->getOwner(), attribPattern);
    //     return attribRestPtr;
    // }
    
    SYS_FORCE_INLINE void clearRestAttrib()
    { restVecComp.getOutAttribArray().clear(); }
    
    SYS_FORCE_INLINE void setRestAttrib()
    { rest = false; }
    
    SYS_FORCE_INLINE void setRestAttrib(const char* const attribPattern)
    { rest = true; restVecComp.newAttribNames = attribPattern; }
    
    SYS_FORCE_INLINE const GFE_AttributeArray& getRestAttrib() const
    { return restVecComp.getOutAttribArray(); }
    
    SYS_FORCE_INLINE virtual void bumpDataId() const override
    {
        GFE_AttribFilterWithRef0::bumpDataId();
        restVecComp.bumpDataId();
    }
    
private:


    virtual bool
        computeCore() override
    {
        UT_ASSERT(comp >= 0);
        UT_ASSERT(comp <= 3);
        if (comp < 0 || comp > 3)
            return false;
        
        if (groupParser.isEmpty())
            return true;

        if (rest)
        {
            restVecComp.groupParser.setGroup(groupParser);
            restVecComp.comp = comp;
        }
        
        const size_t len = getOutAttribArray().size();
        for (size_t i = 0; i < len; ++i)
        {
            attrib = getOutAttribArray()[i];
            UT_ASSERT_P(attrib);

            if (rest)
            {
                restVecComp.setRestAttrib(attrib);
                restVecComp.compute();
            }
            
            if (refAttribNames.getIsValid())
            {
                if (geoRef0)
                {
                    attribRef = geoRef0->findAttribute(attrib->getOwner(), refAttribNames.getNext<UT_StringHolder>());
                    if (attribRef && !attribRef->getAIFTuple())
                        attribRef = nullptr;
                }
                else
                {
                    attribRefPtrNonConst = geo->findAttribute(attrib->getOwner(), refAttribNames.getNext<UT_StringHolder>());
                    if (attribRefPtrNonConst && !attribRefPtrNonConst->getAIFTuple())
                        attribRefPtrNonConst = nullptr;
                    
                    attribRef = attribRefPtrNonConst;
                }
            }
            
            const GA_AIFTuple* const aifTuple = attrib->getAIFTuple();
            if (!aifTuple)
                continue;

            const int tupleSize = attrib->getTupleSize();
            if (comp >= tupleSize)
                continue;

            if (attribRef)
            {
                UT_ASSERT_P(attribRef->getAIFTuple());
                const int tupleSizeRef = attribRef->getTupleSize();
                if (comp >= tupleSizeRef && tupleSizeRef >= 2)
                    continue;
            }

            const GFE_AttribStorage attribStorage = GFE_Type::getAttribStorage(attrib);
            if (!GFE_Variant::isAttribStorageFVF(attribStorage))
                continue;
            auto storageVariant = GFE_Variant::getAttribStorageVariantFVF(attribStorage);
            
            if (attribRef)
            {
                const GFE_AttribStorage attribStorageRef = GFE_Type::getAttribStorage(attribRef);
                if (!GFE_Variant::isAttribStorageFVF(attribStorageRef))
                    continue;
                auto storageRefVariant = GFE_Variant::getAttribStorageVariantFVF(attribStorageRef);
            
                std::visit([&] (auto storageVariant, auto storageRefVariant)
                {
                    using type = typename GFE_Variant::getAttribStorage_t<storageVariant>;
                    using type_ref = typename GFE_Variant::getAttribStorage_t<storageRefVariant>;
                    setVectorComponent<type, type_ref>();
                }, storageVariant, storageRefVariant);
            }
            else
            {
                std::visit([&] (auto storageVariant)
                {
                    using type = typename GFE_Variant::getAttribStorage_t<storageVariant>;
                    setVectorComponent<type>();
                }, storageVariant);
            }
        }
        
        if (delRefAttrib && !geoRef0 && attribRefPtrNonConst)
            geo->destroyAttrib(attribRefPtrNonConst);
        
        return true;
    }


    
    template<typename _Ty>
    void setVectorComponent()
    {
        using value_type = typename GFE_Type::get_value_type_t<_Ty>;
        const value_type attribVal = attribValF;
        UTparallelFor(groupParser.getSplittableRange(attrib), [this, attribVal](const GA_SplittableRange& r)
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
                        if constexpr (GFE_Type::isScalar<_Ty>)
                            attrib_ph.value(elemoff) = attribVal;
                        else
                            attrib_ph.value(elemoff)[comp] = attribVal;
                    }
                }
            }
        }, subscribeRatio, minGrainSize);
    }


    template<typename _Ty, typename _T_REF>
    void setVectorComponent()
    {
        using value_type_ref = typename GFE_Type::get_value_type_t<_T_REF>;
        UT_ASSERT_P(attribRef);
        UT_ASSERT(attrib->getOwner() == attribRef->getOwner());
        UTparallelFor(groupParser.getSplittableRange(attrib), [this](const GA_SplittableRange& r)
        {
            GFE_RWPageHandleT<_Ty> attrib_ph(attrib);
            GFE_ROPageHandleT<_T_REF> attribRef_ph(attribRef);
            GA_Offset start, end;
            for (GA_PageIterator pit = r.beginPages(); !pit.atEnd(); ++pit)
            {
                for (GA_Iterator it(pit.begin()); it.blockAdvance(start, end); )
                {
                    attrib_ph.setPage(start);
                    attribRef_ph.setPage(start);
                    for (GA_Offset elemoff = start; elemoff < end; ++elemoff)
                    {
                        if constexpr (GFE_Type::isScalar<_Ty>)
                            attrib_ph.value(elemoff) = getPageHandleComp<_T_REF, value_type_ref>(attribRef_ph, elemoff);
                        else
                            attrib_ph.value(elemoff)[comp] = getPageHandleComp<_T_REF, value_type_ref>(attribRef_ph, elemoff);
                    }
                }
            }
        }, subscribeRatio, minGrainSize);
    }


    template<typename _Ty, typename value_type>
    SYS_FORCE_INLINE value_type getPageHandleComp(GFE_ROPageHandleT<_Ty>& attrib_ph, const GA_Offset elemoff) const
    {
        if constexpr (GFE_Type::isScalar<_Ty>)
            return attrib_ph.get(elemoff);
        else
            return attrib_ph.get(elemoff)[comp];
    }

public:
    UFE_SplittableString refAttribNames;
    // UFE_SplittableString restAttribNames;
    
    int8 comp = 1;
    fpreal attribValF = 0;
    
    bool delRefAttrib = false;
    // bool restAttrib = false;

private:

    GA_Attribute* attrib;
    const GA_Attribute* attribRef = nullptr;
    GA_Attribute* attribRefPtrNonConst = nullptr;
    //GA_Attribute* attribRestPtr;
    
    bool rest = false;
    GFE_RestVectorComponent restVecComp;
    
    
    exint subscribeRatio = 64;
    exint minGrainSize = 1024;
};









#endif
