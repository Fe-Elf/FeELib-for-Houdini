
#pragma once

#ifndef __GFE_RestVectorComponent_h__
#define __GFE_RestVectorComponent_h__

#include "GFE/GFE_RestVectorComponent.h"

#include "GFE/GFE_GeoFilter.h"



#include "UFE/UFE_SplittableString.h"

class GFE_RestVectorComponent : public GFE_AttribCreateFilterWithRef0 {
    
#define __TEMP_GFE_REST_VECTOR_COMPONENT_ATTRIBNAME "__TEMP_GFE_REST_VECTOR_COMPONENT_ATTRIBNAME"
    
public:

    using GFE_AttribCreateFilterWithRef0::GFE_AttribCreateFilterWithRef0;
    
    // GFE_RestVectorComponent(
    //     GA_Detail& geo,
    //     const GA_Detail* const geoRef,
    //     const SOP_NodeVerb::CookParms* const cookparms = nullptr
    // )
    //     : GFE_AttribCreateFilter(geo, cookparms)
    //     , GFE_GeoFilterRef(geoRef, groupParser.getGOPRef(), cookparms)
    // {
    // }

    
    void
        setComputeParm(
            const int8 comp = 1,
            const bool delOrigin = false,
            const exint subscribeRatio = 64,
            const exint minGrainSize = 1024
        )
    {
        setHasComputed();
        this->comp = comp;
        this->delOrigin = delOrigin;
        this->subscribeRatio = subscribeRatio;
        this->minGrainSize = minGrainSize;
    }
    

    SYS_FORCE_INLINE void setRestAttrib(GA_Attribute* const attribPtr)
    {
        UT_ASSERT_MSG(&attribPtr->getDetail() == geo, "not same detail");
        getRef0AttribArray().clear();
        getInAttribArray().set(attribPtr);
    }

    SYS_FORCE_INLINE void setRestAttrib(const GA_Attribute* const attribPtr)
    {
        UT_ASSERT_MSG(&attribPtr->getDetail() == geoRef0, "not same detail");
        getInAttribArray().clear();
        getRef0AttribArray().set(attribPtr);
    }
    
    SYS_FORCE_INLINE void setRestAttrib(const GA_AttributeOwner owner, const UT_StringHolder& name)
    {
        if (geoRef0)
        {
            getInAttribArray().clear();
            getRef0AttribArray().append(owner, name);
        }
        else
        {
            getRef0AttribArray().clear();
            getInAttribArray().append(owner, name);
        }
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

        const size_t len = getInAttribArray().size();
        for (size_t i = 0; i < len; ++i)
        {
            attribRefPtr = getInAttribArray()[i];
            attribRefPtrNonConst = getInAttribArray()[i];
            UT_ASSERT_P(attribRefPtr);
            restVectorComponent();
        }
        const size_t ref0AttribLen = getRef0AttribArray().size();
        for (size_t i = 0; i < ref0AttribLen; ++i)
        {
            attribRefPtr = getRef0AttribArray()[i];
            attribRefPtrNonConst = nullptr;
            UT_ASSERT_P(attribRefPtr);
            restVectorComponent();
        }
        return true;
    }

    void restVectorComponent()
    {
        const GA_Attribute& attribRef = *attribRefPtr;
        
        
        const UT_StringHolder& newName = newAttribNames.getIsValid() ? newAttribNames.getNext<UT_StringHolder>() : attribRef.getName();
        const bool detached = !GFE_Type::isPublicAttribName(newName);

        const GA_Storage storage = GFE_Attribute::getStorage(attribRef);
        //if (!detached && !attrib.isDetached() && attrib.getName() == newName)
        if (!detached && !attribRef.isDetached() && strcmp(attribRef.getName().c_str(), newName.c_str()) == 0)
        {
            attribRest = getOutAttribArray().findOrCreateTuple(
                false, attribRef.getOwner(), GA_STORECLASS_FLOAT, storage, __TEMP_GFE_REST_VECTOR_COMPONENT_ATTRIBNAME);
            
            attribDuplicate();
            
            if (attribRefPtrNonConst)
                geo->destroyAttrib(attribRefPtrNonConst);
            geo->renameAttrib(attribRest, newName);
        }
        else
        {
            attribRest = getOutAttribArray().findOrCreateTuple(
                detached, attribRef.getOwner(), GA_STORECLASS_FLOAT, storage, newName);
            
            attribDuplicate();
            
            if (delOrigin && attribRefPtrNonConst)
                geo->destroyNonDetachedAttrib(attribRefPtrNonConst);
        }
    }
        

    void attribDuplicate()
    {
        const GA_AIFTuple* const aifTuple = attribRefPtr->getAIFTuple();
        if (!aifTuple)
            return;

        const int tupleSize = attribRefPtr->getTupleSize();
        if (comp >= tupleSize)
            return;

        const GFE_AttribStorage attribStorage = GFE_Type::getAttribStorage(attribRefPtr);
        if (!GFE_Variant::isAttribStorageIFV(attribStorage))
            return;
        auto storageVariant = GFE_Variant::getAttribStorageVariantIFV(attribStorage);
            
        std::visit([&] (auto storageVariant)
        {
            using type = typename GFE_Variant::getAttribStorage_t<storageVariant>;
            restVectorComponent<type>();
        }, storageVariant);
    }

    
    template<typename _Ty>
    void restVectorComponent()
    {
        using value_type = typename GFE_Type::get_value_type_t<_Ty>;
        UTparallelFor(groupParser.getSplittableRange(attribRefPtr), [this](const GA_SplittableRange& r)
        {
            GFE_RWPageHandleT<value_type> attrib_ph(attribRest);
            GFE_ROPageHandleT<_Ty> attribRef_ph(attribRefPtr);
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
                            attrib_ph.value(elemoff) = attribRef_ph.value(elemoff);
                        else
                            attrib_ph.value(elemoff) = attribRef_ph.value(elemoff)[comp];
                    }
                }
            }
        }, subscribeRatio, minGrainSize);
    }



public:
    UFE_SplittableString newAttribNames;
    
    int8 comp = 1;
    bool delOrigin = false;
private:

    GA_Attribute* attribRest;
    
    GA_Attribute* attribRefPtrNonConst;
    const GA_Attribute* attribRefPtr;
    
    
    
    exint subscribeRatio = 64;
    exint minGrainSize   = 1024;

#undef __TEMP_GFE_REST_VECTOR_COMPONENT_ATTRIBNAME
    
}; // End of Class GFE_RestVectorComponent








#endif
