#include "core/memory/common.h"


namespace Memory {
   // -------------------------------------------------------------------------
   template <class T, class... ArgsTy>
   static T* emplace(IAllocator& allocator, ArgsTy&&... args)
   {
      void* ptr = allocator.alloc(sizeof(T));
      new(ptr) T(std::forward<ArgsTy>(args)...);
      return (T*)ptr;
   }

} // namespace Memory