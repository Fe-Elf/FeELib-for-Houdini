
#pragma once

#ifndef __GFE_AttributeValueCount_h__
#define __GFE_AttributeValueCount_h__

#include "GFE/GFE_AttributeValueCount.h"

#include "GFE/GFE_GeoFilter.h"



class GFE_AttribValCount : public GFE_AttribFilter {


public:
    using GFE_AttribFilter::GFE_AttribFilter;


    void
        setComputeParm(
            const exint subscribeRatio = 64,
            const exint minGrainSize = 64
        )
    {
        setHasComputed();
        this->subscribeRatio = subscribeRatio;
        this->minGrainSize   = minGrainSize;
    }
    
    SYS_FORCE_INLINE GA_Attribute* findOrCreateTuple(
        const bool detached = false,
        const GA_AttributeOwner owner = GA_ATTRIB_POINT,
        const GA_StorageClass storageClass = GA_STORECLASS_FLOAT,
        const GA_Storage storage = GA_STORE_INVALID,
        const UT_StringRef& attribName = ""
    )
    { return getOutAttribArray().findOrCreateTuple(detached, owner,
            storageClass, storage, attribName, 1, GA_Defaults(GFE_INVALID_OFFSET)); }



    
    SYS_FORCE_INLINE size_t getAttribValCount() const
    { return attribValCount; }
    
private:

    
    virtual bool
        computeCore() override
    {
        if (getOutAttribArray().isEmpty())
            return false;

        if (groupParser.isEmpty())
            return true;
        
        attribValCount = 0;

        const attribValI = 0

        
        attribPtr = getOutAttribArray()[0];
        // const GA_Storage storage = attribPtr->getAIFTuple()->getStorage(attribPtr);
        switch (attribPtr->getAIFTuple()->getStorage(attribPtr))
        {
        case GA_STORE_INT16:  computeAttribValCount<int16>();           break;
        case GA_STORE_INT32:  computeAttribValCount<int32>();           break;
        case GA_STORE_INT64:  computeAttribValCount<int64>();           break;
        case GA_STORE_REAL16: computeAttribValCount<fpreal16>();        break;
        case GA_STORE_REAL32: computeAttribValCount<fpreal32>();        break;
        case GA_STORE_REAL64: computeAttribValCount<fpreal64>();        break;
        //case GA_STORE_STRING: computeAttribValCount<UT_StringHolder>(); break;
        default: break;
        }

        return true;
    }

    template<typename T>
    size_t computeAttribValCount(const T& attribVal)
    {
        const GA_SplittableRange geoSplittableRange(GA_Range(attribPtr->getIndexMap()));

        AttribValCount<T> body(attribPtr, attribVal);
        UTparallelReduce(geoSplittableRange, body, subscribeRatio, minGrainSize);
        return body.getSum();
    }


private:

    

    template<typename T>
    class AttribValCount {
    public:
        AttribValCount(const GA_Attribute* const attrib, const T& attribVal)
            : myAttrib(attrib)
            , myAttribVal(attribVal)
            , mySum(0)
        {}
        AttribValCount(AttribValCount& src, UT_Split)
            : myAttrib(src.myAttrib)
            , myAttribVal(src.myAttribVal)
            , mySum(0)
        {}
        void operator()(const GA_SplittableRange& r)
        {
            GA_PageHandleT<T, T, true, false, const GA_Attribute, const GA_ATINumeric, const GA_Detail> attrib_ph(myAttrib);
            GA_Offset start, end;
            for (GA_PageIterator pit = r.beginPages(); !pit.atEnd(); ++pit)
            {
                for (GA_Iterator it(pit.begin()); it.blockAdvance(start, end); )
                {
                    attrib_ph.setPage(start);
                    for (GA_Offset elemoff = start; elemoff < end; ++elemoff)
                    {
                        mySum += attrib_ph.value(elemoff) == myAttribVal;
                    }
                }
            }
        }
        sysofrceinline void join(const AttribValCount& other)
        { mySum += other.mySum; }
        sysofrceinline size_t getSum() const
        { return mySum; }
    private:
        size_t mySum;
        const GA_Attribute* const myAttrib;
        const T& myAttribVal;
    }; // End of Class AttribValCount





public:
    

private:
    
    size_t attribValCount;
    
    GA_Attribute* attrib;
    exint subscribeRatio = 64;
    exint minGrainSize = 1024;


}; // End of class GFE_AttribValCount





#endif
