#include "net_transport.h"


using namespace Data;


// ----------------------------------------------------------------------------
// NetEventNames implementation
// ----------------------------------------------------------------------------
NetEventNames::NetEventNames(bool forSerialization)
{
    if (!forSerialization) {
        groups.append(Group());
    }
}

// ----------------------------------------------------------------------------
// NetPeer implementation
// ----------------------------------------------------------------------------
NetPeer::NetPeer(size_t nonce, size_t buffers)
    : nonce(nonce)
{ 
    output.data = Tools::buildArray<Memory::RegBuffer>(nullptr, buffers);
}

// ----------------------------------------------------------------------------
NetPeer::~NetPeer()
{
    nonce = -1;
    Tools::destroyArray(output.data);
}

// ----------------------------------------------------------------------------
// NetPeerId implementation
// ----------------------------------------------------------------------------
NetPeerId::NetPeerId()
    : NetPeerId(-1, -1)
{
}

// ----------------------------------------------------------------------------
NetPeerId::NetPeerId(size_t index, size_t nonce)
    : index(index), nonce(index)
{
}

// ----------------------------------------------------------------------------
size_t NetPeerId::GenNonce(size_t& lastNonce)
{
    size_t nonce = lastNonce;
    if (++lastNonce == -1) {
        lastNonce = 0;
    }
    return nonce;
}

// ----------------------------------------------------------------------------
bool NetPeerId::isValid() const
{
    return nonce != -1;
}


// ----------------------------------------------------------------------------
void NetSerializer<String>::pack(Memory::RegBuffer& data, String const& value)
{
    uint32_t strlen = (uint32_t)count(value);
    write(&data.reserve(sizeof(uint32_t)), strlen);
    write(&data.reserve(strlen), (CBytes)value);
}

// ----------------------------------------------------------------------------
String NetSerializer<String>::unpack(Memory::IAllocator& a, CBytes& data)
{
    uint32_t strlen;
    read(&data, &strlen);

    Bytes str = Tools::newArray<Byte>(&a, strlen);
    read(&data, str);

    return String(str);
}

// ----------------------------------------------------------------------------
void NetSerializer<NetEventNames::Entry>::pack(Memory::RegBuffer& data, Entry const& value)
{
    write(&data.reserve(sizeof(uint32_t)), (uint32_t)value.index);
    write(&data.reserve(sizeof(uint32_t)), (uint32_t)value.type);
}

// ----------------------------------------------------------------------------
auto NetSerializer<NetEventNames::Entry>::unpack(Memory::IAllocator& alloc, CBytes& data) -> Entry
{
    uint32_t index, type;
    read(&data, &index);
    read(&data, &type);

    Entry entry;
    entry.index = index;
    entry.type = (Entry::Type)type;
    return entry;
}

// ----------------------------------------------------------------------------
void NetSerializer<NetEventNames::Group>::pack(Memory::RegBuffer& data, Group const& value)
{
    using Pair = Serializer::HashMapPair<String, Entry>;
    NetSerializer<Array<Pair>>::pack<Row>(data, value.entries.rows);
}

// ----------------------------------------------------------------------------
auto NetSerializer<NetEventNames::Group>::unpack(Memory::IAllocator& alloc, CBytes& data) -> Group
{
    using Pair = Serializer::HashMapPair<String, Entry>;

    Group group;
    auto rows = NetSerializer<Array<Pair>>::unpack<Row>(alloc, data);
    for (auto const& row : iterate(rows)) {
        group.entries.insert(row.key, row.value);
    }
    return group;
}

// ----------------------------------------------------------------------------
void NetSerializer<NetEventNames>::pack(Memory::RegBuffer& data, NetEventNamesRef const& value)
{
    NetSerializer<Array<Group>>::pack(data, value.groups);
}

// ----------------------------------------------------------------------------
NetEventNames NetSerializer<NetEventNames>::unpack(Memory::IAllocator& alloc, CBytes& data)
{
    auto groups = NetSerializer<Array<Group>>::unpack<Group>(alloc, data);

    NetEventNames names(true);
    names.groups.insert(groups);
    return std::move(names);
}