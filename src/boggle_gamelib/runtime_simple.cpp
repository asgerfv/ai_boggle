#include "runtime_simple.hpp"

#include <common/utils.hpp>

#include <fstream>
#include <iostream>


// ----------------------------------------------------------------------------


boggle_game::CRuntime_Simple::CRuntime_Simple()
{
}


boggle_game::CRuntime_Simple::~CRuntime_Simple()
{
}


void boggle_game::CRuntime_Simple::LoadDictionary(const char* path)
{
  std::ifstream fileStream(path, std::ifstream::binary);
  if (!fileStream)
  {
    std::cout << "Couldn't open dictionary: " << path << std::endl;
    return;
  }

  
  std::string currentWord;
  std::string realCurrentWord;
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

    const size_t currentWordLength = currentWord.length();

    /// According to the rules then a valid word must be 3 or more letters, so
    /// start by eliminating words with less letters than that.
    if (currentWordLength < 3)
    {
      continue;
    }

    /// Our own limitation to optimize a lookup table.
    if (currentWordLength > C_MAX_WORD_LENGTH)
    {
      continue;
    }

    size_t i = 0;
    for (; i < currentWordLength; i++)
    {
      if (currentWord[i] == 'q' && currentWord[i + 1] == 'u')
      {
        if (realCurrentWord.empty())
        {
          realCurrentWord = currentWord;
        }

        currentWord.erase(i + 1, 1);

        /// Notice a lack of "Early out" here - this is because some words
        /// have more than 1 pair of QUs, like: "equivoque"
      }
    }

    const auto insertResult = m_dictionary.insert(currentWord);

    /// We changed to word to make traversing much faster, but that ruined the real word,
    /// so we have to save this real word as well.
    if (!realCurrentWord.empty())
    {
      m_realDictionaryNameLookup[currentWord] = realCurrentWord;
      realCurrentWord.clear();
    }
  }
}


boggle_game::SBoggleResults boggle_game::CRuntime_Simple::FindWords(
  const char* pBoard,
  uint32_t width,
  uint32_t height)
{
  InitializeBoard(pBoard, width, height);

  m_currentFoundWords.clear();
  m_currentScore = 0;


  ///
  /// Check every fields in a linear fashion ...
  ///

  for (auto i = m_dictionary.cbegin(); i != m_dictionary.cend(); i++)
  {
    CheckWord(i);
  }


  ///
  /// Provide the result struct ...
  ///

  ///
  /// Word list, which needs to be allocated just for this purpose
  ///
  const uint32_t currentFoundWordsSize = static_cast<uint32_t>(m_currentFoundWords.size());
  
  SBoggleResults result;
  result.score = m_currentScore;
  
  if (currentFoundWordsSize)
  {
    result.words.resize(currentFoundWordsSize, nullptr);

    for (size_t i = 0; i < currentFoundWordsSize; i++)
    {
      Dictionary_t::iterator wordAtItr = m_currentFoundWords[i];

      assert(wordAtItr != m_dictionary.cend());

      const auto realWordItr = m_realDictionaryNameLookup.find(*wordAtItr);
      if (realWordItr != m_realDictionaryNameLookup.cend())
      {
        result.words[i] = realWordItr->second.c_str();
      }
      else
      {
        result.words[i] = m_currentFoundWords[i]->c_str();
      }
    }
  }


  return result;
}


// ----------------------------------------------------------------------------


void boggle_game::CRuntime_Simple::CheckWord(const Dictionary_t::const_iterator wordToFind)
{
  const uint32_t width = m_currentBoardWidth;
  const uint32_t height = m_currentBoardHeight;

  const char firstLetterInCurrentDictionaryWord = wordToFind->c_str()[0];


  for (uint32_t y = 0; y < height; y++)
  {
    for (uint32_t x = 0; x < width; x++)
    {
      ///
      /// Do we have a beginning of our word?
      ///
      const char currentBoardLetter = GetBoardLetter(x, y);
      if (currentBoardLetter != firstLetterInCurrentDictionaryWord)
      {
        continue;
      }


      ///
      /// Remove the letter from the board to ensure that it isn't found again
      /// in the search algorithm below.
      ///
      SetBoardLetter(x, y, char(0));

      ///
      /// Yes, the first letter matched. Now let's recursively search for the
      /// remaining letters.
      ///
      const bool foundWord = DoSearchForMatchingNeighbourLetters(x, y, wordToFind, 1);

      ///
      /// Put our letter back in
      ///
      SetBoardLetter(x, y, currentBoardLetter);


      if (foundWord)
      {
        return;
      }
    }
  }
}


bool boggle_game::CRuntime_Simple::DoSearchForMatchingNeighbourLetters(
  const uint32_t x,
  const uint32_t y,
  const Dictionary_t::const_iterator currentWord,
  const uint8_t levelIndex)
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
    {-1,  0},
    {-1, -1},
    { 0, -1},
    { 1, -1},
    { 1,  0},
    { 1,  1},
    { 0,  1},
    {-1,  1},
  };

  const size_t currentWordLength = currentWord->length();

  if (levelIndex == currentWordLength)
  {
    m_currentFoundWords.push_back(currentWord);

    const auto itr = m_realDictionaryNameLookup.find(*currentWord);
    if (itr == m_realDictionaryNameLookup.cend())
    {
      m_currentScore += GetScoreForWordLength(currentWordLength);
    }
    else
    {
      m_currentScore += GetScoreForWordLength(itr->second.length());
    }

    return true;
  }

  assert(levelIndex < currentWordLength  &&  "Logical error. Perhaps we skipped an index?");


  const char* pCurrentWord = currentWord->c_str();
  const char wantLetter = pCurrentWord[levelIndex];

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

    const char currentBoardLetter = GetBoardLetter(currentX, currentY);

    if (currentBoardLetter != wantLetter)
    {
      continue;
    }


    SetBoardLetter(currentX, currentY, char(0));

    const bool result = DoSearchForMatchingNeighbourLetters(
      currentX,
      currentY,
      currentWord,
      levelIndex + 1);

    SetBoardLetter(currentX, currentY, wantLetter);

    if (result)
    {
      return true;
    }
  }

  return false;
}
