
#pragma once

#ifndef __GFE_AttributePromote_h__
#define __GFE_AttributePromote_h__

#include "GFE/GFE_AttributePromote.h"

#include "GFE/GFE_GeoFilter.h"



#include "UFE/UFE_SplittableString.h"


class GFE_AttribPromote : public GFE_AttribCreateFilterWithRef0 {

public:
    using GFE_AttribCreateFilterWithRef0::GFE_AttribCreateFilterWithRef0;

    void setComputeParm(
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
        if (geoRef0 ? getRef0AttribArray().isEmpty() && getRef0GroupArray().isEmpty() : getInAttribArray().isEmpty() && getInGroupArray().isEmpty())
            return true;

        if (groupParser.isEmpty())
            return true;
        
        geoTmp = geoRef0 ? geoRef0 : geo;

        const size_t sizeInAttrib  = geoRef0 ? getRef0AttribArray().size() : getInAttribArray().size();
        const size_t sizeOutAttrib = getOutAttribArray().size();
        if (sizeOutAttrib > 0)
        {
            const size_t attribSize = std::min(sizeInAttrib, sizeOutAttrib);
            for (size_t i = 0; i < attribSize; ++i)
            {
                srcAttrib = geoRef0 ? getRef0AttribArray()[i] : getInAttribArray()[i];
                dstAttrib = getOutAttribArray()[i];
                
                if (GFE_Attribute::isSameType(*srcAttrib, *dstAttrib))
                    attribPromote();
            }
        }
        else
        {
            for (size_t i = 0; i < sizeInAttrib; ++i)
            {
                srcAttrib = geoRef0 ? getRef0AttribArray()[i] : getInAttribArray()[i];
                if (srcAttrib->getOwner() == dstAttribClass)
                    continue;
            
                const UT_StringHolder& newName = newAttribNames.getValidAttribName(*srcAttrib);
                const bool detached = GFE_Type::isInvalid(newName);
                
                dstAttrib = getOutAttribArray().clone(detached, *srcAttrib, dstAttribClass, newName);
    
                attribPromote();
            }
        }
        
        
        
        const size_t sizeInGroup  = geoRef0 ? getRef0GroupArray().size() : getInGroupArray().size();
        const size_t sizeOutGroup = getOutGroupArray().size();
        if (sizeOutGroup > 0)
        {
            const size_t groupSize = std::min(sizeInGroup, sizeOutGroup);
            for (size_t i = 0; i < groupSize; ++i)
            {
                srcGroup = geoRef0 ? getRef0GroupArray()[i] : getInGroupArray()[i];
                dstGroup = getOutGroupArray()[i];
                
                groupPromote();
            }
        }
        else
        {
            for (size_t i = 0; i < sizeInGroup; ++i)
            {
                srcGroup = geoRef0 ? getRef0GroupArray()[i] : getInGroupArray()[i];
                if (srcGroup->classType() == dstGroupClass)
                    continue;
            
                const UT_StringHolder& newName = newGroupNames.getValidAttribName(*srcGroup);
                const bool detached = GFE_Type::isInvalid(newName);
                
                dstGroup = getOutGroupArray().findOrCreate(detached, dstGroupClass, newName);
    
                groupPromote();
            }
        }
        
        return true;
    }


    
    SYS_FORCE_INLINE void groupPromote()
    {
        if (dstGroup->classType() == GA_GROUP_VERTEX && srcGroup->classType() == GA_GROUP_EDGE)
        {
            GFE_GroupUnion::groupUnion(reinterpret_cast<GA_VertexGroup&>(*dstGroup),
                                       reinterpret_cast<const GA_EdgeGroup&>(*srcGroup),
                                       vertexEdgeConnectElemType, reverseGroup);
        }
        else
            GFE_GroupUnion::groupUnion(*dstGroup, *srcGroup, reverseGroup, subscribeRatio, minGrainSize);
    }
    
    

    void attribPromote()
    {
        const GFE_AttribStorage attribStorage = GFE_Type::getAttribStorage(dstAttrib);
        auto storageVariant = GFE_Variant::getAttribStorageVariantIFVM(attribStorage);
        auto dstOwnerVariant = GFE_Variant::getAttribOwnerVariant(dstAttrib);
        auto srcOwnerVariant = GFE_Variant::getAttribOwnerVariant(srcAttrib);
        auto hasRef0 = GFE_Variant::getBoolVariant(geoRef0);
        
        
        if (GFE_Variant::isAttribStorageIFVM(attribStorage))
        {
            std::visit([&] (auto storageVariant, auto dstOwnerVariant, auto srcOwnerVariant, auto hasRef0)
            {
                using type = typename GFE_Variant::getAttribStorage_t<storageVariant>;
                attribPromote<type, dstOwnerVariant, srcOwnerVariant, hasRef0>();
            }, storageVariant, dstOwnerVariant, srcOwnerVariant, hasRef0);
        }
        else
        {
            std::visit([&] (auto dstOwnerVariant, auto srcOwnerVariant, auto hasRef0)
            {
                attribPromote<std::nullopt_t, dstOwnerVariant, srcOwnerVariant, hasRef0>();
            }, dstOwnerVariant, srcOwnerVariant, hasRef0);
        }
    }

    template<typename _Ty, GA_AttributeOwner dstOwner, GA_AttributeOwner srcOwner, bool hasRef0>
    void attribPromote()
    {
        UTparallelFor(groupParser.getSplittableRange(dstAttrib), [this](const GA_SplittableRange& r)
        {
            if constexpr (GFE_Type::isTuple<_Ty>)
            {
                const GA_ROHandleT<_Ty> srcAttrib_h(srcAttrib);
                GFE_RWPageHandleT<_Ty> attrib_ph(dstAttrib);
                GA_Offset start, end;
                for (GA_PageIterator pit = r.beginPages(); !pit.atEnd(); ++pit)
                {
                    for (GA_Iterator it(pit.begin()); it.blockAdvance(start, end); )
                    {
                        attrib_ph.setPage(start);
                        for (GA_Offset elemoff = start; elemoff < end; ++elemoff)
                        {
                            const GA_Offset srcOff = geo->offsetPromote<dstOwner, srcOwner>(elemoff);
                            
                            if constexpr (hasRef0)
                            {
                                if (GFE_Type::isValidOffset(srcAttrib->getIndexMap(), srcOff))
                                    attrib_ph.value(elemoff) = srcAttrib_h.get(srcOff);
                            }
                            else
                            {
                                if (GFE_Type::isValidOffset(srcOff))
                                    attrib_ph.value(elemoff) = srcAttrib_h.get(srcOff);
                            }
                        }
                    }
                }
            }
            else
            {
                const GA_Attribute& srcAttribRef = *srcAttrib;
                GA_Offset start, end;
                for (GA_PageIterator pit = r.beginPages(); !pit.atEnd(); ++pit)
                {
                    for (GA_Iterator it(pit.begin()); it.blockAdvance(start, end); )
                    {
                        for (GA_Offset elemoff = start; elemoff < end; ++elemoff)
                        {
                            const GA_Offset srcOff = geo->offsetPromote<dstOwner, srcOwner>(elemoff);
                            if constexpr (hasRef0)
                            {
                                //if (GFE_Type::isValidOffset(srcAttrib->getIndexMap(), srcOff))
                                    dstAttrib->copy(elemoff, srcAttribRef, srcOff);
                            }
                            else
                            {
                                //if (GFE_Type::isValidOffset(srcOff))
                                    dstAttrib->copy(elemoff, srcAttribRef, srcOff);
                            }
                        }
                    }
                }
            }
        }, subscribeRatio, minGrainSize);
    }


public:
    GA_AttributeOwner dstAttribClass = GA_ATTRIB_INVALID;
    GA_GroupType dstGroupClass = GA_GROUP_INVALID;
        
    GA_AttributeOwner vertexEdgeConnectElemType = GA_ATTRIB_OWNER_N;
    bool reverseGroup = false;
    
    UFE_SplittableString newAttribNames;
    UFE_SplittableString newGroupNames;

private:
    //GA_AttributeOwner dstAttribClassMid = GA_ATTRIB_INVALID;
    //GA_GroupType dstGroupClassMid = GA_GROUP_INVALID;
        
    const GFE_Detail* geoTmp;
    const GA_Attribute* srcAttrib;
    const GA_Group* srcGroup;
        
    GA_Attribute* dstAttrib;
    GA_Group* dstGroup;
    //GA_AttributeOwner srcOwner;

    exint subscribeRatio = 64;
    exint minGrainSize   = 1024;

}; // End of class GFE_AttribPromote








#endif
