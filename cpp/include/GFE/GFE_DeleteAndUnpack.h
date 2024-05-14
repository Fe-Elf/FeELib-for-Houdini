
#pragma once

#ifndef __GFE_DelAndUnpack_h__
#define __GFE_DelAndUnpack_h__

#include "GFE/GFE_DeleteAndUnpack.h"


#include "GFE/GFE_GeoFilter.h"


#include "GU/GU_PrimPacked.h"



// Replace by GFE_UnpackByGroup Currently
class GFE_DelAndUnpack : public GFE_GeoFilter {

public:
	using GFE_GeoFilter::GFE_GeoFilter;
	
    void
        setComputeParm(
			const GFE_ElemTraversingMethod elemTraversingMethod = GFE_ElemTraversingMethod::OneElem,
            const bool reverseGroup = true,
			const bool delGroup = true
        )
    {
        setHasComputed();
        this->elemTraversingMethod = elemTraversingMethod;
        this->reverseGroup = reverseGroup;
        this->delGroup = delGroup;
    }


	SYS_FORCE_INLINE void setPrimoff(const GA_Offset primoff = 0, const bool inputAsOffset = true)
    {
    	this->primoff = primoff;
    	this->inputAsOffset = inputAsOffset;
    }

	SYS_FORCE_INLINE void setSkipNPrim(const GA_Offset primoff = 0, const bool inputAsOffset = true, const GA_Size skipNPrim = 1)
    {
    	this->primoff = primoff;
    	this->inputAsOffset = inputAsOffset;
    	this->skipNPrim = skipNPrim;
    }


	


private:

    virtual bool
        computeCore() override
    {
    	switch (elemTraversingMethod) {
    	case GFE_ElemTraversingMethod::Custom:    delAndUnpack_Custom();      break;
    	case GFE_ElemTraversingMethod::OneElem:   delAndUnpack_OneElem();     break;
    	case GFE_ElemTraversingMethod::SkipNElem: delAndUnpack_SkipNElem();   break;
    	default: break;
    	}
    
    	if (delGroup)
    		groupParser.delGroup();

    	GA_Offset start, end;
    	for (GA_Iterator it(geo->getPrimitiveRange()); it.fullBlockAdvance(start, end); )
    	{
	        for (GA_Offset elemoff = start; elemoff < end; ++elemoff)
	        {
	        	int elemTraversingMethodId = geo->getPrimitiveTypeId(elemoff);
	        	if (elemTraversingMethodId != 25 && elemTraversingMethodId != 27 )
	        		continue;
	        	GA_Primitive* const prim = geo->getPrimitive(elemoff);
	            GU_PrimPacked* const primPacked = static_cast<GU_PrimPacked*>(prim);
	        	primPacked->unpack(*geo->asGU_Detail());
	        }
    	}
    	
        return true;
    }


	SYS_FORCE_INLINE void delAndUnpack_Custom()
    { geo->deleteElements(groupParser.getGroup(), reverseGroup, true, GA_Detail::GA_DESTROY_DEGENERATE); }
	
	SYS_FORCE_INLINE void delAndUnpack_OneElem()
    { geo->deleteOnePrimitive(inputAsOffset ? primoff : geo->primitiveOffset(primoff), reverseGroup, true); }
	
	SYS_FORCE_INLINE void delAndUnpack_SkipNElem()
    { geo->deletePrimitiveSkipNElem(inputAsOffset ? primoff : geo->primitiveOffset(primoff), skipNPrim, reverseGroup, true); }


public:
    GFE_ElemTraversingMethod elemTraversingMethod = GFE_ElemTraversingMethod::OneElem;
	
    bool reverseGroup = false;
    bool delGroup = true;
	
	GA_Offset primoff = 0;
	bool inputAsOffset = true;
	GA_Size skipNPrim = 1;
	
private:

    //exint subscribeRatio = 64;
    //exint minGrainSize = 1024;


}; // End of class GFE_DelAndUnpack



#endif
