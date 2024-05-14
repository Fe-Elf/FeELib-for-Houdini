
#pragma once

#ifndef __GFE_AttributeCombine_h__
#define __GFE_AttributeCombine_h__

//#include "GFE/GFE_AttributeCombine.h"

#include "GA/GA_Detail.h"

#include "GA/GA_PageHandle.h"
#include "GA/GA_PageIterator.h"


#include "GFE/GFE_GroupUnion.h"
#include "GFE/GFE_GroupPromote.h"

namespace GFE_AttribCombine {

SYS_FORCE_INLINE static void attribCombine(GA_Group& group, const GA_Group* const groupRef)
{ GFE_GroupUnion::groupUnion(group, groupRef); }

    template<typename T>
    static void attribCombine(
        GA_Attribute* const attrib,
        const GA_Group* const groupRef,
        const exint subscribeRatio = 64,
        const exint minGrainSize = 1024)
    {
        const GA_SplittableRange geoSplittableRange(GA_Range(attrib->getIndexMap(), static_cast<const GA_ElementGroup*>(groupRef)));
        UTparallelFor(geoSplittableRange, [attrib](const GA_SplittableRange& r)
        {
            GA_Offset start, end;
            for (GA_Iterator it(r); it.blockAdvance(start, end); )
            {
                GA_PageHandleT<T, T, true, true, GA_Attribute, GA_ATINumeric, GA_Detail> attrib_ph(attrib);
                for (GA_Offset elemoff = start; elemoff < end; ++elemoff)
                {
                    attrib_ph.value(elemoff) = (T)1;
                }
            }
        }, subscribeRatio, minGrainSize);
    }


    template<>
    static void attribCombine<UT_StringHolder>(
        GA_Attribute* const attrib,
        const GA_Group* const groupRef,
        const exint subscribeRatio,
        const exint minGrainSize)
    {
        const GA_RWHandleS attrib_h(attrib);

        const GA_SplittableRange geoSplittableRange(GA_Range(attrib->getIndexMap(), static_cast<const GA_ElementGroup*>(groupRef)));
        UTparallelFor(geoSplittableRange, [&attrib_h](const GA_SplittableRange& r)
        {
            GA_Offset start, end;
            for (GA_Iterator it(r); it.blockAdvance(start, end); )
            {
                for (GA_Offset elemoff = start; elemoff < end; ++elemoff)
                {
                    attrib_h.set(elemoff, "1");
                }
            }
        }, subscribeRatio, minGrainSize);
    }


    static bool attribCombine(
        GA_Attribute* const attrib,
        const GA_Group* const groupRef,
        const exint subscribeRatio = 64,
        const exint minGrainSize = 1024)
    {
        //if (groupRef->classType() == GA_GROUP_EDGE)
        //{
        //}
        GA_GroupUPtr groupRefPromotedUPtr;
        const GA_Group* groupRefPromoted;
        if (attrib->getOwner() == GFE_Type::attributeOwner_groupType(groupRef->classType()))
        {
            groupRefPromoted = groupRef;
        }
        else
        {
            groupRefPromotedUPtr = GFE_GroupPromote::groupPromoteDetached(groupRef, attrib->getOwner());
            groupRefPromoted = groupRefPromotedUPtr.get();
        }

        switch (attrib->getAIFTuple()->getStorage(attrib))
        {
        // case GA_STORE_BOOL: attribCombine<bool>(attrib, groupRefPromoted, subscribeRatio, minGrainSize); break;
        case GA_STORE_UINT8: attribCombine<uint8>(attrib, groupRefPromoted, subscribeRatio, minGrainSize); break;
        case GA_STORE_INT8: attribCombine<int8>(attrib, groupRefPromoted, subscribeRatio, minGrainSize); break;
        case GA_STORE_INT16: attribCombine<int16>(attrib, groupRefPromoted, subscribeRatio, minGrainSize); break;
        case GA_STORE_INT32: attribCombine<int32>(attrib, groupRefPromoted, subscribeRatio, minGrainSize); break;
        case GA_STORE_INT64: attribCombine<int64>(attrib, groupRefPromoted, subscribeRatio, minGrainSize); break;
        case GA_STORE_REAL16: attribCombine<fpreal16>(attrib, groupRefPromoted, subscribeRatio, minGrainSize); break;
        case GA_STORE_REAL32: attribCombine<fpreal32>(attrib, groupRefPromoted, subscribeRatio, minGrainSize); break;
        case GA_STORE_REAL64: attribCombine<fpreal64>(attrib, groupRefPromoted, subscribeRatio, minGrainSize); break;
        case GA_STORE_STRING: attribCombine<UT_StringHolder>(attrib, groupRefPromoted, subscribeRatio, minGrainSize); break;
        default: UT_ASSERT_MSG(0, "unsupport storage"); return false; break;
        }
        return true;
    }

} // End of namespace GFE_AttributeCombine

#endif