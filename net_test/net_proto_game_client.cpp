#ifdef CLIENT
#include "net_proto_game_client.h"

#include "core/memory/string.h"
#include <stdio.h>


// ----------------------------------------------------------------------------
// NetProtocolGameClient implementation
// ----------------------------------------------------------------------------
void NetProtocolGameClient::bind_reliable(NetHost& host)
{
    addHandler(host, this, &NetProtocolGameClient::load)
        .name("GameClient").name("load");

    addHandler(host, this, &NetProtocolGameClient::addDoll)
        .name("GameClient").name("addDoll");
}

// ----------------------------------------------------------------------------
void NetProtocolGameClient::bind_unreliable(NetHost& host)
{
    addHandler(host, this, &NetProtocolGameClient::updateDoll)
        .name("GameClient").name("updateDoll");
}

// ----------------------------------------------------------------------------
void NetProtocolGameClient::load(NetConnection& conn, Data::String level)
{
    sendOnLevelLoaded = conn.event(0).name("GameServer").name("onLevelLoaded").get();

    m_level = level;
    m_server = conn.source();
    printf("game client> load level: %s\n", Memory::ZtString(level).cstr());

    onLevelLoaded();
}

// ----------------------------------------------------------------------------
void NetProtocolGameClient::onLevelLoaded()
{
    sendOnLevelLoaded(m_server);
}

// ----------------------------------------------------------------------------
void NetProtocolGameClient::addDoll(NetConnection& ctx)
{
    // create doll entity
    printf("game client> add doll\n");
}

// ----------------------------------------------------------------------------
void NetProtocolGameClient::updateDoll(NetConnection& ctx)
{
    // update doll entity
    printf("game client> update doll\n");
}

#endif // CLIENT