#pragma once

#include "runtime_base.hpp"

#include <cassert>


// ----------------------------------------------------------------------------


namespace common
{
  class CTriePool;
}


// ----------------------------------------------------------------------------


namespace boggle_game
{
  class CRuntime_TriePool : public CRuntime_Base
  {
  public:

    CRuntime_TriePool();
    virtual ~CRuntime_TriePool();
    
    virtual void LoadDictionary(const char* path) override;
    virtual SBoggleResults FindWords(const char* pBoard, uint32_t width, uint32_t height) override;

  private:
    void DoSearchForWordsAtBoardLocation(
      const uint32_t x,
      const uint32_t y,
      common::CTriePool& currentTrie
    );

  private:
    common::CTriePool* m_pTrie = nullptr;
  };
}


// ----------------------------------------------------------------------------
