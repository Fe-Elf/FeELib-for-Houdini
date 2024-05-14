
#pragma once

#ifndef __GFE_DirectionCross_h__
#define __GFE_DirectionCross_h__

#include "GFE/GFE_DirectionCross.h"

#include "GFE/GFE_GeoFilter.h"



//#include "UFE/UFE_SplittableString.h"

class GFE_DirCross : public GFE_AttribFilter {


public:
    using GFE_AttribFilter::GFE_AttribFilter;


    void
        setComputeParm(
            const UT_Vector3T<fpreal>& inputUp = UT_Vector3T<fpreal>(0,1,0),
            const bool normalize   = false,
            const bool keepLength  = false,
            const bool reverseDir  = false,
            const bool delUpAttrib = false,
            const exint subscribeRatio = 64,
            const exint minGrainSize   = 64
        )
    {
        setHasComputed();
        this->inputUp     = inputUp;
        this->normalize   = normalize;
        this->keepLength  = keepLength;
        this->reverseDir  = reverseDir;
        this->delUpAttrib = delUpAttrib;
        this->subscribeRatio = subscribeRatio;
        this->minGrainSize   = minGrainSize;
    }

    SYS_FORCE_INLINE void setUpAttrib(const GA_AttributeOwner owner, const UT_StringRef& name)
    { setUpAttrib(geo->findAttribute(owner, name)); }
    
    SYS_FORCE_INLINE void setUpAttrib(const GA_Attribute* const attrib)
    {
        attribUp = attrib;
        attribUp_nonConst = nullptr;
    }
    
    SYS_FORCE_INLINE void setUpAttrib(GA_Attribute* const attrib)
    {
        attribUp = attrib;
        attribUp_nonConst = attrib;
    }

    
private:

    virtual bool
        computeCore() override
    {
        if (getOutAttribArray().isEmpty())
            return false;

        if (groupParser.isEmpty())
            return true;

        ownerUp = attribUp ? attribUp->getOwner() : GA_ATTRIB_INVALID;
        bool flag = false;
        const size_t size = getOutAttribArray().size();
        for (size_t i = size-1; ; --i)
        {
            if (flag)
                break;
            attrib = getOutAttribArray()[i];
            // const GA_Storage storage = attribPtr->getAIFTuple()->getStorage(attribPtr);
            const GA_AIFTuple* const aifTuple = attrib->getAIFTuple();
            if (!aifTuple || aifTuple->getTupleSize(attrib) != 3)
            {
                getOutAttribArray().erase(i);
                if (i <= 0)
                    break;
                continue;
            }

            switch (aifTuple->getTupleSize(attrib))
            {
            case 3: break;
            default:
                UT_ASSERT_MSG(0, "Unhandled Tuple Size"); 
            case 1:
            case 2:
            case 4:
                getOutAttribArray().erase(i);
                if (i <= 0)
                    flag = true;
                continue;
            }
            
            owner = attrib->getOwner();
            const GA_Storage storage = aifTuple->getStorage(attrib);

            //const UT_StringHolder& newName = newAttribNames.getIsValid() ? newAttribNames.getNext<UT_StringHolder>() : attrib->getName();
            //const bool detached = !GFE_Type::isPublicAttribName(newName);
            
            if (!attribUp || ownerUp == GA_ATTRIB_GLOBAL)
            {
                switch (storage)
                {
                //case GA_STORE_INT16:  dirCrossDefault<UT_Vector3T<int16   >>(); break;
                case GA_STORE_INT32:  dirCrossDefault<UT_Vector3T<int32   >>(); break;
                case GA_STORE_INT64:  dirCrossDefault<UT_Vector3T<int64   >>(); break;
                //case GA_STORE_REAL16: dirCrossDefault<UT_Vector3T<fpreal16>>(); break;
                case GA_STORE_REAL32: dirCrossDefault<UT_Vector3T<fpreal32>>(); break;
                case GA_STORE_REAL64: dirCrossDefault<UT_Vector3T<fpreal64>>(); break;
                default: break;
                }
            }
            else if (getOutAttribArray()[i]->getOwner() == ownerUp)
            {
                switch (storage)
                {
                //case GA_STORE_INT16:  dirCrossSameOwner<UT_Vector3T<int16   >>(); break;
                case GA_STORE_INT32:  dirCrossSameOwner<UT_Vector3T<int32   >>(); break;
                case GA_STORE_INT64:  dirCrossSameOwner<UT_Vector3T<int64   >>(); break;
                //case GA_STORE_REAL16: dirCrossSameOwner<UT_Vector3T<fpreal16>>(); break;
                case GA_STORE_REAL32: dirCrossSameOwner<UT_Vector3T<fpreal32>>(); break;
                case GA_STORE_REAL64: dirCrossSameOwner<UT_Vector3T<fpreal64>>(); break;
                default: break;
                }
            }
            else
            {
                switch (storage)
                {
                //case GA_STORE_INT16:  dirCross<UT_Vector3T<int16   >>(); break;
                case GA_STORE_INT32:  dirCross<UT_Vector3T<int32   >>(); break;
                case GA_STORE_INT64:  dirCross<UT_Vector3T<int64   >>(); break;
                //case GA_STORE_REAL16: dirCross<UT_Vector3T<fpreal16>>(); break;
                case GA_STORE_REAL32: dirCross<UT_Vector3T<fpreal32>>(); break;
                case GA_STORE_REAL64: dirCross<UT_Vector3T<fpreal64>>(); break;
                default: break;
                }
            }
            
            if (i <= 0)
                break;
        }

        if (delUpAttrib && attribUp_nonConst)
            geo->destroyAttrib(attribUp_nonConst);

        return true;
    }

    
    template<typename VECTOR_T>
    void dirCrossDefault()
    {
        UT_ASSERT_P(attrib);
        VECTOR_T defaultUp;
        if (attribUp)
        {
            UT_ASSERT_P(ownerUp == GA_ATTRIB_GLOBAL);
            attribUp->getAIFTuple()->get(attribUp, 0, defaultUp[0], 0);
            attribUp->getAIFTuple()->get(attribUp, 0, defaultUp[1], 1);
            attribUp->getAIFTuple()->get(attribUp, 0, defaultUp[2], 2);
        }
        else
            defaultUp = inputUp;
        
        UTparallelFor(groupParser.getSplittableRange(owner), [this, &defaultUp](const GA_SplittableRange& r)
        {
            GA_PageHandleT<VECTOR_T, typename VECTOR_T::value_type, true, true, GA_Attribute, GA_ATINumeric, GA_Detail> attrib_ph(attrib);
            for (GA_PageIterator pit = r.beginPages(); !pit.atEnd(); ++pit)
            {
                GA_Offset start, end;
                for (GA_Iterator it(pit.begin()); it.blockAdvance(start, end); )
                {
                    attrib_ph.setPage(start);
                    for (GA_Offset elemoff = start; elemoff < end; ++elemoff)
                    {
                        dirCrossBase(attrib_ph.value(elemoff), defaultUp);
                    }
                }
            }
        }, subscribeRatio, minGrainSize);
    }
    
    template<typename VECTOR_T, GA_AttributeOwner FROM, GA_AttributeOwner TO>
    void dirCross()
    {
        UT_ASSERT_P(attrib);
        UT_ASSERT_P(attribUp);
        const GA_ROHandleT<VECTOR_T> attribUp_h(attribUp);
        UTparallelFor(groupParser.getSplittableRange(owner), [this, &attribUp_h](const GA_SplittableRange& r)
        {
            GA_PageHandleT<VECTOR_T, typename VECTOR_T::value_type, true, true, GA_Attribute, GA_ATINumeric, GA_Detail> attrib_ph(attrib);
            for (GA_PageIterator pit = r.beginPages(); !pit.atEnd(); ++pit)
            {
                GA_Offset start, end;
                for (GA_Iterator it(pit.begin()); it.blockAdvance(start, end); )
                {
                    attrib_ph.setPage(start);
                    for (GA_Offset elemoff = start; elemoff < end; ++elemoff)
                    {
                        const GA_Offset off = geo->offsetPromote<FROM, TO>(elemoff);
                        dirCrossBase(attrib_ph.value(elemoff), attribUp_h.get(off));
                    }
                }
            }
        }, subscribeRatio, minGrainSize);
    }

    template<typename VECTOR_T>
    void dirCrossSameOwner()
    {
        UT_ASSERT_P(attrib);
        UT_ASSERT_P(attribUp);
        UTparallelFor(groupParser.getSplittableRange(owner), [this](const GA_SplittableRange& r)
        {
            GA_PageHandleT<VECTOR_T, typename VECTOR_T::value_type, true, true, GA_Attribute, GA_ATINumeric, GA_Detail> attrib_ph(attrib);
            GA_PageHandleT<VECTOR_T, typename VECTOR_T::value_type, true, false, const GA_Attribute, const GA_ATINumeric, const GA_Detail> attribUp_ph(attribUp);
            for (GA_PageIterator pit = r.beginPages(); !pit.atEnd(); ++pit)
            {
                GA_Offset start, end;
                for (GA_Iterator it(pit.begin()); it.blockAdvance(start, end); )
                {
                    attrib_ph.setPage(start);
                    attribUp_ph.setPage(start);
                    for (GA_Offset elemoff = start; elemoff < end; ++elemoff)
                    {
                        dirCrossBase(attrib_ph.value(elemoff), attribUp_ph.value(elemoff));
                    }
                }
            }
        }, subscribeRatio, minGrainSize);
    }
    
    template<typename VECTOR_T, GA_AttributeOwner FROM>
    SYS_FORCE_INLINE void dirCross()
    {
        switch (ownerUp)
        {
        case GA_ATTRIB_PRIMITIVE: dirCross<VECTOR_T, FROM, GA_ATTRIB_PRIMITIVE>(); break;
        case GA_ATTRIB_POINT:     dirCross<VECTOR_T, FROM, GA_ATTRIB_POINT    >(); break;
        case GA_ATTRIB_VERTEX:    dirCross<VECTOR_T, FROM, GA_ATTRIB_VERTEX   >(); break;
        case GA_ATTRIB_GLOBAL:    dirCross<VECTOR_T, FROM, GA_ATTRIB_GLOBAL   >(); break;
        default: UT_ASSERT_MSG(0, "Unhandled Owner"); break;
        }
    }

    template<typename VECTOR_T>
    SYS_FORCE_INLINE void dirCross()
    {
        switch (owner)
        {
        case GA_ATTRIB_PRIMITIVE: dirCross<VECTOR_T, GA_ATTRIB_PRIMITIVE>(); break;
        case GA_ATTRIB_POINT:     dirCross<VECTOR_T, GA_ATTRIB_POINT    >(); break;
        case GA_ATTRIB_VERTEX:    dirCross<VECTOR_T, GA_ATTRIB_VERTEX   >(); break;
        case GA_ATTRIB_GLOBAL:    dirCross<VECTOR_T, GA_ATTRIB_GLOBAL   >(); break;
        default: UT_ASSERT_MSG(0, "Unhandled Owner"); break;
        }
    }


    template<typename VECTOR_T>
    SYS_FORCE_INLINE void dirCrossBase(VECTOR_T& dir, const VECTOR_T& up)
    {
        fpreal length;
        if (!normalize && keepLength)
            length = dir.length();
        dir.cross(up);
        
        if (normalize)
            dir.normalize();
        else if (keepLength)
        {
            dir.normalize();
            dir *= length;
        }
        
        if (reverseDir)
            dir *= -1.0;
        
    }


public:
    UT_Vector3T<fpreal> inputUp = UT_Vector3T<fpreal>(0,1,0);
    bool normalize   = false;
    bool keepLength  = true;
    bool reverseDir  = false;
    bool delUpAttrib = false;
    
    //UFE_SplittableString newAttribNames;
private:
    
    
    GA_Attribute* attrib;
    const GA_Attribute* attribUp = nullptr;
    GA_Attribute* attribUp_nonConst = nullptr;

    GA_AttributeOwner owner;
    GA_AttributeOwner ownerUp;
    
    exint subscribeRatio = 64;
    exint minGrainSize = 1024;


}; // End of class GFE_DirCross





#endif
