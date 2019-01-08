#pragma once


#ifdef _MSC_VER
  #include <intrin.h>
#else
  #include <csignal>
#endif

#include <cstddef>
#include <cstdint>


// ----------------------------------------------------------------------------


namespace common
{
  template <class T>
  struct TContainerWriter
  {
    T* container;
    uint32_t currentIndex = 0;
  };


  /// Like MS "_countof"
  template <class T, size_t N>
  constexpr size_t GetArrayLength(T const (&)[N])
  {
    return N;
  }


  template <typename T>
  constexpr T AlignUpTo(T p, const size_t alignment)
  {
    return (p + (int64_t(alignment) - 1)) & -int64_t(alignment);
  }


  template <typename T>
  constexpr T* AlignUpToPtr(T* p, const size_t alignment)
  {
    return reinterpret_cast<T*>( (uintptr_t(p) + (int64_t(alignment) - 1)) & -int64_t(alignment) );
  }


  inline long InterlockedIncrement(volatile long* pInteger)
  {
#ifdef _MSC_VER
    return _InterlockedIncrement(pInteger);
#else
    return __atomic_add_fetch(pInteger, 1, __ATOMIC_SEQ_CST);
#endif
  }


  static const uint32_t C_CACHE_SIZE = 64;
}


// ----------------------------------------------------------------------------


#ifdef _MSC_VER
  #define ROTA_BREAK __debugbreak()
#else
  #define ROTA_BREAK raise(SIGTRAP)
#endif


// ----------------------------------------------------------------------------


#define ROTA_ASSERT(__statement) \
  if ( !(__statement) ) ROTA_BREAK;


// ----------------------------------------------------------------------------


//#pragma optimize("", off)


// ----------------------------------------------------------------------------
