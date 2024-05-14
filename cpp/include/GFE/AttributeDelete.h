
#pragma once

#ifndef __GFE_AttributeDelete_h__
#define __GFE_AttributeDelete_h__

#include <GFE/AttributeDelete.h>

#include <GA/GA_Detail.h>
#include <GA/GA_AttributeFilter.h>

_GFE_BEGIN
namespace AttributeDelete {



    static void keepStdAttribute(
        GA_AttributeSet& attribSet,
        const char* primAttribPattern,
        const char* pointAttribPattern,
        const char* vertexAttribPattern,
        const char* detailAttribPattern
    )
    {
        GA_AttributeFilter attribFilter;
        const GA_AttributeFilter attribFilterPublic = GA_AttributeFilter::selectPublic();
        const GA_AttributeFilter attribFilterStd = GA_AttributeFilter::selectAnd(attribFilterPublic, GA_AttributeFilter::selectStandard());

        attribFilter = GA_AttributeFilter::selectByPattern(primAttribPattern);
        attribFilter = GA_AttributeFilter::selectNot(attribFilter);
        attribFilter = GA_AttributeFilter::selectAnd(attribFilter, attribFilterStd);
        attribSet.destroyAttributes(GA_ATTRIB_PRIMITIVE, attribFilter);

        attribFilter = GA_AttributeFilter::selectByPattern(pointAttribPattern);
        attribFilter = GA_AttributeFilter::selectNot(attribFilter);
        attribFilter = GA_AttributeFilter::selectAnd(attribFilter, attribFilterStd);
        attribSet.destroyAttributes(GA_ATTRIB_POINT, attribFilter);

        attribFilter = GA_AttributeFilter::selectByPattern(vertexAttribPattern);
        attribFilter = GA_AttributeFilter::selectNot(attribFilter);
        attribFilter = GA_AttributeFilter::selectAnd(attribFilter, attribFilterStd);
        attribSet.destroyAttributes(GA_ATTRIB_VERTEX, attribFilter);

        attribFilter = GA_AttributeFilter::selectByPattern(detailAttribPattern);
        attribFilter = GA_AttributeFilter::selectNot(attribFilter);
        attribFilter = GA_AttributeFilter::selectAnd(attribFilter, attribFilterStd);
        attribSet.destroyAttributes(GA_ATTRIB_DETAIL, attribFilter);
    }

    static void
        keepAttribute(
            GA_AttributeSet& attribSet,
            const GA_AttributeOwner attribClass,
            const GA_AttributeFilter& attribFilter
        )
    {
        for (GA_AttributeDict::iterator it = attribSet.begin(attribClass); !it.atEnd(); ++it)
        {
            GA_Attribute* attribPtr = it.attrib();
            UT_StringHolder name = attribPtr->getName();
            //if (attribPtr->isDetached())
            //    continue;
            if (attribFilter.match(attribPtr))
                continue;
            //attribPtr->bumpDataId();
            attribSet.destroyAttribute(attribPtr);
        }
    }

    static void
        keepStdAttribute(
            GA_AttributeSet& attribSet,
            const GA_AttributeOwner attribClass,
            const GA_AttributeFilter& attribFilter
        )
    {
        GA_AttributeFilter attribFilterPublic = GA_AttributeFilter::selectAnd(attribFilter, GA_AttributeFilter::selectPublic());
        attribFilterPublic = GA_AttributeFilter::selectOr(attribFilterPublic, GA_AttributeFilter::selectNot(GA_AttributeFilter::selectStandard()));
        //attribFilterPublic = GA_AttributeFilter::selectOr(attribFilterPublic, GA_AttributeFilter::selectNot(GA_AttributeFilter::selectFactory()));
        keepAttribute(attribSet, attribClass, attribFilterPublic);
    }

    SYS_FORCE_INLINE static void
    keepStdAttribute(
        GA_AttributeSet& attribSet,
        const GA_AttributeOwner attribClass,
        const char* pattern
    )
    { keepStdAttribute(attribSet, attribClass, GA_AttributeFilter::selectByPattern(pattern)); }

    //SYS_FORCE_INLINE static void keepStdAttribute(
    //    GA_Detail* const geo,
    //    const UT_StringHolder& primAttribPattern,
    //    const UT_StringHolder& pointAttribPattern,
    //    const UT_StringHolder& vertexAttribPattern,
    //    const UT_StringHolder& detailAttribPattern
    //)
    //{
    //    return keepStdAttribute(geo->getAttributes(), primAttribPattern, pointAttribPattern, vertexAttribPattern, detailAttribPattern);
    //}


    static void delStdAttribute(
        GA_AttributeSet& attribSet,
        const char* primAttribPattern,
        const char* pointAttribPattern,
        const char* vertexAttribPattern,
        const char* detailAttribPattern
    )
    {
        GA_AttributeFilter attribFilter;
        const GA_AttributeFilter attribFilterPublic = GA_AttributeFilter::selectPublic();
        const GA_AttributeFilter attribFilterStd = GA_AttributeFilter::selectAnd(attribFilterPublic, GA_AttributeFilter::selectStandard());

        attribFilter = GA_AttributeFilter::selectByPattern(primAttribPattern);
        //attribFilter = GA_AttributeFilter::selectNot(attribFilter);
        attribFilter = GA_AttributeFilter::selectAnd(attribFilter, attribFilterStd);
        attribSet.destroyAttributes(GA_ATTRIB_PRIMITIVE, attribFilter);

        attribFilter = GA_AttributeFilter::selectByPattern(pointAttribPattern);
        //attribFilter = GA_AttributeFilter::selectNot(attribFilter);
        attribFilter = GA_AttributeFilter::selectAnd(attribFilter, attribFilterStd);
        attribSet.destroyAttributes(GA_ATTRIB_POINT, attribFilter);

        attribFilter = GA_AttributeFilter::selectByPattern(vertexAttribPattern);
        //attribFilter = GA_AttributeFilter::selectNot(attribFilter);
        attribFilter = GA_AttributeFilter::selectAnd(attribFilter, attribFilterStd);
        attribSet.destroyAttributes(GA_ATTRIB_VERTEX, attribFilter);

        attribFilter = GA_AttributeFilter::selectByPattern(detailAttribPattern);
        //attribFilter = GA_AttributeFilter::selectNot(attribFilter);
        attribFilter = GA_AttributeFilter::selectAnd(attribFilter, attribFilterStd);
        attribSet.destroyAttributes(GA_ATTRIB_DETAIL, attribFilter);
    }




    //SYS_FORCE_INLINE static void delStdAttribute(
    //    GA_Detail* const geo,
    //    const UT_StringHolder& primAttribPattern,
    //    const UT_StringHolder& pointAttribPattern,
    //    const UT_StringHolder& vertexAttribPattern,
    //    const UT_StringHolder& detailAttribPattern
    //)
    //{
    //    return delStdAttribute(geo->getAttributes(), primAttribPattern, pointAttribPattern, vertexAttribPattern, detailAttribPattern);
    //}



    //SYS_FORCE_INLINE static bool deleteAttribute(GA_Attribute* const attrib)
    //{
    //    UT_ASSERT_P(attrib);
    //    return attrib->getDetail().getAttributes().destroyAttribute(attrib);
    //}



    //SYS_FORCE_INLINE static bool attributeDelete(GA_Attribute* const attrib)
    //{
    //    UT_ASSERT_P(attrib);
    //    return attrib->getDetail().getAttributes().destroyAttribute(attrib);
    //}

    //SYS_FORCE_INLINE static bool attribDelete(GA_Attribute* const attrib)
    //{
    //    UT_ASSERT_P(attrib);
    //    return attrib->getDetail().getAttributes().destroyAttribute(attrib);
    //}

    //SYS_FORCE_INLINE static bool destroyAttribute(GA_Attribute* const attrib)
    //{
    //    UT_ASSERT_P(attrib);
    //    return attrib->getDetail().getAttributes().destroyAttribute(attrib);
    //}

    


} // End of namespace AttributeDelete
_GFE_END
#endif
