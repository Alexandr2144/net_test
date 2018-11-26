#include "core/memory/string.h"

#include "core/memory/buddy_heap.h"

#include <string.h>
#include <memory.h>
#include <stdarg.h>
#include <stdio.h>


namespace Memory {
    using namespace Data;

    // --------------------------------------------------------------------
    // WeakString implementation
    // --------------------------------------------------------------------
    WeakString::WeakString(char const* begin, char const* end)
        : String(begin, end), m_allocated(false)
    {
    }

    // --------------------------------------------------------------------
    WeakString::WeakString(Data::CBytes const& bytes)
        : String(bytes), m_allocated(false)
    {
    }

    // --------------------------------------------------------------------
    WeakString::WeakString(char const* str)
        : String(str), m_allocated(false)
    {
    }

    // --------------------------------------------------------------------
    WeakString::WeakString()
        : String()
    {
    }

    // --------------------------------------------------------------------
    WeakString::WeakString(WeakString&& s)
        : String({ s.begin, s.end }), m_allocated(s.m_allocated)
    {
        s.m_allocated = false;
        s.begin = nullptr;
        s.end = nullptr;
    }

    // --------------------------------------------------------------------
    WeakString& WeakString::operator=(WeakString&& s)
    {
        new(this) WeakString(std::forward<WeakString>(s));
        return *this;
    }

    // --------------------------------------------------------------------
    Data::String WeakString::store(IAllocator& a)
    {
        if (m_allocated) return *this;

        size_t length = end - begin;
        Bytes bytes = a.alloc(length + 1);
        memcpy(bytes.begin, begin, length);
        bytes[length] = '\0';

        begin = bytes.begin;
        end = bytes.end - 1;
        return *this;
    }

    // --------------------------------------------------------------------
    Data::String WeakString::store()
    {
        return store(buddy_global_heap);
    }

    // --------------------------------------------------------------------
    // ZtString implementation
    // --------------------------------------------------------------------
    ZtString::ZtString(char const* begin, char const* end)
    {
        size_t len = end - begin;
        char* buf = alloc(len);
        memcpy(buf, begin, len);
    }

    // --------------------------------------------------------------------
    ZtString::ZtString(Data::CBytes const& bytes)
    {
        size_t len = bytes.end - bytes.begin;
        char* buf = alloc(len);
        memcpy(buf, bytes.begin, len);
    }

    // --------------------------------------------------------------------
    ZtString::ZtString(char const* str)
    {
        if (str) {
            size_t len = strlen(str);
            char* buf = alloc(len);
            memcpy(buf, str, len);
        }
    }

    // --------------------------------------------------------------------
    ZtString::ZtString(ZtString&& s)
        : String(std::move(s))
    {
        s.reset();
    }

    // --------------------------------------------------------------------
    ZtString& ZtString::operator=(ZtString&& s)
    {
        new(this) ZtString(std::forward<ZtString>(s));
        return *this;
    }

    // --------------------------------------------------------------------
    ZtString::~ZtString()
    {
        clear();
    }

    // --------------------------------------------------------------------
    ZtString ZtString::copy()
    {
        ZtString result;
        size_t length = end - begin;
        char* data = result.alloc(length);
        memcpy(data, begin, length);
        return std::move(result);
    }

    // --------------------------------------------------------------------
    void ZtString::clear()
    {
        free(const_cast<Byte*>(begin));
        begin = nullptr;
        end = nullptr;
    }

    // --------------------------------------------------------------------
    char* ZtString::reset(char* ptr)
    {
        char* out = const_cast<char*>(cstr());
        begin = (Byte*)ptr;
        end = begin ? begin + strlen(ptr) : nullptr;
        return out;
    }

    // --------------------------------------------------------------------
    Array<Byte> ZtString::data() const
    {
        return Array<Byte>(
            const_cast<Byte*>(begin),
            const_cast<Byte*>(end)
        );
    }

    // --------------------------------------------------------------------
    char* ZtString::alloc(size_t len)
    {
        free(const_cast<Byte*>(begin));
        char* data = (char*)malloc(len + 1);
        data[len] = '\0';

        begin = (CByte*)data;
        end = begin + len;
        return data;
    }

    // --------------------------------------------------------------------
    ZtString ZtString::sprintf(char const* fmt, ...)
    {
        va_list args;
        va_start(args, fmt);
        int len = _vscprintf(fmt, args);

        ZtString out;
        char* data = out.alloc(len);
        vsnprintf(data, len + 1, fmt, args);
        va_end(args);
        return out;
    }


} // namespace Memory