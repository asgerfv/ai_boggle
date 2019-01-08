#pragma once

#include "runtime_base.hpp"

#include <cassert>
#include <map>
#include <vector>
#include <set>
#include <string>


// ----------------------------------------------------------------------------


namespace boggle_game
{
  class CRuntime_Simple : public CRuntime_Base
  {
  public:

    CRuntime_Simple();
    virtual ~CRuntime_Simple();
    
    virtual void LoadDictionary(const char* path) override;
    virtual SBoggleResults FindWords(const char* board, uint32_t width, uint32_t height) override;

  private:
    typedef std::string DictinoryName_t;
    typedef std::set<DictinoryName_t> Dictionary_t;
    typedef std::map<DictinoryName_t, DictinoryName_t> RealDictionaryNameLookup_t;

    void CheckWord(const Dictionary_t::const_iterator wordToFind);

    bool DoSearchForMatchingNeighbourLetters(
      const uint32_t x,
      const uint32_t y,
      const Dictionary_t::const_iterator currentWord,
      const uint8_t levelIndex
    );


  private:
    Dictionary_t m_dictionary;

    std::vector<Dictionary_t::const_iterator> m_currentFoundWords;

    RealDictionaryNameLookup_t m_realDictionaryNameLookup;

    uint32_t m_currentScore = 0;
  };
}


// ----------------------------------------------------------------------------
