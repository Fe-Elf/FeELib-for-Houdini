
#pragma once

#ifndef __GFE_SkinPieces_h__
#define __GFE_SkinPieces_h__



//#include "GFE/GFE_SkinPieces.h"

#include "GA/GA_Detail.h"

//#include "GA/GA_Types.h"

#include "GFE/GFE_Skin.h"

namespace GFE_SkinPieces {





static void
skinPieces(
    GA_Detail* const geo,
    const bool closed = false,
    const bool excludeSharedEdge = false,
    const GA_PrimitiveGroup* const endGroup = nullptr,

    const GA_Storage storage = GA_STORE_INVALID,
    const exint subscribeRatio = 32,
    const exint minGrainSize = 1024
)
{

}



} // End of namespace GFE_SkinPieces

#endif
