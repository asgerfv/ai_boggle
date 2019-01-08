#include "runtime_trie-threaded.hpp"

#include <common/utils.hpp>

#include <fstream>
#include <iostream>


// ----------------------------------------------------------------------------


boggle_game::CRuntime_TrieThreaded::CRuntime_TrieThreaded()
{
  common::CTriePoolOptimized::InitializePool();
}


boggle_game::CRuntime_TrieThreaded::~CRuntime_TrieThreaded()
{
  for (size_t i = 0; i < m_workers.size(); i++)
  {
    m_workers[i].pThread.release();
  }
  
  m_workers.clear();
  
  common::CTriePoolOptimized::ClearAllTries();
}


void boggle_game::CRuntime_TrieThreaded::LoadDictionary(const char* path)
{
  std::ifstream fileStream(path, std::ifstream::binary);
  if (!fileStream)
  {
    std::cout << "Couldn't open dictionary: " << path << std::endl;
    return;
  }

  m_workers.resize( std::thread::hardware_concurrency() );


  char threadLetter[common::CTriePoolOptimized::C_CHILDREN_COUNT];
  {
    uint8_t currentThreadIndex = 0;
    for (char& c : threadLetter)
    {
      c = (currentThreadIndex++) % m_workers.size();
    }
  }


  for (auto& var : m_workers)
  {
    var.runtime.InitializeTrieRoot();
  }


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

    const size_t currentWorkerIndex = threadLetter[ currentWord[0] - 'a' ];

    auto& currentWorker = m_workers[currentWorkerIndex].runtime;

    currentWorker.AddWord(currentWord);
  }
}


boggle_game::SBoggleResults boggle_game::CRuntime_TrieThreaded::FindWords(
  const char* pBoard,
  uint32_t width,
  uint32_t height)
{
  static const int32_t C_UNINITIALIZED = -1;

  std::vector<int32_t> wordCountsFromWorkers( m_workers.size(), C_UNINITIALIZED);

  for (size_t i = 1; i < m_workers.size(); i++)
  {
    m_workers[i].pThread.reset(
      new std::thread(
        OnThread, std::ref(m_workers[i].runtime), pBoard, width, height, std::ref(wordCountsFromWorkers[i])
      )
    );
  }

  OnThread(m_workers[0].runtime, pBoard, width, height, wordCountsFromWorkers[0]);


  for (size_t i = 1; i < m_workers.size(); i++)
  {
    m_workers[i].pThread->join();
  }


  for (const auto& val : wordCountsFromWorkers)
  {
    assert(val != C_UNINITIALIZED);
  }


  ///
  /// Provide the result struct ...
  ///


  uint32_t foundWordsCount = 0;
  {
    for (size_t i = 0; i < wordCountsFromWorkers.size(); i++)
    {
      foundWordsCount += wordCountsFromWorkers[i];
    }
  }


  
  SBoggleResults result;

  if (foundWordsCount)
  {
    result.words.resize(foundWordsCount, nullptr);

    uint32_t currentOffset = 0;

    for (size_t i = 0; i < m_workers.size(); i++)
    {
      common::TContainerWriter<const char*> containerWriter;
      containerWriter.container = result.words.data() + currentOffset;

      result.score += m_workers[i].runtime.RetrieveWordsAndGetScore(containerWriter);

      currentOffset += wordCountsFromWorkers[i];
    }
  }


  return result;
}


// ----------------------------------------------------------------------------


void boggle_game::CRuntime_TrieThreaded::OnThread(Runtime_t& runtime, const char* pBoard, uint32_t width, uint32_t height, int32_t& wordCountOut)
{
  wordCountOut = runtime.FindWordsAndStandbyForWordRetrieving(pBoard, width, height);
}
