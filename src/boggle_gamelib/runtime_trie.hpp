#pragma once

#include "runtime_base.hpp"

#include <common/containers/trie.hpp>

#include <cassert>
#include <map>
#include <vector>


// ----------------------------------------------------------------------------


namespace boggle_game
{

  class CRuntime_Trie : public CRuntime_Base
  {
  public:

    CRuntime_Trie();
    virtual ~CRuntime_Trie();
    
    virtual void LoadDictionary(const char* path) override;
    virtual SBoggleResults FindWords(const char* board, uint32_t width, uint32_t height) override;

  private:
    void DoSearchForWordsAtBoardLocation(
      const uint32_t x,
      const uint32_t y,
      common::CTrie& currentTrie
    );

  private:
    common::CTrie m_trie;
  };
}


// ----------------------------------------------------------------------------
