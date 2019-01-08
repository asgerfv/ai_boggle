#pragma once


#include <common/utils.hpp>

#include <array>
#include <cassert>
#include <cstdint>

#include <set>
#include <string>

#if defined(_DEBUG)
  #include <iostream>
#endif


// ----------------------------------------------------------------------------


namespace common
{
  class CTrie
  {
  public:

    typedef uint8_t LetterIndex_t;
    typedef std::set<const std::string*> WordList_t;

    // ----------------------------------------------------------------------------

    CTrie(CTrie* pParent = nullptr);
    CTrie(const CTrie&) = delete;
    CTrie(CTrie&&) = delete;
    ~CTrie();

    CTrie& operator = (const CTrie&) = delete;
    CTrie& operator = (CTrie&&) = delete;

    void AddWord(const std::string&);
    void Clear();

    /// Find the next Trie according to the letter index
    CTrie* FindTrieWithStartingLetter(const LetterIndex_t index, CTrie& parentTrie);

    int32_t GetRemainingWords() const;

    void DumpToScreen() const;

    static LetterIndex_t GetLetterAsIndex(const char letter);
    static char GetIndexAsLetter(const LetterIndex_t letter);

    static void GetFoundWords(const CTrie& currentTrie, TContainerWriter<const char*> wordList, uint32_t& scoreOutput);
    static uint32_t GetFoundWordsSize(const CTrie& currentTrie);

    static uint32_t GetScoreForWordLength(const size_t wordLength);

  private:
    static constexpr int32_t C_CHILDREN_COUNT = ('z' - 'a') + 1;
    static const uint32_t C_MAX_WORD_LENGTH = 15;

    std::array<CTrie*, C_CHILDREN_COUNT> m_children;

    CTrie* m_pParent;

    int32_t m_wordCount : 31;
    int32_t m_found : 1;

    std::string m_word;

#if defined(_DEBUG)
    static int32_t s_instanceCount;
#endif
  };
}


// ----------------------------------------------------------------------------


inline common::CTrie::CTrie(CTrie* pParent)
  : m_pParent(pParent)
  , m_wordCount(0)
  , m_found(false)
{
  m_children.fill(nullptr);

#if defined(_DEBUG)
  s_instanceCount++;
#endif
}


inline common::CTrie::~CTrie()
{
  Clear();

#if defined(_DEBUG)
  s_instanceCount--;
#endif
}


inline void common::CTrie::AddWord(const std::string& word)
{
  CTrie* pCurrentTrie = this;

  for (size_t i = 0; i < word.length(); i++)
  {
    const char currentLetter = word[i];
    const auto currentIndex = GetLetterAsIndex(currentLetter);

    if (currentLetter == 'q' && word[i + 1] == 'u')
    {
      i++;
    }

    pCurrentTrie->m_wordCount++;

    CTrie*& child = pCurrentTrie->m_children[currentIndex];
    if (!child)
    {
      child = new CTrie(pCurrentTrie);
    }

    pCurrentTrie = child;
  }

  pCurrentTrie->m_word = word;
}


inline void common::CTrie::Clear()
{
  m_pParent = nullptr;

  m_wordCount = 0;

  m_found = false;

  m_word.clear();

  for (size_t i = 0; i < m_children.size(); i++)
  {
    CTrie* pChild = m_children[i];

    if (!pChild)
    {
      continue;
    }

    pChild->Clear();

    delete m_children[i];
    m_children[i] = nullptr;
  }
}


inline common::CTrie* common::CTrie::FindTrieWithStartingLetter(const LetterIndex_t index, CTrie& parentTrie)
{
  assert(index < C_CHILDREN_COUNT);
  CTrie* pCurrentTrie = parentTrie.m_children[index];

  if (pCurrentTrie && !pCurrentTrie->m_word.empty() && !pCurrentTrie->m_found)
  {
    assert(pCurrentTrie->m_pParent->m_wordCount);

    pCurrentTrie->m_pParent->m_wordCount--;

    pCurrentTrie->m_found = true;
  }

  return pCurrentTrie;
}


inline int32_t common::CTrie::GetRemainingWords() const
{
  return m_wordCount;
}


inline void common::CTrie::DumpToScreen() const
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

    std::cout << "\tLetter " << int32_t(i) << " is " << GetIndexAsLetter(i) << " has word count: " << m_children[i]->m_wordCount << std::endl;
  }

#endif
}


inline common::CTrie::LetterIndex_t common::CTrie::GetLetterAsIndex(const char letter)
{
  const auto result = letter - 'a';

  assert(result < C_CHILDREN_COUNT);

  return result;
}


inline char common::CTrie::GetIndexAsLetter(const LetterIndex_t letter)
{
  assert(letter < C_CHILDREN_COUNT);

  const auto result = letter + 'a';

  return result;
}


inline void common::CTrie::GetFoundWords
(
  const CTrie& currentTrie,
  TContainerWriter<const char*> wordList,
  uint32_t& scoreOutput
)
{
  if (currentTrie.m_found)
  {
    wordList.container[wordList.currentIndex++] = currentTrie.m_word.c_str();
    scoreOutput += GetScoreForWordLength(currentTrie.m_word.length());
  }

  for (size_t i = 0; i < C_CHILDREN_COUNT; i++)
  {
    const CTrie* pChild = currentTrie.m_children[i];
    if (!pChild)
    {
      continue;
    }

    GetFoundWords(*pChild, wordList, scoreOutput);
  }
}


inline uint32_t common::CTrie::GetFoundWordsSize(const CTrie& currentTrie)
{
  uint32_t result = 0;

  if (currentTrie.m_found)
  {
    result++;
  }

  for (size_t i = 0; i < C_CHILDREN_COUNT; i++)
  {
    const CTrie* pChild = currentTrie.m_children[i];
    if (!pChild)
    {
      continue;
    }

    result += GetFoundWordsSize(*pChild);
  }

  return result;
}


inline uint32_t common::CTrie::GetScoreForWordLength(const size_t wordLength)
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
