#pragma once
#ifndef NATIVE_MEMORY_H
#define NATIVE_MEMORY_H

#include "native/common.h"


namespace Native {
	M_EXPORT size_t      page_size();

	M_EXPORT void*       page_begin_ptr(void* ptr);
	M_EXPORT const void* page_begin_ptr(const void* ptr);


	enum PageProtectFlags {
		PAGE_PROT_READ = 0x01,
		PAGE_PROT_WRITE = 0x02,
		PAGE_PROT_EXECUTABLE = 0x04,
	};
   
   // TODO: ???
	M_EXPORT void* virtual_alloc(size_t size, int prot);
	M_EXPORT bool  virtual_free(void* ptr, size_t size);

	M_EXPORT void* mreserve(size_t size);
	M_EXPORT void  mrelease(void* ptr);

	M_EXPORT void* mcommit(void* ptr, size_t size);
	M_EXPORT void  mdecommit(void* ptr, size_t size);
	

} // namespace Native


#endif // NATIVE_MEMORY_H