#include "net_host.h"


// ----------------------------------------------------------------------------
template <class ClsTy, class... ArgsTy>
NetEvent<ArgsTy...> addAnonymous(NetHost& host, size_t channel, ClsTy* pThis, void(ClsTy::*handler)(NetConnection&, ArgsTy...))
{
    NetHandler<ArgsTy...>* iface = new NetHandler<ArgsTy...>(
        [pThis, handler](NetConnection& conn, ArgsTy&&... args) {
        (pThis->*handler)(conn, std::forward<ArgsTy>(args)...);
    }
    );
    return host.addAnonymous<ArgsTy...>(channel, iface);
}

// ----------------------------------------------------------------------------
template <class ClsTy, class... ArgsTy>
NetHandlerBuilder addHandler(NetHost& host, ClsTy* pThis, void(ClsTy::*handler)(NetConnection&, ArgsTy...))
{
    NetHandlerIface* iface = new NetHandler<ArgsTy...>(
        [pThis, handler](NetConnection& conn, ArgsTy&&... args) {
        (pThis->*handler)(conn, std::forward<ArgsTy>(args)...);
    }
    );
    return host.addHandler(iface);
}

// ----------------------------------------------------------------------------
template <class... ArgsTy>
NetEvent<ArgsTy...> NetHost::addAnonymous(size_t channel, NetHandler<ArgsTy...>* handler)
{
    auto isEmptyNamesTree = [](NetEventNames const& names) {
        if (names.groups.isEmpty()) return true;
        if (names.groups.count() > 1) return false;
        return Data::isEmpty(names.groups[0].entries);
    };
    M_ASSERT_MSG(isEmptyNamesTree(m_state.names), "Anonymous handlers cannot be added after named handler");

    size_t hid = m_state.handlers.append(handler);
    return NetEvent<ArgsTy...>(m_state.peers, channel, hid);
}