#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_BLOCK_INDEX_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_BLOCK_INDEX_HPP_

#include <vector>

#include "veriblock/arith_uint256.hpp"
#include "veriblock/entities/btcblock.hpp"
#include "veriblock/write_stream.hpp"

namespace VeriBlock {

//! Store block
template <typename Block>
struct BlockIndex {
  using hash_t = typename Block::hash_t;
  using height_t = typename Block::height_t;

  //! (memory only) total amount of work in the chain up to and including this
  //! block
  ArithUint256 chainWork = 0;

  //! height of the entry in the chain
  uint32_t height = 0;

  //! block header
  Block header{};

  void toRaw(WriteStream& stream) {
    writeSingleByteLenValue(stream, chainWork);
    stream.writeBE<uint32_t>(height);
    header.toRaw(stream);
  }

  static StoredBlock fromRaw(ReadStream& stream) {
    StoredBlock index{};
    index.chainWork = readSingleByteLenValue(
        stream, ArithUint256::size(), ArithUint256::size());
    index.height = stream.readBE<uint32_t>();
    index.header = Block::fromRaw(stream);
    return index;
  }
};

}  // namespace VeriBlock
#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_BLOCK_INDEX_HPP_
