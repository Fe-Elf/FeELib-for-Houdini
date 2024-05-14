
#pragma once

#ifndef __GFE_VDBActivateByPoint_h__
#define __GFE_VDBActivateByPoint_h__

#include <GFE/GFE_VDBActivateByPoint.h>

#include <GFE/GeoFilter.h>

#include <GU/GU_PrimVDB.h>
#include <openvdb/openvdb.h>

//#include <openvdb/tools/ChangeBackground.h>
//#include <openvdb/tools/Interpolation.h>




#if 0

    gfe::VDBActivateByPoint vdbActivateByPoint(geo, cookparms);
    vdbActivateByPoint.findOrCreateTuple(true, GA_ATTRIB_POINT);
    vdbActivateByPoint.compute();

#endif

_GFEL_BEGIN
class VDBActivateByPoint : public AttribFilterWithRef0 {


public:
    using AttribFilterWithRef0::AttribFilterWithRef0;


    void
        setComputeParm(
            const exint subscribeRatio = 64,
            const exint minGrainSize   = 1024
        )
    {
        setHasComputed();
        this->subscribeRatio = subscribeRatio;
        this->minGrainSize   = minGrainSize;
    }

private:

    virtual bool
        computeCore() override
    {
        if (groupParser.isEmpty())
            return true;

        
#if GFE_DEBUG_MODE
        GA_Size numpt_debug = geoRef0->getNumPoints();
#endif

        const GA_Offset vdboff = geo->getFirstVDBoff(groupParser.getPrimitiveGroup());
        if (gfe::isInvalidOffset(vdboff))
        {
            cookparms->sopAddError(SOP_MESSAGE, "First input must contain a VDB!");
            return false;
        }
        GEO_PrimVDB* const vdbPrim = geo->getPrimitiveT<GEO_PrimVDB*>(vdboff);
        UT_ASSERT_P(vdbPrim);

        // volume primitives in different nodes in Houdini by default share the same volume tree (for memory optimization) this will make sure that we will have our own deep copy of volume tree which we can write to 
        vdbPrim->makeGridUnique();
        
        // get grid base pointer and cast it to float grid pointer
        openvdb::GridBase::Ptr vdbPtrBase = vdbPrim->getGridPtr();
        openvdb::FloatGrid::Ptr vdbPtr = openvdb::gridPtrCast<openvdb::FloatGrid>(vdbPtrBase);

        // get accessor to the float grid
        openvdb::FloatGrid::Accessor vdb_access = vdbPtr->getAccessor();

        // get a reference to transformation of the grid
        const openvdb::math::Transform& vdbGridXform = vdbPtr->transform();

        gfe::ROPageHandleT<UT_Vector3D> posRef0_ph(geoRef0->getP());
        GA_Offset start, end;
        for (GA_PageIterator pit = groupParserRef0.getPointSplittableRange().beginPages(); !pit.atEnd(); ++pit)
        {
            for (GA_Iterator it(pit.begin()); it.blockAdvance(start, end); )
            {
                posRef0_ph.setPage(start);
                for (GA_Offset elemoff = start; elemoff < end; ++elemoff)
                {
                    UT_Vector3D& pos = posRef0_ph.value(elemoff);
                    
                    // create openvdb vector with values from houdini's vector, transform it from world space to vdb's index space (based on vdb's transformation) and activate voxel at point position
                    const openvdb::Vec3d vdbpos(pos[0], pos[1], pos[2]);
                    const openvdb::Coord coord(vdbGridXform.worldToIndexCellCentered(vdbpos));
                    vdb_access.setValueOn(coord);
                }
            }
        }
        return true;
    }




public:
    
private:
    
    
    exint subscribeRatio = 64;
    exint minGrainSize   = 1024;

private:
    
    
}; // End of class VDBActivateByPoint
_GFEL_END
#endif
