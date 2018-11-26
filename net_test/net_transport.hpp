#include "net_transport.h"



// ----------------------------------------------------------------------------
template <class T>
template <class ElemTy>
void NetSerializer<Array<T>>::pack(Memory::RegBuffer& data, Array<ElemTy> const& value)
{
    uint32_t count = (uint32_t)Data::count(value);

    write(&data.reserve(sizeof(uint32_t)), count);
    for (ElemTy const& elem : Data::iterate(value)) {
        NetSerializer<T>::pack(data, elem);
    }
}

// ----------------------------------------------------------------------------
template <class T>
template <class ElemTy>
Data::Array<ElemTy> NetSerializer<Array<T>>::unpack(Memory::IAllocator& alloc, CBytes& data)
{
    uint32_t strlen;
    read(&data, &strlen);

    Array<ElemTy> result = Tools::newArray<ElemTy>(&alloc, strlen);
    for (ElemTy& elem : Data::iterate(result)) {
        elem = NetSerializer<T>::unpack(alloc, data);
    }

    return result;
}

// ----------------------------------------------------------------------------
template <class K, class V>
void NetSerializer<Serializer::HashMapPair<K, V>>::pack(Memory::RegBuffer& data, Row const& value)
{
    NetSerializer<K>::pack(data, value.key);
    NetSerializer<V>::pack(data, value.value);
}

// ----------------------------------------------------------------------------
template <class K, class V>
auto NetSerializer<Serializer::HashMapPair<K, V>>::unpack(Memory::IAllocator& alloc, CBytes& data) -> Row
{
    Row row;
    row.key = std::move(NetSerializer<K>::unpack(alloc, data));
    row.value = std::move(NetSerializer<V>::unpack(alloc, data));
    return row;
}