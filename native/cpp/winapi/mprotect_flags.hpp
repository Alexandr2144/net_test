#pragma once
#ifndef NATIVE_WINAPI_MPROTECT_FLAGS_H
#define NATIVE_WINAPI_MPROTECT_FLAGS_H

#include "native/memory.h"
#include "native/crash.h"
#include <windows.h>


namespace Native {
   // ------------------------------------------------------------------------
   static DWORD mprotect_flags(int flags)
   {
      const bool executable = (flags & PAGE_PROT_EXECUTABLE) != 0;
      const bool write = (flags & PAGE_PROT_WRITE) != 0;
      const bool read = (flags & PAGE_PROT_READ) != 0;

      if (read && !write && !executable) {
         return PAGE_READONLY;
      } else if (read && !write && executable) {
         return PAGE_EXECUTE_READ;
      } else if (write && !executable) {
         return PAGE_READWRITE;
      } else if (write && executable) {
         return PAGE_EXECUTE_READWRITE;
      } else {
         return 0;
      }
   }

   // ------------------------------------------------------------------------
   static DWORD filemap_flags(int flags)
   {
      const bool executable = (flags & PAGE_PROT_EXECUTABLE) != 0;
      const bool write = (flags & PAGE_PROT_WRITE) != 0;
      const bool read = (flags & PAGE_PROT_READ) != 0;

      if (read && !write && !executable) {
         return FILE_MAP_READ;
      } else if (read && !write && executable) {
         return FILE_MAP_READ | FILE_MAP_EXECUTE;
      } else if (write && !executable) {
         return FILE_MAP_WRITE;
      } else if (write && executable) {
         return FILE_MAP_WRITE | FILE_MAP_EXECUTE;
      } else {
         return 0;
      }
   }

   // ------------------------------------------------------------------------
   static int rwx_flags_from_attr(char const* attr)
   {
      int flags = 0;
      while (true) {
         switch (*attr) {
         case '\0': return flags;
         case 'r':  flags |= PAGE_PROT_READ; break;
         case 'w':  flags |= PAGE_PROT_WRITE; break;
         case 'e':  flags |= PAGE_PROT_EXECUTABLE; break;
         default:
            M_ASSERT_FAIL("Invalid mprotect attribute: expected 'rwx' but '%c' found", *attr);
            break;
         }
         attr += 1;
      }
   }

} // namespace Native


#endif NATIVE_WINAPI_MPROTECT_FLAGS_H