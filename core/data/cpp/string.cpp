#include "core/data/string.h"

#include <string.h>
#include <ctype.h>


namespace Data {
    // ------------------------------------------------------------------------
    size_t utf8CharacterLength(String_CRef s)
    {
        uint8_t lead = (uint8_t)*s.begin;
        if (lead < 0x80)               // 0xxxxxxx
            return 1;
        else if ((lead >> 5) == 0x06)  // 110xxxxx 10xxxxxx
            return 2;
        else if ((lead >> 4) == 0x0e)  // 1110xxxx 10xxxxxx 10xxxxxx
            return 3;
        else if ((lead >> 3) == 0x1e)  // 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
            return 4;
        else                           // invalid utf8 character
            return 0;
    }

    // ------------------------------------------------------------------------
    String::String()
        : CBytes(noBytes)
    {
    }

    // ------------------------------------------------------------------------
    String::String(char const* str)
        : CBytes(toBytes(str, strlen(str)))
    {
    }

    // ------------------------------------------------------------------------
    String::String(char const* begin, char const* end)
        : CBytes(toBytes(begin, end))
    {
    }

    // ------------------------------------------------------------------------
    String::String(CBytes bytes)
        : CBytes(bytes)
    {
    }

    // ------------------------------------------------------------------------
    String::String(String&& s)
        : CBytes(std::move(s))
    {
    }

    // ------------------------------------------------------------------------
    String& String::operator=(String&& s)
    {
        (CBytes&)*this = std::move((CBytes&&)s);
        return *this;
    }

    // ------------------------------------------------------------------------
    String String::front() const
    {
        if (isEmpty(*this)) return String();

        size_t length = utf8CharacterLength(*this);
        M_ASSERT_MSG(can_slice(*this, 0, length), "UTF8 string corruption detected");
        return String(slice(*this, 0, length));
    }

    // ------------------------------------------------------------------------
    int String::front_ascii() const
    {
        return (int)(*front().begin);
    }

    // ------------------------------------------------------------------------
    bool String::operator==(String_CRef other) const
    {
        return bytecmp(*this, other);
    }

    // ------------------------------------------------------------------------
    bool String::operator!=(String_CRef other) const
    {
        return !(*this == other);
    }

    // ------------------------------------------------------------------------
    size_t hash(char const* str)
    {
        size_t h = 0;
        while (*str) {
            h += *str; h <<= 8;
            ++str;
        }
        return h;
    }

    // ------------------------------------------------------------------------
    size_t hash(String_CRef str)
    {
        size_t h = 0;
        CByte* byte = str.begin;
        while (byte != str.end) {
            h += (uint8_t)*byte;
            h <<= 8;
            ++byte;
        }

        return h;
    }

    // ------------------------------------------------------------------------
    String lstrip(String_CRef s)
    {
        StringIterator it(s);
        while (isspace(it.front_ascii()) && it.front_ascii() != '\n') {
            it.next();
        }
        return it;
    }

    // ------------------------------------------------------------------------
    String lstrip(String_CRef s, Ref<StringLocation> loc)
    {
        StringIterator it(s);
        while (isspace(it.front_ascii()) && it.front_ascii() != '\n') {
            ++loc.ref.row;
            it.next();
        }
        return it;
    }

    // ------------------------------------------------------------------------
    String rsubtract(String_CRef base, String_CRef right)
    {
        return String({ base.begin, right.begin });
    }

    // ------------------------------------------------------------------------
    String lsubtract(String_CRef base, String_CRef left)
    {
        return String({ left.end, base.end });
    }

    // ------------------------------------------------------------------------
    String takeWhile(String_CRef s, bool(*predicate)(String_CRef))
    {
        StringIterator fwd(s);
        while (predicate(fwd)) {
            fwd.next();
        }
        return String({ s.begin, fwd.begin });
    }

    // ------------------------------------------------------------------------
    String strchr(String_CRef s, int ch)
    {
        CByte* begin = (CByte*)memchr(s.begin, ch, s.end - s.begin);
        return String({ begin, s.end });
    }

    // ------------------------------------------------------------------------
    void lowlvl_step(Ref<String> s, Ref<StringLocation> loc)
    {
        loc->apply(s.ref.front_ascii());
        s->begin += 1;
    }

    // ------------------------------------------------------------------------
    void lowlvl_steps(Ref<String> s, Ref<StringLocation> loc, size_t steps)
    {
        while (steps--) lowlvl_step(&s, &loc);
    }

    // ------------------------------------------------------------------------
    void lowlvl_back(Ref<String> s, Ref<StringLocation> loc)
    {
        if (s->front_ascii() == '\n') {
            loc->line -= 1;
        }
        s.ref.begin -= 1;
    }

    // ------------------------------------------------------------------------
    // StringLocation implementation
    // ------------------------------------------------------------------------
    void StringLocation::apply(String_CRef s)
    {
        apply(s.front_ascii());
    }

    // ------------------------------------------------------------------------
    void StringLocation::apply(int ch)
    {
        if (ch == '\n') {
            line += 1;
            row = 1;
        }
        else {
            row += 1;
        }
    }

    // ------------------------------------------------------------------------
    // StringCollection implementation
    // ------------------------------------------------------------------------
    StringCollection::StringCollection(String_CRef s)
        : string(s)
    {
    }

    // ------------------------------------------------------------------------
    StringIterator StringCollection::begin()
    {
        return StringIterator(string);
    }

    // ------------------------------------------------------------------------
    StringIterator StringCollection::end()
    {
        return StringIterator({ string.end, string.end });
    }

    // ------------------------------------------------------------------------
    // StringIterator implementation
    // ------------------------------------------------------------------------
    bool StringIterator::operator==(StringIterator const& it)
    {
        return this->begin == it.begin;
    }

    // ------------------------------------------------------------------------
    StringIterator StringIterator::operator++(int)
    {
        StringIterator it = *this;
        next(); return it;
    }

    // ------------------------------------------------------------------------
    StringIterator& StringIterator::operator++()
    {
        return next();
    }

    // ------------------------------------------------------------------------
    StringIterator& StringIterator::next()
    {
        M_ASSERT(begin <= end);
        begin += utf8CharacterLength(*this);
        return *this;
    }

    // ------------------------------------------------------------------------
    StringIterator& StringIterator::next(Ref<StringLocation> loc)
    {
        M_ASSERT(begin <= end);
        if (front_ascii() == '\n')
            ++loc.ref.line, loc.ref.row = 1;
        else ++loc.ref.row;

        begin += utf8CharacterLength(*this);
        return *this;
    }

} // namespace Data