#include "net_address.h"

#include "enet/enet.h"


namespace NetAddress {
    Storage any = Storage{ 0, 0 };

    // ------------------------------------------------------------------------
    //Storage ipv6(char const* addr);

    // ------------------------------------------------------------------------
    //Storage ipv6(char const* addr, uint16_t port);

    // ------------------------------------------------------------------------
    //Storage ipv4(char const* addr);

    // ------------------------------------------------------------------------
    Storage ipv4(char const* addr, uint16_t port)
    {
        ENetAddress address;
        enet_address_set_host(&address, addr);
        return Storage{ address.host, port };
    }

} // namespace NetAddress