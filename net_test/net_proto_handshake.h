#pragma once

#include "core/tools/event.h"
#include "net_host.h"


class NetProtocolHandshake {
public:
    Tools::Event<NetConnection&> onConnected;

    void bind(NetHost& host);

    void connect(NetPeerId peer, NetEventNames const& names);
    void request(NetConnection& conn, NetEventNames events);
    void finalize(NetConnection& conn);

private:
    NetEvent<NetEventNames> doRequest;
    NetEvent<void> onAccepted;
};