#include "core/data/hash_index.h"
#include "core/math/common.h"


namespace Data {
    // ------------------------------------------------------------------------
    // CHashMap implementation
    // ------------------------------------------------------------------------
    template <class KeyTy, class ValTy>
    size_t* CHashMap<KeyTy, ValTy>::ord(KeyTy const& key, size_t hashval) const
    {
        if (isEmpty(index.table)) return nullptr;

        size_t entry = hashval % count(index.table);
        ArrayRange range = index.spot(entry);

        for (size_t place = range.begin; place != range.end; ++place) {
            if (!index.is_used(place)) continue;

            HashIndex::Record& founded = index.table[place];
            if (rows[founded.id].key == key)
                return &founded.id;
        }

        return nullptr;
    }


    // ------------------------------------------------------------------------
    template <class KeyTy, class ValTy>
    size_t* CHashMap<KeyTy, ValTy>::ord(KeyTy const& key) const
    {
        return ord(key, Data::hash(key));
    }

    // ------------------------------------------------------------------------
    template <class KeyTy, class ValTy>
    ValTy* CHashMap<KeyTy, ValTy>::lookup(KeyTy const& key, size_t hashval) const
    {
        size_t* order = ord(key, hashval);
        if (order == nullptr) return nullptr;
        return &rows[*order].value;
    }

    // ------------------------------------------------------------------------
    template <class KeyTy, class ValTy>
    ValTy* CHashMap<KeyTy, ValTy>::lookup(KeyTy const& key) const
    {
        return lookup(key, Data::hash(key));
    }

    // ------------------------------------------------------------------------
    // HashMap implementation
    // ------------------------------------------------------------------------
    template <class KeyTy, class ValTy>
    HashMap<KeyTy, ValTy>::~HashMap()
    {
        Memory::buddy_global_heap.dealloc(index.table.begin);
    }

    // ------------------------------------------------------------------------
    template <class KeyTy, class ValTy>
    HashMap<KeyTy, ValTy>::HashMap(HashMap&& map)
        : CHashMap(std::forward<CHashMap>(map))
        , storage(std::move(map.storage))
    {
        map.index = HashIndex();
        map.rows = Array<Row>();
    }

    // ------------------------------------------------------------------------
    template <class KeyTy, class ValTy>
    HashMap<KeyTy, ValTy>& HashMap<KeyTy, ValTy>::operator=(HashMap&& map)
    {
        new(this) HashMap(std::forward<HashMap>(map));
        return *this;
    }

    // ------------------------------------------------------------------------
    template <class KeyTy, class ValTy>
    template <class K>
    auto HashMap<KeyTy, ValTy>::alloc(K&& key, size_t hashval) -> Row*
    {
        if (lookup(key, hashval) != nullptr) return nullptr;
        if (isEmpty(index.table)) extend();

        Row* newRow = storage.alloc();
        rows = storage.asArray();

        size_t id = index.insert(hashval, storage.last());
        while (id != SIZE_MAX) {
            if (!extend()) {
                M_ASSERT_FAIL("Hash table has overflowed");
                return nullptr;
            }

            id = index.insert(hashval, storage.last());
        }

        newRow->key = std::forward<K>(key);
        newRow->hash = hashval;
        return newRow;
    }

    // ------------------------------------------------------------------------
    template <class KeyTy, class ValTy>
    template <class K>
    auto HashMap<KeyTy, ValTy>::alloc(K&& key) -> Row*
    {
        return alloc(std::forward<K>(key), Data::hash(key));
    }

    // ------------------------------------------------------------------------
    template <class KeyTy, class ValTy>
    template <class K, class V>
    bool HashMap<KeyTy, ValTy>::insert(K&& key, V&& value, size_t hashval)
    {
        Row* row = alloc(std::forward<K>(key), hashval);
        if (row == nullptr) return false;

        row->value = std::forward<V>(value);
        return true;
    }

    // ------------------------------------------------------------------------
    template <class KeyTy, class ValTy>
    template <class K, class V>
    bool HashMap<KeyTy, ValTy>::insert(K&& key, V&& value)
    {
        return insert<K, V>(std::forward<K>(key), std::forward<V>(value), Data::hash(key));
    }

    // ------------------------------------------------------------------------
    template <class KeyTy, class ValTy>
    bool HashMap<KeyTy, ValTy>::extend()
    {
        SparseArray<size_t> hashes{ toBytes(rows), offsetof(Row, hash), sizeof(ValTy) };
        HashIndex newIndex;

        size_t newLength = Math::max(size_t(32), count(index.table) << 1);
        while (true) {
            auto table = Tools::newArray<HashIndex::Record>(nullptr, newLength);
            if (isEmpty(table)) return false;

            newIndex = HashIndex(table);
            if (count(newIndex.table) == 0) return false;

            bool rehashed = newIndex.rehash(index, hashes);
            if (rehashed) break;

            newLength <<= 1;
            Memory::buddy_global_heap.dealloc(table.begin);
        }

        Memory::buddy_global_heap.dealloc(index.table.begin);
        index = newIndex;
        return true;
    }

} // namespace Data