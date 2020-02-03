#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_BLOCKCHAIN_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_BLOCKCHAIN_HPP_

#include <memory>
#include <unordered_set>
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

  Blockchain(std::shared_ptr<BlockRepository<index_t>> repo,
             const index_t& bootstrapBlock)
      : blocks_(std::move(repo)) {
    // this one is to ensure that at given height we can have only one bootstrap
    // block
    blocks_->removeByHeight(bootstrapBlock.height);
    insertBlockHeader(bootstrapBlock, false);
    validateBlockchain(index);
  }

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

    insertBlockHeader(block, true);

    return true;
  }

 private:
  std::shared_ptr<BlockRepository<index_t>> blocks_;
  std::shared_ptr<index_t> pindexBestHeader = nullptr;

  index_t insertBlockHeader(const block_t& block, bool checkPrev = true) {
    index_t current;
    if (blocks_->getByHash(block, &current)) {
      // it is a duplicate
      return current;
    }

    current.header = block;

    index_t prev;
    if (!checkPrev) {
      // leave height as is
      current.chainWork = block.getBlockProof();
    } else if (blocks_->getByHash(block.previousHash, &prev)) {
      // prev block found
      current.height = prev.height + 1;
      current.chainWork = prev.chainWork + block.getBlockProof();
    }

    determineBestChain(&pindexBestHeader, current);

    blocks_->put(current);

    return current;
  }

  void validateBlockchain(const index_t& bootstrap) {
    auto batch = blocks_->newBatch();
    auto cursor = blocks_->getCursor();
    cursor.seek(bootstrap.height);

    // remove all blocks before bootstrap
    for (cursor.seekToFirst();
         cursor.isValid() && cursor.key() < bootstrap.height;
         cursor.next()) {
      batch->removeByHeight(cursor.key());
    }

    // remove all blocks after bootstrap that are not linked to bootstrap
    std::unordered_set<hash_t> seen_on_prev_height{bootstrap.header.getHash()};
    std::unordered_set<hash_t> seen_on_current_height{};
    height_t last_visited_height = bootstrap.height;
    for (cursor.seek(bootstrap.height + 1); cursor.isValid(); cursor.next()) {
      auto val = cursor.value();
      auto hash = val.header.getHash();
      if (seen_on_prev_height.find(val.header.previousBlock) !=
          seen_on_prev_height.end()) {
        // linked
        seen_on_current_height.insert({val.header, hash});
      } else {
        // not linked
        batch->removeByHash(hash);
      }

      if (last_visited_height < val.height) {
        // we switched to next height
        last_visited_height = val.height;
        seen_on_current_height.swap(seen_on_prev_height);
        seen_on_current_height.clear();
      }
    }

    blocks_->commit(*batch);
  }
};

}  // namespace VeriBlock

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_BLOCKCHAIN_HPP_
