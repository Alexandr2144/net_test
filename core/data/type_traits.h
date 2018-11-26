#pragma once
#ifndef DATA_TYPE_TRAITS_H
#define DATA_TYPE_TRAITS_H


namespace Data {
    template <class T>
    struct AlwaysFalse {
        static constexpr bool value = false;
    };

} // namespace Data


#endif // DATA_TYPE_TRAITS_H