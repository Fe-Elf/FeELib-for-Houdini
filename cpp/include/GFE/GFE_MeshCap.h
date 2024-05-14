
#pragma once

#ifndef __GFE_MeshCap_h__
#define __GFE_MeshCap_h__



//#include "GFE/GFE_MeshCap.h"

#include "GA/GA_Detail.h"

#include "GU/GU_PolyFill.h"




namespace GFE_MeshCap {








static void
meshCapSingle(GA_Detail* const geo)
{
    UT_Array<GA_OffsetArray> rings;
    UT_Array<GA_OffsetArray> ringOrigs;
    GA_PrimitiveGroup* patchGroup = nullptr;
    GU_PolyFill::singlePolys(static_cast<GU_Detail*>(geo), rings, ringOrigs, patchGroup);
}








} // End of namespace GFE_MeshCap

#endif
