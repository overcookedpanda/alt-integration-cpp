#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_BLOCK_INDEX_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_BLOCK_INDEX_HPP_

#include <vector>

#include "veriblock/entities/btcblock.hpp"

namespace VeriBlock {namespace blockchain {

using arith_uint256 = int; // TODO: change to real arith uint

//! Similar to CBlockIndex from Bitcoin
struct BlockIndex {

  //! total amount of work in the chain up to and including this block
  arith_uint256 chainWork = 0;

  //! height of the entry in the chain
  int height = 0;

  //! pointer to the index of predecessor of this block
  BlockIndex* prev = nullptr;

  //! total number of transactions (regular with pop) in a block
  size_t totalTxes = 0;

  //! number of pop transactions in a block
  size_t popTxes = 0;

  //! bitcoin block header
  BtcBlock header{};
};

}}
#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_BLOCK_INDEX_HPP_
