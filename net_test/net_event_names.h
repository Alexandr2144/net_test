#pragma once

#include "core/memory/containers.h"
#include "net_transport.h"


class NetHandlerIface;


class NetNameResolver {
    using Entry = NetEventNames::Entry;
    using Group = NetEventNames::Group;
public:
    NetNameResolver(NetEventNames const& names);

    NetNameResolver& name(char const* str);
    NetNameResolver& index(size_t idx);
    size_t get() const;

private:
    NetEventNames const& m_names;
    Entry m_entry;
};


class NetHandlerBuilder {
    using Entry = NetEventNames::Entry;
    using Group = NetEventNames::Group;
public:
    size_t handler;

    NetHandlerBuilder(NetEventNames& names, size_t handler);

    NetHandlerBuilder& name(char const* str);
    NetHandlerBuilder& index(size_t idx);

private:
    NetEventNames& m_names;
    size_t m_group;
    Entry m_entry;
};