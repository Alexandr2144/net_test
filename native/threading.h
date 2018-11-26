#pragma once
#ifndef NATIVE_THREADING_H
#define NATIVE_THREADING_H

#include "native/common.h"


namespace Native {
    struct Mutex { void* handle; };

    M_EXPORT Mutex mutex_invalid();
    M_EXPORT Mutex mutex_init(char const* name);

    M_EXPORT int mutex_lock(Mutex mutex);

    M_EXPORT int mutex_trylock(Mutex mutex);

    M_EXPORT int mutex_unlock(Mutex mutex);

    M_EXPORT int mutex_destroy(Mutex mutex);

} // namespace Native


#endif // NATIVE_THREADING_H