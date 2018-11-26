#include "core/filesystem/common.h"

#include "core/memory/containers.h"
#include "core/memory/string.h"

#include "core/data/name_tree.h"
#include "core/math/common.h"

#include <string.h>


namespace Filesystem {
    using namespace Data;

    struct WalkPoint { NameTree::Iterator it; IMountPoint* mp; DirEntry entry; };


    // ------------------------------------------------------------------------
    class FsTree {
    public:
        Memory::RaPool<IMountPoint*> mpoints;
        NameTree nametree;

        static FsTree& instance();

        static IMountPoint* mountpoint(size_t idx);
        static NameTree& tree();

    private:
        FsTree() : nametree(SIZE_MAX) {}
    };

    // ------------------------------------------------------------------------
    class PathProxy
        : public Tools::IStream<Data::String>
    {
    public:
        M_DECL_MOVE_ONLY(PathProxy);

        PathProxy(String const& strpath);
        ~PathProxy();

        virtual bool finished() const override;
        virtual Data::String read() override;

        virtual void restore() override;
        virtual void save() override;

        char const* raw() const { return m_pos; }
        size_t offset() const { return m_pos - s_path; }

    private:
        static thread_local char s_path[PATH_MAX];
        char const* m_saved;
        char const* m_pos;
    };

    // ------------------------------------------------------------------------
    thread_local char PathProxy::s_path[PATH_MAX];

    // ------------------------------------------------------------------------
    FsTree& FsTree::instance()
    {
        static FsTree fs_tree;
        return fs_tree;
    }

    // ------------------------------------------------------------------------
    IMountPoint* FsTree::mountpoint(size_t idx)
    {
        return instance().mpoints[idx];
    }

    // ------------------------------------------------------------------------
    NameTree& FsTree::tree()
    {
        return instance().nametree;
    }

    // ------------------------------------------------------------------------
    static String genPath(Array<Byte> out, Array<WalkPoint> const& points, DirEntry const& entry)
    {
        for (auto const& point : iterate(points)) {
            size_t len = point.entry.getFileName(out);
            out.begin[len - 1] = '/';
            out.begin[len] = '\0';
            out.begin += len;
        }

        Byte* begin = out.begin;
        size_t len = entry.getFileName(out);
        return String({ begin, begin + len - 1 });
    }

    // ------------------------------------------------------------------------
    static Array<Byte> writeBaseName(Byte* buffer, char const* base)
    {
        size_t length = Math::min(strlen(base), PATH_MAX - 1);
        memcpy(buffer + 1, base, length);
        buffer[length + 2] = '\0';
        buffer[length + 1] = '/';
        buffer[0] = '/';

        return Array<Byte>{ buffer + length + 2, buffer + PATH_MAX };
    }

    // ------------------------------------------------------------------------
    void walk(String const& strpath, Ref<IWalkHandler> handler)
    {
        Memory::RaStack<WalkPoint> stack;
        PathProxy path(strpath);

        Byte buffer[PATH_MAX];
        Array<Byte> namebuf = writeBaseName(buffer, path.raw());

        WalkPoint wp;
        wp.it = FsTree::tree().find(&path);
        wp.mp = FsTree::mountpoint(wp.it.id);
        
        char const* subpath = (char*)(buffer + path.offset() + 1);
        wp.entry = wp.mp->find_first(path.raw());
        path.~PathProxy();

        while (true) {
            while (wp.mp->find_next(&wp.entry)) {
                String name = genPath(namebuf, stack.asArray(), wp.entry);
                String relative({ namebuf.begin, name.end });
                String fullname({ buffer, name.end });

                if (name == ".") continue;
                if (name == "..") continue;
                if (wp.entry.isDirectory()) {
                    if (handler->onDirectoryFound(relative, fullname)) {
                        stack.append(wp);
                        wp.mp = FsTree::mountpoint(wp.it.step(name));
                        wp.entry = wp.mp->find_first(subpath);
                        continue;
                    }
                }
                else {
                    handler->onFileFound(relative, fullname);
                }
            }

            Native::direntry_end(&wp.entry.native);
            if (stack.isEmpty()) {
                return;
            }

            wp = stack.lastval();
            stack.pop();
        }
    }

    // ------------------------------------------------------------------------
    Memory::ZtString get_native_path(String const& strpath)
    {
        PathProxy path(strpath);

        size_t id = FsTree::tree().find(&path).id;
        M_ASSERT(id != SIZE_MAX);

        IMountPoint* mp = FsTree::mountpoint(id);
        return mp->path(path.raw());
    }

    // ------------------------------------------------------------------------
    Open open(String const& strpath, char const* attr)
    {
        PathProxy path(strpath);

        size_t id = FsTree::tree().find(&path).id;
        if (id == SIZE_MAX) {
            return Open(Native::FileError::NotExist);
        }

        IMountPoint* mp = FsTree::mountpoint(id);
        return mp->open(path.raw(), attr);
    }

    // ------------------------------------------------------------------------
    bool mount(String const& strpath, IMountPoint* mp)
    {
        auto& mpoints = FsTree::instance().mpoints;
        PathProxy path(strpath);

        size_t idx = mpoints.emplace(mp);
        return FsTree::tree().insert(&path, idx);
    }

    // ------------------------------------------------------------------------
    bool unmount(String const& strpath)
    {
        PathProxy path(strpath);

        size_t id = FsTree::tree().remove(&path);
        if (id == SIZE_T_MAX) {
            return false;
        }

        auto& mpoints = FsTree::instance().mpoints;
        IMountPoint** ptr = mpoints.ptr(id);
        Memory::dealloc(*ptr);
        mpoints.dealloc(ptr);
        return true;
    }

    // ------------------------------------------------------------------------
    static String readDirInPath(String const& str)
    {
        Data::CByte* next = (Data::CByte*)memchr(str.begin, '/', str.end - str.begin);

        if (next == nullptr) next = str.end;
        return String({ str.begin, next });
    }

    // ------------------------------------------------------------------------
    static void canonize(char* dest, String str)
    {
        char* top = dest;
        char* end = dest + PATH_MAX - 1;

        size_t realDirs = 0;
        if (str.front_ascii() == '~') {
            char const* home_prefix = "/home/username/";

            size_t size = strlen(home_prefix);
            strncpy_s(top, PATH_MAX, home_prefix, size);
            top += size;
            realDirs = 2;
        }
        else if (str.front_ascii() != '/') {
            char const* workdir_prefix = "/workdir/";

            size_t size = strlen(workdir_prefix);
            strncpy_s(top, PATH_MAX, workdir_prefix, size);
            top += size;
            realDirs = 1;
        }

        *top = '\0';
        while (!isEmpty(str)) {
            while (!isEmpty(str) && str.front_ascii() == '/') str.begin += 1;
            if (isEmpty(str)) break;

            String dir = readDirInPath(str);
            str.begin = dir.end;

            if (dir == String(".")) continue;
            if (dir == String("..") && realDirs != 0) {
                top = strrchr(dest, '/');
                M_ASSERT(top != nullptr);
                realDirs -= 1;
            }
            else {
                size_t size = Math::min(count(dir), size_t(end - top));
                memcpy(top, dir.begin, size);
                realDirs += 1;
                top += size;

                *(top++) = '/';
                *top = '\0';
            }
        }

        *(top--) = '\0';
        if (*top == '/') *top = '\0';
    }

    // ------------------------------------------------------------------------
    // Path implementation
    // ------------------------------------------------------------------------
    PathProxy::PathProxy(String const& strpath)
        : m_pos(s_path), m_saved(s_path)
    {
        M_ASSERT_MSG(s_path[0] == '\0', "Invalid usage of PathProxy: one instance per thread allowed");
        canonize(s_path, strpath);
    }

    // ------------------------------------------------------------------------
    PathProxy::PathProxy(PathProxy&& proxy)
        : m_pos(proxy.m_pos), m_saved(proxy.m_saved)
    {
        proxy.m_pos = nullptr;
        proxy.m_saved = nullptr;
    }

    // ------------------------------------------------------------------------
    PathProxy::~PathProxy()
    {
        if (m_pos != nullptr) {
            s_path[0] = '\0';
        }
    }

    // ------------------------------------------------------------------------
    PathProxy& PathProxy::operator=(PathProxy&& proxy)
    {
        new(this) PathProxy(std::forward<PathProxy>(proxy));
        return *this;
    }

    // ------------------------------------------------------------------------
    bool PathProxy::finished() const
    {
        return *m_pos == '\0';
    }

    // ------------------------------------------------------------------------
    void PathProxy::restore()
    {
        m_pos = m_saved;
    }

    // ------------------------------------------------------------------------
    void PathProxy::save()
    {
        m_saved = m_pos;
    }

    // ------------------------------------------------------------------------
    String PathProxy::read()
    {
        char const* next = strpbrk(m_pos, "\\/");
        if (next == nullptr) next = m_pos + strlen(m_pos);

        String out(m_pos, next);
        m_pos = *next ? next + 1 : next;
        return out;
    }

    // ------------------------------------------------------------------------
    // Open implementation
    // ------------------------------------------------------------------------
    Open::Open(Native::File native, Native::FileError error)
        : m_file(native), m_error(error)
    {
    }

    // ------------------------------------------------------------------------
    Open::Open(Native::FileError error)
        : m_file(Native::file_invalid()), m_error(error)
    {
    }

    // ------------------------------------------------------------------------
    Open::Open(Open&& file)
        : m_file(file.m_file), m_error(file.m_error)
    {
        file.m_error = Native::FileError::NoErrors;
        file.m_file = Native::file_invalid();
    }

    // ------------------------------------------------------------------------
    Open::~Open()
    {
        if (Native::is_file_valid(m_file)) {
            file_close(m_file);
        }
    }

    // ------------------------------------------------------------------------
    Open& Open::operator=(Open&& file)
    {
        new(this) Open(std::move(file));
        return *this;
    }

    // ------------------------------------------------------------------------
    File Open::file()
    {
        M_ASSERT_MSG(isValid(), "Call 'file' in invalid state is unacceptable");

        Native::File result = m_file;
        m_file = Native::file_invalid();
        return File(result);
    }

    // ------------------------------------------------------------------------
    // File implementation
    // ------------------------------------------------------------------------
    File::File(Native::File native)
        : m_file(native)
    {
    }

    // ------------------------------------------------------------------------
    File::File(File&& file) : m_file(file.m_file)
    {
        file.m_file = Native::file_invalid();
    }

    // ------------------------------------------------------------------------
    File::~File()
    {
        close();
    }

    // ------------------------------------------------------------------------
    File& File::operator=(File&& file)
    {
        new(this) File(std::move(file));
        return *this;
    }

    // ------------------------------------------------------------------------
    void File::close()
    {
        if (Native::is_file_valid(m_file)) {
            Native::file_close(m_file);
        }
    }

    // ------------------------------------------------------------------------
    Mapping File::map(size_t offset, size_t size, char const* mode)
    {
        size_t filesize = Native::file_size(m_file);
        size_t mapsize = Math::min(size, filesize - offset);
        if (mapsize == 0) {
            return Mapping(nullptr, 0);
        }

        void* ptr = Native::file_map(&m_file, offset, mapsize, mode);
        return Mapping(ptr, mapsize);
    }

    // ------------------------------------------------------------------------
    // Mapping implementation
    // ------------------------------------------------------------------------
    Mapping::Mapping(void* ptr, size_t size)
        : m_base(ptr), m_size(size)
    {
    }

    // ------------------------------------------------------------------------
    Mapping::Mapping(Mapping&& mapping)
        : m_base(mapping.m_base), m_size(mapping.m_size)
    {
        mapping.m_base = nullptr;
        mapping.m_size = 0;
    }

    // ------------------------------------------------------------------------
    Mapping::~Mapping()
    {
        Native::file_unmap(m_base);
    }

    // ------------------------------------------------------------------------
    Mapping& Mapping::operator=(Mapping&& mapping)
    {
        Native::file_unmap(m_base);
        new(this) Mapping(std::move(mapping));
        return *this;
    }

    // ------------------------------------------------------------------------
    // DirEntry implementation
    // ------------------------------------------------------------------------
    DirEntry::DirEntry()
        : native(Native::direntry_invalid())
    {
    }

    // ------------------------------------------------------------------------
    DirEntry::DirEntry(Native::DirEntry e)
        : native(e)
    {
    }

    // ------------------------------------------------------------------------
    bool DirEntry::isDirectory() const
    {
        return Native::is_directory(native);
    }

    // ------------------------------------------------------------------------
    size_t DirEntry::getFileName(Array<Byte> buffer) const
    {
        return Native::get_filename(native, buffer);
    }

} // namespace Filesystem