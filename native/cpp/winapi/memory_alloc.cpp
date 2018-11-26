#include "native/memory.h"

#include "mprotect_flags.hpp"


namespace Native {
	// ------------------------------------------------------------------------
	M_EXPORT void* virtual_alloc(size_t size, int prot)
	{
		return VirtualAlloc(nullptr, size, MEM_COMMIT, mprotect_flags(prot));
	}

	// ------------------------------------------------------------------------
	M_EXPORT bool virtual_free(void* ptr, size_t size)
	{
		return VirtualFree(ptr, 0, MEM_RELEASE) != 0;
	}

	// ------------------------------------------------------------------------
	M_EXPORT void* mreserve(size_t size)
	{
		return VirtualAlloc(nullptr, size, MEM_RESERVE, PAGE_READWRITE);
	}

	// ------------------------------------------------------------------------
	M_EXPORT void mrelease(void* ptr)
	{
		VirtualFree(ptr, 0, MEM_RELEASE);
	}

	// ------------------------------------------------------------------------
	M_EXPORT void* mcommit(void* ptr, size_t size)
	{
		return VirtualAlloc(ptr, size, MEM_COMMIT, PAGE_READWRITE);
	}

	// ------------------------------------------------------------------------
	M_EXPORT void mdecommit(void* ptr, size_t size)
	{
		VirtualFree(ptr, size, MEM_DECOMMIT);
	}

} // namespace Native