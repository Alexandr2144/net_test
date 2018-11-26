#include "net_event.h"


template <class... ArgsTy>
template <class... TailTy>
void NetEvent<ArgsTy...>::operator()(NetPeerId const& peerId, TailTy&&... args)
{
    using expand_type = int[];

    auto& peer = (*m_peers)[peerId.index];
    auto& output = peer.output.data[m_entry];

    uint32_t hid = static_cast<uint32_t>(m_handler);
    Data::write(&output.reserve(sizeof(uint32_t)), hid);

    expand_type{ 0, (NetSerializer<ArgsTy>::pack(output, args), 0)... };
}