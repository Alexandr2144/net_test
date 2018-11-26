#include "native/threading.h"

#include <windows.h>


namespace Native {
    // ------------------------------------------------------------------------
    // Mutex implementation
    // ------------------------------------------------------------------------
    M_EXPORT Mutex mutex_invalid()
    {
        return Mutex{ INVALID_HANDLE_VALUE };
    }

    // ------------------------------------------------------------------------
    M_EXPORT Mutex mutex_init(char const* name)
    {
        Mutex mutex;
        mutex.handle = CreateMutex(nullptr, false, name);
        return mutex;
    }

    // ------------------------------------------------------------------------
    M_EXPORT int mutex_lock(Mutex mutex)
    {
        if (mutex.handle == INVALID_HANDLE_VALUE) return 0;
        return WaitForSingleObject(mutex.handle, INFINITE);
    }

    // ------------------------------------------------------------------------
    M_EXPORT int mutex_trylock(Mutex mutex)
    {
        if (mutex.handle == INVALID_HANDLE_VALUE) return 0;
        return -1;
    }

    // ------------------------------------------------------------------------
    M_EXPORT int mutex_unlock(Mutex mutex)
    {
        if (mutex.handle == INVALID_HANDLE_VALUE) return 0;
        return ReleaseMutex(mutex.handle);
    }

    // ------------------------------------------------------------------------
    M_EXPORT int mutex_destroy(Mutex mutex)
    {
        if (mutex.handle == INVALID_HANDLE_VALUE) return 0;
        return CloseHandle(mutex.handle);
    }

} // namespace Native