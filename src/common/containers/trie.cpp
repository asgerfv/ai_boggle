#include "trie.hpp"


// ----------------------------------------------------------------------------


/// Note: Could also have used "__declspec(selectany)" in the header, but this is
/// more cross-platform.
#if defined(_DEBUG)

  int32_t common::CTrie::s_instanceCount = 0;

#endif


// ----------------------------------------------------------------------------
