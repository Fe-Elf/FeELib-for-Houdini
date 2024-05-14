
#pragma once

#ifndef __GFE_GeoFilter_h__
#define __GFE_GeoFilter_h__

#include <GFE/GeoFilter.h>

#include <PRM/PRM_TemplateBuilder.h>
#include <UT/UT_Interrupt.h>
#include <UT/UT_DSOVersion.h>

#include <GFE/Geometry.h>
#include <GFE/GroupParser.h>
#include <GFE/AttributeArray.h>
#include <GFE/VolumeArray.h>
#include <GFE/NodeVerb.h>


_GFE_BEGIN
class GeoFilter {

public:
    
    GeoFilter(
        gfe::Geometry* const geo,
        const SOP_NodeVerb::CookParms* const cookparms = nullptr
    )
        : geo(geo)
        //, geoConst(geo)
        , cookparms(cookparms)
        , groupParser(geo, gop, cookparms)
        , volumeArray(geo, cookparms)
    {
    }

    GeoFilter(
        GA_Detail& geo,
        const SOP_NodeVerb::CookParms* const cookparms = nullptr
    )
        : geo(reinterpret_cast<gfe::Geometry*>(&geo))
        //, geoConst(reinterpret_cast<gfe::Geometry*>(&geo))
        , cookparms(cookparms)
        , groupParser(geo, gop, cookparms)
        , volumeArray(geo, cookparms)
    {
    }
    
    GeoFilter(
        GA_Detail* const geo,
        const SOP_NodeVerb::CookParms* const cookparms = nullptr
    )
        : geo(reinterpret_cast<gfe::Geometry*>(geo))
        //, geoConst(reinterpret_cast<gfe::Geometry*>(&geo))
        , cookparms(cookparms)
        , groupParser(*geo, gop, cookparms)
        , volumeArray(geo, cookparms)
    {
    }
    
    GeoFilter(
        GA_Detail& geo,
        const SOP_NodeVerb::CookParms& cookparms
    )
        : geo(reinterpret_cast<gfe::Geometry*>(&geo))
        //, geoConst(reinterpret_cast<gfe::Geometry*>(&geo))
        , cookparms(&cookparms)
        , groupParser(geo, gop, cookparms)
        , volumeArray(geo, cookparms)
    {
    }


    
    GeoFilter(
        gfe::Geometry* const geo,
        const GA_Detail* const geoSrc,
        const SOP_NodeVerb::CookParms* const cookparms = nullptr
    )
        : geo(geo)
        , geoSrc(geoSrc)
        , cookparms(cookparms)
        , groupParser(geoSrc ? geoSrc : geo, gop, cookparms)
        , volumeArray(geo, cookparms)
    {
    }

    GeoFilter(
        GA_Detail& geo,
        const GA_Detail& geoSrc,
        const SOP_NodeVerb::CookParms& cookparms
    )
        : geo(reinterpret_cast<gfe::Geometry*>(&geo))
        , geoSrc(&geoSrc)
        , cookparms(&cookparms)
        , groupParser(geoSrc, gop, cookparms)
        , volumeArray(geo, cookparms)
    {
    }

    GeoFilter(
        GA_Detail& geo,
        const GA_Detail& geoSrc,
        const SOP_NodeVerb::CookParms* const cookparms = nullptr
    )
        : geo(reinterpret_cast<gfe::Geometry*>(&geo))
        , geoSrc(&geoSrc)
        , cookparms(cookparms)
        , groupParser(geoSrc, gop, cookparms)
        , volumeArray(geo, cookparms)
    {
    }

    GeoFilter(
        GA_Detail& geo,
        const GA_Detail* const geoSrc,
        const SOP_NodeVerb::CookParms* const cookparms = nullptr
    )
        : geo(reinterpret_cast<gfe::Geometry*>(&geo))
        , geoSrc(geoSrc)
        , cookparms(cookparms)
        , groupParser(geoSrc ? *geoSrc : geo, gop, cookparms)
        , volumeArray(geo, cookparms)
    {
    }

    ~GeoFilter()
    {
        //delTopoAttrib();
    }




    SYS_FORCE_INLINE void setGeoSrc(const GA_Detail* const inGeo = nullptr)
    { geoSrc = inGeo; }

    SYS_FORCE_INLINE void setGeoSrc(const GA_Detail& inGeo)
    { geoSrc = &inGeo; }


    virtual void reset(GA_Detail& inGeo, const SOP_NodeVerb::CookParms* const cookparms = nullptr)
    {
        geo = reinterpret_cast<gfe::Geometry*>(&inGeo);
        this->cookparms = cookparms;
        groupParser.reset(geo, cookparms);
        setHasComputed();
    }

    SYS_FORCE_INLINE virtual void reset(GA_Detail* const inGeo, const SOP_NodeVerb::CookParms* const cookparms = nullptr)
    { reset(*inGeo, cookparms); }


    SYS_FORCE_INLINE bool getHasComputed() const
    { return hasComputed; }
    
    // SYS_FORCE_INLINE void setOutTopoAttrib(const bool outTopoAttrib = true)
    // { this->outTopoAttrib = outTopoAttrib; }
    //
    // SYS_FORCE_INLINE bool getOutTopoAttrib() const
    // { return outTopoAttrib; }
    

    GroupParser& getGroupParser()
    { return groupParser; }
    
    SYS_FORCE_INLINE void compute()
    { hasComputed = computeCore(); }


    
    
    SYS_FORCE_INLINE void setPieceAttrib(const GA_Attribute* const inAttrib)
    { pieceAttrib = inAttrib; }

    SYS_FORCE_INLINE void setPieceAttrib(const GA_AttributeOwner owner, const UT_StringRef& attribName)
    { pieceAttrib = geo->findAttribute(owner, attribName); }


    
    SYS_FORCE_INLINE void setPositionAttrib(const GA_Attribute* const inPosAttrib)
    { posAttrib = inPosAttrib; posAttribNonConst = nullptr; }

    SYS_FORCE_INLINE void setPositionAttrib(GA_Attribute* const inPosAttrib)
    { posAttrib = inPosAttrib; posAttribNonConst = inPosAttrib; }

    SYS_FORCE_INLINE void setPositionAttrib(const UT_StringRef& attribName)
    { posAttribNonConst = geo->findPointAttribute(attribName); posAttrib = posAttribNonConst; }

    
    
    SYS_FORCE_INLINE void bumpDataIdsForAddOrRemove(const bool point, const bool vertex, const bool prim) const
    { geo->bumpDataIdsForAddOrRemove(point, vertex, prim); }

    virtual void bumpDataIdsForAddOrRemove() const
    { geo->bumpDataIdsForAddOrRemove(true, true, true); }

    SYS_FORCE_INLINE void computeAndBumpDataIdsForAddOrRemove(const bool point, const bool vertex, const bool prim)
    { compute(); bumpDataIdsForAddOrRemove(point, vertex, prim); }

    SYS_FORCE_INLINE void computeAndBumpDataIdsForAddOrRemove()
    { compute(); bumpDataIdsForAddOrRemove(); }



    virtual void bumpDataId() const
    {
    }

    SYS_FORCE_INLINE void computeAndBumpDataId()
    { compute(); bumpDataId(); }

    SYS_FORCE_INLINE gfe::Geometry* getDetail() const
    { return geo; }
    
    SYS_FORCE_INLINE void visualizeGroup(const GA_Group& group) const
    { if (cookparms && !group.isDetached()) cookparms->select(group); }
    
    SYS_FORCE_INLINE void visualizeGroup(const GA_Group* const group) const
    { if (group) visualizeGroup(*group); }

    SYS_FORCE_INLINE void visualizeGroup(const GA_GroupType groupType, const UT_StringRef& name) const
    { visualizeGroup(geo->findGroup(groupType, name)); }

    SYS_FORCE_INLINE void setValidPosAttrib()
    {
        if (gfe::isInvalidPosAttrib(posAttrib) || gfe::isInvalidPosAttrib(posAttribNonConst))
        {
            posAttribNonConst = geo->getP();
            posAttrib = posAttribNonConst;
        }
        UT_ASSERT_P(posAttrib);
        UT_ASSERT_P(posAttribNonConst);
    }
    
    SYS_FORCE_INLINE void setValidConstPosAttrib()
    { if (gfe::isInvalidPosAttrib(posAttrib)) posAttrib = geo->getP(); UT_ASSERT_P(posAttrib); }


    
    SYS_FORCE_INLINE gfe::VolumeArray& getVolumeArray()
    { return volumeArray; }

    SYS_FORCE_INLINE const gfe::VolumeArray& getVolumeArray() const
    { return volumeArray; }

private:

    virtual bool computeCore()
    { return false; }


private:
    SYS_FORCE_INLINE virtual void setDetailBase(GA_Detail& inGeo)
    {
        geo = reinterpret_cast<gfe::Geometry*>(&inGeo);
        groupParser.setDetail(geo);
    }

    SYS_FORCE_INLINE virtual void delTopoAttrib() const
    { if (outTopoAttrib) geo->delTopoAttrib(); }

    
protected:

    SYS_FORCE_INLINE void setDetail(GA_Detail* const inGeo)
    { setDetailBase(*inGeo); }
    
    SYS_FORCE_INLINE void setDetail(GA_Detail& inGeo)
    { setDetail(&inGeo); }
    
    SYS_FORCE_INLINE void setDetail(GA_Attribute& attrib)
    { setDetail(attrib.getDetail()); }
    
    SYS_FORCE_INLINE void setDetail(GA_ElementGroup& group)
    { setDetail(static_cast<GA_ElementGroup&>(group).getDetail()); }

    SYS_FORCE_INLINE void setDetail(GA_Group& group)
    {
        UT_ASSERT_MSG(group.classType() != GA_GROUP_EDGE, "can not be edge group");
#if 1
        setDetail(static_cast<GA_ElementGroup&>(group));
#else
        if (group.classType() == GA_GROUP_EDGE)
            setDetail(static_cast<GA_EdgeGroup&>(group).getDetail());
        else
            setDetail(static_cast<GA_ElementGroup&>(group));
#endif
    }
    
    SYS_FORCE_INLINE void setHasComputed(const bool inHasComputed = false)
    { hasComputed = inHasComputed; }

    
public:
    GroupParser groupParser;
    bool outTopoAttrib = true;

protected:
    gfe::Geometry* geo;
    const GA_Detail* geoSrc = nullptr;
    //const gfe::Geometry* geoConst;
    const SOP_NodeVerb::CookParms* cookparms;
    
    const GA_Attribute* pieceAttrib = nullptr;
    const GA_Attribute* posAttrib = nullptr;
    GA_Attribute* posAttribNonConst = nullptr;
    
    GOP_Manager gop;
    gfe::VolumeArray volumeArray;

private:
    //bool outTopoAttrib = true;
    bool hasComputed = false;
    

}; // End of class GeoFilter


#if 0
const GA_OffsetListRef& vertices = geo->getPrimitiveVertexList(elemoff);
const GA_Size numvtx = vertices.size();
const GA_Size lastVtxpnum = numvtx-1;
for (GA_Size vtxpnum = 0; vtxpnum < numvtx; ++vtxpnum)
{
    const GA_Offset ptoff = geo->vertexPoint(vertices[vtxpnum]);
}
#endif


#if 0

class GeoFilterRef0 {

public:

    GeoFilterRef0(
        const gfe::Geometry* const geoRef0,
        GOP_Manager& gop,
        const SOP_NodeVerb::CookParms* const cookparms = nullptr
    )
        : geoRef0(geoRef0)
        , cookparmsRef0(cookparms)
        , gopRef0(&gop)
        , groupParserRef0(geoRef0, gop, cookparms)
        , ref0AttribArray(geoRef0, cookparms)
        , ref0GroupArray(geoRef0, cookparms)
    {
        //UT_ASSERT_MSG(geoRef0, "do not find geo");
    }

    GeoFilterRef0(
        const GA_Detail* const geoRef0,
        GOP_Manager& gop,
        const SOP_NodeVerb::CookParms* const cookparms = nullptr
    )
        : geoRef0(reinterpret_cast<const gfe::Geometry*>(geoRef0))
        , cookparmsRef0(cookparms)
        , gopRef0(&gop)
        , groupParserRef0(geoRef0, gop, cookparms)
        , ref0AttribArray(geoRef0, cookparms)
        , ref0GroupArray(geoRef0, cookparms)
    {
        //UT_ASSERT_MSG(geoRef0, "do not find geo");
    }

    GeoFilterRef0(
        const GA_Detail& geoRef0,
        GOP_Manager& gop,
        const SOP_NodeVerb::CookParms* const cookparms = nullptr
    )
        : geoRef0(reinterpret_cast<const gfe::Geometry*>(&geoRef0))
        , cookparmsRef0(cookparms)
        , gopRef0(&gop)
        , groupParserRef0(geoRef0, gop, cookparms)
        , ref0AttribArray(geoRef0, cookparms)
        , ref0GroupArray(geoRef0, cookparms)
    {
    }

    GeoFilterRef0(
        const GA_Detail* const geoRef0,
        GOP_Manager& gop,
        const SOP_NodeVerb::CookParms& cookparms
    )
        : geoRef0(reinterpret_cast<const gfe::Geometry*>(geoRef0))
        , cookparmsRef0(&cookparms)
        , gopRef0(&gop)
        , groupParserRef0(geoRef0, gop, cookparms)
        , ref0AttribArray(geoRef0, cookparms)
        , ref0GroupArray(geoRef0, cookparms)
    {
        //UT_ASSERT_MSG(geoRef0, "do not find geo");
    }


    ~GeoFilterRef0()
    {
    }

    

    void resetGeoFilterRef0(
        const gfe::Geometry* const geoRef0,
        GOP_Manager* const gop,
        const SOP_NodeVerb::CookParms* const cookparms = nullptr
    )
    {
        this->geoRef0 = geoRef0;
        cookparmsRef0 = cookparms;
        gopRef0 = gop;
        UT_ASSERT_P(gopRef0);
        ref0AttribArray.reset(geoRef0, cookparms);
        ref0GroupArray.reset(geoRef0, cookparms);
    }
    
    SYS_FORCE_INLINE void resetGeoFilterRef0(
        const gfe::Geometry& geoRef0,
        GOP_Manager* const gop,
        const SOP_NodeVerb::CookParms* const cookparms = nullptr
    )
    { resetGeoFilterRef0(&geoRef0, gop, cookparmsRef0); }

    SYS_FORCE_INLINE void resetGeoFilterRef0(
        const GA_Detail* const geoRef0,
        GOP_Manager* const gop,
        const SOP_NodeVerb::CookParms* const cookparms = nullptr
    )
    { resetGeoFilterRef0(reinterpret_cast<const gfe::Geometry*>(geoRef0), gop, cookparmsRef0); }

    SYS_FORCE_INLINE void resetGeoFilterRef0(
        const GA_Detail& geoRef0,
        GOP_Manager* const gop,
        const SOP_NodeVerb::CookParms* const cookparms = nullptr
    )
    { resetGeoFilterRef0(reinterpret_cast<const gfe::Geometry&>(geoRef0), gop, cookparmsRef0); }


    
    // SYS_FORCE_INLINE GroupParser& getRef0GroupParser()
    // { return groupParserRef0; }
    SYS_FORCE_INLINE gfe::RefAttributeArray& getRef0AttribArray()
    { return ref0AttribArray; }
    SYS_FORCE_INLINE gfe::RefGroupArray& getRef0GroupArray()
    { return ref0GroupArray; }
    SYS_FORCE_INLINE std::vector<const GA_Attribute*>& getRef0AttribArrayRef()
    { return ref0AttribArray.ref(); }
    SYS_FORCE_INLINE std::vector<const GA_Group*>& getRef0GroupArrayRef()
    { return ref0GroupArray.ref(); }


    SYS_FORCE_INLINE void setPositionRef0Attrib(const GA_Attribute* const inPosAttrib)
    { posRef0Attrib = inPosAttrib; }

    SYS_FORCE_INLINE void setPositionRef0Attrib(const UT_StringRef& attribName)
    { posRef0Attrib = geoRef0->findPointAttribute(attribName); }

    
private:
    SYS_FORCE_INLINE virtual void setRef0DetailBase(const gfe::Geometry* const inGeo)
    {
        geoRef0 = inGeo;
        ref0AttribArray.setDetail(inGeo);
        ref0GroupArray.setDetail(inGeo);
    }

    SYS_FORCE_INLINE virtual void setRef0DetailBase(const GA_Detail* const inGeo)
    { setRef0DetailBase(reinterpret_cast<const gfe::Geometry*>(inGeo)); }



public:
    GroupParser groupParserRef0;

protected:
    const gfe::Geometry* geoRef0;

private:
    const SOP_NodeVerb::CookParms* cookparmsRef0;
    GOP_Manager* gopRef0;
    
    const GA_Attribute* posRef0Attrib = nullptr;

    gfe::RefAttributeArray ref0AttribArray;
    gfe::RefGroupArray ref0GroupArray;

}; // End of class GeoFilterRef0


#else



#define __GeoFilterRef_SPECILIZATION(NUM)                                                                                           \
class GeoFilterRef##NUM {                                                                                                           \
                                                                                                                                    \
public:                                                                                                                             \
                                                                                                                                    \
    GeoFilterRef##NUM(                                                                                                              \
        const gfe::Geometry* const geoRef##NUM,                                                                                        \
        GOP_Manager& gop,                                                                                                           \
        const SOP_NodeVerb::CookParms* const cookparms = nullptr                                                                    \
    )                                                                                                                               \
        : geoRef##NUM(geoRef##NUM)                                                                                                  \
        , cookparmsRef##NUM(cookparms)                                                                                              \
        , gopRef##NUM(&gop)                                                                                                         \
        , groupParserRef##NUM(geoRef##NUM, gop, cookparms)                                                                          \
        , ref##NUM##AttribArray(geoRef##NUM, cookparms)                                                                             \
        , ref##NUM##GroupArray(geoRef##NUM, cookparms)                                                                              \
    {                                                                                                                               \
    }                                                                                                                               \
                                                                                                                                    \
    GeoFilterRef##NUM(                                                                                                              \
        const GA_Detail* const geoRef##NUM,                                                                                         \
        GOP_Manager& gop,                                                                                                           \
        const SOP_NodeVerb::CookParms* const cookparms = nullptr                                                                    \
    )                                                                                                                               \
        : geoRef##NUM(reinterpret_cast<const gfe::Geometry*>(geoRef##NUM))                                                             \
        , cookparmsRef##NUM(cookparms)                                                                                              \
        , gopRef##NUM(&gop)                                                                                                         \
        , groupParserRef##NUM(geoRef##NUM, gop, cookparms)                                                                          \
        , ref##NUM##AttribArray(geoRef##NUM, cookparms)                                                                             \
        , ref##NUM##GroupArray(geoRef##NUM, cookparms)                                                                              \
    {                                                                                                                               \
    }                                                                                                                               \
                                                                                                                                    \
    GeoFilterRef##NUM(                                                                                                              \
        const GA_Detail& geoRef##NUM,                                                                                               \
        GOP_Manager& gop,                                                                                                           \
        const SOP_NodeVerb::CookParms* const cookparms = nullptr                                                                    \
    )                                                                                                                               \
        : geoRef##NUM(reinterpret_cast<const gfe::Geometry*>(&geoRef##NUM))                                                            \
        , cookparmsRef##NUM(cookparms)                                                                                              \
        , gopRef##NUM(&gop)                                                                                                         \
        , groupParserRef##NUM(geoRef##NUM, gop, cookparms)                                                                          \
        , ref##NUM##AttribArray(geoRef##NUM, cookparms)                                                                             \
        , ref##NUM##GroupArray(geoRef##NUM, cookparms)                                                                              \
    {                                                                                                                               \
    }                                                                                                                               \
                                                                                                                                    \
    GeoFilterRef##NUM(                                                                                                              \
        const GA_Detail* const geoRef##NUM,                                                                                         \
        GOP_Manager& gop,                                                                                                           \
        const SOP_NodeVerb::CookParms& cookparms                                                                                    \
    )                                                                                                                               \
        : geoRef##NUM(reinterpret_cast<const gfe::Geometry*>(geoRef##NUM))                                                             \
        , cookparmsRef##NUM(&cookparms)                                                                                             \
        , gopRef##NUM(&gop)                                                                                                         \
        , groupParserRef##NUM(geoRef##NUM, gop, cookparms)                                                                          \
        , ref##NUM##AttribArray(geoRef##NUM, cookparms)                                                                             \
        , ref##NUM##GroupArray(geoRef##NUM, cookparms)                                                                              \
    {                                                                                                                               \
    }                                                                                                                               \
                                                                                                                                    \
                                                                                                                                    \
    ~GeoFilterRef##NUM()                                                                                                            \
    {                                                                                                                               \
    }                                                                                                                               \
                                                                                                                                    \
                                                                                                                                    \
                                                                                                                                    \
    void resetGeoFilterRef##NUM(                                                                                                    \
        const gfe::Geometry* const geoRef##NUM,                                                                                        \
        GOP_Manager* const gop,                                                                                                     \
        const SOP_NodeVerb::CookParms* const cookparms = nullptr                                                                    \
    )                                                                                                                               \
    {                                                                                                                               \
        this->geoRef##NUM = geoRef##NUM;                                                                                            \
        cookparmsRef##NUM = cookparms;                                                                                              \
        gopRef##NUM = gop;                                                                                                          \
        UT_ASSERT_P(gopRef##NUM);                                                                                                   \
        ref##NUM##AttribArray.reset(geoRef##NUM, cookparms);                                                                        \
        ref##NUM##GroupArray.reset(geoRef##NUM, cookparms);                                                                         \
    }                                                                                                                               \
                                                                                                                                    \
    SYS_FORCE_INLINE void resetGeoFilterRef##NUM(                                                                                   \
        const gfe::Geometry& geoRef##NUM,                                                                                              \
        GOP_Manager* const gop,                                                                                                     \
        const SOP_NodeVerb::CookParms* const cookparms = nullptr                                                                    \
    )                                                                                                                               \
    { resetGeoFilterRef##NUM(&geoRef##NUM, gop, cookparmsRef##NUM); }                                                               \
                                                                                                                                    \
    SYS_FORCE_INLINE void resetGeoFilterRef##NUM(                                                                                   \
        const GA_Detail* const geoRef##NUM,                                                                                         \
        GOP_Manager* const gop,                                                                                                     \
        const SOP_NodeVerb::CookParms* const cookparms = nullptr                                                                    \
    )                                                                                                                               \
    { resetGeoFilterRef##NUM(reinterpret_cast<const gfe::Geometry*>(geoRef##NUM), gop, cookparmsRef##NUM); }                           \
                                                                                                                                    \
    SYS_FORCE_INLINE void resetGeoFilterRef##NUM(                                                                                   \
        const GA_Detail& geoRef##NUM,                                                                                               \
        GOP_Manager* const gop,                                                                                                     \
        const SOP_NodeVerb::CookParms* const cookparms = nullptr                                                                    \
    )                                                                                                                               \
    { resetGeoFilterRef##NUM(reinterpret_cast<const gfe::Geometry&>(geoRef##NUM), gop, cookparmsRef##NUM); }                           \
                                                                                                                                    \
                                                                                                                                    \
                                                                                                                                    \
    SYS_FORCE_INLINE gfe::RefAttributeArray& getRef##NUM##AttribArray()                                                              \
    { return ref##NUM##AttribArray; }                                                                                               \
    SYS_FORCE_INLINE gfe::RefGroupArray& getRef##NUM##GroupArray()                                                                   \
    { return ref##NUM##GroupArray; }                                                                                                \
    SYS_FORCE_INLINE std::vector<const GA_Attribute*>& getRef##NUM##AttribArrayRef()                                                \
    { return ref##NUM##AttribArray.ref(); }                                                                                         \
    SYS_FORCE_INLINE std::vector<const GA_Group*>& getRef##NUM##GroupArrayRef()                                                     \
    { return ref##NUM##GroupArray.ref(); }                                                                                          \
                                                                                                                                    \
                                                                                                                                    \
    SYS_FORCE_INLINE void setPositionRef##NUM##Attrib(const GA_Attribute* const inPosAttrib)                                        \
    { posRef##NUM##Attrib = inPosAttrib; }                                                                                          \
                                                                                                                                    \
    SYS_FORCE_INLINE void setPositionRef##NUM##Attrib(const UT_StringRef& attribName)                                               \
    { posRef##NUM##Attrib = geoRef##NUM ? geoRef##NUM->findPointAttribute(attribName) : nullptr; }                                  \
                                                                                                                                    \
                                                                                                                                    \
    SYS_FORCE_INLINE void setValidPosRef##NUM##Attrib()                                                                             \
    { posRef##NUM##Attrib = geoRef##NUM && gfe::isInvalidPosAttrib(posRef##NUM##Attrib) ? geoRef##NUM->getP() : nullptr; }          \
                                                                                                                                    \
                                                                                                                                    \
                                                                                                                                    \
private:                                                                                                                            \
    SYS_FORCE_INLINE virtual void setRef##NUM##DetailBase(const gfe::Geometry* const inGeo)                                            \
    {                                                                                                                               \
        geoRef##NUM = inGeo;                                                                                                        \
        ref##NUM##AttribArray.setDetail(inGeo);                                                                                     \
        ref##NUM##GroupArray.setDetail(inGeo);                                                                                      \
    }                                                                                                                               \
                                                                                                                                    \
    SYS_FORCE_INLINE virtual void setRef##NUM##DetailBase(const GA_Detail* const inGeo)                                             \
    { setRef##NUM##DetailBase(reinterpret_cast<const gfe::Geometry*>(inGeo)); }                                                        \
                                                                                                                                    \
                                                                                                                                    \
                                                                                                                                    \
public:                                                                                                                             \
    GroupParser groupParserRef##NUM;                                                                                                \
    const GA_Attribute* posRef##NUM##Attrib = nullptr;                                                                              \
                                                                                                                                    \
protected:                                                                                                                          \
    const gfe::Geometry* geoRef##NUM;                                                                                                  \
                                                                                                                                    \
private:                                                                                                                            \
    const SOP_NodeVerb::CookParms* cookparmsRef##NUM;                                                                               \
    GOP_Manager* gopRef##NUM;                                                                                                       \
                                                                                                                                    \
                                                                                                                                    \
    gfe::RefAttributeArray ref##NUM##AttribArray;                                                                                    \
    gfe::RefGroupArray     ref##NUM##GroupArray;                                                                                     \
                                                                                                                                    \
}; // End of class GeoFilterRef


__GeoFilterRef_SPECILIZATION(0)
__GeoFilterRef_SPECILIZATION(1)
__GeoFilterRef_SPECILIZATION(2)
__GeoFilterRef_SPECILIZATION(3)
__GeoFilterRef_SPECILIZATION(4)
        
#undef __GeoFilterRef_SPECILIZATION

#endif






class AttribFilter : public GeoFilter {

public:
    
    AttribFilter(
        gfe::Geometry* const geo,
        const SOP_NodeVerb::CookParms* const cookparms = nullptr
    )
        : GeoFilter(geo, cookparms)
        , outAttribArray(geo, cookparms)
        , outGroupArray(geo, cookparms)
    {
    }

    AttribFilter(
        GA_Detail* const geo,
        const SOP_NodeVerb::CookParms* const cookparms = nullptr
    )
        : GeoFilter(geo, cookparms)
        , outAttribArray(geo, cookparms)
        , outGroupArray(geo, cookparms)
    {
    }

    AttribFilter(
        GA_Detail& geo,
        const SOP_NodeVerb::CookParms* const cookparms = nullptr
    )
        : GeoFilter(geo, cookparms)
        , outAttribArray(geo, cookparms)
        , outGroupArray(geo, cookparms)
    {
    }

    AttribFilter(
        GA_Detail& geo,
        const SOP_NodeVerb::CookParms& cookparms
    )
        : GeoFilter(geo, cookparms)
        , outAttribArray(geo, cookparms)
        , outGroupArray(geo, cookparms)
    {
    }


    
    AttribFilter(
        gfe::Geometry* const geo,
        const GA_Detail* const geoSrc,
        const SOP_NodeVerb::CookParms* const cookparms = nullptr
    )
        : GeoFilter(geo, geoSrc, cookparms)
        , outAttribArray(geo, cookparms)
        , outGroupArray(geo, cookparms)
    {
        //UT_ASSERT_MSG(geo, "do not find geo");
    }

    AttribFilter(
        GA_Detail& geo,
        const GA_Detail& geoSrc,
        const SOP_NodeVerb::CookParms& cookparms
    )
        : GeoFilter(geo, geoSrc, cookparms)
        , outAttribArray(geo, cookparms)
        , outGroupArray(geo, cookparms)
    {
        //UT_ASSERT_MSG(geo, "do not find geo");
    }

    AttribFilter(
        GA_Detail& geo,
        const GA_Detail& geoSrc,
        const SOP_NodeVerb::CookParms* const cookparms = nullptr
    )
        : GeoFilter(geo, geoSrc, cookparms)
        , outAttribArray(geo, cookparms)
        , outGroupArray(geo, cookparms)
    {
        //UT_ASSERT_MSG(geo, "do not find geo");
    }

    AttribFilter(
        GA_Detail& geo,
        const GA_Detail* const geoSrc,
        const SOP_NodeVerb::CookParms* const cookparms
    )
        : GeoFilter(geo, geoSrc, cookparms)
        , outAttribArray(geo, cookparms)
        , outGroupArray(geo, cookparms)
    {
        //UT_ASSERT_MSG(geo, "do not find geo");
    }

    
    ~AttribFilter()
    {
    }


    virtual void reset(GA_Detail& geo, const SOP_NodeVerb::CookParms* const cookparms = nullptr) override
    {
        GeoFilter::reset(geo, cookparms);
        outAttribArray.reset(geo, cookparms);
        outGroupArray. reset(geo, cookparms);
    }

    SYS_FORCE_INLINE virtual void reset(GA_Detail* const geo, const SOP_NodeVerb::CookParms* const cookparms = nullptr) override
    { reset(*geo, cookparms); }


    
    virtual void bumpDataId() const override
    {
        if (doDelGroupElement)
            bumpDataIdsForAddOrRemove();
        else
        {
            outAttribArray.bumpDataId();
            outGroupArray.bumpDataId();
        }
    }

    SYS_FORCE_INLINE virtual GA_Group* findOrCreateGroup(const bool detached, const GA_GroupType groupType, const UT_StringRef& name = "")
    { return getOutGroupArray().findOrCreate(detached, groupType, name); }

    SYS_FORCE_INLINE virtual GA_Group* findOrCreateGroup(const GA_GroupType groupType, const UT_StringRef& groupName = "")
    { return findOrCreateGroup(doDelGroupElement, groupType, groupName); }

    SYS_FORCE_INLINE virtual GA_Group* findOrCreateGroup(const bool detached, const GA_AttributeOwner attribOwner, const UT_StringRef& name = "")
    { return findOrCreateGroup(detached, gfe::attributeOwner_groupType(attribOwner), name); }

    SYS_FORCE_INLINE virtual GA_Group* findOrCreateGroup(const GA_AttributeOwner attribOwner, const UT_StringRef& name = "")
    { return findOrCreateGroup(doDelGroupElement, attribOwner, name); }

    
    SYS_FORCE_INLINE virtual GA_PrimitiveGroup* findOrCreatePrimitiveGroup(const bool detached = true, const UT_StringRef& name = "")
    { return getOutGroupArray().findOrCreatePrimitive(detached, name); }

    SYS_FORCE_INLINE virtual GA_PointGroup*  findOrCreatePointGroup (const bool detached = true, const UT_StringRef& name = "")
    { return getOutGroupArray().findOrCreatePoint(detached, name); }

    SYS_FORCE_INLINE virtual GA_VertexGroup* findOrCreateVertexGroup(const bool detached = true, const UT_StringRef& name = "")
    { return getOutGroupArray().findOrCreateVertex(detached, name); }

    SYS_FORCE_INLINE virtual GA_EdgeGroup*   findOrCreateEdgeGroup  (const bool detached = true, const UT_StringRef& name = "")
    { return getOutGroupArray().findOrCreateEdge(detached, name); }



virtual void delGroupElement(const GA_Group* group = nullptr)
{
    if (!group)
    {
        if (outGroupArray.isEmpty() || !outGroupArray[0])
            return;
        group = outGroupArray[0];
    }

    if (group->classType() == GA_GROUP_EDGE)
    {
        geo->asGU_Detail()->dissolveEdges(static_cast<const GA_EdgeGroup&>(*group),
            false, 0, delUnusedPoint, GU_Detail::GU_BRIDGEMODE_BRIDGE, false, false);
    }
    else
    {
        const GA_Range range(static_cast<const GA_ElementGroup&>(*group));
        switch (group->classType())
        {
        case GA_GROUP_PRIMITIVE:
            geo->destroyPrimitiveOffsets(range, delUnusedPoint);
        break;
        case GA_GROUP_POINT:
            geo->destroyPointOffsets(range, GA_Detail::GA_DestroyPointMode::GA_DESTROY_DEGENERATE_INCOMPATIBLE);
        break;
        case GA_GROUP_VERTEX:
            geo->destroyVertexOffsets(range);
        break;
        }
    }
}

virtual SYS_FORCE_INLINE void visualizeOutGroup() const
{ if (!doDelGroupElement && !outGroupArray.isEmpty()) visualizeGroup(outGroupArray[0]); }

    
SYS_FORCE_INLINE void delOrVisualizeGroup()
{ doDelGroupElement ? delGroupElement() : visualizeOutGroup(); }





#if 0
    const size_t sizeAttrib = getOutAttribArray().size();
    for (size_t i = 0; i < sizeAttrib; ++i)
    {
        attrib = getOutAttribArray()[i];
        switch (getOutAttribArray()[i]->getOwner())
        {
        case GA_ATTRIB_PRIMITIVE: break;
        case GA_ATTRIB_VERTEX:    break;
        case GA_ATTRIB_POINT:     break;
        case GA_ATTRIB_DETAIL:    break;
        default: UT_ASSERT_MSG(0, "Unhandled Attrib Owner"); break;
        }
        const GA_AIFTuple* const aifTuple = attrib->getAIFTuple();
        if (aifTuple)
        {
            switch (aifTuple->getStorage(attrib))
            {
            case GA_STORE_INT8:      break;
            case GA_STORE_INT16:     break;
            case GA_STORE_INT32:     break;
            case GA_STORE_INT64:     break;
            case GA_STORE_REAL16:    break;
            case GA_STORE_REAL32:    break;
            case GA_STORE_REAL64:    break;
            default: UT_ASSERT_MSG(0, "Unhandled Attrib Storage"); break;
            }
        }
        else
        {
            const GA_AIFNumericArray* const aifNumArray = attrib->getAIFNumericArray();
            if (aifNumArray)
            {
                switch (aifNumArray->getStorage(attrib))
                {
                case GA_STORE_INT8:      break;
                case GA_STORE_INT16:     break;
                case GA_STORE_INT32:     break;
                case GA_STORE_INT64:     break;
                case GA_STORE_REAL16:    break;
                case GA_STORE_REAL32:    break;
                case GA_STORE_REAL64:    break;
                default: UT_ASSERT_MSG(0, "Unhandled Attrib Storage"); break;
                }
            }
        }
    }
#endif
    
SYS_FORCE_INLINE AttributeArray& getOutAttribArray()
{ return outAttribArray; }


    
#if 0
    const size_t sizeGroup = getOutGroupArray().size();
    for (size_t i = 0; i < sizeGroup; ++i)
    {
        switch (getOutGroupArray()[i]->classType())
        {
        case GA_GROUP_PRIMITIVE: break;
        case GA_GROUP_POINT:     break;
        case GA_GROUP_VERTEX:    break;
        case GA_GROUP_EDGE:      break;
        default: UT_ASSERT_MSG(0, "Unhandled Group Type"); break;
        }
    }
#endif

SYS_FORCE_INLINE gfe::GroupArray& getOutGroupArray()
{ return outGroupArray; }

SYS_FORCE_INLINE std::vector<GA_Attribute*>& getOutAttribArrayRef()
{ return outAttribArray.ref(); }

SYS_FORCE_INLINE std::vector<GA_Group*>& getOutGroupArrayRef()
{ return outGroupArray.ref(); }



SYS_FORCE_INLINE const AttributeArray& getOutAttribArray() const
{ return outAttribArray; } 

SYS_FORCE_INLINE const GroupArray& getOutGroupArray() const
{ return outGroupArray; }

SYS_FORCE_INLINE const std::vector<GA_Attribute*>& getOutAttribArrayRef() const
{ return outAttribArray.ref(); }

SYS_FORCE_INLINE const std::vector<GA_Group*>& getOutGroupArrayRef() const
{ return outGroupArray.ref(); }


    
private:

SYS_FORCE_INLINE virtual void setDetailBase(GA_Detail& inGeo) override
{
    setDetail(inGeo);
    outAttribArray.setDetail(inGeo);
    outGroupArray.setDetail(inGeo);
}



public:
    GFE_GroupSetter groupSetter;
    //bool reverseOutGroup = false;
    bool doDelGroupElement = false;
    bool delUnusedPoint = true;
    
private:
    //std::vector<GA_AttributeUPtr> attribUPtrArray;
    //std::vector<GA_Attribute*> attribArray;
    //std::vector<GA_GroupUPtr> groupUPtrArray;
    //std::vector<GA_Group*> groupArray;
    AttributeArray outAttribArray;
    GroupArray outGroupArray;


}; // End of class AttribFilter



















class AttribCreateFilter : public AttribFilter {

public:

    AttribCreateFilter(
        gfe::Geometry* const geo,
        const SOP_NodeVerb::CookParms* const cookparms = nullptr
    )
        : AttribFilter(geo, cookparms)
        , inAttribArray(geo, cookparms)
        , inGroupArray(geo, cookparms)
    {
    }

    AttribCreateFilter(
        GA_Detail* const geo,
        const SOP_NodeVerb::CookParms* const cookparms = nullptr
    )
        : AttribFilter(geo, cookparms)
        , inAttribArray(geo, cookparms)
        , inGroupArray(geo, cookparms)
    {
    }

    AttribCreateFilter(
        GA_Detail& geo,
        const SOP_NodeVerb::CookParms* const cookparms = nullptr
    )
        : AttribFilter(geo, cookparms)
        , inAttribArray(geo, cookparms)
        , inGroupArray(geo, cookparms)
    {
    }

    AttribCreateFilter(
        GA_Detail& geo,
        const SOP_NodeVerb::CookParms& cookparms
    )
        : AttribFilter(geo, cookparms)
        , inAttribArray(geo, cookparms)
        , inGroupArray(geo, cookparms)
    {
    }



    ~AttribCreateFilter()
    {
    }





    virtual void reset(GA_Detail& geo, const SOP_NodeVerb::CookParms* const cookparms = nullptr) override
    {
        AttribFilter::reset(geo, cookparms);
        inAttribArray.reset(geo, cookparms);
        inGroupArray.reset(geo, cookparms);
    }

    SYS_FORCE_INLINE virtual void reset(GA_Detail* const geo, const SOP_NodeVerb::CookParms* const cookparms = nullptr) override
    { reset(*geo, cookparms); }


    
    SYS_FORCE_INLINE AttributeArray& getInAttribArray()
    { return inAttribArray; }
    
    SYS_FORCE_INLINE GroupArray& getInGroupArray()
    { return inGroupArray; }
    
    SYS_FORCE_INLINE std::vector<GA_Attribute*>& getInAttribArrayRef()
    { return inAttribArray.ref(); }
    
    SYS_FORCE_INLINE std::vector<GA_Group*>& getInGroupArrayRef()
    { return inGroupArray.ref(); }

    SYS_FORCE_INLINE const AttributeArray& getInAttribArray() const
    { return inAttribArray; }
    
    SYS_FORCE_INLINE const GroupArray& getInGroupArray() const
    { return inGroupArray; }
    
    SYS_FORCE_INLINE const std::vector<GA_Attribute*>& getInAttribArrayRef() const
    { return inAttribArray.ref(); }
    
    SYS_FORCE_INLINE const std::vector<GA_Group*>& getInGroupArrayRef() const
    { return inGroupArray.ref(); }


    SYS_FORCE_INLINE void setInAttribBumpDataId(bool val = true)
    { inAttribBumpDataId = val; }
    
    SYS_FORCE_INLINE void setInGroupBumpDataId(bool val = true)
    { inGroupBumpDataId = val; }

    virtual void bumpDataId() const override
    {
        if (doDelGroupElement)
            bumpDataIdsForAddOrRemove();
        else
        {
            getOutAttribArray().bumpDataId();
            getOutGroupArray().bumpDataId();
            if (inAttribBumpDataId)
                inAttribArray.bumpDataId();
            if (inGroupBumpDataId)
                inGroupArray.bumpDataId();
        }
    }

    void visualizeInGroup()
    {
        if (cookparms && !inGroupArray.isEmpty() && inGroupArray[0] && !inGroupArray[0]->isDetached())
            cookparms->select(*inGroupArray[0]);
        //cookparms->getNode()->setHighlight(true);
    }


private:

    SYS_FORCE_INLINE virtual void setDetailBase(GA_Detail& inGeo) override
    {
        setDetail(inGeo);
        inAttribArray.setDetail(inGeo);
        inGroupArray.setDetail(inGeo);
    }


private:
    bool inAttribBumpDataId = false;
    bool inGroupBumpDataId = false;
    AttributeArray inAttribArray;
    GroupArray inGroupArray;

}; // End of class AttribCreateFilter
















class GeoFilterWithRef0 : public GeoFilter, public GeoFilterRef0 {

public:

    GeoFilterWithRef0(
        gfe::Geometry* const geo,
        const GA_Detail* const geoRef0,
        const SOP_NodeVerb::CookParms* const cookparms = nullptr
    )
        : GeoFilter(geo, cookparms)
        , GeoFilterRef0(geoRef0, groupParser.getGOPRef(), cookparms)
    {
    }

    GeoFilterWithRef0(
        gfe::Geometry& geo,
        const GA_Detail* const geoRef0,
        const SOP_NodeVerb::CookParms* const cookparms = nullptr
    )
        : GeoFilter(geo, cookparms)
        , GeoFilterRef0(geoRef0, groupParser.getGOPRef(), cookparms)
    {
    }

    GeoFilterWithRef0(
        GA_Detail& geo,
        const GA_Detail* const geoRef0,
        const SOP_NodeVerb::CookParms* const cookparms = nullptr
    )
        : GeoFilter(geo, cookparms)
        , GeoFilterRef0(geoRef0, groupParser.getGOPRef(), cookparms)
    {
    }


    GeoFilterWithRef0(
        GA_Detail* const geo,
        const GA_Detail* const geoRef0,
        const SOP_NodeVerb::CookParms* const cookparms = nullptr
    )
        : GeoFilter(geo, cookparms)
        , GeoFilterRef0(geoRef0, groupParser.getGOPRef(), cookparms)
    {
    }

    GeoFilterWithRef0(
        GA_Detail* const geo,
        const GA_Detail& geoRef0,
        const SOP_NodeVerb::CookParms* const cookparms = nullptr
    )
        : GeoFilter(geo, cookparms)
        , GeoFilterRef0(geoRef0, groupParser.getGOPRef(), cookparms)
    {
    }

    GeoFilterWithRef0(
        GA_Detail& geo,
        const GA_Detail* const geoRef0,
        const SOP_NodeVerb::CookParms& cookparms
    )
        : GeoFilter(geo, cookparms)
        , GeoFilterRef0(geoRef0, groupParser.getGOPRef(), cookparms)
    {
    }

    GeoFilterWithRef0(
        GA_Detail& geo,
        const GA_Detail& geoRef0,
        const SOP_NodeVerb::CookParms& cookparms
    )
        : GeoFilter(geo, cookparms)
        , GeoFilterRef0(geoRef0, groupParser.getGOPRef(), &cookparms)
    {
    }


    
    GeoFilterWithRef0(
        GA_Detail& geo,
        const GA_Detail* const geoSrc,
        const GA_Detail* const geoRef0,
        const SOP_NodeVerb::CookParms* const cookparms
    )
        : GeoFilter(geo, geoSrc, cookparms)
        , GeoFilterRef0(geoRef0, groupParser.getGOPRef(), cookparms)
    {
    }


    ~GeoFilterWithRef0()
    {
    }



}; // End of class GeoFilterWithRef0




class GeoFilterWithRef1 : public GeoFilterWithRef0, public GeoFilterRef1 {

public:

    GeoFilterWithRef1(
        gfe::Geometry* const geo,
        const GA_Detail* const geoRef0,
        const GA_Detail* const geoRef1,
        const SOP_NodeVerb::CookParms* const cookparms = nullptr
    )
        : GeoFilterWithRef0(geo, geoRef0, cookparms)
        , GeoFilterRef1(geoRef1, groupParser.getGOPRef(), cookparms)
    {
    }

    GeoFilterWithRef1(
        gfe::Geometry& geo,
        const GA_Detail* const geoRef0,
        const GA_Detail* const geoRef1,
        const SOP_NodeVerb::CookParms* const cookparms = nullptr
    )
        : GeoFilterWithRef0(geo, geoRef0, cookparms)
        , GeoFilterRef1(geoRef1, groupParser.getGOPRef(), cookparms)
    {
    }

    GeoFilterWithRef1(
        GA_Detail& geo,
        const GA_Detail* const geoRef0,
        const GA_Detail* const geoRef1,
        const SOP_NodeVerb::CookParms* const cookparms = nullptr
    )
        : GeoFilterWithRef0(geo, geoRef0, cookparms)
        , GeoFilterRef1(geoRef1, groupParser.getGOPRef(), cookparms)
    {
    }


    GeoFilterWithRef1(
        GA_Detail* const geo,
        const GA_Detail* const geoRef0,
        const GA_Detail* const geoRef1,
        const SOP_NodeVerb::CookParms* const cookparms = nullptr
    )
        : GeoFilterWithRef0(geo, geoRef0, cookparms)
        , GeoFilterRef1(geoRef1, groupParser.getGOPRef(), cookparms)
    {
    }

    GeoFilterWithRef1(
        GA_Detail* const geo,
        const GA_Detail& geoRef0,
        const GA_Detail* const geoRef1,
        const SOP_NodeVerb::CookParms* const cookparms = nullptr
    )
        : GeoFilterWithRef0(geo, geoRef0, cookparms)
        , GeoFilterRef1(geoRef1, groupParser.getGOPRef(), cookparms)
    {
    }

    GeoFilterWithRef1(
        GA_Detail& geo,
        const GA_Detail* const geoRef0,
        const GA_Detail* const geoRef1,
        const SOP_NodeVerb::CookParms& cookparms
    )
        : GeoFilterWithRef0(geo, geoRef0, cookparms)
        , GeoFilterRef1(geoRef1, groupParser.getGOPRef(), cookparms)
    {
    }

    GeoFilterWithRef1(
        GA_Detail& geo,
        const GA_Detail& geoRef,
        const GA_Detail* const geoRef1,
        const SOP_NodeVerb::CookParms& cookparms
    )
        : GeoFilterWithRef0(geo, geoRef0, cookparms)
        , GeoFilterRef1(geoRef1, groupParser.getGOPRef(), cookparms)
    {
    }



    
    GeoFilterWithRef1(
        GA_Detail& geo,
        const GA_Detail* const geoSrc,
        const GA_Detail* const geoRef0,
        const GA_Detail* const geoRef1,
        const SOP_NodeVerb::CookParms* const cookparms = nullptr
    )
        : GeoFilterWithRef0(geo, geoSrc, geoRef0, cookparms)
        , GeoFilterRef1(geoRef1, groupParser.getGOPRef(), cookparms)
    {
    }


    
    ~GeoFilterWithRef1()
    {
    }

}; // End of class GeoFilterWithRef1




class AttribFilterWithRef0 : public AttribFilter, public GeoFilterRef0 {

public:

    AttribFilterWithRef0(
        gfe::Geometry* const geo,
        const GA_Detail* const geoRef,
        const SOP_NodeVerb::CookParms* const cookparms = nullptr
    )
        : AttribFilter(geo, cookparms)
        , GeoFilterRef0(geoRef, groupParser.getGOPRef(), cookparms)
    {
    }

    AttribFilterWithRef0(
        gfe::Geometry& geo,
        const GA_Detail* const geoRef,
        const SOP_NodeVerb::CookParms* const cookparms = nullptr
    )
        : AttribFilter(geo, cookparms)
        , GeoFilterRef0(geoRef, groupParser.getGOPRef(), cookparms)
    {
    }

    AttribFilterWithRef0(
        GA_Detail& geo,
        const GA_Detail* const geoRef,
        const SOP_NodeVerb::CookParms* const cookparms = nullptr
    )
        : AttribFilter(geo, cookparms)
        , GeoFilterRef0(geoRef, groupParser.getGOPRef(), cookparms)
    {
    }


    AttribFilterWithRef0(
        GA_Detail* const geo,
        const GA_Detail* const geoRef,
        const SOP_NodeVerb::CookParms* const cookparms = nullptr
    )
        : AttribFilter(geo, cookparms)
        , GeoFilterRef0(geoRef, groupParser.getGOPRef(), cookparms)
    {
    }

    AttribFilterWithRef0(
        GA_Detail* const geo,
        const GA_Detail& geoRef,
        const SOP_NodeVerb::CookParms* const cookparms = nullptr
    )
        : AttribFilter(geo, cookparms)
        , GeoFilterRef0(geoRef, groupParser.getGOPRef(), cookparms)
    {
    }

    AttribFilterWithRef0(
        GA_Detail& geo,
        const GA_Detail* const geoRef,
        const SOP_NodeVerb::CookParms& cookparms
    )
        : AttribFilter(geo, cookparms)
        , GeoFilterRef0(geoRef, groupParser.getGOPRef(), cookparms)
    {
    }

    AttribFilterWithRef0(
        GA_Detail& geo,
        const GA_Detail& geoRef,
        const SOP_NodeVerb::CookParms& cookparms
    )
        : AttribFilter(geo, cookparms)
        , GeoFilterRef0(geoRef, groupParser.getGOPRef(), &cookparms)
    {
    }


    ~AttribFilterWithRef0()
    {
    }



}; // End of class AttribFilterWithRef0





class AttribCreateFilterWithRef0 : public AttribCreateFilter, public GeoFilterRef0 {

public:

    AttribCreateFilterWithRef0(
        gfe::Geometry* const geo,
        const SOP_NodeVerb::CookParms* const cookparms = nullptr
    )
        : AttribCreateFilter(geo, cookparms)
        , GeoFilterRef0(reinterpret_cast<const GA_Detail*>(nullptr), groupParser.getGOPRef(), cookparms)
    {
    }

    AttribCreateFilterWithRef0(
        GA_Detail& geo,
        const SOP_NodeVerb::CookParms* const cookparms = nullptr
    )
        : AttribCreateFilter(geo, cookparms)
        , GeoFilterRef0(reinterpret_cast<const GA_Detail*>(nullptr), groupParser.getGOPRef(), cookparms)
    {
    }

    AttribCreateFilterWithRef0(
        GA_Detail& geo,
        const SOP_NodeVerb::CookParms& cookparms
    )
        : AttribCreateFilter(geo, cookparms)
        , GeoFilterRef0(nullptr, groupParser.getGOPRef(), cookparms)
    {
    }

    
    AttribCreateFilterWithRef0(
        gfe::Geometry* const geo,
        const GA_Detail* const geoRef,
        const SOP_NodeVerb::CookParms* const cookparms = nullptr
    )
        : AttribCreateFilter(geo, cookparms)
        , GeoFilterRef0(geoRef, groupParser.getGOPRef(), cookparms)
    {
    }

    AttribCreateFilterWithRef0(
        gfe::Geometry& geo,
        const GA_Detail* const geoRef,
        const SOP_NodeVerb::CookParms* const cookparms = nullptr
    )
        : AttribCreateFilter(geo, cookparms)
        , GeoFilterRef0(geoRef, groupParser.getGOPRef(), cookparms)
    {
    }

    AttribCreateFilterWithRef0(
        GA_Detail& geo,
        const GA_Detail* const geoRef,
        const SOP_NodeVerb::CookParms* const cookparms = nullptr
    )
        : AttribCreateFilter(geo, cookparms)
        , GeoFilterRef0(geoRef, groupParser.getGOPRef(), cookparms)
    {
    }


    AttribCreateFilterWithRef0(
        GA_Detail* const geo,
        const GA_Detail* const geoRef,
        const SOP_NodeVerb::CookParms* const cookparms = nullptr
    )
        : AttribCreateFilter(geo, cookparms)
        , GeoFilterRef0(geoRef, groupParser.getGOPRef(), cookparms)
    {
    }

    AttribCreateFilterWithRef0(
        GA_Detail* const geo,
        const GA_Detail& geoRef,
        const SOP_NodeVerb::CookParms* const cookparms = nullptr
    )
        : AttribCreateFilter(geo, cookparms)
        , GeoFilterRef0(geoRef, groupParser.getGOPRef(), cookparms)
    {
    }

    AttribCreateFilterWithRef0(
        GA_Detail& geo,
        const GA_Detail* const geoRef,
        const SOP_NodeVerb::CookParms& cookparms
    )
        : AttribCreateFilter(geo, cookparms)
        , GeoFilterRef0(geoRef, groupParser.getGOPRef(), cookparms)
    {
    }

    AttribCreateFilterWithRef0(
        GA_Detail& geo,
        const GA_Detail& geoRef,
        const SOP_NodeVerb::CookParms& cookparms
    )
        : AttribCreateFilter(geo, cookparms)
        , GeoFilterRef0(geoRef, groupParser.getGOPRef(), &cookparms)
    {
    }


    ~AttribCreateFilterWithRef0()
    {
    }


    SYS_FORCE_INLINE void appendInRef0Attribs(const GA_AttributeOwner attribClass, const UT_StringRef& attribPattern)
    {
        if (geoRef0)
            getRef0AttribArray().appends(attribClass, attribPattern);
        else
            getInAttribArray().appends(attribClass, attribPattern);
    }

}; // End of class AttribCreateFilterWithRef0
















class AttribFilterWithRef1 : public AttribFilterWithRef0, public GeoFilterRef1 {

public:

    AttribFilterWithRef1(
        gfe::Geometry* const geo,
        const GA_Detail* const geoRef,
        const GA_Detail* const geoRef1,
        const SOP_NodeVerb::CookParms* const cookparms = nullptr
    )
        : AttribFilterWithRef0(geo, geoRef, cookparms)
        , GeoFilterRef1(geoRef1, groupParser.getGOPRef(), cookparms)
    {
    }

    AttribFilterWithRef1(
        gfe::Geometry& geo,
        const GA_Detail* const geoRef,
        const GA_Detail* const geoRef1,
        const SOP_NodeVerb::CookParms* const cookparms = nullptr
    )
        : AttribFilterWithRef0(geo, geoRef, cookparms)
        , GeoFilterRef1(geoRef1, groupParser.getGOPRef(), cookparms)
    {
    }

    AttribFilterWithRef1(
        GA_Detail& geo,
        const GA_Detail* const geoRef,
        const GA_Detail* const geoRef1,
        const SOP_NodeVerb::CookParms* const cookparms = nullptr
    )
        : AttribFilterWithRef0(geo, geoRef, cookparms)
        , GeoFilterRef1(geoRef1, groupParser.getGOPRef(), cookparms)
    {
    }


    AttribFilterWithRef1(
        GA_Detail* const geo,
        const GA_Detail* const geoRef,
        const GA_Detail* const geoRef1,
        const SOP_NodeVerb::CookParms* const cookparms = nullptr
    )
        : AttribFilterWithRef0(geo, geoRef, cookparms)
        , GeoFilterRef1(geoRef1, groupParser.getGOPRef(), cookparms)
    {
    }

    AttribFilterWithRef1(
        GA_Detail* const geo,
        const GA_Detail& geoRef,
        const GA_Detail& geoRef1,
        const SOP_NodeVerb::CookParms* const cookparms = nullptr
    )
        : AttribFilterWithRef0(geo, geoRef, cookparms)
        , GeoFilterRef1(geoRef1, groupParser.getGOPRef(), cookparms)
    {
    }

    AttribFilterWithRef1(
        GA_Detail& geo,
        const GA_Detail* const geoRef,
        const GA_Detail* const geoRef1,
        const SOP_NodeVerb::CookParms& cookparms
    )
        : AttribFilterWithRef0(geo, geoRef, cookparms)
        , GeoFilterRef1(geoRef1, groupParser.getGOPRef(), cookparms)
    {
    }

    AttribFilterWithRef1(
        GA_Detail& geo,
        const GA_Detail& geoRef,
        const GA_Detail& geoRef1,
        const SOP_NodeVerb::CookParms& cookparms
    )
        : AttribFilterWithRef0(geo, geoRef, cookparms)
        , GeoFilterRef1(geoRef1, groupParser.getGOPRef(), &cookparms)
    {
    }


    ~AttribFilterWithRef1()
    {
    }



}; // End of class AttribFilterWithRef1






class AttribFilterWithRef2 : public AttribFilterWithRef1, public GeoFilterRef2 {

public:

    AttribFilterWithRef2(
        gfe::Geometry* const geo,
        const GA_Detail* const geoRef,
        const GA_Detail* const geoRef1,
        const GA_Detail* const geoRef2,
        const SOP_NodeVerb::CookParms* const cookparms = nullptr
    )
        : AttribFilterWithRef1(geo, geoRef, geoRef1, cookparms)
        , GeoFilterRef2(geoRef2, groupParser.getGOPRef(), cookparms)
    {
    }

    AttribFilterWithRef2(
        gfe::Geometry& geo,
        const GA_Detail* const geoRef,
        const GA_Detail* const geoRef1,
        const GA_Detail* const geoRef2,
        const SOP_NodeVerb::CookParms* const cookparms = nullptr
    )
        : AttribFilterWithRef1(geo, geoRef, geoRef1, cookparms)
        , GeoFilterRef2(geoRef2, groupParser.getGOPRef(), cookparms)
    {
    }

    AttribFilterWithRef2(
        GA_Detail& geo,
        const GA_Detail* const geoRef,
        const GA_Detail* const geoRef1,
        const GA_Detail* const geoRef2,
        const SOP_NodeVerb::CookParms* const cookparms = nullptr
    )
        : AttribFilterWithRef1(geo, geoRef, geoRef1, cookparms)
        , GeoFilterRef2(geoRef2, groupParser.getGOPRef(), cookparms)
    {
    }


    AttribFilterWithRef2(
        GA_Detail* const geo,
        const GA_Detail* const geoRef,
        const GA_Detail* const geoRef1,
        const GA_Detail* const geoRef2,
        const SOP_NodeVerb::CookParms* const cookparms = nullptr
    )
        : AttribFilterWithRef1(geo, geoRef, geoRef1, cookparms)
        , GeoFilterRef2(geoRef2, groupParser.getGOPRef(), cookparms)
    {
    }

    AttribFilterWithRef2(
        GA_Detail* const geo,
        const GA_Detail& geoRef,
        const GA_Detail& geoRef1,
        const GA_Detail* const geoRef2,
        const SOP_NodeVerb::CookParms* const cookparms = nullptr
    )
        : AttribFilterWithRef1(geo, geoRef, geoRef1, cookparms)
        , GeoFilterRef2(geoRef2, groupParser.getGOPRef(), cookparms)
    {
    }

    AttribFilterWithRef2(
        GA_Detail& geo,
        const GA_Detail* const geoRef,
        const GA_Detail* const geoRef1,
        const GA_Detail* const geoRef2,
        const SOP_NodeVerb::CookParms& cookparms
    )
        : AttribFilterWithRef1(geo, geoRef, geoRef1, cookparms)
        , GeoFilterRef2(geoRef2, groupParser.getGOPRef(), cookparms)
    {
    }

    AttribFilterWithRef2(
        GA_Detail& geo,
        const GA_Detail& geoRef,
        const GA_Detail& geoRef1,
        const GA_Detail* const geoRef2,
        const SOP_NodeVerb::CookParms& cookparms
    )
        : AttribFilterWithRef1(geo, geoRef, geoRef1, cookparms)
        , GeoFilterRef2(geoRef2, groupParser.getGOPRef(), &cookparms)
    {
    }


    ~AttribFilterWithRef2()
    {
    }


}; // End of class AttribFilterWithRef2



_GFE_END
#endif
