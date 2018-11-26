#include "native/crash.h"

#include "core/math/common.h"

#include <Windows.h>
#include <DbgHelp.h>
#include <stdio.h>

#pragma comment(lib, "Dbghelp.lib")


namespace Native {
    static ICrashHandler* crash_handler = nullptr;

    // ------------------------------------------------------------------------
    M_EXPORT ICrashHandler* setCrashHandler(ICrashHandler* handler)
    {
        ICrashHandler* backup = crash_handler;
        crash_handler = handler;
        return backup;
    }

    // ------------------------------------------------------------------------
    M_EXPORT void crash(Native::CL cl, uint32_t code, char const* msg)
    {
        crash_guard_push(cl, code, msg);
        *(int*)nullptr = 0;
    }

    // ------------------------------------------------------------------------
    M_EXPORT void crash_guard_push(Native::CL cl, uint32_t code, char const* msg)
    {
        if (crash_handler) {
            crash_handler->push(cl, code, msg);
        }
    }

    // ------------------------------------------------------------------------
    M_EXPORT void crash_guard_pop()
    {
        if (crash_handler) {
            crash_handler->pop();
        }
    }

    // ------------------------------------------------------------------------
    M_EXPORT int call_main(int argc, char** argv, int(*proc_main)(int, char**))
    {
        __try {
            return proc_main(argc, argv);
        }
        __except (EXCEPTION_EXECUTE_HANDLER)
        {
#ifdef _DEBUG
            DebugBreak();
#endif // _DEBUG

            if (crash_handler) {
                crash_handler->onSignal(0);
            }
            return -1;
        }
    }

    // ------------------------------------------------------------------------
    M_EXPORT char const* signame(int sig)
    {
        return "SIGABRT";
    }

    // ------------------------------------------------------------------------
    static SYMBOL_INFO* prepareProcessSymbols(HANDLE hProc, char* buffer, size_t symLen)
    {

        SymInitialize(hProc, NULL, TRUE);

        SYMBOL_INFO* symbol = (SYMBOL_INFO*)buffer;
        symbol->SizeOfStruct = sizeof(SYMBOL_INFO);
        symbol->MaxNameLen = (ULONG)symLen;
        return symbol;
    }

    // ------------------------------------------------------------------------
    M_EXPORT void stacktrace(size_t from, size_t to)
    {
        static const size_t maxTraceDepth = 512;
        static const size_t maxSymbolLen = 512;

        void* trace[maxTraceDepth];
        char memSymbol[sizeof(SYMBOL_INFO) + maxSymbolLen + 1];

        HANDLE hProc = GetCurrentProcess();
        SYMBOL_INFO* symbol = prepareProcessSymbols(hProc, memSymbol, maxSymbolLen);

        WORD depth = RtlCaptureStackBackTrace((DWORD)from, (DWORD)Math::clamp(from, from + maxTraceDepth, to), trace, NULL);

        DWORD displacement;
        IMAGEHLP_LINE64 line;
        for (int i = 0; i < depth; i++) {
            DWORD64 address = (DWORD64)(trace[i]);
            BOOL ok = SymFromAddr(hProc, address, NULL, symbol);

            if (SymGetLineFromAddr64(hProc, address, &displacement, &line))
                crash_handler->onTrace(trace[i], symbol->Name, line.FileName, line.LineNumber);
            else crash_handler->onTrace(trace[i], symbol->Name, nullptr, 0);
        }
    }


    // ------------------------------------------------------------------------
    M_EXPORT void assert(AssertHandle handle)
    {
        if (handle.passed) return;
        crash(handle.cl, 0, handle.msg);
    }

    // ------------------------------------------------------------------------
    M_EXPORT void assert_ext(AssertHandle handle, const char* fmt, ...);

    // ------------------------------------------------------------------------
    M_EXPORT void assert_msg(Native::CL cl, bool passed, const char* fmt, ...)
    {
        if (passed) return;

        va_list args;
        va_start(args, fmt);
        assert_vfail(cl, fmt, args);
        va_end(args);
    }

    // ------------------------------------------------------------------------
    M_EXPORT void assert_vmsg(Native::CL cl, bool passed, const char* fmt, va_list args)
    {
        if (passed) return;
        assert_vfail(cl, fmt, args);
    }

    // ------------------------------------------------------------------------
    M_EXPORT void assert_fail(Native::CL cl, const char* fmt, ...)
    {
        va_list args;
        va_start(args, fmt);
        assert_vfail(cl, fmt, args);
        va_end(args);
    }

    // ------------------------------------------------------------------------
    M_EXPORT void assert_vfail(Native::CL cl, const char* fmt, va_list args)
    {
        char buf[512];
        vsnprintf(buf, sizeof(buf), fmt, args);
        Native::crash(cl, 0, buf);
    }

} // namespace Native