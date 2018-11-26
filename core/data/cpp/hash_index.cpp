#include "core/data/hash_index.h"
#include "core/math/common.h"

#include "core/tools/utils.h"


namespace Data {
    // ------------------------------------------------------------------------
    template <class T>
    static void xorSwap(T& a, T& b)
    {
        a ^= b ^= a ^= b;
    }

    // ------------------------------------------------------------------------
    static size_t getClosestPrime(size_t value)
    {
        static const size_t primes[] = {
            0, 2, 3, 7, 13, 31, 61, 127,
            251,  509,  1021,  2039,
            4093, 8191, 16381, 32749,
            65521,
        };

        size_t power = (size_t)Math::log2((int)value);
        if (power > 16) return 0;

        return primes[power];
    }

    // ------------------------------------------------------------------------
    static Array<HashIndex::Record> truncateStorage(Array<HashIndex::Record> storage)
    {
        size_t elemsCount = getClosestPrime(count(storage));
        return slice(storage, 0, elemsCount);
    }

    // ------------------------------------------------------------------------
    static void fillStorage(Array<HashIndex::Record> storage)
    {
        for (auto& record : iterate(storage)) {
            record.factor = SIZE_MAX;
        }
    }

    // ------------------------------------------------------------------------
    HashIndex::HashIndex()
        : table{ nullptr, nullptr }
        , depth(0)
    {
    }

    // ------------------------------------------------------------------------
    HashIndex::HashIndex(Array_CRef<Record> storage, bool direct)
        : table(direct ? storage : truncateStorage(storage))
    {
        if (!direct) fillStorage(storage);

        size_t length = Data::count(storage);
        depth = Math::max(2, Math::log2((int)length) - 4);
    }

    // ------------------------------------------------------------------------
    HashIndex::InsertStatus HashIndex::can_insert(size_t entry, size_t place) const
    {
        HashIndex::Record record = table[place];
        if (record.factor == SIZE_MAX) return InsertStatus::Yes;

        size_t factor = place - entry;
        if (record.factor > factor) return InsertStatus::Swap;

        return InsertStatus::No;
    }

    // ------------------------------------------------------------------------
    ArrayRange HashIndex::spot(size_t entry) const
    {
        size_t length = Data::count(table);
        size_t end = Math::min(entry + depth, length);
        return ArrayRange{ entry, end };
    }

    // ------------------------------------------------------------------------
    size_t HashIndex::insert(size_t hashval, size_t id)
    {
        size_t entry = hashval % count(table);

        InsertStatus stat = InsertStatus::Swap;
        while (stat == InsertStatus::Swap) {
            ArrayRange entries = spot(entry);
            for (size_t place = entries.begin; place != entries.end; ++place) {
                stat = can_insert(entry, place);
                if (stat != InsertStatus::No) {
                    HashIndex::Record& record = table[place];

                    xorSwap(record.id, id);
                    xorSwap(record.factor, entry);

                    record.factor = place - record.factor;
                    entry = place - entry;
                    break;
                }
            }
        }

        if (stat == InsertStatus::Yes) {
            return SIZE_MAX;
        }
        else {
            return id;
        }
    }

    // ------------------------------------------------------------------------
    bool HashIndex::rehash(HashIndex const& index, SparseArray<size_t> hashes)
    {
        for (auto& record : iterate(index.table)) {
            if (record.factor == SIZE_MAX) continue;

            size_t hashval = hashes[record.id];
            size_t displaced = insert(hashval, record.id);
            if (displaced != SIZE_MAX) return false;
        }
        return true;
    }

    // ------------------------------------------------------------------------
    bool HashIndex::is_used(size_t place) const
    {
        HashIndex::Record record = table[place];
        return record.factor != SIZE_MAX;
    }

    // ------------------------------------------------------------------------
    // HashIndex serialization
    // ------------------------------------------------------------------------
    HashIndexSerializer HashIndexSerializer::forSerialize(SparseArray<size_t> hashes, HashIndex_CRef index)
    {
        HashIndexSerializer serializer;
        serializer.index = index;
        serializer.data = hashes;
        return serializer;
    }

    // ------------------------------------------------------------------------
    HashIndexSerializer HashIndexSerializer::forDeserialize(MemberPointer<size_t> hash)
    {
        HashIndexSerializer serializer;
        serializer.data.offset = hash.offset;
        serializer.data.stride = hash.stride;
        return serializer;
    }

    // ------------------------------------------------------------------------
    bool HashIndexSerializer::serialize(Ref<Bytes> outBytes, Memory::IAllocator& allocator)
    {
        size_t elemsCount = count(data);
        size_t totalSize = 8 + size(index.table) + 8 * elemsCount;

        Bytes dest = allocator.alloc(totalSize);
        outBytes.ref = dest;

        if (!write(&dest, toBytes(&elemsCount))) return throw_error(Error::Count);
        if (!write(&dest, toBytes(index.table))) return throw_error(Error::Table);

        for (size_t& hashval : iterate(data)) {
            if (!write<uint64_t>(&dest, hashval)) return throw_error(Error::Hashes);
        }
        return true;
    }

    // ------------------------------------------------------------------------
    bool HashIndexSerializer::deserialize(CBytes src, Memory::IAllocator& metaAlloc, Memory::IAllocator& dataAlloc)
    {
        uint64_t count64;
        if (!read(&src, &count64)) return throw_error(Error::Count);

        size_t count = narrow_cast<size_t>(M_CL, count64);
        auto table = Tools::newArray<HashIndex::Record>(&metaAlloc, count);
        if (!can_split(src, size(table))) return throw_error(Error::Table);

        bytecopy(toBytes(table), split<CByte>(&src, size(table)));

        index = HashIndex(table, true);
        data = Tools::newSparseArray<size_t>(&dataAlloc, { data.offset, data.stride }, count);

        bytecopy(toBytes(index.table), split<CByte>(&src, size(index.table)));
        for (size_t& hashval : iterate(data)) {
            uint64_t hashval64;
            if (!read<uint64_t>(&src, &hashval64)) return throw_error(Error::Hashes);

            hashval = narrow_cast<size_t>(M_CL, hashval64);
        }
        return true;
    }

} // namespace Data