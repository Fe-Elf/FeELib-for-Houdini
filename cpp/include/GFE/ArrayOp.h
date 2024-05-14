
#pragma once

#ifndef __GFE_ArrayOp_h__
#define __GFE_ArrayOp_h__

#include <GFE/ArrayOp.h>

#include <GA/GA_Detail.h>

_GFE_BEGIN
namespace ArrayOp {

template<typename _ArrayT, typename _ArrayValueType>
static GA_Size appendSorted(_ArrayT& arr, const _ArrayValueType val)
{
    const GA_Size size = arr.size();
    if (size == 0)
        return arr.emplace_back(val);
    if (arr[size-1] <= val)
        return arr.emplace_back(val);
    if (size == 1)
        return arr.insert(val, 0);
        
    GA_Size l = 0;
    GA_Size r = size;
    GA_Size m = r >> 1;
    while (l < r)
    {
        if (arr[m] > val)
        {
            r = m;
        }
        else if (arr[m] < val)
        {
            l = m+1;
        }
        else
        {
            return arr.insert(val, m+1);
            //break;
        }
        m = (l+r) >> 1;
    }
    UT_ASSERT_P(l <= r || l == r+1);
    return l > r ? arr.insert(val, l) : arr.insert(val, m);
}
    

template<>
static GA_Size appendSorted(UT_ValArray<GA_Offset>& arr, const GA_Offset val)
{
#if GFE_DEBUG_MODE
    const GA_Offset* const data = arr.data();
    ::std::vector<GA_Offset> array(data, data + arr.size());
#endif
    const GA_Size size = arr.size();
    if (size == 0)
        return arr.emplace_back(val);
    if (arr[size-1] <= val)
        return arr.emplace_back(val);
    if (size == 1)
        return arr.insert(val, 0);
        
    GA_Size l = 0;
    GA_Size r = size;
    GA_Size m = r >> 1;
    while (l < r)
    {
        if (arr[m] > val)
        {
            r = m;
        }
        else if (arr[m] < val)
        {
            l = m+1;
        }
        else
        {
            return arr.insert(val, m+1);
            //break;
        }
        m = (l+r) >> 1;
    }
    UT_ASSERT_P(l <= r || l == r+1);
    return l > r ? arr.insert(val, l) : arr.insert(val, m);
}
    
template<>
static GA_Size appendSorted(GA_OffsetList& arr, const GA_Offset val)
{
    const GA_Size size = arr.size();
    if (size == 0)
        return arr.append(val);
    if (arr[size-1] <= val)
        return arr.append(val);
    if (size == 1)
        return arr.insert(0, val);
    
    GA_Size l = 0;
    GA_Size r = size;
    GA_Size m = r >> 1;
    while (l < r)
    {
        if (arr[m] > val)
        {
            r = m;
        }
        else if (arr[m] < val)
        {
            l = m+1;
        }
        else
        {
            return arr.insert(val, m+1);
            //break;
        }
        m = (l+r) >> 1;
    }
    UT_ASSERT_P(l <= r || l == r+1);
    return l > r ? arr.insert(l, val) : arr.insert(m, val);
}


template<typename _ArrayT, typename _ArrayValueType>
SYS_FORCE_INLINE static void uniqueAppendSorted(_ArrayT& arr, const _ArrayValueType val)
{
    if (arr.uniqueSortedFind(val) == gfe::FIND_INVALID_INDEX)
        ArrayOp::appendSorted(arr, val);
}

template<>
SYS_FORCE_INLINE static void uniqueAppendSorted(UT_ValArray<GA_Offset>& arr, const GA_Offset val)
{
    if (arr.uniqueSortedFind(val) == gfe::FIND_INVALID_INDEX)
        ArrayOp::appendSorted(arr, val);
}

template<>
SYS_FORCE_INLINE static void uniqueAppendSorted(GA_OffsetList& arr, const GA_Offset val)
{
    if (arr.findSorted(val) == gfe::FIND_INVALID_INDEX)
        ArrayOp::appendSorted(arr, val);
}

    

template<typename T>
std::vector<int> argsort(const std::vector<T>& inArray)
{
    std::vector<int> indexs(inArray.size());
    for (int i = 0; i < inArray.size(); ++i)
        indexs[i] = i;

    std::sort(indexs.begin(), indexs.end(),
        [&inArray](int pos1, int pos2) {return (inArray[pos1] < inArray[pos2]); });

    return indexs;
}



template<typename T>
UT_Array<int> argsort_UT_Array(const UT_Array<T>& inArray)
{
    UT_Array<int> indexs(inArray.size());
    for (int i = 0; i < inArray.size(); ++i)
        indexs[i] = i;

    std::sort(indexs.begin(), indexs.end(),
        [&inArray](int pos1, int pos2) {return (inArray[pos1] < inArray[pos2]); });

    return indexs;
}

//UT_Array<int> argsort_UT_Array(const UT_Array<fpreal>& inArray)
//{
//    UT_Array<int> indexs(inArray.size());
//    for (int i = 0; i < inArray.size(); ++i)
//        indexs[i] = i;
//
//    std::sort(indexs.begin(), indexs.end(),
//        [&inArray](int pos1, int pos2) {return (inArray[pos1] < inArray[pos2]); });
//
//    return indexs;
//}


} // End of namespace ArrayOp
_GFE_END
#endif
