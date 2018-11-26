#pragma once

#include <stdint.h>


namespace NetAddress {
    struct Storage {
        uint64_t host;
        uint16_t port;
    };

    extern Storage any;

    Storage ipv6(char const* addr);
    Storage ipv6(char const* addr, uint16_t port);

    Storage ipv4(char const* addr);
    Storage ipv4(char const* addr, uint16_t port);

} // namespace NetAddress