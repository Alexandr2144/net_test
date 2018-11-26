#include "core/tools/event.h"


namespace Tools {
    // ------------------------------------------------------------------------
    // Event implementation
    // ------------------------------------------------------------------------
    template <class... ArgsTy>
    void Event<ArgsTy...>::operator()(ArgsTy&&... args)
    {
        // TODO: WTF?!
        auto handlers = m_handlers.asArray();
        for (auto& handler : Data::iterate(handlers)) {
            handler(std::forward<ArgsTy>(args)...);
        }
    }

    // ------------------------------------------------------------------------
    template <class... ArgsTy>
    Event<ArgsTy...>& Event<ArgsTy...>::operator+=(Handler const& handler)
    {
        m_handlers.append(handler);
        return *this;
    }

} // namespace Tools