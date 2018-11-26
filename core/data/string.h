#pragma once
#ifndef DATA_STRING_H
#define DATA_STRING_H

#include "core/data/pointers.h"
#include "core/data/array.h"


namespace Data {
    // ------------------------------------------------------------------------
    struct String : public CBytes {
        explicit String(CBytes bytes);

        String(char const* beg, char const* end);
        String(char const* str);
        String();

        String(String&&);
        String(String const&) = default;
        String& operator=(String&&);
        String& operator=(String const&) = default;

    public:
        String operator[](size_t idx) = delete;
        bool operator==(String const& other) const;
        bool operator!=(String const& other) const;

        String front() const;
        int front_ascii() const;
    };
    using String_CRef = String const&;

    // ------------------------------------------------------------------------
    template <int N>
    struct StringOnStack : public String {
        char buffer[N];

        StringOnStack() : String{ buffer, buffer + N } {}
    };

    // ------------------------------------------------------------------------
    struct StringLocation {
        size_t line = 1;
        size_t row = 1;

    public:
        void apply(String_CRef s);
        void apply(int ch);
    };

    // ------------------------------------------------------------------------
    struct StringIterator : public String {
        explicit StringIterator(CBytes bytes) : String(bytes) {}

        StringIterator(char const* beg, char const* end) : String(beg, end) {}
        StringIterator(char const* str) : String(str) {}
        StringIterator() : String() {}

        bool operator==(StringIterator const& it);

        StringIterator operator++(int);
        StringIterator& operator++();

        StringIterator& next(Ref<StringLocation> loc);
        StringIterator& next();
    };

    struct StringCollection {
        String string;

    public:
        StringCollection(String_CRef s);

        StringIterator begin();
        StringIterator end();
    };

    // ------------------------------------------------------------------------

    size_t hash(char const* str);
    size_t hash(String_CRef str);

    String lstrip(String_CRef s);
    String lstrip(String_CRef s, Ref<StringLocation> loc);
    String lsubtract(String_CRef base, String_CRef left);
    String rsubtract(String_CRef base, String_CRef right);
    String takeWhile(String_CRef s, bool(*predicate)(String_CRef));

    String strchr(String_CRef s, int ch);

    void lowlvl_step(Ref<String> s, Ref<StringLocation> loc);
    void lowlvl_steps(Ref<String> s, Ref<StringLocation> loc, size_t steps);
    void lowlvl_back(Ref<String> s, Ref<StringLocation> loc);

    void sprintf(String_CRef str, const char* fmt, ...);

} // namespace Data


#endif // DATA_STRING_H