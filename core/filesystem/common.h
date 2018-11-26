#pragma once
#ifndef FILESYSTEM_COMMON_H
#define FILESYSTEM_COMMON_H

#include "core/tools/utils.h"
#include "core/tools/stream.h"
#include "core/memory/string.h"

#include "native/file.h"


// TODO: make it right
namespace Filesystem {
    using FileError = Native::FileError;

    using Data::String;
    using Data::Unique;
    using Data::Bytes;
    using Data::Ref;


    // ------------------------------------------------------------------------
    class Mapping {
    public:
        M_DECL_MOVE_ONLY(Mapping);

        Mapping(void* ptr = nullptr, size_t size = 0);
        ~Mapping();

        Bytes get() const { return Data::toBytes(m_base, m_size); }
        size_t size() const { return m_size; }

    private:
        size_t m_size;
        void* m_base;
    };

    class File {
    public:
        M_DECL_MOVE_ONLY(File);

        File(Native::File native = Native::file_invalid());
        ~File();

        void close();

        Mapping map(size_t offset, size_t size, char const* mode);

    private:
        Native::File m_file;
    };

    class Open {
    public:
        M_DECL_MOVE_ONLY(Open);

        Open(Native::File native, Native::FileError error);
        Open(Native::FileError error);
        ~Open();

        bool operator!() { return !isValid(); }
        operator bool() { return isValid(); }

        bool isValid() const { return Native::is_file_valid(m_file); }

        FileError error() const { return m_error; }
        File file();

    private:
        Native::File m_file;
        FileError m_error;
    };

    // ------------------------------------------------------------------------
    struct DirEntry {
        Native::DirEntry native;

    public:
        DirEntry();
        DirEntry(Native::DirEntry e);

        bool isDirectory() const;
        size_t getFileName(Data::Array<Data::Byte> buffer) const;
    };

    struct IMountPoint {
        virtual ~IMountPoint() = default;

        virtual Memory::ZtString path(char const* relative) const = 0;

        virtual DirEntry find_first(char const* path) = 0;
        virtual bool find_next(Ref<DirEntry> entry) = 0;
        
        virtual Open open(char const* path, char const* attr) = 0;
    };

    template <class T, class... ArgsTy>
    bool mount(String const& path, ArgsTy&&... args) { return mount(path, new T(args...)); }

    bool mount(String const& path, IMountPoint* mp);
    bool unmount(String const& path);

    Open open(String const& path, char const* attr);

    Memory::ZtString get_native_path(String const& path);

    // ------------------------------------------------------------------------
    struct IWalkHandler {
        virtual bool onDirectoryFound(String const& relative, String const& absolute) = 0;
        virtual void onFileFound(String const& relative, String const& absolute) = 0;
    };

    void walk(String const& path, Ref<IWalkHandler> handler);

} // namespace Filesystem


#endif // FILESYSTEM_COMMON_H