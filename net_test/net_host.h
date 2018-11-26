#pragma once

#include "core/memory/containers.h"
#include "core/data/array.h"

#include "net_event_names.h"
#include "net_address.h"
#include "net_handler.h"
#include "net_event.h"

#include "enet/enet.h"


using Data::Bytes;
using Data::Array;


struct NetHostState {
    Memory::RaStack<NetHandlerIface*> handlers;
    Memory::RaPool<NetPeer> peers;
    NetEventNames names;
};


class NetEventResolver {
public:
    NetEventResolver(NetHostState& state, NetPeerId peer, size_t channel);

    NetEventResolver& name(char const* str);
    NetEventResolver& index(size_t idx);
    NetEventProxy get() const;

private:
    NetHostState& m_state;

    NetNameResolver m_resolver;
    size_t m_channel;
};


class NetConnection {
public:
    NetConnection(NetHostState& state, NetPeerId source);

    NetEventNamesRef getNames(NetPeerId peer) const;
    void setNames(NetPeerId peer, NetEventNamesRef names) const;

    NetPeerId peer(size_t index) const;
    NetPeerId source() const;
    size_t peers() const;

    NetEventResolver event(size_t channel) const;

private:
    NetHostState& m_state;
    NetPeerId m_source;
};


class NetHost {
public:
    M_DECL_MOVE_ONLY(NetHost);

    Memory::ChainAllocator allocator;
    std::function<void(NetPeerId, NetEventNames const&)> onConnected;

    NetHost(char const* dbgname);
    ~NetHost();

    template <class... ArgsTy>
    NetEvent<ArgsTy...> addAnonymous(size_t channel, NetHandler<ArgsTy...>* handler);
    NetHandlerBuilder addHandler(NetHandlerIface* handler);

    bool listen(size_t maxPeers, NetAddress::Storage address);
    bool connect(NetAddress::Storage address);

    void update();

private:
    char const* m_dbgname;

    NetHostState m_state;
    ENetHost* m_enetHost;
    size_t m_nonce;

    void addPeer(ENetPeer* peer);
    void delPeer(ENetPeer* peer);
    void receive(ENetPeer* peer, ENetPacket* packet);
    void send();
};


template <class ClsTy, class... ArgsTy>
NetEvent<ArgsTy...>
    addAnonymous(NetHost& host, size_t channel, ClsTy* pThis, void(ClsTy::*handler)(NetConnection&, ArgsTy...));

template <class ClsTy, class... ArgsTy>
NetHandlerBuilder
    addHandler(NetHost& host, ClsTy* pThis, void(ClsTy::*handler)(NetConnection&, ArgsTy...));


#include "net_host.hpp"