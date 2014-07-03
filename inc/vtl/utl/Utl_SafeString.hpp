#ifndef __UTL_SAFESTRING_HPP__
#define __UTL_SAFESTRING_HPP__

/*************************************************************************\
Oliver Liyin Copyright (c) 2003

Module Name:
     Safe string helper

Abstract:

Notes:

Usage:

History:
    Created  on 2005 April 19 by oliver_liyin

\*************************************************************************/

#include <stdlib.h>
#include <assert.h>
#include <wtypes.h>
#include <tchar.h>

#include "xtl/Xtl_Platform.hpp"

namespace utl
{
    class SafeString
    {
    public:
        typedef TCHAR CharType;

#ifdef _UNICODE
        typedef CHAR ExCharType;
#else
        typedef WCHAR ExCharType;
#endif /// _UNICODE

    public:

        SafeString()
            : m_maxLen(0), m_buffer(NULL), m_bufferEx(NULL)
        {
            Resize(m_defaultMaxLen);
        }

        ~SafeString()
        {
            Release();
        }

        SafeString(const SafeString& strSource)
            : m_maxLen(0), m_buffer(NULL), m_bufferEx(NULL)
        {
            if(strSource.m_buffer != NULL)
            {
                assert(strSource.m_maxLen > 0);

                Resize(strSource.m_maxLen);
                if (m_buffer != NULL)
                {
                    assert(m_maxLen == strSource.m_maxLen);
                    memcpy(m_buffer, strSource.m_buffer, m_maxLen);
                }
            }
        }

        explicit SafeString(size_t size)
            : m_maxLen(0), m_buffer(NULL), m_bufferEx(NULL)
        {
            Resize(size);
        }

        SafeString(const TCHAR* strSource)
            : m_maxLen(0), m_buffer(NULL), m_bufferEx(NULL)
        {
            Resize(m_defaultMaxLen);
            if (m_buffer != NULL)
            {
                Copy(strSource);
            }
        }

        explicit SafeString(const TCHAR* format, ...)
            : m_maxLen(0), m_buffer(NULL), m_bufferEx(NULL)
        {
            Resize(m_defaultMaxLen);
            
            if (m_buffer != NULL)
            {
                va_list args;
                va_start(args, format);
                FormatV(format, args);
                va_end(args);
            }
        }

        const TCHAR* operator = (const TCHAR* strSource)
        {
            if (strSource != NULL)
            {
                Resize(__max(m_defaultMaxLen, _tcslen(strSource)));
                Copy(strSource);
            }
            else
            {
                Clear();
            }
            return m_buffer;
        }

        const TCHAR* operator = (const SafeString& strSource)
        {
            Resize(strSource.m_maxLen);
            Copy(strSource);
            return m_buffer;
        }

        void Release()
        {
            if(m_buffer != NULL) delete m_buffer;
            if(m_bufferEx != NULL) delete m_bufferEx;
            m_buffer = NULL;
            m_bufferEx = NULL;
            m_maxLen = 0;
        }

        /// clear conents to zero, memory is not changed
        void Clear()
        {
            if(m_buffer != NULL)
            {
                ::ZeroMemory(m_buffer, m_maxLen * sizeof(CharType));
            }

            if(m_bufferEx != NULL)
            {
                ::ZeroMemory(m_bufferEx, m_maxLen * sizeof(ExCharType));
            }
        }

        void Resize(size_t len)
        {
            /// Allocate in temporary buffer
            TCHAR* newBuffer = new TCHAR[len];
            if (newBuffer != NULL)
            {
                /// Initialize buffer to zeros
                ZeroMemory(newBuffer, len * sizeof(TCHAR));

                if(m_buffer != NULL)
                {
                    assert(m_maxLen != 0);

                    /// Copy old stuff to new buffer
                    _tcscpy_s(newBuffer, min(m_maxLen, len), m_buffer);

                    /// relase old buffer
                    Release();
                }

                /// assign new buffer to this
                m_buffer = newBuffer;
                m_maxLen = len;
            }
        }

        /// ------------------------------------------
        /// Get the string buffers
        /// ------------------------------------------

        operator const TCHAR* () const
        {
            return m_buffer;
        }

        operator TCHAR* ()
        {
            return m_buffer;
        }

        TCHAR* GetBuffer()
        {
            assert(m_buffer != NULL);
            return m_buffer;
        }

        const TCHAR* GetString() const
        {
            assert(m_buffer != NULL);
            return (m_buffer == NULL) ? _T("") : m_buffer;
        }

        const WCHAR* GetStringW() const
        {
        #ifdef _UNICODE
            assert(m_buffer != NULL);
            return (m_buffer == NULL) ? L"" : m_buffer;
        #else
            UpdateBufferEx();
            assert(m_bufferEx != NULL);
            return (m_bufferEx == NULL) ? L"" : m_bufferEx;
        #endif
        }

        const CHAR* GetStringA() const
        {
        #ifdef _UNICODE
            UpdateBufferEx();
            assert(m_bufferEx != NULL);
            return (m_bufferEx == NULL) ? "" : m_bufferEx;
        #else
            assert(m_buffer != NULL);
            return (m_buffer == NULL) ? "" : m_buffer;
        #endif
        }

        /// ------------------------------------------
        /// operators support of strings
        /// ------------------------------------------

        const TCHAR* operator += (const TCHAR* strSource)
        {
            Concat(strSource);
            return m_buffer;
        }

        friend SafeString operator + (const SafeString& a, const TCHAR* b)
        {
            SafeString result = a;
            result += b;
            return result;
        }

        friend SafeString operator + (const TCHAR* a, const SafeString& b)
        {
            SafeString result = a;
            result += b;
            return result;
        }

        friend SafeString operator + (const SafeString& a, const SafeString& b)
        {
            SafeString result = a;
            result += b;
            return result;
        }

#define STRING_COMPARE(OP)                                  \
        bool operator OP (const TCHAR* strSource) const     \
        {                                                   \
            return Compare(strSource) OP 0;                 \
        }                                                   \

        STRING_COMPARE(==)
        STRING_COMPARE(!=)
        STRING_COMPARE(>)
        STRING_COMPARE(<)
        STRING_COMPARE(>=)
        STRING_COMPARE(<=)
#undef STRING_COMPARE

        /// ------------------------------------------
        /// wrappers of tchar.h and secure string functions
        /// function names are according to System::String and ATL CString
        /// ------------------------------------------

        size_t MaxLength() const
        {
            return m_maxLen;
        }

        size_t Length() const
        {
            if (m_buffer == NULL) return 0;
            return _tcsnlen(m_buffer, m_maxLen);
        }

        int Copy(const TCHAR* strSource)
        {
            if (m_buffer == NULL || strSource == NULL) return 0;
            return _tcscpy_s(m_buffer, m_maxLen, strSource);
        }

        int Concat(const TCHAR* strSource)
        {
            if (m_buffer == NULL || strSource == NULL) return 0;
            return _tcscat_s(m_buffer, m_maxLen, strSource);
        }

        int Set(TCHAR c)
        {
            if (m_buffer == NULL) return 0;
            return _tcsset_s(m_buffer, m_maxLen, c);
        }

        int Set(TCHAR c, size_t count)
        {
            if (m_buffer == NULL) return 0;
            return _tcsnset_s(m_buffer, m_maxLen, c, count);
        }

        int Format(const TCHAR* format, ...)
        {
            if (format == NULL) return 0;

            va_list args;
            va_start(args, format);
            int result = FormatV(format, args);
            va_end(args);
            return result;
        }

        int FormatV(const TCHAR* format, va_list args)
        {
            if (format == NULL) return 0;

            /// use _TRUNCATE to ensure result is null terminated
            return _vsntprintf_s(m_buffer, m_maxLen, _TRUNCATE, format, args);
        }

        int AppendFormat(const TCHAR* format, ...)
        {
            if (format == NULL) return 0;

            va_list args;
            va_start(args, format);
            const size_t offset = Length();
            TCHAR* position = m_buffer + offset;
            int result = _vsntprintf_s(position, m_maxLen - offset, _TRUNCATE, format, args);
            va_end(args);
            return result;
        }

        int Compare(const TCHAR* strSource) const
        {
            if (m_buffer == NULL || strSource == NULL) return 0;

            return _tcsncmp(m_buffer, strSource, m_maxLen);
        }

        int Compare(const TCHAR* strSource, bool ingoreCase) const
        {
            if (m_buffer == NULL || strSource == NULL) return 0;

            if(ingoreCase)
                return _tcsnicmp(m_buffer, strSource, m_maxLen);
            else
                return _tcsncmp(m_buffer, strSource, m_maxLen);
        }

    private:
        HRESULT UpdateBufferEx() const
        {
            SafeString* This = const_cast<SafeString*>(this);
            size_t converted = 0;

        #ifdef _UNICODE
            if(m_bufferEx == NULL)
            {
                This->m_bufferEx = new CHAR[m_maxLen];
                if (m_bufferEx == NULL) return E_OUTOFMEMORY;
                ZeroMemory(m_bufferEx, m_maxLen * sizeof(CHAR));
            }
            errno_t e = wcstombs_s(
                &converted, This->m_bufferEx, m_maxLen, m_buffer, _TRUNCATE);
            if (e != 0) return E_FAIL;
        #else /// SCBC and MCBC
            if(m_bufferEx == NULL)
            {
                This->m_bufferEx = new WCHAR[This->m_maxLen];
                if (m_bufferEx == NULL) return E_OUTOFMEMORY;
                ZeroMemory(m_bufferEx, This->m_maxLen * sizeof(WCHAR));
            }
            errno_t e  = mbstowcs_s(
                &converted, This->m_bufferEx, m_maxLen, m_buffer, _TRUNCATE);
            if (e != 0) return E_FAIL;
        #endif /// _UNICODE

            return S_OK;
        }

        size_t m_maxLen;
        CharType*   m_buffer;
        ExCharType* m_bufferEx;

        const static size_t m_defaultMaxLen = MAX_PATH;
    };

}   /// namespace utl


#endif//__UTL_SAFESTRING_HPP__
