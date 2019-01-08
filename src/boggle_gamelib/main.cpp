#include "runtime_simple.hpp"
#include "runtime_trie.hpp"
#include "runtime_trie-pool.hpp"
#include "runtime_trie-pool-optimized.hpp"
#include "runtime_trie-threaded.hpp"

#include <include/runtimefactory.hpp>

#include <memory>


// ----------------------------------------------------------------------------


std::unique_ptr<boggle_game::IRuntime> boggle_game::CreateRuntimeSolver(const EBoggleSolver solver)
{
  switch (solver)
  {
    case EBoggleSolver::Simple:
    {
      return std::make_unique<CRuntime_Simple>();
    }

    case EBoggleSolver::Trie:
    {
      return std::make_unique<CRuntime_Trie>();
    }

    case EBoggleSolver::TriePool:
    {
      return std::make_unique<CRuntime_TriePool>();
    }

    case EBoggleSolver::TriePoolOptimized:
    {
      return std::make_unique<CRuntime_TriePoolOptimized>();
    }

    case EBoggleSolver::TrieThreaded:
    {
      return std::make_unique<CRuntime_TrieThreaded>();
    }

    default:
    {
      assert(false && "Missing case");
      break;
    }
  }

  return nullptr;
}


// ----------------------------------------------------------------------------
