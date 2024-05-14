
#pragma once

#ifndef __GFE_NodeVerb_h__
#define __GFE_NodeVerb_h__

#include <GFE/NodeVerb.h>

#include <SOP/SOP_NodeVerb.h>



_GFE_BEGIN
namespace NodeVerb {

        
    static SOP_NodeVerb::CookParms newCookParms(
        const SOP_NodeVerb::CookParms& cookparms,
        const SOP_NodeParms* const nodeParms,
        SOP_NodeCache* const nodeCache = nullptr,
        GU_DetailHandle* const destgdh = nullptr,
        const UT_Array<GU_ConstDetailHandle>* const inputs = nullptr
    )
    {
        UT_Array<GU_ConstDetailHandle> nodeInputs;
        if (!inputs)
        {
            const size_t nInputs = cookparms.nInputs();
            for (size_t i = 0; i < nInputs; ++i)
            {
                nodeInputs.emplace_back(cookparms.inputGeoHandle(i));
            }
        }
        
        SOP_Node* const node = cookparms.getNode();
        const SOP_CookEngine cookEngine = node == nullptr ? SOP_COOK_COMPILED : SOP_COOK_TRADITIONAL;
        
        return SOP_NodeVerb::CookParms(
            destgdh ? *destgdh : cookparms.gdh(),
            inputs  ? *inputs  : nodeInputs,
            cookEngine, node, cookparms.getContext(),
            nodeParms, nodeCache, cookparms.error(), cookparms.depnode());
    }

        
    SYS_FORCE_INLINE static SOP_NodeVerb::CookParms newCookParms(
        const SOP_NodeVerb::CookParms* const cookparms,
        const SOP_NodeParms* const nodeParms,
        SOP_NodeCache* const nodeCache = nullptr,
        GU_DetailHandle* const destgdh = nullptr,
        const UT_Array<GU_ConstDetailHandle>* const inputs = nullptr
    )
    { return newCookParms(*cookparms, nodeParms, nodeCache, destgdh, inputs); }

    SYS_FORCE_INLINE static SOP_NodeVerb::CookParms newCookParms(
        const SOP_NodeVerb::CookParms* const cookparms,
        const SOP_NodeParms& nodeParms,
        SOP_NodeCache* const nodeCache = nullptr,
        GU_DetailHandle* const destgdh = nullptr,
        const UT_Array<GU_ConstDetailHandle>* const inputs = nullptr
    )
    { return newCookParms(*cookparms, &nodeParms, nodeCache, destgdh, inputs); }

        SYS_FORCE_INLINE static SOP_NodeVerb::CookParms newCookParms(
            const SOP_NodeVerb::CookParms* const cookparms,
            const SOP_NodeParms& nodeParms,
            const SOP_NodeVerb& nodeVerb,
            GU_DetailHandle* const destgdh = nullptr,
            const UT_Array<GU_ConstDetailHandle>* const inputs = nullptr
        )
    { return newCookParms(*cookparms, &nodeParms, nodeVerb.allocCache(), destgdh, inputs); }




} // End of namespace NodeVerb
_GFE_END
#endif
