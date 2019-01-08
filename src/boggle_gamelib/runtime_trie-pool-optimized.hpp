#pragma once

#include "runtime_base.hpp"

#include <common/containers/trie-pool-optimized.hpp>

#include <cassert>


// ----------------------------------------------------------------------------


namespace common
{
  class CTriePoolOptimized;
}


// ----------------------------------------------------------------------------


namespace boggle_game
{
  class CRuntime_TriePoolOptimized : public CRuntime_Base
  {
  public:

    CRuntime_TriePoolOptimized();
    virtual ~CRuntime_TriePoolOptimized();
    
    virtual void LoadDictionary(const char* path) override;
    virtual SBoggleResults FindWords(const char* board, uint32_t width, uint32_t height) override;

    void InitializeTrieRoot();
    void AddWord(const std::string& word);
    uint32_t FindWordsAndStandbyForWordRetrieving(const char* board, unsigned width, unsigned height);
    uint32_t RetrieveWordsAndGetScore(common::TContainerWriter<const char*> output);

  private:
    void DoSearchForWordsAtBoardLocation(
      const uint32_t x,
      const uint32_t y,
      common::CTriePoolOptimized& currentTrie
    );

  private:
    common::CTriePoolOptimized* m_pTrie = nullptr;
  };
}


// ----------------------------------------------------------------------------


inline void boggle_game::CRuntime_TriePoolOptimized::AddWord(const std::string& word)
{
  assert(m_pTrie);

  m_pTrie->AddWord(word);
}
