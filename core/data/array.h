#pragma once
#ifndef DATA_ARRAY_H
#define DATA_ARRAY_H


#include "core/data/pointers.h"
#include "native/crash.h"


#define M_TAIL_ARRAY_DATA(TYPE, NAME) \
    TYPE* NAME##_end = (TYPE*)(this + 1);

#define M_TAIL_ARRAY_GETTER(TYPE, NAME) \
    Data::Array<TYPE> NAME () const { return Data::Array<TYPE>{ (TYPE*)(this + 1), NAME##_end }; } \
    size_t sizeOf() const { return (char*)NAME##_end - (char*)this; } \

#define M_TAIL_ARRAY(TYPE, NAME) \
    M_TAIL_ARRAY_DATA(TYPE, NAME) \
	M_TAIL_ARRAY_GETTER(TYPE, NAME)


namespace Data {
   // ------------------------------------------------------------------------
   // Array indices range
   // ------------------------------------------------------------------------
   struct ArrayRange {
      size_t begin;
      size_t end;
   };
   using ArrayRange_CRef = ArrayRange const&;

   // ------------------------------------------------------------------------
   // Stored in virtual memory array
   // ------------------------------------------------------------------------
   template <class T>
   struct Array {
      T* begin = nullptr;
      T* end = nullptr;

   public:
      Array(T* begin, T* end) : begin(begin), end(end) {}
      Array() = default;

      Array(Array<T>&& a);
      Array(Array<T> const&) = default;
      Array& operator=(Array<T>&&);
      Array& operator=(Array<T> const&) = default;

      bool isValid() const { return begin <= end; }
      bool isValidIndex(size_t idx) const { return idx < size_t(end - begin); }

      operator Array<T const>() const { M_ASSERT(isValid()); return Array<T const>{ begin, end }; }

      T& operator[](size_t idx) const { M_ASSERT(isValid()); M_ASSERT(isValidIndex(idx)); return begin[idx]; }

      T& front() const { return begin[0]; }
      T& back() const { return end[-1]; }
   };
   template <class T>  using CArray = Array<T const>;
   template <class T>  using Array_CRef = Array<T> const&;
   template <class T>  using CArray_CRef = CArray<T> const&;


   template <class T>  CArray<T>  toArray(T const* base, size_t count) { return Array<T const>{ base, base + count }; }
   template <class T>  Array<T>   toArray(T* base, size_t count)       { return Array<T>{ base, base + count }; }

   template <class T>  bool isNull(Array_CRef<T> a)     { return a.begin == nullptr; }
   template <class T>  bool isEmpty(Array_CRef<T> a)    { return a.begin == a.end; }
   template <class T>  size_t count(Array_CRef<T> a)    { return a.end - a.begin; }
   template <class T>  size_t size(Array_CRef<T> a)     { return sizeof(T) * count(a); }

   template <class T>  bool can_split(Array_CRef<T> base, size_t splitSize);
   template <class T>  bool can_slice(Array_CRef<T> base, size_t from, size_t to);

   template <class T>  Array<T> split(Ref<Array<T>> base, size_t splitSize);
   template <class T>  Array<T> slice(Array_CRef<T> base, size_t from, size_t to);
   template <class T>  Array<T> slice(Array_CRef<T> base, ArrayRange_CRef range)    { return slice(base, range.begin, range.end); }

   template <class T>  Array<T> slice_back(Array_CRef<T> base, size_t count) { return Array<T>{ base.end - count, base.end }; }

   template <class T>  Array<T> pop_front(Array_CRef<T> base) { return Array<T>{ base.begin + 1, base.end }; }
   template <class T>  Array<T> pop_back(Array_CRef<T> base) { return Array<T>{ base.begin, base.end - 1 }; }

   template <class T>  void replace(Array_CRef<T> a, T const& from, T const& to);
   template <class T>  bool contain(Array_CRef<T> a, T const& val);

   // ------------------------------------------------------------------------
   // Array of bytes
   // ------------------------------------------------------------------------
   using Byte = uint8_t;
   using CByte = Byte const;

   using Bytes = Array<Byte>;
   using Bytes_CRef = Bytes const&;

   using CBytes = CArray<Byte>;
   using CBytes_CRef = CBytes const&;

   extern Bytes noBytes;

   // ------------------------------------------------------------------------
   // Array with paddings
   // ------------------------------------------------------------------------
   template <class T>
   struct SparseArray {
      Bytes memory;
      size_t offset;
      size_t stride;

   public:
      bool isValid() { return memory.isValid() && offset < count(memory) && stride < count(memory); }
      bool isValidIndex(size_t idx) const { return getOffsetByIndex(idx) < count(memory); }
      size_t getOffsetByIndex(size_t idx) const { return idx * stride + offset; }

      operator SparseArray<T const> const&() { M_ASSERT(isValid()); return SparseArray<T const>{ memory, offset, stride }; }

      T& operator[](size_t idx) { M_ASSERT(isValid()); M_ASSERT(isValidIndex(idx)); return *(T*)((uint8_t*)memory.begin + getOffsetByIndex(idx)); }
   };
   template <class T>  using CSparseArray = SparseArray<T const>;
   template <class T>  using SparseArray_CRef = SparseArray<T> const&;
   template <class T>  using CSparseArray_CRef = CSparseArray<T> const&;

   template <class T>  bool isNull(SparseArray_CRef<T> a)       { return isNull(a.memory); }
   template <class T>  bool isEmpty(SparseArray_CRef<T> a)      { return isEmpty(a.memory); }
   template <class T>  size_t count(SparseArray_CRef<T> a)      { return size(a.memory) / a.stride; }
   template <class T>  size_t size(SparseArray_CRef<T> a)       { return size(a.memory); }

   // ------------------------------------------------------------------------
   template <class T>  Array<T const> toArray(CBytes_CRef& bytes)  { return Array<T const>{ (T const*)bytes.begin, (T const*)bytes.end }; }
   template <class T>  Array<T>       toArray(Bytes_CRef& bytes)   { return Array<T>{ (T*)bytes.begin, (T*)bytes.end }; }

   template <class T>  CBytes  toBytes(CArray_CRef<T> a) { return Bytes{ (Byte*)a.begin, (Byte*)a.end }; }
   template <class T>  Bytes   toBytes(Array_CRef<T> a) { return Bytes{ (Byte*)a.begin, (Byte*)a.end }; }

   template <class T>  CBytes toBytes(T const* ptr) { return toBytes((void const*)ptr, sizeof(T)); }
   template <class T>  Bytes  toBytes(T* ptr)       { return toBytes((void*)ptr, sizeof(T)); }

   CBytes toBytes(void const* ptr, size_t size);
   Bytes  toBytes(void* ptr, size_t size);

   CBytes toBytes(void const* begin, void const* end);
   Bytes  toBytes(void* begin, void* end);


   template <class T>
   bool read(Ref<CBytes> src, T* dest);
   bool read(Ref<CBytes> src, Bytes dest);

   template <class T>
   bool write(Ref<Bytes> dest, T const& val) { return write(dest, toBytes(&val)); }
   bool write(Ref<Bytes> dest, CBytes src);


   Bytes align(Bytes_CRef bytes, uintptr_t alignment);

   bool bytecmp(CBytes_CRef right, CBytes_CRef left);
   bool bytecopy(Bytes_CRef dest, CBytes_CRef src);
   void byteset(Bytes_CRef dest, Byte val);

   // ------------------------------------------------------------------------
   // Range-for cycle helpers
   // ------------------------------------------------------------------------
   template <class T>
   class ArrayCollection {
   public:
      ArrayCollection(Array_CRef<T> a);

      T* begin() { return m_array.begin; }
      T* end()   { return m_array.end; }

   private:
      Array_CRef<T> m_array;
   };

   template <class T>
   ArrayCollection<T> iterate(Array_CRef<T> a) { return ArrayCollection<T>(a); }

   // ------------------------------------------------------------------------
   template <class T>
   class SparseArrayCollection {
   public:
      struct Iterator {
         size_t stride;
         Byte* base;

      public:
         Iterator(uint8_t* base, size_t stride);
         T& operator*() { return *(T*)base; }

         bool operator<(Iterator const& other)   { return base < other.base; }
         bool operator==(Iterator const& other)  { return base == other.base; }
         bool operator!=(Iterator const& other)  { return base != other.base; }
         Iterator& operator++(int);
         Iterator& operator++();
      };

      SparseArrayCollection(SparseArray_CRef<T> a);

      Iterator begin() const;
      Iterator end() const;

   private:
      SparseArray_CRef<T> m_array;
   };

   template <class T> 
   SparseArrayCollection<T> iterate(SparseArray_CRef<T> a) { return SparseArrayCollection<T>(a); }

} // namespace Data


#include "hpp/array.hpp"

#endif // DATA_ARRAY_H