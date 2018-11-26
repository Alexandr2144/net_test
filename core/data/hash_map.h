#pragma once
#ifndef DATA_HASH_TABLE_H
#define DATA_HASH_TABLE_H

#include "core/data/hash_index.h"
#include "core/data/pointers.h"
#include "core/data/array.h"

#include "core/memory/containers.h"


namespace Data {
    template <class KeyTy, class ValTy>
    struct CHashMap {
        struct Row {
            size_t hash;
            KeyTy key;
            ValTy value;
        };

        Array<Row> rows;
        HashIndex index;

    public:
        ValTy* lookup(KeyTy const& key, size_t hash) const;
        ValTy* lookup(KeyTy const& key) const;

        size_t* ord(KeyTy const& key, size_t hash) const;
        size_t* ord(KeyTy const& key) const;
    };


    template <class KeyTy, class ValTy>
    struct HashMap : public CHashMap<KeyTy, ValTy> {
        Memory::RaStack<Row> storage;

    public:
        HashMap() = default;
        ~HashMap();

        HashMap(HashMap&& map);
        HashMap(HashMap const& map) = delete;
        HashMap& operator=(HashMap&& map);
        HashMap& operator=(HashMap const& map) = delete;

        template <class K>
        Row* alloc(K&& key);

        template <class K>
        Row* alloc(K&& key, size_t hashval);

        template <class K, class V>
        bool insert(K&& key, V&& value);

        template <class K, class V>
        bool insert(K&& key, V&& value, size_t hashval);

        bool extend();
    };

    template <class KeyTy, class ValTy>
    bool isEmpty(CHashMap<KeyTy, ValTy> const& map) { return isEmpty(map.rows); }

} // namespace Data


#include "hpp/hash_map.hpp"

#endif // DATA_HASH_TABLE_H