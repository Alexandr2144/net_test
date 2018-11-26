#pragma once

#ifndef DLMALLOC_EXPORT
#define DLMALLOC_EXPORT extern
#endif

/*
malloc(size_t n)
Returns a pointer to a newly allocated chunk of at least n bytes, or
null if no space is available, in which case errno is set to ENOMEM
on ANSI C systems.

If n is zero, malloc returns a minimum-sized chunk. (The minimum
size is 16 bytes on most 32bit systems, and 32 bytes on 64bit
systems.)  Note that size_t is an unsigned type, so calls with
arguments that would be negative if signed are interpreted as
requests for huge amounts of space, which will often fail. The
maximum supported value of n differs across systems, but is in all
cases less than the maximum representable value of a size_t.
*/
DLMALLOC_EXPORT void* dlmalloc(size_t);

/*
free(void* p)
Releases the chunk of memory pointed to by p, that had been previously
allocated using malloc or a related routine such as realloc.
It has no effect if p is null. If p was not malloced or already
freed, free(p) will by default cause the current program to abort.
*/
DLMALLOC_EXPORT void  dlfree(void*);

/*
calloc(size_t n_elements, size_t element_size);
Returns a pointer to n_elements * element_size bytes, with all locations
set to zero.
*/
DLMALLOC_EXPORT void* dlcalloc(size_t, size_t);

/*
realloc(void* p, size_t n)
Returns a pointer to a chunk of size n that contains the same data
as does chunk p up to the minimum of (n, p's size) bytes, or null
if no space is available.

The returned pointer may or may not be the same as p. The algorithm
prefers extending p in most cases when possible, otherwise it
employs the equivalent of a malloc-copy-free sequence.

If p is null, realloc is equivalent to malloc.

If space is not available, realloc returns null, errno is set (if on
ANSI) and p is NOT freed.

if n is for fewer bytes than already held by p, the newly unused
space is lopped off and freed if possible.  realloc with a size
argument of zero (re)allocates a minimum-sized chunk.

The old unix realloc convention of allowing the last-free'd chunk
to be used as an argument to realloc is not supported.
*/
DLMALLOC_EXPORT void* dlrealloc(void*, size_t);

/*
realloc_in_place(void* p, size_t n)
Resizes the space allocated for p to size n, only if this can be
done without moving p (i.e., only if there is adjacent space
available if n is greater than p's current allocated size, or n is
less than or equal to p's size). This may be used instead of plain
realloc if an alternative allocation strategy is needed upon failure
to expand space; for example, reallocation of a buffer that must be
memory-aligned or cleared. You can use realloc_in_place to trigger
these alternatives only when needed.

Returns p if successful; otherwise null.
*/
DLMALLOC_EXPORT void* dlrealloc_in_place(void*, size_t);

/*
memalign(size_t alignment, size_t n);
Returns a pointer to a newly allocated chunk of n bytes, aligned
in accord with the alignment argument.

The alignment argument should be a power of two. If the argument is
not a power of two, the nearest greater power is used.
8-byte alignment is guaranteed by normal malloc calls, so don't
bother calling memalign with an argument of 8 or less.

Overreliance on memalign is a sure way to fragment space.
*/
DLMALLOC_EXPORT void* dlmemalign(size_t, size_t);

/*
int posix_memalign(void** pp, size_t alignment, size_t n);
Allocates a chunk of n bytes, aligned in accord with the alignment
argument. Differs from memalign only in that it (1) assigns the
allocated memory to *pp rather than returning it, (2) fails and
returns EINVAL if the alignment is not a power of two (3) fails and
returns ENOMEM if memory cannot be allocated.
*/
DLMALLOC_EXPORT int dlposix_memalign(void**, size_t, size_t);

/*
valloc(size_t n);
Equivalent to memalign(pagesize, n), where pagesize is the page
size of the system. If the pagesize is unknown, 4096 is used.
*/
DLMALLOC_EXPORT void* dlvalloc(size_t);

/*
mallopt(int parameter_number, int parameter_value)
Sets tunable parameters The format is to provide a
(parameter-number, parameter-value) pair.  mallopt then sets the
corresponding parameter to the argument value if it can (i.e., so
long as the value is meaningful), and returns 1 if successful else
0.  To workaround the fact that mallopt is specified to use int,
not size_t parameters, the value -1 is specially treated as the
maximum unsigned size_t value.

SVID/XPG/ANSI defines four standard param numbers for mallopt,
normally defined in malloc.h.  None of these are use in this malloc,
so setting them has no effect. But this malloc also supports other
options in mallopt. See below for details.  Briefly, supported
parameters are as follows (listed defaults are for "typical"
configurations).

Symbol            param #  default    allowed param values
M_TRIM_THRESHOLD     -1   2*1024*1024   any   (-1 disables)
M_GRANULARITY        -2     page size   any power of 2 >= page size
M_MMAP_THRESHOLD     -3      256*1024   any   (or 0 if no MMAP support)
*/
DLMALLOC_EXPORT int dlmallopt(int, int);

/*
malloc_footprint();
Returns the number of bytes obtained from the system.  The total
number of bytes allocated by malloc, realloc etc., is less than this
value. Unlike mallinfo, this function returns only a precomputed
result, so can be called frequently to monitor memory consumption.
Even if locks are otherwise defined, this function does not use them,
so results might not be up to date.
*/
DLMALLOC_EXPORT size_t dlmalloc_footprint(void);

/*
malloc_max_footprint();
Returns the maximum number of bytes obtained from the system. This
value will be greater than current footprint if deallocated space
has been reclaimed by the system. The peak number of bytes allocated
by malloc, realloc etc., is less than this value. Unlike mallinfo,
this function returns only a precomputed result, so can be called
frequently to monitor memory consumption.  Even if locks are
otherwise defined, this function does not use them, so results might
not be up to date.
*/
DLMALLOC_EXPORT size_t dlmalloc_max_footprint(void);

/*
malloc_footprint_limit();
Returns the number of bytes that the heap is allowed to obtain from
the system, returning the last value returned by
malloc_set_footprint_limit, or the maximum size_t value if
never set. The returned value reflects a permission. There is no
guarantee that this number of bytes can actually be obtained from
the system.
*/
DLMALLOC_EXPORT size_t dlmalloc_footprint_limit();

/*
malloc_set_footprint_limit();
Sets the maximum number of bytes to obtain from the system, causing
failure returns from malloc and related functions upon attempts to
exceed this value. The argument value may be subject to page
rounding to an enforceable limit; this actual value is returned.
Using an argument of the maximum possible size_t effectively
disables checks. If the argument is less than or equal to the
current malloc_footprint, then all future allocations that require
additional system memory will fail. However, invocation cannot
retroactively deallocate existing used memory.
*/
DLMALLOC_EXPORT size_t dlmalloc_set_footprint_limit(size_t bytes);
