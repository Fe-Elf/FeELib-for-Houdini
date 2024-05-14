
#pragma once

#ifndef __AttributeArray_h__
#define __AttributeArray_h__

#include <GFE/AttributeArray.h>

#include <GA/GA_Detail.h>


#include <GFE/Type.h>
#include <GFE/Attribute.h>

_GFE_BEGIN
class AttribFilter;
class AttribCreateFilter;

class GeoFilterRef0;
class GeoFilterRef1;
class GeoFilterRef2;
class GeoFilterRef3;
class GeoFilterRef4;
_GFE_END

//for (std::vector<GA_Attribute*>::iterator it = attribArray.ref().begin(); it != attribArray.ref().end(); ++it) {
//    GA_Attribute* ptr = *it;
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
class AttributeArray {
    
public:
    AttributeArray(
        GA_Detail* const geo,
        const SOP_NodeVerb::CookParms* const cookparms = nullptr
    )
        : geo(geo)
        , cookparms(cookparms)
    {
        attribUPtrArray.reserve(16);
        attribArray.reserve(16);
    }

    AttributeArray(
        GA_Detail* const geo,
        const SOP_NodeVerb::CookParms& cookparms
    )
        : geo(geo)
        , cookparms(&cookparms)
    {
        attribUPtrArray.reserve(16);
        attribArray.reserve(16);
    }

    AttributeArray(
        GA_Detail& geo,
        const SOP_NodeVerb::CookParms* const cookparms = nullptr
    )
        : geo(&geo)
        , cookparms(cookparms)
    {
        attribUPtrArray.reserve(16);
        attribArray.reserve(16);
    }

    AttributeArray(
        GA_Detail& geo,
        const SOP_NodeVerb::CookParms& cookparms
    )
        : geo(&geo)
        , cookparms(&cookparms)
    {
        attribUPtrArray.reserve(16);
        attribArray.reserve(16);
    }

    ~AttributeArray()
    {
    }



    
    
    SYS_FORCE_INLINE GA_Attribute* &operator[](const size_t i)
    { return attribArray[i]; }

    SYS_FORCE_INLINE GA_Attribute* operator[](const size_t i) const
    { return attribArray[i]; }

    SYS_FORCE_INLINE bool isEmpty() const
    { return attribArray.size() == 0; }
    
    SYS_FORCE_INLINE void erase(const size_t i)
    { attribArray.erase(attribArray.begin()+i); }

    void destroy(const size_t i)
    {
        UT_ASSERT_P(i < attribArray.size());
        if (!attribArray[i]->isDetached())
            geo->getAttributes().destroyAttribute(attribArray[i]);
        erase(i);
    }
    
    void destroyAll()
    {
        const size_t sizeAttrib = attribArray.size();
        for (size_t i = 0; i < sizeAttrib; ++i)
        {
            if (!attribArray[i]->isDetached() && !gfe::Type::stringEqual(attribArray[i]->getName().c_str(), "P"))
                geo->getAttributes().destroyAttribute(attribArray[i]);
        }
        attribArray.clear();
        attribUPtrArray.clear();
    }
    
    SYS_FORCE_INLINE GA_Attribute* clone(
        const GA_Attribute& attrib,
        const GA_AttributeOwner owner,
        const UT_StringHolder& name,
        const bool cloneOption = true,
        const GA_DataIdStrategy dataIdStrategy = GA_DATA_ID_BUMP,
        const GA_ReuseStrategy& reuse = GA_ReuseStrategy()
    )
    {
        GA_Attribute* outAttrib = geo->getAttributes().cloneAttribute(owner, name, attrib, cloneOption, dataIdStrategy, reuse);
        append(outAttrib);
        return outAttrib;
    }
    
    //SYS_FORCE_INLINE GA_Attribute* clone(
    //    const GA_AttributeOwner owner,
    //    const GA_Attribute& attrib,
    //    const bool cloneOption
    //)
    //{
    //    GA_Attribute* outAttrib = geo->getAttributes().cloneTempAttribute(owner, attrib, cloneOption);
    //    append(outAttrib);
    //    return outAttrib;
    //}
    
    
    GA_Attribute* cloneDetached(
        const GA_Attribute& attrib,
        const GA_AttributeOwner owner,
        const bool cloneOption = true
    )
    {
        GA_Attribute* outAttrib = geo->getAttributes().cloneTempAttribute(owner, attrib, cloneOption);
        appendDetached(outAttrib);
        return outAttrib;
    }
    
    SYS_FORCE_INLINE GA_Attribute* cloneDetached(
        const GA_Attribute& attrib,
        const bool cloneOption = true
    )
    { return cloneDetached(attrib, attrib.getOwner(), cloneOption); }
    
    
    SYS_FORCE_INLINE GA_Attribute* clone(const GA_Attribute& attrib)
    { return clone(attrib, attrib.getOwner(), attrib.getName(), true); }


    
    SYS_FORCE_INLINE GA_Attribute* clone(
        const bool detached,
        const GA_Attribute& attrib,
        const GA_AttributeOwner owner,
        const UT_StringHolder& name,
        const bool cloneOption = true
    )
    { return detached ? cloneDetached(attrib, owner, cloneOption) : clone(attrib, owner, name, cloneOption); }
    
    
    SYS_FORCE_INLINE void clear()
    { attribArray.clear(); attribUPtrArray.clear(); }

    SYS_FORCE_INLINE std::vector<GA_Attribute*>::size_type size() const
    { return attribArray.size(); }

    void reset(GA_Detail* const inGeo, const SOP_NodeVerb::CookParms* const cookparms = nullptr)
    {
        geo = inGeo;
        this->cookparms = cookparms;
        clear();
    }

    void reset(GA_Detail& inGeo, const SOP_NodeVerb::CookParms* const cookparms = nullptr)
    {
        geo = &inGeo;
        this->cookparms = cookparms;
        clear();
    }

    
    void set(GA_Attribute* const attribPtr)
    {
        if (attribPtr)
        {
            clear();
            attribArray.emplace_back(attribPtr);
        }
    }

    SYS_FORCE_INLINE void set(GA_Attribute& attribPtr)
    {
        clear();
        attribArray.emplace_back(&attribPtr);
    }

    GA_Attribute* set(const GA_AttributeOwner attribClass, const UT_StringRef& attribPattern)
    {
        if (gfe::Type::isInvalid(attribPattern))
            return nullptr;

        GA_Attribute* const attribPtr = geo->findAttribute(attribClass, attribPattern);
        set(attribPtr);
        return attribPtr;
    }

    GA_Attribute* set(const GA_AttributeOwner attribClass, const char* const attribPattern)
    {
        if (!attribPattern)
            return nullptr;

        GA_Attribute* attribPtr = geo->findAttribute(attribClass, attribPattern);
        set(attribPtr);
        return attribPtr;
    }




    void uappend(GA_Attribute* const attrib)
    {
        const size_t size = attribArray.size();
        for (size_t i = 0; i < size; ++i)
        {
            if (attrib == attribArray[i])
                return;
        }
        attribArray.emplace_back(attrib);
        //attribArray.emplace_back();
    }
    
    void unique()
    {
        const size_t size = attribArray.size();
        if (size <= 0)
            return;
        
        const auto begin = attribArray.begin();
        for (size_t i = size-1; ; --i)
        {
            const GA_Attribute* const attrib = attribArray[i];
            for (size_t j = 0; j < i; ++j)
            {
                if (attrib == attribArray[j])
                {
                    attribArray.erase(begin+i);
                    break;
                }
            }
            if (i <= 0)
                break;
        }
    }
    
    
    void append(const AttributeArray& inAttribArray)
    {
        const size_t size = inAttribArray.size();
        for (size_t i = 0; i < size; ++i)
        {
            attribArray.emplace_back(inAttribArray[i]);
        }
    }
    
    SYS_FORCE_INLINE void set(const AttributeArray& inAttribArray)
    { clear(); append(inAttribArray); }
    
    SYS_FORCE_INLINE void appendDetached(GA_Attribute* const attrib)
    {
        if (attrib)
        {
            attribUPtrArray.emplace_back(GA_AttributeUPtr(attrib));
            attribArray.emplace_back(attrib);
        }
    }

    SYS_FORCE_INLINE void appendUPtr(GA_Attribute* const attrib)
    { if (attrib) attribUPtrArray.emplace_back(GA_AttributeUPtr(attrib)); }

    SYS_FORCE_INLINE void append(GA_Attribute* const attrib)
    { if (attrib) attribArray.emplace_back(attrib); }

    SYS_FORCE_INLINE void append(GA_Attribute& attrib)
    { attribArray.emplace_back(&attrib); }

    GA_Attribute* append(const GA_AttributeOwner attribClass, const UT_StringRef& attribPattern)
    {
        if (gfe::Type::isInvalid(attribPattern))
            return nullptr;
        GA_Attribute* const attribPtr = geo->findAttribute(attribClass, attribPattern);
        append(attribPtr);
        return attribPtr;
    }

    GA_Attribute* appendUV(const UT_StringRef& attribPattern, const GA_AttributeOwner attribClass = GA_ATTRIB_INVALID)
    {
        if (gfe::Type::isInvalid(attribPattern))
            return nullptr;

        GA_Attribute* const attribPtr = gfe::Attribute::findUVAttributePointVertex(*geo, attribClass, attribPattern);

        append(attribPtr);
        return attribPtr;
    }




    void appends(const GA_AttributeOwner attribClass, const char* const attribPattern)
    {
        if (!attribPattern)
            return;

        for (GA_AttributeDict::iterator it = geo->getAttributes().begin(attribClass); !it.atEnd(); ++it)
        {
            GA_Attribute* const attribPtr = it.attrib();
            if (attribPtr->getScope() != GA_SCOPE_PUBLIC)
                continue;
            if (!attribPtr->getName().multiMatch(attribPattern))
                continue;
            attribArray.emplace_back(attribPtr);
        }
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



    void oappends(const GA_AttributeOwner attribClass, const char* const attribPattern)
    {
        if (!attribPattern)
            return;

        for (GA_AttributeDict::ordered_iterator it = geo->getAttributes().getDict(attribClass).obegin(GA_SCOPE_PUBLIC); !it.atEnd(); ++it)
        {
            GA_Attribute* const attribPtr = *it;
            if (!attribPtr->getName().multiMatch(attribPattern))
                continue;
            attribArray.emplace_back(attribPtr);
        }
    }

    SYS_FORCE_INLINE void oappends(const GA_AttributeOwner attribClass, const UT_StringRef& attribPattern)
    {
        if (gfe::Type::isValid(attribPattern))
            oappends(attribClass, attribPattern.c_str());
    }
    
    SYS_FORCE_INLINE void oappendPrims(const char* const attribPattern)
    { return oappends(GA_ATTRIB_PRIMITIVE, attribPattern); }

    SYS_FORCE_INLINE void oappendPoints(const char* const attribPattern)
    { return oappends(GA_ATTRIB_POINT, attribPattern); }
    
    SYS_FORCE_INLINE void oappendVertices(const char* const attribPattern)
    { return oappends(GA_ATTRIB_VERTEX, attribPattern); }
    
    SYS_FORCE_INLINE void oappendDetails(const char* const attribPattern)
    { return oappends(GA_ATTRIB_DETAIL, attribPattern); }


    
    SYS_FORCE_INLINE void appendPointVertexs(const UT_StringRef& attribPattern)
    {
        appends(GA_ATTRIB_POINT, attribPattern);
        appends(GA_ATTRIB_VERTEX, attribPattern);
    }


    GA_Attribute* createDetachedAttribute(
        const GA_AttributeOwner owner = GA_ATTRIB_POINT,
        // const GA_StorageClass storageClass = GA_STORECLASS_FLOAT,
        // const GA_Storage storage = GA_STORE_INVALID,
        const bool emplaceBack = true,
        const UT_StringRef& attribtype = "numeric",
        const UT_Options* const create_args = nullptr,
        const GA_AttributeOptions* const attribute_options = nullptr
    )
    {
        // const GA_Storage finalStorage = gfe::Type::getPreferredStorage(geo, storageClass, storage);

        attribUPtrArray.emplace_back(geo->createDetachedAttribute(owner, attribtype, create_args, attribute_options));
        GA_Attribute* attribPtr = attribUPtrArray[attribUPtrArray.size() - 1].get();

        if (emplaceBack)
            attribArray.emplace_back(attribPtr);
        return attribPtr;
    }


    GA_Attribute* createDetachedAttribute(
        const GA_AttributeOwner owner,
        //const GA_StorageClass storageClass,
        //const GA_Storage storage,
        const bool emplaceBack,
        const GA_AttributeType& attribtype,
        const UT_Options* const create_args = nullptr,
        const GA_AttributeOptions* const attribute_options = nullptr
    )
    {
        //const GA_Storage finalStorage = gfe::Type::getPreferredStorage(geo, storageClass, storage);

        attribUPtrArray.emplace_back(geo->createDetachedAttribute(owner, attribtype, create_args, attribute_options));
        GA_Attribute* attribPtr = attribUPtrArray[attribUPtrArray.size() - 1].get();

        if (emplaceBack)
            attribArray.emplace_back(attribPtr);
        return attribPtr;
    }


    GA_Attribute* createDetachedTupleAttribute(
        const GA_AttributeOwner owner = GA_ATTRIB_POINT,
        const GA_StorageClass storageClass = GA_STORECLASS_FLOAT,
        const GA_Storage storage = GA_STORE_INVALID,
        const int tupleSize = 1,
        const GA_Defaults& defaults = GA_Defaults(0.0f),
        const bool emplaceBack = true,
        const GA_AttributeOptions* const attribute_options = nullptr
    )
    {
        const GA_Storage finalStorage = gfe::Type::getPreferredStorage(geo, storageClass, storage);
    
        attribUPtrArray.emplace_back(geo->createDetachedTupleAttribute(owner, finalStorage, tupleSize, defaults, attribute_options));
        GA_Attribute* attribPtr = attribUPtrArray[attribUPtrArray.size() - 1].get();

        if (emplaceBack)
            attribArray.emplace_back(attribPtr);
        return attribPtr;
    }


    GA_Attribute* createDetachedStringAttribute(
        const GA_AttributeOwner owner = GA_ATTRIB_POINT,
        const bool emplaceBack = true,
        const GA_AttributeOptions* const attribute_options = nullptr
    )
    {
        attribUPtrArray.emplace_back(geo->createDetachedTupleAttribute(owner, GA_STORE_STRING, 1, GA_Defaults(0), attribute_options));
        GA_Attribute* attribPtr = attribUPtrArray[attribUPtrArray.size() - 1].get();

        if (emplaceBack)
            attribArray.emplace_back(attribPtr);
        return attribPtr;
    }

        
    
GA_Attribute* createDetachedArrayAttribute(
    const GA_AttributeOwner owner = GA_ATTRIB_POINT,
    // const GA_StorageClass storageClass = GA_STORECLASS_FLOAT,
    // const GA_Storage storage = GA_STORE_INVALID,
    // const int tupleSize = 1,
    const bool emplaceBack = true,
    const UT_Options* const create_args = nullptr,
    const GA_AttributeOptions* const attribute_options = nullptr
)
{
    //const GA_Storage finalStorage = gfe::Type::getPreferredStorage(geo, storageClass, storage);

    attribUPtrArray.emplace_back(geo->createDetachedAttribute(owner, "arraydata", create_args, attribute_options));
    GA_Attribute* attribPtr = attribUPtrArray[attribUPtrArray.size() - 1].get();

    if (emplaceBack)
        attribArray.emplace_back(attribPtr);
    return attribPtr;
}


GA_Attribute*
findOrCreateTuple(
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
    GA_Attribute* attribPtr = geo->findAttribute(owner, attribName);
    if (checkTupleAttrib(detached, attribPtr, storageClass, storage, tupleSize, nullptr, emplaceBack))
        return attribPtr;

    const GA_Storage finalStorage = gfe::Type::getPreferredStorage(geo, storageClass, storage);
        
    if (detached || gfe::Type::isInvalid(attribName))
    {
        if (finalStorage == GA_STORE_STRING)
            attribUPtrArray.emplace_back(geo->createDetachedAttribute(owner, "string", create_args, attribute_options));
        else
            attribUPtrArray.emplace_back(geo->createDetachedTupleAttribute(owner, finalStorage, tupleSize, defaults, attribute_options));
        attribPtr = attribUPtrArray[attribUPtrArray.size()-1].get();
        //attribPtr = attribUPtr.get();
    }
    else
    {
        if (finalStorage == GA_STORE_STRING)
            attribPtr = geo->createStringAttribute(owner, attribName, create_args, attribute_options);
        else
            attribPtr = geo->createTupleAttribute(owner, attribName, finalStorage,
                        tupleSize, defaults, create_args, attribute_options);
        
        if (attribPtr)
            uniquePointVertexAttrib(attribPtr);
        else
        {
            if (cookparms)
                cookparms->sopAddError(SOP_ATTRIBUTE_INVALID, attribName);
            UT_ASSERT_MSG(attribPtr, "No Attrib");
            return nullptr;
        }
    }
    if (emplaceBack)
        attribArray.emplace_back(attribPtr);
    return attribPtr;
}


    
template<typename T>
SYS_FORCE_INLINE GA_Attribute*
findOrCreateTuple(
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
        return findOrCreateTuple(detached, owner, GA_STORECLASS_FLOAT,  GA_STORE_REAL16, attribName, tupleSize, defaults, emplaceBack, create_args, attribute_options);
    else if constexpr(std::is_same_v<T, fpreal32>)
        return findOrCreateTuple(detached, owner, GA_STORECLASS_FLOAT,  GA_STORE_REAL32, attribName, tupleSize, defaults, emplaceBack, create_args, attribute_options);
    else if constexpr(std::is_same_v<T, fpreal64>)
        return findOrCreateTuple(detached, owner, GA_STORECLASS_FLOAT,  GA_STORE_REAL64, attribName, tupleSize, defaults, emplaceBack, create_args, attribute_options);
    else if constexpr(std::is_same_v<T, int16>)
        return findOrCreateTuple(detached, owner, GA_STORECLASS_INT,    GA_STORE_INT16,  attribName, tupleSize, defaults, emplaceBack, create_args, attribute_options);
    else if constexpr(std::is_same_v<T, int32>)
        return findOrCreateTuple(detached, owner, GA_STORECLASS_INT,    GA_STORE_INT32,  attribName, tupleSize, defaults, emplaceBack, create_args, attribute_options);
    else if constexpr(std::is_same_v<T, int64>)
        return findOrCreateTuple(detached, owner, GA_STORECLASS_INT,    GA_STORE_INT64,  attribName, tupleSize, defaults, emplaceBack, create_args, attribute_options);
    else if constexpr(std::is_same_v<T, UT_StringHolder>)
        return findOrCreateTuple(detached, owner, GA_STORECLASS_STRING, GA_STORE_STRING, attribName, tupleSize, defaults, emplaceBack, create_args, attribute_options);
}

// SYS_FORCE_INLINE GA_Attribute* findOrCreateTuple(
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
//     const GA_Storage storage = gfe::Type::getPreferredStorage(precision, storageClass);
//     return findOrCreateTuple(detached, owner, storageClass, storage, attribName, tupleSize, defaults, emplaceBack, create_args, attribute_options);
// }

SYS_FORCE_INLINE GA_Attribute*
    findOrCreateOffset(
        const bool detached = true,
        const GA_AttributeOwner owner = GA_ATTRIB_POINT,
        const GA_Storage storage = GA_STORE_INVALID,
        const UT_StringRef& attribName = "",
        const bool emplaceBack = true
)
{ return findOrCreateTuple(detached, owner, GA_STORECLASS_INT, storage, attribName, 1, GA_Defaults(gfe::INVALID_OFFSET), emplaceBack); }



    GA_Attribute*
    findOrCreateArray(
        const bool detached = true,
        const GA_AttributeOwner owner = GA_ATTRIB_POINT,
        const GA_StorageClass storageClass = GA_STORECLASS_FLOAT,
        const GA_Storage storage = GA_STORE_INVALID,
        const UT_StringRef& attribName = "",
        const int tupleSize = 1,
        const bool emplaceBack = true,
        const UT_Options* const create_args = nullptr,
        const GA_AttributeOptions* const attribute_options = nullptr
    )
    {
        GA_Attribute* attribPtr = geo->findAttribute(owner, attribName);
        if (checkArrayAttrib(detached, attribPtr, storageClass, storage, tupleSize, emplaceBack))
            return attribPtr;
        
        const GA_Storage finalStorage = gfe::Type::getPreferredStorage(geo, storageClass, storage);

        if (detached || gfe::Type::isInvalid(attribName))
        {
#if 0
            switch (finalStorage)
            {
            case GA_STORE_INT16:
                break;
            case GA_STORE_INT32:
                break;
            case GA_STORE_INT64:
                break;
            case GA_STORE_REAL16:
                break;
            case GA_STORE_REAL32:
                break;
            case GA_STORE_REAL64:
                break;
            case GA_STORE_STRING:
                break;
            case GA_STORE_DICT:
                break;
            default:
                break;
            }
            attribUPtrArray.emplace_back(static_cast<GEO_Detail*>(geo)->createDetachedAttribute(owner, "arraydata", tupleSize, create_args, attribute_options));
#else
            attribUPtrArray.emplace_back(geo->getAttributes().createDetachedAttribute(owner, "arraydata", create_args, attribute_options));
#endif
            attribPtr = attribUPtrArray[attribUPtrArray.size() - 1].get();
            //attribPtr = attribUPtr.get();
        }
        else
        {
            attribPtr = geo->getAttributes().createArrayAttribute(owner, GA_SCOPE_PUBLIC, attribName, finalStorage,
                tupleSize, create_args, attribute_options);

            if (attribPtr)
                uniquePointVertexAttrib(attribPtr);
            else
            {
                if (cookparms)
                    cookparms->sopAddError(SOP_ATTRIBUTE_INVALID, attribName);
                UT_ASSERT_MSG(attribPtr, "No Attrib");
                return nullptr;
            }
        }
        if (emplaceBack)
            attribArray.emplace_back(attribPtr);
        return attribPtr;
    }





    
//SYS_FORCE_INLINE
//void
//findOrCreate(
//    const GA_AttributeOwner owner,
//    const GA_Storage storage,
//    const UT_StringRef& attribName
//)
//{
//    findOrCreate(owner, storage, attribName);
//}





    GA_Attribute*
    findOrCreatePiece(
        const bool detached = true,
        const gfe::PieceAttribSearchOrder pieceAttribSearchOrder = gfe::PieceAttribSearchOrder::Primitive,
        const GA_AttributeOwner owner = GA_ATTRIB_PRIMITIVE,
        const GA_StorageClass storageClass = GA_STORECLASS_INT,
        const GA_Storage storage = GA_STORE_INVALID,
        const UT_StringRef& attribName = "",
        const int tupleSize = 1,
        const GA_Defaults& defaults = GA_Defaults(0.0f),
        const bool emplaceBack = true,
        const UT_Options* const create_args = nullptr,
        const GA_AttributeOptions* const attribute_options = nullptr
    )
    {

        GA_Attribute* attribPtr = findPieceAttrib(pieceAttribSearchOrder, attribName);
        if (checkTupleAttrib(detached, attribPtr, storageClass, storage, tupleSize, nullptr, emplaceBack))
            return attribPtr;

        const GA_Storage finalStorage = gfe::Type::getPreferredStorage(geo, storageClass, storage);
        // if (attribPtr)
        // {
        //     const bool promoteFromOtherClass = sopparms.getPromoteFromOtherClass();
        //     if (promoteFromOtherClass)
        //     {
        //         if (geo0AttribClass != attribPtr->getOwner())
        //         {
        //             attribPtr = GFE_Attribpr::attribPromote(outGeo0, attribPtr, geo0AttribClass);
        //             //attribPtr = GFE_AttribPromote::promote(*static_cast<GU_Detail*>(outGeo0), attribPtr, geo0AttribClass, sopparms.getDelOriginalAttrib(), GU_Promote::GU_PROMOTE_FIRST);
        //         }
        //     }
        //
        //     const bool forceCastAttribType = sopparms.getForceCastAttribType();
        //     if (forceCastAttribType)
        //     {
        //         GFE_AttributeCast::attribCast(outGeo0, attribPtr, connectivityStorageClass, "", outGeo0->getPreferredPrecision());
        //     }
        //     return;
        // }
        

        if (detached || gfe::Type::isInvalid(attribName))
        {
            attribUPtrArray.emplace_back(geo->createDetachedTupleAttribute(owner, finalStorage, tupleSize, defaults, attribute_options));
            attribPtr = attribUPtrArray[attribUPtrArray.size() - 1].get();
            //attribPtr = attribUPtr.get();
        }
        else
        {
            attribPtr = geo->createTupleAttribute(owner, attribName, finalStorage,
                tupleSize, defaults, create_args, attribute_options);
            
            if (attribPtr)
                uniquePointVertexAttrib(attribPtr);
            else
            {
                if (cookparms)
                    cookparms->sopAddError(SOP_ATTRIBUTE_INVALID, attribName);
                UT_ASSERT_MSG(attribPtr, "No Attrib");
                return nullptr;
            }
        }
        if (emplaceBack)
            attribArray.emplace_back(attribPtr);
        return attribPtr;
    }


    GA_Attribute*
    findPieceAttrib(
        const gfe::PieceAttribSearchOrder pieceAttribSearchOrder,
        const UT_StringRef& pieceAttribName
    )
    {
        GA_Attribute* attribPtr = nullptr;

        switch (pieceAttribSearchOrder)
        {
        case gfe::PieceAttribSearchOrder::Primitive:  attribPtr = geo->findAttribute(GA_ATTRIB_PRIMITIVE, pieceAttribName); break;
        case gfe::PieceAttribSearchOrder::Point:      attribPtr = geo->findAttribute(GA_ATTRIB_POINT,     pieceAttribName); break;
        case gfe::PieceAttribSearchOrder::Vertex:     attribPtr = geo->findAttribute(GA_ATTRIB_VERTEX,    pieceAttribName); break;
        case gfe::PieceAttribSearchOrder::PrimPoint:
            {
                GA_AttributeOwner searchOrder[2] = { GA_ATTRIB_PRIMITIVE, GA_ATTRIB_POINT };
                attribPtr = geo->findAttribute(pieceAttribName, searchOrder, 2);
            }
            break;
        case gfe::PieceAttribSearchOrder::PointPrim:
            {
                GA_AttributeOwner searchOrder[2] = { GA_ATTRIB_POINT, GA_ATTRIB_PRIMITIVE };
                attribPtr = geo->findAttribute(pieceAttribName, searchOrder, 2);
            }
            break;
        case gfe::PieceAttribSearchOrder::All:
            {
                GA_AttributeOwner searchOrder[3] = { GA_ATTRIB_PRIMITIVE, GA_ATTRIB_POINT, GA_ATTRIB_VERTEX };
                attribPtr = geo->findAttribute(pieceAttribName, searchOrder, 3);
            }
            break;
        default:
            UT_ASSERT_MSG(0, "Unhandled Geo Piece Attrib Search Order!");
            break;
        }
        return attribPtr;
    }





GA_Attribute*
findOrCreateUV(
    const bool detached = true,
    const GA_AttributeOwner owner = GA_ATTRIB_VERTEX,
    const GA_Storage storage = GA_STORE_INVALID,
    const UT_StringRef& attribName = "",
    const int tupleSize = 3,
    const GA_Defaults& defaults = GA_Defaults(0.0f),
    const bool emplaceBack = true,
    const UT_Options* const create_args = nullptr,
    const GA_AttributeOptions* const attribute_options = nullptr,
    const GA_StorageClass storageClass = GA_STORECLASS_FLOAT
)
{
    GA_Attribute* attribPtr = gfe::Attribute::findAttributePointVertex(*geo, owner, attribName);
    if (checkTupleAttrib(detached, attribPtr, storageClass, storage, tupleSize, nullptr, emplaceBack))
        return attribPtr;
        
    const GA_Storage finalStorage = gfe::Type::getPreferredStorage(geo, storageClass, storage);
        
    const GA_AttributeOwner validOwner = owner == GA_ATTRIB_POINT ? GA_ATTRIB_POINT : GA_ATTRIB_VERTEX;
    if (detached || gfe::Type::isInvalid(attribName))
    {
        attribUPtrArray.emplace_back(geo->createDetachedTupleAttribute(validOwner, finalStorage, tupleSize, defaults, attribute_options));
        attribPtr = attribUPtrArray[attribUPtrArray.size() - 1].get();
        //attribPtr = attribUPtr.get();
    }
    else
    {
#if 1
        const UT_Options& finalCreateArgs = create_args ? *create_args : UT_Options("uvw");

        //GA_AttributeOptions& finalOptions = attribute_options ? *attribute_options : GA_AttributeOptions();
        if (attribute_options)
        {
            attribPtr = geo->createTupleAttribute(validOwner, attribName, finalStorage,
                tupleSize, defaults, &finalCreateArgs, attribute_options);
        }
        else
        {
            GA_AttributeOptions finalOptions = GA_AttributeOptions();
            finalOptions.setTypeInfo(GA_TYPE_TEXTURE_COORD);
            attribPtr = geo->createTupleAttribute(validOwner, attribName, finalStorage,
                tupleSize, defaults, &finalCreateArgs, &finalOptions);
        }
#else
        attribPtr = static_cast<GEO_Detail*>(geo)->addTextureAttribute(owner == GA_ATTRIB_POINT ? GA_ATTRIB_POINT : GA_ATTRIB_VERTEX, finalStorage);
#endif
        

        if (attribPtr)
            uniquePointVertexAttrib(attribPtr);
        else
        {
            if (cookparms)
                cookparms->sopAddError(SOP_ATTRIBUTE_INVALID, attribName);
            UT_ASSERT_MSG(attribPtr, "No Attrib");
            return nullptr;
        }
        gfe::Attribute::renameAttribute(*attribPtr, attribName);
    }
    if (emplaceBack)
        attribArray.emplace_back(attribPtr);
    return attribPtr;
}

GA_Attribute*
findOrCreateDir(
    const bool detached = true,
    const GA_AttributeOwner owner = GA_ATTRIB_POINT,
    const GA_Storage storage = GA_STORE_INVALID,
    const UT_StringRef& attribName = "",
    const int tupleSize = 3,
    const GA_Defaults& defaults = GA_Defaults(0.0f),
    const bool emplaceBack = true,
    const UT_Options* const create_args = nullptr,
    const GA_AttributeOptions* const attribute_options = nullptr,
    const GA_StorageClass storageClass = GA_STORECLASS_FLOAT
)
{
    GA_Attribute* attribPtr = geo->findAttribute(owner, attribName);
    if (checkTupleAttrib(detached, attribPtr, storageClass, storage, tupleSize, nullptr, emplaceBack))
        return attribPtr;

    const GA_Storage finalStorage = gfe::Type::getPreferredStorage(geo, storageClass, storage);
        
    if (detached || gfe::Type::isInvalid(attribName))
    {
        attribUPtrArray.emplace_back(geo->createDetachedTupleAttribute(owner, finalStorage, tupleSize, defaults, attribute_options));
        attribPtr = attribUPtrArray[attribUPtrArray.size()-1].get();
        //attribPtr = attribUPtr.get();
    }
    else
    {
#if 1
        const UT_Options& finalCreateArgs = create_args ? *create_args : UT_Options("vector3");

        //GA_AttributeOptions& finalOptions = attribute_options ? *attribute_options : GA_AttributeOptions();
        if (attribute_options)
        {
            attribPtr = geo->createTupleAttribute(owner, attribName, finalStorage,
                tupleSize, defaults, &finalCreateArgs, attribute_options);
        }
        else
        {
            GA_AttributeOptions finalOptions = GA_AttributeOptions();
            finalOptions.setTypeInfo(GA_TYPE_NORMAL);
            attribPtr = geo->createTupleAttribute(owner, attribName, finalStorage,
                tupleSize, defaults, &finalCreateArgs, &finalOptions);
        }
#else
        attribPtr = static_cast<GEO_Detail*>(geo)->addTextureAttribute(owner == GA_ATTRIB_POINT ? GA_ATTRIB_POINT : GA_ATTRIB_VERTEX, finalStorage);
#endif

        if (attribPtr)
            uniquePointVertexAttrib(attribPtr);
        else
        {
            if (cookparms)
                cookparms->sopAddError(SOP_ATTRIBUTE_INVALID, attribName);
            UT_ASSERT_MSG(attribPtr, "No Attrib");
            return nullptr;
        }
        gfe::Attribute::renameAttribute(*attribPtr, attribName);
    }
    if (emplaceBack)
        attribArray.emplace_back(attribPtr);
    return attribPtr;
}


GA_Attribute*
findOrCreateNormal3D(
    const bool detached = true,
    const gfe::NormalSearchOrder owner = gfe::NormalSearchOrder::All,
    const GA_Storage storage = GA_STORE_INVALID,
    const UT_StringRef& attribName = "",
    const int tupleSize = 3,
    const GA_Defaults& defaults = GA_Defaults(0.0f),
    const bool emplaceBack = true,
    const UT_Options* const create_args = nullptr,
    const GA_AttributeOptions* const attribute_options = nullptr,
    const GA_StorageClass storageClass = GA_STORECLASS_FLOAT
)
{
    GA_Attribute* attribPtr = gfe::Attribute::findNormal3D(*geo, owner, attribName);
    if (checkTupleAttrib(detached, attribPtr, storageClass, storage, tupleSize, nullptr, emplaceBack))
        return attribPtr;
        
    const GA_Storage finalStorage = gfe::Type::getPreferredStorage(geo, storageClass, storage);
        
    const GA_AttributeOwner validOwner = gfe::Attribute::toValidOwner(owner);
    if (detached || gfe::Type::isInvalid(attribName))
    {
        attribUPtrArray.emplace_back(geo->createDetachedTupleAttribute(validOwner, finalStorage, tupleSize, defaults, attribute_options));
        attribPtr = attribUPtrArray[attribUPtrArray.size()-1].get();
        //attribPtr = attribUPtr.get();
    }
    else
    {
#if 1
        const UT_Options& finalCreateArgs = create_args ? *create_args : UT_Options("vector3");

        //GA_AttributeOptions& finalOptions = attribute_options ? *attribute_options : GA_AttributeOptions();
        if (attribute_options)
        {
            attribPtr = geo->createTupleAttribute(validOwner, attribName, finalStorage,
                tupleSize, defaults, &finalCreateArgs, attribute_options);
        }
        else
        {
            GA_AttributeOptions finalOptions = GA_AttributeOptions();
            finalOptions.setTypeInfo(GA_TYPE_NORMAL);
            attribPtr = geo->createTupleAttribute(validOwner, attribName, finalStorage,
                tupleSize, defaults, &finalCreateArgs, &finalOptions);
        }
#else
        attribPtr = static_cast<GEO_Detail*>(geo)->addTextureAttribute(owner == GA_ATTRIB_POINT ? GA_ATTRIB_POINT : GA_ATTRIB_VERTEX, finalStorage);
#endif
        
        if (attribPtr)
            uniquePointVertexAttrib(attribPtr);
        else
        {
            if (cookparms)
                cookparms->sopAddError(SOP_ATTRIBUTE_INVALID, attribName);
            UT_ASSERT_MSG(attribPtr, "No Attrib");
            return nullptr;
        }
        gfe::Attribute::renameAttribute(*attribPtr, attribName);
    }
    if (emplaceBack)
        attribArray.emplace_back(attribPtr);
    return attribPtr;
}





SYS_FORCE_INLINE void pop_back()
{ attribArray.pop_back(); }

SYS_FORCE_INLINE GA_Attribute* last() const
{ return attribArray[attribArray.size()-1]; }


void bumpDataId() const
{
    const size_t len = attribArray.size();
    for (size_t i = 0; i < len; i++)
    {
        GA_Attribute* const attrib = attribArray[i];
        if (attrib->isDetached())
            continue;
        attrib->bumpDataId();
    }
}

SYS_FORCE_INLINE std::vector<GA_Attribute*>& ref()
{ return attribArray; }

SYS_FORCE_INLINE const std::vector<GA_Attribute*>& ref() const
{ return attribArray; }








    
protected:
    SYS_FORCE_INLINE void setDetail(GA_Detail* const inGeo)
    { geo = inGeo; }

    SYS_FORCE_INLINE void setDetail(GA_Detail& inGeo)
    { geo = &inGeo; }


bool checkTupleAttrib(
    const bool detached,
    GA_Attribute*& attrib,
    const GA_StorageClass storageClass = GA_STORECLASS_INVALID,
    const GA_Storage storage = GA_STORE_INVALID,
    const int tupleSize = 3,
    const GA_Defaults* const defaults = nullptr,
    const bool emplaceBack = true
)
{
    if (!attrib)
        return false;
    if (gfe::Type::checkTupleAttrib(attrib, storageClass, storage, tupleSize, defaults))
    {
        if (emplaceBack)
            attribArray.emplace_back(attrib);
        return true;
    }
    else
    {
        if (!detached)
            geo->getAttributes().destroyAttribute(attrib);
        attrib = nullptr;
        return false;
    }
}

bool checkArrayAttrib(
    const bool detached,
    GA_Attribute*& attrib,
    const GA_StorageClass storageClass = GA_STORECLASS_INVALID,
    const GA_Storage storage = GA_STORE_INVALID,
    const int tupleSize = 3,
    const bool emplaceBack = true
)
{
    if (!attrib)
        return false;
    if (gfe::Type::checkArrayAttrib(attrib, storageClass, storage, tupleSize))
    {
        if (emplaceBack)
            attribArray.emplace_back(attrib);
        return true;
    }
    else
    {
        if (!detached)
            geo->getAttributes().destroyAttribute(attrib);
        attrib = nullptr;
        return false;
    }
}


void uniquePointVertexAttrib(GA_Attribute* const attrib)
{
    UT_ASSERT_P(attrib);
    if (size() <= 0 || (attrib->getOwner() != GA_ATTRIB_POINT && attrib->getOwner() != GA_ATTRIB_VERTEX))
        return;
    
    const UT_StringRef& attribName = attrib->getName();
    const GA_AttributeOwner ownerReverse = attrib->getOwner() == GA_ATTRIB_POINT ? GA_ATTRIB_VERTEX : GA_ATTRIB_POINT;
    const auto begin = attribArray.begin();
    
    for (size_t i = size()-1; ; --i)
    {
        GA_Attribute* const attribRef = attribArray[i];
        if (!attribRef->isDetached() && attribRef->getOwner() == ownerReverse && gfe::Type::stringEqual(attribRef->getName(), attribName))
        {
            attribArray.erase(begin + i);
        }
        if (i <= 0)
            break;
    }
}


    
    
protected:
    GA_Detail* geo;
    const SOP_NodeVerb::CookParms* cookparms;

private:
    std::vector<GA_AttributeUPtr> attribUPtrArray;
    std::vector<GA_Attribute*> attribArray;

    friend class gfe::AttribFilter;
    friend class gfe::AttribCreateFilter;

}; // End of class AttributeArray





class GroupArray {

public:
    GroupArray(
        GA_Detail* const geo,
        const SOP_NodeVerb::CookParms* const cookparms = nullptr
    )
        : geo(geo)
        , cookparms(cookparms)
    {
        groupUPtrArray.reserve(16);
        groupArray.reserve(16);
    }

    GroupArray(
        GA_Detail* const geo,
        const SOP_NodeVerb::CookParms& cookparms
    )
        : geo(geo)
        , cookparms(&cookparms)
    {
        groupUPtrArray.reserve(16);
        groupArray.reserve(16);
    }

    GroupArray(
        GA_Detail& geo,
        const SOP_NodeVerb::CookParms* const cookparms = nullptr
    )
        : geo(&geo)
        , cookparms(cookparms)
    {
        groupUPtrArray.reserve(16);
        groupArray.reserve(16);
    }

    GroupArray(
        GA_Detail& geo,
        const SOP_NodeVerb::CookParms& cookparms
    )
        : geo(&geo)
        , cookparms(&cookparms)
    {
        groupUPtrArray.reserve(16);
        groupArray.reserve(16);
    }
    
    ~GroupArray()
    {
    }


    SYS_FORCE_INLINE GA_Group* &operator[](const size_t i)
    { return groupArray[i]; }

    SYS_FORCE_INLINE GA_Group* const &operator[](const size_t i) const
    { return groupArray[i]; }


    void eraseByGroupType(const GA_GroupType groupType, const bool reverse = false)
    {
        const auto begin = groupArray.begin();
        if (groupArray.size() > 0)
        {
            for (size_t i = groupArray.size()-1; ; --i)
            {
                if (groupArray[i]->classType() == groupType ^ reverse)
                    groupArray.erase(begin+i);
                if (i == 0)
                    break;
            }
        }
        if (groupUPtrArray.size() > 0)
        {
            const auto uPtrbegin = groupUPtrArray.begin();
            for (size_t i = groupUPtrArray.size()-1; ; --i)
            {
                if (groupUPtrArray[i].get()->classType() == groupType ^ reverse)
                    groupUPtrArray.erase(uPtrbegin+i);
                if (i == 0)
                    break;
            }
        }
    }

    SYS_FORCE_INLINE void erasePrimitiveGroup()
    { eraseByGroupType(GA_GROUP_PRIMITIVE, false); }
    
    SYS_FORCE_INLINE void eraseNonPrimitiveGroup()
    { eraseByGroupType(GA_GROUP_PRIMITIVE, true); }

    SYS_FORCE_INLINE void erasePointGroup()
    { eraseByGroupType(GA_GROUP_POINT, false); }
    
    SYS_FORCE_INLINE void eraseNonPointGroup()
    { eraseByGroupType(GA_GROUP_POINT, true); }

    SYS_FORCE_INLINE void eraseVertexGroup()
    { eraseByGroupType(GA_GROUP_VERTEX, false); }
    
    SYS_FORCE_INLINE void eraseNonVertexGroup()
    { eraseByGroupType(GA_GROUP_VERTEX, true); }

    SYS_FORCE_INLINE void eraseEdgeGroup()
    { eraseByGroupType(GA_GROUP_EDGE, false); }
    
    SYS_FORCE_INLINE void eraseNonEdgeGroup()
    { eraseByGroupType(GA_GROUP_EDGE, true); }

    
    //SYS_FORCE_INLINE GA_Group* clone(const GA_Group& group, const GA_GroupType owner, const UT_StringRef& name)
    //{ return findOrCreate(false, owner, name); }
    //
    //SYS_FORCE_INLINE GA_ElementGroup* cloneElement(const GA_ElementGroup& group, const GA_GroupType owner, const UT_StringRef& name)
    //{ return findOrCreateElement(false, owner, name); }
    
    SYS_FORCE_INLINE GA_Group* clone(const GA_Group& group)
    { return findOrCreate(false, group.classType(), group.getName()); }
    
    SYS_FORCE_INLINE GA_ElementGroup* cloneElement(const GA_ElementGroup& group)
    { return findOrCreateElement(false, group.classType(), group.getName()); }
    
    GA_Group* cloneDetached(const GA_Group& group)
    {
        UT_ASSERT_P(geo);
        GA_Group* outGroup = geo->getGroupTable(group.classType())->newDetachedGroup();
        appendDetached(outGroup);
        return outGroup;
    }
    
    SYS_FORCE_INLINE GA_Group* clone(const bool detached, const GA_Group& group)
    { return detached ? cloneDetached(group) : clone(group); }
    
    
    
    SYS_FORCE_INLINE bool isElementGroup(const size_t i) const
    { return groupArray[i]->isElementGroup(); }

    SYS_FORCE_INLINE bool isPrimitiveGroup(const size_t i) const
    { return groupArray[i]->classType() == GA_GROUP_PRIMITIVE; }

    SYS_FORCE_INLINE bool isPointGroup(const size_t i) const
    { return groupArray[i]->classType() == GA_GROUP_POINT; }

    SYS_FORCE_INLINE bool isVertexGroup(const size_t i) const
    { return groupArray[i]->classType() == GA_GROUP_VERTEX; }
    
    SYS_FORCE_INLINE bool isEdgeGroup(const size_t i) const
    { return groupArray[i]->classType() == GA_GROUP_EDGE; }

    
    SYS_FORCE_INLINE GA_ElementGroup* getElementGroup(const size_t i) const
    {
        UT_ASSERT(groupArray[i]->isElementGroup());
        return static_cast<GA_ElementGroup*>(groupArray[i]);
    }

    SYS_FORCE_INLINE GA_ElementGroup* getElementGroup(const size_t i)
    {
        UT_ASSERT(groupArray[i]->isElementGroup());
        return static_cast<GA_ElementGroup*>(groupArray[i]);
    }

    SYS_FORCE_INLINE GA_PrimitiveGroup* getPrimitiveGroup(const size_t i) const
    {
        UT_ASSERT(groupArray[i]->classType() == GA_GROUP_PRIMITIVE);
        return static_cast<GA_PrimitiveGroup*>(groupArray[i]);
    }

    SYS_FORCE_INLINE GA_PointGroup* getPointGroup(const size_t i) const
    {
        UT_ASSERT(groupArray[i]->classType() == GA_GROUP_POINT);
        return static_cast<GA_PointGroup*>(groupArray[i]);
    }

    SYS_FORCE_INLINE GA_VertexGroup* getVertexGroup(const size_t i) const
    {
        UT_ASSERT(groupArray[i]->classType() == GA_GROUP_VERTEX);
        return static_cast<GA_VertexGroup*>(groupArray[i]);
    }

    SYS_FORCE_INLINE GA_EdgeGroup* getEdgeGroup(const size_t i) const
    {
        UT_ASSERT(groupArray[i]->classType() == GA_GROUP_EDGE);
        return static_cast<GA_EdgeGroup*>(groupArray[i]);
    }

    SYS_FORCE_INLINE bool isEmpty() const
    { return groupArray.size() == 0; }

    SYS_FORCE_INLINE void erase(const size_t i)
    { groupArray.erase(groupArray.begin()+i); }
    
    SYS_FORCE_INLINE void clear()
    {
        groupArray.clear();
        groupUPtrArray.clear();
    }

    void destroy(const size_t i)
    {
        UT_ASSERT_P(i < groupArray.size());
        if (!groupArray[i]->isDetached())
            geo->destroyGroup(groupArray[i]);
        erase(i);
    }
    
    void destroyAll()
    {
        const size_t sizeGroup = groupArray.size();
        for (size_t i = 0; i < sizeGroup; ++i)
        {
            if (!groupArray[i]->isDetached())
                geo->destroyGroup(groupArray[i]);
        }
        groupArray.clear();
        groupUPtrArray.clear();
    }
    
    SYS_FORCE_INLINE std::vector<GA_Group*>::size_type size() const
    { return groupArray.size(); }

    SYS_FORCE_INLINE GA_Group* last() const
    { return groupArray[groupArray.size() - 1]; }

    SYS_FORCE_INLINE void reset(GA_Detail* const inGeo, const SOP_NodeVerb::CookParms* const cookparms = nullptr)
    {
        geo = inGeo;
        this->cookparms = cookparms;
        clear();
    }

    SYS_FORCE_INLINE void reset(GA_Detail& inGeo, const SOP_NodeVerb::CookParms* const cookparms = nullptr)
    {
        geo = &inGeo;
        this->cookparms = cookparms;
        clear();
    }

    SYS_FORCE_INLINE void set(GA_Group* const groupPtr)
    {
        clear();
        if (groupPtr)
            groupArray.emplace_back(groupPtr);
    }

    SYS_FORCE_INLINE void set(GA_Group& groupPtr)
    {
        clear();
        groupArray.emplace_back(&groupPtr);
    }

    GA_Group* set(const GA_GroupType groupClass, const UT_StringRef& groupPattern)
    {
        if (gfe::Type::isInvalid(groupPattern))
            return nullptr;

        GA_Group* groupPtr = geo->getGroupTable(groupClass)->find(groupPattern);
        set(groupPtr);
        return groupPtr;
    }
    
    SYS_FORCE_INLINE void appendDetached(GA_Group* const group)
    {
        if (group)
        {
            groupUPtrArray.emplace_back(GA_GroupUPtr(group));
            groupArray.emplace_back(group);
        }
    }

    SYS_FORCE_INLINE void appendUPtr(GA_Group* const group)
    { if (group) groupUPtrArray.emplace_back(GA_GroupUPtr(group)); }

    SYS_FORCE_INLINE void append(GA_Group& group)
    { groupArray.emplace_back(&group); }

    SYS_FORCE_INLINE void append(GA_Group* const group)
    { if (group) groupArray.emplace_back(group); }

    void append(const GA_GroupType groupClass, const UT_StringRef& groupPattern)
    {
        if (gfe::Type::isValid(groupPattern))
        {
            GA_Group* groupPtr = geo->getGroupTable(groupClass)->find(groupPattern);
            append(groupPtr);
        }
    }
    
    void appends(const GA_GroupType groupClass, const char* const groupPattern)
    {
        if (!groupPattern)
            return;

        for (GA_GroupTable::iterator<GA_Group> it = geo->getGroupTable(groupClass)->beginTraverse(); !it.atEnd(); ++it)
        {
            GA_Group* const groupPtr = it.group();
            if (!groupPtr->getName().multiMatch(groupPattern))
                continue;
            groupArray.emplace_back(groupPtr);
        }
    }
    SYS_FORCE_INLINE void appends(const GA_GroupType groupClass, const UT_StringRef& groupPattern)
    { if (gfe::Type::isValid(groupPattern)) appends(groupClass, groupPattern.c_str()); }

    SYS_FORCE_INLINE void appendPrimitives(const char* const groupPattern)
    { return appends(GA_GROUP_PRIMITIVE, groupPattern); }

    SYS_FORCE_INLINE void appendPoints(const char* const groupPattern)
    { return appends(GA_GROUP_POINT, groupPattern); }

    SYS_FORCE_INLINE void appendVertexs(const char* const groupPattern)
    { return appends(GA_GROUP_VERTEX, groupPattern); }

    SYS_FORCE_INLINE void appendEdges(const char* const groupPattern)
    { return appends(GA_GROUP_EDGE, groupPattern); }

    void oappends(const GA_GroupType groupClass, const char* groupPattern)
    {
        if (!groupPattern)
            return;

        for (GA_GroupTable::ordered_iterator it = geo->getGroupTable(groupClass)->obegin(); !it.atEnd(); ++it)
        {
            GA_Group* const groupPtr = *it;
            if (!groupPtr->getName().multiMatch(groupPattern))
                continue;
            groupArray.emplace_back(groupPtr);
        }
    }
    SYS_FORCE_INLINE void oappends(const GA_GroupType groupClass, const UT_StringRef& groupPattern)
    {
        if (gfe::Type::isValid(groupPattern))
            appends(groupClass, groupPattern.c_str());
    }

    SYS_FORCE_INLINE void oappendPrims(const char* const attribPattern)
    { return oappends(GA_GROUP_PRIMITIVE, attribPattern); }

    SYS_FORCE_INLINE void oappendPoints(const char* const attribPattern)
    { return oappends(GA_GROUP_POINT, attribPattern); }
    
    SYS_FORCE_INLINE void oappendVertices(const char* const attribPattern)
    { return oappends(GA_GROUP_VERTEX, attribPattern); }
    
    SYS_FORCE_INLINE void oappendEdges(const char* const attribPattern)
    { return oappends(GA_GROUP_EDGE, attribPattern); }


    void append(const GroupArray& inArray)
    {
        const size_t size = inArray.size();
        for (size_t i = 0; i < size; ++i)
        {
            groupArray.emplace_back(inArray[i]);
        }
    }

    SYS_FORCE_INLINE void appends(const GA_AttributeOwner groupClass, const char* groupPattern)
    { appends(gfe::Type::attributeOwner_groupType(groupClass), groupPattern); }

    SYS_FORCE_INLINE void appends(const GA_AttributeOwner groupClass, const UT_StringRef& groupPattern)
    { (gfe::Type::attributeOwner_groupType(groupClass), groupPattern); }

    
    SYS_FORCE_INLINE void set(const GA_AttributeOwner groupClass, const UT_StringRef& groupPattern)
    { set(gfe::Type::attributeOwner_groupType(groupClass), groupPattern); }

    SYS_FORCE_INLINE void append(const GA_AttributeOwner groupClass, const UT_StringRef& groupPattern)
    { append(gfe::Type::attributeOwner_groupType(groupClass), groupPattern); }


    GA_Group* findOrCreate(
        const bool detached = true,
        const GA_GroupType groupType = GA_GROUP_POINT,
        const UT_StringRef& groupName = "",
        const bool emplaceBack = true
    )
    {
        GA_Group* groupPtr;
        if (detached || gfe::Type::isInvalid(groupName))
        {
            switch (groupType)
            {
            case GA_GROUP_PRIMITIVE: groupUPtrArray.emplace_back(geo->createDetachedPrimitiveGroup()); break;
            case GA_GROUP_POINT:     groupUPtrArray.emplace_back(geo->createDetachedPointGroup());     break;
            case GA_GROUP_VERTEX:    groupUPtrArray.emplace_back(geo->createDetachedVertexGroup());    break;
            case GA_GROUP_EDGE:      groupUPtrArray.emplace_back(geo->createDetachedEdgeGroup());      break;
            default:
                if (cookparms)
                    cookparms->sopAddError(SOP_ERR_BADGROUP, groupName);
                UT_ASSERT_MSG(0, "No Group");
                return nullptr;
                break;
            }
            groupPtr = groupUPtrArray[groupUPtrArray.size()-1].get();
            //groupPtr = attribUPtr.get();
        }
        else
        {
            if (gfe::Type::isInvalid(groupName))
            {
                if (cookparms)
                    cookparms->sopAddError(SOP_ERR_BADGROUP, groupName);
                return nullptr;
            }
            
            groupPtr = geo->getGroupTable(groupType)->find(groupName);

            if (!groupPtr)
                groupPtr = geo->getGroupTable(groupType)->newGroup(groupName);

            if (!groupPtr)
            {
                if (cookparms)
                    cookparms->sopAddError(SOP_ERR_BADGROUP, groupName);
                //UT_ASSERT_MSG(groupPtr, "Invalid Group Name");
                return nullptr;
            }
        }
        if (emplaceBack)
            groupArray.emplace_back(groupPtr);
        return groupPtr;
    }


    SYS_FORCE_INLINE GA_Group*
        findOrCreate(
            const bool detached = true,
            const GA_AttributeOwner owner = GA_ATTRIB_POINT,
            const UT_StringRef& groupName = "",
            const bool emplaceBack = true
        )
    { return findOrCreate(detached, gfe::Type::attributeOwner_groupType(owner), groupName, emplaceBack); }

    SYS_FORCE_INLINE GA_ElementGroup*
        findOrCreateElement(
            const bool detached = true,
            const GA_GroupType owner = GA_GROUP_POINT,
            const UT_StringRef& groupName = "",
            const bool emplaceBack = true
        )
    { return static_cast<GA_ElementGroup*>(findOrCreate(detached, owner, groupName, emplaceBack)); }

    SYS_FORCE_INLINE GA_ElementGroup*
        findOrCreateElement(
            const bool detached = true,
            const GA_AttributeOwner owner = GA_ATTRIB_POINT,
            const UT_StringRef& groupName = "",
            const bool emplaceBack = true
        )
    { return findOrCreateElement(detached, gfe::Type::attributeOwner_groupType(owner), groupName, emplaceBack); }


    SYS_FORCE_INLINE GA_PrimitiveGroup*
        findOrCreatePrimitive(
            const bool detached = true,
            const UT_StringRef& groupName = "",
            const bool emplaceBack = true
        )
    { return static_cast<GA_PrimitiveGroup*>(findOrCreate(detached, GA_GROUP_PRIMITIVE, groupName, emplaceBack)); }

    SYS_FORCE_INLINE GA_PointGroup*
        findOrCreatePoint(
            const bool detached = true,
            const UT_StringRef& groupName = "",
            const bool emplaceBack = true
        )
    { return static_cast<GA_PointGroup*>(findOrCreate(detached, GA_GROUP_POINT, groupName, emplaceBack)); }

    SYS_FORCE_INLINE GA_VertexGroup*
        findOrCreateVertex(
            const bool detached = true,
            const UT_StringRef& groupName = "",
            const bool emplaceBack = true
        )
    { return static_cast<GA_VertexGroup*>(findOrCreate(detached, GA_GROUP_VERTEX, groupName, emplaceBack)); }

    SYS_FORCE_INLINE GA_EdgeGroup*
        findOrCreateEdge(
            const bool detached = true,
            const UT_StringRef& groupName = "",
            const bool emplaceBack = true
        )
    { return static_cast<GA_EdgeGroup*>(findOrCreate(detached, GA_GROUP_EDGE, groupName, emplaceBack)); }

    
    
    void bumpDataId() const
    {
        const size_t size = groupArray.size();
        for (size_t i = 0; i < size; i++)
        {
            GA_Group* const group = groupArray[i];
            if (group->isDetached())
                continue;
            if (group->classType() == GA_GROUP_EDGE)
                static_cast<GA_EdgeGroup*>(group)->bumpDataId();
            else
                static_cast<GA_ElementGroup*>(group)->bumpDataId();
        }
    }

    SYS_FORCE_INLINE std::vector<GA_Group*>& ref()
    { return groupArray; }

    SYS_FORCE_INLINE const std::vector<GA_Group*>& ref() const
    { return groupArray; }


protected:
    SYS_FORCE_INLINE void setDetail(GA_Detail* const inGeo)
    { geo = inGeo; }

    SYS_FORCE_INLINE void setDetail(GA_Detail& inGeo)
    { geo = &inGeo; }




protected:
    GA_Detail* geo;
    const SOP_NodeVerb::CookParms* cookparms;

private:
    std::vector<GA_GroupUPtr> groupUPtrArray;
    std::vector<GA_Group*> groupArray;

    friend class gfe::AttribFilter;
    friend class gfe::AttribCreateFilter;

}; // End of class GroupArray







class RefAttributeArray {

public:
    RefAttributeArray(
        const GA_Detail* const geo,
        const SOP_NodeVerb::CookParms* const cookparms = nullptr
    )
        : geo(geo)
        , cookparms(cookparms)
    {
        attribUPtrArray.reserve(16);
        attribArray.reserve(16);
    }

    RefAttributeArray(
        const GA_Detail* const geo,
        const SOP_NodeVerb::CookParms& cookparms
    )
        : geo(geo)
        , cookparms(&cookparms)
    {
        attribUPtrArray.reserve(16);
        attribArray.reserve(16);
    }

    RefAttributeArray(
        const GA_Detail& geo,
        const SOP_NodeVerb::CookParms* const cookparms = nullptr
    )
        : geo(&geo)
        , cookparms(cookparms)
    {
        attribUPtrArray.reserve(16);
        attribArray.reserve(16);
    }

    RefAttributeArray(
        const GA_Detail& geo,
        const SOP_NodeVerb::CookParms& cookparms
    )
        : geo(&geo)
        , cookparms(&cookparms)
    {
        attribUPtrArray.reserve(16);
        attribArray.reserve(16);
    }
    
    ~RefAttributeArray()
    {
    }

    SYS_FORCE_INLINE const GA_Attribute* &operator[](const size_t i)
    { return attribArray[i]; }

    SYS_FORCE_INLINE bool isValid() const
    { return bool(geo); }

    SYS_FORCE_INLINE bool isEmpty() const
    { return attribArray.size() == 0; }

    SYS_FORCE_INLINE void erase(const size_t i)
    { attribArray.erase(attribArray.begin()+i); }

    SYS_FORCE_INLINE void clear()
    {
        attribArray.clear();
        attribUPtrArray.clear();
    }

    SYS_FORCE_INLINE std::vector<GA_Attribute*>::size_type size() const
    { return attribArray.size(); }

    SYS_FORCE_INLINE void reset(
        const GA_Detail* const geo,
        const SOP_NodeVerb::CookParms* const cookparms = nullptr
    )
    {
        this->geo = geo;
        this->cookparms = cookparms;
        clear();
    }

    SYS_FORCE_INLINE void set(const GA_Attribute* const attrib)
    {
        clear();
        if (attrib)
            attribArray.emplace_back(attrib);
    }

    SYS_FORCE_INLINE void set(const GA_Attribute& attrib)
    { clear(); attribArray.emplace_back(&attrib); }

    const GA_Attribute* set(const GA_AttributeOwner attribClass, const UT_StringRef& attribPattern)
    {
        if (gfe::Type::isInvalid(attribPattern))
            return nullptr;
        const GA_Attribute* attribPtr = geo->findAttribute(attribClass, attribPattern);
        set(attribPtr);
        return attribPtr;
    }

    SYS_FORCE_INLINE void append(const GA_Attribute* const attrib)
    { if (attrib) attribArray.emplace_back(attrib); }

    SYS_FORCE_INLINE void append(const GA_Attribute& attrib)
    { attribArray.emplace_back(&attrib); }

    const GA_Attribute* append(const GA_AttributeOwner attribClass, const UT_StringRef& attribPattern)
    {
        if (gfe::Type::isInvalid(attribPattern))
            return nullptr;
        const GA_Attribute* attribPtr = geo->findAttribute(attribClass, attribPattern);
        append(attribPtr);
        return attribPtr;
    }

    const GA_Attribute* appendUV(const UT_StringRef& attribPattern, const GA_AttributeOwner attribClass = GA_ATTRIB_INVALID)
    {
        if (gfe::Type::isInvalid(attribPattern))
            return nullptr;

        const GA_Attribute* const attrib = gfe::Attribute::findUVAttributePointVertex(*geo, attribClass, attribPattern);
        append(attrib);
        return attrib;
    }




    void appends(const GA_AttributeOwner attribClass, const char* const attribPattern)
    {
        UT_ASSERT_P(geo);
        if (!attribPattern)
            return;
            
        for (GA_AttributeDict::iterator it = geo->getAttributes().begin(attribClass); !it.atEnd(); ++it)
        {
            GA_Attribute* const attrib = it.attrib();
            if (attrib->getScope() != GA_SCOPE_PUBLIC)
                continue;
            if (!attrib->getName().multiMatch(attribPattern))
                continue;
            attribArray.emplace_back(attrib);
        }
    }

    SYS_FORCE_INLINE void appends(const GA_AttributeOwner attribClass, const UT_StringRef& attribPattern)
    {
        if (gfe::Type::isValid(attribPattern))
            appends(attribClass, attribPattern.c_str());
    }


    
    void oappends(const GA_AttributeOwner attribClass, const char* const attribPattern)
    {
        if (!attribPattern)
            return;

        for (GA_AttributeDict::ordered_iterator it = geo->getAttributes().getDict(attribClass).obegin(GA_SCOPE_PUBLIC); !it.atEnd(); ++it)
        {
            GA_Attribute* const attribPtr = *it;
            if (!attribPtr->getName().multiMatch(attribPattern))
                continue;
            attribArray.emplace_back(attribPtr);
        }
    }

    SYS_FORCE_INLINE void oappends(const GA_AttributeOwner attribClass, const UT_StringRef& attribPattern)
    {
        if (gfe::Type::isValid(attribPattern))
            oappends(attribClass, attribPattern.c_str());
    }
    
    SYS_FORCE_INLINE void oappendPrims(const char* const attribPattern)
    { return oappends(GA_ATTRIB_PRIMITIVE, attribPattern); }

    SYS_FORCE_INLINE void oappendPoints(const char* const attribPattern)
    { return oappends(GA_ATTRIB_POINT, attribPattern); }
    
    SYS_FORCE_INLINE void oappendVertices(const char* const attribPattern)
    { return oappends(GA_ATTRIB_VERTEX, attribPattern); }
    
    SYS_FORCE_INLINE void oappendDetails(const char* const attribPattern)
    { return oappends(GA_ATTRIB_DETAIL, attribPattern); }



    
    SYS_FORCE_INLINE void appendPointVertexs(const UT_StringRef& attribPattern)
    {
        appends(GA_ATTRIB_POINT, attribPattern);
        appends(GA_ATTRIB_VERTEX, attribPattern);
    }

    const GA_Attribute*
        findTuple(
            const GA_AttributeOwner owner = GA_ATTRIB_POINT,
            const GA_StorageClass storageClass = GA_STORECLASS_FLOAT,
            const GA_Storage storage = GA_STORE_INVALID,
            const UT_StringRef& attribName = "",
            const int tupleSize = 1,
            const GA_Defaults& defaults = GA_Defaults(0.0f),
            const bool emplaceBack = true,
            const GA_AttributeOptions* const attribute_options = nullptr
        )
    {
        const GA_Storage finalStorage = gfe::Type::getPreferredStorage(geo, storageClass, storage);

        const GA_Attribute* attribPtr = geo->findAttribute(owner, attribName);
        if (attribPtr)
        {
            const GA_AIFTuple* const aifTuple = attribPtr->getAIFTuple();
            if (attribPtr->getTupleSize() != tupleSize ||
                aifTuple->getStorage(attribPtr) != finalStorage ||
                aifTuple->getDefaults(attribPtr) != defaults)
            {
                attribPtr = nullptr;
            }
            else
            {
                if (emplaceBack)
                    attribArray.emplace_back(attribPtr);
                return attribPtr;
            }
        }

        if (!attribPtr)
        {
            if (cookparms)
                cookparms->sopAddError(SOP_ATTRIBUTE_INVALID, attribName);
            UT_ASSERT_MSG(attribPtr, "No Attrib");
            return nullptr;
        }
        if (emplaceBack)
            attribArray.emplace_back(attribPtr);
        return attribPtr;
    }

    //SYS_FORCE_INLINE
    //void
    //findOrCreate(
    //    const GA_AttributeOwner owner,
    //    const GA_Storage storage,
    //    const UT_StringRef& attribName
    //)
    //{
    //    findOrCreate(owner, storage, attribName);
    //}

    const GA_Attribute*
        findUV(
            const GA_AttributeOwner owner = GA_ATTRIB_POINT,
            const GA_Storage storage = GA_STORE_INVALID,
            const UT_StringRef& attribName = "",
            const int tupleSize = 3,
            const GA_Defaults& defaults = GA_Defaults(0.0f),
            const bool emplaceBack = true,
            const GA_StorageClass storageClass = GA_STORECLASS_FLOAT
        )
    {
        const GA_Storage finalStorage = gfe::Type::getPreferredStorage(geo, storageClass, storage);

        const GA_Attribute* attribPtr = gfe::Attribute::findAttributePointVertex(*geo, owner, attribName);
        if (attribPtr)
        {
            const GA_AIFTuple* const aifTuple = attribPtr->getAIFTuple();
            if (!aifTuple ||
                attribPtr->getTupleSize() != tupleSize ||
                aifTuple->getStorage(attribPtr) != finalStorage ||
                aifTuple->getDefaults(attribPtr) != defaults)
            {
                attribPtr = nullptr;
            }
            else
            {
                if (emplaceBack)
                    attribArray.emplace_back(attribPtr);
                return attribPtr;
            }
        }

        const GA_AttributeOwner validOwner = owner == GA_ATTRIB_POINT ? GA_ATTRIB_POINT : GA_ATTRIB_VERTEX;
        if (!attribPtr)
        {
            if (cookparms)
                cookparms->sopAddError(SOP_ATTRIBUTE_INVALID, attribName);
            UT_ASSERT_MSG(attribPtr, "No Attrib");
            return nullptr;
        }
        if (emplaceBack)
            attribArray.emplace_back(attribPtr);
        return attribPtr;
    }

    const GA_Attribute*
        findDir(
            const GA_AttributeOwner owner = GA_ATTRIB_POINT,
            const GA_Storage storage = GA_STORE_INVALID,
            const UT_StringRef& attribName = "",
            const int tupleSize = 3,
            const GA_Defaults& defaults = GA_Defaults(0.0f),
            const bool emplaceBack = true,
            const GA_StorageClass storageClass = GA_STORECLASS_FLOAT
        )
    {
        const GA_Storage finalStorage = Type::getPreferredStorage(geo, storageClass, storage);

        const GA_Attribute* attribPtr = geo->findAttribute(owner, attribName);
        if (attribPtr)
        {
            const GA_AIFTuple* const aifTuple = attribPtr->getAIFTuple();
            if (attribPtr->getTupleSize() != tupleSize ||
                aifTuple->getStorage(attribPtr) != finalStorage ||
                aifTuple->getDefaults(attribPtr) != defaults)
            {
                attribPtr = nullptr;
            }
            else
            {
                if (emplaceBack)
                    attribArray.emplace_back(attribPtr);
                return attribPtr;
            }
        }

        if (!attribPtr)
        {
            if (cookparms)
                cookparms->sopAddError(SOP_ATTRIBUTE_INVALID, attribName);
            UT_ASSERT_MSG(attribPtr, "No Attrib");
            return nullptr;
        }
        if (emplaceBack)
            attribArray.emplace_back(attribPtr);
        return attribPtr;
    }


    const GA_Attribute*
        findNormal3D(
            const gfe::NormalSearchOrder owner = gfe::NormalSearchOrder::All,
            const GA_Storage storage = GA_STORE_INVALID,
            const UT_StringRef& attribName = "",
            const int tupleSize = 3,
            const GA_Defaults& defaults = GA_Defaults(0.0f),
            const bool emplaceBack = true,
            const GA_StorageClass storageClass = GA_STORECLASS_FLOAT
        )
    {
        const GA_Storage finalStorage = gfe::Type::getPreferredStorage(geo, storageClass, storage);

        const GA_Attribute* attribPtr = gfe::Attribute::findNormal3D(*geo, owner, attribName);
        if (attribPtr)
        {
            const GA_AIFTuple* const aifTuple = attribPtr->getAIFTuple();
            if (attribPtr->getTupleSize() != tupleSize ||
                aifTuple->getStorage(attribPtr) != finalStorage ||
                aifTuple->getDefaults(attribPtr) != defaults)
            {
                attribPtr = nullptr;
            }
            else
            {
                if (emplaceBack)
                    attribArray.emplace_back(attribPtr);
                return attribPtr;
            }
        }

        const GA_AttributeOwner validOwner = gfe::Attribute::toValidOwner(owner);

        if (!attribPtr)
        {
            if (cookparms)
                cookparms->sopAddError(SOP_ATTRIBUTE_INVALID, attribName);
            UT_ASSERT_MSG(attribPtr, "No Attrib");
            return nullptr;
        }
        if (emplaceBack)
            attribArray.emplace_back(attribPtr);
        return attribPtr;
    }





    SYS_FORCE_INLINE void pop_back()
    { attribArray.pop_back(); }

    SYS_FORCE_INLINE const GA_Attribute* last() const
    { return attribArray[attribArray.size() - 1]; }


    SYS_FORCE_INLINE std::vector<const GA_Attribute*>& ref()
    { return attribArray; }


protected:
    SYS_FORCE_INLINE void setDetail(const GA_Detail* const inGeo)
    { geo = inGeo; }



protected:
    const GA_Detail* geo;
    const SOP_NodeVerb::CookParms* cookparms;

private:
    std::vector<GA_AttributeUPtr> attribUPtrArray;
    std::vector<const GA_Attribute*> attribArray;

    friend class gfe::GeoFilterRef0;
    friend class gfe::GeoFilterRef1;
    friend class gfe::GeoFilterRef2;
    friend class gfe::GeoFilterRef3;
    friend class gfe::GeoFilterRef4;

}; // End of class RefAttributeArray














class RefGroupArray {

public:
    RefGroupArray(
        const GA_Detail* const geo,
        const SOP_NodeVerb::CookParms* const cookparms = nullptr
    )
        : geo(geo)
        , cookparms(cookparms)
    {
        groupUPtrArray.reserve(16);
        groupArray.reserve(16);
    }

    RefGroupArray(
        const GA_Detail* const geo,
        const SOP_NodeVerb::CookParms& cookparms
    )
        : geo(geo)
        , cookparms(&cookparms)
    {
        groupUPtrArray.reserve(16);
        groupArray.reserve(16);
    }

    RefGroupArray(
        const GA_Detail& geo,
        const SOP_NodeVerb::CookParms* const cookparms = nullptr
    )
        : geo(&geo)
        , cookparms(cookparms)
    {
        groupUPtrArray.reserve(16);
        groupArray.reserve(16);
    }

    RefGroupArray(
        const GA_Detail& geo,
        const SOP_NodeVerb::CookParms& cookparms
    )
        : geo(&geo)
        , cookparms(&cookparms)
    {
        groupUPtrArray.reserve(16);
        groupArray.reserve(16);
    }

    ~RefGroupArray()
    {
    }
    

    
    
    SYS_FORCE_INLINE const GA_ElementGroup* getElementGroup(const size_t i) const
    {
        UT_ASSERT(groupArray[i]->isElementGroup());
        return static_cast<const GA_ElementGroup*>(groupArray[i]);
    }

    SYS_FORCE_INLINE const GA_PrimitiveGroup* getPrimitiveGroup(const size_t i) const
    {
        UT_ASSERT(groupArray[i]->classType() == GA_GROUP_PRIMITIVE);
        return static_cast<const GA_PrimitiveGroup*>(groupArray[i]);
    }

    SYS_FORCE_INLINE const GA_PointGroup* getPointGroup(const size_t i) const
    {
        UT_ASSERT(groupArray[i]->classType() == GA_GROUP_POINT);
        return static_cast<const GA_PointGroup*>(groupArray[i]);
    }

    SYS_FORCE_INLINE const GA_VertexGroup* getVertexGroup(const size_t i) const
    {
        UT_ASSERT(groupArray[i]->classType() == GA_GROUP_VERTEX);
        return static_cast<const GA_VertexGroup*>(groupArray[i]);
    }

    SYS_FORCE_INLINE const GA_EdgeGroup* getEdgeGroup(const size_t i) const
    {
        UT_ASSERT(groupArray[i]->classType() == GA_GROUP_EDGE);
        return static_cast<const GA_EdgeGroup*>(groupArray[i]);
    }

    SYS_FORCE_INLINE const GA_Group* &operator[](const size_t i)
    { return groupArray[i]; }

    SYS_FORCE_INLINE bool isEmpty() const
    { return groupArray.size() == 0; }

    SYS_FORCE_INLINE void erase(const size_t i)
    { groupArray.erase(groupArray.begin()+i); }
    
    SYS_FORCE_INLINE void clear()
    {
        groupArray.clear();
        groupUPtrArray.clear();
    }

    SYS_FORCE_INLINE std::vector<GA_Group*>::size_type size() const
    { return groupArray.size(); }

    SYS_FORCE_INLINE const GA_Group* last()
    { return groupArray[groupArray.size() - 1]; }

    SYS_FORCE_INLINE void reset(
        const GA_Detail* const geo,
        const SOP_NodeVerb::CookParms* const cookparms = nullptr
    )
    {
        this->geo = geo;
        this->cookparms = cookparms;
        clear();
    }

    SYS_FORCE_INLINE void set(GA_Group* const groupPtr)
    {
        clear();
        if (groupPtr)
            groupArray.emplace_back(groupPtr);
    }

    SYS_FORCE_INLINE void set(GA_Group& groupPtr)
    {
        clear();
        groupArray.emplace_back(&groupPtr);
    }

    void set(const GA_GroupType groupClass, const UT_StringRef& groupPattern)
    {
        if (gfe::Type::isValid(groupPattern))
        {
            GA_Group* groupPtr = geo->getGroupTable(groupClass)->find(groupPattern);
            set(groupPtr);
        }
    }

    SYS_FORCE_INLINE void append(GA_Group* const groupPtr)
    { if (groupPtr) groupArray.emplace_back(groupPtr); }

    SYS_FORCE_INLINE void append(GA_Group& groupPtr)
    { groupArray.emplace_back(&groupPtr); }

    void append(const GA_GroupType groupClass, const UT_StringRef& groupPattern)
    {
        if (gfe::Type::isValid(groupPattern))
        {
            GA_Group* groupPtr = geo->getGroupTable(groupClass)->find(groupPattern);
            append(groupPtr);
        }
    }

    void appends(const GA_GroupType groupClass, const char* groupPattern)
    {
        if (!groupPattern)
            return;

        for (GA_GroupTable::iterator<GA_Group> it = geo->getGroupTable(groupClass)->beginTraverse(); !it.atEnd(); ++it)
        {
            GA_Group* const groupPtr = it.group();
            if (!groupPtr->getName().multiMatch(groupPattern))
                continue;
            groupArray.emplace_back(groupPtr);
        }
    }

    SYS_FORCE_INLINE void appends(const GA_GroupType groupClass, const UT_StringRef& groupPattern)
    {
        if (gfe::Type::isValid(groupPattern))
            appends(groupClass, groupPattern.c_str());
    }

    SYS_FORCE_INLINE void appends(const GA_AttributeOwner groupClass, const char* groupPattern)
    { appends(gfe::Type::attributeOwner_groupType(groupClass), groupPattern); }

    SYS_FORCE_INLINE void appends(const GA_AttributeOwner groupClass, const UT_StringRef& groupPattern)
    { appends(gfe::Type::attributeOwner_groupType(groupClass), groupPattern); }
    

    SYS_FORCE_INLINE void set(const GA_AttributeOwner groupClass, const UT_StringRef& groupPattern)
    { set(gfe::Type::attributeOwner_groupType(groupClass), groupPattern); }

    SYS_FORCE_INLINE void append(const GA_AttributeOwner groupClass, const UT_StringRef& groupPattern)
    { append(gfe::Type::attributeOwner_groupType(groupClass), groupPattern); }



    GA_Group* find(const GA_GroupType groupType = GA_GROUP_POINT, const UT_StringRef& groupName = "")
    {
        GA_Group* groupPtr = geo->getGroupTable(groupType)->find(groupName);

        if (!groupPtr)
        {
            if (cookparms)
                cookparms->sopAddError(SOP_ERR_BADGROUP, groupName);
            UT_ASSERT_MSG(groupPtr, "No Group");
            return nullptr;
        }
        groupArray.emplace_back(groupPtr);
        return groupPtr;
    }



    SYS_FORCE_INLINE std::vector<const GA_Group*>& ref()
    { return groupArray; }


protected:
    SYS_FORCE_INLINE void setDetail(const GA_Detail* const inGeo)
    { geo = inGeo; }


protected:
    const GA_Detail* geo;
    const SOP_NodeVerb::CookParms* cookparms;

private:
    std::vector<GA_GroupUPtr> groupUPtrArray;
    std::vector<const GA_Group*> groupArray;

    friend class gfe::GeoFilterRef0;
    friend class gfe::GeoFilterRef1;
    friend class gfe::GeoFilterRef2;
    friend class gfe::GeoFilterRef3;
    friend class gfe::GeoFilterRef4;

}; // End of class RefGroupArray
_GFE_END
#endif
