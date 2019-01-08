#pragma once


///
/// Factory for a given technique
///

// ----------------------------------------------------------------------------


#include <cstdint>
#include <memory>


// ----------------------------------------------------------------------------


namespace boggle_game
{
  enum class EBoggleSolver
  {
    Simple,
    Trie,
    TriePool,
    TriePoolOptimized,
    TrieThreaded,
  };

  class IRuntime;


  // ----------------------------------------------------------------------------


  /// Retrieve a new interface. Notice the unique_ptr to indicate how
  /// ownership is transfered to the caller.
  std::unique_ptr<IRuntime> CreateRuntimeSolver(const EBoggleSolver);
}


// ----------------------------------------------------------------------------
