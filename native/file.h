#pragma once
#ifndef NATIVE_FILE_H
#define NATIVE_FILE_H

#include "core/data/pointers.h"
#include "core/data/array.h"
#include "native/common.h"

#define PATH_MAX size_t(260)


/*
    file_open attributes:
        + - file must be exist
        - - file must be not exist
        n - create new file or replace existed
        r - read access
        w - read and write access
        x - access for execution
        R - shared reading allowed
        W - shared writing allowed
    file_map attributes:
        r - read access
        w - read and write access
        x - access for execution
*/


namespace Native {
    using Data::Ref;

    enum class FileError {
        NoErrors, InvalidArgs, NonUcsPath, AlreadyExist, NotExist,
        AccessDenied, InvalidAligment, UnknownError
    };


    struct File { void* hFile; void* hMapping; };
    struct DirEntry { void* handle; void* data; };


    bool is_file_valid(File file);
    FileError file_error();
    
    File  file_invalid();
    File  file_open(char const* path, char const* attr);
    void* file_map(Ref<File> file, uint64_t offset, size_t size, char const* attr);

    bool file_close(File file);
    bool file_unmap(void* ptr);

    uint64_t file_size(File file);

    DirEntry direntry_invalid();
    DirEntry direntry_first(char const* path);
    bool     direntry_next(Ref<DirEntry> entry);
    void     direntry_end(Ref<DirEntry> entry);

    bool is_directory(DirEntry const& entry);
    size_t get_filename(DirEntry const& entry, Data::Array<Data::Byte> buffer);

} // namespace Native

#endif // NATIVE_FILE_H