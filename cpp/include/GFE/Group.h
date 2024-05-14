
#pragma once

#ifndef __GFE_Group_h__
#define __GFE_Group_h__

#include <GFE/Group.h>

#include <GFE/Type.h>


_GFE_BEGIN
class GFE_GroupSetter
{
public:
    GFE_GroupSetter() {}
    
    GFE_GroupSetter(GA_Group*          const group) { operator=(group); }
    GFE_GroupSetter(GA_ElementGroup*   const group) : elementGroup(group) {}
    GFE_GroupSetter(GA_PrimitiveGroup* const group) : elementGroup(static_cast<GA_ElementGroup*>(group)) {}
    GFE_GroupSetter(GA_PointGroup*     const group) : elementGroup(static_cast<GA_ElementGroup*>(group)) {}
    GFE_GroupSetter(GA_VertexGroup*    const group) : elementGroup(static_cast<GA_ElementGroup*>(group)) {}
    GFE_GroupSetter(GA_EdgeGroup*      const group) : edgeGroup(group) {}

    ~GFE_GroupSetter(){}


    
    SYS_FORCE_INLINE void clear()
    { elementGroup = nullptr; edgeGroup = nullptr; }

    SYS_FORCE_INLINE bool isElementValid() const
    { return elementGroup; }

    SYS_FORCE_INLINE bool isEdgeValid() const
    { return elementGroup; }

    SYS_FORCE_INLINE bool isValid() const
    { return elementGroup || edgeGroup; }

    SYS_FORCE_INLINE bool isElementInvalid() const
    { return !elementGroup; }

    SYS_FORCE_INLINE bool isEdgeInvalid() const
    { return !edgeGroup; }

    SYS_FORCE_INLINE bool isInvalid() const
    { return !isValid(); }

    
    SYS_FORCE_INLINE void setParm(const gfe::EnumClass::GroupMergeType groupMergeType, const bool reverseGroup = false)
    { this->groupMergeType = groupMergeType; this->reverseGroup = reverseGroup; }

    SYS_FORCE_INLINE void setParm(const bool reverseGroup, const gfe::EnumClass::GroupMergeType groupMergeType = gfe::EnumClass::GroupMergeType::Replace)
    { setParm(groupMergeType, reverseGroup); }

    SYS_FORCE_INLINE void setComputeParm(const gfe::EnumClass::GroupMergeType groupMergeType, const bool reverseGroup = false)
    { setParm(groupMergeType, reverseGroup); }

    SYS_FORCE_INLINE void setComputeParm(const bool reverseGroup, const gfe::EnumClass::GroupMergeType groupMergeType = gfe::EnumClass::GroupMergeType::Replace)
    { setParm(groupMergeType, reverseGroup); }
    
    
    GFE_GroupSetter& operator=(GA_Group* const inGroupPtr)
    {
        if (inGroupPtr->isElementGroup())
        {
            elementGroup = static_cast<GA_ElementGroup*>(inGroupPtr);
            edgeGroup    = nullptr;
        }
        else
        {
            edgeGroup    = static_cast<GA_EdgeGroup*>(inGroupPtr);
            elementGroup = nullptr;
        }
        return *this;
    }


    SYS_FORCE_INLINE GFE_GroupSetter& operator=(GA_ElementGroup* const inGroupPtr)
    { elementGroup = inGroupPtr; edgeGroup = nullptr; return *this; }

    SYS_FORCE_INLINE GFE_GroupSetter& operator=(GA_PrimitiveGroup* const inGroupPtr)
    { elementGroup = static_cast<GA_ElementGroup*>(inGroupPtr); edgeGroup = nullptr; return *this; }

    SYS_FORCE_INLINE GFE_GroupSetter& operator=(GA_PointGroup* const inGroupPtr)
    { elementGroup = static_cast<GA_ElementGroup*>(inGroupPtr); edgeGroup = nullptr; return *this; }

    SYS_FORCE_INLINE GFE_GroupSetter& operator=(GA_VertexGroup* const inGroupPtr)
    { elementGroup = static_cast<GA_ElementGroup*>(inGroupPtr); edgeGroup = nullptr; return *this; }

    SYS_FORCE_INLINE GFE_GroupSetter& operator=(GA_EdgeGroup* const inGroupPtr)
    { edgeGroup = inGroupPtr; elementGroup = nullptr; return *this; }

    

    SYS_FORCE_INLINE void invalidateGroupEntries()
    {  if (elementGroup) elementGroup->invalidateGroupEntries(); }

    
    SYS_FORCE_INLINE GA_Group* getGroup() const
    { 
         if (elementGroup)
             return static_cast<GA_Group*>(elementGroup);
         else
             return static_cast<GA_Group*>(edgeGroup);
    }

    SYS_FORCE_INLINE GA_ElementGroup* getElementGroup() const
    { return elementGroup; }
    
    SYS_FORCE_INLINE GA_EdgeGroup* getEdgeGroup() const
    { return edgeGroup; }


    
    void set(const GA_Offset elemoff, bool b)
    {
        UT_ASSERT_P(elementGroup);

        b ^= reverseGroup;
        
        switch (groupMergeType)
        {
        case gfe::EnumClass::GroupMergeType::Union:     b |=  elementGroup->contains(elemoff); break;
        case gfe::EnumClass::GroupMergeType::Intersect: b &=  elementGroup->contains(elemoff); break;
        case gfe::EnumClass::GroupMergeType::Subtract:  b &= !elementGroup->contains(elemoff); break;
        }
        elementGroup->setElement(elemoff, b);
    }
    
    SYS_FORCE_INLINE void set(const GA_Offset elemoff0, const GA_Offset elemoff1, bool b)
    {
#if 1
        set(GA_Edge(elemoff0, elemoff1), b);
#else
        UT_ASSERT_P(edgeGroup);
        
        b ^= reverseGroup;
        
        switch (groupMergeType)
        {
        case gfe::EnumClass::GroupMergeType::Union:     b |=  edgeGroup->contains(elemoff0, elemoff1); break;
        case gfe::EnumClass::GroupMergeType::Intersect: b &=  edgeGroup->contains(elemoff0, elemoff1); break;
        case gfe::EnumClass::GroupMergeType::Subtract:  b &= !edgeGroup->contains(elemoff0, elemoff1); break;
        // case gfe::EnumClass::GroupMergeType::Replace: break;
        default: break;
        }
        if (b)
            edgeGroup->add(elemoff0, elemoff1);
        else
            edgeGroup->remove(elemoff0, elemoff1);
#endif
    }
    
    void set(const GA_Edge& edge, bool b)
    {
        UT_ASSERT_P(edgeGroup);
        
        b ^= reverseGroup;
        
        switch (groupMergeType)
        {
        case gfe::EnumClass::GroupMergeType::Union:     b |=  edgeGroup->contains(edge); break;
        case gfe::EnumClass::GroupMergeType::Intersect: b &=  edgeGroup->contains(edge); break;
        case gfe::EnumClass::GroupMergeType::Subtract:  b &= !edgeGroup->contains(edge); break;
        // case gfe::EnumClass::GroupMergeType::Replace:
        //default: break;
        }
        if (b)
            edgeGroup->add(edge);
        else
            edgeGroup->remove(edge);
    }
    
    
public:
    
    gfe::EnumClass::GroupMergeType groupMergeType = gfe::EnumClass::GroupMergeType::Replace;
    bool reverseGroup = false;

    
private:
    GA_ElementGroup* elementGroup = nullptr;
    GA_EdgeGroup* edgeGroup = nullptr;
    
}; // End of Class GFE_GroupSetter

















namespace Group {

    



    static GA_Offset getFirstElement(const GA_ElementGroup& group)
    {
        GA_Offset start, end;
        for (GA_Iterator it(GA_Range(group.getIndexMap(), &group)); it.blockAdvance(start, end); )
        {
            //const GA_Offset baseOff = start - GAgetPageOff(start);
            for (GA_Offset elemoff = start; elemoff < end; ++elemoff)
            {
                if (group.contains(elemoff))
                    return elemoff;
            }
        }
        return gfe::Core::INVALID_OFFSET;
    }



    
SYS_FORCE_INLINE static GA_AttributeOwner getOwner(const GA_Group& group)
{ return gfe::Type::attributeOwner_groupType(group.classType()); }

SYS_FORCE_INLINE static GA_AttributeOwner getOwner(const GA_Group* const group)
{ return group ? getOwner(*group) : GA_ATTRIB_INVALID; }

SYS_FORCE_INLINE static void groupBumpDataId(GA_Group& group)
{
    if (group.isElementGroup())
        static_cast<GA_ElementGroup&>(group).bumpDataId();
    else
        static_cast<GA_EdgeGroup&>(group).bumpDataId();
}

SYS_FORCE_INLINE static void groupBumpDataId(GA_Group* group)
{ groupBumpDataId(*group); }


static void groupBumpDataId(GA_GroupTable& groupTable, const char* groupPattern)
{
    if (!groupPattern || groupPattern == "")
        return;
    for (GA_GroupTable::iterator<GA_Group> it = groupTable.beginTraverse(); !it.atEnd(); ++it)
    {
        GA_Group& group = *it.group();
        //if (group->isDetached())
        //    continue;
        if (!group.getName().multiMatch(groupPattern))
            continue;
        groupBumpDataId(group);
    }
}







#if 1
static void groupToggle(GA_EdgeGroup& group)
{
    group.toggle();
    for (GA_EdgeGroup::iterator it = group.begin(); !it.atEnd(); ++it)
    {
        //const GA_Edge& edge = *it;
        const auto& edge = *it;
        if (edge.p0() == edge.p1())
            group.remove(edge);
    }
}
#else
static void groupToggle(GA_EdgeGroup& group)
{
    group.toggle();
    GA_Offset start, end;
    for (GA_Iterator it(group.getDetail().getPointRange()); it.fullBlockAdvance(start, end); )
    {
        for (GA_Offset elemoff = start; elemoff < end; ++elemoff)
        {
            group.remove(GA_Edge(elemoff, elemoff));
        }
    }
}
#endif
    
SYS_FORCE_INLINE static void groupToggle(GA_ElementGroup& group)
{ group.toggleAll(group.getIndexMap().indexSize()); }

SYS_FORCE_INLINE static void groupToggle(GA_Group& group)
{
    return group.isElementGroup() ?
        groupToggle(static_cast<GA_ElementGroup&>(group)) :
        groupToggle(static_cast<GA_EdgeGroup&>(group));
}


void groupToggle(const GA_GroupTable& groupTable, const char* groupName)
{
    for (GA_GroupTable::iterator<GA_Group> it = groupTable.beginTraverse(); !it.atEnd(); ++it)
    {
        GA_Group& groupPtr = *it.group();
        if (!groupPtr.getName().multiMatch(groupName))
            continue;
        groupToggle(groupPtr);
    }
}






void delStdGroup(GA_GroupTable& groupTable, const char* groupName)
{
    for (GA_GroupTable::iterator<GA_Group> it = groupTable.beginTraverse(); !it.atEnd(); ++it)
    {
        GA_Group& groupPtr = *it.group();
        if (!groupPtr.getName().multiMatch(groupName))
            continue;
        groupTable.destroy(&groupPtr);
    }
}

static void keepStdGroup(GA_GroupTable& groupTable, const char* pattern)
{
    if (!pattern || pattern == "*")
        return;
    for (GA_GroupTable::iterator<GA_Group> it = groupTable.beginTraverse(); !it.atEnd(); ++it)
    {
        GA_Group* const group = it.group();
        //if (group->isDetached())
        //    continue;
        if (group->getName().multiMatch(pattern))
            continue;
        groupTable.destroy(group);
    }
}




SYS_FORCE_INLINE static bool groupIsEmpty(const GA_Group& group)
{
    return group.classType()==GA_GROUP_EDGE ?
           static_cast<const GA_EdgeGroup&>(group).isEmpty() :
           static_cast<const GA_ElementGroup&>(group).isEmpty();
}








//
// SYS_FORCE_INLINE static void groupBumpDataId(
//     GA_Detail& geo, const GA_GroupType groupType, const UT_StringRef& groupPattern
//     )
// { return groupBumpDataId(*geo.getGroupTable(groupType), groupPattern); }


//
// SYS_FORCE_INLINE
// static void
// delStdGroup(
//     GA_Detail& geo,
//     const GA_GroupType groupType,
//     const UT_StringRef& groupPattern
// )
// { return delStdGroup(*geo.getGroupTable(groupType), groupPattern); }
//
//
// static void
// delStdGroup(
//     GA_Detail& geo,
//     const UT_StringRef& primGroupPattern,
//     const UT_StringRef& pointGroupPattern,
//     const UT_StringRef& vertexGroupPattern,
//     const UT_StringRef& edgeGroupPattern
// )
// {
//     delStdGroup(geo, GA_GROUP_PRIMITIVE, primGroupPattern);
//     delStdGroup(geo, GA_GROUP_POINT,     pointGroupPattern);
//     delStdGroup(geo, GA_GROUP_VERTEX,    vertexGroupPattern);
//     delStdGroup(geo, GA_GROUP_EDGE,      edgeGroupPattern);
// }



//
//
// static void keepStdGroup(GA_GroupTable& groupTable, const UT_StringRef& keepGroupPattern)
// {
//     if (keepGroupPattern == "*")
//         return;
//     for (GA_GroupTable::iterator<GA_Group> it = groupTable.beginTraverse(); !it.atEnd(); ++it)
//     {
//         GA_Group* const group = it.group();
//         //if (group->isDetached())
//         //    continue;
//         if (group->getName().multiMatch(keepGroupPattern))
//             continue;
//         groupTable.destroy(group);
//     }
// }


//
// SYS_FORCE_INLINE
// static void
// keepStdGroup(
//     GA_Detail& geo,
//     const GA_GroupType groupTable,
//     const UT_StringRef& keepGroupPattern
// )
// {
//     return keepStdGroup(*geo.getGroupTable(groupTable), keepGroupPattern);
// }
//
//
// static void
// keepStdGroup(
//     GA_Detail& geo,
//     const UT_StringRef& primGroupPattern,
//     const UT_StringRef& pointGroupPattern,
//     const UT_StringRef& vertexGroupPattern,
//     const UT_StringRef& edgeGroupPattern
// )
// {
//     keepStdGroup(geo, GA_GROUP_PRIMITIVE, primGroupPattern);
//     keepStdGroup(geo, GA_GROUP_POINT,     pointGroupPattern);
//     keepStdGroup(geo, GA_GROUP_VERTEX,    vertexGroupPattern);
//     keepStdGroup(geo, GA_GROUP_EDGE,      edgeGroupPattern);
// }
//




//
// static GA_Group&
// groupDuplicate(
//     GA_Detail& geo,
//     const GA_Group& group,
//     const UT_StringRef& groupName
// )
// {
//     GA_Group& newGroup = *geo.getGroupTable(group.classType())->newGroup(groupName);
//     if (group.isElementGroup())
//         static_cast<GA_ElementGroup&>(newGroup).combine(&group);
//     else
//         static_cast<GA_EdgeGroup&>(newGroup).combine(&group);
//     return newGroup;
// }
//
//
// static GA_Group&
// groupDuplicate(
//     const GA_Detail& geo,
//     const GA_Group& group
// )
// {
//     GA_Group& newGroup = *geo.getGroupTable(group.classType())->newDetachedGroup();
//     if (group.isElementGroup())
//         static_cast<GA_ElementGroup&>(newGroup).combine(&group);
//     else
//         static_cast<GA_EdgeGroup&>(newGroup).combine(&group);
//     return newGroup;
// }
//
//
//
// SYS_FORCE_INLINE static GA_Group& groupDuplicateDetached(const GA_Detail& geo,const GA_Group& group)
// { return groupDuplicate(geo, group); }
//
//
//
//
//

//
// SYS_FORCE_INLINE static void elementGroupToggle(GA_ElementGroup& group)
// { groupToggle(group); }
//
// SYS_FORCE_INLINE static void elementGroupToggle(GA_Group& group)
// { elementGroupToggle(static_cast<GA_ElementGroup&>(group)); }
//
// static void groupToggle(GA_Group& group)
// {
//     if (group.isElementGroup())
//     {
//         return elementGroupToggle(group);
//     }
//     else
//     {
//         return edgeGroupToggle(static_cast<GA_EdgeGroup&>(group));
//     }
// }
//
// SYS_FORCE_INLINE static void groupToggle(const GA_Detail& geo, GA_Group& group)
// {
//     if (group.isElementGroup())
//     {
//         return elementGroupToggle(group);
//     }
//     else
//     {
//         return edgeGroupToggle(geo, static_cast<GA_EdgeGroup&>(group));
//     }
// }
//
//
// static void
// groupToggle(
//     const GA_Detail& geo,
//     const GA_GroupType groupType,
//     const UT_StringRef& groupName
// )
// {
//     for (GA_GroupTable::iterator<GA_Group> it = geo.getGroupTable(groupType)->beginTraverse(); !it.atEnd(); ++it)
//     {
//         GA_Group& groupPtr = *it.group();
//         //if (groupPtr->isDetached())
//         //    continue;
//         if (!groupPtr.getName().multiMatch(groupName))
//             continue;
//         groupToggle(geo, groupPtr);
//         groupBumpDataId(groupPtr);
//     }
// }
//














//
//
// SYS_FORCE_INLINE static GA_Group* newGroup(
//         GA_Detail& geo,
//         const GA_Group& group,
//         const UT_StringRef& groupName
//     )
// {
//     return geo.getGroupTable(group.classType())->newGroup(groupName);
// }
//
//
// SYS_FORCE_INLINE
//     static GA_Group*
//     newDetachedGroup(
//         const GA_Detail& geo,
//         const GA_Group& group
//     )
// {
//     return geo.getGroupTable(group.classType())->newDetachedGroup();
// }
//
//
//
//
//
// static GA_Group*
// findGroupBase(
//     const GA_Detail& geo,
//     const GA_GroupType groupType,
//     const UT_StringRef& groupName
// )
// {
//     const GA_GroupTable* const groupTable = geo.getGroupTable(groupType);
//     if (!groupTable)
//         return nullptr;
//     return groupTable->find(groupName);
// }
//
// static GA_Group*
// findGroupN(
//     const GA_Detail& geo,
//     const UT_StringRef& groupName
// )
// {
//     GA_Group* outGroup = findGroupBase(geo, GA_GROUP_PRIMITIVE, groupName);
//     if (outGroup)
//         return outGroup;
//     outGroup = findGroupBase(geo, GA_GROUP_POINT, groupName);
//     if (outGroup)
//         return outGroup;
//     outGroup = findGroupBase(geo, GA_GROUP_VERTEX, groupName);
//     if (outGroup)
//         return outGroup;
//     return findGroupBase(geo, GA_GROUP_EDGE, groupName);
// }
//
// static GA_Group*
// findGroup(
//     const GA_Detail& geo,
//     const GA_GroupType groupType,
//     const UT_StringRef& groupName
// )
// {
//     if (groupType == GA_GROUP_N)
//     {
//         return findGroupN(geo, groupName);
//     }
//     else
//     {
//         return findGroupBase(geo, groupType, groupName);
//     }
// }


//
//
// static GA_Group*
// findOrCreateGroup(
//     GA_Detail& geo,
//     const GA_GroupType groupType,
//     const UT_StringRef& groupName
// )
// {
//     GA_GroupTable* const groupTable = geo.getGroupTable(groupType);
//     if (!groupTable)
//         return nullptr;
//     GA_Group* const group = groupTable->find(groupName);
//     if (!group)
//         return groupTable->newGroup(groupName);
//     return nullptr;
// }




// SYS_FORCE_INLINE
// static GA_ElementGroup*
// findElementGroup(
//     GA_Detail& geo,
//     const GA_GroupType groupType,
//     const UT_StringRef& groupName
// )
// {
//     UT_ASSERT_P(groupType != GA_GROUP_EDGE);
//     return geo.findElementGroup(gfe::Type::attributeOwner_groupType(groupType), groupName);
// }

// SYS_FORCE_INLINE static const GA_ElementGroup*
// findElementGroup(
//     const GA_Detail& geo,
//     const GA_GroupType groupType,
//     const UT_StringRef& groupName
// )
// {
//     UT_ASSERT_P(groupType != GA_GROUP_EDGE);
//     return geo.findElementGroup(gfe::Type::attributeOwner_groupType(groupType), groupName);
// }
// 
// 
// 
// SYS_FORCE_INLINE bool groupRename(GA_Detail& geo, const GA_Group& group, const UT_StringRef& newName)
// {
//     if (group.getName() == newName)
//         return false;
//     return geo.getGroupTable(group.classType())->renameGroup(group.getName(), newName);
// }


    
//SYS_FORCE_INLINE
//static void
//groupDestroy(
//    GA_Detail& geo,
//    GA_Group& group
//)
//{
//    if (!group)
//        return;
//    geo.destroyGroup(&group);
//    group = nullptr;
//    //geo.getGroupTable(group.classType())->destroy(&group);
//}









//
// SYS_FORCE_INLINE static GA_Range getRangeByAnyGroup(const GA_Detail& geo, const GA_ElementGroup* const group)
// {
//     if (!group)
//         return GA_Range();
//
//     const GA_AttributeOwner attribOwner = gfe::Type::attributeOwner_groupType(group->classType());
//     return GA_Range(geo.getIndexMap(attribOwner), group);
// }
//
// SYS_FORCE_INLINE static GA_Range getRangeByAnyGroup(const GA_Detail& geo, const GA_Group* const group)
// { return getRangeByAnyGroup(geo, static_cast<const GA_ElementGroup*>(group)); }
//
// SYS_FORCE_INLINE static GA_SplittableRange getSplittableRangeByAnyGroup(const GA_Detail& geo, const GA_ElementGroup* const group)
// { return GA_SplittableRange(getRangeByAnyGroup(geo, group)); }
//
// SYS_FORCE_INLINE static GA_SplittableRange getSplittableRangeByAnyGroup(const GA_Detail& geo, const GA_Group* const group)
// { return getSplittableRangeByAnyGroup(geo, static_cast<const GA_ElementGroup*>(group)); }




} // End of namespace Group
_GFE_END
#endif
