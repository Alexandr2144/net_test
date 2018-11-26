#include "net_event_names.h"

#include "core/data/string.h"


// ----------------------------------------------------------------------------
// NetNameResolver implementation
// ----------------------------------------------------------------------------
NetNameResolver::NetNameResolver(NetEventNames const& names)
    : m_names(names), m_entry{ Entry::Scope, 0 }
{
}

// ----------------------------------------------------------------------------
NetNameResolver& NetNameResolver::name(char const* str)
{
    M_ASSERT(m_entry.type == Entry::Scope);

    Group const& group = m_names.groups[m_entry.index];
    Entry const* entry = group.entries.lookup(str);

    if (entry == nullptr) {
        m_entry.type = Entry::Handler;
        m_entry.index = -1;
    }
    else {
        m_entry.type = entry->type;
        m_entry.index = entry->index;
    }
    return *this;
}

// ----------------------------------------------------------------------------
NetNameResolver& NetNameResolver::index(size_t idx)
{
    M_ASSERT(m_entry.type == Entry::List);

    Group const& group = m_names.groups[m_entry.index];
    Entry const& entry = group.entries.rows[idx].value;

    m_entry = entry;
    return *this;
}

// ----------------------------------------------------------------------------
size_t NetNameResolver::get() const
{
    M_ASSERT(m_entry.type == Entry::Handler);
    return m_entry.index;
}

// ----------------------------------------------------------------------------
// NetHandlerBuilder implementation
// ----------------------------------------------------------------------------
NetHandlerBuilder::NetHandlerBuilder(NetEventNames& names, size_t handler)
    : handler(handler), m_names(names), m_group(0), m_entry{ Entry::Scope, 0 }
{
}

// ----------------------------------------------------------------------------
NetHandlerBuilder& NetHandlerBuilder::name(char const* str)
{
    if (m_entry.type == Entry::List) {
        M_ASSERT_FAIL("Cannot add named entry to list");
    }
    if (m_entry.type == Entry::Handler) {
        Group const& group = m_names.groups[m_group];
        Entry& entry = group.entries.rows[m_entry.index].value;
        if (entry.index != handler) {
            M_ASSERT_FAIL("Cannot override handler");
        }

        entry.index = m_names.groups.append(Group());
        entry.type = Entry::Scope;
        m_group = entry.index;
        m_entry = entry;
    }

    Group& group = m_names.groups[m_group];

    Entry hEntry;
    hEntry.type = Entry::Handler;
    hEntry.index = handler;

    group.entries.insert(str, hEntry);
    m_entry.index = *group.entries.ord(str);

    Entry const& entry = group.entries.rows[m_entry.index].value;
    if (entry.type != Entry::Handler) {
        m_group = entry.index;
    }

    m_entry.type = entry.type;
    return *this;
}

// ----------------------------------------------------------------------------
NetHandlerBuilder& NetHandlerBuilder::index(size_t idx)
{
    return *this;
}