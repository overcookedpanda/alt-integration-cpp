#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_CHAIN_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_CHAIN_HPP_

#include <memory>
#include <vector>
#include <veriblock/btc/block_index.hpp>

namespace VeriBlock {

template <typename BlockPtr,
          typename = std::enable_if_t<std::is_pointer_v<BlockPtr>>>
struct Chain {
  std::vector<BlockPtr> chain;
};

}  // namespace VeriBlock

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_CHAIN_HPP_
