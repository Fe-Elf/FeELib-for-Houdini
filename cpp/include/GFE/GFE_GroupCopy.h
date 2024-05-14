
#pragma once

#ifndef __GFE_GroupCopy_h__
#define __GFE_GroupCopy_h__

//#include "GFE/GFE_GroupCopy.h"

#include "GA/GA_Detail.h"
#include "GA/GA_SplittableRange.h"

#include "GFE/GFE_Type.h"







namespace GFE_GroupCopy_Namespace {













static void
copyGroup(
    GA_EdgeGroup& attribNew,
    const GA_EdgeGroup& attribRef,
    const GA_Attribute* const attribID = nullptr
)
{
    if (attribID)
    {
        const GA_ROHandleT<exint> iDAttrib_h(attribID);
        for (GA_EdgeGroup::const_iterator it = attribRef.begin(); it.atEnd(); ++it)
        {
            const GA_Edge edge = it.getEdge();
            const exint id0 = iDAttrib_h.get(edge.p0());
            const exint id1 = iDAttrib_h.get(edge.p1());
            attribNew.add(id0, id1);
        }
    }
    else
    {
        for (GA_EdgeGroup::const_iterator it = attribRef.begin(); it.atEnd(); ++it)
        {
            GA_Edge edge = it.getEdge();
            attribNew.add(edge.p0(), edge.p1());
        }
    }
}

static void
copyGroup(
    const GA_SplittableRange& geoSplittableRange,
    GA_ElementGroup& attribNew,
    const GA_ElementGroup& attribRef,
    const GA_Attribute* const attribID = nullptr,
    const int subscribeRatio = 8,
    const int minGrainSize = 1024
)
{
    if (attribID)
    {
        const GA_ROHandleT<exint> iDAttrib_h(attribID);
        UTparallelFor(geoSplittableRange, [&attribNew, &attribRef, &iDAttrib_h](const GA_SplittableRange& r)
        {
            GA_Offset start, end;
            for (GA_Iterator it(r); it.blockAdvance(start, end); )
            {
                for (GA_Offset elemoff = start; elemoff < end; ++elemoff)
                {
                    const exint id = iDAttrib_h.get(elemoff);
                    //attribNew.copy(id, attribRef, elemoff);
                    attribNew.setElement(id, attribRef.contains(elemoff));
                }
            }
        }, subscribeRatio, minGrainSize);
    }
    else
    {
        UTparallelFor(geoSplittableRange, [&attribNew, &attribRef](const GA_SplittableRange& r)
        {
            GA_Offset start, end;
            for (GA_Iterator it(r); it.blockAdvance(start, end); )
            {
                for (GA_Offset elemoff = start; elemoff < end; ++elemoff)
                {
                    //attribNew.copy(elemoff, attribRef, elemoff);
                    attribNew.setElement(elemoff, attribRef.contains(elemoff));
                }
            }
        }, subscribeRatio, minGrainSize);
    }
}





static void
copyGroup(
    const GA_SplittableRange& geoSplittableRange,
    GA_Group& attribNew,
    const GA_Group& attribRef,
    const GA_Attribute* const attribID = nullptr,
    const int subscribeRatio = 8,
    const int minGrainSize = 1024
)
{
    if (attribNew.classType() == GA_GROUP_EDGE)
    {
        copyGroup(static_cast<GA_EdgeGroup&>(attribNew), static_cast<const GA_EdgeGroup&>(attribRef), attribID);
    }
    else
    {
        copyGroup(geoSplittableRange, static_cast<GA_ElementGroup&>(attribNew), static_cast<const GA_ElementGroup&>(attribRef), attribID, subscribeRatio, minGrainSize);
    }
}


static void
copyGroup(
    GA_Detail* const geo,
    const GA_GroupType owner,
    const GA_Detail* const geoRef,
    const GA_GroupType ownerRef,
    const UT_StringHolder& attribPattern,
    //const UT_StringHolder& attribNameNewPattern,
    const UT_StringHolder& iDAttribName = "",
    const bool iDAttribInput = false,
    const int subscribeRatio = 8,
    const int minGrainSize = 1024
)
{
    if (attribPattern.length() == 0)
        return;

    GA_GroupTable* const attribsDst = geo->getGroupTable(owner);
    GA_Group* attribNew = nullptr;
    const GA_Group* attribRef = nullptr;


    const GA_Attribute* iDAttrib = nullptr;

    GA_Range range;
    if (iDAttribInput)//DESTINATION
    {
        if (iDAttribName.length() > 0)
            iDAttrib = geo->getAttributes().findAttribute(GFE_Type::attributeOwner_groupType(owner), iDAttribName);
        range = GA_Range(geo->getIndexMap(GFE_Type::attributeOwner_groupType(owner)));
    }
    else//Src
    {
        if (iDAttribName.length() > 0)
            iDAttrib = geoRef->getAttributes().findAttribute(GFE_Type::attributeOwner_groupType(ownerRef), iDAttribName);
        range = GA_Range(geoRef->getIndexMap(GFE_Type::attributeOwner_groupType(ownerRef)));
    }
    const GA_SplittableRange geoSplittableRange(range);


    for (GA_GroupTable::iterator<GA_Group> it = geoRef->getGroupTable(ownerRef)->beginTraverse(); !it.atEnd(); ++it)
    {
        attribRef = it.group();
        //if (attribRef->isDetached())
        //    continue;
        const UT_StringHolder& attribName = attribRef->getName();

        if (!attribName.multiMatch(attribPattern, true, " "))
            continue;

        const UT_StringHolder& attribNameNew = attribName;

        attribNew = attribsDst->find(attribNameNew);
        if (attribNew)
        {
            GFE_Group::groupBumpDataId(attribNew);
        }
        else
        {
            if (owner == GA_GROUP_EDGE)
            {
                attribNew = attribsDst->newGroup(attribNameNew);
            }
            else
            {
                attribNew = attribsDst->newGroup(attribNameNew);
                //attribNew = static_cast<GA_ElementGroupTable*>(attribsDst)->cloneAttribute(owner, attribNameNew, *attribRef, true);
            }
        }

        copyGroup(geoSplittableRange, *attribNew, *attribRef, iDAttrib, subscribeRatio, minGrainSize);
        //if (iDAttribInput)//DESTINATION
        //{
        //    copyGroup(geoSplittableRange, *attribNew, *attribRef, iDAttrib, subscribeRatio, minGrainSize);
        //}
        //else//Src
        //{
        //    copyGroup(geoSplittableRange, *attribNew, *attribRef, iDAttrib, subscribeRatio, minGrainSize);
        //}
    }
}


















} // End of namespace GFE_GroupCopy

#endif
