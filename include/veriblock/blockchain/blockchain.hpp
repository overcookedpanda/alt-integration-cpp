#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_BLOCKCHAIN_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_BLOCKCHAIN_HPP_

#include <memory>
#include <veriblock/blockchain/block_index.hpp>
#include <veriblock/blockchain/fork_resolution.hpp>
#include <veriblock/stateless_validation.hpp>
#include <veriblock/storage/block_repository.hpp>
#include <veriblock/validation_state.hpp>

namespace VeriBlock {

template <typename Block>
struct Blockchain {
  using block_t = Block;
  using index_t = BlockIndex<block_t>;
  using hash_t = typename Block::hash_t;
  using height_t = typename Block::height_t;

  Blockchain(std::shared_ptr<BlockRepository<index_t>> repo)
      : blocks_(std::move(repo)) {}

  bool bootstrap(const std::vector<Block>& blocks) { (void)blocks; }

  bool acceptBlockHeader(const Block& block, ValidationState& state) {
    if (!checkBlock(block, state)) {
      return state.addStackFunction("acceptBlockHeader()");
    }

    if (!blocks_->contains(block.previousHash)) {
      return state.Invalid("acceptBlockHeader()",
                           "no-prev-block",
                           "can not find previous block");
    }

    // TODO: maybe some other validations?
    // - difficulty calculator(s)?
    // - block too new?
    // - block too old?

    insertBlockHeader(block);

    return true;
  }

 private:
  std::shared_ptr<BlockRepository<index_t>> blocks_;
  std::shared_ptr<index_t> pindexBestHeader = nullptr;

  index_t insertBlockHeader(const block_t& block) {
    index_t current;
    if (blocks_->getByHash(block, &current)) {
      // it is a duplicate
      return current;
    }

    current.header = block;

    index_t prev;
    if (blocks_->getByHash(block.previousHash, &prev)) {
      // prev block found
      current.height = prev.height + 1;
      current.chainWork = prev.chainWork + block.getBlockProof();
    }

    determineBestChain(&pindexBestHeader, current);

    blocks_->put(current);

    return current;
  }
};

}  // namespace VeriBlock

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_BLOCKCHAIN_HPP_
