
#pragma once

#ifndef __GFE_VolumeArray_h__
#define __GFE_VolumeArray_h__

#include <GFE/VolumeArray.h>

#include <GU/GU_PrimVolume.h>

#include <GFE/Geometry.h>

//class GU_PrimVolume;

//for (std::vector<GU_PrimVolume*>::iterator it = volumeArray.ref().begin(); it != volumeArray.ref().end(); ++it) {
//    GU_PrimVolume* ptr = *it;
//    if (ptr->isDetached())
//        continue;
//}
//
//for (std::vector<GA_Group*>::iterator it = groupArray.ref().begin(); it != groupArray.ref().end(); ++it)
//{
//    GA_Group* ptr = *it;
//    if (!ptr->isDetached())
//        continue;
//}


_GFE_BEGIN
class VolumeArray {
    
public:
    VolumeArray(
        GA_Detail* const geo,
        const SOP_NodeVerb::CookParms* const cookparms = nullptr
    )
        : geo(reinterpret_cast<GFE_Detail*>(geo))
        , cookparms(cookparms)
    {
        volumeArray.reserve(16);
    }

    VolumeArray(
        GA_Detail* const geo,
        const SOP_NodeVerb::CookParms& cookparms
    )
        : geo(reinterpret_cast<GFE_Detail*>(geo))
        , cookparms(&cookparms)
    {
        volumeArray.reserve(16);
    }

    VolumeArray(
        GA_Detail& geo,
        const SOP_NodeVerb::CookParms* const cookparms = nullptr
    )
        : geo(reinterpret_cast<GFE_Detail*>(&geo))
        , cookparms(cookparms)
    {
        volumeArray.reserve(16);
    }

    VolumeArray(
        GA_Detail& geo,
        const SOP_NodeVerb::CookParms& cookparms
    )
        : geo(reinterpret_cast<GFE_Detail*>(&geo))
        , cookparms(&cookparms)
    {
        volumeArray.reserve(16);
    }

    ~VolumeArray()
    {
    }



    
    
    SYS_FORCE_INLINE GU_PrimVolume* &operator[](const size_t i)
    { return volumeArray[i]; }

    SYS_FORCE_INLINE GU_PrimVolume* operator[](const size_t i) const
    { return volumeArray[i]; }

    SYS_FORCE_INLINE bool isEmpty() const
    { return volumeArray.size() == 0; }
    
    SYS_FORCE_INLINE void erase(const size_t i)
    { volumeArray.erase(volumeArray.begin()+i); }

    void destroy(const size_t i)
    {
        UT_ASSERT_P(i < volumeArray.size());
        geo->destroyPrimitive(reinterpret_cast<GA_Primitive&>(*volumeArray[i]));
        erase(i);
    }
    
    void destroyAll()
    {
        const size_t sizeAttrib = volumeArray.size();
        for (size_t i = 0; i < sizeAttrib; ++i)
        {
            geo->destroyPrimitive(reinterpret_cast<GA_Primitive&>(*volumeArray[i]));
        }
        volumeArray.clear();
    }
    
    SYS_FORCE_INLINE void clear()
    { volumeArray.clear(); }

    SYS_FORCE_INLINE std::vector<GU_PrimVolume*>::size_type size() const
    { return volumeArray.size(); }

    void reset(GA_Detail* const inGeo, const SOP_NodeVerb::CookParms* const cookparms = nullptr)
    {
        geo = reinterpret_cast<GFE_Detail*>(geo);
        this->cookparms = cookparms;
        clear();
    }

    void reset(GA_Detail& inGeo, const SOP_NodeVerb::CookParms* const cookparms = nullptr)
    {
        geo = reinterpret_cast<GFE_Detail*>(&geo);
        this->cookparms = cookparms;
        clear();
    }

    
    void set(GU_PrimVolume* const attribPtr)
    {
        if (attribPtr)
        {
            clear();
            volumeArray.emplace_back(attribPtr);
        }
    }

    SYS_FORCE_INLINE void set(GU_PrimVolume& attribPtr)
    {
        clear();
        volumeArray.emplace_back(&attribPtr);
    }

    //GU_PrimVolume* set(const GA_AttributeOwner attribClass, const UT_StringRef& attribPattern)
    //{
    //    if (gfe::isInvalid(attribPattern))
    //        return nullptr;
    //
    //    GU_PrimVolume* const attribPtr = geo->byname(attribClass, attribPattern);
    //    set(attribPtr);
    //    return attribPtr;
    //}
    //
    //GU_PrimVolume* set(const GA_AttributeOwner attribClass, const char* const attribPattern)
    //{
    //    if (!attribPattern)
    //        return nullptr;
    //
    //    GU_PrimVolume* attribPtr = geo->findAttribute(attribClass, attribPattern);
    //    set(attribPtr);
    //    return attribPtr;
    //}




    void uappend(GU_PrimVolume* const attrib)
    {
        const size_t size = volumeArray.size();
        for (size_t i = 0; i < size; ++i)
        {
            if (attrib == volumeArray[i])
                return;
        }
        volumeArray.emplace_back(attrib);
        //volumeArray.emplace_back();
    }
    
    void unique()
    {
        const size_t size = volumeArray.size();
        if (size <= 0)
            return;
        
        const auto begin = volumeArray.begin();
        for (size_t i = size-1; ; --i)
        {
            const GU_PrimVolume* const attrib = volumeArray[i];
            for (size_t j = 0; j < i; ++j)
            {
                if (attrib == volumeArray[j])
                {
                    volumeArray.erase(begin+i);
                    break;
                }
            }
            if (i <= 0)
                break;
        }
    }
    
    
    void append(const VolumeArray& involumeArray)
    {
        const size_t size = involumeArray.size();
        for (size_t i = 0; i < size; ++i)
        {
            volumeArray.emplace_back(involumeArray[i]);
        }
    }
    
    SYS_FORCE_INLINE void set(const VolumeArray& involumeArray)
    { clear(); append(involumeArray); }
    
    SYS_FORCE_INLINE void append(GU_PrimVolume* const attrib)
    { if (attrib) volumeArray.emplace_back(attrib); }

    SYS_FORCE_INLINE void append(GU_PrimVolume& attrib)
    { volumeArray.emplace_back(&attrib); }

    GU_PrimVolume* append(const char* const name, const char* const nameAttribName = "name")
    {
        if (gfe::isInvalid(name))
            return nullptr;
        GU_PrimVolume* const vol = reinterpret_cast<GU_PrimVolume*>(geo->asGEO_Detail()->findPrimitiveByName(name, GEO_PrimTypeCompat::GEOPRIMVOLUME, nameAttribName));
        append(vol);
        return vol;
    }

   

    void appends(const UT_Array<GU_PrimVolume*>& prims)
    {
        const size_t size = prims.size();
        for (size_t i = 0; i < size; ++i)
        {
            append(prims[i]);
        }
    }
    
    void appends(const UT_Array<GEO_Primitive*>& prims)
    {
        const size_t size = prims.size();
        for (size_t i = 0; i < size; ++i)
        {
            UT_ASSERT_P(dynamic_cast<GU_PrimVolume*>(prims[i]));
            append(reinterpret_cast<GU_PrimVolume*>(prims[i]));
        }
    }
    
    void appends(const char* const name, const char* const nameAttribName = "name")
    {
        if (gfe::isInvalid(name))
            return;
        
        UT_Array<GEO_Primitive*> prims;
        geo->asGEO_Detail()->findAllPrimitivesByName(prims, name, GEO_PrimTypeCompat::GEOPRIMVOLUME, nameAttribName);
        
        appends(prims);
    }
    
    SYS_FORCE_INLINE void appends(const GA_AttributeOwner attribClass, const UT_StringRef& attribPattern)
    { appends(attribClass, attribPattern.c_str()); }

    SYS_FORCE_INLINE void appendPrimitives(const char* const attribPattern)
    { appends(GA_ATTRIB_PRIMITIVE, attribPattern); }

    SYS_FORCE_INLINE void appendPoints(const char* const attribPattern)
    { appends(GA_ATTRIB_POINT, attribPattern); }

    SYS_FORCE_INLINE void appendVertices(const char* const attribPattern)
    { appends(GA_ATTRIB_VERTEX, attribPattern); }

    SYS_FORCE_INLINE void appendDetails(const char* const attribPattern)
    { appends(GA_ATTRIB_DETAIL, attribPattern); }
    

GU_PrimVolume*
findOrCreate(
    const bool detached = true,
    const GA_AttributeOwner owner = GA_ATTRIB_POINT,
    const GA_StorageClass storageClass = GA_STORECLASS_FLOAT,
    const GA_Storage storage = GA_STORE_INVALID,
    const UT_StringRef& attribName = "",
    const int tupleSize = 1,
    const GA_Defaults& defaults = GA_Defaults(0.0f),
    const bool emplaceBack = true,
    const UT_Options* const create_args = nullptr,
    const GA_AttributeOptions* const attribute_options = nullptr
)
{
}


    
template<typename T>
SYS_FORCE_INLINE GU_PrimVolume*
findOrCreate(
    const bool detached = true,
    const GA_AttributeOwner owner = GA_ATTRIB_POINT,
    const UT_StringRef& attribName = "",
    const int tupleSize = 1,
    const GA_Defaults& defaults = GA_Defaults(0.0f),
    const bool emplaceBack = true,
    const UT_Options* const create_args = nullptr,
    const GA_AttributeOptions* const attribute_options = nullptr
)
{
    if      constexpr(std::is_same_v<T, fpreal16>)
        return findOrCreate(detached, owner, GA_STORECLASS_FLOAT,  GA_STORE_REAL16, attribName, tupleSize, defaults, emplaceBack, create_args, attribute_options);
    else if constexpr(std::is_same_v<T, fpreal32>)
        return findOrCreate(detached, owner, GA_STORECLASS_FLOAT,  GA_STORE_REAL32, attribName, tupleSize, defaults, emplaceBack, create_args, attribute_options);
    else if constexpr(std::is_same_v<T, fpreal64>)
        return findOrCreate(detached, owner, GA_STORECLASS_FLOAT,  GA_STORE_REAL64, attribName, tupleSize, defaults, emplaceBack, create_args, attribute_options);
    else if constexpr(std::is_same_v<T, int16>)
        return findOrCreate(detached, owner, GA_STORECLASS_INT,    GA_STORE_INT16,  attribName, tupleSize, defaults, emplaceBack, create_args, attribute_options);
    else if constexpr(std::is_same_v<T, int32>)
        return findOrCreate(detached, owner, GA_STORECLASS_INT,    GA_STORE_INT32,  attribName, tupleSize, defaults, emplaceBack, create_args, attribute_options);
    else if constexpr(std::is_same_v<T, int64>)
        return findOrCreate(detached, owner, GA_STORECLASS_INT,    GA_STORE_INT64,  attribName, tupleSize, defaults, emplaceBack, create_args, attribute_options);
    else if constexpr(std::is_same_v<T, UT_StringHolder>)
        return findOrCreate(detached, owner, GA_STORECLASS_STRING, GA_STORE_STRING, attribName, tupleSize, defaults, emplaceBack, create_args, attribute_options);
}

// SYS_FORCE_INLINE GU_PrimVolume* findOrCreate(
//     const bool detached = true,
//     const GA_AttributeOwner owner = GA_ATTRIB_POINT,
//     const GA_Precision precision = GA_PRECISION_INVALID,
//     const GA_StorageClass storageClass = GA_STORECLASS_FLOAT,
//     const UT_StringRef& attribName = "",
//     const int tupleSize = 1,
//     const GA_Defaults& defaults = GA_Defaults(0.0f),
//     const bool emplaceBack = true,
//     const UT_Options* const create_args = nullptr,
//     const GA_AttributeOptions* const attribute_options = nullptr
// )
// {
//     const GA_Storage storage = gfe::getPreferredStorage(precision, storageClass);
//     return findOrCreate(detached, owner, storageClass, storage, attribName, tupleSize, defaults, emplaceBack, create_args, attribute_options);
// }

SYS_FORCE_INLINE GU_PrimVolume*
    findOrCreateOffset(
        const bool detached = true,
        const GA_AttributeOwner owner = GA_ATTRIB_POINT,
        const GA_Storage storage = GA_STORE_INVALID,
        const UT_StringRef& attribName = "",
        const bool emplaceBack = true
)
{ return findOrCreate(detached, owner, GA_STORECLASS_INT, storage, attribName, 1, GA_Defaults(gfe::INVALID_OFFSET), emplaceBack); }





SYS_FORCE_INLINE void pop_back()
{ volumeArray.pop_back(); }

SYS_FORCE_INLINE GU_PrimVolume* last() const
{ return volumeArray[volumeArray.size()-1]; }


SYS_FORCE_INLINE std::vector<GU_PrimVolume*>& ref()
{ return volumeArray; }

SYS_FORCE_INLINE const std::vector<GU_PrimVolume*>& ref() const
{ return volumeArray; }








    
protected:
    SYS_FORCE_INLINE void setDetail(GA_Detail* const inGeo)
    { geo = reinterpret_cast<GFE_Detail*>(geo); }

    SYS_FORCE_INLINE void setDetail(GA_Detail& inGeo)
    { geo = reinterpret_cast<GFE_Detail*>(&geo); }


    
protected:
    GFE_Detail* geo;
    const SOP_NodeVerb::CookParms* cookparms;

private:
    std::vector<GU_PrimVolume*> volumeArray;

    friend class GFE_GeoFilter;

}; // End of class VolumeArray
_GFE_END
#endif
