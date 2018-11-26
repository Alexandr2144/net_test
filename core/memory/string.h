#pragma once
#ifndef MEMORY_STRING_H
#define MEMORY_STRING_H

#include "core/memory/common.h"
#include "core/data/string.h"


namespace Memory {
    using Data::CByte;
    using Data::Byte;


    Data::String newString(IAllocator& a, Data::String s);


    class WeakString
        : public Data::String
    {
    public:
        WeakString(Data::CBytes const& bytes);
        WeakString(char const* beg, char const* end);
        WeakString(char const* str);
        WeakString();

        WeakString(WeakString&& s);
        WeakString(WeakString const& s) = delete;

        WeakString& operator=(WeakString&& s);
        WeakString& operator=(WeakString const& s) = delete;

        Data::String store(IAllocator& a);
        Data::String store();

        bool isStored() const { return m_allocated; }

    private:
        bool m_allocated;
    };


    class ZtString
        : public Data::String
    {
    public:
        ~ZtString();
        ZtString(Data::CBytes const& bytes);
        ZtString(char const* beg, char const* end);
        ZtString(char const* str);
        ZtString() = default;

        ZtString(ZtString&& s);
        ZtString(ZtString const& s) = delete;

        ZtString& operator=(ZtString&& s);
        ZtString& operator=(ZtString const& s) = delete;

        ZtString copy();

        char* alloc(size_t len);
        char* reset(char* ptr = nullptr);
        void clear();

        char const* cstr() const { return (char const*)begin; }
        Array<Byte> data() const;

        static ZtString sprintf(char const* fmt, ...);
    };

} // namespace Memory


#endif // MEMORY_STRING_H