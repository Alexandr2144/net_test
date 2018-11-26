#include "net_proto_handshake.h"


// ----------------------------------------------------------------------------
// NetProtocolHandshake implementation
// ----------------------------------------------------------------------------
void NetProtocolHandshake::bind(NetHost& host)
{
    doRequest = addAnonymous(host, 0, this, &NetProtocolHandshake::request);
    onAccepted = addAnonymous(host, 0, this, &NetProtocolHandshake::finalize);

    using namespace std::placeholders;
    host.onConnected = std::bind(&NetProtocolHandshake::connect, this, _1, _2);
}

// ----------------------------------------------------------------------------
void NetProtocolHandshake::connect(NetPeerId peer, NetEventNames const& names)
{
    printf("handshake> request connection\n");
    doRequest(peer, names);
}

// ----------------------------------------------------------------------------
void NetProtocolHandshake::request(NetConnection& conn, NetEventNames names)
{
    printf("handshake> connection accepted\n");

    NetPeerId peer = conn.source();
    conn.setNames(peer, names);
    onAccepted(peer);
}

// ----------------------------------------------------------------------------
void NetProtocolHandshake::finalize(NetConnection& conn)
{
    printf("handshake> connection syncronized\n");
    onConnected(conn);
}