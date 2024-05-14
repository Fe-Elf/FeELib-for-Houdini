
#pragma once

#ifndef __GFE_Intersection_h__
#define __GFE_Intersection_h__

#include "GFE/GFE_Intersection.h"

#include "GFE/GFE_GeoFilter.h"




#include "GFE/GFE_Enumerate.h"
#include "GFE/GFE_NodeVerb.h"
#include "SOP/SOP_IntersectionStitch.proto.h"
#include "SOP/SOP_IntersectionAnalysis.proto.h"

class GFE_Intersection : public GFE_GeoFilterWithRef1
{
public:
    enum OutType
    {
        Point,
        FirstGeo,
        SecondGeo,
        DualGeo,
    };

    
    using GFE_GeoFilterWithRef1::GFE_GeoFilterWithRef1;
    
    void
        setComputeParm(
            const OutType outType = OutType::Point
        )
    {
        setHasComputed();
        
        this->outType = outType;
    }
    
    void
        setIntersectionAnalysisComputeParm(
            const bool triangulateMesh = false,
            const bool detectVertexIntersection = false,
            const bool outIntersectionSegment   = false
        )
    {
        setHasComputed();
        
        this->triangulateMesh = triangulateMesh;
        this->detectVertexIntersection = detectVertexIntersection;
        this->outIntersectionSegment   = outIntersectionSegment;
    }
    
    void
        setIntersectionStitchComputeParm(
            const bool splitCurve      = false,
            const bool repairResult    = false,
            const exint subscribeRatio = 64,
            const exint minGrainSize   = 1024
        )
    {
        setHasComputed();
        
        this->splitCurve     = splitCurve;
        this->repairResult   = repairResult;
        this->subscribeRatio = subscribeRatio;
        this->minGrainSize   = minGrainSize;
    }
    
    SYS_FORCE_INLINE void setTolerance()
    { useTol = false; }
    
    SYS_FORCE_INLINE void setTolerance(const bool tolerance)
    { useTol = true; this->tolerance = tolerance; }


    
    //SYS_FORCE_INLINE void setInsertPoint(const bool outSecondInputGeo)
    //{ insertPoint = false; this->outSecondInputGeo = outSecondInputGeo; }
    
    //SYS_FORCE_INLINE void setInsertPoint()
    //{ insertPoint = true; }


    
#define __GFE_Intersection_Set_Attrib_SPECILIZATION(AttribNameLowner, AttribNameUpper)                                                         \
        private:                                                                                                                                     \
            const GA_Attribute* AttribNameLowner##Attrib = nullptr;                                                                                  \
            const char* AttribNameLowner##AttribName = nullptr;                                                                                      \
        public:                                                                                                                                      \
            SYS_FORCE_INLINE void set##AttribNameUpper##Attrib(const GA_Attribute* const inAttrib)                                                   \
            { AttribNameLowner##Attrib = inAttrib->getOwner()==GA_ATTRIB_POINT && &inAttrib->getDetail() == geoRef1 ? inAttrib : nullptr; }          \
                                                                                                                                                     \
            const GA_Attribute* set##AttribNameUpper##Attrib(const bool outAttrib = true, const char* const name = nullptr)                          \
            {                                                                                                                                        \
                if (geoRef1)                                                                                                                         \
                {                                                                                                                                    \
                    AttribNameLowner##AttribName = outAttrib ? name : nullptr;                                                                       \
                    return AttribNameLowner##Attrib = geoRef1->findPointAttribute(name);                                                             \
                }                                                                                                                                    \
                else                                                                                                                                 \
                {                                                                                                                                    \
                    AttribNameLowner##AttribName = outAttrib ? name : nullptr;                                                                       \
                    return AttribNameLowner##Attrib = nullptr;                                                                                       \
                }                                                                                                                                    \
            }                                                                                                                                        \
                                                                                                                                                     \
            SYS_FORCE_INLINE const GA_Attribute* set##AttribNameUpper##Attrib(const bool outAttrib, const UT_StringRef& name)                        \
            { return set##AttribNameUpper##Attrib(outAttrib, name.c_str()); }                                                                        \
        



    __GFE_Intersection_Set_Attrib_SPECILIZATION(inputnum, Inputnum)
    __GFE_Intersection_Set_Attrib_SPECILIZATION(primnum,  Primnum)
    __GFE_Intersection_Set_Attrib_SPECILIZATION(primuvw,  Primuvw)
    __GFE_Intersection_Set_Attrib_SPECILIZATION(ptnum,    Ptnum)

    #undef __GFE_Intersection_Set_Attrib_SPECILIZATION
    

    
        //SYS_FORCE_INLINE void setInputnumAttrib(const GA_Attribute* const inAttrib)
        //{ inputnumAttrib = inAttrib->getOwner()==GA_ATTRIB_POINT && &inAttrib->getDetail() == geoRef1 ? inAttrib : nullptr; }
        //
        //SYS_FORCE_INLINE const GA_Attribute* setInputnumAttrib(const UT_StringRef& name)
        //{ return inputnumAttrib = geoRef1->findPointAttribute(name); }
        //
        //
        //SYS_FORCE_INLINE void setPrimnumAttrib(const GA_Attribute* const inAttrib)
        //{ primnumAttrib = inAttrib->getOwner()==GA_ATTRIB_POINT && &inAttrib->getDetail() == geoRef1 ? inAttrib : nullptr; }
        //
        //SYS_FORCE_INLINE const GA_Attribute* setPrimnumAttrib(const UT_StringRef& name)
        //{ return primnumAttrib = geoRef1->findPointAttribute(name); }
        //
        //
        //SYS_FORCE_INLINE void setPrimuvwAttrib(const GA_Attribute* const inAttrib)
        //{ primuvwAttrib = inAttrib->getOwner()==GA_ATTRIB_POINT && &inAttrib->getDetail() == geoRef1 ? inAttrib : nullptr; }
        //
        //SYS_FORCE_INLINE const GA_Attribute* setPrimuvwAttrib(const UT_StringRef& name)
        //{ return primuvwAttrib = geoRef1->findPointAttribute(name); }
        //
        //
        //SYS_FORCE_INLINE void setPtnumAttrib(const UT_StringRef& name)
        //{ ptnumAttribName = &name; }


private:


    
#define __TEMP_GFE_Intersection_InGroupName  "__TEMP_GFE_Intersection_InGroup"
#define __TEMP_GFE_Intersection_RefGroupName "__TEMP_GFE_Intersection_RefGroup"
    
#define __TEMP_GFE_Intersection_ClosedGroupName "__TEMP_GFE_Intersection_Closed"
    
#define __TEMP_GFE_Intersection_inputnumAttribName "__TEMP_GFE_Intersection_inputnumAttrib"
#define __TEMP_GFE_Intersection_primnumAttribName  "__TEMP_GFE_Intersection_primnumAttrib"
#define __TEMP_GFE_Intersection_primuvwAttribName  "__TEMP_GFE_Intersection_primuvwAttrib"
#define __TEMP_GFE_Intersection_ptnumAttribName    "__TEMP_GFE_Intersection_ptnumAttrib"
    
    virtual bool
        computeCore() override
    {
        UT_ASSERT_MSG_P(interStitchVerb, "Does not have interStitch Verb");
        
        UT_ASSERT_MSG_P(cookparms, "Does not have cookparms");
        if (!cookparms)
            return false;
        
        if (geoRef1 && !(inputnumAttrib && primnumAttrib && primuvwAttrib))
            return false;
            
        if (!geoSrc && !geoRef0)
        {
            geo->clear();
            cookparms->sopAddError(0, "cant not without geo0 and geo1");
            return false;
        }
            
        if (groupParser.isEmpty())
            return true;
        

        GU_DetailHandle geoTmp_h;
        GU_DetailHandle geoRefTmp_h;
        GU_DetailHandle geoPointTmp_h;

        const bool keepPointAttrib = inputnumAttribName || primnumAttribName || primuvwAttribName;
        interStitchParms.setKeeppointattribs(keepPointAttrib);
        
        const bool repairFinal = repairResult && (outType != OutType::Point || geoRef1);
        const bool outDualGeo = outType == OutType::DualGeo && geoSrc && geoRef0 && !(keepPointAttrib || ptnumAttribName) && !repairFinal;
        const bool calAnalysis = !geoRef1 && !outDualGeo;
        const bool triangulateMeshFinal = calAnalysis && outType == OutType::Point && triangulateMesh;
        
        geoTmp = new GU_Detail();
        geoTmp_h.allocateAndSet(geoTmp);
        if (geoSrc)
            geoTmp->replaceWith(*geoSrc);
            
        geoRefTmp = new GU_Detail();
        geoRefTmp_h.allocateAndSet(geoRefTmp);
        if (geoRef0)
            geoRefTmp->replaceWith(*geoRef0);
        
        geoPointTmp = new GU_Detail();
        geoPointTmp_h.allocateAndSet(geoPointTmp);
        if (geoRef1)
            geoPointTmp->replaceWith(*geoRef1);
        
        GFE_Enumerate enumerate0(geoTmp, cookparms);
        GFE_Enumerate enumerate1(geoRefTmp, cookparms);
        
        if (triangulateMeshFinal)
        {
            enumAttrib0 = enumerate0.findOrCreateTuple(true, GA_ATTRIB_PRIMITIVE);
            enumAttrib1 = enumerate1.findOrCreateTuple(true, GA_ATTRIB_PRIMITIVE);
            enumerate0.compute();
            enumerate1.compute();
                
            geoTmp->convex();
            geoRefTmp->convex();
        }
        
        if (groupParser.getHasGroup())
        {
            if (groupParser.isDetached() || !groupParser.isPrimitiveGroup())
            {
                GA_PrimitiveGroup* const inGroup = geoTmp->newPrimitiveGroup(__TEMP_GFE_Intersection_InGroupName);
                GFE_GroupUnion::groupUnion(inGroup, groupParser.getPrimitiveGroup());
                //inGroup->combine(groupParser.getPrimitiveGroup());
                interAnalysisParms.setAgroup(__TEMP_GFE_Intersection_InGroupName);
                interStitchParms.setAgroup(__TEMP_GFE_Intersection_InGroupName);
            }
            else
            {
                interAnalysisParms.setAgroup(groupParser.getName());
                interStitchParms.setAgroup(groupParser.getName());
            }
        }
        else
        {
            interAnalysisParms.setAgroup("");
            interStitchParms.setAgroup("");
        }
        
        if (groupParserRef0.getHasGroup())
        {
            if (groupParserRef0.isDetached() || !groupParserRef0.isPrimitiveGroup())
            {
                GA_PrimitiveGroup* const refGroup = geoRefTmp->newPrimitiveGroup(__TEMP_GFE_Intersection_RefGroupName);
                GFE_GroupUnion::groupUnion(refGroup, groupParserRef0.getPrimitiveGroup());
                interAnalysisParms.setBgroup(__TEMP_GFE_Intersection_RefGroupName);
                interStitchParms.setBgroup(__TEMP_GFE_Intersection_RefGroupName);
            }
            else
            {
                interAnalysisParms.setBgroup(groupParserRef0.getName());
                interStitchParms.setBgroup(groupParserRef0.getName());
            }
        }
        else
        {
            interAnalysisParms.setBgroup("");
            interStitchParms.setBgroup("");
        }
        
        if (calAnalysis)
        {
            inputgdh.clear();
            inputgdh.emplace_back(geoTmp_h);
            inputgdh.emplace_back(geoRefTmp_h);
            intersectionAnalysis();
        }




        
        if (geoRef1 || outType != OutType::Point)
        {
            GU_DetailHandle geoEmpty_h;
            GU_Detail* const geoEmpty = new GU_Detail;
            geoEmpty_h.allocateAndSet(geoEmpty);
            
            inputgdh.clear();
            //inputgdh.emplace_back(outType != OutType::SecondGeo && geoSrc  ? geoTmp_h    : GU_DetailHandle());
            //inputgdh.emplace_back(outType != OutType::FirstGeo  && geoRef0 ? geoRefTmp_h : GU_DetailHandle());
            if (outType != OutType::SecondGeo && geoSrc)
            {
                if (repairFinal)
                    groupClosedPrim(geoTmp);
                inputgdh.emplace_back(geoTmp_h);
            }
            else
            {
                interStitchParms.setAgroup("");
                inputgdh.emplace_back(geoEmpty_h);
            }
            
            if (outType != OutType::FirstGeo && geoRef0)
            {
                if (repairFinal)
                    groupClosedPrim(geoRefTmp);
                inputgdh.emplace_back(geoRefTmp_h);
            }
            else
            {
                interStitchParms.setBgroup("");
                inputgdh.emplace_back(geoEmpty_h);
            }
            if (!geoRef1 && !calAnalysis)
                inputgdh.emplace_back(GU_DetailHandle());
            else
                inputgdh.emplace_back(geoPointTmp_h);

            
            intersectionStitch(calAnalysis);
            
            if (repairFinal)
                repair();
        }

        
        return true;
    }

    void intersectionAnalysis()
    {
        interAnalysisParms.setUseproxtol(useTol);
        interAnalysisParms.setProxtol(tolerance);
        interAnalysisParms.setDetectverts(detectVertexIntersection);
        interAnalysisParms.setOutputsegs(outIntersectionSegment);
        
        const bool outPoint = outType == OutType::Point;
        
        if (outPoint)
        {
            interAnalysisParms.setUseinputnumattrib(inputnumAttribName);
            interAnalysisParms.setUseprimnumattrib(primnumAttribName);
            interAnalysisParms.setUseprimuvwattrib(primuvwAttribName);
            interAnalysisParms.setUseptnumattrib(ptnumAttribName);
            if (inputnumAttribName)
                interAnalysisParms.setInputnumattrib(inputnumAttribName);
            if (primnumAttribName)
                interAnalysisParms.setPrimnumattrib(primnumAttribName);
            if (primuvwAttribName)
                interAnalysisParms.setPrimuvwattrib(primuvwAttribName);
            if (ptnumAttribName)
                interAnalysisParms.setPtnumattrib(ptnumAttribName);
        }
        else
        {
            interAnalysisParms.setUseinputnumattrib(true);
            interAnalysisParms.setUseprimnumattrib(true);
            interAnalysisParms.setUseprimuvwattrib(true);
            interAnalysisParms.setUseptnumattrib(ptnumAttribName);
            
            interAnalysisParms.setInputnumattrib(__TEMP_GFE_Intersection_inputnumAttribName);
            interAnalysisParms.setPrimnumattrib( __TEMP_GFE_Intersection_primnumAttribName);
            interAnalysisParms.setPrimuvwattrib( __TEMP_GFE_Intersection_primuvwAttribName);
            if (ptnumAttribName)
                interAnalysisParms.setPtnumattrib(ptnumAttribName);
        }

        if (outPoint)
            geo->clear();
        
        GU_DetailHandle destgdh;
        destgdh.allocateAndSet(outPoint ? geo->asGU_Detail() : geoPointTmp, false);
        
        SOP_NodeCache* const nodeCache = interAnalysisVerb->allocCache();
        const auto interCookparms = GFE_NodeVerb::newCookParms(cookparms, interAnalysisParms, nodeCache, &destgdh, &inputgdh);
        interAnalysisVerb->cook(interCookparms);
        
        if (!outPoint)
        {
            inputnumAttrib = geoPointTmp->findPointAttribute(__TEMP_GFE_Intersection_inputnumAttribName);
            primnumAttrib  = geoPointTmp->findPointAttribute(__TEMP_GFE_Intersection_primnumAttribName);
            primuvwAttrib  = geoPointTmp->findPointAttribute(__TEMP_GFE_Intersection_primuvwAttribName);
        }
    }



    
    void intersectionStitch(const bool calAnalysis)
    {
        interStitchParms.setUseproxtol(useTol);
        interStitchParms.setProxtol(tolerance);
        interStitchParms.setSplitcurves(splitCurve);
        interStitchParms.setKeeppointattribs(inputnumAttribName || primnumAttribName || primuvwAttribName);
        
        //const UT_StringHolder& inputnumAttribNameTemp = UT_StringHolder(__TEMP_GFE_Intersection_inputnumAttribName);
        //const UT_StringHolder& primnumAttribNameTemp  = UT_StringHolder(__TEMP_GFE_Intersection_primnumAttribName) ;
        //const UT_StringHolder& primuvwAttribNameTemp  = UT_StringHolder(__TEMP_GFE_Intersection_primuvwAttribName) ;
        
        const char* inputnumAttribNameMid;
        const char* primnumAttribNameMid ;
        const char* primuvwAttribNameMid ;
        
        UT_ASSERT_P(inputnumAttrib);
        UT_ASSERT_P(primnumAttrib);
        UT_ASSERT_P(primuvwAttrib);
        const bool detachedInputnum = inputnumAttrib->isDetached();
        const bool detachedPrimnum  = primnumAttrib->isDetached();
        const bool detachedPrimuvw  = primuvwAttrib->isDetached();
        if (geoRef1)
        {
            if (detachedInputnum)
                GFE_Attribute::clone(*geoRefTmp, *inputnumAttrib, __TEMP_GFE_Intersection_inputnumAttribName);
            if (detachedPrimnum)
                GFE_Attribute::clone(*geoRefTmp, *primnumAttrib,  __TEMP_GFE_Intersection_primnumAttribName);
            if (detachedPrimuvw)
                GFE_Attribute::clone(*geoRefTmp, *primuvwAttrib,  __TEMP_GFE_Intersection_primuvwAttribName);
        }
        inputnumAttribNameMid = !geoRef1 || detachedInputnum ? __TEMP_GFE_Intersection_inputnumAttribName : inputnumAttrib->getName().c_str();
        primnumAttribNameMid  = !geoRef1 || detachedPrimnum  ? __TEMP_GFE_Intersection_primnumAttribName  : primnumAttrib->getName() .c_str();
        primuvwAttribNameMid  = !geoRef1 || detachedPrimuvw  ? __TEMP_GFE_Intersection_primuvwAttribName  : primuvwAttrib->getName() .c_str();
        //
        // if (!calAnalysis)
        // {
        //     inputnumAttribNameMid = inputnumAttribName;
        //     primnumAttribNameMid  = primnumAttribName ;
        //     primuvwAttribNameMid  = primuvwAttribName ;
        // }
        // else
        // {
        //     
        // }
        interStitchParms.setInputnumattrib(inputnumAttribNameMid);
        interStitchParms.setPrimnumattrib (primnumAttribNameMid);
        interStitchParms.setPrimuvwattrib (primuvwAttribNameMid);
            
        geo->clear();
        GU_DetailHandle destgdh;
        destgdh.allocateAndSet(geo->asGU_Detail(), false);
        SOP_NodeCache* const nodeCache = interStitchVerb->allocCache();
        const auto interStitchCookparms = GFE_NodeVerb::newCookParms(cookparms, interStitchParms, nodeCache, &destgdh, &inputgdh);
        interStitchVerb->cook(interStitchCookparms);

        GA_GroupTable& primGroupTable = *geo->getGroupTable(GA_GROUP_PRIMITIVE);
        primGroupTable.destroy(__TEMP_GFE_Intersection_InGroupName);
        primGroupTable.destroy(__TEMP_GFE_Intersection_RefGroupName);
        
        if (inputnumAttribName)
            geo->forceRenameAttribute(GA_ATTRIB_POINT, inputnumAttribNameMid, inputnumAttribName);
        else
            geo->destroyAttribute(GA_ATTRIB_POINT, inputnumAttribNameMid);
        
        if (primnumAttribName)
            geo->forceRenameAttribute(GA_ATTRIB_POINT, primnumAttribNameMid,  primnumAttribName);
        else
            geo->destroyAttribute(GA_ATTRIB_POINT, primnumAttribNameMid);
        
        if (primuvwAttribName)
            geo->forceRenameAttribute(GA_ATTRIB_POINT, primuvwAttribNameMid,  primuvwAttribName);
        else
            geo->destroyAttribute(GA_ATTRIB_POINT, primuvwAttribNameMid);
    }




    void repair()
    {
        GA_PrimitiveGroup* const delGroup = geo->findPrimitiveGroup(__TEMP_GFE_Intersection_ClosedGroupName);
        if (!delGroup)
            return;
        const GA_Index ptnumNewLast = geoPointTmp->getNumPoints();
        const GA_Size  numprimInput = (geoSrc && outType != OutType::SecondGeo ? geoSrc->getNumPrimitives() : 0) + (geoRef0 && outType != OutType::FirstGeo ? geoRef0->getNumPrimitives() : 0);

        UTparallelFor(geo->getPrimitiveSplittableRange(delGroup), [this, delGroup, ptnumNewLast](const GA_SplittableRange& r)
        {
            GA_Offset start, end;
            for (GA_Iterator it(r); it.blockAdvance(start, end); )
            {
                for (GA_Offset primoff = start; primoff < end; ++primoff)
                {
                    const GA_Size numvtx = geo->getPrimitiveVertexCount(primoff);
                    if (numvtx <= 0)
                        continue;
                    
                    const GA_Offset primPtoff_last = geo->primPoint(primoff, numvtx-1);
                    const GA_Index  primPtnum_last = geo->pointIndex(primPtoff_last);
                    
                    //const GA_Offset primPtoff_first = geo->primPoint(primoff, 0);
                    //const GA_Index  primPtnum_first = geo->pointIndex(primPtoff_first);
                    
                    if (primPtnum_last >= ptnumNewLast)
                        delGroup->setElement(primoff, false);
                }
            }
        }, subscribeRatio, minGrainSize);
        delGroup->invalidateGroupEntries();
        
        GA_Offset start, end;
        for (GA_Iterator it(geo->getPrimitiveRange(delGroup)); it.blockAdvance(start, end); )
        {
            for (GA_Offset primoff = start; primoff < end; ++primoff)
            {
                const GA_Size numvtx = geo->getPrimitiveVertexCount(primoff);
                
                const GA_Offset primvtxoff0 = geo->getPrimitiveVertexOffset(primoff, 0);
                const GA_Offset primptoff0  = geo->vertexPoint(primvtxoff0);
                
                GA_Offset nextPrimoff;
                for (GA_Offset pointVtxoff = geo->pointVertex(primptoff0); GFE_Type::isValidOffset(pointVtxoff); pointVtxoff = geo->vertexToNextVertex(pointVtxoff))
                {
                    nextPrimoff = geo->vertexPrimitive(pointVtxoff);
                    if (geo->primitiveIndex(nextPrimoff) >= numprimInput)
                        break;
                }
                GEO_PrimPoly* const primPoly = geo->getGEOPrimPoly(nextPrimoff);
                // const GA_Size numvtx_nextPrim = geo->getPrimitiveVertexCount(nextPrimoff);
                for (GA_Size vtxpnum = 1; vtxpnum < numvtx; ++vtxpnum)
                {
                    primPoly->stealVertex(geo->primVertex(primoff, vtxpnum));
                    
                    // const GA_Offset nextPrimptoff = geo->primPoint(primoff, vtxpnum);
                    //const GA_Offset newvtx = geo->appendVertex();
                    //geo->getTopology().wireVertexPrimitive(newvtx, nextPrimoff);
                    //geo->getTopology().wireVertexPoint(newvtx, nextPrimptoff);
                    //geo->getTopology().addVertex(newvtx);
                    //const GEO_PrimPoly* const gPrimPoly = geo->getGEOPrimPoly(nextPrimoff);
                    //geo->getGEOPrimPoly(nextPrimoff)->appendVertex(nextPrimptoff);
                    //static_cast<GEO_PrimPoly*>(geo->asGEO_Detail()->getGEOPrimitive(nextPrimoff))->appendVertex(nextPrimptoff);
                }
                geo->destroyPrimitiveOffset(primoff);
            }
        }
        geo->destroyGroup(delGroup);
    }
    
    

    void groupClosedPrim(GA_Detail* const geo)
    {
        GA_PrimitiveGroup* const closedGroup = geo->newPrimitiveGroup(__TEMP_GFE_Intersection_ClosedGroupName);
        UTparallelFor(GA_SplittableRange(geo->getPrimitiveRange()), [geo, closedGroup](const GA_SplittableRange& r)
        {
            GA_Offset start, end;
            for (GA_Iterator it(r); it.blockAdvance(start, end); )
            {
                for (GA_Offset primoff = start; primoff < end; ++primoff)
                {
                    const GA_Size numvtx = geo->getPrimitiveVertexCount(primoff);
                    if (numvtx <= 2)
                        continue;
                                
                    if (geo->getPrimitiveClosedFlag(primoff))
                    {
                        closedGroup->setElement(primoff, true);
                        continue;
                    }
                            
                    const GA_Offset primPtoff0 = getPrimitivePointOffset(geo, primoff, 0);
                    const GA_Offset primPtoff1 = getPrimitivePointOffset(geo, primoff, numvtx-1);
                            
                    if (primPtoff0 == primPtoff1)
                        closedGroup->setElement(primoff, true);
                }
            }
        }, subscribeRatio, minGrainSize);
        closedGroup->invalidateGroupEntries();
    }
    
    SYS_FORCE_INLINE static GA_Offset getPrimitivePointOffset(const GA_Detail* const geo, const GA_Offset primoff, const GA_Size vtxpnum)
    { return geo->vertexPoint(geo->getPrimitiveVertexOffset(primoff, vtxpnum)); }








    
public:
    OutType outType = OutType::Point;

public:
    bool triangulateMesh = false;
    bool detectVertexIntersection = false;
    bool outIntersectionSegment = false;
    
public:
    bool splitCurve = false;
    bool repairResult = false;
    
public:
    // bool outInputnumAttrib = true;
    // bool outPrimnumAttrib = true;
    // bool outPrimuvwAttrib = true;
    // bool outPtnumAttrib = true;
    
private:
    bool useTol = true;
    fpreal64 tolerance = 1e-05;
    
private:
    GU_Detail* geoTmp      = nullptr;
    GU_Detail* geoRefTmp   = nullptr;
    GU_Detail* geoPointTmp = nullptr;

    // const GA_Detail* geoSrcConst   = nullptr;
    // const GA_Detail* geoRefConst   = nullptr;
    // const GA_Detail* geoPointConst = nullptr;
private:
    
    const GA_Attribute* enumAttrib0;
    const GA_Attribute* enumAttrib1;
    
    //const GA_Attribute* inputnumAttrib = nullptr;
    //const GA_Attribute* primnumAttrib  = nullptr;
    //const GA_Attribute* primuvwAttrib  = nullptr;
    //const GA_Attribute* ptnumAttrib    = nullptr;
    //
    //const char* inputnumAttribName = nullptr;
    //const char* primnumAttribName  = nullptr;
    //const char* primuvwAttribName  = nullptr;
    //const char* ptnumAttribName    = nullptr;
    
    UT_Array<GU_ConstDetailHandle> inputgdh;
    SOP_IntersectionStitchParms interStitchParms;
    const SOP_NodeVerb* const interStitchVerb   = SOP_NodeVerb::lookupVerb("intersectionstitch");

    
    SOP_IntersectionAnalysisParms interAnalysisParms;
    const SOP_NodeVerb* const interAnalysisVerb = SOP_NodeVerb::lookupVerb("intersectionanalysis");
        
    
    exint subscribeRatio = 64;
    exint minGrainSize = 1024;

#undef __TEMP_GFE_Intersection_InGroupName
#undef __TEMP_GFE_Intersection_RefGroupName
#undef __TEMP_GFE_Intersection_ClosedGroupName
    
#undef __TEMP_GFE_Intersection_inputnumAttribName
#undef __TEMP_GFE_Intersection_primnumAttribName
#undef __TEMP_GFE_Intersection_primuvwAttribName
#undef __TEMP_GFE_Intersection_ptnumAttribName
    
    
}; // End of class GFE_Intersection





#endif
