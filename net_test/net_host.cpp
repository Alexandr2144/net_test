#include "net_host.h"


using namespace Data;


// ----------------------------------------------------------------------------
// NetEventResolver implementation
// ----------------------------------------------------------------------------
NetEventResolver::NetEventResolver(NetHostState& state, NetPeerId peer, size_t channel)
    : m_state(state), m_channel(channel), m_resolver(state.peers[peer.index].names)
{
}

// ----------------------------------------------------------------------------
NetEventResolver& NetEventResolver::name(char const* str)
{
    m_resolver.name(str);
    return *this;
}

// ----------------------------------------------------------------------------
NetEventResolver& NetEventResolver::index(size_t idx)
{
    m_resolver.index(idx);
    return *this;
}


// ----------------------------------------------------------------------------
NetEventProxy NetEventResolver::get() const
{
    return NetEventProxy(m_state.peers, m_channel, m_resolver.get());
}

// ----------------------------------------------------------------------------
// NetConnection implementation
// ----------------------------------------------------------------------------
NetConnection::NetConnection(NetHostState& state, NetPeerId source)
    : m_state(state), m_source(source)
{
}

// ----------------------------------------------------------------------------
NetPeerId NetConnection::peer(size_t index) const
{
    return NetPeerId(index, m_state.peers[index].nonce);
}

// ----------------------------------------------------------------------------
NetPeerId NetConnection::source() const
{
    return m_source;
}

// ----------------------------------------------------------------------------
size_t NetConnection::peers() const
{
    return m_state.peers.count();
}

// ----------------------------------------------------------------------------
NetEventNamesRef NetConnection::getNames(NetPeerId peer) const
{
    return m_state.peers[peer.index].names;
}

// ----------------------------------------------------------------------------
void NetConnection::setNames(NetPeerId peer, NetEventNamesRef names) const
{
    m_state.peers[peer.index].names.groups.clear();
    m_state.peers[peer.index].names.groups.insert(names.groups);
}

// ----------------------------------------------------------------------------
NetEventResolver NetConnection::event(size_t channel) const
{
    return NetEventResolver(m_state, m_source, channel);
}

// ----------------------------------------------------------------------------
// NetHost implementation
// ----------------------------------------------------------------------------
NetHost::NetHost(char const* dbgname)
    : m_dbgname(dbgname)
{
}

// ----------------------------------------------------------------------------
NetHost::~NetHost()
{
    if (m_enetHost) {
        for (auto const& peer : iterate(m_state.peers.asArray())) {
            enet_peer_disconnect(peer.enetPeer, 0);
            enet_peer_reset(peer.enetPeer);
        }

        enet_host_destroy(m_enetHost);
    }
}

// ----------------------------------------------------------------------------
NetHandlerBuilder NetHost::addHandler(NetHandlerIface* handler)
{
    size_t hid = m_state.handlers.append(handler);
    return NetHandlerBuilder(m_state.names, hid);
}

// ----------------------------------------------------------------------------
bool NetHost::listen(size_t maxPeers, NetAddress::Storage address)
{
    ENetAddress enetAddr;
    enetAddr.host = (uint32_t)address.host;
    enetAddr.port = address.port;

    m_enetHost = enet_host_create(&enetAddr, maxPeers, 1, 0, 0);
    return (m_enetHost != nullptr);
}

// ----------------------------------------------------------------------------
bool NetHost::connect(NetAddress::Storage address)
{
    ENetAddress enetAddr;
    enetAddr.host = (uint32_t)address.host;
    enetAddr.port = address.port;

    ENetPeer* peer = enet_host_connect(m_enetHost, &enetAddr, 1, 0);
    return (peer != nullptr);
}

// ----------------------------------------------------------------------------
void NetHost::update()
{
    if (m_enetHost == nullptr) {
        return;
    }

    ENetEvent event;
    while (enet_host_service(m_enetHost, &event, 0)) {
        switch (event.type) {
        case ENetEventType::ENET_EVENT_TYPE_DISCONNECT:
            delPeer(event.peer);
            break;
        case ENetEventType::ENET_EVENT_TYPE_CONNECT:
            addPeer(event.peer);
            break;
        case ENetEventType::ENET_EVENT_TYPE_RECEIVE:
            receive(event.peer, event.packet);
            enet_packet_destroy(event.packet);
            break;
        }
    }

    send();
}

// ----------------------------------------------------------------------------
void NetHost::addPeer(ENetPeer* enetPeer)
{
    NetPeer* peer = m_state.peers.alloc();

    new(peer) NetPeer(NetPeerId::GenNonce(m_nonce), 1);
    peer->enetPeer = enetPeer;
    enetPeer->data = peer;

    NetPeerId pid(m_state.peers.index(peer), peer->nonce);
    onConnected(pid, m_state.names);
}

// ----------------------------------------------------------------------------
void NetHost::delPeer(ENetPeer* enetPeer)
{
    NetPeer* peer = static_cast<NetPeer*>(enetPeer->data);
    peer->~NetPeer();

    m_state.peers.dealloc(peer);
    enetPeer->data = nullptr;
}

// ----------------------------------------------------------------------------
void NetHost::receive(ENetPeer* enetPeer, ENetPacket* enetPacket)
{
    NetPeer* peer = static_cast<NetPeer*>(enetPeer->data);
    NetPeerId peerId(m_state.peers.index(peer), peer->nonce);
    NetConnection conn(m_state, peerId);

    // TODO: unpack input by transport protocols
    CBytes packet = toBytes(enetPacket->data, enetPacket->dataLength);
    while (true) {
        uint32_t hid;
        read(&packet, &hid);
        if (hid == UINT32_MAX) return;

        m_state.handlers[hid]->call(conn, packet);
    }
}

// ----------------------------------------------------------------------------
void NetHost::send()
{
    Array<NetPeer> peers = m_state.peers.asArray();
    for (NetPeer& peer : iterate(peers)) {
        Array<Memory::RegBuffer> buffers = peer.output.data;
        for (Memory::RegBuffer& data : iterate(buffers)) {
            if (data.size() == 0) continue;

            Bytes bytes = peer.dataOut.reserve(data.size());
            data.extract(bytes);
            data.reset();
        }
        if (peer.dataOut.size() == 0) {
            continue;
        }

        write(&peer.dataOut.reserve(sizeof(uint32_t)), UINT32_MAX);

        ENetPacket* packet = enet_packet_create(peer.dataOut.memory.begin, peer.dataOut.size(),
            ENET_PACKET_FLAG_RELIABLE | ENET_PACKET_FLAG_NO_ALLOCATE);
        enet_peer_send(peer.enetPeer, 0, packet);

        peer.dataOut.reset();
    }
}