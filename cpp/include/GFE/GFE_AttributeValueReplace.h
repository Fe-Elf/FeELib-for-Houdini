
#pragma once

#ifndef __GFE_AttributeValueReplace_h__
#define __GFE_AttributeValueReplace_h__

#include "GFE_RestVectorComponent.h"
#include "GFE/GFE_AttributeValueReplace.h"



#include "GFE/GFE_GeoFilter.h"


#include "UFE/UFE_SplittableString.h"



class GFE_AttribValReplace : public GFE_AttribCreateFilter {

    
#define GFE_TEMP_ATTRIB_VALUE_REPLACE_ATTRIBNAME "__GFE_TEMP_ATTRIB_VALUE_REPLACE_ATTRIBNAME"
    
using EntriesValueType = UT_Array<SOP_FeE_AttribValReplace_1_0Parms::NumEntries>;

    
public:
    using GFE_AttribCreateFilter::GFE_AttribCreateFilter;

    void
        setComputeParm(
            const EntriesValueType& entries,
            const bool outNewAttrib = false,
            const UT_StringRef& newAttribNames = "",
            const bool delInAttrib = true,
            const exint subscribeRatio = 64,
            const exint minGrainSize   = 1024
        )
    {
        setHasComputed();
        this->entries        = &entries;
        this->outNewAttrib   = outNewAttrib;
        this->newAttribNames = newAttribNames;
        this->delInAttrib    = delInAttrib;
        this->subscribeRatio = subscribeRatio;
        this->minGrainSize   = minGrainSize;
    }


    
private:

    virtual bool
        computeCore() override
    {
        if (outNewAttrib ? getInAttribArray().isEmpty() : getOutAttribArray().isEmpty())
            return false;
        
        if (!entries)
            return false;

        if (groupParser.isEmpty())
            return true;
        
        const UT_StringHolder emptyString = "";
        const UT_StringHolder tmpName = GFE_TEMP_ATTRIB_VALUE_REPLACE_ATTRIBNAME;
        
        if (outNewAttrib)
        {
            const size_t size = getInAttribArray().size();
            for (size_t i = 0; i < size; ++i)
            {
                attrib = getInAttribArray()[i];

                const UT_StringHolder& newName = newAttribNames.getIsValid() ? newAttribNames.getNext<UT_StringHolder>() : emptyString;

                const bool detached = !GFE_Type::isPublicAttribName(newName);
                
                if (detached)
                {
                    attribNew = getOutAttribArray().findOrCreateTuple(detached, attrib->getOwner(),
                        newStorageClass, GFE_Type::getPreferredStorage(newStorageClass, newPrecision), "");
                }
                else
                {
                    isSameName = strcmp(attrib->getName().c_str(), newName.c_str()) == 0;
                    const UT_StringHolder& newNameTemp = isSameName ? tmpName : newName;

                    attribNew = getOutAttribArray().findOrCreateTuple(detached, attrib->getOwner(),
                        newStorageClass, GFE_Type::getPreferredStorage(newStorageClass, newPrecision), newNameTemp);
                }
                
                const GFE_AttribStorage attribStorageNew = GFE_Type::getAttribStorage(attribNew);
                const GFE_AttribStorage attribStorage = GFE_Type::getAttribStorage(attrib);
                if (!GFE_Variant::isAttribStorageIFS(attribStorage) || !GFE_Variant::isAttribStorageIFS(attribStorageNew))
                    continue;
                auto storageNewVariant = GFE_Variant::getAttribStorageVariantIFS(attribStorageNew);
                auto storageVariant = GFE_Variant::getAttribStorageVariantIFS(attribStorage);
                std::visit([&] (auto storageVariant, auto storageNewVariant)
                {
                    using ENTRY_T = typename GFE_Variant::getAttribStorage_t<storageNewVariant>;
                    using KEY_T = typename GFE_Variant::getAttribStorage_t<storageVariant>;
                    
                    attribValReplace<KEY_T, ENTRY_T>();
                }, storageVariant, storageNewVariant);

                
                if (delInAttrib)
                    geo->destroyAttrib(attrib);

                
                if (!detached && isSameName)
                {
                    if (!delInAttrib)
                        geo->destroyAttrib(attrib);
                    geo->renameAttrib(attribNew, newName);
                }
            }
            
        }
        else
        {
            const size_t size = getOutAttribArray().size();
            for (size_t i = 0; i < size; ++i)
            {
                attrib = getOutAttribArray()[i];

                const GFE_AttribStorage attribStorage = GFE_Type::getAttribStorage(attrib);
                if (!GFE_Variant::isAttribStorageIS(attribStorage))
                    continue;
                auto storageVariant = GFE_Variant::getAttribStorageVariantIS(attribStorage);
                std::visit([&] (auto storageVariant)
                {
                    using KEY_T = typename GFE_Variant::getAttribStorage_t<storageVariant>;
                    attribValReplace<KEY_T>();
                }, storageVariant);
                
            }
        }

        
        return true;
    }
    
    
    template<typename KEY_T, typename ENTRY_T>
    void attribValReplace()
    {
        UT_Map<KEY_T, ENTRY_T> dict = genGntriesDict<KEY_T, ENTRY_T>();
        
#if 1
        std::unique_ptr<GA_RWHandleS> attribNew_h;
        std::unique_ptr<GA_ROHandleS> attrib_h;
            
        if constexpr (GFE_Type::isString<ENTRY_T>)
            attribNew_h = std::make_unique<GA_RWHandleS>(attribNew);
        if constexpr (GFE_Type::isString<KEY_T>)
            attrib_h = std::make_unique<GA_ROHandleS>(attrib);
#else
        const GA_RWHandleS attribNew_h(attribNew);
        const GA_ROHandleS attrib_h(attrib);
#endif
        UTparallelFor(groupParser.getSplittableRange(attrib), [this, &dict, &attrib_h, &attribNew_h](const GA_SplittableRange& r)
        {
#if 1
            std::unique_ptr<GFE_RWPageHandleT<ENTRY_T>> attribNew_ph;
            std::unique_ptr<GFE_ROPageHandleT<KEY_T>> attrib_ph;
            
            if constexpr (!GFE_Type::isString<ENTRY_T>)
                attribNew_ph = std::make_unique<GFE_RWPageHandleT<ENTRY_T>>(attribNew);
            if constexpr (!GFE_Type::isString<KEY_T>)
                attrib_ph = std::make_unique<GFE_ROPageHandleT<KEY_T>>(attrib);
#else
            GFE_RWPageHandleT<ENTRY_T> attribNew_ph(attribNew);
            GFE_ROPageHandleT<KEY_T> attrib_ph(attrib);
#endif
            GA_Offset start, end;
            for (GA_PageIterator pit = r.beginPages(); !pit.atEnd(); ++pit)
            {
                for (GA_Iterator it(pit.begin()); it.blockAdvance(start, end); )
                {
                    if constexpr (!GFE_Type::isString<ENTRY_T>)
                        attribNew_ph->setPage(start);
                    if constexpr (!GFE_Type::isString<KEY_T>)
                        attrib_ph->setPage(start);
                    for (GA_Offset elemoff = start; elemoff < end; ++elemoff)
                    {
                        if constexpr (GFE_Type::isString<KEY_T>)
                        {
                            if (!dict.contains(attrib_h->get(elemoff)))
                                continue;
                        }
                        else
                        {
                            if (!dict.contains(attrib_ph->value(elemoff)))
                                continue;
                        }
                                    
                        if constexpr (GFE_Type::isString<ENTRY_T>)
                        {
                            if constexpr (GFE_Type::isString<KEY_T>)
                                attribNew_h->set(elemoff, dict[attrib_h->get(elemoff)]);
                            else
                                attribNew_h->set(elemoff, dict[attrib_ph->value(elemoff)]);
                        }
                        else
                        {
                            if constexpr (GFE_Type::isString<KEY_T>)
                                attribNew_ph->value(elemoff) = dict[attrib_h->get(elemoff)];
                            else
                                attribNew_ph->value(elemoff) = dict[attrib_ph->value(elemoff)];
                        }
                    }
                }
            }
        }, subscribeRatio, minGrainSize);
    }

    
    template<typename KEY_T>
    void attribValReplace()
    {
        using ENTRY_T = KEY_T;
        UT_Map<KEY_T, ENTRY_T> dict = genGntriesDict<KEY_T, ENTRY_T>();
        
        if constexpr(GFE_Type::isString<KEY_T>)
        {
            const GA_RWHandleS attrib_h(attrib);
            UTparallelFor(groupParser.getSplittableRange(attrib), [this, &dict, &attrib_h](const GA_SplittableRange& r)
            {
                GA_Offset start, end;
                for (GA_Iterator it(r); it.blockAdvance(start, end); )
                {
                    for (GA_Offset elemoff = start; elemoff < end; ++elemoff)
                    {
                        if (dict.contains(attrib_h.get(elemoff)))
                            attrib_h.set(elemoff, dict[attrib_h.get(elemoff)]);
                    }
                }
            }, subscribeRatio, minGrainSize);
        }
        else
        {
            UTparallelFor(groupParser.getSplittableRange(attrib), [this, &dict](const GA_SplittableRange& r)
            {
                GFE_RWPageHandleT<KEY_T> attrib_ph(attrib);
                GA_Offset start, end;
                for (GA_PageIterator pit = r.beginPages(); !pit.atEnd(); ++pit)
                {
                    for (GA_Iterator it(pit.begin()); it.blockAdvance(start, end); )
                    {
                        attrib_ph.setPage(start);
                        for (GA_Offset elemoff = start; elemoff < end; ++elemoff)
                        {
                            if (dict.contains(attrib_ph.value(elemoff)))
                                attrib_ph.value(elemoff) = dict[attrib_ph.value(elemoff)];
                        }
                    }
                }
            }, subscribeRatio, minGrainSize);
        }
    }


    
    template<typename KEY_T, typename ENTRY_T>
    UT_Map<KEY_T, ENTRY_T> genGntriesDict()
    {
        UT_ASSERT_P(entries);
        
        UT_Map<KEY_T, ENTRY_T> dict;
        const size_t numEntries = entries->size();
        for (size_t i = 0; i < numEntries; ++i)
        {
            const SOP_FeE_AttribValReplace_1_0Parms::NumEntries& entry = entries->operator[](i);
            if (!entry.enable)
                continue;
            
            if constexpr (std::is_floating_point_v<KEY_T>)
            {
                dict[KEY_T(std::atof(entry.key.c_str()))] = getEntryValue<ENTRY_T>(entry);
            }
            else if constexpr (std::is_integral_v<KEY_T>)
            {
                dict[KEY_T(std::atol(entry.key.c_str()))] = getEntryValue<ENTRY_T>(entry);
            }
            else if constexpr(GFE_Type::isString<KEY_T>)
            {
                dict[entry.key] = getEntryValue<ENTRY_T>(entry);
            }
        }
        return dict;
    }

    template<typename ENTRY_T>
    const ENTRY_T getEntryValue(const SOP_FeE_AttribValReplace_1_0Parms::NumEntries& entry)
    {
        if constexpr (std::is_floating_point_v<ENTRY_T>)
        {
            return entry.valf;
        }
        else if constexpr (std::is_integral_v<ENTRY_T>)
        {
            return entry.vali;
        }
        else if constexpr(GFE_Type::isString<ENTRY_T>)
        {
            return entry.vals;
        }
    }


    
public:
    const EntriesValueType* entries = nullptr;

    
    bool outNewAttrib = false;
    GA_StorageClass newStorageClass = GA_STORECLASS_INVALID;
    GA_Precision newPrecision = GA_PRECISION_INVALID;

    UFE_SplittableString newAttribNames;
    
    bool delInAttrib = true;
    
    
private:
    
    GA_Attribute* attrib;
    GA_Attribute* attribNew;
    bool isSameName;
    
    exint subscribeRatio = 64;
    exint minGrainSize   = 1024;

    
#undef GFE_TEMP_ATTRIB_VALUE_REPLACE_ATTRIBNAME
    
}; // End of class GFE_AttribValReplace








#endif
