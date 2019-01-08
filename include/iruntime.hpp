#pragma once


///
/// Interface for a runtime of the Boggle game
///

// ----------------------------------------------------------------------------


#include <cstdint>
#include <vector>


// ----------------------------------------------------------------------------


namespace boggle_game
{
  struct SBoggleResults
  {
    std::vector<const char*> words;
    uint32_t score = 0;
  };
}


// ----------------------------------------------------------------------------


namespace boggle_game
{
  class IRuntime
  {
  public:

    IRuntime() {}
    virtual ~IRuntime() {}

    virtual void LoadDictionary(const char* path) = 0;

    virtual SBoggleResults FindWords(const char* pBoardData, uint32_t width, uint32_t height) = 0;

  protected:
    static const uint8_t C_MAX_WORD_LENGTH = 15;
  };
}
