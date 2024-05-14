
#pragma once

#ifndef __GFE_AttributeCopy_h__
#define __GFE_AttributeCopy_h__

#include "GFE/GFE_AttributeCopy.h"

#include "GFE/GFE_GeoFilter.h"

#include "UFE/UFE_SplittableString.h"
#include "GFE/GFE_OffsetAttribute.h"

#if 0
    GFE_AttribCopy attribCopy(geo, geoTmp, cookparms);
    attribCopy.ownerSrc = GA_ATTRIB_PRIMITIVE;
    attribCopy.ownerDst = GA_ATTRIB_POINT;
    attribCopy.attribMergeType = GFE_AttribMergeType::Set;
    attribCopy.iDAttribInput = true;
    attribCopy.setOffsetAttrib(*srcElemoffAttrib, true);
    
    attribCopy.getRef0GroupArray ().appends(attribCopy.ownerSrc, pattern);
    attribCopy.getRef0AttribArray().appends(attribCopy.ownerSrc, pattern);
    attribCopy.appendGroups ("*");
    attribCopy.appendAttribs("*");
    
    attribCopy.compute();
#endif


class GFE_AttribCopy : public GFE_AttribFilterWithRef0 {

public:
    using GFE_AttribFilterWithRef0::GFE_AttribFilterWithRef0;


    void
        setComputeParm(
            const GFE_AttribMergeType attribMergeType = GFE_AttribMergeType::Set,
            const bool iDAttribInput = false,
            const GA_Offset copyFromSingleOff = GFE_INVALID_OFFSET,
            const exint subscribeRatio = 64,
            const exint minGrainSize   = 1024
        )
    {
        setHasComputed();
        this->attribMergeType = attribMergeType;
        this->iDAttribInput   = iDAttribInput;
        this->copyFromSingleOff = copyFromSingleOff;
        
        this->subscribeRatio = subscribeRatio;
        this->minGrainSize   = minGrainSize;
    }

    SYS_FORCE_INLINE void appendGroups(const UT_StringRef& pattern)
    { getRef0GroupArray().appends(ownerSrc, pattern); }

    SYS_FORCE_INLINE void appendAttribs(const UT_StringRef& pattern)
    { getRef0AttribArray().appends(ownerSrc, pattern); }

    void setOffsetAttrib(const GA_Attribute& attrib, const bool isOffset)
    {
        UT_ASSERT_P(iDAttribInput ? (geo == &attrib.getDetail()) : (geoRef0 == &attrib.getDetail()));
        iDAttrib.bind(attrib, isOffset);
        iDAttrib.bindIndexMap(
            iDAttribInput ?
            geoRef0->getIndexMap(ownerSrc) :
            geo->getIndexMap(ownerDst));
    }

    void setOffsetAttrib(const UT_StringRef& iDAttribName, const bool isOffset)
    {
        if (!iDAttribName.isstring() || iDAttribName.length()==0)
        {
            iDAttrib.clear();
            return;
        }

        if (iDAttribInput)//Destination
        {
            iDAttrib.bind(*geo, ownerDst, iDAttribName, isOffset);
            iDAttrib.bindIndexMap(geoRef0->getIndexMap(ownerSrc));
        }
        else//Source
        {
            iDAttrib.bind(*geoRef0, ownerSrc, iDAttribName, isOffset);
            iDAttrib.bindIndexMap(geo->getIndexMap(ownerDst));
        }
    }






private:


    virtual bool
        computeCore() override
    {
        const bool isAttribEmpty = getRef0AttribArray().isEmpty();

        if (isAttribEmpty && getRef0GroupArray().isEmpty())
            return false;

        doCcopyFromSingleOff = GFE_Type::isValidOffset(copyFromSingleOff);
        if (doCcopyFromSingleOff)
        {
            const GA_IndexMap& indexMapDst = geo->getIndexMap(ownerDst);
            doCcopyFromSingleOff = copyFromSingleOff < indexMapDst.offsetSize() && !indexMapDst.isOffsetVacant(copyFromSingleOff);
        }

        const GA_IndexMap& indexMap_runOver = iDAttribInput || doCcopyFromSingleOff
                                            ? geo->getIndexMap(ownerDst)
                                            : geoRef0->getIndexMap(ownerSrc);

        const GA_Range geoRange(indexMap_runOver);
        const GA_SplittableRange geoSplittableRange(geoRange);



        const GA_GroupType groupTypeDst = GFE_Type::attributeOwner_groupType(ownerDst);

        GA_GroupTable* const groupDst = geo->getGroupTable(groupTypeDst);

        const size_t sizeGroup    = getOutGroupArray().size();
        const size_t sizeRefGroup = getRef0GroupArray().size();
        for (size_t i = 0; i < sizeRefGroup; i++)
        {
            UT_ASSERT_P(getRef0GroupArray()[i]);
            const GA_Group& groupRef = *getRef0GroupArray()[i];

            GA_Group* newAttrib;
            if (i >= sizeGroup)
            {
                const UT_StringHolder& newName = newGroupNames.getValidAttribName(groupRef);
                const bool detached = GFE_Type::isInvalid(newName);
            
                newAttrib = detached ? nullptr : groupDst->find(newName);
                if (newAttrib)
                    getOutGroupArray().append(newAttrib);
                else
                {
                    if (detached)
                    {
                        newAttrib = groupDst->newDetachedGroup();
                        getOutGroupArray().appendDetached(newAttrib);
                    }
                    else
                    {
                        newAttrib = groupDst->newGroup(newName);
                        getOutGroupArray().append(newAttrib);
                    }
                }
            }
            else
            {
                newAttrib = getOutGroupArray()[i];
                UT_ASSERT_P(&newAttrib->getDetail() == geo);
            }
            
            copyGroup(geoSplittableRange, *newAttrib, groupRef);
        }

        GA_AttributeSet& attribDst = geo->getAttributes();

        const size_t sizeAttrib    = getOutAttribArray().size();
        const size_t sizeRefAttrib = getRef0AttribArray().size();
        for (size_t i = 0; i < sizeRefAttrib; i++)
        {
            UT_ASSERT_P(getRef0AttribArray()[i]);
            const GA_Attribute& attribRef = *getRef0AttribArray()[i];

            GA_Attribute* newAttrib = nullptr;
            if (i < sizeAttrib)
            {
                newAttrib = getOutAttribArray()[i];
                UT_ASSERT_P(&newAttrib->getDetail() == geo);
            }
            if (newAttrib && !GFE_Type::checkTupleAttrib(newAttrib, attribRef.getStorageClass(), GA_STORE_INVALID, attribRef.getTupleSize()))
            {
                attribDst.destroyAttribute(newAttrib);
                newAttrib = nullptr;
            }
            
            if (!newAttrib)
            {
                const UT_StringHolder& newName = newAttribNames.getValidAttribName(attribRef);
                const bool detached = GFE_Type::isInvalid(newName);

                newAttrib = detached ? nullptr : attribDst.findAttribute(ownerDst, newName);
                if (newAttrib && !GFE_Type::checkTupleAttrib(newAttrib, attribRef.getStorageClass(), GA_STORE_INVALID, attribRef.getTupleSize()))
                {
                    attribDst.destroyAttribute(newAttrib);
                    newAttrib = nullptr;
                }
                if (newAttrib)
                {
                    //newAttrib->bumpDataId();
                    getOutAttribArray().append(newAttrib);
                }
                else
                {
                    if (detached)
                    {
                        newAttrib = getOutAttribArray().cloneDetached(attribRef);
                        //GA_AttributeUPtr attribUPtr;
                        //GA_Attribute attrib(attribRef.getType(), attribRef.getIndexMap(), attribRef.getScope(), "");
                        //attribUPtr.reset(&attrib);
                        //getOutAttribArray().appendUPtr(attrib);
                    
                        //newAttrib = attribDst.cloneTempAttribute(ownerDst, attribRef, true);
                    }
                    else
                    {
                        newAttrib = attribDst.cloneAttribute(ownerDst, newName, attribRef, true);
                    }
                }
            }
            
            copyAttrib(geoSplittableRange, *newAttrib, attribRef);
        }

        return true;
    }


    void copyGroup(GA_EdgeGroup& attribNew, const GA_EdgeGroup& attribRef)
    {
        if (iDAttrib.isValidh())
        {
            for (GA_EdgeGroup::const_iterator it = attribRef.begin(); it.atEnd(); ++it)
            {
                const GA_Edge edge = it.getEdge();
                const exint id0 = iDAttrib.getOffset(edge.p0());
                const exint id1 = iDAttrib.getOffset(edge.p1());
                attribNew.add(id0, id1);
            }
        }
        else
        {
            for (GA_EdgeGroup::const_iterator it = attribRef.begin(); it.atEnd(); ++it)
            {
                const GA_Edge edge = it.getEdge();
                attribNew.add(edge.p0(), edge.p1());
            }
        }
    }

    void
        copyGroup(
            const GA_SplittableRange& geoSplittableRange,
            GA_ElementGroup& attribNew,
            const GA_ElementGroup& attribRef
        )
    {
        const GA_IndexMap& indexMap_target = iDAttribInput ? attribRef.getIndexMap() : attribNew.getIndexMap();
        const GFE_OffsetAttrib& iDAttribRef = iDAttrib;
        if (iDAttrib.isValidh())
        {
            if (iDAttribInput)//Dst
            {
                UTparallelFor(geoSplittableRange, [&attribNew, &attribRef, &iDAttribRef, &indexMap_target](const GA_SplittableRange& r)
                {
                    GA_Offset start, end;
                    for (GA_Iterator it(r); it.blockAdvance(start, end); )
                    {
                        for (GA_Offset elemoff = start; elemoff < end; ++elemoff)
                        {
                            const exint id = iDAttribRef.getOffset(elemoff);
                            if (GFE_Type::isValidOffset(indexMap_target, id))
                                attribNew.setElement(elemoff, attribRef.contains(id));
                        }
                    }
                }, subscribeRatio, minGrainSize);
            }
            else//Src
            {
                UTparallelFor(geoSplittableRange, [&attribNew, &attribRef, &iDAttribRef, &indexMap_target](const GA_SplittableRange& r)
                {
                    GA_Offset start, end;
                    for (GA_Iterator it(r); it.blockAdvance(start, end); )
                    {
                        for (GA_Offset elemoff = start; elemoff < end; ++elemoff)
                        {
                            const exint id = iDAttribRef.getOffset(elemoff);
                            if (GFE_Type::isValidOffset(indexMap_target, id))
                                attribNew.setElement(id, attribRef.contains(elemoff));
                        }
                    }
                }, subscribeRatio, minGrainSize);
            }
        }
        else
        {
            if (doCcopyFromSingleOff)
            {
                const GA_Offset copyFromSingleOffTmp = copyFromSingleOff;
                if (GFE_Type::isValidOffset(indexMap_target, copyFromSingleOff))
                {
                    UTparallelFor(geoSplittableRange, [&attribNew, &attribRef, &indexMap_target, copyFromSingleOffTmp](const GA_SplittableRange& r)
                    {
                        GA_Offset start, end;
                        for (GA_Iterator it(r); it.blockAdvance(start, end); )
                        {
                            for (GA_Offset elemoff = start; elemoff < end; ++elemoff)
                            {
                                attribNew.setElement(elemoff, attribRef.contains(copyFromSingleOffTmp));
                            }
                        }
                    }, subscribeRatio, minGrainSize);
                }
            }
            else
            {
                UTparallelFor(geoSplittableRange, [&attribNew, &attribRef, &indexMap_target](const GA_SplittableRange& r)
                {
                    GA_Offset start, end;
                    for (GA_Iterator it(r); it.blockAdvance(start, end); )
                    {
                        for (GA_Offset elemoff = start; elemoff < end; ++elemoff)
                        {
                            //attribNew.copy(elemoff, attribRef, elemoff);
                            if (GFE_Type::isValidOffset(indexMap_target, elemoff))
                                attribNew.setElement(elemoff, attribRef.contains(elemoff));
                        }
                    }
                }, subscribeRatio, minGrainSize);
            }
        }
        attribNew.invalidateGroupEntries();
    }

    SYS_FORCE_INLINE void copyGroup(
        const GA_SplittableRange& geoSplittableRange,
        GA_Group& attribNew,
        const GA_Group& attribRef
    )
    {
        if (attribNew.classType() == GA_GROUP_EDGE)
            copyGroup(static_cast<GA_EdgeGroup&>(attribNew), static_cast<const GA_EdgeGroup&>(attribRef));
        else
            copyGroup(geoSplittableRange, static_cast<GA_ElementGroup&>(attribNew), static_cast<const GA_ElementGroup&>(attribRef));
    }



    void copyAttrib(
        const GA_SplittableRange& geoSplittableRange,
        GA_Attribute& attribNew,
        const GA_Attribute& attribRef
    )
    {
        //const GA_IndexMap& indexMap = geo->getIndexMap(GFE_Type::attributeOwner_groupType(attribNew.classType()));
        const GA_IndexMap& indexMap_target = iDAttribInput ? attribRef.getIndexMap() : attribNew.getIndexMap();
        const GFE_OffsetAttrib& iDAttribRef = iDAttrib;
        if (iDAttrib.isValidh())
        {
            if (iDAttribInput)//Dst
            {
                UTparallelFor(geoSplittableRange, [&attribNew, &attribRef, &iDAttribRef, &indexMap_target](const GA_SplittableRange& r)
                {
                    GA_Offset start, end;
                    for (GA_Iterator it(r); it.blockAdvance(start, end); )
                    {
                        for (GA_Offset elemoff = start; elemoff < end; ++elemoff)
                        {
                            const exint id = iDAttribRef.getOffset(elemoff);
                            if (GFE_Type::isValidOffset(indexMap_target, id))
                                attribNew.copy(elemoff, attribRef, id);
                            //attribNew.set(elemoff, attribRef.contains(id));
                        }
                    }
                }, subscribeRatio, minGrainSize);
            }
            else//Src
            {
                UTparallelFor(geoSplittableRange, [&attribNew, &attribRef, &iDAttribRef, &indexMap_target](const GA_SplittableRange& r)
                {
                    GA_Offset start, end;
                    for (GA_Iterator it(r); it.blockAdvance(start, end); )
                    {
                        for (GA_Offset elemoff = start; elemoff < end; ++elemoff)
                        {
                            const exint id = iDAttribRef.getOffset(elemoff);
                            if (GFE_Type::isValidOffset(indexMap_target, id))
                                attribNew.copy(id, attribRef, elemoff);
                        }
                    }
                }, subscribeRatio, minGrainSize);
            }
        }
        else
        {
            if (doCcopyFromSingleOff)
            {
                const GA_Offset copyFromSingleOffTmp = copyFromSingleOff;
                if (GFE_Type::isValidOffset(indexMap_target, copyFromSingleOffTmp))
                {
                    UTparallelFor(geoSplittableRange, [&attribNew, &attribRef, copyFromSingleOffTmp](const GA_SplittableRange& r)
                    {
                        GA_Offset start, end;
                        for (GA_Iterator it(r); it.blockAdvance(start, end); )
                        {
                            for (GA_Offset elemoff = start; elemoff < end; ++elemoff)
                            {
                                attribNew.copy(elemoff, attribRef, copyFromSingleOffTmp);
                            }
                        }
                    }, subscribeRatio, minGrainSize);
                }
            }
            else
            {
                UTparallelFor(geoSplittableRange, [&attribNew, &attribRef, &indexMap_target](const GA_SplittableRange& r)
                {
                    GA_Offset start, end;
                    for (GA_Iterator it(r); it.blockAdvance(start, end); )
                    {
                        for (GA_Offset elemoff = start; elemoff < end; ++elemoff)
                        {
                            //attribNew.copy(elemoff, attribRef, elemoff);
                            if (GFE_Type::isValidOffset(indexMap_target, elemoff))
                                attribNew.copy(elemoff, attribRef, elemoff);
                        }
                    }
                }, subscribeRatio, minGrainSize);
            }
        }
    }




public:
    GA_AttributeOwner ownerDst;
    GA_AttributeOwner ownerSrc;
    
    UFE_SplittableString newAttribNames;
    UFE_SplittableString newGroupNames;
    
    GFE_AttribMergeType attribMergeType = GFE_AttribMergeType::Set;
    bool iDAttribInput = false; //True means ** ID ATTRIB ** is on DESTINATION(geo); While false means id attrib is on Source(geoRef0)

    GA_Offset copyFromSingleOff = GFE_INVALID_OFFSET;
    //GA_GroupType owner;
    //GA_GroupType ownerRef;
    
private:
    GFE_OffsetAttrib iDAttrib;
    bool doCcopyFromSingleOff;

    int subscribeRatio = 8;
    int minGrainSize = 1024;

}; // End of class GFE_AttribCopy































#endif
