
#pragma once

#ifndef __GFE_OffsetAttributeToIndex_h__
#define __GFE_OffsetAttributeToIndex_h__

#include <GFE/GFE_OffsetAttributeToIndex.h>

#include <GFE/GeoFilter.h>

#if 0
    GFE_OffsetAttribToIndex offsetAttribToIndex(geo, cookparms);
    offsetAttribToIndex.offsetToIndex = true;
    offsetAttribToIndex.getOutAttribArray().appends(GA_ATTRIB_PRIMITIVE, );
    offsetAttribToIndex.compute();
#endif
    
_GFEL_BEGIN
class OffsetAttribToIndex : public AttribFilter {

public:
    using AttribFilter::AttribFilter;

    void setComputeParm(
        const bool offsetToIndex = false,
        const exint subscribeRatio = 64,
        const exint minGrainSize   = 1024
    )
    {
        setHasComputed();
        this->offsetToIndex = offsetToIndex;
        this->subscribeRatio = subscribeRatio;
        this->minGrainSize   = minGrainSize;
    }

private:

    virtual bool
        computeCore() override
    {
        const size_t sizeAttrib = getOutAttribArray().size();
        for (size_t i = 0; i < sizeAttrib; i++)
        {
            attrib = getOutAttribArray()[i];

            bool isArray;
            const GA_AIFTuple* const aifTuple = attrib->getAIFTuple();
            if (aifTuple)
            {
                isArray = false;
            }
            else
            {
                const GA_AIFNumericArray* const aifNumArray = attrib->getAIFNumericArray();
                if (!aifNumArray)
                    continue;
                isArray = true;
            }
            
            auto offsetToIndexVariant = gfe::getBoolVariant(offsetToIndex);
            auto isArrayVariant = gfe::getBoolVariant(isArray);
            std::visit([&] (auto offsetToIndexVariant, auto isArrayVariant)
            {
                offsetAttribToIndex<offsetToIndexVariant, isArrayVariant>();
            }, offsetToIndexVariant, isArrayVariant);
            //const char* name = attrib->getName().c_str();
            // switch (attrib->getType().getTypeId())
            // {
            // case 0:                              break; // unknown
            // case 1:                              break; // topo
            // case 2:  offsetTupleAttribToIndex(); break; // int float vector(tuple)
            // case 3:                              break; // string
            // case 4:                              break; // unknown
            // case 5:                              break; // unknown
            // case 6:  offsetArrayAttribToIndex(); break; // int array
            // case 7:                              break; // unknown
            // case 8:  offsetArrayAttribToIndex(); break; // float array
            // case 9:                              break; // unknown
            // case 10:                             break; // string array
            // case 11:                             break; // dict
            // case 12:                             break; // dict array
            // default:                             break;
            // }
        }

        return true;
    }


    template<bool offsetToIndex, bool isArray>
    void offsetAttribToIndex()
    {
        const GA_IndexMap& indexMap = attrib->getIndexMap();
        
        GA_Offset offsetSize;
        if constexpr (offsetToIndex)
            offsetSize = indexMap.lastOffset();
        else
            offsetSize = indexMap.indexSize();
        
        if constexpr (isArray)
        {
            const GA_RWHandleT<UT_ValArray<GA_Offset>> attrib_h(attrib);
            UTparallelFor(groupParser.getSplittableRange(attrib), [&indexMap, offsetSize, &attrib_h](const GA_SplittableRange& r)
            {
                UT_ValArray<GA_Offset> attribVal;
                GA_Offset start, end;
                for (GA_Iterator it(r); it.blockAdvance(start, end); )
                {
                    for (GA_Offset elemoff = start; elemoff < end; ++elemoff)
                    {
                        attrib_h.get(elemoff, attribVal);
                        const size_t arrayLen = attribVal.size();
                        for (size_t i = 0; i < arrayLen; ++i)
                        {
                            if (attribVal[i] < 0 || attribVal[i] >= offsetSize)
                                continue;
                            
                            if constexpr (offsetToIndex)
                                attribVal[i] = indexMap.indexFromOffset(attribVal[i]);
                            else
                                attribVal[i] = indexMap.offsetFromIndex(attribVal[i]);
                        }
                        attrib_h.set(elemoff, attribVal);
                    }
                }
            }, subscribeRatio, minGrainSize);
        }
        else
        {
            UTparallelFor(groupParser.getSplittableRange(attrib), [&indexMap, offsetSize, this](const GA_SplittableRange& r)
            {
                GA_Offset attribVal;
                GA_PageHandleScalar<GA_Offset>::RWType attrib_ph(attrib);
                GA_Offset start, end;
                for (GA_PageIterator pit = r.beginPages(); !pit.atEnd(); ++pit)
                {
                    for (GA_Iterator it(pit.begin()); it.blockAdvance(start, end); )
                    {
                        attrib_ph.setPage(start);
                        for (GA_Offset elemoff = start; elemoff < end; ++elemoff)
                        {
                            attribVal = attrib_ph.value(elemoff);
                            if (attribVal < 0 || attribVal >= offsetSize)
                                continue;
                        
                            if constexpr (offsetToIndex)
                                attrib_ph.value(elemoff) = indexMap.indexFromOffset(attribVal);
                            else
                                attrib_ph.value(elemoff) = indexMap.offsetFromIndex(attribVal);
                        }
                    }
                }
            }, subscribeRatio, minGrainSize);
        }
    }

    
public:
    bool offsetToIndex = true;
    

private:
    GA_Attribute* attrib;
    
    exint subscribeRatio = 64;
    exint minGrainSize   = 1024;

}; // End of class OffsetAttribToIndex
_GFEL_END
#endif
