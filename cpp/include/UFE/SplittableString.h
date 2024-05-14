
#pragma once

#ifndef __UFE_SplittableString_h__
#define __UFE_SplittableString_h__

#include <UFE/SplittableString.h>

#include <UFE/Core.h>

class UT_StringHolder;

_UFE_BEGIN
class SplittableString {

public:
    SplittableString()
        : sep(" ")
    {
    }
    
    SplittableString(const char* const fullStr, const char* const sep = " ")
        : fullStr(fullStr)
        , sep(sep)
    {
    }
    
    SplittableString(const ::std::string&& fullStr, const char* const sep = " ")
        : fullStr(::std::move(fullStr))
        , sep(sep)
    {
    }
    
    SplittableString(const ::std::string& fullStr, const char* const sep = " ")
        : fullStr(fullStr)
        , sep(sep)
    {
    }
    
    ~SplittableString()
    {
    }

    SYS_FORCE_INLINE SplittableString &operator=(const GA_Group& attribPtr)
    {
        set(attribPtr.isDetached() ? "" : attribPtr.getName().c_str());
        return *this;
    }
    SYS_FORCE_INLINE SplittableString &operator=(const GA_Group* const attribPtr)
    {
        UT_ASSERT_P(attribPtr);
        set(attribPtr->isDetached() ? "" : attribPtr->getName().c_str());
        return *this;
    }
    SYS_FORCE_INLINE SplittableString &operator=(const GA_Attribute& attribPtr)
    {
        set(attribPtr.isDetached() ? "" : attribPtr.getName().c_str());
        return *this;
    }
    SYS_FORCE_INLINE SplittableString &operator=(const GA_Attribute* const attribPtr)
    {
        UT_ASSERT_P(attribPtr);
        set(attribPtr->isDetached() ? "" : attribPtr->getName().c_str());
        return *this;
    }
    SYS_FORCE_INLINE SplittableString &operator=(const UT_StringRef& s)
    {
        set(s.c_str());
        return *this;
    }
    SYS_FORCE_INLINE SplittableString &operator=(const char* const s)
    {
        set(s);
        return *this;
    }
    SYS_FORCE_INLINE SplittableString &operator=(SplittableString &s)
    {
        set(s.fullStr);
        return *this;
    }
    SYS_FORCE_INLINE SplittableString &operator=(SplittableString &&s)
    {
        set(::std::move(s).fullStr);
        return *this;
    }

    SYS_FORCE_INLINE void reset()
    {
        startOffset = 0;
        endOffset = 0;
        
        // hasCheckIsSpace = false;
        // isSpace = false;
        // hasCheckIsSep = false;
        // isSep = false;
    }
    
    // bool isspace()
    // {
    //     if (hasCheckIsSpace)
    //         return isSpace;
    //     
    //     hasCheckIsSpace = true;
    //     
    //     const ::std::string::size_type strlength = fullStr.length();
    //     for (::std::string::size_type i = 0; i < strlength; ++i)
    //     {
    //         if ( !::std::isspace(fullStr[i]) )
    //             return false;
    //     }
    //     
    //     isSpace = true;
    //     return true;
    // }
    //
    // bool issep()
    // {
    //     if (hasCheckIsSep)
    //         return isSep;
    //     
    //     hasCheckIsSep = true;
    //     
    //     const ::std::string::size_type strlength = fullStr.length();
    //     for (::std::string::size_type i = 0; i < strlength; ++i)
    //     {
    //         if ( fullStr[i] != *sep )
    //             return false;
    //     }
    //     
    //     isSep = true;
    //     return true;
    // }
    
    // SYS_FORCE_INLINE bool getIsInvalid()
    // {
    //     if(strlen(sep)==1 && ::std::isspace(*sep))
    //         return isspace();
    //     else
    //         return issep();
    // }
    //
    // SYS_FORCE_INLINE bool getIsValid()
    // {
    //     return !isInvalid();
    // }
    
    SYS_FORCE_INLINE UT_StringHolder getValidAttribName(const GA_Attribute& attrib)
    {
        return getIsValid()
               ? getNext<UT_StringHolder>()
               : (attrib.isDetached() ? UT_StringHolder("") : attrib.getName());
    }
    
    SYS_FORCE_INLINE UT_StringHolder getValidAttribName(const GA_Group& attrib)
    {
        return getIsValid()
               ? getNext<UT_StringHolder>()
               : (attrib.isDetached() ? UT_StringHolder("") : attrib.getName());
    }
    
    SYS_FORCE_INLINE UT_StringHolder getValidAttribName(const GA_ElementGroup& attrib)
    { return getValidAttribName(reinterpret_cast<const GA_Group&>(attrib)); }
    
    SYS_FORCE_INLINE UT_StringHolder getValidAttribName(const GA_PrimitiveGroup& attrib)
    { return getValidAttribName(reinterpret_cast<const GA_Group&>(attrib)); }
    
    SYS_FORCE_INLINE UT_StringHolder getValidAttribName(const GA_PointGroup& attrib)
    { return getValidAttribName(reinterpret_cast<const GA_Group&>(attrib)); }
    
    SYS_FORCE_INLINE UT_StringHolder getValidAttribName(const GA_VertexGroup& attrib)
    { return getValidAttribName(reinterpret_cast<const GA_Group&>(attrib)); }
    
    SYS_FORCE_INLINE UT_StringHolder getValidAttribName(const GA_EdgeGroup& attrib)
    { return getValidAttribName(reinterpret_cast<const GA_Group&>(attrib)); }
    
    SYS_FORCE_INLINE bool getIsValid()
    {
        computeIsValidIfNeeded();
        return isValid;
    }
    SYS_FORCE_INLINE bool getIsInvalid()
    { return !getIsValid(); }

    SYS_FORCE_INLINE bool forceGetIsValid()
    { return isValid; }

    template<typename T>
    SYS_FORCE_INLINE T getNext()
    { return (T)getNextCore(); }
    
    template<>
    SYS_FORCE_INLINE UT_StringHolder getNext<UT_StringHolder>()
    { return UT_StringHolder(getNextCore().c_str()); }

    template<>
    SYS_FORCE_INLINE const char* getNext<const char*>()
    { return getNextCore().c_str(); }

    template<>
    SYS_FORCE_INLINE UT_StringRef getNext<UT_StringRef>()
    { return UT_StringRef(getNextCore().c_str()); }

    template<>
    SYS_FORCE_INLINE ::std::string getNext<::std::string>()
    { return getNextCore(); }
    
    
    
private:
    
    void computeIsValid()
    {
        shouldComputeIsValid = false;
        getNextCore();
        reset();
    }
    
    SYS_FORCE_INLINE void computeIsValidIfNeeded()
    { if (shouldComputeIsValid) computeIsValid(); }

    
    void set(const char* const s)
    {
        if (!s)
            return;
        reset();
        fullStr = s;
        shouldComputeIsValid = true;
    }
    SYS_FORCE_INLINE void set(const ::std::string& s)
    {
        reset();
        fullStr = s;
    }
    
    ::std::string getNextCore()
    {
        if (endOffset == ::std::string::npos)
        {
            startOffset = 0;
            endOffset = 0;
            if(!isValid)
                return ::std::string();
            return getNextCore();
        }
        
        endOffset = fullStr.find(sep, startOffset);
        if (endOffset == ::std::string::npos)
        {
            return getNextCore_valid();
        }
        else
        {
            if (startOffset == endOffset)
            {
                ++startOffset;
                if(startOffset == fullStr.length())
                {
                    startOffset = 0;
                    endOffset = 0;
                    if(!isValid)
                        return ::std::string();
                }
                return getNextCore();
            }
            else
            {
                return getNextCore_valid();
            }
        }
    }

    ::std::string getNextCore_valid()
    {
        char sub = fullStr[startOffset];
        
        bool valid = sub=='_' || (sub>='a' && sub<='z') || (sub>='A' && sub<='Z');
            
        if ( valid )
        {
            for (::std::string::size_type i = startOffset + 1; i < endOffset; ++i)
            {
                sub = fullStr[i];
                if ( sub=='\0' )
                    break;
                if ( sub=='_' || (sub>='a' && sub<='z') || (sub>='A' && sub<='Z') || (sub>='0' && sub<='9') )
                    continue;
                
                valid = false;
                break;
            }
        }
        if (valid)
        {
            isValid = true;
            return fullStr.substr(startOffset, endOffset-startOffset);
        }
        else
        {
            startOffset = endOffset + 1;
            if(startOffset == fullStr.length())
            {
                startOffset = 0;
                endOffset = 0;
                if(!isValid)
                    return ::std::string();
            }
            return getNextCore();
        }
    }
    
public:
    ::std::string fullStr;
    const char* sep;
    
private:
    ::std::string::size_type startOffset = 0;
    ::std::string::size_type endOffset = 0;

    bool isValid = false;
    bool shouldComputeIsValid = true;
    
}; // End of class SplittableString
_UFE_END





#endif
