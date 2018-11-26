#pragma once

#include "core/data/type_traits.h"
#include "core/data/hash_map.h"
#include "core/data/string.h"
#include "core/data/array.h"

#include "enet/enet.h"


using Data::Array;
using Data::Bytes;
using Data::CBytes;


struct NetPacketIn {
    Bytes data;
};

struct NetPacketOut {
    Array<Memory::RegBuffer> data;
};


struct NetEventNames {
    struct Entry {
        enum Type { Handler, Scope, List } type;
        size_t index;
    };
    struct Group {
        Data::HashMap<Data::String, Entry> entries;
    };
public:
    Memory::RaStack<Group> groups;

public:
    NetEventNames(bool forSerialization = false);
};

struct NetEventNamesRef {
    Data::Array<NetEventNames::Group> groups;

public:
    NetEventNamesRef(NetEventNames const& names)
        : groups(names.groups.asArray()) {}
};


struct NetPeerId {
    size_t index;
    size_t nonce;

public:
    NetPeerId();
    NetPeerId(size_t index, size_t nonce);

    static size_t GenNonce(size_t& lastNonce);
    bool isValid() const;
};

struct NetPeer {
    size_t nonce;

    Memory::RegBuffer dataIn;
    Memory::RegBuffer dataOut;

    NetPacketIn input;
    NetPacketOut output;
    NetEventNames names;

    ENetPeer* enetPeer;

public:
    NetPeer(size_t nonce, size_t buffers);
    ~NetPeer();

    bool isValid() const { return nonce != -1; }
};


namespace Serializer {
    struct Plain {};

    struct Int32 {};
    struct Int64 {};

    struct String {};
    struct ZtString {};

    template <class K, class V>
    struct HashMapRow {};

    template <class K, class V>
    struct HashMapPair {};
}


template <class T>
struct NetSerializer {
    static_assert(Data::AlwaysFalse<T>::value, "NetSerializer not instanced for this type");

    static void pack(Memory::RegBuffer& data, T const& value);
    static T unpack(Memory::IAllocator& alloc, CBytes& data);
};


// Array serialization
template <class T>
struct NetSerializer<Array<T>> {
    template <class ElemTy>
    static void pack(Memory::RegBuffer& data, Array<ElemTy> const& value);

    template <class ElemTy>
    static Array<ElemTy> unpack(Memory::IAllocator& alloc, CBytes& data);
};

// Hash map serialization
template <class K, class V>
struct NetSerializer<Serializer::HashMapPair<K, V>> {
    using Row = typename Data::CHashMap<K, V>::Row;

    static void pack(Memory::RegBuffer& data, Row const& value);
    static Row unpack(Memory::IAllocator& alloc, CBytes& data);
};

// CNetEventNames serialization
template <>
struct NetSerializer<NetEventNames::Entry> {
    using Entry = NetEventNames::Entry;

    static void pack(Memory::RegBuffer& data, Entry const& value);
    static Entry unpack(Memory::IAllocator& alloc, CBytes& data);
};

template <>
struct NetSerializer<NetEventNames::Group> {
    using Group = NetEventNames::Group;
    using Entry = NetEventNames::Entry;
    using Row = Data::HashMap<Data::String, Entry>::Row;

    static void pack(Memory::RegBuffer& data, Group const& value);
    static NetEventNames::Group unpack(Memory::IAllocator& alloc, CBytes& data);
};

template <>
struct NetSerializer<NetEventNames> {
    using Group = NetEventNames::Group;
    using Entry = NetEventNames::Entry;

    static void pack(Memory::RegBuffer& data, NetEventNamesRef const& value);
    static NetEventNames unpack(Memory::IAllocator& alloc, CBytes& data);
};

 // Strings serialization
template <>
struct NetSerializer<Data::String> {
    static void pack(Memory::RegBuffer& data, Data::String const& value);
    static Data::String unpack(Memory::IAllocator& alloc, CBytes& data);
};


#include "net_transport.hpp"