#pragma once
#ifndef DATA_THREADING_H
#define DATA_THREADING_H

#include "native/threading.h"

#include <atomic>


namespace Data {
    class MutexGuard {
    public:
        MutexGuard(Native::Mutex mutex);
        ~MutexGuard();

        MutexGuard(MutexGuard&& guard);
        MutexGuard(MutexGuard const& guard) = delete;
        MutexGuard& operator=(MutexGuard&& guard);
        MutexGuard& operator=(MutexGuard const& guard) = delete;

    private:
        Native::Mutex m_mutex;
    };

    class Mutex {
    public:
        Mutex(char const* name = nullptr);
        ~Mutex();

        void lock();
        void unlock();
        MutexGuard guard();

    private:
        Native::Mutex m_mutex;
    };

    using AtomicUint = std::atomic<size_t>;
    using AtomicInt = std::atomic<intptr_t>;

} // namespace Data

#endif // DATA_THREADING_H