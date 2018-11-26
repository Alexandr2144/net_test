#pragma once
#ifndef DATA_HASH_INDEX_H
#define DATA_HASH_INDEX_H

#include "core/memory/common.h"
#include "core/data/array.h"


namespace Data {
    struct HashIndex {
        enum class InsertStatus { Yes, Swap, No };
        struct Record {
            size_t factor;
            size_t id;
        };

        Array<Record> table;
        size_t depth;

    public:
        HashIndex();
        HashIndex(Array_CRef<Record> storage, bool direct = false);

        InsertStatus can_insert(size_t entry, size_t place) const;
        ArrayRange spot(size_t entry) const;

        size_t insert(size_t hashval, size_t id);
        bool rehash(HashIndex const& index, SparseArray<size_t> hashes);

        bool is_used(size_t place) const;
    };
    using HashIndex_CRef = HashIndex const&;


    struct HashIndexSerializer {
        enum class Error { NoErrors, Count, Table, Hashes };

        SparseArray<size_t> data;
        HashIndex index;
        Error error;

        bool serialize(Ref<Bytes> outBytes, Memory::IAllocator& allocator);
        bool deserialize(CBytes src, Memory::IAllocator& metaAlloc, Memory::IAllocator& dataAlloc);

    public:
        static HashIndexSerializer forSerialize(SparseArray<size_t> hashes, HashIndex_CRef index);
        static HashIndexSerializer forDeserialize(MemberPointer<size_t> hash);

    private:
        HashIndexSerializer() {}
        bool throw_error(Error err) { error = err; return false; }
    };

} // namespace Data


#endif // DATA_HASH_INDEX_H