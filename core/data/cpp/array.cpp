#include "core/data/array.h"

#include <memory.h>


namespace Data {
    Bytes noBytes{ nullptr, nullptr };


    // ------------------------------------------------------------------------
    CBytes toBytes(void const* ptr, size_t size)
    {
        return CBytes{ (Byte*)ptr, (Byte*)ptr + size };
    }

    // ------------------------------------------------------------------------
    Bytes toBytes(void* ptr, size_t size)
    {
        return Bytes{ (Byte*)ptr, (Byte*)ptr + size };
    }

    // ------------------------------------------------------------------------
    CBytes toBytes(void const* begin, void const* end)
    {
        return CBytes{ (Byte*)begin, (Byte*)end };
    }

    // ------------------------------------------------------------------------
    Bytes toBytes(void* begin, void* end)
    {
        return Bytes{ (Byte*)begin, (Byte*)end };
    }

    // ------------------------------------------------------------------------
    bool bytecmp(CBytes_CRef right, CBytes_CRef left)
    {
        size_t len1 = count(right);
        size_t len2 = count(left);

        if (len1 != len2) return false;
        return !memcmp(right.begin, left.begin, len1);
    }

    // ------------------------------------------------------------------------
    bool bytecopy(Bytes_CRef dest, CBytes_CRef src)
    {
        size_t dstlen = count(dest);
        size_t srclen = count(src);

        if (dstlen < srclen) return false;
        memcpy(dest.begin, src.begin, srclen);
        return true;
    }

    // ------------------------------------------------------------------------
    void byteset(Bytes dest, Byte val)
    {
        memset(dest.begin, (int)val, count(dest));
    }

    // ------------------------------------------------------------------------
    bool read(Ref<CBytes> src, Bytes dest)
    {
        size_t length = dest.end - dest.begin;
        if (!can_split(src.ref, length)) return false;

        CBytes toCopy = split<CByte>(&src.ref, length);
        memcpy(dest.begin, toCopy.begin, length);
        return true;
    }

    // ------------------------------------------------------------------------
    bool write(Ref<Bytes> dest, CBytes src)
    {
        if (!bytecopy(dest.ref, src)) return false;
        split<Byte>(&dest, count(src));
        return true;
    }

}