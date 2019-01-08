#pragma once


#include <common/utils.hpp>

#include <array>
#include <cassert>
#include <cstdint>

#include <string>

#if defined(_DEBUG)
  #include <iostream>
#endif

//TODO: Board should be in letter-format as well

// ----------------------------------------------------------------------------


namespace common
{
  class CTriePoolOptimized
  {
  public:

    typedef uint8_t LetterIndex_t;
    typedef uint32_t Index_t;

    static constexpr int32_t C_CHILDREN_COUNT = ('z' - 'a') + 1;
    static const uint32_t C_MAX_WORD_LENGTH = 15;

    // ----------------------------------------------------------------------------


    CTriePoolOptimized(const Index_t parent = 0);
    CTriePoolOptimized(const CTriePoolOptimized&) = delete;
    CTriePoolOptimized(CTriePoolOptimized&&) = delete;
    ~CTriePoolOptimized();

    CTriePoolOptimized& operator = (const CTriePoolOptimized&) = delete;
    CTriePoolOptimized& operator = (CTriePoolOptimized&&) = delete;

    void AddWord(const std::string&);

    /// Find the next Trie according to the letter index
    Index_t FindTrieWithStartingLetter(const LetterIndex_t index, CTriePoolOptimized& parentTrie);

    int32_t GetRemainingWords() const;

    void DumpToScreen() const;

    static void InitializePool();
    static void ClearAllTries();

    static Index_t AllocateTrie(Index_t parent = 0);

    static CTriePoolOptimized* GetPtrFromIndex(Index_t index);
    static CTriePoolOptimized::Index_t GetIndexFromPtr(CTriePoolOptimized*);

    static LetterIndex_t GetLetterAsIndex(const char letter);
    static char GetIndexAsLetter(const LetterIndex_t letter);

    static void GetFoundWords(const CTriePoolOptimized& currentTrie, TContainerWriter<const char*>& wordList, uint32_t& scoreOutput);
    static uint32_t GetFoundWordsSize(const CTriePoolOptimized& currentTrie);

    static uint32_t GetScoreForWordLength(const size_t wordLength);

  private:
    std::array<Index_t, C_CHILDREN_COUNT> m_children;

    const Index_t m_parent;

    int32_t m_wordCount : 31;
    int32_t m_found : 1;

    char m_word[C_MAX_WORD_LENGTH + 1];

#if defined(_DEBUG)
    CTriePoolOptimized* m_debugParent = nullptr;
    std::array<CTriePoolOptimized*, C_CHILDREN_COUNT> m_debugChildren;
    Index_t m_DebugMyId = 0;
    static int32_t s_instanceCount;
#endif
  };
}


// ----------------------------------------------------------------------------


inline common::CTriePoolOptimized::CTriePoolOptimized(const Index_t parent)
  : m_parent(parent)
  , m_wordCount(0)
  , m_found(false)
{
  m_children.fill(0);
  m_word[0] = '\0';

#if defined(_DEBUG)
  m_debugParent = GetPtrFromIndex(parent);
  m_debugChildren.fill(nullptr);
  m_DebugMyId = GetIndexFromPtr(this);
  s_instanceCount++;
#endif
}


inline common::CTriePoolOptimized::~CTriePoolOptimized()
{
#if defined(_DEBUG)
  s_instanceCount--;
#endif
}


inline void common::CTriePoolOptimized::AddWord(const std::string& word)
{
  CTriePoolOptimized* pCurrentTrie = this;
  Index_t currentAsIndex = GetIndexFromPtr(pCurrentTrie);

  assert(GetPtrFromIndex(currentAsIndex) == pCurrentTrie);

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
      CTriePoolOptimized* pChildTrie = GetPtrFromIndex(childIndex);

      assert(pChildTrie != pCurrentTrie);

#if defined(_DEBUG)
      assert(pChildTrie == pCurrentTrie->m_debugChildren.at(currentIndex));
#endif

      pCurrentTrie = pChildTrie;
    }
    else
    {
      const Index_t newChildIndex = AllocateTrie(currentAsIndex);

      pCurrentTrie->m_children[currentIndex] = newChildIndex;

#if defined(_DEBUG)
      pCurrentTrie->m_debugChildren.at(currentIndex) = GetPtrFromIndex(newChildIndex);
#endif

      /// Next trie in the word-chain ...
      pCurrentTrie = GetPtrFromIndex(newChildIndex);
      currentAsIndex = newChildIndex;
    }
  }

  /// Add the actual word to the end trie
  //TODO: Just calculate the word from the children.
  assert(word.length() < sizeof(m_word));
  strcpy(pCurrentTrie->m_word, word.c_str());
}


inline common::CTriePoolOptimized::Index_t common::CTriePoolOptimized::FindTrieWithStartingLetter(const LetterIndex_t index, CTriePoolOptimized& parentTrie)
{
  assert(index < C_CHILDREN_COUNT);

  const Index_t childIndex = parentTrie.m_children[index];
  if (!childIndex)
  {
    return 0;
  }

  CTriePoolOptimized* pCurrentTrie = GetPtrFromIndex(childIndex);

  if (pCurrentTrie->m_word[0] && !pCurrentTrie->m_found)
  {
    assert(parentTrie.m_wordCount);

    parentTrie.m_wordCount--;

    pCurrentTrie->m_found = true;
  }

  return childIndex;
}


inline int32_t common::CTriePoolOptimized::GetRemainingWords() const
{
  return m_wordCount;
}


inline void common::CTriePoolOptimized::DumpToScreen() const
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


inline common::CTriePoolOptimized::LetterIndex_t common::CTriePoolOptimized::GetLetterAsIndex(const char letter)
{
  const auto result = letter - 'a';

  assert(result < C_CHILDREN_COUNT);

  return result;
}


inline char common::CTriePoolOptimized::GetIndexAsLetter(const LetterIndex_t letter)
{
  assert(letter < C_CHILDREN_COUNT);

  const auto result = letter + 'a';

  return result;
}


inline void common::CTriePoolOptimized::GetFoundWords
(
  const CTriePoolOptimized& currentTrie,
  TContainerWriter<const char*>& wordList,
  uint32_t& scoreOutput
)
{
  if (currentTrie.m_found)
  {
    wordList.container[wordList.currentIndex++] = currentTrie.m_word;
    scoreOutput += GetScoreForWordLength(strlen(currentTrie.m_word));
  }

  for (size_t i = 0; i < C_CHILDREN_COUNT; i++)
  {
    const Index_t childIndex = currentTrie.m_children[i];
    if (!childIndex)
    {
      continue;
    }

    const CTriePoolOptimized* pChild = GetPtrFromIndex(childIndex);

    GetFoundWords(*pChild, wordList, scoreOutput);
  }
}


inline uint32_t common::CTriePoolOptimized::GetFoundWordsSize(const CTriePoolOptimized& currentTrie)
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

    const CTriePoolOptimized* pChild = GetPtrFromIndex(childIndex);

#if defined(_DEBUG)
    assert(currentTrie.m_debugChildren.at(i) == pChild);
#endif

    result += GetFoundWordsSize(*pChild);
  }

  return result;
}


inline uint32_t common::CTriePoolOptimized::GetScoreForWordLength(const size_t wordLength)
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
