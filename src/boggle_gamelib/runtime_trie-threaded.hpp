#pragma once

#include "runtime_trie-pool-optimized.hpp"

#include <cassert>
#include <thread>


// ----------------------------------------------------------------------------


namespace boggle_game
{
  class CRuntime_TrieThreaded : public IRuntime
  {
  public:

    CRuntime_TrieThreaded();
    virtual ~CRuntime_TrieThreaded();
    
    virtual void LoadDictionary(const char* path) override;
    virtual SBoggleResults FindWords(const char* board, uint32_t width, uint32_t height) override;

  private:
    typedef CRuntime_TriePoolOptimized Runtime_t;
    static void OnThread(Runtime_t& runtime, const char* pBoard, uint32_t width, uint32_t height, int32_t& wordCountOut);

  private:
    struct WorkerEntry_t
    {
      Runtime_t runtime;
      std::unique_ptr<std::thread> pThread;
    };

    std::vector<WorkerEntry_t> m_workers;
  };
}


// ----------------------------------------------------------------------------
