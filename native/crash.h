#pragma once
#ifndef NATIVE_CRASH_H
#define NATIVE_CRASH_H

#include "native/common.h"


// ----------------------------------------------------------------------------
#define M_WITH_CHECKS


// ----------------------------------------------------------------------------
#define M_CL Native::CL{ __FUNCTION__, __FILE__, __LINE__ }
#define M_COND(X) Native::AssertHandle{ #X, (X), M_CL }
#define M_COND_E(CL, X) Native::AssertHandle{ #X, (X), CL }

#define M_ASSERT(X)                Native::assert(M_COND(X))
#define M_ASSERT_MSG(X, MSG, ...)  Native::assert_msg(M_CL, (X), MSG, ##__VA_ARGS__)
#define M_ASSERT_FAIL(MSG, ...)    Native::assert_fail(M_CL, MSG, ##__VA_ARGS__)

#define M_CHECK(X) { auto cond = M_COND(X); Native::assert(cond); }

#define M_ON_CRASH()


namespace Native {
    struct CL {
        const char* function;
        const char* file;
        int line;
    };

    struct ICrashHandler {
        virtual void push(CL cl, uint32_t code, char const* msg) = 0;
        virtual void pop() = 0;

        virtual void onTrace(void* fPtr, char const* fName, char const* file, uint32_t line) = 0;
        virtual void onSignal(int sig) = 0;
    };

    M_EXPORT ICrashHandler* setCrashHandler(ICrashHandler* handler);

    M_EXPORT int call_main(int argc, char** argv, int(*proc_main)(int, char**));

    M_EXPORT void crash(CL cl, uint32_t code, char const* msg);
    M_EXPORT void crash_guard_push(CL cl, uint32_t code, char const* msg);
    M_EXPORT void crash_guard_pop();

    M_EXPORT void stacktrace(size_t from, size_t to);

    M_EXPORT char const* signame(int sig);


    struct CrashGuard {
        CrashGuard(CL cl, uint32_t code, char const* msg) { crash_guard_push(cl, code, msg); }
        ~CrashGuard() { crash_guard_pop(); }
    };


    struct AssertHandle {
        const char* msg;
        bool passed;
        CL cl;
    };

    M_EXPORT void assert(AssertHandle handle);
    M_EXPORT void assert_ext(AssertHandle handle, const char* fmt, ...);
    M_EXPORT void assert_msg(CL cl, bool passed, const char* fmt, ...);
    M_EXPORT void assert_vmsg(CL cl, bool passed, const char* fmt, va_list args);

    M_EXPORT void assert_fail(CL cl, const char* fmt, ...);
    M_EXPORT void assert_vfail(CL cl, const char* fmt, va_list args);

} // namespace Native


#endif // NATIVE_CRASH_H