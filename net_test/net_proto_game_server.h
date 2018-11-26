#pragma once
#ifdef SERVER

#include "core/data/string.h"
#include "net_host.h"


class NetProtocolGameServer {
public:
    Data::String level;

    void bind_reliable(NetHost& host);
    void bind_unreliable(NetHost& host);

    void onConnected(NetConnection& conn);
    void onLevelLoaded(NetConnection& conn);

private:
    NetEvent<Data::String> load;

    NetEvent<void> addDoll;
    NetEvent<void> updateDoll;
};


#endif // SERVER