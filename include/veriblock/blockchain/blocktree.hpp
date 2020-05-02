// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_BLOCKTREE_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_BLOCKTREE_HPP_

#include <algorithm>
#include <memory>
#include <unordered_map>
#include <veriblock/blockchain/base_block_tree.hpp>
#include <veriblock/blockchain/block_index.hpp>
#include <veriblock/blockchain/blockchain_util.hpp>
#include <veriblock/blockchain/chain.hpp>
#include <veriblock/blockchain/tree_algo.hpp>
#include <veriblock/stateless_validation.hpp>
#include <veriblock/storage/block_repository.hpp>
#include <veriblock/validation_state.hpp>

namespace altintegration {

/**
 * BlockTree is a tree of blocks with single "bootstrap" block as root.
 * @tparam Block
 */
template <typename Block, typename ChainParams>
struct BlockTree : public BaseBlockTree<Block> {
  using base = BaseBlockTree<Block>;
  using block_t = Block;
  using params_t = ChainParams;
  using index_t = BlockIndex<block_t>;
  using hash_t = typename Block::hash_t;
  using prev_block_hash_t = decltype(Block::previousBlock);
  using height_t = typename Block::height_t;
  using payloads_t = typename block_t::payloads_t;

  ~BlockTree() override = default;

  BlockTree(const ChainParams& param) : param_(&param) {}

  const ChainParams& getParams() const { return *param_; }

  /**
   * Bootstrap blockchain with a single genesis block, from "chain parameters"
   * passed in constructor.
   *
   * This function does all blockchain integrity checks, does blockchain cleanup
   * and in general, very slow.
   *
   * @return true if bootstrap was successful, false otherwise
   */
  virtual bool bootstrapWithGenesis(ValidationState& state) {
    assert(base::blocks_.empty() && "already bootstrapped");
    auto block = param_->getGenesisBlock();
    return this->bootstrap(0, block, state);
  }

  /**
   * Bootstrap network with a chain that starts at 'startHeight'
   *
   * This function does all blockchain integrity checks, does blockchain cleanup
   * and in general, very slow.
   *
   * @param startHeight start height of the chain
   * @param chain bootstrap chain
   * @return true if bootstrap was successful, false otherwise
   */
  virtual bool bootstrapWithChain(height_t startHeight,
                                  const std::vector<block_t>& chain,
                                  ValidationState& state) {
    assert(base::blocks_.empty() && "already bootstrapped");
    if (chain.empty()) {
      return state.Invalid("bootstrap-empty-chain",
                           "provided bootstrap chain is empty");
    }

    if (chain.size() < param_->numBlocksForBootstrap()) {
      return state.Invalid(
          "bootstrap-small-chain",
          "number of blocks in the provided chain is too small: " +
              std::to_string(chain.size()) + ", expected at least " +
              std::to_string(param_->numBlocksForBootstrap()));
    }

    // pick first block from the chain, bootstrap with a single block
    auto genesis = chain[0];
    if (!this->bootstrap(startHeight, genesis, state)) {
      return state.Invalid("blocktree-bootstrap");
    }

    // apply the rest of the blocks from the chain on top of our bootstrap
    // block. disable difficulty checks, because we have not enough blocks in
    // our store (yet) to check it correctly
    for (size_t i = 1, size = chain.size(); i < size; i++) {
      auto& block = chain[i];
      if (!this->acceptBlock(std::make_shared<block_t>(block), state, false)) {
        return state.Invalid("blocktree-accept");
      }
    }

    return true;
  }

  bool acceptBlock(const block_t& block, ValidationState& state) {
    return acceptBlock(std::make_shared<block_t>(block), state, true);
  }

  bool acceptBlock(const std::shared_ptr<block_t>& block,
                   ValidationState& state) {
    return acceptBlock(block, state, true);
  }

  void removeSubtree(const hash_t& hash) {
    auto* index = base::getBlockIndex(hash);
    if (!index) {
      return;
    }
    return removeSubtree(*index, [] { /* do nothing */ });
  }

  void removeSubtree(index_t& blockIndex) {
    bool isOnMainChain = activeChain_.contains(&blockIndex);
    if (isOnMainChain) {
      activeChain_.setTip(blockIndex.pprev);
    }

    base::removeSubtree(blockIndex, [] { /* do nothing */ });

    if (isOnMainChain) {
      // find new best chain among remaining forks
      for (auto it = base::tips_.begin(); it != base::tips_.end();) {
        index_t* index = *it;
        if (!index->isValid()) {
          it = base::tips_.erase(it);
        } else {
          determineBestChain(activeChain_, *index);
          ++it;
        }
      }
    }
  }

  void invalidateTip(enum BlockStatus reason = BLOCK_FAILED_BLOCK) {
    auto* tip = getBestChain().tip();
    if (tip == nullptr) {
      // trying to invalidate a tip on empty tree
      return;
    }

    invalidateSubtree(*tip, reason);
  }

  void invalidateSubtree(index_t& blockIndex,
                         enum BlockStatus reason = BLOCK_FAILED_BLOCK) {
    bool isOnMainChain = activeChain_.contains(&blockIndex);
    if (isOnMainChain) {
      activeChain_.setTip(blockIndex.pprev);
    }

    base::invalidateBlock(blockIndex, reason, [](index_t& index) {
      // when we encounter an invalid block, we can stop
      // traversal, as any future blocks in that subtree will have
      // at least BLOCK_FAILED_CHILD
      return index.isValid();
    });

    // find new best chain among remaining forks
    for (auto it = base::tips_.begin(); it != base::tips_.end();) {
      index_t* index = *it;
      if (!index->isValid()) {
        it = base::tips_.erase(it);
      } else {
        determineBestChain(activeChain_, *index);
        ++it;
      }
    }
  }

  void invalidateBlock(const hash_t& blockHash,
                       enum BlockStatus reason = BLOCK_FAILED_BLOCK) {
    index_t* blockIndex = base::getBlockIndex(blockHash);
    if (blockIndex == nullptr) {
      return;
    }

    invalidateSubtree(*blockIndex, reason);
  }

  const Chain<index_t>& getBestChain() const { return this->activeChain_; }

  std::string toPrettyString(size_t level = 0) const {
    std::ostringstream s;
    std::string pad(level, ' ');
    s << pad << Block::name() << "BlockTree{blocks=" << base::blocks_.size()
      << "\n";
    s << pad << "{tip=\n";
    s << activeChain_.tip()->toPrettyString(level + 2) << "\n";
    s << base::toPrettyString(level + 2) << "\n";
    s << pad << "}";
    return s.str();
  }

  bool operator==(const BlockTree&) const {
    return false;  // TODO
  }

 protected:
  Chain<index_t> activeChain_;
  //! a set of tips among all blocks
  const ChainParams* param_ = nullptr;

  index_t* insertBlockHeader(const std::shared_ptr<block_t>& block) {
    auto hash = block->getHash();
    index_t* current = base::getBlockIndex(hash);
    if (current != nullptr) {
      // it is a duplicate
      return current;
    }

    current = base::doInsertBlockHeader(block);
    if (current->pprev) {
      current->chainWork = current->pprev->chainWork + getBlockProof(*block);
    } else {
      current->chainWork = getBlockProof(*block);
    }

    current->raiseValidity(BLOCK_VALID_TREE);

    return current;
  }

  bool acceptBlock(const std::shared_ptr<block_t>& block,
                   ValidationState& state,
                   bool shouldContextuallyCheck) {
    index_t* index = nullptr;
    if (!validateAndAddBlock(block, state, shouldContextuallyCheck, &index)) {
      return false;
    }

    bool isBootstrap = !shouldContextuallyCheck;
    determineBestChain(activeChain_, *index, isBootstrap);

    return true;
  }

  bool bootstrap(height_t height,
                 const block_t& block,
                 ValidationState& state) {
    return bootstrap(height, std::make_shared<block_t>(block), state);
  }

  bool bootstrap(height_t height,
                 const std::shared_ptr<block_t>& block,
                 ValidationState& state) {
    if (!checkBlock(*block, state, *param_)) {
      return state.Invalid("bootstrap");
    }

    auto* index = insertBlockHeader(block);
    index->height = height;

    activeChain_ = Chain<index_t>(height, index);

    if (!base::blocks_.empty() && !base::getBlockIndex(block->getHash())) {
      return state.Error("block-index-no-genesis");
    }

    return true;
  }

  bool validateAndAddBlock(const std::shared_ptr<block_t>& block,
                           ValidationState& state,
                           bool shouldContextuallyCheck,
                           index_t** ret) {
    if (!checkBlock(*block, state, *param_)) {
      return state.Invalid("check-block");
    }

    // we must know previous block
    auto* prev = base::getBlockIndex(block->previousBlock);
    if (prev == nullptr) {
      return state.Invalid("bad-prev-block", "can not find previous block");
    }

    if (shouldContextuallyCheck &&
        !contextuallyCheckBlock(*prev, *block, state, *param_)) {
      return state.Invalid("contextually-check-block");
    }

    auto index = this->insertBlockHeader(block);
    assert(index != nullptr &&
           "insertBlockHeader should have never returned nullptr");

    if (ret) {
      *ret = index;
    }

    // if prev block is invalid, mark this block as invalid
    if (!prev->isValid()) {
      index->setFlag(BLOCK_FAILED_CHILD);
      return state.Invalid("bad-chain", "One of previous blocks is invalid");
    }

    return true;
  }

  virtual void determineBestChain(Chain<index_t>& currentBest,
                                  index_t& indexNew,
                                  bool isBootstrap = false) {
    if (currentBest.tip() == &indexNew) {
      return;
    }

    // do not even consider invalid indices
    if (!indexNew.isValid()) {
      return;
    }

    auto* prev = currentBest.tip();
    if (prev == nullptr || prev->chainWork < indexNew.chainWork) {
      currentBest.setTip(&indexNew);
      onTipChanged(prev, indexNew, isBootstrap);
    }
  }

  //! callback, executed every time when tip is changed. Useful for derived
  //! classes.
  virtual void onTipChanged(index_t* from, index_t& to, bool isBootstrap) {
    (void)from;
    (void)to;
    (void)isBootstrap;
  }
};

}  // namespace altintegration

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_BLOCKTREE_HPP_
