
#pragma once

#ifndef __GFE_MatchAxis_h__
#define __GFE_MatchAxis_h__

#include "GFE/GFE_MatchAxis.h"

#include "GFE/GFE_GeoFilter.h"




#include "GFE/GFE_XformByAttrib.h"

class GFE_MatchAxis : public GFE_AttribFilter {


public:
    using GFE_AttribFilter::GFE_AttribFilter;
    
    void
        setComputeParm(
            const exint subscribeRatio = 64,
            const exint minGrainSize   = 64
        )
    {
        setHasComputed();
        this->subscribeRatio = subscribeRatio;
        this->minGrainSize   = minGrainSize;
    }
    
    void
        setXformVector(
            const UT_Vector3T<fpreal>& fromVec = UT_Vector3T<fpreal>(0,0,1),
            const UT_Vector3T<fpreal>& toVec   = UT_Vector3T<fpreal>(1,0,0),
            const UT_Vector3T<fpreal>& upVec   = UT_Vector3T<fpreal>(1,1,0),
            const UT_Vector3T<fpreal>& center  = UT_Vector3T<fpreal>(0,0,0)
        )
    {
        setHasComputed();
        this->fromVec = fromVec;
        this->toVec   = toVec;
        this->upVec   = upVec;
        this->center  = center;
    }
    
    SYS_FORCE_INLINE GA_Attribute* setXformAttrib(GA_Attribute* const inAttrib = nullptr)
    { return xformAttrib = !inAttrib || inAttrib->getTupleSize() != 16 || inAttrib->getStorageClass() != GA_STORECLASS_REAL || !inAttrib->getAIFTuple() ? nullptr : inAttrib; }
    
    SYS_FORCE_INLINE GA_Attribute* setXformAttrib(const bool detached = false, const GA_Storage storage = GA_STORE_INVALID, const UT_StringRef& attribName = "xform")
    { return getOutAttribArray().findOrCreateTuple(detached, GA_ATTRIB_DETAIL, GA_STORECLASS_FLOAT, storage, attribName, 16, GA_Defaults(0.0f), false); }
    
    
    
private:

    // can not use in parallel unless for each GA_Detail
    virtual bool
        computeCore() override
    {
        if (getOutAttribArray().isEmpty())
            return false;

        if (groupParser.isEmpty())
            return true;

        UT_ASSERT_MSG_P(!xformAttrib ||
                       (xformAttrib->getTupleSize() == 16 &&
                        xformAttrib->getStorageClass() == GA_STORECLASS_REAL &&
                        xformAttrib->getAIFTuple()), "not correct xform attrib type");
        
        UT_Matrix3T<fpreal> xform3;
        xform3.lookat(fromVec, toVec, upVec);

        xform.identity();
        xform.translate(-center);
        xform *= xform3;
        xform.translate(center);
        
        if (xformAttrib)
        {
            const GA_RWHandleT<UT_Matrix4T<fpreal>> xform_h(xformAttrib);
            xform_h.set(0, xform);
        }
        
        GFE_XformByAttrib xformByAttrib(geo, nullptr, cookparms);
    
        xformByAttrib.getOutAttribArray().append(getOutAttribArray());

        xformByAttrib.setXformAttrib();
        xformByAttrib.setXform4(xform);
        
        //xformByAttrib.setComputeParm(false, false, false);

        xformByAttrib.groupParser.setGroup(groupParser);
    
        xformByAttrib.compute();

        return true;
    }
    

public:
    UT_Vector3T<fpreal> fromVec = UT_Vector3T<fpreal>(0,0,1);
    UT_Vector3T<fpreal> toVec   = UT_Vector3T<fpreal>(1,0,0);
    UT_Vector3T<fpreal> upVec   = UT_Vector3T<fpreal>(1,1,0);
    UT_Vector3T<fpreal> center  = UT_Vector3T<fpreal>(0,0,0);
    
private:
    UT_Matrix4T<fpreal> xform;
    GA_Attribute* xformAttrib = nullptr;
    
    exint subscribeRatio = 64;
    exint minGrainSize = 1024;


}; // End of class GFE_MatchAxis


#endif
