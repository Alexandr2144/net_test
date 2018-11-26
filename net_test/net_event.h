#pragma once

#include "core/data/array.h"
#include "net_transport.h"


using Data::Array;
using Data::Bytes;


template <class... ArgsTy>
class NetEvent {
public:
    NetEvent() : m_peers(nullptr), m_handler(0), m_entry(0) {}
    NetEvent(Memory::RaPool<NetPeer> const& peers, size_t entry, size_t handler)
        : m_peers(&peers), m_handler(handler), m_entry(entry) {}

    template <class... TailTy>
    void operator()(NetPeerId const& peer, TailTy&&... args);

private:
    Memory::RaPool<NetPeer> const* m_peers;
    size_t m_handler;
    size_t m_entry;
};

template <>
class NetEvent<void>
    : public NetEvent<>
{
public:
    NetEvent() : NetEvent<>() {}
    NetEvent(NetEvent<> const& e) : NetEvent<>(e) {}
    NetEvent(Memory::RaPool<NetPeer> const& peers, size_t entry, size_t handler)
        : NetEvent<>(peers, entry, handler) {}
};


class NetEventProxy {
public:
    NetEventProxy(Memory::RaPool<NetPeer> const& peers, size_t entry, size_t handler)
        : m_entry(entry), m_handler(handler), m_peers(peers) {}

    template <class... ArgsTy>
    operator NetEvent<ArgsTy...>() { return NetEvent<ArgsTy...>(m_peers, m_entry, m_handler); }

private:
    Memory::RaPool<NetPeer> const& m_peers;
    size_t m_handler;
    size_t m_entry;
};

#include "net_event.hpp"