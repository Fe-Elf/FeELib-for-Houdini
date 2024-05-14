
#pragma once

#ifndef __GFE_WindingNumber_h__
#define __GFE_WindingNumber_h__

#include "GFE/GFE_WindingNumber.h"

#include "GA/GA_Detail.h"


#include "UT/UT_SolidAngle.h"

#include <GEO/GEO_Curve.h>
#include <GEO/GEO_PrimMesh.h>
#include <GEO/GEO_PrimPolySoup.h>
#include <GEO/GEO_PrimSphere.h>
#include <GEO/GEO_PrimTetrahedron.h>
#include <GEO/GEO_TPSurf.h>
//#include <GA/GA_Handle.h>
#include <GA/GA_SplittableRange.h>
//#include <GA/GA_Types.h>
//#include <OP/OP_Operator.h>
//#include <OP/OP_OperatorTable.h>
//#include "PRM/PRM_TemplateBuilder.h"
//#include <UT/UT_DSOVersion.h>
#include <UT/UT_Interrupt.h>
#include <UT/UT_ParallelUtil.h>
#include <UT/UT_StringHolder.h>
#include <UT/UT_UniquePtr.h>
#include <SYS/SYS_Math.h>
#include <SOP/SOP_NodeVerb.h>


#include "GFE/GFE_GeoFilter.h"


#include "GFE/GFE_AttributeCopy.h"
#include "GFE/GFE_GroupUnion.h"
#include "GFE/GFE_PointGeneratePerElement.h"


enum class GFE_WNType
{
    XYZ,
    XY,
    YZ,
    ZX,
};









/// This class is for caching data between cooks, e.g. so that we don't have to
/// rebuild the UT_SolidAngle tree on every cook if the input hasn't changed.
class GFE_WindingNumber_Cache : public SOP_NodeCache {

    
public:
    GFE_WindingNumber_Cache() : SOP_NodeCache()
        , mySolidAngleTree()
        , mySubtendedAngleTree()
        , myTopologyDataID(GA_INVALID_DATAID)
        , myPrimitiveListDataID(GA_INVALID_DATAID)
        , myPDataID(GA_INVALID_DATAID)
        , myApproxOrder(-1)
        , myAxis0(-1)
        , myHadGroup(false)
        , myGroupString()
        , myUniqueId(-1)
        , myMetaCacheCount(-1)
    {}
    
    ~GFE_WindingNumber_Cache() {}



    
    void update3D(const GA_Detail& geoRefMesh, const GA_PrimitiveGroup* prim_group, const UT_StringHolder& group_string, const int approx_order)
    {
        const GA_DataId topology_data_id = geoRefMesh.getTopology().getDataId();
        const GA_DataId primitive_list_data_id = geoRefMesh.getPrimitiveList().getDataId();
        const GA_DataId P_data_id = geoRefMesh.getP()->getDataId();
        const bool has_group = (prim_group != nullptr);
        if (mySubtendedAngleTree.isClear() &&
            topology_data_id == myTopologyDataID &&
            primitive_list_data_id == myPrimitiveListDataID &&
            P_data_id == myPDataID &&
            approx_order == myApproxOrder &&
            has_group == myHadGroup &&
            (!has_group || (
                group_string == myGroupString &&
                geoRefMesh.getUniqueId() == myUniqueId &&
                geoRefMesh.getMetaCacheCount() == myMetaCacheCount)))
        {
            return;
        }
        mySubtendedAngleTree.clear();
        myPositions2D.clear();

        UT_AutoInterrupt boss("Constructing Solid Angle Tree");

        myTopologyDataID = topology_data_id;
        myPrimitiveListDataID = primitive_list_data_id;
        myPDataID = P_data_id;
        myApproxOrder = approx_order;
        myHadGroup = has_group;
        myGroupString = group_string;
        int a = 1;
        myUniqueId = geoRefMesh.getUniqueId();
        myMetaCacheCount = geoRefMesh.getMetaCacheCount();

        GA_Size nprimitives = prim_group ? prim_group->entries() : geoRefMesh.getNumPrimitives();
        myTrianglePoints.setSize(0);
        myTrianglePoints.setCapacity(3 * nprimitives);
        myPositions3D.setSize(0);
        myPositions3D.setCapacity(0);
        UT_Array<int> ptmap;
        if (!prim_group)
        {
            // Copy all point positions
            geoRefMesh.getAttributeAsArray(geoRefMesh.getP(), geoRefMesh.getPointRange(), myPositions3D);
        }
        else
        {
            // Find the used points
            GA_PointGroup mesh_ptgroup(geoRefMesh);
            mesh_ptgroup.combine(prim_group);
            myPositions3D.setSizeNoInit(mesh_ptgroup.entries());
            ptmap.setSizeNoInit(geoRefMesh.getNumPointOffsets());
            int ptnum = 0;
            geoRefMesh.forEachPoint(&mesh_ptgroup, [&geoRefMesh, this, &ptmap, &ptnum](const GA_Offset ptoff)
            {
                ptmap[ptoff] = ptnum;
                myPositions3D[ptnum] = geoRefMesh.getPos3(ptoff);
                ++ptnum;
            });
        }

        GA_Offset start, end;
        for (GA_Iterator it(geoRefMesh.getPrimitiveRange(prim_group)); it.blockAdvance(start, end); )
        {
            for (GA_Offset primoff = start; primoff < end; ++primoff)
            {
                sopAccumulateTriangles(geoRefMesh, primoff, ptmap, myTrianglePoints);
            }
        }

        mySolidAngleTree.init(myTrianglePoints.size() / 3, myTrianglePoints.array(), myPositions3D.size(), myPositions3D.array(), approx_order);
    }

    void update2D(
        const GA_Detail& geoRefMesh,
        const GA_PrimitiveGroup* prim_group,
        const UT_StringHolder& group_string,
        const int approx_order,
        const int axis0, const int axis1)
    {
        const GA_DataId topology_data_id = geoRefMesh.getTopology().getDataId();
        const GA_DataId primitive_list_data_id = geoRefMesh.getPrimitiveList().getDataId();
        const GA_DataId P_data_id = geoRefMesh.getP()->getDataId();
        const bool has_group = (prim_group != nullptr);
        if (mySolidAngleTree.isClear() &&
            topology_data_id == myTopologyDataID &&
            primitive_list_data_id == myPrimitiveListDataID &&
            P_data_id == myPDataID &&
            approx_order == myApproxOrder &&
            axis0 == myAxis0 &&
            has_group == myHadGroup &&
            (!has_group || (
                group_string == myGroupString &&
                geoRefMesh.getUniqueId() == myUniqueId &&
                geoRefMesh.getMetaCacheCount() == myMetaCacheCount)))
        {
            return;
        }
        mySolidAngleTree.clear();
        myPositions3D.clear();

        UT_AutoInterrupt boss("Constructing Solid Angle Tree");

        myTopologyDataID = topology_data_id;
        myPrimitiveListDataID = primitive_list_data_id;
        myPDataID = P_data_id;
        myApproxOrder = approx_order;
        myAxis0 = axis0;
        myHadGroup = has_group;
        myGroupString = group_string;
        myUniqueId = geoRefMesh.getUniqueId();
        myMetaCacheCount = geoRefMesh.getMetaCacheCount();

        GA_Size nprimitives = prim_group ? prim_group->entries() : geoRefMesh.getNumPrimitives();
        // NOTE: myTrianglePoints is actually segment points
        myTrianglePoints.setSize(0);
        myTrianglePoints.setCapacity(2 * nprimitives);
        myPositions2D.setSize(0);
        myPositions2D.setCapacity(0);
        UT_Array<int> ptmap;
        if (!prim_group)
        {
            // Copy all point positions
            myPositions2D.setSizeNoInit(geoRefMesh.getNumPoints());
            int ptnum = 0;
            geoRefMesh.forEachPoint([&geoRefMesh, this, &ptnum, axis0, axis1](const GA_Offset ptoff)
            {
                UT_Vector3 pos = geoRefMesh.getPos3(ptoff);
                myPositions2D[ptnum] = UT_Vector2(pos[axis0], pos[axis1]);
                ++ptnum;
            });
        }
        else
        {
            // Find the used points
            GA_PointGroup mesh_ptgroup(geoRefMesh);
            mesh_ptgroup.combine(prim_group);
            myPositions2D.setSizeNoInit(mesh_ptgroup.entries());
            ptmap.setSizeNoInit(geoRefMesh.getNumPointOffsets());
            int ptnum = 0;
            geoRefMesh.forEachPoint(&mesh_ptgroup, [&geoRefMesh, this, &ptmap, &ptnum, axis0, axis1](const GA_Offset ptoff)
            {
                ptmap[ptoff] = ptnum;
                UT_Vector3 pos = geoRefMesh.getPos3(ptoff);
                myPositions2D[ptnum] = UT_Vector2(pos[axis0], pos[axis1]);
                ++ptnum;
            });
        }

        GA_Offset start, end;
        for (GA_Iterator it(geoRefMesh.getPrimitiveRange(prim_group)); it.blockAdvance(start, end); )
        {
            for (GA_Offset primoff = start; primoff < end; ++primoff)
            {
                sopAccumulateSegments(geoRefMesh, primoff, ptmap, myTrianglePoints);
            }
        }

        mySubtendedAngleTree.init(myTrianglePoints.size() / 2, myTrianglePoints.array(), myPositions2D.size(), myPositions2D.array(), approx_order);
    }

    void clear()
    {
        mySolidAngleTree.clear();
        mySubtendedAngleTree.clear();
        myTrianglePoints.setCapacity(0);
        myPositions2D.setCapacity(0);
        myPositions3D.setCapacity(0);
        myTopologyDataID = GA_INVALID_DATAID;
        myPrimitiveListDataID = GA_INVALID_DATAID;
        myPDataID = GA_INVALID_DATAID;
        myApproxOrder = -1;
        myHadGroup = false;
        myGroupString.clear();
        myUniqueId = -1;
        myMetaCacheCount = -1;
    }



    
private:

    //
    // Help is stored in a "wiki" style text file.  This text file should be copied
    // to $HOUDINI_PATH/help/nodes/sop/hdk_windingnumber.txt
    //
    // See the sample_install.sh file for an example.
    //

    // Forward declarations of file-scope functions used before they're defined


    void sopAccumulateTriangles(
            const GA_Detail& geoRefMesh,
            const GA_Offset primoff,
            const UT_Array<int>&ptmap,
            UT_Array<int>&triangle_points)
    {
        const bool has_ptmap = !ptmap.isEmpty();
        const int primtype = geoRefMesh.getPrimitiveTypeId(primoff);
        switch (primtype)
        {
        case GA_PRIMPOLY:
        {
            const GA_OffsetListRef vertices = geoRefMesh.getPrimitiveVertexList(primoff);
            const GA_Size n = vertices.size();
            const bool closed = vertices.getExtraFlag();
            if (n < 3 || !closed)
                return;

            // A triangle fan suffices, even if the polygon is non-convex,
            // because the contributions in the opposite direction will
            // partly cancel out the ones in the other direction,
            // in just the right amount.
            const GA_Offset p0 = geoRefMesh.vertexPoint(vertices(0));
            const int p0i = has_ptmap ? ptmap[p0] : int(geoRefMesh.pointIndex(p0));
            GA_Offset prev = geoRefMesh.vertexPoint(vertices(1));
            int previ = has_ptmap ? ptmap[prev] : int(geoRefMesh.pointIndex(prev));
            for (GA_Size i = 2; i < n; ++i)
            {
                const GA_Offset next = geoRefMesh.vertexPoint(vertices(i));
                const int nexti = has_ptmap ? ptmap[next] : int(geoRefMesh.pointIndex(next));
                triangle_points.append(p0i);
                triangle_points.append(previ);
                triangle_points.append(nexti);
                previ = nexti;
            }
        }
        break;
        case GA_PRIMTETRAHEDRON:
        {
            const GA_OffsetListRef vertices = geoRefMesh.getPrimitiveVertexList(primoff);
            const GEO_PrimTetrahedron tet(SYSconst_cast(&geoRefMesh), primoff, vertices);

            for (int i = 0; i < 4; ++i)
            {
                // Ignore shared tet faces.  They would contribute exactly opposite amounts.
                if (tet.isFaceShared(i))
                    continue;

                const int* const face_indices = GEO_PrimTetrahedron::fastFaceIndices(i);
                const GA_Offset a = geoRefMesh.vertexPoint(vertices(face_indices[0]));
                const GA_Offset b = geoRefMesh.vertexPoint(vertices(face_indices[1]));
                const GA_Offset c = geoRefMesh.vertexPoint(vertices(face_indices[2]));
                const int ai = has_ptmap ? ptmap[a] : int(geoRefMesh.pointIndex(a));
                const int bi = has_ptmap ? ptmap[b] : int(geoRefMesh.pointIndex(b));
                const int ci = has_ptmap ? ptmap[c] : int(geoRefMesh.pointIndex(c));
                triangle_points.append(ai);
                triangle_points.append(bi);
                triangle_points.append(ci);
            }
        }
        case GA_PRIMPOLYSOUP:
        {
            const GEO_PrimPolySoup* const soup = UTverify_cast<const GEO_PrimPolySoup*>(geoRefMesh.getPrimitive(primoff));
            for (GEO_PrimPolySoup::PolygonIterator poly(*soup); !poly.atEnd(); ++poly)
            {
                const GA_Size n = poly.nvertices();
                if (n < 3)
                    continue;

                // A triangle fan suffices, even if the polygon is non-convex,
                // because the contributions in the opposite direction will
                // partly cancel out the ones in the other direction,
                // in just the right amount.
                const GA_Offset p0 = poly.getPointOffset(0);
                const int p0i = has_ptmap ? ptmap[p0] : int(geoRefMesh.pointIndex(p0));
                const GA_Offset prev = poly.getPointOffset(1);
                int previ = has_ptmap ? ptmap[prev] : int(geoRefMesh.pointIndex(prev));
                for (GA_Size i = 2; i < n; ++i)
                {
                    const GA_Offset next = poly.getPointOffset(i);
                    const int nexti = has_ptmap ? ptmap[next] : int(geoRefMesh.pointIndex(next));
                    triangle_points.append(p0i);
                    triangle_points.append(previ);
                    triangle_points.append(nexti);
                    previ = nexti;
                }
            }
        }
        break;
        case GEO_PRIMMESH:
        case GEO_PRIMNURBSURF:
        case GEO_PRIMBEZSURF:
        {
            // In this mode, we're only using points in the detail, so no grevilles.
            const GEO_Hull* const mesh = UTverify_cast<const GEO_Hull*>(geoRefMesh.getPrimitive(primoff));
            const int nquadrows = mesh->getNumRows() - !mesh->isWrappedV();
            const int nquadcols = mesh->getNumCols() - !mesh->isWrappedU();
            for (int row = 0; row < nquadrows; ++row)
            {
                for (int col = 0; col < nquadcols; ++col)
                {
                    GEO_Hull::Poly poly(*mesh, row, col);
                    const GA_Offset a = poly.getPointOffset(0);
                    const GA_Offset b = poly.getPointOffset(1);
                    const GA_Offset c = poly.getPointOffset(2);
                    const GA_Offset d = poly.getPointOffset(3);
                    const int ai = has_ptmap ? ptmap[a] : int(geoRefMesh.pointIndex(a));
                    const int bi = has_ptmap ? ptmap[b] : int(geoRefMesh.pointIndex(b));
                    const int ci = has_ptmap ? ptmap[c] : int(geoRefMesh.pointIndex(c));
                    const int di = has_ptmap ? ptmap[d] : int(geoRefMesh.pointIndex(d));
                    triangle_points.append(ai);
                    triangle_points.append(bi);
                    triangle_points.append(ci);
                    triangle_points.append(ai);
                    triangle_points.append(ci);
                    triangle_points.append(di);
                }
            }
        }
        break;
        default: break;
        }
    }

    void sopAccumulateSegments(
            const GA_Detail& geoRefMesh,
            const GA_Offset primoff,
            const UT_Array<int>&ptmap,
            UT_Array<int>&segment_points)
    {
        const bool has_ptmap = !ptmap.isEmpty();
        const int primtype = geoRefMesh.getPrimitiveTypeId(primoff);
        switch (primtype)
        {
        case GA_PRIMPOLY:
        case GA_PRIMBEZCURVE:
        case GA_PRIMNURBCURVE:
        {
            const GA_OffsetListRef vertices = geoRefMesh.getPrimitiveVertexList(primoff);
            const GA_Size n = vertices.size();
            const bool closed = vertices.getExtraFlag();
            if (n < 2 + int(closed))
                return;

            GA_Offset prev = geoRefMesh.vertexPoint(vertices(0));
            int previ = has_ptmap ? ptmap[prev] : int(geoRefMesh.pointIndex(prev));
            const int pt0i = previ;
            for (GA_Size i = 1; i < n; ++i)
            {
                const GA_Offset next = geoRefMesh.vertexPoint(vertices(i));
                const int nexti = has_ptmap ? ptmap[next] : int(geoRefMesh.pointIndex(next));
                segment_points.append(previ);
                segment_points.append(nexti);
                previ = nexti;
            }
            if (closed)
            {
                segment_points.append(previ);
                segment_points.append(pt0i);
            }
        }
        break;
        case GA_PRIMTETRAHEDRON:
        {
            // NOTE: Tetrahedra never contribute to the 2D winding number, since
            //       every point is contained in 1 forward triangle and 1 backward triangle.
        }
        break;
        case GA_PRIMPOLYSOUP:
        {
            const GEO_PrimPolySoup* const soup = UTverify_cast<const GEO_PrimPolySoup*>(geoRefMesh.getPrimitive(primoff));
            for (GEO_PrimPolySoup::PolygonIterator poly(*soup); !poly.atEnd(); ++poly)
            {
                GA_Size n = poly.nvertices();
                if (n < 3)
                    continue;

                GA_Offset prev = poly.getPointOffset(0);
                int previ = has_ptmap ? ptmap[prev] : int(geoRefMesh.pointIndex(prev));
                const int pt0i = previ;
                for (GA_Size i = 1; i < n; ++i)
                {
                    const GA_Offset next = poly.getPointOffset(i);
                    const int nexti = has_ptmap ? ptmap[next] : int(geoRefMesh.pointIndex(next));
                    segment_points.append(previ);
                    segment_points.append(nexti);
                    previ = nexti;
                }
                segment_points.append(previ);
                segment_points.append(pt0i);
            }
        }
        break;
        case GEO_PRIMMESH:
        case GEO_PRIMNURBSURF:
        case GEO_PRIMBEZSURF:
        {
            // In this mode, we're only using points in the detail, so no grevilles.
            const GEO_Hull* const mesh = UTverify_cast<const GEO_Hull*>(geoRefMesh.getPrimitive(primoff));
            const int nquadrows = mesh->getNumRows() - !mesh->isWrappedV();
            const int nquadcols = mesh->getNumCols() - !mesh->isWrappedU();
            for (int row = 0; row < nquadrows; ++row)
            {
                for (int col = 0; col < nquadcols; ++col)
                {
                    GEO_Hull::Poly poly(*mesh, row, col);
                    const GA_Offset a = poly.getPointOffset(0);
                    const GA_Offset b = poly.getPointOffset(1);
                    const GA_Offset c = poly.getPointOffset(2);
                    const GA_Offset d = poly.getPointOffset(3);
                    const int ai = has_ptmap ? ptmap[a] : int(geoRefMesh.pointIndex(a));
                    const int bi = has_ptmap ? ptmap[b] : int(geoRefMesh.pointIndex(b));
                    const int ci = has_ptmap ? ptmap[c] : int(geoRefMesh.pointIndex(c));
                    const int di = has_ptmap ? ptmap[d] : int(geoRefMesh.pointIndex(d));
                    segment_points.append(ai);
                    segment_points.append(bi);
                    segment_points.append(bi);
                    segment_points.append(ci);
                    segment_points.append(ci);
                    segment_points.append(di);
                    segment_points.append(di);
                    segment_points.append(ai);
                }
            }
        }
        break;
        default: break;
        }
    }




    
public:
    UT_SolidAngle<float, float> mySolidAngleTree;
    UT_SubtendedAngle<float, float> mySubtendedAngleTree;
    UT_Array<int> myTrianglePoints;
    UT_Array<UT_Vector2> myPositions2D;
    UT_Array<UT_Vector3> myPositions3D;
    GA_DataId myTopologyDataID;
    GA_DataId myPrimitiveListDataID;
    GA_DataId myPDataID;
    int myApproxOrder;
    int myAxis0;
    bool myHadGroup;
    UT_StringHolder myGroupString;
    exint myUniqueId;
    exint myMetaCacheCount;
    
}; // End of Class GFE_WindingNumber_Cache





class GFE_WindingNumber : public GFE_AttribFilterWithRef0 {

public:
    //using GFE_AttribFilterWithRef0::GFE_AttribFilterWithRef0;

    GFE_WindingNumber(
        GA_Detail& geo,
        const GA_Detail& geoRef0,
        GFE_WindingNumber_Cache* const sopcache = nullptr,
        const SOP_NodeVerb::CookParms* const cookparms = nullptr
    )
        : GFE_AttribFilterWithRef0(geo, &geoRef0, cookparms)
        , sopcachePoint(sopcache)
    {
    }

    ~GFE_WindingNumber()
    {
    }

    virtual void reset(
        GA_Detail& geo,
        const GA_Detail& geoRef0,
        GFE_WindingNumber_Cache* const sopcache = nullptr,
        const SOP_NodeVerb::CookParms* const cookparms = nullptr
    )
    {
        resetGeoFilterRef0(geoRef0, &getGroupParser().getGOPRef(), cookparms);
        GFE_AttribFilter::reset(geo, cookparms);
        this->sopcachePoint = sopcache;
    }


    
    void
        setComputeParm(
            const GFE_WNType wnType = GFE_WNType::XYZ,
            const bool fullAccuracy = false,
            const fpreal accuracyScale = 2.0,
            const bool asSolidAngle = false,
            const bool negate = false
        )
    {
        setHasComputed();
        this->wnType        = wnType;
        this->fullAccuracy  = fullAccuracy;
        this->accuracyScale = accuracyScale;
        this->asSolidAngle  = asSolidAngle;
        this->negate        = negate;
    }
    
    void
        setPointInMeshComputeParm(
            const bool groupInGeoPoint = true,
            const bool groupOnGeoPoint = true,
            const fpreal pointInMeshThreshold = 1e-05
        )
    {
        setHasComputed();
        this->groupInGeoPoint = groupInGeoPoint;
        this->groupOnGeoPoint = groupOnGeoPoint;
        this->pointInMeshThreshold = pointInMeshThreshold;
    }
    

    //SYS_FORCE_INLINE void setInGroup(
    //    const GA_PointGroup* const geoPointGroup = nullptr,
    //    const GA_PrimitiveGroup* const geoRefMeshGroup = nullptr
    //)
    //{
    //    groupParser.setGroup(geoPointGroup);
    //    groupParserRef0.setGroup(geoRefMeshGroup);
    //}
    //
    //SYS_FORCE_INLINE void setInGroup(
    //    const UT_StringHolder& geoPoint_groupName,
    //    const UT_StringHolder& geoRefMesh_groupName
    //)
    //{
    //    groupParser.setPointGroup(geoPoint_groupName);
    //    groupParserRef0.setPrimitiveGroup(geoRefMesh_groupName);
    //}


    SYS_FORCE_INLINE GA_Attribute*
        findOrCreateTuple(
            const bool detached = false,
            const GA_AttributeOwner owner = GA_ATTRIB_POINT,
            const GA_Storage storage = GA_STORE_INVALID,
            const UT_StringHolder& attribName = "__topo_windingNumber"
        )
    { return getOutAttribArray().findOrCreateTuple(detached, owner, GA_STORECLASS_FLOAT, storage, attribName); }

    // SYS_FORCE_INLINE GA_PointGroup*
    //     findOrCreateGroup(
    //         const bool detached = false,
    //         const UT_StringRef& groupName = "pointInMesh_wn"
    //     )
    // { return getOutGroupArray().findOrCreatePoint(detached, groupName); }

    

    


private:

    virtual bool
        computeCore() override
    {
        if (getOutAttribArray().isEmpty() && getOutGroupArray().isEmpty())
            return false;

        if (groupParser.isEmpty())
            return true;


        GA_PrimitiveGroup* outPrimGroup    = nullptr;
        GA_PointGroup*     outPointGroup   = nullptr;
        GA_VertexGroup*    outVtxedgeGroup = nullptr;
        GA_EdgeGroup*      outEdgeGroup    = nullptr;
        GA_Attribute* outPrimAttrib   = nullptr;
        GA_Attribute* outPointAttrib  = nullptr;
        GA_Attribute* outVertexAttrib = nullptr;

        
        if (!getOutGroupArray().isEmpty())
        {
            for (size_t i = getOutGroupArray().size()-1; ; --i)
            {
                switch (getOutGroupArray()[i]->classType())
                {
                case GA_GROUP_PRIMITIVE: UT_ASSERT_MSG(!outPrimGroup,    "already have prim    group"); outPrimGroup    = getOutGroupArray().getPrimitiveGroup(i); break;
                case GA_GROUP_POINT:     UT_ASSERT_MSG(!outPointGroup,   "already have point   group"); outPointGroup   = getOutGroupArray().getPointGroup(i);     break;
                case GA_GROUP_VERTEX:    UT_ASSERT_MSG(!outVtxedgeGroup, "already have vtxedge group"); outVtxedgeGroup = getOutGroupArray().getVertexGroup(i);    break;
                case GA_GROUP_EDGE:      UT_ASSERT_MSG(!outEdgeGroup,    "already have edge    group"); outEdgeGroup    = getOutGroupArray().getEdgeGroup(i);      break;
                default: getOutAttribArray().erase(i); break;
                }
                if (i <= 0)
                    break;
            }
        }


        if (!getOutAttribArray().isEmpty())
        {
            for (size_t i = getOutAttribArray().size()-1; ; --i)
            {
                switch (getOutAttribArray()[i]->getOwner())
                {
                case GA_ATTRIB_PRIMITIVE: UT_ASSERT_MSG(!outPrimAttrib,   "already have prim  attrib");  outPrimAttrib   = getOutAttribArray()[i]; break;
                case GA_ATTRIB_POINT:     UT_ASSERT_MSG(!outPointAttrib,  "already have point attrib");  outPointAttrib  = getOutAttribArray()[i]; break;
                case GA_ATTRIB_VERTEX:    UT_ASSERT_MSG(!outVertexAttrib, "already have vertex attrib"); outVertexAttrib = getOutAttribArray()[i]; break;
                default: getOutAttribArray().erase(i); break;
                }
                if (i <= 0)
                    break;
            }
        }


        const GA_Storage storage = geo->getPreferredStorageF();
        
        if (outPrimAttrib || outPrimGroup)
        {
            GU_DetailHandle geoTmp_h;
            GU_Detail* const geoTmp = new GU_Detail();
            geoTmp_h.allocateAndSet(geoTmp);
            geoPoint = static_cast<GA_Detail*>(geoTmp);
            
            //GFE_WindingNumber_Cache sopcacheTemp;
            //sopcache = &sopcacheTemp;
            sopcache = nullptr;
            
            GFE_PointGenPerElem pointGenPerElem(*geoPoint, geo, cookparms);
            pointGenPerElem.elemType = GA_GROUP_PRIMITIVE;
            pointGenPerElem.srcElemoffAttribName = "";
            pointGenPerElem.groupParser.setGroup(groupParser);
            pointGenPerElem.compute();
            
            
            const GA_ATINumericUPtr wnAttribUPtr = geoPoint->getAttributes().createDetachedTupleAttribute(GA_ATTRIB_POINT, storage, 1);
            wnAttrib = wnAttribUPtr.get();
            GA_PointGroupUPtr pointGroupUPtr;
            if (outPrimGroup)
            {
                pointGroupUPtr = geoPoint->createDetachedPointGroup();
                groupSetter = pointGroupUPtr.get();
            }
            else
            {
                groupSetter.clear();
            }
            
            const GA_SplittableRange geoPointRange = geoPoint->getPointRange();
            switch (storage)
            {
            case GA_STORE_REAL16:
            case GA_STORE_REAL32: computeWindingNumber<fpreal32>(geoPointRange); break;
            case GA_STORE_REAL64: computeWindingNumber<fpreal64>(geoPointRange); break;
            }
            
            
            GFE_AttribCopy attribCopy(geo, geoPoint, cookparms);
            attribCopy.ownerSrc = GA_ATTRIB_POINT;
            attribCopy.ownerDst = GA_ATTRIB_PRIMITIVE;
            attribCopy.attribMergeType = GFE_AttribMergeType::Set;
            attribCopy.iDAttribInput = false;
            attribCopy.setOffsetAttrib(*pointGenPerElem.getSrcElemoffAttrib(), true);

            if (outPrimAttrib)
                attribCopy.getRef0AttribArray().append(wnAttrib);
            attribCopy.getRef0GroupArray ().append(groupSetter.getElementGroup());
            
            attribCopy.getOutAttribArray().append(outPrimAttrib);
            attribCopy.getOutGroupArray() .append(outPrimGroup);
            
            attribCopy.compute();
        }
        
        if (outPointAttrib || outPointGroup)
        {
            geoPoint = static_cast<GA_Detail*>(geo);
            sopcache = sopcachePoint;
            
            GA_ATINumericUPtr wnAttribUPtr;
            if (outPointAttrib)
            {
                wnAttrib = outPointAttrib;
            }
            else
            {
                wnAttribUPtr = geoPoint->getAttributes().createDetachedTupleAttribute(GA_ATTRIB_POINT, storage, 1);
                wnAttrib = wnAttribUPtr.get();
            }
            groupSetter = outPointGroup;
            const GA_SplittableRange geoPointRange(groupParser.getPointSplittableRange());
            switch (storage)
            {
            case GA_STORE_REAL16:
            case GA_STORE_REAL32: computeWindingNumber<fpreal32>(geoPointRange); break;
            case GA_STORE_REAL64: computeWindingNumber<fpreal64>(geoPointRange); break;
            }
        }
        
        if (outEdgeGroup || outVtxedgeGroup || outVertexAttrib)
        {
            //GFE_WindingNumber_Cache sopcacheTemp;
            //sopcache = &sopcacheTemp;
            sopcache = nullptr;
            
            GU_DetailHandle geoTmp_h;
            GU_Detail* const geoTmp = new GU_Detail();
            geoTmp_h.allocateAndSet(geoTmp);
            geoPoint = static_cast<GA_Detail*>(geoTmp);
            
            GFE_PointGenPerElem pointGenPerElem(*geoPoint, geo, cookparms);
            pointGenPerElem.elemType = GA_GROUP_EDGE;
            pointGenPerElem.srcElemoffAttribName = "";
            pointGenPerElem.groupParser.setGroup(groupParser);
            pointGenPerElem.compute();
            
            
            const GA_ATINumericUPtr wnAttribUPtr = geoPoint->getAttributes().createDetachedTupleAttribute(GA_ATTRIB_POINT, storage, 1);
            wnAttrib = wnAttribUPtr.get();
            const GA_PointGroupUPtr pointGroupUPtr = geoPoint->createDetachedPointGroup();
            groupSetter = pointGroupUPtr.get();
            
            const GA_SplittableRange geoPointRange = geoPoint->getPointRange();
            switch (storage)
            {
            case GA_STORE_REAL16:
            case GA_STORE_REAL32: computeWindingNumber<fpreal32>(geoPointRange); break;
            case GA_STORE_REAL64: computeWindingNumber<fpreal64>(geoPointRange); break;
            }
            
            GA_VertexGroupUPtr vertexGroupUPtr;
            if (outEdgeGroup && !outVtxedgeGroup)
            {
                vertexGroupUPtr = geo->createDetachedVertexGroup();
                outVtxedgeGroup = vertexGroupUPtr.get();
            }
            
            GFE_AttribCopy attribCopy(geo, geoPoint, cookparms);
            attribCopy.ownerSrc = GA_ATTRIB_POINT;
            attribCopy.ownerDst = GA_ATTRIB_VERTEX;
            attribCopy.attribMergeType = GFE_AttribMergeType::Set;
            attribCopy.iDAttribInput = false;
            attribCopy.setOffsetAttrib(*pointGenPerElem.getSrcElemoffAttrib(), true);
            
            if (outVertexAttrib)
                attribCopy.getRef0AttribArray().append(wnAttrib);
            attribCopy.getRef0GroupArray ().append(groupSetter.getElementGroup());
            
            attribCopy.getOutAttribArray().append(outVertexAttrib);
            attribCopy.getOutGroupArray() .append(outVtxedgeGroup);
            
            attribCopy.compute();
            
            if (outEdgeGroup)
                GFE_GroupUnion::groupUnion(outEdgeGroup, outVtxedgeGroup);
                //GFE_GroupUnion::groupUnion(outEdgeGroup, attribCopy.getOutGroupArray()[0]);
        }
        
        
        if (doDelGroupElement)
            delGroupElement();
        
        return true;
    }


    template<typename FLOAT_T>
    void computeWindingNumber(const GA_SplittableRange& geoPointRange)
    {
        UT_ASSERT_P(wnAttrib);

        const GA_RWHandleT<FLOAT_T> wn_h(wnAttrib);
        const GA_PrimitiveGroup* const geoRefMeshGroup = groupParserRef0.getPrimitiveGroup();

        if (wnType == GFE_WNType::XYZ)
        {
            // NOTE: The negation is because Houdini's normals are
            //       left-handed with respect to the winding order,
            //       whereas most use the right-handed convension.
            const bool finalNegate = !negate;
            if (fullAccuracy)
            {
                if (sopcache)
                    sopcache->clear();

                sop3DFullAccuracy(
                    geoPointRange,
                    geoRefMeshGroup,
                    wn_h,
                    asSolidAngle, finalNegate);
            }
            else
            {
                const bool noCache = !sopcache;
                if (noCache)
                    sopcache = new GFE_WindingNumber_Cache();

                const UT_StringHolder& geoRefMeshGroupNameEmpty = "";
                const UT_StringHolder& geoRefMeshGroupName = !geoRefMeshGroup || geoRefMeshGroup->isDetached() ? UT_StringHolder() : geoRefMeshGroup->getName();
                sopcache->update3D(*geoRef0, geoRefMeshGroup, geoRefMeshGroupName, 2);
                const UT_SolidAngle<float, float>& solid_angle_tree = sopcache->mySolidAngleTree;

                sop3DApproximate(
                    geoPointRange,
                    solid_angle_tree, accuracyScale,
                    wn_h,
                    asSolidAngle, finalNegate);

                if (noCache)
                    delete sopcache;
            }
        }
        else
        {
            const int axis0 = int(wnType) - 1;
            const int axis1 = (axis0 == 2) ? 0 : (axis0 + 1);

            if (fullAccuracy)
            {
                if (sopcache)
                    sopcache->clear();

                sop2DFullAccuracy(
                    geoPointRange,
                    geoRefMeshGroup,
                    wn_h,
                    asSolidAngle, negate,
                    axis0, axis1);
            }
            else
            {
                const bool noCache = !sopcache;
                if (noCache)
                    sopcache = new GFE_WindingNumber_Cache();

                const UT_StringHolder& geoRefMeshGroupName = !geoRefMeshGroup || geoRefMeshGroup->isDetached() ? UT_StringHolder() : geoRefMeshGroup->getName();
                sopcache->update2D(*geoRef0, geoRefMeshGroup, geoRefMeshGroupName, 2, axis0, axis1);
                const UT_SubtendedAngle<float, float>& subtended_angle_tree = sopcache->mySubtendedAngleTree;

                sop2DApproximate(
                    geoPointRange,
                    subtended_angle_tree, accuracyScale,
                    wn_h,
                    asSolidAngle, negate,
                    axis0, axis1);

                if (noCache)
                    delete sopcache;
            }
        }
        
        computeOutGroup<FLOAT_T>(geoPointRange);
    }



        // UT_SolidAngle doesn't currently have a special case to handle quads correctly
        // as bilinear patches; it always treats them as two triangles, which can result
        // in a difference of about 1 between points that are just inside or just
        // outside.  If you want the accurate version to treat them as triangles too,
        // (e.g. for testing the approximation error of UT_SolidAngle), set this to 0.
#define QUADS_AS_BILINEAR_PATCHES 1






        template<typename FLOAT_T>
        void sopSumContributions3D(
            FLOAT_T& sum,
            const UT_Vector3T<FLOAT_T>&query_point,
            const GA_OffsetList & primoffs,
            const exint start,
            const exint end
        )
        {
            if (end - start > 1024)
            {
                exint mid = (start + end) >> 1;
                FLOAT_T sum0, sum1;
                UTparallelInvoke(true, [&] {
                    sopSumContributions3D(sum0, query_point, primoffs, start, mid);
                }, [&] {
                    sopSumContributions3D(sum1, query_point, primoffs, mid, end);
                });
                sum = sum0 + sum1;
                return;
            }

            FLOAT_T local_sum = 0;
            for (exint arrayi = start; arrayi < end; ++arrayi)
            {
                const GA_Offset primoff = primoffs(arrayi);
                const int primtype = geoRef0->getPrimitiveTypeId(primoff);
                switch (primtype)
                {
                case GA_PRIMPOLY:
                {
                    const GA_OffsetListRef vertices = geoRef0->getPrimitiveVertexList(primoff);
                    const GA_Size n = vertices.size();
                    const bool closed = vertices.getExtraFlag();
                    if (n < 3 || !closed)
                        continue;

                    if (n == 3)
                    {
                        const GA_Offset pt0 = geoRef0->vertexPoint(vertices(0));
                        const GA_Offset pt1 = geoRef0->vertexPoint(vertices(1));
                        const GA_Offset pt2 = geoRef0->vertexPoint(vertices(2));
                        const UT_Vector3T<FLOAT_T>& p0 = geoRef0->getPos3T<FLOAT_T>(pt0);
                        const UT_Vector3T<FLOAT_T>& p1 = geoRef0->getPos3T<FLOAT_T>(pt1);
                        const UT_Vector3T<FLOAT_T>& p2 = geoRef0->getPos3T<FLOAT_T>(pt2);
                        const FLOAT_T poly_sum = UTsignedSolidAngleTri(p0, p1, p2, query_point);
                        local_sum += poly_sum;
                        continue;
                    }
                    if (n == 4)
                    {
                        // Special case for quads, to divide the correct diagonal
                        // in the case where the point is inside the convex hull.
                        const GA_Offset pt0 = geoRef0->vertexPoint(vertices(0));
                        const GA_Offset pt1 = geoRef0->vertexPoint(vertices(1));
                        const GA_Offset pt2 = geoRef0->vertexPoint(vertices(2));
                        const GA_Offset pt3 = geoRef0->vertexPoint(vertices(3));
                        const UT_Vector3T<FLOAT_T>& p0 = geoRef0->getPos3T<FLOAT_T>(pt0);
                        const UT_Vector3T<FLOAT_T>& p1 = geoRef0->getPos3T<FLOAT_T>(pt1);
                        const UT_Vector3T<FLOAT_T>& p2 = geoRef0->getPos3T<FLOAT_T>(pt2);
                        const UT_Vector3T<FLOAT_T>& p3 = geoRef0->getPos3T<FLOAT_T>(pt3);
#if QUADS_AS_BILINEAR_PATCHES
                        const FLOAT_T poly_sum = UTsignedSolidAngleQuad(p0, p1, p2, p3, query_point);
#else
                        const FLOAT_T poly_sum =
                            UTsignedSolidAngleTri(p0, p1, p2, query_point) +
                            UTsignedSolidAngleTri(p0, p2, p3, query_point);
#endif
                        local_sum += poly_sum;
                        continue;
                    }
                    // A triangle fan suffices, even if the polygon is non-convex,
                    // because the contributions in the opposite direction will
                    // partly cancel out the ones in the other direction,
                    // in just the right amount.
                    FLOAT_T poly_sum = 0;
                    const UT_Vector3T<FLOAT_T>& p0 = geoRef0->getPos3T<FLOAT_T>(geoRef0->vertexPoint(vertices(0)));
                    UT_Vector3T<FLOAT_T> prev = geoRef0->getPos3T<FLOAT_T>(geoRef0->vertexPoint(vertices(1)));
                    for (GA_Size i = 2; i < n; ++i)
                    {
                        const UT_Vector3T<FLOAT_T>& next = geoRef0->getPos3T<FLOAT_T>(geoRef0->vertexPoint(vertices(i)));
                        poly_sum += UTsignedSolidAngleTri(p0, prev, next, query_point);
                        prev = next;
                    }
                    local_sum += poly_sum;
                }
                break;
                case GA_PRIMTETRAHEDRON:
                {
                    const GA_OffsetListRef vertices = geoRef0->getPrimitiveVertexList(primoff);
                    const GEO_PrimTetrahedron tet(SYSconst_cast(geoRef0), primoff, vertices);

                    FLOAT_T tet_sum = 0;
                    for (int i = 0; i < 4; ++i)
                    {
                        // Ignore shared tet faces.  They would contribute exactly opposite amounts.
                        if (tet.isFaceShared(i))
                            continue;

                        const int* face_indices = GEO_PrimTetrahedron::fastFaceIndices(i);
                        const GA_Offset pt0 = geoRef0->vertexPoint(vertices(face_indices[0]));
                        const GA_Offset pt1 = geoRef0->vertexPoint(vertices(face_indices[1]));
                        const GA_Offset pt2 = geoRef0->vertexPoint(vertices(face_indices[2]));
                        const UT_Vector3T<FLOAT_T>& a = geoRef0->getPos3T<FLOAT_T>(pt0);
                        const UT_Vector3T<FLOAT_T>& b = geoRef0->getPos3T<FLOAT_T>(pt1);
                        const UT_Vector3T<FLOAT_T>& c = geoRef0->getPos3T<FLOAT_T>(pt2);

                        tet_sum += UTsignedSolidAngleTri(a, b, c, query_point);
                    }
                    local_sum += tet_sum;
                }
                break;
                case GA_PRIMPOLYSOUP:
                {
                    FLOAT_T soup_sum = 0;
                    const GEO_PrimPolySoup* const soup = UTverify_cast<const GEO_PrimPolySoup*>(geoRef0->getPrimitive(primoff));
                    for (GEO_PrimPolySoup::PolygonIterator poly(*soup); !poly.atEnd(); ++poly)
                    {
                        GA_Size n = poly.nvertices();
                        if (n < 3)
                            continue;

                        if (n == 3)
                        {
#if 1
                            const UT_Vector3T<FLOAT_T>& p0 = geoRef0->getPos3T<FLOAT_T>(poly.getPointOffset(0));
                            const UT_Vector3T<FLOAT_T>& p1 = geoRef0->getPos3T<FLOAT_T>(poly.getPointOffset(1));
                            const UT_Vector3T<FLOAT_T>& p2 = geoRef0->getPos3T<FLOAT_T>(poly.getPointOffset(2));
#else
                            const UT_Vector3T<FLOAT_T>& p0 = poly.getPos3(0);
                        const UT_Vector3T<FLOAT_T>& p1 = poly.getPos3(1);
                        const UT_Vector3T<FLOAT_T>& p2 = poly.getPos3(2);
#endif
                        const FLOAT_T poly_sum = UTsignedSolidAngleTri(p0, p1, p2, query_point);
                        local_sum += poly_sum;
                        continue;
                        }
                        if (n == 4)
                        {
                            // Special case for quads, to divide the correct diagonal
                            // in the case where the point is inside the convex hull.
                            #if 1
                            const UT_Vector3T<FLOAT_T>& p0 = geoRef0->getPos3T<FLOAT_T>(poly.getPointOffset(0));
                            const UT_Vector3T<FLOAT_T>& p1 = geoRef0->getPos3T<FLOAT_T>(poly.getPointOffset(1));
                            const UT_Vector3T<FLOAT_T>& p2 = geoRef0->getPos3T<FLOAT_T>(poly.getPointOffset(2));
                            const UT_Vector3T<FLOAT_T>& p3 = geoRef0->getPos3T<FLOAT_T>(poly.getPointOffset(3));
#else
                            const UT_Vector3T<FLOAT_T>& p0 = poly.getPos3(0);
                            const UT_Vector3T<FLOAT_T>& p1 = poly.getPos3(1);
                            const UT_Vector3T<FLOAT_T>& p2 = poly.getPos3(2);
                            const UT_Vector3T<FLOAT_T>& p3 = poly.getPos3(3);
#endif
#if QUADS_AS_BILINEAR_PATCHES
                            const FLOAT_T poly_sum = UTsignedSolidAngleQuad(p0, p1, p2, p3, query_point);
#else
                            const FLOAT_T poly_sum =
                                UTsignedSolidAngleTri(p0, p1, p2, query_point) +
                                UTsignedSolidAngleTri(p0, p2, p3, query_point);
#endif
                            local_sum += poly_sum;
                            continue;
                        }
                        // A triangle fan suffices, even if the polygon is non-convex,
                        // because the contributions in the opposite direction will
                        // partly cancel out the ones in the other direction,
                        // in just the right amount.
                        FLOAT_T poly_sum = 0;
#if 1
                        const UT_Vector3T<FLOAT_T>& p0 = geoRef0->getPos3T<FLOAT_T>(poly.getPointOffset(0));
                        UT_Vector3T<FLOAT_T> prev = geoRef0->getPos3T<FLOAT_T>(poly.getPointOffset(1));
#else
                        const UT_Vector3T<FLOAT_T>& p0 = poly.getPos3(0);
                        UT_Vector3T<FLOAT_T> prev = poly.getPos3(1);
#endif
                        for (GA_Size i = 2; i < n; ++i)
                        {
#if 1
                            const UT_Vector3T<FLOAT_T>& next = geoRef0->getPos3T<FLOAT_T>(poly.getPointOffset(i));
#else
                            const UT_Vector3T<FLOAT_T>& next = poly.getPos3(i);
#endif
                        poly_sum += UTsignedSolidAngleTri(p0, prev, next, query_point);
                        prev = next;
                        }
                        soup_sum += poly_sum;
                    }
                    local_sum += soup_sum;
                }
                    break;
                case GEO_PRIMMESH:
                {
                    FLOAT_T mesh_sum = 0;
                    const GEO_PrimMesh* mesh = UTverify_cast<const GEO_PrimMesh*>(geoRef0->getPrimitive(primoff));
                    const int nquadrows = mesh->getNumRows() - !mesh->isWrappedV();
                    const int nquadcols = mesh->getNumCols() - !mesh->isWrappedU();
                    for (int row = 0; row < nquadrows; ++row)
                    {
                        for (int col = 0; col < nquadcols; ++col)
                        {
                            GEO_Hull::Poly poly(*mesh, row, col);
#if 1
                            const UT_Vector3T<FLOAT_T>& p0 = geoRef0->getPos3T<FLOAT_T>(poly.getPointOffset(0));
                            const UT_Vector3T<FLOAT_T>& p1 = geoRef0->getPos3T<FLOAT_T>(poly.getPointOffset(1));
                            const UT_Vector3T<FLOAT_T>& p2 = geoRef0->getPos3T<FLOAT_T>(poly.getPointOffset(2));
                            const UT_Vector3T<FLOAT_T>& p3 = geoRef0->getPos3T<FLOAT_T>(poly.getPointOffset(3));
#else
                            const UT_Vector3T<FLOAT_T>& p0 = poly.getPos3(0);
                            const UT_Vector3T<FLOAT_T>& p1 = poly.getPos3(1);
                            const UT_Vector3T<FLOAT_T>& p2 = poly.getPos3(2);
                            const UT_Vector3T<FLOAT_T>& p3 = poly.getPos3(3);
#endif
#if QUADS_AS_BILINEAR_PATCHES
                            const FLOAT_T poly_sum = UTsignedSolidAngleQuad(p0, p1, p2, p3, query_point);
#else
                            const FLOAT_T poly_sum =
                                UTsignedSolidAngleTri(p0, p1, p2, query_point) +
                                UTsignedSolidAngleTri(p0, p2, p3, query_point);
#endif
                            mesh_sum += poly_sum;
                        }
                    }
                    local_sum += mesh_sum;
                }
                    break;
                case GEO_PRIMNURBSURF:
                case GEO_PRIMBEZSURF:
                {
                    const GEO_TPSurf* const surf = UTverify_cast<const GEO_TPSurf*>(geoRef0->getPrimitive(primoff));
                    const int nquadrows = surf->getNumRows() - !surf->isWrappedV();
                    const int nquadcols = surf->getNumCols() - !surf->isWrappedU();
                    if (nquadcols <= 0 || nquadrows <= 0)
                        continue;

                    // For slightly better accuracy, we use the greville points
                    // instead of the hull points.
                    FLOAT_T surf_sum = 0;
                    UT_FloatArray ugrevilles;
                    UT_FloatArray vgrevilles;
                    surf->getUGrevilles(ugrevilles);
                    if (surf->isWrappedU())
                        ugrevilles.append(0);
                    surf->getVGrevilles(vgrevilles);
                    if (surf->isWrappedV())
                        vgrevilles.append(0);
                    UT_ASSERT(ugrevilles.size() >= nquadcols + 1);
                    UT_ASSERT(vgrevilles.size() >= nquadrows + 1);
                    for (int row = 0; row < nquadrows; ++row)
                    {
                        for (int col = 0; col < nquadcols; ++col)
                        {
                            UT_Vector4 a4;
                            UT_Vector4 b4;
                            UT_Vector4 c4;
                            UT_Vector4 d4;
                            surf->evaluateInteriorPoint(a4, ugrevilles(col), vgrevilles(row));
                            surf->evaluateInteriorPoint(b4, ugrevilles(col + 1), vgrevilles(row));
                            surf->evaluateInteriorPoint(c4, ugrevilles(col + 1), vgrevilles(row + 1));
                            surf->evaluateInteriorPoint(d4, ugrevilles(col), vgrevilles(row + 1));
                            const UT_Vector3T<FLOAT_T> p0(a4);
                            const UT_Vector3T<FLOAT_T> p1(b4);
                            const UT_Vector3T<FLOAT_T> p2(c4);
                            const UT_Vector3T<FLOAT_T> p3(d4);
#if QUADS_AS_BILINEAR_PATCHES
                            const FLOAT_T poly_sum = UTsignedSolidAngleQuad(p0, p1, p2, p3, query_point);
#else
                            const FLOAT_T poly_sum =
                                UTsignedSolidAngleTri(p0, p1, p2, query_point) +
                                UTsignedSolidAngleTri(p0, p2, p3, query_point);
#endif
                            surf_sum += poly_sum;
                        }
                    }
                    local_sum += surf_sum;
                }
                    break;
                case GEO_PRIMSPHERE:
                {
                    const GEO_PrimSphere* const sphere = UTverify_cast<const GEO_PrimSphere*>(geoRef0->getPrimitive(primoff));
                    UT_Vector3T<FLOAT_T> query_minus_centre = query_point - UT_Vector3T<FLOAT_T>(sphere->getPos3T<FLOAT_T>(0));
                    UT_Matrix3D transformD;
                    sphere->getLocalTransform(transformD);
                    UT_Matrix3T<FLOAT_T> transform(transformD);

                    // If we're outside, no contribution.
                    // If we're inside and the determinant is negative, +4pi
                    // If we're inside and the determinant is positive, -4pi
                    // The negation is for consistency with the winding order of polygons
                    // that would be created if the sphere were converted to polygons.
                    FLOAT_T determinant = transform.determinant();
                    if (determinant == 0)
                        continue;

                    int failed = transform.invert();
                    if (failed)
                        continue;

                    query_minus_centre.rowVecMult(transform);
                    float length2 = query_minus_centre.length2();

                    FLOAT_T sphere_sum = 0;
                    if (length2 < 1)
                        sphere_sum = 4 * M_PI;
                    else if (length2 == 1)
                        sphere_sum = 2 * M_PI;

                    if (determinant > 0)
                        sphere_sum = -sphere_sum;

                    local_sum += sphere_sum;
                }
                }
            }
            sum = local_sum;
        }

        template<typename FLOAT_T>
        void sopSumContributions2D(
                FLOAT_T& sum,
                const UT_Vector2T<FLOAT_T>& query_point,
                const GA_OffsetList& primoffs,
                const exint start,
                const exint end,
                const int axis0,
                const int axis1
            )
        {
            if (end - start > 1024)
            {
                exint mid = (start + end) >> 1;
                FLOAT_T sum0, sum1;
                UTparallelInvoke(true, [&] {
                    sopSumContributions2D(sum0, query_point, primoffs, start, mid, axis0, axis1);
                }, [&] {
                    sopSumContributions2D(sum1, query_point, primoffs, mid, end, axis0, axis1);
                });
                sum = sum0 + sum1;
                return;
            }

            FLOAT_T local_sum = 0;
            for (exint arrayi = start; arrayi < end; ++arrayi)
            {
                const GA_Offset primoff = primoffs(arrayi);
                const int primtype = geoRef0->getPrimitiveTypeId(primoff);
                switch (primtype)
                {
                case GA_PRIMPOLY:
                    {
                        const GA_OffsetListRef vertices = geoRef0->getPrimitiveVertexList(primoff);
                        const GA_Size n = vertices.size();
                        const bool closed = vertices.getExtraFlag();
                        if (n < 2 + int(closed))
                            continue;

                        FLOAT_T poly_sum = 0;
                        UT_Vector3T<FLOAT_T> pos = geoRef0->getPos3T<FLOAT_T>(geoRef0->vertexPoint(vertices(0)));
                        UT_Vector2T<FLOAT_T> prev(pos[axis0], pos[axis1]);
                        const UT_Vector2T<FLOAT_T>& p0 = prev;
                        for (GA_Size i = 1; i < n; ++i)
                        {
                            pos = geoRef0->getPos3T<FLOAT_T>(geoRef0->vertexPoint(vertices(i)));
                            const UT_Vector2T<FLOAT_T> next(pos[axis0], pos[axis1]);
                            poly_sum += UTsignedAngleSegment(prev, next, query_point);
                            prev = next;
                        }
                        if (closed)
                            poly_sum += UTsignedAngleSegment(prev, p0, query_point);

                        local_sum += poly_sum;
                    }
                    break;
                case GA_PRIMTETRAHEDRON:
                    {
                        // NOTE: Tetrahedra never contribute to the 2D winding number, since
                        //       every point is contained in 1 forward triangle and 1 backward triangle.
                    }
                    break;
                case GA_PRIMPOLYSOUP:
                    {
                        FLOAT_T soup_sum = 0;
                        const GEO_PrimPolySoup* const soup = UTverify_cast<const GEO_PrimPolySoup*>(geoRef0->getPrimitive(primoff));
                        for (GEO_PrimPolySoup::PolygonIterator poly(*soup); !poly.atEnd(); ++poly)
                        {
                            GA_Size n = poly.nvertices();
                            if (n < 3)
                                continue;

                            FLOAT_T poly_sum = 0;
#if 1
                            UT_Vector3T<FLOAT_T> pos = geoRef0->getPos3T<FLOAT_T>(poly.getPointOffset(0));
#else
                            UT_Vector3T<FLOAT_T> pos = poly.getPos3(0);
#endif
                            UT_Vector2T<FLOAT_T> prev(pos[axis0], pos[axis1]);
                            const UT_Vector2T<FLOAT_T>& p0 = prev;
                            for (GA_Size i = 1; i < n; ++i)
                            {
#if 1
                                pos = geoRef0->getPos3T<FLOAT_T>(poly.getPointOffset(i));
#else
                                pos = poly.getPos3(i);
#endif
                            const UT_Vector2T<FLOAT_T> next(pos[axis0], pos[axis1]);
                            poly_sum += UTsignedAngleSegment(prev, next, query_point);
                            prev = next;
                            }
                            poly_sum += UTsignedAngleSegment(prev, p0, query_point);
                            soup_sum += poly_sum;
                        }
                        local_sum += soup_sum;
                    }
                    break;
                case GEO_PRIMMESH:
                    {
                        FLOAT_T mesh_sum = 0;
                        const GEO_PrimMesh* mesh = UTverify_cast<const GEO_PrimMesh*>(geoRef0->getPrimitive(primoff));
                        const int nquadrows = mesh->getNumRows() - !mesh->isWrappedV();
                        const int nquadcols = mesh->getNumCols() - !mesh->isWrappedU();
                        for (int row = 0; row < nquadrows; ++row)
                        {
                            for (int col = 0; col < nquadcols; ++col)
                            {
                                GEO_Hull::Poly poly(*mesh, row, col);
                                FLOAT_T poly_sum = 0;
#if 1
                                UT_Vector3T<FLOAT_T> pos = geoRef0->getPos3T<FLOAT_T>(poly.getPointOffset(0));
#else
                                UT_Vector3T<FLOAT_T> pos = poly.getPos3(0);
#endif
                                UT_Vector2T<FLOAT_T> prev(pos[axis0], pos[axis1]);
                                const UT_Vector2T<FLOAT_T>& p0 = prev;
                                for (GA_Size i = 1; i < 4; ++i)
                                {
#if 1
                                    pos = geoRef0->getPos3T<FLOAT_T>(poly.getPointOffset(i));
#else
                                    pos = poly.getPos3(i);
#endif
                                const UT_Vector2T<FLOAT_T> next(pos[axis0], pos[axis1]);
                                poly_sum += UTsignedAngleSegment(prev, next, query_point);
                                prev = next;
                                }
                                poly_sum += UTsignedAngleSegment(prev, p0, query_point);
                                mesh_sum += poly_sum;
                            }
                        }
                        local_sum += mesh_sum;
                    }
                    break;
                case GEO_PRIMNURBSURF:
                case GEO_PRIMBEZSURF:
                    {
                        const GEO_TPSurf* const surf = UTverify_cast<const GEO_TPSurf*>(geoRef0->getPrimitive(primoff));
                        const int nquadrows = surf->getNumRows() - !surf->isWrappedV();
                        const int nquadcols = surf->getNumCols() - !surf->isWrappedU();
                        if (nquadcols <= 0 || nquadrows <= 0)
                            continue;

                        // For slightly better accuracy, we use the greville points
                        // instead of the hull points.
                        FLOAT_T surf_sum = 0;
                        UT_FloatArray ugrevilles;
                        UT_FloatArray vgrevilles;
                        surf->getUGrevilles(ugrevilles);
                        if (surf->isWrappedU())
                            ugrevilles.append(0);
                        surf->getVGrevilles(vgrevilles);
                        if (surf->isWrappedV())
                            vgrevilles.append(0);
                        UT_ASSERT(ugrevilles.size() >= nquadcols + 1);
                        UT_ASSERT(vgrevilles.size() >= nquadrows + 1);
                        for (int row = 0; row < nquadrows; ++row)
                        {
                            for (int col = 0; col < nquadcols; ++col)
                            {
                                UT_Vector4 a4;
                                UT_Vector4 b4;
                                UT_Vector4 c4;
                                UT_Vector4 d4;
                                surf->evaluateInteriorPoint(a4, ugrevilles(col), vgrevilles(row));
                                surf->evaluateInteriorPoint(b4, ugrevilles(col + 1), vgrevilles(row));
                                surf->evaluateInteriorPoint(c4, ugrevilles(col + 1), vgrevilles(row + 1));
                                surf->evaluateInteriorPoint(d4, ugrevilles(col), vgrevilles(row + 1));
                                const UT_Vector2T<FLOAT_T> p0(a4[axis0], a4[axis1]);
                                const UT_Vector2T<FLOAT_T> p1(b4[axis0], b4[axis1]);
                                const UT_Vector2T<FLOAT_T> p2(c4[axis0], c4[axis1]);
                                const UT_Vector2T<FLOAT_T> p3(d4[axis0], d4[axis1]);
                                FLOAT_T poly_sum;
                                poly_sum  = UTsignedAngleSegment(p0, p1, query_point);
                                poly_sum += UTsignedAngleSegment(p1, p2, query_point);
                                poly_sum += UTsignedAngleSegment(p2, p3, query_point);
                                poly_sum += UTsignedAngleSegment(p3, p0, query_point);
                                surf_sum += poly_sum;
                            }
                        }
                        local_sum += surf_sum;
                    }
                    break;
                case GEO_PRIMNURBCURVE:
                case GEO_PRIMBEZCURVE:
                    {
                        const GEO_Curve* curve = UTverify_cast<const GEO_Curve*>(geoRef0->getPrimitive(primoff));
                        const int nedges = curve->getVertexCount() - !curve->isClosed();
                        if (nedges <= 0)
                            continue;

                        // For slightly better accuracy, we use the greville points
                        // instead of the hull points.
                        FLOAT_T curve_sum = 0;
                        UT_Array<GEO_Greville> grevilles;
                        grevilles.setCapacity(nedges + 1);
                        curve->buildGrevilles(grevilles);
                        if (curve->isClosed())
                            grevilles.append(grevilles[0]);
                        UT_ASSERT(grevilles.size() >= nedges + 1);
                        UT_Vector2T<FLOAT_T> prev(grevilles[0].getCVImage()[axis0], grevilles[0].getCVImage()[axis1]);
                        for (int i = 0; i < nedges; ++i)
                        {
                            const UT_Vector2T<FLOAT_T> next(grevilles[i].getCVImage()[axis0], grevilles[i].getCVImage()[axis1]);
                            curve_sum += UTsignedAngleSegment(prev, next, query_point);
                        }
                        local_sum += curve_sum;
                    }
                    break;
                default: break;
                }
            }
            sum = local_sum;
        }

        template<typename FLOAT_T>
        void sop3DFullAccuracy(
            const GA_SplittableRange& geoPointRange,
            const GA_PrimitiveGroup* const geoRefMeshGroup,
            const GA_RWHandleT<FLOAT_T>& wn_h,
            const bool asSolidAngle,
            const bool negate
        )
        {
            UT_AutoInterrupt boss("Computing Winding Numbers");

            // See comment below for why we create this GA_Offsetlist of mesh primitive offsets.
            GA_OffsetList primoffs;
            if (!geoRefMeshGroup && geoRef0->getPrimitiveMap().isTrivialMap())
            {
                primoffs.setTrivial(GA_Offset(0), geoRef0->getNumPrimitives());
            }
            else
            {
                GA_Offset start, end;
                for (GA_Iterator it(geoRef0->getPrimitiveRange(geoRefMeshGroup)); it.fullBlockAdvance(start, end); )
                {
                    primoffs.setTrivialRange(primoffs.size(), start, end - start);
                }
            }

            UTparallelFor(geoPointRange, [this, &wn_h, &primoffs, asSolidAngle, negate, &boss](const GA_SplittableRange& r)
            {
                GA_Offset start, end;
                for (GA_Iterator it(r); it.blockAdvance(start, end); )
                {
                    for (GA_Offset ptoff = start; ptoff != end; ++ptoff)
                    {
                        if (boss.wasInterrupted())
                            return;

                        const UT_Vector3T<FLOAT_T>& query_point = geoPoint->getPos3D(ptoff);

                        // NOTE: We can't use UTparallelReduce, because that would have
                        //       nondeterministic roundoff error due to floating-point
                        //       addition being non-associative.  We can't just use
                        //       UTparallelInvoke with GA_SplittableRange either, because
                        //       the roundoff error would change with defragmentation,
                        //       e.g. from locking the mesh input and reloading the HIP file.
                        //       Instead, we always split in the middle of the list of offsets.
                        FLOAT_T sum;
                        sopSumContributions3D(sum, query_point, primoffs, 0, primoffs.size());

                        if (!asSolidAngle)
                            sum *= (0.25 * M_1_PI); // Divide by 4pi (solid angle of full sphere)
                        if (negate)
                            sum = -sum;

                        wn_h.set(ptoff, sum);
                    }
                }
            });
        }

        template<typename FLOAT_T>
        void sop2DFullAccuracy(
            const GA_SplittableRange& geoPointRange,
            const GA_PrimitiveGroup* const geoRefMeshGroup,
            const GA_RWHandleT<FLOAT_T>& wn_h,
            const bool asSolidAngle,
            const bool negate,
            const int axis0,
            const int axis1
        )
        {
            UT_AutoInterrupt boss("Computing Winding Numbers");

            // See comment below for why we create this GA_Offsetlist of mesh primitive offsets.
            GA_OffsetList primoffs;
            if (!geoRefMeshGroup && geoRef0->getPrimitiveMap().isTrivialMap())
            {
                primoffs.setTrivial(GA_Offset(0), geoRef0->getNumPrimitives());
            }
            else
            {
                GA_Offset start, end;
                for (GA_Iterator it(geoRef0->getPrimitiveRange(geoRefMeshGroup)); it.fullBlockAdvance(start, end); )
                {
                    primoffs.setTrivialRange(primoffs.size(), start, end - start);
                }
            }

            UTparallelFor(geoPointRange, [this, &wn_h, &primoffs, asSolidAngle, negate, &boss, axis0, axis1](const GA_SplittableRange& r)
            {
                GA_Offset start, end;
                for (GA_Iterator it(r); it.blockAdvance(start, end); )
                {
                    for (GA_Offset ptoff = start; ptoff != end; ++ptoff)
                    {
                        if (boss.wasInterrupted())
                            return;

                        const UT_Vector3T<FLOAT_T>& query_point_3d = geoPoint->getPos3T<FLOAT_T>(ptoff);
                        const UT_Vector2T<FLOAT_T> query_point(query_point_3d[axis0], query_point_3d[axis1]);

                        // NOTE: We can't use UTparallelReduce, because that would have
                        //       nondeterministic roundoff error due to floating-point
                        //       addition being non-associative.  We can't just use
                        //       UTparallelInvoke with GA_SplittableRange either, because
                        //       the roundoff error would change with defragmentation,
                        //       e.g. from locking the mesh input and reloading the HIP file.
                        //       Instead, we always split in the middle of the list of offsets.
                        FLOAT_T sum;
                        sopSumContributions2D(sum, query_point, primoffs, 0, primoffs.size(), axis0, axis1);

                        if (!asSolidAngle)
                            sum *= (0.5 * M_1_PI); // Divide by 2pi (angle of full circle)
                        if (negate)
                            sum = -sum;

                        wn_h.set(ptoff, sum);
                    }
                }
            });
        }

        template<typename FLOAT_T>
        void sop3DApproximate(
            const GA_SplittableRange& geoPointRange,
            const UT_SolidAngle<float, float>& solid_angle_tree,
            const fpreal accuracyScale,
            const GA_RWHandleT<FLOAT_T>&wn_h,
            const bool asSolidAngle,
            const bool negate
        )
        {
            UT_AutoInterrupt boss("Computing Winding Numbers");

            UTparallelFor(geoPointRange, [this, &wn_h, &solid_angle_tree, asSolidAngle, negate, accuracyScale, &boss](const GA_SplittableRange& r)
            {
                GA_Offset start, end;
                for (GA_Iterator it(r); it.blockAdvance(start, end); )
                {
                    if (boss.wasInterrupted())
                        return;

                    for (GA_Offset ptoff = start; ptoff != end; ++ptoff)
                    {
                        const UT_Vector3T<FLOAT_T>& query_point = geoPoint->getPos3T<FLOAT_T>(ptoff);

                        FLOAT_T sum = solid_angle_tree.computeSolidAngle(query_point, accuracyScale);

                        if (!asSolidAngle)
                            sum *= (0.25 * M_1_PI); // Divide by 4pi (solid angle of full sphere)
                        if (negate)
                            sum = -sum;

                        wn_h.set(ptoff, sum);
                    }
                }
            }, 10); // Large subscribe ratio, because expensive points are often clustered
        }

        template<typename FLOAT_T>
        void sop2DApproximate(
            const GA_SplittableRange& geoPointRange,
            const UT_SubtendedAngle<float, float>&subtended_angle_tree,
            const fpreal accuracyScale,
            const GA_RWHandleT<FLOAT_T>&wn_h,
            const bool as_angle,
            const bool negate,
            const int axis0,
            const int axis1
        )
        {
            UT_AutoInterrupt boss("Computing Winding Numbers");

            UTparallelFor(geoPointRange, [this, &wn_h, &subtended_angle_tree, as_angle, negate, accuracyScale, &boss, axis0, axis1](const GA_SplittableRange& r)
            {
                GA_Offset start, end;
                for (GA_Iterator it(r); it.blockAdvance(start, end); )
                {
                    if (boss.wasInterrupted())
                        return;

                    for (GA_Offset ptoff = start; ptoff != end; ++ptoff)
                    {
                        const UT_Vector3T<FLOAT_T>& query_point_3d = geoPoint->getPos3T<FLOAT_T>(ptoff);
                        const UT_Vector2T<FLOAT_T> query_point(query_point_3d[axis0], query_point_3d[axis1]);

                        FLOAT_T sum = subtended_angle_tree.computeAngle(query_point, accuracyScale);

                        if (!as_angle)
                            sum *= (0.5 * M_1_PI); // Divide by 2pi (angle of full circle)
                        if (negate)
                            sum = -sum;

                        wn_h.set(ptoff, sum);
                    }
                }
            }, 10); // Large subscribe ratio, because expensive points are often clustered
        }



#undef QUADS_AS_BILINEAR_PATCHES




    template<typename FLOAT_T>
    void computeOutGroup(const GA_SplittableRange& geoPointRange)
    {
        if (groupSetter.isElementInvalid())
            return;
        
        UT_ASSERT_P(wnAttrib);
        //UT_ASSERT(!getOutAttribArray().isEmpty());
        //UT_ASSERT(!getOutGroupArray().isEmpty());
        
        UTparallelFor(geoPointRange, [this](const GA_SplittableRange& r)
        {
            GA_PageHandleT<FLOAT_T, FLOAT_T, true, false, const GA_Attribute, const GA_ATINumeric, const GA_Detail> attrib_ph(wnAttrib);
            for (GA_PageIterator pit = r.beginPages(); !pit.atEnd(); ++pit)
            {
                GA_Offset start, end;
                for (GA_Iterator it(pit.begin()); it.blockAdvance(start, end); )
                {
                    attrib_ph.setPage(start);
                    for (GA_Offset elemoff = start; elemoff < end; ++elemoff)
                    {
                        FLOAT_T windingNumber = attrib_ph.value(elemoff);
                        windingNumber = abs(windingNumber);

                        if (asSolidAngle)
                            windingNumber /= PI * 2.0;

                        bool outval;
                        if (groupInGeoPoint && groupOnGeoPoint)
                        {
                            outval = windingNumber > 0.5 - pointInMeshThreshold;
                        }
                        else
                        {
                            if (groupInGeoPoint)
                            {
                                outval = windingNumber > 0.5 + pointInMeshThreshold;
                            }
                            if (groupOnGeoPoint)
                            {
                                outval = windingNumber > (0.5 - pointInMeshThreshold) && windingNumber < (0.5 + pointInMeshThreshold);
                            }
                        }
                        
                        groupSetter.set(elemoff, outval);
                    }
                }
            }
        }, subscribeRatio, minGrainSize);
    }




    
public:
    GFE_WNType wnType = GFE_WNType::XYZ;
    bool fullAccuracy = false;
    fpreal accuracyScale = 2.0;
    bool asSolidAngle = false;
    bool negate = false;
    
public:
    bool groupInGeoPoint = true;
    bool groupOnGeoPoint = true;
    fpreal pointInMeshThreshold = 1e-05;

private:
    GA_Attribute* wnAttrib;
    //GU_Detail* geoTmp;
    GA_Detail* geoPoint;
    
    GFE_WindingNumber_Cache* sopcachePoint;
    GFE_WindingNumber_Cache* sopcache;
    
    exint subscribeRatio = 64;
    exint minGrainSize = 1024;

}; // End of class GFE_WindingNumber




#endif
