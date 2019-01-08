#include "trie-pool.hpp"


// ----------------------------------------------------------------------------

#define ROTA_USE_CUSTOM_ALLOCATOR 1

// ----------------------------------------------------------------------------


/// Note: Could also have used "__declspec(selectany)" in the header, but this is
/// more cross-platform.
#if defined(_DEBUG)

  int32_t common::CTriePool::s_instanceCount = 0;

#endif


// ----------------------------------------------------------------------------


namespace common
{
  namespace details
  {
    class CTriePoolAllocator
    {
    public:

      CTriePoolAllocator();
      ~CTriePoolAllocator();

      CTriePool::Index_t AllocateTrie(CTriePool::Index_t parent);
      void FreeAll();

      CTriePool* GetPtrFromIndex(const CTriePool::Index_t index);
      CTriePool::Index_t GetIndexFromPtr(const CTriePool*);

    private:
      static const size_t C_SSE_ALIGNMENT = 16;
      static const size_t C_TRIE_SIZE = common::AlignUpTo(sizeof(CTriePool), C_SSE_ALIGNMENT);
      static const size_t C_PREALLOCATED_INSTANCE_COUNT = 600 * 1000;

      uint8_t* m_pMem = nullptr;
      uint8_t* m_pAlignedMem = nullptr;
      uint32_t m_freeCount = C_PREALLOCATED_INSTANCE_COUNT;
      uint32_t m_instanceCount = 1;
    };
  }
}


// ----------------------------------------------------------------------------


static common::details::CTriePoolAllocator* g_pTrieAllocator = nullptr;


// ----------------------------------------------------------------------------


common::details::CTriePoolAllocator::CTriePoolAllocator()
{
  m_pMem = new uint8_t[(C_PREALLOCATED_INSTANCE_COUNT * C_TRIE_SIZE) + C_CACHE_SIZE];

  m_pAlignedMem = common::AlignUpToPtr(m_pMem, C_CACHE_SIZE);
}


common::details::CTriePoolAllocator::~CTriePoolAllocator()
{
  FreeAll();
}


common::CTriePool::Index_t common::details::CTriePoolAllocator::AllocateTrie(CTriePool::Index_t parent)
{
  const uint32_t indexToUse = m_instanceCount;

  assert(indexToUse < C_PREALLOCATED_INSTANCE_COUNT);

  void* pMemForTrie = &m_pAlignedMem[indexToUse * C_TRIE_SIZE];

  m_instanceCount++;

  new (pMemForTrie) CTriePool(parent);

  return indexToUse;
}


void common::details::CTriePoolAllocator::FreeAll()
{
  delete [] m_pMem;
  m_pMem = nullptr;
}


common::CTriePool* common::details::CTriePoolAllocator::GetPtrFromIndex(const CTriePool::Index_t index)
{
  void* pMemForTrie = &m_pAlignedMem[index * C_TRIE_SIZE];

  return reinterpret_cast<common::CTriePool*>(pMemForTrie);
}


common::CTriePool::Index_t common::details::CTriePoolAllocator::GetIndexFromPtr(const CTriePool* pTrie)
{
  const uintptr_t diff = uintptr_t(pTrie) - uintptr_t(m_pAlignedMem);

  const CTriePool::Index_t result = static_cast<CTriePool::Index_t>(diff / C_TRIE_SIZE);

  return result;
}


// ----------------------------------------------------------------------------


common::CTriePool::Index_t common::details::AllocateTrie(const CTriePool::Index_t parent)
{
  return g_pTrieAllocator->AllocateTrie(parent);
}


common::CTriePool* common::details::GetPtrFromIndex(CTriePool::Index_t index)
{
  return g_pTrieAllocator->GetPtrFromIndex(index);
}


common::CTriePool::Index_t common::details::GetIndexFromPtr(CTriePool* pTrie)
{
  return g_pTrieAllocator->GetIndexFromPtr(pTrie);
}


// ----------------------------------------------------------------------------


void common::CTriePool::InitializePool()
{
  g_pTrieAllocator = new details::CTriePoolAllocator();
}


void common::CTriePool::ClearAllTries()
{
  delete g_pTrieAllocator;
  g_pTrieAllocator = nullptr;
}
