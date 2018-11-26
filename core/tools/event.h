#pragma once
#ifndef TOOLS_EVENT_H
#define TOOLS_EVENT_H

#include "core/memory/containers.h"
#include <functional>


namespace Tools {
    template <class... ArgsTy>
    struct Event {
    public:
        using Handler = std::function<void(ArgsTy...)>;

        void operator()(ArgsTy&&... args);
        Event& operator+=(Handler const& handler);

    private:
        Memory::RaStack<Handler> m_handlers;
    };

    template <>
    struct Event<void>
        : public Event<>
    {
    };

} // namespace Tools


#include "hpp/event.hpp"

#endif // TOOLS_EVENT_H