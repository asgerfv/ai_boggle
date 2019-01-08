#pragma once


#include <common/utils.hpp>

#include <array>
#include <cassert>
#include <cstdint>

#include <string>

#if defined(_DEBUG)
  #include <iostream>
#endif


// ----------------------------------------------------------------------------


namespace common
{
  class CTriePool
  {
  public:

    typedef uint8_t LetterIndex_t;
    typedef uint32_t Index_t;

    CTriePool(const Index_t parent = 0);
    CTriePool(const CTriePool&) = delete;
    CTriePool(CTriePool&&) = delete;
    ~CTriePool();

    CTriePool& operator = (const CTriePool&) = delete;
    CTriePool& operator = (CTriePool&&) = delete;

    void AddWord(const std::string&);

    /// Find the next Trie according to the letter index
    CTriePool* FindTrieWithStartingLetter(const LetterIndex_t index, CTriePool& parentTrie);

    int32_t GetRemainingWords() const;

    void DumpToScreen() const;

    static void InitializePool();
    static void ClearAllTries();

    static LetterIndex_t GetLetterAsIndex(const char letter);
    static char GetIndexAsLetter(const LetterIndex_t letter);

    static void GetFoundWords(const CTriePool& currentTrie, TContainerWriter<const char*> wordList, uint32_t& scoreOutput);
    static uint32_t GetFoundWordsSize(const CTriePool& currentTrie);

    static uint32_t GetScoreForWordLength(const size_t wordLength);

  private:
    static constexpr int32_t C_CHILDREN_COUNT = ('z' - 'a') + 1;
    static const uint32_t C_MAX_WORD_LENGTH = 15;

    /// Notes:
    /// 600.000 needs 20 bits, which would also fit: 1.048.575 entries
    /// If we need 20 bits in each entry then that would be: 20 bits * 26 chars = 520 bits for all!
    /// At the moment we use 832 bits - or 104 chars. This would then be reduced to 65 chars.
    ///
    /// A dictionary with 300.000 words would require 19 bits, and would also fit: 524.287 words.
    /// 

    std::array<Index_t, C_CHILDREN_COUNT> m_children;

    const Index_t m_parent;

    int32_t m_wordCount : 31;
    int32_t m_found : 1;

    char m_word[C_MAX_WORD_LENGTH + 1];

#if defined(_DEBUG)
    static int32_t s_instanceCount;
#endif
  };
}


// ----------------------------------------------------------------------------


namespace common
{
  namespace details
  {
    CTriePool::Index_t AllocateTrie(CTriePool::Index_t parent = 0);

    CTriePool* GetPtrFromIndex(CTriePool::Index_t index);
    CTriePool::Index_t GetIndexFromPtr(CTriePool*);
  }
}


// ----------------------------------------------------------------------------


inline common::CTriePool::CTriePool(const Index_t parent)
  : m_parent(parent)
  , m_wordCount(0)
  , m_found(false)
{
  m_children.fill(0);
  m_word[0] = '\0';

#if defined(_DEBUG)
  s_instanceCount++;
#endif
}


inline common::CTriePool::~CTriePool()
{
#if defined(_DEBUG)
  s_instanceCount--;
#endif
}


inline void common::CTriePool::AddWord(const std::string& word)
{
  CTriePool* pCurrentTrie = this;

  for (size_t i = 0; i < word.length(); i++)
  {
    const char currentLetter = word[i];
    const auto currentIndex = GetLetterAsIndex(currentLetter);

    if (currentLetter == 'q' && word[i + 1] == 'u')
    {
      i++;
    }

    pCurrentTrie->m_wordCount++;

    const Index_t childIndex = pCurrentTrie->m_children[currentIndex];
    if (childIndex)
    {
      pCurrentTrie = details::GetPtrFromIndex(childIndex);
    }
    else
    {
      const Index_t newChildIndex = details::AllocateTrie(details::GetIndexFromPtr(pCurrentTrie));

      pCurrentTrie->m_children[currentIndex] = newChildIndex;

      pCurrentTrie = details::GetPtrFromIndex(newChildIndex);
    }
  }

  assert(word.length() < sizeof(m_word));
  strcpy(pCurrentTrie->m_word, word.c_str());
}


inline common::CTriePool* common::CTriePool::FindTrieWithStartingLetter(const LetterIndex_t index, CTriePool& parentTrie)
{
  assert(index < C_CHILDREN_COUNT);

  const Index_t childIndex = parentTrie.m_children[index];
  if (!childIndex)
  {
    return nullptr;
  }

  CTriePool* pCurrentTrie = details::GetPtrFromIndex(childIndex);

  if (pCurrentTrie->m_word[0] && !pCurrentTrie->m_found)
  {
    assert(parentTrie.m_wordCount);

    parentTrie.m_wordCount--;

    pCurrentTrie->m_found = true;
  }

  return pCurrentTrie;
}


inline int32_t common::CTriePool::GetRemainingWords() const
{
  return m_wordCount;
}


inline void common::CTriePool::DumpToScreen() const
{
#if defined(_DEBUG)
  std::cout << "Trie stats:" << std::endl;
  std::cout << "\tWord count: " << m_wordCount << std::endl;

  for (LetterIndex_t i = 0; i < C_CHILDREN_COUNT; i++)
  {
    if (!m_children[i])
    {
      continue;
    }

    //std::cout << "\tLetter " << int32_t(i) << " is " << GetIndexAsLetter(i) << " has word count: " << m_children[i]->m_wordCount << std::endl;
  }

#endif
}


inline common::CTriePool::LetterIndex_t common::CTriePool::GetLetterAsIndex(const char letter)
{
  const auto result = letter - 'a';

  assert(result < C_CHILDREN_COUNT);

  return result;
}


inline char common::CTriePool::GetIndexAsLetter(const LetterIndex_t letter)
{
  assert(letter < C_CHILDREN_COUNT);

  const auto result = letter + 'a';

  return result;
}


inline void common::CTriePool::GetFoundWords
(
  const CTriePool& currentTrie,
  TContainerWriter<const char*> wordList,
  uint32_t& scoreOutput
)
{
  if (currentTrie.m_found)
  {
    wordList.container[wordList.currentIndex++] = currentTrie.m_word;
    scoreOutput += GetScoreForWordLength( strlen(currentTrie.m_word) );
  }

  for (size_t i = 0; i < C_CHILDREN_COUNT; i++)
  {
    const Index_t childIndex = currentTrie.m_children[i];
    if (!childIndex)
    {
      continue;
    }

    const CTriePool* pChild = details::GetPtrFromIndex(childIndex);

    GetFoundWords(*pChild, wordList, scoreOutput);
  }
}


inline uint32_t common::CTriePool::GetFoundWordsSize(const CTriePool& currentTrie)
{
  uint32_t result = 0;

  if (currentTrie.m_found)
  {
    result++;
  }

  for (size_t i = 0; i < C_CHILDREN_COUNT; i++)
  {
    const Index_t childIndex = currentTrie.m_children[i];
    if (!childIndex)
    {
      continue;
    }

    const CTriePool* pChild = details::GetPtrFromIndex(childIndex);

    result += GetFoundWordsSize(*pChild);
  }

  return result;
}


inline uint32_t common::CTriePool::GetScoreForWordLength(const size_t wordLength)
{
  static const uint8_t s_scoreTable[C_MAX_WORD_LENGTH + 1] =
  {
    0,
    0,
    0,
    1,    //< 3
    1,    //< 4
    2,    //< 5
    3,    //< 6
    5,    //< 7
    11,   //< 8
    11,
    11,
    11,
    11,
    11,
    11,
    11,
  };

  assert(wordLength >= 3 && "This should be checked for while loading the dictionary.");
  assert(wordLength < sizeof(s_scoreTable) && "This should be checked for while loading the dictionary.");

  return s_scoreTable[wordLength];
}
