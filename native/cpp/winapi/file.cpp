#include "native/file.h"
#include "core/data/pointers.h"

#include "mprotect_flags.hpp"


namespace Native {
    using namespace Data;

    enum FileFlagsBits {
        FILE_ACCESS_READ = 0x0001,
        FILE_ACCESS_WRITE = 0x0002,
        FILE_ACCESS_EXECUTE = 0x0004,

        FILE_SHARED_READ = 0x0010,
        FILE_SHARED_WRITE = 0x0020,

        FILE_MUST_EXIST = 0x0100,
        FILE_MUST_NOT_EXIST = 0x0200,

        FILE_CREATE_TRUNCATE = 0x1000,
    };

    static thread_local FileError filesystem_error = FileError::NoErrors;

    // ------------------------------------------------------------------------
    // static functions
    // ------------------------------------------------------------------------
    static int read_file_open_attributes(char const* attr)
    {
        int flags = 0;
        while (true) {
            switch (*attr) {
            case 'r': flags |= FILE_ACCESS_READ; break;
            case 'w': flags |= FILE_ACCESS_WRITE; break;
            case 'x': flags |= FILE_ACCESS_EXECUTE; break;
            case 'n': flags |= FILE_CREATE_TRUNCATE; break;
            case 'R': flags |= FILE_SHARED_READ; break;
            case 'W': flags |= FILE_SHARED_WRITE; break;
            case '+':
                M_ASSERT_MSG(!(flags & FILE_MUST_NOT_EXIST), "Incompatible file_open attributes: '+' after '-'");
                flags |= FILE_MUST_EXIST;
                break;
            case '-':
                M_ASSERT_MSG(!(flags & FILE_MUST_EXIST), "Incompatible file_open attributes: '+' after '-'");
                flags |= FILE_MUST_NOT_EXIST;
                break;
            case '\0':
                if ((flags & FILE_MUST_NOT_EXIST) && !(flags & FILE_CREATE_TRUNCATE)) {
                    M_ASSERT_FAIL("To open file that not exists is impossible");
                }
                return flags;
            default:
                M_ASSERT_FAIL("Invalid mprotect attribute: expected '+-nrwxRW' but '%c' found", *attr);
            }
            attr += 1;
        }
    }

    // ------------------------------------------------------------------------
    static DWORD get_desired_access(int flags)
    {
        DWORD dwFlags = 0;
        if (flags & FILE_ACCESS_READ) {
            dwFlags |= GENERIC_READ;
        }
        if (flags & FILE_ACCESS_WRITE) {
            dwFlags |= GENERIC_WRITE;
        }
        if (flags & FILE_ACCESS_EXECUTE) {
            dwFlags |= GENERIC_EXECUTE;
        }
        return dwFlags;
    }

    // ------------------------------------------------------------------------
    static DWORD get_share_mode(int flags)
    {
        DWORD dwFlags = 0;
        if (flags & FILE_SHARED_READ) {
            dwFlags |= FILE_SHARE_READ;
        }
        if (flags & FILE_SHARED_WRITE) {
            dwFlags |= FILE_SHARE_WRITE;
        }
        return dwFlags;
    }

    // ------------------------------------------------------------------------
    static DWORD get_disposition(int flags)
    {
        bool create = (flags & FILE_CREATE_TRUNCATE) != 0;
        if (flags & FILE_MUST_NOT_EXIST) {
            return CREATE_NEW;
        }
        else if (flags & FILE_MUST_EXIST) {
            return create ? TRUNCATE_EXISTING : OPEN_EXISTING;
        }
        else {
            return create ? CREATE_ALWAYS : OPEN_ALWAYS;
        }
    }

    // ------------------------------------------------------------------------
    static Unique<wchar_t> convUtf8ToUcs(char const* str, size_t* ucslen, size_t extra)
    {
        size_t length = strlen(str) + 1;
        auto wide = Unique<wchar_t>::alloc(length + extra);

        *ucslen = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, str, (int)length, wide.get(), (int)length);
        if (*ucslen == 0) return nullptr;
        return wide;
    }

    // ------------------------------------------------------------------------
    template <class T>
    static T set_error(T const& retval, FileError error)
    {
        filesystem_error = error;
        return retval;
    }

    // ------------------------------------------------------------------------
    // interface functions
    // ------------------------------------------------------------------------
    FileError file_error()
    {
        auto result = filesystem_error;
        filesystem_error = FileError::NoErrors;
        return result;
    }

    // ------------------------------------------------------------------------
    bool is_file_valid(File file)
    {
        return file.hFile != INVALID_HANDLE_VALUE;
    }

    // ------------------------------------------------------------------------
    File file_invalid()
    {
        return File{ INVALID_HANDLE_VALUE, NULL };
    }

    // ------------------------------------------------------------------------
    File file_open(char const* path, char const* attr)
    {
        if (path == nullptr || attr == nullptr) {
            return set_error(file_invalid(), FileError::InvalidArgs);
        }
        int flags = read_file_open_attributes(attr);

        size_t ucslen;
        Unique<wchar_t> widepath = convUtf8ToUcs(path, &ucslen, 0);
        if (!widepath) return set_error(file_invalid(), FileError::NonUcsPath);

        HANDLE hFile = CreateFileW(widepath.get(), get_desired_access(flags), get_share_mode(flags), NULL, get_disposition(flags), FILE_ATTRIBUTE_NORMAL, NULL);
        if (hFile == INVALID_HANDLE_VALUE) {
            DWORD error = GetLastError();
            if (error == ERROR_ALREADY_EXISTS) {
                return set_error(file_invalid(), FileError::AlreadyExist);
            } else if (error == NULL) {
                return set_error(file_invalid(), FileError::NotExist);
            } else {
                return set_error(file_invalid(), FileError::UnknownError);
            }
        }

        return File{ hFile };
    }

    // ------------------------------------------------------------------------
    bool file_close(File file)
    {
        bool ok = true;
        if (file.hMapping != NULL) {
            if (CloseHandle(file.hMapping) == FALSE) {
                ok = false;
            }
        }
        if (file.hFile != INVALID_HANDLE_VALUE) {
            if (CloseHandle(file.hFile) == FALSE) {
                ok = false;
            }
        }
        return ok;
    }

    // ------------------------------------------------------------------------
    void* file_map(Ref<File> file, uint64_t offset, size_t size, char const* attr)
    {
        int flags = rwx_flags_from_attr(attr);

        if (file->hMapping == NULL) {
            file->hMapping = CreateFileMappingA(file->hFile, NULL, mprotect_flags(flags), 0, 0, NULL);
            if (file->hMapping == NULL) {
                DWORD error = GetLastError();
                if (error == ERROR_FILE_INVALID) {
                    return set_error(nullptr, FileError::NoErrors);
                } else if (error == ERROR_ACCESS_DENIED) {
                    return set_error(nullptr, FileError::AccessDenied);
                } else {
                    return set_error(nullptr, FileError::UnknownError);
                }
            }
        }

        ULARGE_INTEGER uiOffset;
        uiOffset.QuadPart = offset;
        void* ptr = MapViewOfFile(file->hMapping, filemap_flags(flags), uiOffset.HighPart, uiOffset.LowPart, size);
        if (ptr == nullptr) {
            if (GetLastError() == ERROR_MAPPED_ALIGNMENT) {
                return set_error(nullptr, FileError::InvalidAligment);
            } else {
                return set_error(nullptr, FileError::UnknownError);
            }
        }

        return ptr;
    }

    // ------------------------------------------------------------------------
    bool file_unmap(void* ptr)
    {
        if (ptr != nullptr) {
            if (UnmapViewOfFile(ptr) == FALSE) {
                return false;
            }
        }
        return true;
    }

    // ------------------------------------------------------------------------
    uint64_t file_size(File file)
    {
        ULARGE_INTEGER uiSize;
        uiSize.LowPart = GetFileSize(file.hFile, &uiSize.HighPart);
        return uiSize.QuadPart;
    }

    // ------------------------------------------------------------------------
    DirEntry direntry_invalid()
    {
        return DirEntry{ INVALID_HANDLE_VALUE, nullptr };
    }

    // ------------------------------------------------------------------------
    DirEntry direntry_first(char const* path)
    {
        if (path == nullptr) {
            return set_error(direntry_invalid(), FileError::InvalidArgs);
        }

        size_t ucslen;
        auto data = Unique<WIN32_FIND_DATAW>::alloc(1);
        auto widepath = convUtf8ToUcs(path, &ucslen, 2);
        if (!widepath) return set_error(direntry_invalid(), FileError::NonUcsPath);

        widepath.get()[ucslen + 1] = L'\0';
        widepath.get()[ucslen - 1] = L'/';
        widepath.get()[ucslen] = L'*';

        HANDLE hSearch = FindFirstFileW(widepath.get(), data.get());
        if (hSearch == INVALID_HANDLE_VALUE) {
            if (GetLastError() == ERROR_FILE_NOT_FOUND) {
                return set_error(direntry_invalid(), FileError::NotExist);
            }
            else {
                return set_error(direntry_invalid(), FileError::UnknownError);
            }
        }

        return DirEntry{ hSearch, data.reset() };
    }

    // ------------------------------------------------------------------------
    bool direntry_next(Ref<DirEntry> entry)
    {
        if (entry->data == nullptr || entry->handle == INVALID_HANDLE_VALUE) {
            return set_error(nullptr, FileError::InvalidArgs);
        }

        auto data = (WIN32_FIND_DATAW*)entry->data;
        BOOL result = FindNextFileW(entry->handle, data);
        return result == TRUE;
    }

    // ------------------------------------------------------------------------
    void direntry_end(Ref<DirEntry> entry)
    {
        if (entry->handle != INVALID_HANDLE_VALUE) {
            FindClose(entry->handle);
            entry->handle = INVALID_HANDLE_VALUE;
        }
        if (entry->data) {
            free(entry->data);
            entry->data = nullptr;
        }
    }

    // ------------------------------------------------------------------------
    bool is_directory(DirEntry const& entry)
    {
        if (entry.data == nullptr) {
            return set_error(nullptr, FileError::InvalidArgs);
        }

        auto data = (WIN32_FIND_DATAW*)entry.data;
        bool isDir = (data->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
        return isDir;
    }

    // ------------------------------------------------------------------------
    size_t get_filename(DirEntry const& entry, Array<Byte> buffer)
    {
        if (entry.data == nullptr) {
            filesystem_error = FileError::InvalidArgs;
            return 0;
        }

        auto data = (WIN32_FIND_DATAW*)entry.data;
        return WideCharToMultiByte(CP_UTF8, 0, data->cFileName, -1, (LPSTR)buffer.begin, (int)size(buffer), NULL, NULL);
    }

} // namespace Native