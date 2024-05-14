
#pragma once

#ifndef __GFE_PackedArrayOfArrays_H__
#define __GFE_PackedArrayOfArrays_H__

#include "GFE/GFE_PackedArrayOfArrays.h"


#include "UT/UT_IteratorRange.h"


template <typename T>
class GFE_PackedArrayOfArrays {
	
public:
    typedef ::std::vector<T> value_type;

    GFE_PackedArrayOfArrays()
    {
    }

    SYS_FORCE_INLINE void clear()
    { myOffsets.clear(); myData.clear(); }
	
	SYS_FORCE_INLINE exint size() const
    { return myOffsets.size() ? myOffsets.size() : 0; }

    SYS_FORCE_INLINE bool isEmpty() const
    { return size()==0; }

    // SYS_FORCE_INLINE int64 getMemoryUsage(bool inclusive = false) const
    // {
    //     return (inclusive ? sizeof(*this) : 0) +
    //            myOffsets.getMemoryUsage(false) +
    //            myData.getMemoryUsage(false);
    // }
	
	SYS_FORCE_INLINE void emplace0()
    {
    	if (myOffsets.size()==0)
    		myOffsets.emplace_back(0);
    }

	void emplace_back_unsafe(const T* const data, const exint len)
    {
    	myData.reserve(myData.size() + len);
    	for (exint i = 0; i < len; i++)
    		myData.emplace_back(data[i]);
    	myOffsets.emplace_back(myData.size());
    }
	
	void emplace_back(const T* const data, const exint len)
    {
    	emplace0();
    	emplace_back_unsafe();
    }
	
    SYS_FORCE_INLINE void emplace_back(const ::std::vector<T>& arr)
	{ emplace_back(arr.data(), arr.size()); }
	
	SYS_FORCE_INLINE void emplace_back_unsafe(const GFE_PackedArrayOfArrays<T>& arr, const exint idx)
    { emplace_back_unsafe(arr.arrayData(idx), arr.arraySize(idx)); }
	
    SYS_FORCE_INLINE void emplace_back(const GFE_PackedArrayOfArrays<T>& arr, const exint idx)
    { emplace_back(arr.arrayData(idx), arr.arraySize(idx)); }
	
    SYS_FORCE_INLINE void emplace_back(const GFE_PackedArrayOfArrays<T>& arr)
	{
    	emplace0();
	    for (exint i = 0; i < arr.size(); i++)
			emplace_back_unsafe(arr, i);
	}

    /// Appends a new array and returns a pointer to the resulting
    /// data so it can be filled in.
    T *appendArray(const exint len)
	{
    	emplace0();
	    const exint size = myOffsets.size();
	    const exint newSize = size+len;
	    myOffsets.emplace_back(newSize);
	    myData.reserve(newSize);
	    return myData.data() + size;
	}

    /// From the given idxth array, return the idx_in_array element
    const T	&operator()(const exint idx, const exint idx_in_array) const
	{
	    UT_ASSERT_P(idx >= 0 && idx < size());
	    const exint base = myOffsets[idx];
	    UT_ASSERT_P(base+idx_in_array < myOffsets[idx+1]);
	    return myData[base+idx_in_array];
	}
	
    /// Extracts an array into a ::std::vector.
    void extract(::std::vector<T>& result, const exint idx) const
	{
	    UT_ASSERT_P(idx >= 0 && idx < size());
	    const exint base = myOffsets[idx];
	    const exint len  = myOffsets[idx+1]-base;
	    result.reserve(len);
	    for (exint i = 0; i < len; i++)
			result[i] = myData[base+i];
	}

    T* arrayData(const exint idx)
	{
	    // This is less than equal to as it is valid to
	    // point to the final element if the corresponding
	    // array is empty!
	    UT_ASSERT_P(myOffsets[idx] <= myData.size());
	    return myData.data() + myOffsets[idx];
	}
	
    SYS_FORCE_INLINE const T* arrayData(const exint idx) const
	{
	    UT_ASSERT_P(myOffsets[idx] <= myData.size());
	    return myData.data() + myOffsets[idx];
	}
	
    SYS_FORCE_INLINE exint arraySize(const exint idx) const
	{ return myOffsets[idx+1]-myOffsets[idx]; }

    /// @{
    /// Returns an iterator range for the specified array.
    UT_IteratorRange<T*> arrayRange(const exint idx)
    { return UTmakeRange(arrayData(idx), arrayData(idx + 1)); }
	
    UT_IteratorRange<const T*> arrayRange(const exint idx) const
    { return UTmakeRange(arrayData(idx), arrayData(idx + 1)); }
    /// @}

    /// Decreases, but never expands, to the given maxsize.
    void truncate(const exint maxsize)
    {
        if (maxsize >= 0 && size() > maxsize)
		{
		    if (maxsize == 0)
				clear();
		    else
		    {
				myOffsets.resize(maxsize+1);
				myData.resize(myOffsets[maxsize]);
		    }
		}
    }

	
    SYS_FORCE_INLINE void reserveData(const exint capacity)
	{ myData.reserve(capacity); }
	
    SYS_FORCE_INLINE void reserveOffsets(const exint capacity)
	{ myOffsets.reserve(capacity); }

	
	SYS_FORCE_INLINE ::std::vector<exint> &offsets()
    { return myOffsets; }
	
	SYS_FORCE_INLINE const ::std::vector<exint> &offsets() const
    { return myOffsets; }

	SYS_FORCE_INLINE ::std::vector<T> &data()
    { return myData; }
	
	SYS_FORCE_INLINE const ::std::vector<T> &data() const
    { return myData; }
    
protected:
    ::std::vector<exint> myOffsets;
    ::std::vector<T> myData;
	
}; // End of Class GFE_PackedArrayOfArrays










class GFE_PackedOffsetArrayOfArrays : public GFE_PackedArrayOfArrays<GA_Offset> {
	
public:
	typedef GFE_PackedArrayOfArrays<GA_Offset> Base;
	typedef Base::value_type value_type;

	GFE_PackedOffsetArrayOfArrays()
	{
	}
	

void emplace_back_trivial_unsafe(const GA_Offset trivialStart, const GA_Offset size)
{
	const exint newSize = myData.size()+size;
	myOffsets.emplace_back(newSize);
	myData.reserve(newSize);
	const GA_Offset end = trivialStart + size;
	for (GA_Offset i = trivialStart; i < end; ++i)
	{
		myData.emplace_back(i);
	}
}
	
SYS_FORCE_INLINE void emplace_back_trivial(const GA_Offset trivialStart, const GA_Offset size)
{
	emplace0();
	emplace_back_trivial_unsafe(trivialStart, size);
}

SYS_FORCE_INLINE void emplace_back_offsetList_unsafe(const GA_OffsetListRef& vertices)
{
	if (vertices.isTrivial())
		emplace_back_trivial_unsafe(vertices.trivialStart(), vertices.size());
	else
		emplace_back_unsafe(vertices.getArray(), vertices.size());
}

SYS_FORCE_INLINE void emplace_back_offsetList(const GA_OffsetListRef& vertices)
{
	emplace0();
	emplace_back_offsetList_unsafe(vertices);
}

}; // End of Class GFE_PackedArrayOfArrays







#endif
