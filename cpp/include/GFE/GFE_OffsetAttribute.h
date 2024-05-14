
#pragma once

#ifndef __GFE_OffsetAttribute_h__
#define __GFE_OffsetAttribute_h__

#include "GFE/GFE_OffsetAttribute.h"

#include "GA/GA_Detail.h"
#include "GA/GA_PageHandle.h"

#include "GFE/GFE_Type.h"





class GFE_OffsetAttrib {

public:

	GFE_OffsetAttrib(){}

	~GFE_OffsetAttrib(){}

public:

	// SYS_FORCE_INLINE const GA_Attribute* getAttribute()
	// { return attrib_h.getAttribute(); }


	SYS_FORCE_INLINE bool isValidh() const
	{ return attrib_h.isValid(); }

	SYS_FORCE_INLINE bool isValidph() const
	{ return attrib_ph.isValid(); }
	
	SYS_FORCE_INLINE void clear()
	{ attrib_h.clear(); attrib_ph.clear(); }

	
	void bind(const GA_Attribute& attrib, const bool isOffset)
	{
		attrib_h.bind(&attrib);
		this->isOffset = isOffset;
		indexMap = &attrib.getIndexMap();
	}

	void bind(const GA_Detail& geo, const GA_AttributeOwner owner, const UT_StringRef& attribName, const bool isOffset)
	{
		const GA_Attribute* const attrib = geo.findAttribute(owner, attribName);
		if (attrib)
			bind(*attrib, isOffset);
		else
			attrib_h.clear();
	}

	SYS_FORCE_INLINE void bind(const GA_Detail& geo, const GA_GroupType owner, const UT_StringRef& attribName, const bool isOffset)
	{ bind(geo, GFE_Type::attributeOwner_groupType(owner), attribName, isOffset); }



	void bindPage(const GA_Attribute& attrib, const bool isOffset)
	{
		attrib_ph.bind(&attrib);
		this->isOffset = isOffset;
		indexMap = &attrib.getIndexMap();
	}

	SYS_FORCE_INLINE void bindPage(const GA_Detail& geo, const GA_AttributeOwner owner, const UT_StringRef& attribName, const bool isOffset)
	{
		const GA_Attribute* const attrib = geo.findAttribute(owner, attribName);
		if (attrib)
			bindPage(*attrib, isOffset);
		else
			attrib_ph.clear();
	}

	SYS_FORCE_INLINE void bindPage(const GA_Detail& geo, const GA_GroupType owner, const UT_StringRef& attribName, const bool isOffset)
	{ bindPage(geo, GFE_Type::attributeOwner_groupType(owner), attribName, isOffset); }





	SYS_FORCE_INLINE void setPage(const GA_Offset pageBase)
	{
		UT_ASSERT(attrib_ph.getAttribute());
		attrib_ph.setPage(pageBase);
	}

	SYS_FORCE_INLINE void bindIndexMap(const GA_IndexMap& indexMap)
	{ this->indexMap = &indexMap; }




	SYS_FORCE_INLINE GA_Offset getOffset(const GA_Offset elemoff) const
	{
		UT_ASSERT(attrib_h.getAttribute());
		return isOffset ? attrib_h.get(elemoff) : indexMap->offsetFromIndex(attrib_h.get(elemoff));
	}

	SYS_FORCE_INLINE GA_Index getIndex(const GA_Offset elemoff) const
	{
		UT_ASSERT(attrib_h.getAttribute());
		return isOffset ? indexMap->indexFromOffset(attrib_h.get(elemoff)) : attrib_h.get(elemoff);
	}

	SYS_FORCE_INLINE GA_Offset getPageOffset(const GA_Offset elemoff) const
	{
		UT_ASSERT(attrib_ph.getAttribute());
		return isOffset ? attrib_ph.get(elemoff) : indexMap->offsetFromIndex(attrib_ph.get(elemoff));
	}
	
	SYS_FORCE_INLINE GA_Index getPageIndex(const GA_Offset elemoff) const
	{
		UT_ASSERT(attrib_ph.getAttribute());
		return isOffset ? indexMap->indexFromOffset(attrib_ph.get(elemoff)) : attrib_ph.get(elemoff);
	}

private:
	//const GA_Attribute* attrib;
	GA_ROHandleT<GA_Offset> attrib_h;
	GA_PageHandleScalar<GA_Offset>::ROType attrib_ph;
	//GA_AttributeOwner attribOwner;
	const GA_IndexMap* indexMap;

	//GA_Attribute* attribNonconst;
	bool isOffset;

}; // End of class GFE_OffsetAttribute

#endif
