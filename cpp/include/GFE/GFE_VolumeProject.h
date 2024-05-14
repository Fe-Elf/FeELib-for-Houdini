
#pragma once

#ifndef __GFE_VolumeProject_h__
#define __GFE_VolumeProject_h__

#include "GFE/GFE_VolumeProject.h"

#include "GFE/GFE_GeoFilter.h"



#include "UT/UT_StopWatch.h"
#include "UT/UT_VoxelArray.h"
#include "GU/GU_PrimVolume.h"


#if 0
    GFE_VolumeProject volumeProject(geo, cookparms);
    volumeProject.findOrCreateTuple(true, GA_ATTRIB_POINT);
    volumeProject.compute();
#endif
    
#define MIN_DIV 1e-8


class GFE_VolumeProject : public GFE_AttribFilterWithRef2 {

public:
    using GFE_AttribFilterWithRef2::GFE_AttribFilterWithRef2;


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

	
	SYS_FORCE_INLINE void bumpDataId() const override
    { geo->getPrimitiveList().bumpDataId(); }
	//{ geo->bumpAllDataIds(); }
	
private:

    virtual bool
        computeCore() override
    {
	    if (getOutAttribArray().isEmpty())
	    	return false;

    	if (groupParser.isEmpty())
    		return true;
        


    	auto&& sopparms = cookparms->parms<SOP_FeE_VolumeProject_1_0Parms>();
		

    	const GA_PrimitiveGroup* velgrp = groupParser.getPrimitiveGroup();
    	cookparms->selectInputGroup(velgrp, GA_GROUP_PRIMITIVE);



    	
    	UT_Array<GEO_PrimVolume*> velx, vely, velz;
    	UT_Array<const GEO_PrimVolume*> div;
    	UT_Array<const GEO_PrimVolume*> active;

    	
    	//const GA_PrimitiveGroup* velgrp;
    	//const GA_PrimitiveGroup* divgrp;
    	//const GA_PrimitiveGroup* lutgrp;
    	//const GA_PrimitiveGroup* activegrp;
    	//GOP_Manager gop;
    	//
    	//velgrp = nullptr;
    	//if (sopparms.getGroup().isstring())
    	//{
    	//	velgrp = gop.parsePrimitiveGroups(sopparms.getGroup(), GOP_Manager::GroupCreator(geo->asGEO_Detail(), false), /*numok=*/true, /*ordered=*/true);
    	//	if (!velgrp)
    	//	{
    	//	    cookparms->sopAddWarning(SOP_ERR_BADGROUP, sopparms.getGroup());
    	//	    return false;
    	//	}
    	//}
    	////notifyGroupParmListeners(cookparms->getNode(), 0, -1, geo, velgrp);
    	//cookparms->selectInputGroup(velgrp, GA_GROUP_PRIMITIVE);
    	//
    	//
    	//divgrp = nullptr;
    	//if (sopparms.getDivGroup().isstring())
    	//{
    	//	divgrp = gop.parsePrimitiveGroups(sopparms.getDivGroup(), GOP_Manager::GroupCreator(geoRef0), /*numok=*/true, /*ordered=*/true);
    	//	if (!divgrp)
    	//	{
    	//	    cookparms->sopAddWarning(SOP_ERR_BADGROUP, sopparms.getDivGroup());
    	//	    return false;
    	//	}
    	//}
    	//
    	//
    	//lutgrp = nullptr;
    	//if (sopparms.getLUTGroup().isstring())
    	//{
    	//	lutgrp = gop.parsePrimitiveGroups(sopparms.getLUTGroup(), GOP_Manager::GroupCreator(geoRef1), /*numok=*/true, /*ordered=*/true);
    	//	if (!lutgrp)
    	//	{
    	//	    cookparms->sopAddWarning(SOP_ERR_BADGROUP, sopparms.getLUTGroup());
    	//	    return false;
    	//	}
    	//}
    	//
    	//activegrp = nullptr;
    	//if (sopparms.getActiveGroup().isstring())
    	//{
    	//	activegrp = gop.parsePrimitiveGroups(sopparms.getActiveGroup(), GOP_Manager::GroupCreator(geoRef2), /*numok=*/true, /*ordered=*/true);
    	//	if (!activegrp)
    	//	{
    	//	    cookparms->sopAddWarning(SOP_ERR_BADGROUP, sopparms.getActiveGroup());
    	//	    return false;
    	//	}
    	//}

    	GEO_PrimVolume* vel[3] = {0, 0, 0};

    	// Each velocity triple is collated.
    	GEO_Primitive* prim;
    	int	numvel = 0;
    	GA_FOR_ALL_OPT_GROUP_PRIMITIVES(geo, velgrp, prim)
    	{
    		if (prim->getTypeId() == GEO_PRIMVOLUME)
    		{
    			vel[numvel++] = (GEO_PrimVolume*)prim;

    			if (numvel == 3)
    			{
    				// Complete set
    				velx.append(vel[0]);
    				vely.append(vel[1]);
    				velz.append(vel[2]);
    				numvel = 0;
    			}
    		}
    	}
    	// Check to see if we have a dangling pair.
    	if (numvel)
    		cookparms->sopAddWarning(SOP_MESSAGE, "Unmatched veloicty volume ignored; provide matching triples of velocity volumes.");

    	
		div.append(geoRef0->getFirstHoudiniVolumePrimitive(groupParserRef0.getPrimitiveGroup()));

    	
    	const GEO_PrimVolume* lut[3] = { 0, 0, 0 };
    	geoRef1->getVectorHoudiniVolumePrimitive(lut, groupParserRef1.getPrimitiveGroup());
    	if (!lut[2])
    	{
    		cookparms->sopAddError(SOP_MESSAGE, "Insufficient lut volume specified.");
    		return false;
    	}

    	
	    if (geoRef2)
			active.append(geoRef2->getFirstHoudiniVolumePrimitive(groupParserRef2.getPrimitiveGroup()));




	    if (velx.size() != div.size())
			cookparms->sopAddWarning(SOP_MESSAGE, "Unmatched sets of velocity and divergence volumes specified.  Unmatched ignored.");

	    if (active.size() && (active.size() != div.size()))
	    {
			cookparms->sopAddError(SOP_MESSAGE, "Unmatched sets of divergence and active volumes specified.  Abandoning.");
			return false;
	    }

	    int	npass = std::min(velx.size(), div.size());
	    for (int pass = 0; pass < npass; pass++)
	    {
	    	UT_VoxelArrayReadHandleF activeh;
			UT_VoxelArrayReadHandleF divh  = div[pass]->getVoxelHandle();
			UT_VoxelArrayReadHandleF lutxh = lut[0]->getVoxelHandle();
			UT_VoxelArrayReadHandleF lutyh = lut[1]->getVoxelHandle();
			UT_VoxelArrayReadHandleF lutzh = lut[2]->getVoxelHandle();

			UT_VoxelArrayWriteHandleF vxh = velx[pass]->getVoxelWriteHandle();
			UT_VoxelArrayWriteHandleF vyh = vely[pass]->getVoxelWriteHandle();
			UT_VoxelArrayWriteHandleF vzh = velz[pass]->getVoxelWriteHandle();

			const UT_VoxelArrayF* activev = 0;
			if (pass < active.size())
			{
			    activeh = active[pass]->getVoxelHandle();
			    activev = &*activeh;
			}

			// First, we apply our LUT approximation for the near field.
			applyVelLUT(&*vxh, &*divh, activev, &*lutxh, velx[pass]->getVoxelSize());
			applyVelLUT(&*vyh, &*divh, activev, &*lutyh, vely[pass]->getVoxelSize());
			applyVelLUT(&*vzh, &*divh, activev, &*lutzh, velz[pass]->getVoxelSize());
	    	
			if (!sopparms.getDoMIP())
				continue;
			
		    UT_Array<UT_VoxelArrayV4*> pos_mip, neg_mip;

		    buildMipMap(pos_mip, neg_mip, div[pass]);

		    if (sopparms.getMipBy4())
		    {
				applyMipMap4(0, velx[pass], &*vxh, activev, pos_mip, neg_mip);
				applyMipMap4(1, vely[pass], &*vyh, activev, pos_mip, neg_mip);
				applyMipMap4(2, velz[pass], &*vzh, activev, pos_mip, neg_mip);
		    }
		    else
		    {
				applyMipMap(0, velx[pass], &*vxh, activev, pos_mip, neg_mip);
				applyMipMap(1, vely[pass], &*vyh, activev, pos_mip, neg_mip);
				applyMipMap(2, velz[pass], &*vzh, activev, pos_mip, neg_mip);
		    }

		    for (auto && map : pos_mip)
				delete map;
		    for (auto && map : neg_mip)
				delete map;
	    }
    	
        return true;
    }



		
	void applyVelLUT(
		UT_VoxelArrayF* vel,
		const UT_VoxelArrayF* div,
		const UT_VoxelArrayF* active,
		const UT_VoxelArrayF* lut,
		UT_Vector3 voxelsize
	)
	{
	    auto&& sopparms = cookparms->parms<SOP_FeE_VolumeProject_1_0Parms>();
	    bool doround = sopparms.getLUTRound();

	    UT_StopWatch watch;
	    watch.start();

	    // Verify lut is big enough.
	    if (sopparms.getLUTCenter() - sopparms.getLUTRad() < 0)
	    {
			cookparms->sopAddError(SOP_MESSAGE, "LUT range invalid");
			return;
	    }
	    if (sopparms.getLUTCenter() + sopparms.getLUTRad() >= SYSmax(lut->getXRes(), lut->getYRes(), lut->getZRes()) )
	    {
			cookparms->sopAddError(SOP_MESSAGE, "LUT range exceeds provided table");
			return;
	    }
	    if (sopparms.getLUTCenter() - sopparms.getLUTRad() < 0)
	    {
			cookparms->sopAddError(SOP_MESSAGE, "LUT range exceeds provided table");
			return;
	    }

	    int	divxres = div->getXRes();
	    int	divyres = div->getYRes();
	    int	divzres = div->getZRes();

	    // Assume our lookup table is built with unit-sized voxels.
	    // Then it has the update value for unit voxels, so we need
	    // to divide by our voxel size.
	    // Correspondingly, we divide by 3 for 3 dimensions we sum divergence
	    // in to build our correction matrix.
	    float lutmagic = sopparms.getLUTMagic();

	    lutmagic /= voxelsize.x();
	    lutmagic /= 3;

	    UTparallelForEachNumber(vel->numTiles(), [&](const UT_BlockedRange<int> &r)
	    {
		    const exint curtile = r.begin();
		    UT_VoxelTileIteratorF vit;
		    vit.setLinearTile(curtile, vel);
		    for (vit.rewind(); !vit.atEnd(); vit.advance())
		    {
			    float delta = 0;
			    int	rad = sopparms.getLUTRad();

			    if (active && active->getValue(vit.x(), vit.y(), vit.z()) < 0.5)
			    {
				    if (sopparms.getZeroInactive())
					    vit.setValue(0);
				    continue;
			    }

			    int radzmin = -rad;
			    int radzmax = rad;
			    // Bump so we fit fully in the next level mip map.
			    if (doround)
			    {
				    if ( (vit.z() - rad) & 1 )
					    radzmin--;
				    if ( !( (vit.z() + rad) & 1 ) )
					    radzmax++;
			    }

			    for (int z = radzmin; z <= radzmax; z++)
			    {
				    if (vit.z() + z < 0)
					    continue;
				    if (vit.z() + z >= divzres)
					    continue;

				    int radymin = -rad;
				    int radymax = rad;
				    // Bump so we fit fully in the next level mip map.
				    if (doround)
				    {
					    if ( (vit.y() - rad) & 1 )
						    radymin--;
					    if ( !( (vit.y() + rad) & 1 ) )
						    radymax++;
				    }

				    for (int y = radymin; y <= radymax; y++)
				    {
					    if (vit.y() + y < 0)
						    continue;
					    if (vit.y() + y >= divyres)
						    continue;

					    int radxmin = -rad;
					    int radxmax = rad;
					    // Bump so we fit fully in the next level mip map.
					    if (doround)
					    {
						    if ( (vit.x() - rad) & 1 )
							    radxmin--;
						    if ( !( (vit.x() + rad) & 1 ) )
							    radxmax++;
					    }

					    for (int x = radxmin; x <= radxmax; x++)
					    {
						    if (vit.x() + x < 0)
							    continue;
						    if (vit.x() + x >= divxres)
							    continue;

						    // Compute the divergence relative index.
						    float val = (*div)(vit.x()+x, vit.y()+y, vit.z()+z);

						    val *= lutmagic;
						    val *= (*lut)(sopparms.getLUTCenter() - x,
						                  sopparms.getLUTCenter() - y,
						                  sopparms.getLUTCenter() - z);

						    delta += val;
					    }
				    }
			    }

			    // If we ended up with a delta, update our current voxel.
			    if (delta)
				    vit.setValue(vit.getValue() - delta);
		    }
		});
    	//}, subscribeRatio, minGrainSize);

	    UTdebugPrintCd(none, "Apply LUT Time:", watch.stop());
	}


	void buildMipMap(UT_Array<UT_VoxelArrayV4*>& pos_mip, UT_Array<UT_VoxelArrayV4*>& neg_mip, const GEO_PrimVolume* div)
	{
	    UT_StopWatch watch;
	    watch.start();

	    UT_VoxelArrayReadHandleF divh = div->getVoxelHandle();

	    const UT_VoxelArrayF* div_vox = &*divh;
	    GEO_PrimVolumeXform	divxform = div->getIndexSpaceTransform(*div_vox);
	    UT_ASSERT(pos_mip.size() == 0);
	    UT_ASSERT(neg_mip.size() == 0);

	    // Base level we have to initialize specially as we must
	    // compute actual coordinates...
	    pos_mip.append(new UT_VoxelArrayV4());
	    neg_mip.append(new UT_VoxelArrayV4());

	    UT_VoxelArrayV4	*pos, *neg;

	    pos = pos_mip(0);
	    neg = neg_mip(0);

	    pos->size((div_vox->getXRes()+1) / 2,
			      (div_vox->getYRes()+1) / 2,
			      (div_vox->getZRes()+1) / 2);
    	
	    neg->size((div_vox->getXRes()+1) / 2,
			      (div_vox->getYRes()+1) / 2,
			      (div_vox->getZRes()+1) / 2);

	    UTparallelForEachNumber(pos->numTiles(), [&](const UT_BlockedRange<int> &r)
	    {
		    const exint curtile = r.begin();
		    UT_VoxelTileIteratorV4 vit;
		    vit.setLinearTile(curtile, pos);
		    for (vit.rewind(); !vit.atEnd(); vit.advance())
		    {
			    // Parents are vit * 2 & vit * 2 + 1
			    UT_Vector4		pos_v, neg_v;
			    pos_v = 0;
			    neg_v = 0;

			    for (int dz = 0; dz <= 1; dz++)
			    {
				    int z = vit.z() * 2 + dz;
				    if (z >= div_vox->getZRes())
					    continue;
				    for (int dy = 0; dy <= 1; dy++)
				    {
					    int y = vit.y() * 2 + dy;
					    if (y >= div_vox->getYRes())
						    continue;
					    for (int dx = 0; dx <= 1; dx++)
					    {
						    int x = vit.x() * 2 + dx;
						    if (x >= div_vox->getXRes())
							    continue;

						    float	div_v = (*div_vox)(x, y, z);

						    // Note exact equality to zero is ignored.
						    if (div_v > MIN_DIV)
						    {
							    UT_Vector3	p = divxform.fromVoxelSpace(UT_Vector3(x, y, z));
							    p *= div_v;
							    pos_v.x() += p.x();
							    pos_v.y() += p.y();
							    pos_v.z() += p.z();
							    pos_v.w() += div_v;
						    }
						    else if (div_v < -MIN_DIV)
						    {
							    UT_Vector3	p = divxform.fromVoxelSpace(UT_Vector3(x, y, z));
							    p *= -div_v;
							    neg_v.x() += p.x();
							    neg_v.y() += p.y();
							    neg_v.z() += p.z();
							    neg_v.w() += -div_v;
						    }
					    }
				    }
			    }

			    pos->setValue(vit.x(), vit.y(), vit.z(), pos_v);
			    neg->setValue(vit.x(), vit.y(), vit.z(), neg_v);
		    }
		});
    	//}, subscribeRatio, minGrainSize);

	    // We've now built mip-map level 0, which is half the res of
	    // the incoming div but is of v4 dipole format.  We now
	    // repeatedly decimate until it disappears.
	    while (1)
	    {
			UT_VoxelArrayV4 *old_pos, *old_neg;
			old_pos = pos_mip.last();
			old_neg = neg_mip.last();

			// We compute 6 voxels on each layer, so as soon as we get down
			// to max 6 on each side, there is no point going farther.
			if (SYSmax(old_pos->getXRes(), old_pos->getYRes(), old_pos->getZRes()) <= 6)
			{
			    // We've converged
			    break;
			}

			pos_mip.append(new UT_VoxelArrayV4());
			neg_mip.append(new UT_VoxelArrayV4());
			pos = pos_mip.last();
			neg = neg_mip.last();
			pos->size((old_pos->getXRes()+1) / 2,
					  (old_pos->getYRes()+1) / 2,
					  (old_pos->getZRes()+1) / 2);
			neg->size((old_neg->getXRes()+1) / 2,
					  (old_neg->getYRes()+1) / 2,
					  (old_neg->getZRes()+1) / 2);

			UT_VoxelArrayIteratorV4	vit(pos);
			for (vit.rewind(); !vit.atEnd(); vit.advance())
			{
			    // Parents are vit * 2 & vit * 2 + 1
			    UT_Vector4		pos_v, neg_v;
			    pos_v = 0;
			    neg_v = 0;

			    for (int dz = 0; dz <= 1; dz++)
			    {
					int z = vit.z() * 2 + dz;
					if (z >= old_pos->getZRes())
					    continue;
					for (int dy = 0; dy <= 1; dy++)
					{
					    int y = vit.y() * 2 + dy;
					    if (y >= old_pos->getYRes())
						continue;
					    for (int dx = 0; dx <= 1; dx++)
					    {
							int x = vit.x() * 2 + dx;
							if (x >= old_pos->getXRes())
							    continue;

							// Because they are already weighted,
							// it is trivial to blend.
							// (Maybe even use UT_MipMap here...)
							pos_v += (*old_pos)(x, y, z);
							neg_v += (*old_neg)(x, y, z);
					    }
					}
			    }
			    // Write back.
			    pos->setValue(vit.x(), vit.y(), vit.z(), pos_v);
			    neg->setValue(vit.x(), vit.y(), vit.z(), neg_v);
			}
	    }

	    UTdebugPrintCd(none, "Build MipMap time:", watch.stop());
	    UTdebugPrintCd(none, "Build div mip map, levels", pos_mip.entries());
	}
		
	void applyMipMap(
		const int axis,
		GEO_PrimVolume* prim_vel,
		UT_VoxelArrayF* vel, 
		const UT_VoxelArrayF* active,
		const UT_Array<UT_VoxelArrayV4*>& pos_mip,
		const UT_Array<UT_VoxelArrayV4*>& neg_mip
	)
	{
	    auto&& sopparms = cookparms->parms<SOP_FeE_VolumeProject_1_0Parms>();

	    UT_StopWatch		watch;
	    watch.start();

	    GEO_PrimVolumeXform		velxform = prim_vel->getIndexSpaceTransform(*vel);

	    float mipmagic = sopparms.getMIPMagic();
	    mipmagic = 1.0 / mipmagic;

	    UT_Vector3	voxelsize = prim_vel->getVoxelSize();

	    // See applyMipMap4 for derivation.
	    mipmagic *= voxelsize.x();
	    mipmagic /= 4 * M_PI;

	    UTparallelForEachNumber(vel->numTiles(), [&](const UT_BlockedRange<int> &r)
	    {
			const exint curtile = r.begin();
			UT_VoxelTileIteratorF vit;
			vit.setLinearTile(curtile, vel);
			for (vit.rewind(); !vit.atEnd(); vit.advance())
			{
				if (active && active->getValue(vit.x(), vit.y(), vit.z()) < 0.5)
				    continue;

			    UT_Vector3	velpos = velxform.fromVoxelSpace(UT_Vector3(vit.x(), vit.y(), vit.z()));

			    bool inspect = false;
			    //if (vit.x() == 21 &&
				//	  vit.y() == 11 &&
				//	  vit.z() == 13)
				//	  inspect = false;

			    // INCLUSIVE range of things already computed
			    UT_Vector3I	minvalid, maxvalid;

			    // Our LUT has already fully computed any of our tiles touched
			    // by -rad .. rad inclusive.
			    minvalid.x() = vit.x() - sopparms.getLUTRad();
			    maxvalid.x() = vit.x() + sopparms.getLUTRad();
			    minvalid.y() = vit.y() - sopparms.getLUTRad();
			    maxvalid.y() = vit.y() + sopparms.getLUTRad();
			    minvalid.z() = vit.z() - sopparms.getLUTRad();
			    maxvalid.z() = vit.z() + sopparms.getLUTRad();

			    // It is correct to reduce this & round to zero.  If the min
			    // is short, it would have been expanded, and if the max does
			    // not end with a 1, it would have been expanded.
			    minvalid.x() /= 2;
			    minvalid.y() /= 2;
			    minvalid.z() /= 2;
			    maxvalid.x() /= 2;
			    maxvalid.y() /= 2;
			    maxvalid.z() /= 2;

			    if (inspect)
			    {
					UTdebugPrintCd(none, "VIT:", vit.x(), vit.y(), vit.z());
					UTdebugPrintCd(none, "Already computed:", minvalid, maxvalid);
					UTdebugPrintCd(none, "Vel pos", velpos);
			    }

			    float		delta = 0;
			    for (int pass = 0; pass < pos_mip.entries(); pass++)
			    {
				auto && pos = pos_mip(pass);
				auto && neg = neg_mip(pass);

				if (inspect)
				    UTdebugPrintCd(none, "pass", pass);

				// The minvalid now contains the inclusive box of all
				// of our tile sthat are already computed.
				// We need to find out what 6x6 neighbour hood in our
				// box we need to compute for the next level to be happy.
				// 
				// For the next level to be happy, we want a 3x3 neighbourhood
				// centered on vit to already be evaluated.  So if vit is 0,
				// we want -1, 0, 1.  Then, in local space, this is -2, 3
				// as the inclusive range.
				//   | . -1 . | .  0 . | .  1 . |
				//   | -2 |-1 |  0 | 1 | 2  | 3 |
				UT_Vector3I		mingoal, maxgoal;
				mingoal.x() = (vit.x() >> (pass+2)) - 1;
				mingoal.y() = (vit.y() >> (pass+2)) - 1;
				mingoal.z() = (vit.z() >> (pass+2)) - 1;
				maxgoal.x() = (vit.x() >> (pass+2)) + 1;
				maxgoal.y() = (vit.y() >> (pass+2)) + 1;
				maxgoal.z() = (vit.z() >> (pass+2)) + 1;
				// Convert from parent's space to ours.
				mingoal *= 2;
				maxgoal *= 2;
				maxgoal += 1;		// Inclusive.

				if (inspect)
				    UTdebugPrintCd(none, "Need to compute", mingoal, maxgoal);

				// Clamp.
				mingoal.x() = SYSmax(mingoal.x(), 0);
				mingoal.y() = SYSmax(mingoal.y(), 0);
				mingoal.z() = SYSmax(mingoal.z(), 0);
				maxgoal.x() = SYSmin(maxgoal.x(), pos->getXRes()-1);
				maxgoal.y() = SYSmin(maxgoal.y(), pos->getYRes()-1);
				maxgoal.z() = SYSmin(maxgoal.z(), pos->getZRes()-1);

				// Final pass needs to compute everything.
				if (pass == pos_mip.entries()-1)
				{
				    mingoal.x() = 0;
				    mingoal.y() = 0;
				    mingoal.z() = 0;
				    maxgoal.x() = pos->getXRes()-1;
				    maxgoal.y() = pos->getYRes()-1;
				    maxgoal.z() = pos->getZRes()-1;
				}

				if (inspect)
				    UTdebugPrintCd(none, "Clamped to compute", mingoal, maxgoal);

				// Inclusive loop
				for (int z = mingoal.z(); z <= maxgoal.z(); z++)
				{
				    bool	ztest = (z >= minvalid.z() && z <= maxvalid.z());

				    for (int y = mingoal.y(); y <= maxgoal.y(); y++)
				    {
						bool	ytest = (y >= minvalid.y() && y <= maxvalid.y());
						for (int x = mingoal.x(); x <= maxgoal.x(); x++)
						{
						    bool	xtest = (x >= minvalid.x() && x <= maxvalid.x());

						    if (ztest && ytest && xtest)
						    {
							// Already computed at base level
							// (should be about 27 out of 216.)
							continue;
						    }

						    // Read our two dipoles, normalize, and compute
						    // a value.
						    UT_Vector4	pos_v = (*pos)(x, y, z);
						    if (pos_v.w())
						    {
							UT_Vector3		p;
							p = UT_Vector3(pos_v);
							p /= pos_v.w();

							if (inspect)
							    UTdebugPrintCd(none,"Positive pole at", p, "weight", pos_v.w());

							// We now have the external position p,
							// the local @P equivalent of velpos
							p -= velpos;

							float	displen = p.length();
							float	paxis = p(axis);

							// We want to divide normalized disp by
							// r^2, so this comes to r^3
							paxis /= (displen*displen*displen);
							// Scale by magic
							paxis *= mipmagic;

							delta += pos_v.w() * paxis;
						    }
						    UT_Vector4	neg_v = (*neg)(x, y, z);
						    if (neg_v.w())
						    {
							UT_Vector3		p;
							p = UT_Vector3(neg_v);
							p /= neg_v.w();

							if (inspect)
							    UTdebugPrintCd(none,"Negative pole at", p, "weight", pos_v.w());
							// We now have the external position p,
							// the local @P equivalent of velpos
							p -= velpos;

							float	displen = p.length();
							float	paxis = p(axis);

							// We want to divide normalized disp by
							// r^2, so this comes to r^3
							paxis /= (displen*displen*displen);
							// Scale by magic
							paxis *= mipmagic;

							delta -= neg_v.w() * paxis;
						    }
						}
				    }
				}

				// We now have [mingoal..maxgoal] computed, so update our valid
				// list.
				minvalid = mingoal;
				maxvalid = maxgoal;
				minvalid.x() /= 2;
				minvalid.y() /= 2;
				minvalid.z() /= 2;
				maxvalid.x() /= 2;
				maxvalid.y() /= 2;
				maxvalid.z() /= 2;
			    }

			    // If we ended up with a delta, update our current voxel.
			    if (delta)
					vit.setValue(vit.getValue() + delta);
			}
		});
    	//}, subscribeRatio, minGrainSize);

	    UTdebugPrintCd(none,"MipMap Time:", watch.stop());
	}
		
	void applyMipMap4(
		int axis,
		GEO_PrimVolume* prim_vel,
		UT_VoxelArrayF* vel, 
		const UT_VoxelArrayF* active,
		const UT_Array<UT_VoxelArrayV4*>& pos_mip,
		const UT_Array<UT_VoxelArrayV4*>& neg_mip)
	{
	    auto&& sopparms = cookparms->parms<SOP_FeE_VolumeProject_1_0Parms>();

	    GEO_PrimVolumeXform velxform = prim_vel->getIndexSpaceTransform(*vel);

	    float mipmagic = sopparms.getMIPMagic();
	    mipmagic = 1 / mipmagic;

	    // We compute 1/r^2, but this is with normalized voxels.
	    // So must apply voxel^2 to normalize.
	    // Divergence is normalized as well, however, so we cancel out
	    // one voxel size from this.
	    // Finally, surface of a sphere is 4 * M_PI.
	    
	    UT_Vector3	voxelsize = prim_vel->getVoxelSize();

	    mipmagic *= voxelsize.x();

	    // Unit of divergence applied to surface of a sphere gives 4 PI
	    mipmagic /= 4 * M_PI;

	    UT_StopWatch watch;
	    watch.start();

	    UTparallelForEachNumber(vel->numTiles(), [&](const UT_BlockedRange<int> &r)
	    {
		    const exint curtile = r.begin();
		    UT_VoxelTileIteratorF vit;
		    vit.setLinearTile(curtile, vel);
		    for (vit.rewind(); !vit.atEnd(); vit.advance())
		    {
			    if (active && active->getValue(vit.x(), vit.y(), vit.z()) < 0.5)
				    continue;

			    UT_Vector3 velpos = velxform.fromVoxelSpace(UT_Vector3(vit.x(), vit.y(), vit.z()));

			    bool inspect = false;
			    // if (vit.x() == 21 &&
				//     vit.y() == 11 &&
				//     vit.z() == 13)
				//     inspect = false;

			    // INCLUSIVE range of things already computed
			    UT_Vector3I	minvalid, maxvalid;

			    // Our LUT has already fully computed any of our tiles touched
			    // by -rad .. rad inclusive.
			    minvalid.x() = vit.x() - sopparms.getLUTRad();
			    maxvalid.x() = vit.x() + sopparms.getLUTRad();
			    minvalid.y() = vit.y() - sopparms.getLUTRad();
			    maxvalid.y() = vit.y() + sopparms.getLUTRad();
			    minvalid.z() = vit.z() - sopparms.getLUTRad();
			    maxvalid.z() = vit.z() + sopparms.getLUTRad();

			    // It is correct to reduce this & round to zero.  If the min
			    // is short, it would have been expanded, and if the max does
			    // not end with a 1, it would have been expanded.
			    minvalid.x() /= 2;
			    minvalid.y() /= 2;
			    minvalid.z() /= 2;
			    maxvalid.x() /= 2;
			    maxvalid.y() /= 2;
			    maxvalid.z() /= 2;

			    if (inspect)
			    {
					UTdebugPrintCd(none, "VIT:", vit.x(), vit.y(), vit.z());
					UTdebugPrintCd(none, "Already computed:", minvalid, maxvalid);
					UTdebugPrintCd(none, "Vel pos", velpos);
			    }
		    	
			    float delta = 0;
			    for (int pass = 0; pass < pos_mip.entries(); pass++)
			    {
				    auto&& pos = pos_mip(pass);
				    auto&& neg = neg_mip(pass);

				    if (inspect)
					    UTdebugPrintCd(none, "pass", pass);

				    // The minvalid now contains the inclusive box of all
				    // of our tile sthat are already computed.
				    // We need to find out what 4x4 neighbour hood in our
				    // box we need to compute for the next level to be happy.
				    // 
				    // For the next level to be happy, we want a 2x2 neighbourhood
				    // so vit is closest to that neighbours node.
				    // We want a [-1, 0] if we will round down and
				    // [0, 1] if we will round up.
				    // So start iwth [-1, 0] and add one if vit will round up,
				    // which is if its first decimal bit is 1.
				    UT_Vector3I	mingoal, maxgoal;
				    mingoal.x() = (vit.x() >> (pass+2)) - 1;
				    mingoal.y() = (vit.y() >> (pass+2)) - 1;
				    mingoal.z() = (vit.z() >> (pass+2)) - 1;
				    maxgoal.x() = (vit.x() >> (pass+2));
				    maxgoal.y() = (vit.y() >> (pass+2));
				    maxgoal.z() = (vit.z() >> (pass+2));
				    if ( (vit.x() >> (pass+1)) & 1)
				    {
					    // Round up.
					    mingoal.x()++;
					    maxgoal.x()++;
				    }
				    if ( (vit.y() >> (pass+1)) & 1)
				    {
					    // Round up.
					    mingoal.y()++;
					    maxgoal.y()++;
				    }
				    if ( (vit.z() >> (pass+1)) & 1)
				    {
					    // Round up.
					    mingoal.z()++;
					    maxgoal.z()++;
				    }
				    // Convert from parent's space to ours.
				    mingoal *= 2;
				    maxgoal *= 2;
				    maxgoal += 1;		// Inclusive.

				    if (inspect)
					    UTdebugPrintCd(none, "Need to compute", mingoal, maxgoal);

				    // Clamp.
				    mingoal.x() = SYSmax(mingoal.x(), 0);
				    mingoal.y() = SYSmax(mingoal.y(), 0);
				    mingoal.z() = SYSmax(mingoal.z(), 0);
				    maxgoal.x() = SYSmin(maxgoal.x(), pos->getXRes()-1);
				    maxgoal.y() = SYSmin(maxgoal.y(), pos->getYRes()-1);
				    maxgoal.z() = SYSmin(maxgoal.z(), pos->getZRes()-1);

				    // Final pass needs to compute everything.
				    if (pass == pos_mip.entries()-1)
				    {
					    mingoal.x() = 0;
					    mingoal.y() = 0;
					    mingoal.z() = 0;
					    maxgoal.x() = pos->getXRes()-1;
					    maxgoal.y() = pos->getYRes()-1;
					    maxgoal.z() = pos->getZRes()-1;
				    }

				    if (inspect)
					    UTdebugPrintCd(none, "Clamped to compute", mingoal, maxgoal);

				    // Inclusive loop
				    for (int z = mingoal.z(); z <= maxgoal.z(); z++)
				    {
					    bool ztest = (z >= minvalid.z() && z <= maxvalid.z());

					    for (int y = mingoal.y(); y <= maxgoal.y(); y++)
					    {
						    bool ytest = (y >= minvalid.y() && y <= maxvalid.y());
						    for (int x = mingoal.x(); x <= maxgoal.x(); x++)
						    {
							    bool xtest = (x >= minvalid.x() && x <= maxvalid.x());

							    if (ztest && ytest && xtest)
							    {
								    // Already computed at base level
								    // (should be about 8 out of 64.)
								    continue;
							    }

							    // Read our two dipoles, normalize, and compute
							    // a value.
							    UT_Vector4	pos_v = (*pos)(x, y, z);
							    if (pos_v.w())
							    {
								    UT_Vector3		p;
								    p = UT_Vector3(pos_v);
								    p /= pos_v.w();

								    if (inspect)
									    UTdebugPrintCd(none,"Positive pole at", p, "weight", pos_v.w());

								    // We now have the external position p,
								    // the local @P equivalent of velpos
								    p -= velpos;

								    float	displen = p.length();
								    float	paxis = p(axis);

								    // We want to divide normalized disp by
								    // r^2, so this comes to r^3
								    paxis /= (displen*displen*displen);
								    // Scale by magic
								    paxis *= mipmagic;

								    delta += pos_v.w() * paxis;
							    }
							    UT_Vector4	neg_v = (*neg)(x, y, z);
							    if (neg_v.w())
							    {
								    UT_Vector3		p;
								    p = UT_Vector3(neg_v);
								    p /= neg_v.w();

								    if (inspect)
									    UTdebugPrintCd(none,"Negative pole at", p, "weight", pos_v.w());
								    // We now have the external position p,
								    // the local @P equivalent of velpos
								    p -= velpos;

								    float displen = p.length();
								    float paxis = p(axis);

								    // We want to divide normalized disp by
								    // r^2, so this comes to r^3
								    paxis /= (displen*displen*displen);
								    // Scale by magic
								    paxis *= mipmagic;

								    delta -= neg_v.w() * paxis;
							    }
						    }
					    }
				    }

				    // We now have [mingoal..maxgoal] computed, so update our valid
				    // list.
				    minvalid = mingoal;
				    maxvalid = maxgoal;
				    minvalid.x() /= 2;
				    minvalid.y() /= 2;
				    minvalid.z() /= 2;
				    maxvalid.x() /= 2;
				    maxvalid.y() /= 2;
				    maxvalid.z() /= 2;
			    }

			    // If we ended up with a delta, update our current voxel.
			    if (delta)
				    vit.setValue(vit.getValue() + delta);
		    }
	    });
	    //}, subscribeRatio, minGrainSize);

	    UTdebugPrintCd(none,"MipMap4 Time:", watch.stop());
	}



public:
	bool doMIP  = false;
	bool mipBy4 = false;

private:
    
    
    exint subscribeRatio = 64;
    exint minGrainSize   = 1024;

private:

}; // End of class GFE_VolumeProject





#endif
