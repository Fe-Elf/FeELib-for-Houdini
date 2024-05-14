
#pragma once

#ifndef __GFE_GroupPromote_h__
#define __GFE_GroupPromote_h__

#include "GFE/GFE_GroupPromote.h"

#include "GA/GA_Detail.h"



#include "GFE/GFE_Group.h"
#include "GFE/GFE_GroupUnion.h"

namespace GFE_GroupPromote {

    



    static const GA_Group*
        groupPromote(
            GA_Detail& geo,
            const GA_Group* const group,
            const GA_GroupType newType
        )
    {
        if (!group)
            return nullptr;

        if (group->classType() == newType)
            return group;

        GA_GroupTable* const groupTable = geo.getGroupTable(newType);
        if (!groupTable)
            return nullptr;

        GA_Group* const newGroup = groupTable->newGroup(group->getName());
        GFE_GroupUnion::groupUnion(*newGroup, group);

        return newGroup;
    }

    static GA_Group*
        groupPromote(
            GA_Detail& geo,
            GA_Group* const group,
            const GA_GroupType newType
        )
    {
        if (!group)
            return nullptr;

        if (group->classType() == newType)
            return group;

        GA_GroupTable* const groupTable = geo.getGroupTable(newType);
        if (!groupTable)
            return nullptr;

        GA_Group* const newGroup = groupTable->newGroup(group->getName());
        GFE_GroupUnion::groupUnion(*newGroup, group);

        return newGroup;
    }

    static const GA_Group*
        groupPromote(
            GA_Detail& geo,
            const GA_Group* const group,
            const GA_GroupType newType,
            const UT_StringHolder& newName
        )
    {
        if (!group)
            return nullptr;

        if (group->classType() == newType)
        {
            //GFE_Group::groupRename(geo, *group, newName);
            geo.getGroupTable(group->classType())->renameGroup(group->getName(), newName);
            return group;
        }

        GA_GroupTable* const groupTable = geo.getGroupTable(newType);
        if (!groupTable)
            return nullptr;

        GA_Group* const newGroup = groupTable->newGroup(newName);
        GFE_GroupUnion::groupUnion(*newGroup, group);

        return newGroup;
    }

    static GA_Group*
        groupPromote(
            GA_Detail& geo,
            GA_Group* const group,
            const GA_GroupType newType,
            const UT_StringHolder& newName
        )
    {
        if (!group)
            return nullptr;

        if (group->classType() == newType)
        {
            //GFE_Group::groupRename(geo, *group, newName);
            geo.getGroupTable(group->classType())->renameGroup(group->getName(), newName);
            return group;
        }

        GA_GroupTable* const groupTable = geo.getGroupTable(newType);
        if (!groupTable)
            return nullptr;

        GA_Group* const newGroup = groupTable->newGroup(newName);
        GFE_GroupUnion::groupUnion(*newGroup, group);

        return newGroup;
    }




    static GA_GroupUPtr
    groupPromoteDetached(
        const GA_Group* const group,
        const GA_GroupType newType
    )
    {
        if (!group)
            return GA_GroupUPtr();

        const GA_GroupTable* const groupTable = group->getDetail().getGroupTable(newType);
        if (!groupTable)
            return GA_GroupUPtr();
        
        GA_Group* const newGroup = groupTable->newDetachedGroup();
        
        GFE_GroupUnion::groupUnion(*newGroup, group);

        return GA_GroupUPtr(newGroup);
    }

    //static GA_GroupUPtr
    //    groupPromoteDetached(
    //        const GA_Detail& geo,
    //        const GA_Group* const group,
    //        const GA_GroupType newType
    //    )
    //{
    //    if (!group)
    //        return GA_GroupUPtr();

    //    const GA_GroupTable* const groupTable = geo.getGroupTable(newType);
    //    if (!groupTable)
    //        return GA_GroupUPtr();

    //    GA_Group* const newGroup = groupTable->newDetachedGroup();

    //    GFE_GroupUnion::groupUnion(newGroup, group);

    //    return GA_GroupUPtr(newGroup);
    //}

    static GA_Group*
        groupFindPromoteDetached(
            const GA_Detail& geo,
            GA_Group* const group,
            const GA_GroupType newType
        )
    {
        if (!group)
            return nullptr;

        if (group->classType() == newType)
            return group;

        const GA_GroupTable* const groupTable = geo.getGroupTable(newType);
        if (!groupTable)
            return nullptr;

        GA_Group* const newGroup = groupTable->newDetachedGroup();
        GFE_GroupUnion::groupUnion(*newGroup, group);

        return newGroup;
    }

    static GA_Group*
        groupFindPromoteDetached(
            GA_Group* const group,
            const GA_GroupType newType
        )
    {
        if (!group)
            return nullptr;

        if (group->classType() == newType)
            return group;

        const GA_GroupTable* const groupTable = group->getDetail().getGroupTable(newType);
        if (!groupTable)
            return nullptr;

        GA_Group* const newGroup = groupTable->newDetachedGroup();
        GFE_GroupUnion::groupUnion(*newGroup, group);

        return newGroup;
    }

    static const GA_Group*
        groupFindPromoteDetached(
            const GA_Detail& geo,
            const GA_Group* const group,
            const GA_GroupType newType
        )
    {
        if (!group)
            return nullptr;

        if (group->classType() == newType)
            return group;

        const GA_GroupTable* const groupTable = geo.getGroupTable(newType);
        if (!groupTable)
            return nullptr;

        GA_Group* const newGroup = groupTable->newDetachedGroup();
        GFE_GroupUnion::groupUnion(*newGroup, group);

        return newGroup;
    }

    static const GA_Group*
        groupFindPromoteDetached(
            const GA_Group* const group,
            const GA_GroupType newType
        )
    {
        if (!group)
            return nullptr;

        if (group->classType() == newType)
            return group;

        const GA_GroupTable* const groupTable = group->getDetail().getGroupTable(newType);
        if (!groupTable)
            return nullptr;

        GA_Group* const newGroup = groupTable->newDetachedGroup();
        GFE_GroupUnion::groupUnion(*newGroup, group);

        return newGroup;
    }




    SYS_FORCE_INLINE
        static const GA_Group*
        groupPromote(
            GA_Detail& geo,
            const GA_Group* const group,
            const GA_AttributeOwner newType,
            const UT_StringHolder& newName
        )
    {
        return groupPromote(geo, group, GFE_Type::attributeOwner_groupType(newType), newName);
    }

    SYS_FORCE_INLINE
        static GA_Group*
        groupPromote(
            GA_Detail& geo,
            GA_Group* const group,
            const GA_AttributeOwner newType,
            const UT_StringHolder& newName
        )
    {
        return groupPromote(geo, group, GFE_Type::attributeOwner_groupType(newType), newName);
    }

    SYS_FORCE_INLINE
        static GA_GroupUPtr
        groupPromoteDetached(
            const GA_Group* const group,
            const GA_AttributeOwner newType
        )
    {
        return groupPromoteDetached(group, GFE_Type::attributeOwner_groupType(newType));
    }

    SYS_FORCE_INLINE
        static const GA_Group*
        groupFindPromoteDetached(
            const GA_Group* const group,
            const GA_AttributeOwner newType
        )
    {
        return groupFindPromoteDetached(group, GFE_Type::attributeOwner_groupType(newType));
    }




    SYS_FORCE_INLINE
    static GA_Group*
    groupPromote(
        GA_Detail& geo,
        GA_Group*& group,
        const GA_GroupType newType,
        const UT_StringHolder& newName,
        const bool delOriginal
    )
    {
        GA_Group* newGroup = groupPromote(geo, group, newType, newName);
        if (delOriginal && group != newGroup)
        {
            geo.destroyGroup(group);
            group = nullptr;
        }
        return newGroup;
    }

    SYS_FORCE_INLINE
        static GA_Group*
        groupPromote(
            GA_Detail& geo,
            GA_PrimitiveGroup*& group,
            const GA_GroupType newType,
            const UT_StringHolder& newName,
            const bool delOriginal
        )
    {
        GA_Group* newGroup = groupPromote(geo, group, newType, newName);
        if (delOriginal && group != newGroup)
        {
            geo.destroyGroup(group);
            group = nullptr;
        }
        return newGroup;
    }


    SYS_FORCE_INLINE
        static GA_Group*
        groupPromote(
            GA_Detail& geo,
            GA_PointGroup*& group,
            const GA_GroupType newType,
            const UT_StringHolder& newName,
            const bool delOriginal
        )
    {
        GA_Group* newGroup = groupPromote(geo, group, newType, newName);
        if (delOriginal && group != newGroup)
        {
            geo.destroyGroup(group);
            group = nullptr;
        }
        return newGroup;
    }


    SYS_FORCE_INLINE
        static GA_Group*
        groupPromote(
            GA_Detail& geo,
            GA_VertexGroup*& group,
            const GA_GroupType newType,
            const UT_StringHolder& newName,
            const bool delOriginal
        )
    {
        GA_Group* newGroup = groupPromote(geo, group, newType, newName);
        if (delOriginal && group != newGroup)
        {
            geo.destroyGroup(group);
            group = nullptr;
        }
        return newGroup;
    }


    SYS_FORCE_INLINE
        static GA_Group*
        groupPromote(
            GA_Detail& geo,
            GA_EdgeGroup*& group,
            const GA_GroupType newType,
            const UT_StringHolder& newName,
            const bool delOriginal
        )
    {
        GA_Group* newGroup = groupPromote(geo, group, newType, newName);
        if (delOriginal && group != newGroup)
        {
            geo.destroyGroup(group);
            group = nullptr;
        }
        return newGroup;
    }

    SYS_FORCE_INLINE
    static GA_GroupUPtr
    groupPromoteDetached(
        GA_Detail& geo,
        GA_Group*& group,
        const GA_GroupType newType,
        const bool delOriginal
    )
    {
        GA_GroupUPtr newGroupUPtr = groupPromoteDetached(group, newType);
        if (delOriginal && group != newGroupUPtr.get())
        {
            geo.destroyGroup(group);
            group = nullptr;
        }
        return newGroupUPtr;
    }

    SYS_FORCE_INLINE
    static const GA_Group*
    groupFindPromoteDetached(
        GA_Detail& geo,
        GA_Group*& group,
        const GA_GroupType newType,
        const bool delOriginal
    )
    {
        const GA_Group* const newGroup = groupFindPromoteDetached(geo, group, newType);
        if (delOriginal && group != newGroup)
        {
            geo.destroyGroup(group);
            group = nullptr;
        }
        return newGroup;
    }





    SYS_FORCE_INLINE
        static const GA_PrimitiveGroup*
        groupPromotePrimitive(
            GA_Detail& geo,
            const GA_Group* const group,
            const UT_StringHolder& newName
        )
    {
        return static_cast<const GA_PrimitiveGroup*>(groupPromote(geo, group, GA_GROUP_PRIMITIVE, newName));
    }

    SYS_FORCE_INLINE
        static const GA_PointGroup*
        groupPromotePoint(
            GA_Detail& geo,
            const GA_Group* const group,
            const UT_StringHolder& newName
        )
    {
        return static_cast<const GA_PointGroup*>(groupPromote(geo, group, GA_GROUP_POINT, newName));
    }

    SYS_FORCE_INLINE
        static const GA_VertexGroup*
        groupPromoteVertex(
            GA_Detail& geo,
            const GA_Group* const group,
            const UT_StringHolder& newName
        )
    {
        return static_cast<const GA_VertexGroup*>(groupPromote(geo, group, GA_GROUP_VERTEX, newName));
    }

    SYS_FORCE_INLINE
        static const GA_EdgeGroup*
        groupPromoteEdge(
            GA_Detail& geo,
            const GA_Group* const group,
            const UT_StringHolder& newName
        )
    {
        return static_cast<const GA_EdgeGroup*>(groupPromote(geo, group, GA_GROUP_EDGE, newName));
    }






    SYS_FORCE_INLINE
        static GA_PrimitiveGroup*
        groupPromotePrimitive(
            GA_Detail& geo,
            GA_Group* const group,
            const UT_StringHolder& newName
        )
    {
        return static_cast<GA_PrimitiveGroup*>(groupPromote(geo, group, GA_GROUP_PRIMITIVE, newName));
    }

    SYS_FORCE_INLINE
        static GA_PointGroup*
        groupPromotePoint(
            GA_Detail& geo,
            GA_Group* const group,
            const UT_StringHolder& newName
        )
    {
        return static_cast<GA_PointGroup*>(groupPromote(geo, group, GA_GROUP_POINT, newName));
    }

    SYS_FORCE_INLINE
        static GA_VertexGroup*
        groupPromoteVertex(
            GA_Detail& geo,
            GA_Group* const group,
            const UT_StringHolder& newName
        )
    {
        return static_cast<GA_VertexGroup*>(groupPromote(geo, group, GA_GROUP_VERTEX, newName));
    }

    SYS_FORCE_INLINE
        static GA_EdgeGroup*
        groupPromoteEdge(
            GA_Detail& geo,
            GA_Group* const group,
            const UT_StringHolder& newName
        )
    {
        return static_cast<GA_EdgeGroup*>(groupPromote(geo, group, GA_GROUP_EDGE, newName));
    }







    SYS_FORCE_INLINE
        static const GA_PrimitiveGroup*
        groupPromotePrimitive(
            GA_Detail& geo,
            const GA_Group* const group
        )
    {
        return static_cast<const GA_PrimitiveGroup*>(groupPromote(geo, group, GA_GROUP_PRIMITIVE));
    }

    SYS_FORCE_INLINE
        static const GA_PointGroup*
        groupPromotePoint(
            GA_Detail& geo,
            const GA_Group* const group
        )
    {
        return static_cast<const GA_PointGroup*>(groupPromote(geo, group, GA_GROUP_POINT));
    }

    SYS_FORCE_INLINE
        static const GA_VertexGroup*
        groupPromoteVertex(
            GA_Detail& geo,
            const GA_Group* const group
        )
    {
        return static_cast<const GA_VertexGroup*>(groupPromote(geo, group, GA_GROUP_VERTEX));
    }

    SYS_FORCE_INLINE
        static const GA_EdgeGroup*
        groupPromoteEdge(
            GA_Detail& geo,
            const GA_Group* const group
        )
    {
        return static_cast<const GA_EdgeGroup*>(groupPromote(geo, group, GA_GROUP_EDGE));
    }





    SYS_FORCE_INLINE
        static GA_PrimitiveGroup*
        groupPromotePrimitive(
            GA_Detail& geo,
            GA_Group* const group
        )
    {
        return static_cast<GA_PrimitiveGroup*>(groupPromote(geo, group, GA_GROUP_PRIMITIVE));
    }

    SYS_FORCE_INLINE
        static GA_PointGroup*
        groupPromotePoint(
            GA_Detail& geo,
            GA_Group* const group
        )
    {
        return static_cast<GA_PointGroup*>(groupPromote(geo, group, GA_GROUP_POINT));
    }

    SYS_FORCE_INLINE
        static GA_VertexGroup*
        groupPromoteVertex(
            GA_Detail& geo,
            GA_Group* const group
        )
    {
        return static_cast<GA_VertexGroup*>(groupPromote(geo, group, GA_GROUP_VERTEX));
    }

    SYS_FORCE_INLINE
        static GA_EdgeGroup*
        groupPromoteEdge(
            GA_Detail& geo,
            GA_Group* const group
        )
    {
        return static_cast<GA_EdgeGroup*>(groupPromote(geo, group, GA_GROUP_EDGE));
    }





    SYS_FORCE_INLINE
        static const GA_PrimitiveGroup*
        groupPromotePrimitive(
            GA_Detail& geo,
            GA_Group*& group,
            const UT_StringHolder& newName,
            const bool delOriginal
        )
    {
        return static_cast<const GA_PrimitiveGroup*>(groupPromote(geo, group, GA_GROUP_PRIMITIVE, newName, delOriginal));
    }

    SYS_FORCE_INLINE
        static const GA_PointGroup*
        groupPromotePoint(
            GA_Detail& geo,
            GA_Group*& group,
            const UT_StringHolder& newName,
            const bool delOriginal
        )
    {
        return static_cast<const GA_PointGroup*>(groupPromote(geo, group, GA_GROUP_POINT, newName, delOriginal));
    }

    SYS_FORCE_INLINE
        static const GA_VertexGroup*
        groupPromoteVertex(
            GA_Detail& geo,
            GA_Group*& group,
            const UT_StringHolder& newName,
            const bool delOriginal
        )
    {
        return static_cast<const GA_VertexGroup*>(groupPromote(geo, group, GA_GROUP_VERTEX, newName, delOriginal));
    }

    SYS_FORCE_INLINE
        static const GA_EdgeGroup*
        groupPromoteEdge(
            GA_Detail& geo,
            GA_Group*& group,
            const UT_StringHolder& newName,
            const bool delOriginal
        )
    {
        return static_cast<const GA_EdgeGroup*>(groupPromote(geo, group, GA_GROUP_EDGE, newName, delOriginal));
    }















    static GA_PrimitiveGroupUPtr
        groupPromotePrimitiveDetached(
            const GA_Group* const group
        )
    {
        if (!group)
            return GA_PrimitiveGroupUPtr();

        const GA_GroupTable* const groupTable = group->getDetail().getGroupTable(GA_GROUP_PRIMITIVE);
        if (!groupTable)
            return GA_PrimitiveGroupUPtr();

        GA_Group* const newGroup = groupTable->newDetachedGroup();

        GFE_GroupUnion::groupUnion(*newGroup, group);

        return GA_PrimitiveGroupUPtr(static_cast<GA_PrimitiveGroup*>(newGroup));
    }

    static GA_PointGroupUPtr groupPromotePointDetached(const GA_Group* const group)
    {
        if (!group)
            return GA_PointGroupUPtr();

        const GA_GroupTable* const groupTable = group->getDetail().getGroupTable(GA_GROUP_POINT);
        if (!groupTable)
            return GA_PointGroupUPtr();

        GA_Group* const newGroup = groupTable->newDetachedGroup();

        GFE_GroupUnion::groupUnion(*newGroup, group);

        return GA_PointGroupUPtr(static_cast<GA_PointGroup*>(newGroup));
    }

    static GA_VertexGroupUPtr
        groupPromoteVertexDetached(
            const GA_Group* const group
        )
    {
        if (!group)
            return GA_VertexGroupUPtr();

        const GA_GroupTable* const groupTable = group->getDetail().getGroupTable(GA_GROUP_VERTEX);
        if (!groupTable)
            return GA_VertexGroupUPtr();

        GA_Group* const newGroup = groupTable->newDetachedGroup();

        GFE_GroupUnion::groupUnion(*newGroup, group);

        return GA_VertexGroupUPtr(static_cast<GA_VertexGroup*>(newGroup));
    }

    static GA_EdgeGroupUPtr
        groupPromoteEdgeDetached(
            const GA_Group* const group
        )
    {
        if (!group)
            return GA_EdgeGroupUPtr();

        const GA_GroupTable* const groupTable = group->getDetail().getGroupTable(GA_GROUP_EDGE);
        if (!groupTable)
            return GA_EdgeGroupUPtr();

        GA_Group* const newGroup = groupTable->newDetachedGroup();

        GFE_GroupUnion::groupUnion(*newGroup, group);

        return GA_EdgeGroupUPtr(static_cast<GA_EdgeGroup*>(newGroup));
    }




    static const GA_Group*
        groupFindPromoteDetached(
            const GA_Group* const group,
            const GA_GroupType groupType,
            GOP_Manager& gop
        )
    {
        if (!group)
            return nullptr;

        if (group->classType() == groupType)
            return group;

        const GA_GroupTable* const groupTable = group->getDetail().getGroupTable(groupType);
        if (!groupTable)
            return group;

        GA_Group* const newGroup = groupTable->newDetachedGroup();

        GFE_GroupUnion::groupUnion(*newGroup, group);

        gop.appendAdhocGroup(newGroup, false);

        return newGroup;
    }

    SYS_FORCE_INLINE
        static const GA_PrimitiveGroup*
        groupFindPromotePrimitiveDetached(
            const GA_Group* const group,
            GOP_Manager& gop
        )
    {
        return static_cast<const GA_PrimitiveGroup*>(groupFindPromoteDetached(group, GA_GROUP_PRIMITIVE, gop));
    }

    SYS_FORCE_INLINE
        static const GA_PointGroup*
        groupFindPromotePointDetached(
            const GA_Group* const group,
            GOP_Manager& gop
        )
    {
        return static_cast<const GA_PointGroup*>(groupFindPromoteDetached(group, GA_GROUP_POINT, gop));
    }

    SYS_FORCE_INLINE
        static const GA_VertexGroup*
        groupFindPromoteVertexDetached(
            const GA_Group* const group,
            GOP_Manager& gop
        )
    {
        return static_cast<const GA_VertexGroup*>(groupFindPromoteDetached(group, GA_GROUP_VERTEX, gop));
    }

    SYS_FORCE_INLINE
        static const GA_EdgeGroup*
        groupFindPromoteEdgeDetached(
            const GA_Group* const group,
            GOP_Manager& gop
        )
    {
        return static_cast<const GA_EdgeGroup*>(groupFindPromoteDetached(group, GA_GROUP_EDGE, gop));
    }




    SYS_FORCE_INLINE
        static GA_PrimitiveGroupUPtr
        groupPromotePrimitiveDetached(
            GA_Detail& geo,
            GA_Group*& group,
            const bool delOriginal
        )
    {
        GA_PrimitiveGroupUPtr newGroupUPtr = groupPromotePrimitiveDetached(group);
        if (delOriginal && group != newGroupUPtr.get())
        {
            geo.destroyGroup(group);
            group = nullptr;
        }
        return newGroupUPtr;
    }

    SYS_FORCE_INLINE
        static GA_PointGroupUPtr
        groupPromotePointDetached(
            GA_Detail& geo,
            GA_Group*& group,
            const bool delOriginal
        )
    {
        GA_PointGroupUPtr newGroupUPtr = groupPromotePointDetached(group);
        if (delOriginal && group != newGroupUPtr.get())
        {
            geo.destroyGroup(group);
            group = nullptr;
        }
        return newGroupUPtr;
    }

    SYS_FORCE_INLINE
        static GA_VertexGroupUPtr
        groupPromoteVertexDetached(
            GA_Detail& geo,
            GA_Group*& group,
            const bool delOriginal
        )
    {
        GA_VertexGroupUPtr newGroupUPtr = groupPromoteVertexDetached(group);
        if (delOriginal && group != newGroupUPtr.get())
        {
            geo.destroyGroup(group);
            group = nullptr;
        }
        return newGroupUPtr;
    }

    SYS_FORCE_INLINE
    static GA_EdgeGroupUPtr
    groupPromoteEdgeDetached(
        GA_Detail& geo,
        GA_Group*& group,
        const bool delOriginal
    )
    {
        GA_EdgeGroupUPtr newGroupUPtr = groupPromoteEdgeDetached(group);
        if (delOriginal && group != newGroupUPtr.get())
        {
            geo.destroyGroup(group);
            group = nullptr;
        }
        return newGroupUPtr;
    }






    //const UT_UniquePtr<const GA_PrimitiveGroup> geo0GroupUptr(geo0PrimGroup);
    SYS_FORCE_INLINE
        static const GA_PrimitiveGroup*
        groupFindPromotePrimitiveDetached(
            const GA_Detail& geo,
            const GA_Group* const group
        )
    {
        return static_cast<const GA_PrimitiveGroup*>(groupFindPromoteDetached(geo, group, GA_GROUP_PRIMITIVE));
    }

    //const UT_UniquePtr<const GA_PointGroup> geo0GroupUptr(geo0PointGroup);
    SYS_FORCE_INLINE
        static const GA_PointGroup*
        groupFindPromotePointDetached(
            const GA_Detail& geo,
            const GA_Group* const group
        )
    {
        return static_cast<const GA_PointGroup*>(groupFindPromoteDetached(geo, group, GA_GROUP_POINT));
    }

    //const UT_UniquePtr<const GA_VertexGroup> geo0GroupUptr(geo0VtxGroup);
    SYS_FORCE_INLINE
        static const GA_VertexGroup*
        groupFindPromoteVertexDetached(
            const GA_Detail& geo,
            const GA_Group* const group
        )
    {
        return static_cast<const GA_VertexGroup*>(groupFindPromoteDetached(geo, group, GA_GROUP_VERTEX));
    }

    //const UT_UniquePtr<const GA_EdgeGroup> geo0GroupUptr(geo0EdgeGroup);
    SYS_FORCE_INLINE
        static const GA_EdgeGroup*
        groupFindPromoteEdgeDetached(
            const GA_Detail& geo,
            const GA_Group* const group
        )
    {
        return static_cast<const GA_EdgeGroup*>(groupFindPromoteDetached(geo, group, GA_GROUP_EDGE));
    }













    SYS_FORCE_INLINE
        static const GA_ElementGroup*
        elementGroupFindPromoteDetached(
            const GA_Detail& geo,
            const GA_Group* const group,
            const GA_GroupType newType
        )
    {
        return static_cast<const GA_ElementGroup*>(groupFindPromoteDetached(geo, group, newType));
    }

    SYS_FORCE_INLINE
        static const GA_ElementGroup*
        elementGroupFindPromotePrimitiveDetached(
            const GA_Detail& geo,
            const GA_Group* const group
        )
    {
        return elementGroupFindPromoteDetached(geo, group, GA_GROUP_PRIMITIVE);
    }

    SYS_FORCE_INLINE
        static const GA_ElementGroup*
        elementGroupFindPromotePointDetached(
            const GA_Detail& geo,
            const GA_Group* const group
        )
    {
        return elementGroupFindPromoteDetached(geo, group, GA_GROUP_POINT);
    }

    SYS_FORCE_INLINE
        static const GA_ElementGroup*
        elementGroupFindPromoteVertexDetached(
            const GA_Detail& geo,
            const GA_Group* const group
        )
    {
        return elementGroupFindPromoteDetached(geo, group, GA_GROUP_VERTEX);
    }


    SYS_FORCE_INLINE
        static const GA_ElementGroup*
        elementGroupFindPromoteDetached(
            GA_Detail& geo,
            GA_Group*& group,
            const GA_GroupType newType,
            const bool delOriginal
        )
    {
        return static_cast<const GA_ElementGroup*>(groupFindPromoteDetached(geo, group, newType, delOriginal));
    }

    SYS_FORCE_INLINE
        static const GA_ElementGroup*
        elementGroupFindPromotePrimitiveDetached(
            GA_Detail& geo,
            GA_Group*& group,
            const bool delOriginal
        )
    {
        return elementGroupFindPromoteDetached(geo, group, GA_GROUP_PRIMITIVE, delOriginal);
    }

    SYS_FORCE_INLINE
        static const GA_ElementGroup*
        elementGroupFindPromotePointDetached(
            GA_Detail& geo,
            GA_Group*& group,
            const bool delOriginal
        )
    {
        return elementGroupFindPromoteDetached(geo, group, GA_GROUP_POINT, delOriginal);
    }

    SYS_FORCE_INLINE
        static const GA_ElementGroup*
        elementGroupFindPromoteVertexDetached(
            GA_Detail& geo,
            GA_Group*& group,
            const bool delOriginal
        )
    {
        return elementGroupFindPromoteDetached(geo, group, GA_GROUP_VERTEX, delOriginal);
    }





    static GA_ElementGroupUPtr
    elementGroupPromoteDetached(
        const GA_Group* const group,
        const GA_GroupType newType
    )
    {
        if (!group)
            return GA_ElementGroupUPtr();

        const GA_GroupTable* const groupTable = group->getDetail().getGroupTable(newType);
        if (!groupTable)
            return GA_ElementGroupUPtr();

        GA_ElementGroup* const newGroup = static_cast<GA_ElementGroup*>(groupTable->newDetachedGroup());

        GFE_GroupUnion::groupUnion(*newGroup, group);

        return GA_ElementGroupUPtr(static_cast<GA_ElementGroup*>(newGroup));
    }

    SYS_FORCE_INLINE
        static GA_ElementGroupUPtr
        elementGroupPromotePrimitiveDetached(
            const GA_Group* const group
        )
    {
        return elementGroupPromoteDetached(group, GA_GROUP_PRIMITIVE);
    }

    SYS_FORCE_INLINE
        static GA_ElementGroupUPtr
        elementGroupPromotePointDetached(
            const GA_Group* const group
        )
    {
        return elementGroupPromoteDetached(group, GA_GROUP_POINT);
    }

    SYS_FORCE_INLINE
        static GA_ElementGroupUPtr
        elementGroupPromoteVertexDetached(
            const GA_Group* const group
        )
    {
        return elementGroupPromoteDetached(group, GA_GROUP_VERTEX);
    }











} // End of namespace GFE_GroupPromote

#endif
