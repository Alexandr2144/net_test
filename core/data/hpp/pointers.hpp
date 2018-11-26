#include "core/data/pointers.h"
#include <new>


namespace Data {
    // ---------------------------------------------------------------------------------------------
    template <class T>
    template <class... ArgsTy>
    void Plain<T>::make(ArgsTy&&... args)
    {
        new(bytes) T(std::forward<ArgsTy>(args)...);
    }

    // ---------------------------------------------------------------------------------------------
    template <class T>
    void Plain<T>::destroy()
    {
        ptr()->~T();
    }

    // ---------------------------------------------------------------------------------------------
    template <class T>
    T& ref(T* ptr)
    {
        M_ASSERT_MSG(ptr != nullptr, "Try get reference from NULL");
        return *ptr;
    }

} // namespace Data