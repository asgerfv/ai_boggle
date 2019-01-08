#include "runtime_trie-pool-optimized.hpp"

#include <common/utils.hpp>

#include <fstream>
#include <iostream>


// ----------------------------------------------------------------------------


boggle_game::CRuntime_TriePoolOptimized::CRuntime_TriePoolOptimized()
{
}


boggle_game::CRuntime_TriePoolOptimized::~CRuntime_TriePoolOptimized()
{
}


void boggle_game::CRuntime_TriePoolOptimized::LoadDictionary(const char* path)
{
  std::ifstream fileStream(path, std::ifstream::binary);
  if (!fileStream)
  {
    std::cout << "Couldn't open dictionary: " << path << std::endl;
    return;
  }


  common::CTriePoolOptimized::InitializePool();

  InitializeTrieRoot();


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


boggle_game::SBoggleResults boggle_game::CRuntime_TriePoolOptimized::FindWords(
  const char* pBoard,
  uint32_t width,
  uint32_t height)
{
  const uint32_t currentFoundWordsSize = FindWordsAndStandbyForWordRetrieving(pBoard, width, height);


  ///
  /// Provide the result struct ...
  ///

  
  ///
  /// Word list, which needs to be allocated just for this purpose
  ///
  SBoggleResults result;

  if (currentFoundWordsSize)
  {
    result.words.resize(currentFoundWordsSize, nullptr);

    common::TContainerWriter<const char*> containerWriter;
    containerWriter.container = &result.words.at(0);

    result.score = RetrieveWordsAndGetScore(containerWriter);
  }


  return result;
}


// ----------------------------------------------------------------------------


void boggle_game::CRuntime_TriePoolOptimized::InitializeTrieRoot()
{
  assert(!m_pTrie);

  m_pTrie = common::CTriePoolOptimized::GetPtrFromIndex(
    common::CTriePoolOptimized::AllocateTrie());
}


uint32_t boggle_game::CRuntime_TriePoolOptimized::FindWordsAndStandbyForWordRetrieving(const char* board, unsigned width, unsigned height)
{
  InitializeBoard(board, width, height);

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

  const uint32_t currentFoundWordsSize = common::CTriePoolOptimized::GetFoundWordsSize(*m_pTrie);

  return currentFoundWordsSize;
}


uint32_t boggle_game::CRuntime_TriePoolOptimized::RetrieveWordsAndGetScore(common::TContainerWriter<const char*> output)
{
  uint32_t currentScore = 0;

  common::CTriePoolOptimized::GetFoundWords(*m_pTrie, output, currentScore);

  return currentScore;
}


// ----------------------------------------------------------------------------


void boggle_game::CRuntime_TriePoolOptimized::DoSearchForWordsAtBoardLocation(
  const uint32_t x,
  const uint32_t y,
  common::CTriePoolOptimized& currentTrie
)
{
  const char currentBoardLetter = GetBoardLetter(x, y);

  if (!currentBoardLetter)
  {
    return;
  }

  //TODO: Convert the entire board to this range
  const common::CTriePoolOptimized::LetterIndex_t letterAsIndex = common::CTriePoolOptimized::GetLetterAsIndex(currentBoardLetter);

  common::CTriePoolOptimized::Index_t foundTrie = currentTrie.FindTrieWithStartingLetter(letterAsIndex, currentTrie);

  if (!foundTrie)
  {
    return;
  }

  common::CTriePoolOptimized* pFoundTrie = common::CTriePoolOptimized::GetPtrFromIndex(foundTrie);

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
