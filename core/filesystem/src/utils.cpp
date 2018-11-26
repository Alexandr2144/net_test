#include "core/filesystem/utils.h"

#include "core/math/common.h"
#include "native/file.h"
#include <string.h>


namespace Filesystem {
    using namespace Data;

    // ------------------------------------------------------------------------
    static Unique<char> getOriginPath(String prefix, char const* path)
    {
        size_t length = strlen(path);
        auto buffer = Unique<char>::alloc(count(prefix) + length + 2);
        char* offset = buffer.get() + count(prefix);

        memcpy(buffer.get(), prefix.begin, count(prefix));
        memcpy(offset + 1, path, length);

        *(offset + length + 1) = '\0';
        *offset = '/';

        return buffer;
    }

    // ------------------------------------------------------------------------
    String extension(String filename)
    {
        Byte const* end = filename.end;
        while (!isEmpty(filename)) {
            Byte const* ch = filename.end - 1;
            if (*ch == '.') {
                return String({ filename.end, end });
            }
            filename.end -= 1;
        }
        return filename;
    }

    // ------------------------------------------------------------------------
    String basename(String filename)
    {
        String ext = extension(filename);
        if (isEmpty(ext)) return filename;

        ext.begin -= 1;
        return rsubtract(filename, ext);
    }

    // ------------------------------------------------------------------------
    // NativeLink implementation
    // ------------------------------------------------------------------------
    NativeLink::NativeLink(char const* prefix)
        : m_prefix(prefix)
    {
    }

    // ------------------------------------------------------------------------
    DirEntry NativeLink::find_first(char const* path)
    {
        auto buffer = getOriginPath(m_prefix, path);

        Native::DirEntry entry = Native::direntry_first(buffer.get());
        return DirEntry{ entry };
    }

    // ------------------------------------------------------------------------
    bool NativeLink::find_next(Ref<DirEntry> entry)
    {
        return Native::direntry_next(&entry->native);
    }

    // ------------------------------------------------------------------------
    Open NativeLink::open(char const* path, char const* attr)
    {
        auto buffer = getOriginPath(m_prefix, path);

        Native::File file = Native::file_open(buffer.get(), attr);
        return Open(file, Native::file_error());
    }

    // ------------------------------------------------------------------------
    Memory::ZtString NativeLink::path(char const* relative) const
    {
        return Memory::ZtString::sprintf("%s/%s", m_prefix.begin, relative);
    }

} // namespace Filesystem