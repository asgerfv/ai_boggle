#include "trie-pool-optimized.hpp"


// ----------------------------------------------------------------------------

#define ROTA_USE_CUSTOM_ALLOCATOR 1

// ----------------------------------------------------------------------------


/// Note: Could also have used "__declspec(selectany)" in the header, but this is
/// more cross-platform.
#if defined(_DEBUG)

  int32_t common::CTriePoolOptimized::s_instanceCount = 0;

#endif


// ----------------------------------------------------------------------------


namespace common
{
  namespace details
  {
    class CTriePoolOptimizedAllocator
    {
    public:

      CTriePoolOptimizedAllocator();
      ~CTriePoolOptimizedAllocator();

      CTriePoolOptimized::Index_t AllocateTrie(CTriePoolOptimized::Index_t parent);
      void FreeAll();

      CTriePoolOptimized* GetPtrFromIndex(const CTriePoolOptimized::Index_t index);
      CTriePoolOptimized::Index_t GetIndexFromPtr(const CTriePoolOptimized*);

    private:
      static const size_t C_SSE_ALIGNMENT = 16;
      static const size_t C_TRIE_SIZE = common::AlignUpTo(sizeof(CTriePoolOptimized), C_SSE_ALIGNMENT);
      static const size_t C_PREALLOCATED_INSTANCE_COUNT = 600 * 1000;

      uint8_t* m_pMem = nullptr;
      uint8_t* m_pAlignedMem = nullptr;
      uint32_t m_freeCount = C_PREALLOCATED_INSTANCE_COUNT;
      volatile long m_instanceCount = 1;

#if defined(_DEBUG)
      const size_t m_debugTrieSize = C_TRIE_SIZE;
      CTriePoolOptimized* m_pDebugTriesArray = nullptr;
#endif
    };
  }
}


// ----------------------------------------------------------------------------


/*static*/ common::details::CTriePoolOptimizedAllocator* g_pTriePoolOptimizedAllocator = nullptr;


// ----------------------------------------------------------------------------


common::details::CTriePoolOptimizedAllocator::CTriePoolOptimizedAllocator()
{
  m_pMem = new uint8_t[(C_PREALLOCATED_INSTANCE_COUNT * C_TRIE_SIZE) + C_CACHE_SIZE];

  m_pAlignedMem = common::AlignUpToPtr(m_pMem, C_CACHE_SIZE);

#if defined(_DEBUG)
  m_pDebugTriesArray = reinterpret_cast<CTriePoolOptimized*>(m_pAlignedMem);
#endif
}


common::details::CTriePoolOptimizedAllocator::~CTriePoolOptimizedAllocator()
{
  FreeAll();
}


common::CTriePoolOptimized::Index_t common::details::CTriePoolOptimizedAllocator::AllocateTrie(CTriePoolOptimized::Index_t parent)
{
  const uint32_t indexToUse = m_instanceCount;

  assert(indexToUse < C_PREALLOCATED_INSTANCE_COUNT);

  void* pMemForTrie = &m_pAlignedMem[indexToUse * C_TRIE_SIZE];

  common::InterlockedIncrement(&m_instanceCount);

  new (pMemForTrie) CTriePoolOptimized(parent);

  return indexToUse;
}


void common::details::CTriePoolOptimizedAllocator::FreeAll()
{
  m_instanceCount = 0;

  delete[] m_pMem;
  m_pMem = nullptr;
}


common::CTriePoolOptimized* common::details::CTriePoolOptimizedAllocator::GetPtrFromIndex(const CTriePoolOptimized::Index_t index)
{
  void* pMemForTrie = &m_pAlignedMem[index * C_TRIE_SIZE];

  return reinterpret_cast<common::CTriePoolOptimized*>(pMemForTrie);
}


common::CTriePoolOptimized::Index_t common::details::CTriePoolOptimizedAllocator::GetIndexFromPtr(const CTriePoolOptimized* pTrie)
{
  const uintptr_t diff = uintptr_t(pTrie) - uintptr_t(m_pAlignedMem);

  const CTriePoolOptimized::Index_t result = static_cast<CTriePoolOptimized::Index_t>(diff / C_TRIE_SIZE);

  return result;
}


// ----------------------------------------------------------------------------


void common::CTriePoolOptimized::InitializePool()
{
  if (!g_pTriePoolOptimizedAllocator)
  {
    g_pTriePoolOptimizedAllocator = new details::CTriePoolOptimizedAllocator();
  }
}


void common::CTriePoolOptimized::ClearAllTries()
{
  delete g_pTriePoolOptimizedAllocator;
  g_pTriePoolOptimizedAllocator = nullptr;
}


common::CTriePoolOptimized::Index_t common::CTriePoolOptimized::AllocateTrie(const CTriePoolOptimized::Index_t parent)
{
  return g_pTriePoolOptimizedAllocator->AllocateTrie(parent);
}


common::CTriePoolOptimized* common::CTriePoolOptimized::GetPtrFromIndex(CTriePoolOptimized::Index_t index)
{
  return g_pTriePoolOptimizedAllocator->GetPtrFromIndex(index);
}


common::CTriePoolOptimized::Index_t common::CTriePoolOptimized::GetIndexFromPtr(CTriePoolOptimized* pTrie)
{
  return g_pTriePoolOptimizedAllocator->GetIndexFromPtr(pTrie);
}
