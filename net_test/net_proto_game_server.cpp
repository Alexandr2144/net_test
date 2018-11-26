#ifdef SERVER

#include "net_proto_game_server.h"


// ----------------------------------------------------------------------------
// NetProtocolGameServer implementation
// ----------------------------------------------------------------------------
void NetProtocolGameServer::bind_reliable(NetHost& host)
{
    addHandler(host, this, &NetProtocolGameServer::onLevelLoaded)
        .name("GameServer").name("onLevelLoaded");
}

// ----------------------------------------------------------------------------
void NetProtocolGameServer::bind_unreliable(NetHost& host)
{
}

// ----------------------------------------------------------------------------
void NetProtocolGameServer::onConnected(NetConnection& conn)
{
    load = conn.event(0).name("GameClient").name("load").get();
    addDoll = conn.event(0).name("GameClient").name("addDoll").get();
    updateDoll = conn.event(0).name("GameClient").name("updateDoll").get();

    printf("game server> connected new client (nonce: %zd)\n", conn.source().nonce);
    load(conn.source(), level);
}

// ----------------------------------------------------------------------------
void NetProtocolGameServer::onLevelLoaded(NetConnection& conn)
{
    printf("game server> request to add new doll\n");
    addDoll(conn.source());
}

#endif // SERVER