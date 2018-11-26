#pragma once

#include "core/data/array.h"
#include "core/memory/buddy_heap.h"
#include <functional>


using Data::Array;
using Data::Bytes;

class NetConnection;


class NetHandlerIface {
public:
    virtual ~NetHandlerIface() = default;
    virtual void call(NetConnection& conn, CBytes& input) = 0;
};


template <class... ArgsTy>
class NetHandler
    : public NetHandlerIface
{
public:
    using Function = std::function<void(NetConnection& sender, ArgsTy...)>;
public:
    NetHandler(Function const& func) : NetHandler(nullptr, func) {}
    NetHandler(Memory::IAllocator* alloc, Function const& func) 
        : m_alloc(alloc ? *alloc : Memory::buddy_global_heap), m_func(func) {}

    virtual void call(NetConnection& conn, CBytes& input) override;

private:
    Memory::IAllocator& m_alloc;
    Function m_func;
};


struct NetHandlerInfo {
    char const* name;
    NetHandlerIface* handler;
};




// ----------------------------------------------------------------------------
template <class... ArgsTy>
void NetHandler<ArgsTy...>::call(NetConnection& conn, CBytes& input)
{
    m_func(conn, NetSerializer<ArgsTy>::unpack(m_alloc, input)...);
}