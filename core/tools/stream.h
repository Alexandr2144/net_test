#pragma once

#ifndef TOOLS_STREAM_H
#define TOOLS_STREAM_H


namespace Tools {
   template <class T>
   struct IStream {
      virtual ~IStream() {}
      virtual bool finished() const = 0;
	  virtual void restore() = 0;
      virtual void save() = 0;
      virtual T read() = 0;
   };
   template <class T>
   using IStream_CRef = IStream<T> const&;

} // namespace Tools


#endif // TOOLS_STREAM_H