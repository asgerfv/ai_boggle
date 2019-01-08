#include "runtime_trie-pool.hpp"

#include <common/containers/trie-pool.hpp>
#include <common/utils.hpp>

#include <fstream>
#include <iostream>


// ----------------------------------------------------------------------------


boggle_game::CRuntime_TriePool::CRuntime_TriePool()
{
  common::CTriePool::InitializePool();

  m_pTrie = common::details::GetPtrFromIndex(common::details::AllocateTrie());
}


boggle_game::CRuntime_TriePool::~CRuntime_TriePool()
{
  common::CTriePool::ClearAllTries();
}


void boggle_game::CRuntime_TriePool::LoadDictionary(const char* path)
{
  std::ifstream fileStream(path, std::ifstream::binary);
  if (!fileStream)
  {
    std::cout << "Couldn't open dictionary: " << path << std::endl;
    return;
  }

  
  common::CTriePool::ClearAllTries();
  common::CTriePool::InitializePool();

  
  std::string currentWord;
  while ( std::getline(fileStream, currentWord) )
  {
    if (currentWord.empty())
    {
      continue;
    }
    
    const char lastLetter = currentWord.at(currentWord.length() - 1);
    
    if (!isalpha(lastLetter))
    {
      currentWord.resize(currentWord.length() - 1);
    }

    /// According to the rules then a valid word must be 3 or more letters, so
    /// start by eliminating words with less letters than that.
    if (currentWord.length() < 3)
    {
      continue;
    }

    /// Our own limitation to optimize a lookup table.
    if (currentWord.length() > C_MAX_WORD_LENGTH)
    {
      continue;
    }

    //std::transform(currentWord.begin(), currentWord.end(), currentWord.begin(), ::tolower);

    m_pTrie->AddWord(currentWord);
  }
}


boggle_game::SBoggleResults boggle_game::CRuntime_TriePool::FindWords(
  const char* pBoard,
  uint32_t width,
  uint32_t height)
{
  InitializeBoard(pBoard, width, height);

  ///
  /// Check every fields in a linear fashion ...
  ///

  for (uint32_t y = 0; y < height; y++)
  {
    for (uint32_t x = 0; x < width; x++)
    {
      DoSearchForWordsAtBoardLocation(x, y, *m_pTrie);
    }
  }


  ///
  /// Provide the result struct ...
  ///

  ///
  /// Word list, which needs to be allocated just for this purpose
  ///
  const uint32_t currentFoundWordsSize = common::CTriePool::GetFoundWordsSize(*m_pTrie);
  
  SBoggleResults result;
  
  if (currentFoundWordsSize)
  {
    result.words.resize(currentFoundWordsSize, nullptr);

    common::TContainerWriter<const char*> containerWriter;
    containerWriter.container = result.words.data();

    common::CTriePool::GetFoundWords(*m_pTrie, containerWriter, result.score);
  }


  return result;
}


// ----------------------------------------------------------------------------


void boggle_game::CRuntime_TriePool::DoSearchForWordsAtBoardLocation(
  const uint32_t x,
  const uint32_t y,
  common::CTriePool& currentTrie
)
{
  const char currentBoardLetter = GetBoardLetter(x, y);

  if (!currentBoardLetter)
  {
    return;
  }

  //TODO: Convert the entire board to this range
  const common::CTriePool::LetterIndex_t letterAsIndex = common::CTriePool::GetLetterAsIndex(currentBoardLetter);

  common::CTriePool* pFoundTrie = currentTrie.FindTrieWithStartingLetter(letterAsIndex, currentTrie);

  if (pFoundTrie && pFoundTrie->GetRemainingWords() > 0)
  {
    ///
    /// Check all directions for a match
    ///

    enum Axis_t : uint8_t
    {
      X = 0,
      Y = 1,
      MAX
    };
    typedef int8_t Direction_t[Axis_t::MAX];

    static const Direction_t s_directions[] =
    {
      { -1,  0 },
      { -1, -1 },
      { 0, -1 },
      { 1, -1 },
      { 1,  0 },
      { 1,  1 },
      { 0,  1 },
      { -1,  1 },
    };

    SetBoardLetter(x, y, char(0));

    for (uint32_t i = 0; i < common::GetArrayLength(s_directions); i++)
    {
      const Direction_t& currentDirection = s_directions[i];
      const uint32_t currentX = x + currentDirection[Axis_t::X];
      const uint32_t currentY = y + currentDirection[Axis_t::Y];

      /// Edge detection. Notice how minus values underflow, and as such we only
      /// need to test for max, not minimum.
      if (currentX >= m_currentBoardWidth || currentY >= m_currentBoardHeight)
      {
        continue;
      }

      DoSearchForWordsAtBoardLocation(currentX, currentY, *pFoundTrie);
    }

    SetBoardLetter(x, y, currentBoardLetter);
  }
}
