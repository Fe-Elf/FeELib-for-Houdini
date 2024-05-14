
#pragma once

#ifndef __GFE_AttribDuplicate_h__
#define __GFE_AttribDuplicate_h__

#include "GFE/GFE_AttributeDuplicate.h"

#include "GFE/GFE_GeoFilter.h"



#include "UFE/UFE_SplittableString.h"

class GFE_AttribDuplicate : public GFE_AttribCreateFilterWithRef0 {

public:
    using GFE_AttribCreateFilterWithRef0::GFE_AttribCreateFilterWithRef0;


    void
        setComputeParm(
            const bool delSrcAttrib = false,
            const exint subscribeRatio = 64,
            const exint minGrainSize = 1024
        )
    {
        setHasComputed();
        this->delSrcAttrib = delSrcAttrib;
        this->subscribeRatio = subscribeRatio;
        this->minGrainSize = minGrainSize;
    }





private:


    virtual bool
        computeCore() override
    {
        const bool isAttribEmpty = getInAttribArray().isEmpty();

        if (isAttribEmpty && getInGroupArray().isEmpty())
            return false;
        

        const size_t sizeRefGroup = getInGroupArray().size();
        for (size_t i = 0; i < sizeRefGroup; i++)
        {
            const GA_Group& groupRef = *getInGroupArray()[i];

            const UT_StringHolder& newName = newGroupNames.getIsValid() ? newGroupNames.getNext<UT_StringHolder>() : groupRef.getName();
            const bool detached = !newGroupNames.getIsValid() || !GFE_Type::isPublicAttribName(newName);
            
            GA_GroupTable* const groupDst = geo->getGroupTable(groupRef.classType());
            GA_Group* newAttrib = detached ? nullptr : groupDst->find(newName);
            if (newAttrib)
                GFE_Group::groupBumpDataId(newAttrib);
            else
            {
                if (detached)
                    newAttrib = groupDst->newDetachedGroup();
                else
                    newAttrib = groupDst->newGroup(newName);
            }
            getOutGroupArray().append(newAttrib);
            
            groupDuplicate(*newAttrib, groupRef);
        }

        GA_AttributeSet& attribDst = geo->getAttributes();

        const size_t sizeRefAttrib = getInAttribArray().size();
        for (size_t i = 0; i < sizeRefAttrib; i++)
        {
            const GA_Attribute& attribRef = *getInAttribArray()[i];
            
            const GA_SplittableRange& geoSplittableRange = groupParser.getSplittableRange(attribRef);
            
            const UT_StringHolder& newName = newAttribNames.getIsValid() ? newAttribNames.getNext<UT_StringHolder>() : attribRef.getName();
            const bool detached = !newAttribNames.getIsValid() || !GFE_Type::isPublicAttribName(newName);

            GA_Attribute* newAttrib = attribDst.findAttribute(attribRef.getOwner(), newName);
            if (newAttrib)
                newAttrib->bumpDataId();
            else
            {
                if (detached)
                    newAttrib = attribDst.cloneTempAttribute(attribRef.getOwner(), attribRef, true);
                else
                    newAttrib = attribDst.cloneAttribute(attribRef.getOwner(), newName, attribRef, true);
            }

            getOutAttribArray().append(newAttrib);
            
            attribDuplicate(geoSplittableRange, *newAttrib, attribRef);
        }

        UT_ASSERT_MSG(!delSrcAttrib || srcAttribPtr_nonConst, "cant destory Src Attrib");
        if (delSrcAttrib && srcAttribPtr_nonConst)
            geo->destroyAttrib(srcAttribPtr_nonConst);

        return true;
    }


    SYS_FORCE_INLINE void groupDuplicate(GA_EdgeGroup& attribNew, const GA_EdgeGroup& attribRef)
    { attribNew = attribRef; }

    SYS_FORCE_INLINE void groupDuplicate(GA_ElementGroup& attribNew, const GA_ElementGroup& attribRef)
    { attribNew.copyMembership(attribRef); }

    SYS_FORCE_INLINE void groupDuplicate(GA_Group& attribNew, const GA_Group& attribRef)
    {
        if (attribNew.classType() == GA_GROUP_EDGE)
            groupDuplicate(static_cast<GA_EdgeGroup&>(attribNew), static_cast<const GA_EdgeGroup&>(attribRef));
        else
            groupDuplicate(static_cast<GA_ElementGroup&>(attribNew), static_cast<const GA_ElementGroup&>(attribRef));
    }

    
    void attribDuplicate(
        const GA_SplittableRange& geoSplittableRange,
        GA_Attribute& attribNew,
        const GA_Attribute& attribRef
    )
    {
        UTparallelFor(geoSplittableRange, [&attribNew, &attribRef](const GA_SplittableRange& r)
        {
            GA_Offset start, end;
            for (GA_Iterator it(r); it.blockAdvance(start, end); )
            {
                for (GA_Offset elemoff = start; elemoff < end; ++elemoff)
                {
                    attribNew.copy(elemoff, attribRef, elemoff);
                }
            }
        }, subscribeRatio, minGrainSize);
    }




public:
    bool delSrcAttrib = false;
    GA_Attribute* dstAttribPtr;
    
    UFE_SplittableString newAttribNames;
    UFE_SplittableString newGroupNames;

private:
    const GA_Attribute* srcAttribPtr;
    GA_Attribute* srcAttribPtr_nonConst;
    GA_AttributeOwner attribOwner;

    int subscribeRatio = 8;
    int minGrainSize = 1024;

}; // End of class GFE_AttribDuplicate







#endif
