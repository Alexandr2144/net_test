#pragma once
#ifndef FILESYSTEM_NATIVE_LINK_H
#define FILESYSTEM_NATIVE_LINK_H

#include "core/filesystem/common.h"


namespace Filesystem {
    class NativeLink
        : public IMountPoint
    {
    public:
        M_DECL_MOVE_ONLY(NativeLink);

        NativeLink(char const* prefix);

        virtual Memory::ZtString path(char const* relative) const override;

        virtual bool find_next(Ref<DirEntry> entry) override;
        virtual DirEntry find_first(char const* path) override;

        virtual Open open(char const* path, char const* attr) override;

    private:
        Data::String m_prefix;
    };


    Data::String extension(Data::String filename);
    Data::String basename(String filename);

} // namespace Filesystem


#endif // FILESYSTEM_NATIVE_LINK_H