
#pragma once

#ifndef __GFE_AttributeCast_h__
#define __GFE_AttributeCast_h__

#include <GFE/GFE_AttributeCast.h>

#include <GFE/GeoFilter.h>

#include <UFE/SplittableString.h>

#if 0
        gfe::AttribCast attribCast(geo, cookparms);
        attribCast.getInAttribArray().set(outAttrib);
        attribCast.newStorageClass = enumAttrib->getStorageClass();
        attribCast.newPrecision    = gfe::Attribute::getPrecision(enumAttrib);
        if (attribCast.newStorageClass == GA_STORECLASS_STRING)
        {
            attribCast.prefix = prefix;
            attribCast.sufix  = sufix;
        }
        attribCast.newAttribNames = sopparms.getNewAttribName();
        attribCast.compute();
#endif

_GFEL_BEGIN
class AttribCast : public AttribCreateFilter {
    
#define GFE_TEMP_ATTRIBCAST_ATTRIBNAME "__GFE_TEMP_ATTRIBCAST_ATTRIBNAME"
    
public:

    using AttribCreateFilter::AttribCreateFilter;
    
    void setComputeParm(
        const bool delOrigin = true,
        const exint subscribeRatio = 64,
        const exint minGrainSize   = 1024
    )
    {
        setHasComputed();
        this->delOrigin = delOrigin;
        this->subscribeRatio = subscribeRatio;
        this->minGrainSize   = minGrainSize;
    }

    void setDestinationAttribute(const GA_Attribute& inAttrib)
    {
        newStorageClass = inAttrib.getStorageClass();
        if (inAttrib.getAIFTuple())
        {
            newPrecision = gfe::precisionFromStorage(inAttrib.getAIFTuple()->getStorage(&inAttrib));
        }
        else if (inAttrib.getAIFNumericArray())
        {
            newPrecision = gfe::precisionFromStorage(inAttrib.getAIFNumericArray()->getStorage(&inAttrib));
        }
        else
        {
            newPrecision = GA_PRECISION_INVALID;
        }
        newAttribNames = inAttrib;
    }

    SYS_FORCE_INLINE void setDestinationGroup(const GA_Group& inGroup)
    {
        newStorageClass = GA_STORECLASS_OTHER;
        newPrecision    = GA_PRECISION_1;
        newGroupNames = inGroup;
    }


private:

    virtual bool
        computeCore() override
    {
        for (size_t i = 0; i < getInAttribArray().size(); i++)
        {
            const GA_Attribute& attrib = *getInAttribArray()[i];
            if (attrib.getAIFTuple() || attrib.getAIFStringTuple() || attrib.getAIFSharedStringTuple())
                continue;
            getInAttribArray().erase(i);
        }
        
        if (getInAttribArray().isEmpty() && getInGroupArray().isEmpty())
            return false;

        if (newStorageClass == GA_STORECLASS_INVALID)
            return false;
        
        if (groupParser.isEmpty())
            return true;
        
        newPrecision = geo->getValidPrecision(newPrecision);
        
        const size_t attribSize = getInAttribArray().size();
        for (size_t i = 0; i < attribSize; ++i)
        {
            attribCast(*getInAttribArray()[i]);
        }
            
        if (newStorageClass == GA_STORECLASS_OTHER)
            return true;

        const size_t groupSize = getInGroupArray().size();
        for (size_t i = 0; i < groupSize; ++i)
        {
            attribCast(*getInGroupArray()[i]);
        }
        
        return true;
    }


    void attribCast(GA_Attribute& attrib)
    {
        const UT_StringHolder& newName = newAttribNames.getIsValid() ? newAttribNames.getNext<UT_StringHolder>() : attrib.getName();
        const bool detached = gfe::isInvalid(newName);

#if GFE_DEBUG_MODE
        GA_StorageClass ga_StorageClass = attrib.getStorageClass();
        GA_Precision gA_Precision = gfe::Attribute::getPrecision(attrib);
        GA_Storage gA_Storage = gfe::Attribute::getStorage(attrib);
#endif
        if (attrib.getStorageClass() == newStorageClass
            &&  (   newStorageClass == GA_STORECLASS_INVALID
                 || newStorageClass == GA_STORECLASS_OTHER
                 || gfe::Attribute::getPrecision(attrib) == newPrecision
                )
        )
        {
            if (!detached && !attrib.isDetached())
            {
                if (attrib.getOwner() == GA_ATTRIB_POINT && gfe::stringEqualP(newName))
                    gfe::Attribute::clone(*geo->getP(), attrib, subscribeRatio, minGrainSize);
                else
                    geo->forceRenameAttribute(attrib, newName);
            }
            return;
        }
        
        if (newStorageClass == GA_STORECLASS_OTHER)
        {
            GA_Group& newAttrib = *getOutGroupArray().findOrCreate(detached, attrib.getOwner(), newName);
            
            attribDuplicate(newAttrib, attrib);

            if (delOrigin)
                geo->destroyNonDetachedAttrib(attrib);
        }
        else
        {
            const GA_Storage storage = gfe::getPreferredStorage(newStorageClass, newPrecision);
            GA_Attribute* newAttrib;
            if (!detached && !attrib.isDetached() && gfe::stringEqual(attrib.getName(), newName))
            {
                if (attrib.getOwner() == GA_ATTRIB_POINT && gfe::stringEqualP(newName))
                {
                    geo->asGEO_Detail()->changePointAttributeStorage("P", storage);
                    newAttrib = geo->getP();
                }
                else
                {
                    newAttrib = getOutAttribArray().findOrCreateTuple(false, attrib.getOwner(), newStorageClass, storage, GFE_TEMP_ATTRIBCAST_ATTRIBNAME);
                    attribDuplicate(*newAttrib, attrib);
                    
                    geo->destroyAttrib(attrib);
                    geo->renameAttrib(newAttrib, newName);
                }
            }
            else
            {
                if (attrib.getOwner() == GA_ATTRIB_POINT && gfe::stringEqualP(newName))
                {
                    geo->asGEO_Detail()->changePointAttributeStorage("P", storage);
                    newAttrib = geo->getP();
                }
                else
                {
                    newAttrib = getOutAttribArray().findOrCreateTuple(detached, attrib.getOwner(), newStorageClass, storage, newName);
                }
                attribDuplicate(*newAttrib, attrib);
                
                if (delOrigin)
                    geo->destroyNonDetachedAttrib(attrib);
            }
        }
    }

    void attribCast(GA_ElementGroup& group)
    {
        const GA_AttributeOwner attribClass = gfe::attributeOwner_groupType(group.classType());
        
        const UT_StringHolder& newName = newGroupNames.getValidAttribName(group);
        const bool detached = gfe::isInvalid(newName);
        
        GA_Attribute& newAttrib = *getOutAttribArray().findOrCreateTuple(
            detached, attribClass, newStorageClass, GA_STORE_INVALID, newName);
        
        attribDuplicate(newAttrib, group);
        
        if (delOrigin)
            geo->destroyElementGroup(&group);
    }
    
    SYS_FORCE_INLINE void attribCast(GA_Group& group)
    { attribCast(static_cast<GA_ElementGroup&>(group)); }

    
    template<typename T>
    void attribDuplicate(GA_ElementGroup& group, const GA_Attribute& attribRef)
    {
        UTparallelFor(groupParser.getSplittableRange(attribRef), [&group, &attribRef](const GA_SplittableRange& r)
        {
            GA_PageHandleT<T, T, true, false, const GA_Attribute, const GA_ATINumeric, const GA_Detail> attrib_ph(&attribRef);
            GA_Offset start, end;
            for (GA_PageIterator pit = r.beginPages(); !pit.atEnd(); ++pit)
            {
                for (GA_Iterator it(pit.begin()); it.blockAdvance(start, end); )
                {
                    attrib_ph.setPage(start);
                    for (GA_Offset elemoff = start; elemoff < end; ++elemoff)
                    {
                        group.setElement(elemoff, bool(attrib_ph.value(elemoff)));
                    }
                }
            }
        }, subscribeRatio, minGrainSize);
        group.invalidateGroupEntries();
    }
    
    template<>
    void attribDuplicate<UT_StringHolder>(GA_ElementGroup& group, const GA_Attribute& attribRef)
    {
        const GA_ROHandleS attrib_h(&attribRef);
        UTparallelFor(groupParser.getSplittableRange(attribRef), [&group, &attrib_h](const GA_SplittableRange& r)
        {
            GA_Offset start, end;
            for (GA_PageIterator pit = r.beginPages(); !pit.atEnd(); ++pit)
            {
                for (GA_Iterator it(pit.begin()); it.blockAdvance(start, end); )
                {
                    for (GA_Offset elemoff = start; elemoff < end; ++elemoff)
                    {
                        const UT_StringHolder& strval = attrib_h.get(elemoff);
                        group.setElement(elemoff, strval == "1" || strval == "true" || strval == "True");
                    }
                }
            }
        }, subscribeRatio, minGrainSize);
        group.invalidateGroupEntries();
    }
    
    void attribDuplicate(GA_ElementGroup& group, const GA_Attribute& attribRef)
    {
        switch (attribRef.getStorageClass())
        {
        case GA_STORECLASS_INT:
            switch (newPrecision)
            {
            case GA_PRECISION_8:  attribDuplicate<int8> (group, attribRef); break;
            case GA_PRECISION_16: attribDuplicate<int8> (group, attribRef); break;
            case GA_PRECISION_32: attribDuplicate<int16>(group, attribRef); break;
            case GA_PRECISION_64: attribDuplicate<int32>(group, attribRef); break;
            default:         break;
            }
            break;
        case GA_STORECLASS_REAL:
            switch (newPrecision)
            {
            case GA_PRECISION_16: attribDuplicate<fpreal16>(group, attribRef); break;
            case GA_PRECISION_32: attribDuplicate<fpreal32>(group, attribRef); break;
            case GA_PRECISION_64: attribDuplicate<fpreal64>(group, attribRef); break;
            default:         break;
            }
            break;
        case GA_STORECLASS_STRING: attribDuplicate<UT_StringHolder>(group, attribRef); break;
        case GA_STORECLASS_DICT:  break;
        case GA_STORECLASS_OTHER: break;
        default:             break;
        }
    }
    
    SYS_FORCE_INLINE void attribDuplicate(GA_Group& group, const GA_Attribute& attribRef)
    { return attribDuplicate(static_cast<GA_ElementGroup&>(group), attribRef); }
    



    bool attribDuplicate(GA_Attribute& attrib, const GA_ElementGroup& groupRef)
    {
        //const GA_SplittableRange geoSplittableRange = GFE_Group::getSplittableRangeByAnyGroup(*this, &group);
        const GA_SplittableRange geoSplittableRange = geo->getSplittableRangeByAnyGroup(groupRef);

        switch (newStorageClass)
        {
        case GA_STORECLASS_INT:
            switch (newPrecision)
            {
            case GA_PRECISION_8:  setAttribValueTo1<int8> (attrib, geoSplittableRange); return true; break;
            case GA_PRECISION_16: setAttribValueTo1<int16>(attrib, geoSplittableRange); return true; break;
            case GA_PRECISION_32: setAttribValueTo1<int32>(attrib, geoSplittableRange); return true; break;
            case GA_PRECISION_64: setAttribValueTo1<int64>(attrib, geoSplittableRange); return true; break;
            default:         break;
            }
            break;
        case GA_STORECLASS_REAL:
            switch (newPrecision)
            {
            case GA_PRECISION_16: setAttribValueTo1<fpreal16>(attrib, geoSplittableRange); return true; break;
            case GA_PRECISION_32: setAttribValueTo1<fpreal32>(attrib, geoSplittableRange); return true; break;
            case GA_PRECISION_64: setAttribValueTo1<fpreal64>(attrib, geoSplittableRange); return true; break;
            default:         break;
            }
            break;
        case GA_STORECLASS_STRING: setAttribValueTo1<UT_StringHolder>(attrib, geoSplittableRange); return true; break;
        case GA_STORECLASS_DICT:  break;
        case GA_STORECLASS_OTHER: break;
        default:                  break;
        }
        UT_ASSERT_MSG(0, "Unhandled Precision!");
        return false;
    }
    

    
    SYS_FORCE_INLINE void attribCast(GA_Group& group,const UT_StringHolder& newName)
    { attribCast(static_cast<GA_ElementGroup&>(group), newName); }
    




    
private:
    
    
    template<typename SCALAR_T, typename SCALAR_T_REF>
    SYS_FORCE_INLINE SCALAR_T scalarConvert(const SCALAR_T_REF inScalar)
    { return SCALAR_T(inScalar); }

#if 0
    template<>
    SYS_FORCE_INLINE int16 scalarConvert<int16, UT_StringHolder>(const UT_StringHolder inScalar)
    {
        return int16(atoi(inScalar.c_str()));
    }
    template<>
    SYS_FORCE_INLINE int32 scalarConvert<int32>(const char* const inScalar)
    {
        return atoi(inScalar);
    }
    template<>
    SYS_FORCE_INLINE int64 scalarConvert<int64>(const char* const inScalar)
    {
        return atol(inScalar);
    }
    template<>
    SYS_FORCE_INLINE fpreal16 scalarConvert<fpreal16>(const char* const inScalar)
    {
        return fpreal16(atof(inScalar));
    }
    template<>
    SYS_FORCE_INLINE fpreal32 scalarConvert<fpreal32>(const char* const inScalar)
    {
        return fpreal32(atof(inScalar));
    }
    template<>
    SYS_FORCE_INLINE fpreal64 scalarConvert<fpreal64>(const char* const inScalar)
    {
        return atof(inScalar);
    }
#else
    template<typename SCALAR_T>
    SYS_FORCE_INLINE SCALAR_T scalarConvertFromString(const char* const inScalar)
    { return atof(inScalar); }

    template<typename SCALAR_T>
    SYS_FORCE_INLINE SCALAR_T scalarConvertFromString(const UT_StringHolder& inScalar)
    { return scalarConvertFromString<SCALAR_T>(inScalar.c_str()); }

    template<>
    SYS_FORCE_INLINE int16 scalarConvertFromString<int16>(const char* const inScalar)
    { return int16(atoi(inScalar)); }
        
    template<>
    SYS_FORCE_INLINE int32 scalarConvertFromString<int32>(const char* const inScalar)
    { return atoi(inScalar); }
        
    template<>
    SYS_FORCE_INLINE int64 scalarConvertFromString<int64>(const char* const inScalar)
    { return atol(inScalar); }
        
    template<>
    SYS_FORCE_INLINE fpreal16 scalarConvertFromString<fpreal16>(const char* const inScalar)
    { return fpreal16(atof(inScalar)); }
        
    template<>
    SYS_FORCE_INLINE fpreal32 scalarConvertFromString<fpreal32>(const char* const inScalar)
    { return fpreal32(atof(inScalar)); }
        
    template<>
    SYS_FORCE_INLINE fpreal64 scalarConvertFromString<fpreal64>(const char* const inScalar)
    { return atof(inScalar); }
#endif
    
    template<>
    SYS_FORCE_INLINE UT_StringHolder scalarConvert<UT_StringHolder, int16>(const int16 inScalar)
    {
        char buffer[20];
        sprintf(buffer, "%s%d%s", prefix, inScalar, sufix);
        return UT_StringHolder(buffer);
    }
        
    template<>
    SYS_FORCE_INLINE UT_StringHolder scalarConvert<UT_StringHolder, int32>(const int32 inScalar)
    {
        char buffer[20];
        sprintf(buffer, "%s%d%s", prefix, inScalar, sufix);
        return UT_StringHolder(buffer);
    }
        
    template<>
    SYS_FORCE_INLINE UT_StringHolder scalarConvert<UT_StringHolder, int64>(const int64 inScalar)
    {
        char buffer[20];
        sprintf(buffer, "%s%I64d%s", prefix, inScalar, sufix);
        return UT_StringHolder(buffer);
    }
    template<>
    SYS_FORCE_INLINE UT_StringHolder scalarConvert<UT_StringHolder, fpreal16>(const fpreal16 inScalar)
    {
        char buffer[20];
        sprintf(buffer, "%s%f%s", prefix, fpreal32(inScalar), sufix);
        return UT_StringHolder(buffer);
    }
    template<>
    SYS_FORCE_INLINE UT_StringHolder scalarConvert<UT_StringHolder, fpreal32>(const fpreal32 inScalar)
    {
        char buffer[20];
        sprintf(buffer, "%s%f20%s", prefix, inScalar, sufix);
        return UT_StringHolder(buffer);
    }
    template<>
    SYS_FORCE_INLINE UT_StringHolder scalarConvert<UT_StringHolder, fpreal64>(const fpreal64 inScalar)
    {
        char buffer[20];
        sprintf(buffer, "%s%f20%s", prefix, inScalar, sufix);
        return UT_StringHolder(buffer);
    }
    
    template<typename SCALAR_T, typename SCALAR_T_REF>
    void attribDuplicate(GA_Attribute& attrib, const GA_Attribute& attribRef)
    {
        UTparallelFor(groupParser.getSplittableRange(attrib),
            [this, &attrib, &attribRef](const GA_SplittableRange& r)
        {
            GA_PageHandleT<SCALAR_T,     SCALAR_T,     true, true, GA_Attribute, GA_ATINumeric, GA_Detail> attrib_ph(&attrib);
            GA_PageHandleT<SCALAR_T_REF, SCALAR_T_REF, true, false, const GA_Attribute, const GA_ATINumeric, const GA_Detail> attribRef_ph(&attribRef);
            GA_Offset start, end;
            for (GA_PageIterator pit = r.beginPages(); !pit.atEnd(); ++pit)
            {
                for (GA_Iterator it(pit.begin()); it.blockAdvance(start, end); )
                {
                    attrib_ph.setPage(start);
                    attribRef_ph.setPage(start);
                    for (GA_Offset elemoff = start; elemoff < end; ++elemoff)
                    {
                        attrib_ph.value(elemoff) = scalarConvert<SCALAR_T, SCALAR_T_REF>(attribRef_ph.value(elemoff));
                    }
                }
            }
        }, subscribeRatio, minGrainSize);
    }



#if 0
    template<typename SCALAR_T_REF>
    void attribDuplicate<UT_StringHolder, SCALAR_T_REF>(GA_Attribute& attrib, const GA_Attribute& attribRef)
    {
        const GA_RWHandleS attrib_h(&attrib);
        UTparallelFor(groupParser.getSplittableRange(attrib),
            [this, &attrib_h, &attribRef](const GA_SplittableRange& r)
        {
            GA_PageHandleT<SCALAR_T_REF, SCALAR_T_REF, true, false, const GA_Attribute, const GA_ATINumeric, const GA_Detail> attribRef_ph(&attribRef);
            GA_Offset start, end;
            for (GA_PageIterator pit = r.beginPages(); !pit.atEnd(); ++pit)
            {
                for (GA_Iterator it(pit.begin()); it.blockAdvance(start, end); )
                {
                    attribRef_ph.setPage(start);
                    for (GA_Offset elemoff = start; elemoff < end; ++elemoff)
                    {
                        attrib_h.set(elemoff, scalarConvert<UT_StringHolder, SCALAR_T_REF>(attribRef_ph.value(elemoff)));
                    }
                }
            }
        }, subscribeRatio, minGrainSize);
    }
#else
    template<typename SCALAR_T_REF>
    void attribDuplicateString(GA_Attribute& attrib, const GA_Attribute& attribRef)
    {
        const GA_RWHandleS attrib_h(&attrib);
        UTparallelFor(groupParser.getSplittableRange(attrib),
            [this, &attrib_h, &attribRef](const GA_SplittableRange& r)
        {
            GA_PageHandleT<SCALAR_T_REF, SCALAR_T_REF, true, false, const GA_Attribute, const GA_ATINumeric, const GA_Detail> attribRef_ph(&attribRef);
            GA_Offset start, end;
            for (GA_PageIterator pit = r.beginPages(); !pit.atEnd(); ++pit)
            {
                for (GA_Iterator it(pit.begin()); it.blockAdvance(start, end); )
                {
                    attribRef_ph.setPage(start);
                    for (GA_Offset elemoff = start; elemoff < end; ++elemoff)
                    {
                        attrib_h.set(elemoff, scalarConvert<UT_StringHolder, SCALAR_T_REF>(attribRef_ph.get(elemoff)));
                    }
                }
            }
        }, subscribeRatio, minGrainSize);
    }
#endif



#if 0
    template<typename SCALAR_T>
    void attribDuplicate<SCALAR_T, UT_StringHolder>(GA_Attribute& attrib, const GA_Attribute& attribRef)
    {
        const GA_ROHandleS attribRef_h(&attribRef);
        UTparallelFor(groupParser.getSplittableRange(attrib),
            [this, &attrib, &attribRef_h](const GA_SplittableRange& r)
        {
            GA_PageHandleT<SCALAR_T, SCALAR_T, true, true, GA_Attribute, GA_ATINumeric, GA_Detail> attrib_ph(&attrib);
            GA_Offset start, end;
            for (GA_PageIterator pit = r.beginPages(); !pit.atEnd(); ++pit)
            {
                for (GA_Iterator it(pit.begin()); it.blockAdvance(start, end); )
                {
                    attrib_ph.setPage(start);
                    for (GA_Offset elemoff = start; elemoff < end; ++elemoff)
                    {
                        attrib_ph.value(elemoff) = scalarConvert<SCALAR_T, UT_StringHolder>(attribRef_h.get(elemoff));
                    }
                }
            }
        }, subscribeRatio, minGrainSize);
    }
#else
    template<typename SCALAR_T>
    void attribDuplicateFromString(GA_Attribute& attrib, const GA_Attribute& attribRef)
    {
        const GA_ROHandleS attribRef_h(&attribRef);
        UTparallelFor(groupParser.getSplittableRange(attrib),
            [this, &attrib, &attribRef_h](const GA_SplittableRange& r)
        {
            GA_PageHandleT<SCALAR_T, SCALAR_T, true, true, GA_Attribute, GA_ATINumeric, GA_Detail> attrib_ph(&attrib);
            GA_Offset start, end;
            for (GA_PageIterator pit = r.beginPages(); !pit.atEnd(); ++pit)
            {
                for (GA_Iterator it(pit.begin()); it.blockAdvance(start, end); )
                {
                    attrib_ph.setPage(start);
                    for (GA_Offset elemoff = start; elemoff < end; ++elemoff)
                    {
                        attrib_ph.value(elemoff) = scalarConvertFromString<SCALAR_T>(attribRef_h.get(elemoff));
                    }
                }
            }
        }, subscribeRatio, minGrainSize);
    }


#define GFE_ATTRIBDUPLICATE_SPECIALIZATION(SCALAR_T)                     \
    template<>                                                           \
    SYS_FORCE_INLINE void attribDuplicate<SCALAR_T, UT_StringHolder>     \
        (GA_Attribute& attrib, const GA_Attribute& attribRef)            \
    { attribDuplicateFromString<SCALAR_T>(attrib, attribRef); }          \


    GFE_ATTRIBDUPLICATE_SPECIALIZATION(int16)
    GFE_ATTRIBDUPLICATE_SPECIALIZATION(int32)
    GFE_ATTRIBDUPLICATE_SPECIALIZATION(int64)
    GFE_ATTRIBDUPLICATE_SPECIALIZATION(fpreal16)
    GFE_ATTRIBDUPLICATE_SPECIALIZATION(fpreal32)
    GFE_ATTRIBDUPLICATE_SPECIALIZATION(fpreal64)

#undef GFE_ATTRIBDUPLICATE_SPECIALIZATION
    


#define GFE_ATTRIBDUPLICATE_SPECIALIZATION(SCALAR_T_REF)                     \
    template<>                                                               \
    SYS_FORCE_INLINE void attribDuplicate<UT_StringHolder, SCALAR_T_REF>(    \
            GA_Attribute& attrib, const GA_Attribute& attribRef)             \
    { attribDuplicateString<SCALAR_T_REF>(attrib, attribRef); }              \


        GFE_ATTRIBDUPLICATE_SPECIALIZATION(int16)
        GFE_ATTRIBDUPLICATE_SPECIALIZATION(int32)
        GFE_ATTRIBDUPLICATE_SPECIALIZATION(int64)
        GFE_ATTRIBDUPLICATE_SPECIALIZATION(fpreal16)
        GFE_ATTRIBDUPLICATE_SPECIALIZATION(fpreal32)
        GFE_ATTRIBDUPLICATE_SPECIALIZATION(fpreal64)

#undef GFE_ATTRIBDUPLICATE_SPECIALIZATION

#endif


    // template<typename SCALAR_T>
    // void attribDuplicate(
    //     GA_Attribute& attrib,
    //     const GA_Attribute& attribRef
    // )
    // {
    //     switch (attribRef.getAIFTuple()->getStorage(&attribRef))
    //     {
    //     case GA_STORE_INT16:  attribDuplicate<SCALAR_T, int16>          (attrib, attribRef);  break;
    //     case GA_STORE_INT32:  attribDuplicate<SCALAR_T, int32>          (attrib, attribRef);  break;
    //     case GA_STORE_INT64:  attribDuplicate<SCALAR_T, int64>          (attrib, attribRef);  break;
    //     case GA_STORE_REAL16: attribDuplicate<SCALAR_T, fpreal16>       (attrib, attribRef);  break;
    //     case GA_STORE_REAL32: attribDuplicate<SCALAR_T, fpreal32>       (attrib, attribRef);  break;
    //     case GA_STORE_REAL64: attribDuplicate<SCALAR_T, fpreal64>       (attrib, attribRef);  break;
    //     case GA_STORE_STRING: attribDuplicate<SCALAR_T, UT_StringHolder>(attrib, attribRef);  break;
    //     default: UT_ASSERT_MSG(0, "unhandled attrib storage"); break;
    //     }
    // }
    
    void attribDuplicate(GA_Attribute& attrib, const GA_Attribute& attribRef)
    {
        // GA_AttributeOwner owner = attrib.getOwner();
        // const GA_AIFStringTuple* a = attrib.getAIFStringTuple();
        // const GA_AIFSharedStringTuple* b = attrib.getAIFSharedStringTuple();
        UT_ASSERT_MSG(attrib.getOwner() == attribRef.getOwner(), "two attrib owner must be same");
        UT_ASSERT_P(attrib.getAIFTuple() || attrib.getAIFStringTuple() || attrib.getAIFSharedStringTuple());
        UT_ASSERT_P(attribRef.getAIFTuple() || attribRef.getAIFStringTuple() || attribRef.getAIFSharedStringTuple());

        const GA_AIFTuple* aifTuple = attribRef.getAIFTuple();

        GA_Storage storageRef;
        if (aifTuple)
            storageRef = aifTuple->getStorage(&attribRef);
        else if (attribRef.getAIFStringTuple())
            storageRef = GA_STORE_STRING;
        else
            UT_ASSERT_MSG(0, "impossible");
            

        aifTuple = attrib.getAIFTuple();
        if (aifTuple)
        {
            switch (aifTuple->getStorage(&attrib))
            {
            case GA_STORE_INT16:
                switch (storageRef)
                {
                    //case GA_STORE_INT16:  attribDuplicate<int16, int16>                                break;
                    case GA_STORE_INT32:  attribDuplicate<int16, int32>           (attrib, attribRef); break;
                    case GA_STORE_INT64:  attribDuplicate<int16, int64>           (attrib, attribRef); break;
                    case GA_STORE_REAL16: attribDuplicate<int16, fpreal16>        (attrib, attribRef); break;
                    case GA_STORE_REAL32: attribDuplicate<int16, fpreal32>        (attrib, attribRef); break;
                    case GA_STORE_REAL64: attribDuplicate<int16, fpreal64>        (attrib, attribRef); break;
                    case GA_STORE_STRING: attribDuplicate<int16, UT_StringHolder> (attrib, attribRef); break;
                    default: UT_ASSERT_MSG(0, "unhandled attrib storage"); break;
                }
                break;
            case GA_STORE_INT32:
                switch (storageRef)
                {
                    case GA_STORE_INT16:  attribDuplicate<int32, int16>           (attrib, attribRef); break;
                    //case GA_STORE_INT32:  attribDuplicate<int32, int32>           (attrib, attribRef); break;
                    case GA_STORE_INT64:  attribDuplicate<int16, int64>           (attrib, attribRef); break;
                    case GA_STORE_REAL16: attribDuplicate<int16, fpreal16>        (attrib, attribRef); break;
                    case GA_STORE_REAL32: attribDuplicate<int16, fpreal32>        (attrib, attribRef); break;
                    case GA_STORE_REAL64: attribDuplicate<int16, fpreal64>        (attrib, attribRef); break;
                    case GA_STORE_STRING: attribDuplicate<int16, UT_StringHolder> (attrib, attribRef); break;
                    default: UT_ASSERT_MSG(0, "unhandled attrib storage"); break;
                }
                break;
            case GA_STORE_INT64:
                switch (storageRef)
                {
                    case GA_STORE_INT16:  attribDuplicate<int64, int16>           (attrib, attribRef); break;
                    case GA_STORE_INT32:  attribDuplicate<int64, int32>           (attrib, attribRef); break;
                    //case GA_STORE_INT64:  attribDuplicate<int64, int64>           (attrib, attribRef); break;
                    case GA_STORE_REAL16: attribDuplicate<int64, fpreal16>        (attrib, attribRef); break;
                    case GA_STORE_REAL32: attribDuplicate<int64, fpreal32>        (attrib, attribRef); break;
                    case GA_STORE_REAL64: attribDuplicate<int64, fpreal64>        (attrib, attribRef); break;
                    case GA_STORE_STRING: attribDuplicate<int64, UT_StringHolder> (attrib, attribRef); break;
                    default: UT_ASSERT_MSG(0, "unhandled attrib storage"); break;
                }
                break;
            case GA_STORE_REAL16:
                switch (storageRef)
                {
                    case GA_STORE_INT16:  attribDuplicate<fpreal16, int16>           (attrib, attribRef); break;
                    case GA_STORE_INT32:  attribDuplicate<fpreal16, int32>           (attrib, attribRef); break;
                    case GA_STORE_INT64:  attribDuplicate<fpreal16, int64>           (attrib, attribRef); break;
                    //case GA_STORE_REAL16: attribDuplicate<fpreal16, fpreal16>        (attrib, attribRef); break;
                    case GA_STORE_REAL32: attribDuplicate<fpreal16, fpreal32>        (attrib, attribRef); break;
                    case GA_STORE_REAL64: attribDuplicate<fpreal16, fpreal64>        (attrib, attribRef); break;
                    case GA_STORE_STRING: attribDuplicate<fpreal16, UT_StringHolder> (attrib, attribRef); break;
                    default: UT_ASSERT_MSG(0, "unhandled attrib storage"); break;
                }
                break;
            case GA_STORE_REAL32:
                switch (storageRef)
                {
                    case GA_STORE_INT16:  attribDuplicate<fpreal32, int16>           (attrib, attribRef); break;
                    case GA_STORE_INT32:  attribDuplicate<fpreal32, int32>           (attrib, attribRef); break;
                    case GA_STORE_INT64:  attribDuplicate<fpreal32, int64>           (attrib, attribRef); break;
                    case GA_STORE_REAL16: attribDuplicate<fpreal32, fpreal16>        (attrib, attribRef); break;
                    //case GA_STORE_REAL32: attribDuplicate<fpreal32, fpreal32>        (attrib, attribRef); break;
                    case GA_STORE_REAL64: attribDuplicate<fpreal32, fpreal64>        (attrib, attribRef); break;
                    case GA_STORE_STRING: attribDuplicate<fpreal32, UT_StringHolder> (attrib, attribRef); break;
                    default: UT_ASSERT_MSG(0, "unhandled attrib storage"); break;
                }
                break;
            case GA_STORE_REAL64:
                switch (storageRef)
                {
                    case GA_STORE_INT16:  attribDuplicate<fpreal64, int16>           (attrib, attribRef); break;
                    case GA_STORE_INT32:  attribDuplicate<fpreal64, int32>           (attrib, attribRef); break;
                    case GA_STORE_INT64:  attribDuplicate<fpreal64, int64>           (attrib, attribRef); break;
                    case GA_STORE_REAL16: attribDuplicate<fpreal64, fpreal16>        (attrib, attribRef); break;
                    case GA_STORE_REAL32: attribDuplicate<fpreal64, fpreal32>        (attrib, attribRef); break;
                    //case GA_STORE_REAL64: attribDuplicate<fpreal64, fpreal64>        (attrib, attribRef); break;
                    case GA_STORE_STRING: attribDuplicate<fpreal64, UT_StringHolder> (attrib, attribRef); break;
                    default: UT_ASSERT_MSG(0, "unhandled attrib storage"); break;
                }
                break;
            case GA_STORE_STRING:
                switch (storageRef)
                {
    #if 0
                    case GA_STORE_INT16:  attribDuplicateString<int16>               (attrib, attribRef); break;
                    case GA_STORE_INT32:  attribDuplicateString<int32>               (attrib, attribRef); break;
                    case GA_STORE_INT64:  attribDuplicateString<int64>               (attrib, attribRef); break;
                    case GA_STORE_REAL16: attribDuplicateString<fpreal16>            (attrib, attribRef); break;
                    case GA_STORE_REAL32: attribDuplicateString<fpreal32>            (attrib, attribRef); break;
                    case GA_STORE_REAL64: attribDuplicateString<fpreal64>            (attrib, attribRef); break;
    #else
                    case GA_STORE_INT16:  attribDuplicate<UT_StringHolder, int16>    (attrib, attribRef); break;
                    case GA_STORE_INT32:  attribDuplicate<UT_StringHolder, int32>    (attrib, attribRef); break;
                    case GA_STORE_INT64:  attribDuplicate<UT_StringHolder, int64>    (attrib, attribRef); break;
                    case GA_STORE_REAL16: attribDuplicate<UT_StringHolder, fpreal16> (attrib, attribRef); break;
                    case GA_STORE_REAL32: attribDuplicate<UT_StringHolder, fpreal32> (attrib, attribRef); break;
                    case GA_STORE_REAL64: attribDuplicate<UT_StringHolder, fpreal64> (attrib, attribRef); break;
    #endif
                    //case GA_STORE_STRING: attribDuplicate<UT_StringHolder, UT_StringHolder> (attrib, attribRef); break;
                    default: UT_ASSERT_MSG(0, "unhandled attrib storage"); break;
                }
                break;
            default: UT_ASSERT_MSG(0, "unhandled attrib storage"); break;
            }
        }
        else
        {
            //attrib.getAIFStringTuple()
            //attrib.getAIFSharedStringTuple();
            switch (storageRef)
            {
#if 0
            case GA_STORE_INT16:  attribDuplicateString<int16>               (attrib, attribRef); break;
            case GA_STORE_INT32:  attribDuplicateString<int32>               (attrib, attribRef); break;
            case GA_STORE_INT64:  attribDuplicateString<int64>               (attrib, attribRef); break;
            case GA_STORE_REAL16: attribDuplicateString<fpreal16>            (attrib, attribRef); break;
            case GA_STORE_REAL32: attribDuplicateString<fpreal32>            (attrib, attribRef); break;
            case GA_STORE_REAL64: attribDuplicateString<fpreal64>            (attrib, attribRef); break;
#else
            case GA_STORE_INT16:  attribDuplicate<UT_StringHolder, int16>    (attrib, attribRef); break;
            case GA_STORE_INT32:  attribDuplicate<UT_StringHolder, int32>    (attrib, attribRef); break;
            case GA_STORE_INT64:  attribDuplicate<UT_StringHolder, int64>    (attrib, attribRef); break;
            case GA_STORE_REAL16: attribDuplicate<UT_StringHolder, fpreal16> (attrib, attribRef); break;
            case GA_STORE_REAL32: attribDuplicate<UT_StringHolder, fpreal32> (attrib, attribRef); break;
            case GA_STORE_REAL64: attribDuplicate<UT_StringHolder, fpreal64> (attrib, attribRef); break;
#endif
            //case GA_STORE_STRING: attribDuplicate<UT_StringHolder, UT_StringHolder> (attrib, attribRef); break;
            default: UT_ASSERT_MSG(0, "unhandled attrib storage"); break;
            }
        }
    }
    
    // void
    // attribDuplicate(
    //     GA_Attribute& attrib,
    //     const GA_Attribute& attribRef
    // )
    // {
    //     UT_ASSERT_MSG(attrib.getOwner() == attribRef.getOwner(), "two attrib owner must be same");
    //     
    //     UT_ASSERT_P(attrib.getAIFTuple());
    //     switch (attrib.getAIFTuple()->getStorage(&attrib))
    //     {
    //     case GA_STORE_INT16:  attribDuplicate<int16>          (attrib, attribRef); break;
    //     case GA_STORE_INT32:  attribDuplicate<int32>          (attrib, attribRef); break;
    //     case GA_STORE_INT64:  attribDuplicate<int64>          (attrib, attribRef); break;
    //     case GA_STORE_REAL16: attribDuplicate<fpreal16>       (attrib, attribRef); break;
    //     case GA_STORE_REAL32: attribDuplicate<fpreal32>       (attrib, attribRef); break;
    //     case GA_STORE_REAL64: attribDuplicate<fpreal64>       (attrib, attribRef); break;
    //     case GA_STORE_STRING: attribDuplicate<UT_StringHolder>(attrib, attribRef);  break;
    //     default: UT_ASSERT_MSG(0, "unhandled attrib storage"); break;
    //     }
    // }

    template<typename T>
    void setAttribValueTo1(GA_Attribute& attrib, const GA_SplittableRange& geoSplittableRange)
    {
        UTparallelFor(geoSplittableRange, [&attrib](const GA_SplittableRange& r)
        {
            GA_PageHandleScalar<GA_Offset>::RWType attrib_ph(&attrib);
            GA_Offset start, end;
            for (GA_PageIterator pit = r.beginPages(); !pit.atEnd(); ++pit)
            {
                for (GA_Iterator it(pit.begin()); it.blockAdvance(start, end); )
                {
                    attrib_ph.setPage(start);
                    for (GA_Offset elemoff = start; elemoff < end; ++elemoff)
                    {
                        attrib_ph.value(elemoff) = T(1);
                    }
                }
            }
        }, subscribeRatio, minGrainSize);
    }

    template<>
    void setAttribValueTo1<UT_StringHolder>(GA_Attribute& attrib, const GA_SplittableRange& geoSplittableRange)
    {
        const GA_RWHandleS attrib_h(&attrib);
        UTparallelFor(geoSplittableRange, [&attrib_h](const GA_SplittableRange& r)
        {
            GA_Offset start, end;
            for (GA_Iterator it(r); it.blockAdvance(start, end); )
            {
                for (GA_Offset elemoff = start; elemoff < end; ++elemoff)
                {
                    attrib_h.set(elemoff, "1");
                }
            }
        }, subscribeRatio, minGrainSize);
    }






public:
    GA_StorageClass newStorageClass = GA_STORECLASS_INVALID;
    GA_Precision newPrecision = GA_PRECISION_INVALID;
    bool delOrigin = true;

    ufe::SplittableString newAttribNames;
    ufe::SplittableString newGroupNames;

    const char* prefix = "";
    const char* sufix  = "";

private:

    bool renameAttrib = false;
    bool renameGroup = false;
    // ::std::string newAttribNames;
    // ::std::string newGroupNames;
    //UT_String newAttribNames;
    //UT_String newGroupNames;

    exint subscribeRatio = 64;
    exint minGrainSize   = 1024;


#undef GFE_TEMP_ATTRIBCAST_ATTRIBNAME
    
}; // End of class AttribCast
_GFEL_END
#endif
