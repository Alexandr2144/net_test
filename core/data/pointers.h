#pragma once
#ifndef DATA_POINTERS_H
#define DATA_POINTERS_H

#include "native/crash.h"


namespace Data {
    template <class T>
    struct MemberPointer {
        size_t offset;
        size_t stride;
    };

    // -------------------------------------------------------------------------
    // Plain<T> container, using for convert custom class object to POD
    // -------------------------------------------------------------------------
    template <class T>
    struct Plain {
        uint8_t bytes[sizeof(T)];

        template <class... ArgsTy>
        void make(ArgsTy&&... args);
        void destroy();

        T const* ptr() const { return (T const*)bytes; }
        T* ptr() { return (T*)bytes; }

        T const* operator->() const { return ptr(); }
        T* operator->() { return ptr(); }

        T const& operator*() const { return *ptr(); }
        T& operator*() { return *ptr(); }
    };

    // -------------------------------------------------------------------------
    // Ref<T> container, using instead T&
    // -------------------------------------------------------------------------
    template <class T>
    struct Ref {
        T& ref;

        T const* operator->() const { return &ref; }
        T const& operator*() const { return ref; }

        T* operator->() { return &ref; }
        T& operator*() { return ref; }

        Ref(T* val) : ref(Data::ref(val)) {}
        Ref(Ref* wrapper) : ref(wrapper->ref) {}
    };

    // -------------------------------------------------------------------------
    // Weak pointer, will be null if his owner has died
    // -------------------------------------------------------------------------
    struct WeakOwner {
        WeakOwner* owner;
        bool isAlive;
    };

    template <class T>
    struct Weak {
        WeakOwner handle;
        T& ref;

        Weak(T* val, WeakOwner* owner = nullptr)
            : ref(Data::ref(val)), handle{ owner, true }
        {
        }
    };

    // -------------------------------------------------------------------------
    // Unique pointer
    // -------------------------------------------------------------------------
    template <class T>
    class Unique {
    public:
        Unique(T* ptr) : m_ptr(ptr) {}
        ~Unique() { if (m_ptr) free(m_ptr); }

        Unique(Unique<T>&& p) : m_ptr(p.m_ptr) { p.m_ptr = nullptr; }
        Unique(Unique<T> const& p) = delete;

        Unique& operator=(Unique<T>&& p) { m_ptr = p.m_ptr; p.m_ptr = nullptr; return *this; }
        Unique& operator=(Unique<T> const& p) = delete;

        bool operator!() { return m_ptr == nullptr; }
        operator bool() { return m_ptr != nullptr; }

        T* reset(T* ptr = nullptr) { T* p = m_ptr; m_ptr = ptr; return p; }

        T const* get() const { return m_ptr; }
        T* get() { return m_ptr; }

        T const* operator->() const { return m_ptr; }
        T* operator->() { return m_ptr; }

        T const& operator*() const { return *m_ptr; }
        T& operator*() { return *m_ptr; }

        static Unique<T> alloc(size_t size) { return Unique<T>((T*)malloc(size * sizeof(T))); }

    private:
        T* m_ptr;
    };


    template <class T>  T& ref(T* ptr);

} // namespace Data


#include "hpp/pointers.hpp"

#endif // DATA_POINTERS_H