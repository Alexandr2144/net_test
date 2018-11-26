#pragma once
#ifdef CLIENT

#include "core/data/string.h"
#include "net_host.h"


class NetProtocolGameClient {
public:
    void bind_reliable(NetHost& host);
    void bind_unreliable(NetHost& host);

    void load(NetConnection& ctx, Data::String level);
    void onLevelLoaded();

    void addDoll(NetConnection& ctx);
    void updateDoll(NetConnection& ctx);

private:
    NetEvent<void> sendOnLevelLoaded;

    NetPeerId m_server;
    Data::String m_level;
};

#endif // CLIENT