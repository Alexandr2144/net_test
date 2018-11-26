#include "core/data/threading.h"


namespace Data {
    // ------------------------------------------------------------------------
    // MutexGuard implementation
    // ------------------------------------------------------------------------
    MutexGuard::MutexGuard(Native::Mutex mutex)
        : m_mutex(mutex)
    {
        Native::mutex_lock(m_mutex);
    }

    // ------------------------------------------------------------------------
    MutexGuard::~MutexGuard()
    {
        Native::mutex_unlock(m_mutex);
    }

    // ------------------------------------------------------------------------
    MutexGuard::MutexGuard(MutexGuard&& guard)
        : m_mutex(guard.m_mutex)
    {
        guard.m_mutex = Native::Mutex();
    }

    // ------------------------------------------------------------------------
    MutexGuard& MutexGuard::operator=(MutexGuard&& guard)
    {
        new(this) MutexGuard(std::forward<MutexGuard>(guard));
        return *this;
    }

    // ------------------------------------------------------------------------
    // Mutex implementation
    // ------------------------------------------------------------------------
    Mutex::Mutex(char const* name)
        : m_mutex(Native::mutex_init(name))
    {
    }

    // ------------------------------------------------------------------------
    Mutex::~Mutex()
    {
        Native::mutex_destroy(m_mutex);
    }

    // ------------------------------------------------------------------------
    void Mutex::lock()
    {
        Native::mutex_lock(m_mutex);
    }

    // ------------------------------------------------------------------------
    void Mutex::unlock()
    {
        Native::mutex_unlock(m_mutex);
    }

    // ------------------------------------------------------------------------
    MutexGuard Mutex::guard()
    {
        return MutexGuard(m_mutex);
    }

} // namespace Data