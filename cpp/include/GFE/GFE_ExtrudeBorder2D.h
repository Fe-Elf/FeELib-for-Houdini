
#pragma once

#ifndef __GFE_ExtrudeBorder2D_h__
#define __GFE_ExtrudeBorder2D_h__

#include "GFE/GFE_ExtrudeBorder2D.h"

#include "GFE/GFE_GeoFilter.h"


#include "GU/GU_PolyExtrude2.h"


#include "GFE/GFE_Normal2D.h"

class GFE_ExtrudeBorder2D : public GFE_AttribFilter {


public:
    using GFE_AttribFilter::GFE_AttribFilter;


    void
        setComputeParm(
            const fpreal dist = 0.01,
            const bool reverseMesh = false
        )
    {
        setHasComputed();
        this->dist = dist;
        this->reverseMesh = reverseMesh;
    }

private:

    virtual bool
        computeCore() override
    {
        if (groupParser.isEmpty())
            return true;
        
        GFE_Normal2D normal2D(*geo, cookparms);
        normal2D.groupParser.setGroup(groupParser);
        normal2D.setNormal2DAttrib();
        normal2D.compute();
        
        GU_PolyExtrude2 polyExtrude(geo->asGU_Detail(), groupParser.getPrimitiveGroup());
        polyExtrude.setExtrusionMode(GU_PolyExtrude2::ExtrusionMode::POINT_NORMAL);
        polyExtrude.setPointNormalAttrib(normal2D.getOutAttribArray()[0]);
        polyExtrude.setOutputFront(false);
        polyExtrude.setOutputBack(true);
        polyExtrude.setOutputSide(true);
        
        polyExtrude.extrude(dist);
        
        return true;
    }




public:
    
    fpreal dist = 0.01;
    bool reverseMesh = false;

    
private:
    


}; // End of class GFE_ExtrudeBorder2D

#endif
