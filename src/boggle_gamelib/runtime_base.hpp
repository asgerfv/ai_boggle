#pragma once

///
/// Interface for a runtime of the Boggle game
///

// ----------------------------------------------------------------------------


#include <include/iruntime.hpp>

#include <cassert>
#include <cstdint>
#include <vector>


// ----------------------------------------------------------------------------


namespace boggle_game
{
  class CRuntime_Base : public IRuntime
  {
  public:

    CRuntime_Base() {}
    virtual ~CRuntime_Base() {}

    void SetBoardLetter(const uint32_t x, const uint32_t y, const char newLetter);
    char GetBoardLetter(const uint32_t x, const uint32_t y) const;

    static uint32_t GetScoreForWordLength(const size_t wordLength);

  protected:
    void InitializeBoard(const char* pBoard, uint32_t width, uint32_t height);

  protected:
    std::vector<char> m_currentBoard;
    uint32_t m_currentBoardWidth = 0;
    uint32_t m_currentBoardHeight = 0;
  };
}


// ----------------------------------------------------------------------------


inline void boggle_game::CRuntime_Base::SetBoardLetter(const uint32_t x, const uint32_t y, const char newLetter)
{
  assert(!m_currentBoard.empty());
  assert(x < m_currentBoardWidth);
  assert(y < m_currentBoardHeight);

  m_currentBoard[x + (y * m_currentBoardHeight)] = newLetter;
}


inline char boggle_game::CRuntime_Base::GetBoardLetter(const uint32_t x, const uint32_t y) const
{
  assert(!m_currentBoard.empty());
  assert(x < m_currentBoardWidth);
  assert(y < m_currentBoardHeight);

  return m_currentBoard[x + (y * m_currentBoardHeight)];
}


// ----------------------------------------------------------------------------


inline uint32_t boggle_game::CRuntime_Base::GetScoreForWordLength(const size_t wordLength)
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


// ----------------------------------------------------------------------------


inline void boggle_game::CRuntime_Base::InitializeBoard
(
  const char* pBoard,
  uint32_t width,
  uint32_t height
)
{
  m_currentBoard.resize(width * height);
  memcpy(m_currentBoard.data(), pBoard, m_currentBoard.size());

  m_currentBoardWidth = width;
  m_currentBoardHeight = height;
}
