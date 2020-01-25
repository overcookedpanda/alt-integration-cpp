#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_BLOCK_INDEX_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_BLOCK_INDEX_HPP_

#include <vector>

#include "veriblock/entities/btcblock.hpp"
#include "veriblock/write_stream.hpp"

namespace VeriBlock {
namespace btc {

using arith_uint256 = int;  // TODO: change to real arith uint

//! Similar to CBlockIndex from Bitcoin
struct BlockIndex {
  //! (memory only) total amount of work in the chain up to and including this
  //! block
  arith_uint256 chainWork = 0;

  //! (memory only) pointer to the predecessor block
  BlockIndex* prev = nullptr;

  //! height of the entry in the chain
  uint32_t height = 0;

  //! bitcoin block header
  BtcBlock header{};

  void toRaw(WriteStream& stream) {
    stream.writeBE<uint32_t>(height);
    header.toRaw(stream);
  }

  static BlockIndex fromRaw(ReadStream& stream) {
    BlockIndex index{};
    index.height = stream.readBE<uint32_t>();
    index.header = BtcBlock::fromRaw(stream);
    return index;
  }
};

}  // namespace btc
}  // namespace VeriBlock
#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_BLOCK_INDEX_HPP_
