#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_VBK_BLOCK_TREE_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_VBK_BLOCK_TREE_HPP_

#include <utility>
#include <veriblock/blockchain/blocktree.hpp>
#include <veriblock/blockchain/pop/fork_resolution.hpp>
#include <veriblock/blockchain/pop/pop_state_machine.hpp>
#include <veriblock/blockchain/vbk_chain_params.hpp>
#include <veriblock/entities/btcblock.hpp>
#include <veriblock/finalizer.hpp>
#include <veriblock/state_manager.hpp>
#include <veriblock/storage/endorsement_repository.hpp>

namespace altintegration {

struct VbkBlockTree : public BlockTree<VbkBlock, VbkChainParams> {
  using VbkTree = BlockTree<VbkBlock, VbkChainParams>;
  using BtcTree = BlockTree<BtcBlock, BtcChainParams>;
  using index_t = VbkTree::index_t;
  using PopForkComparator =
      PopAwareForkResolutionComparator<VbkBlock, VbkChainParams, BtcTree>;

  ~VbkBlockTree() override = default;

  VbkBlockTree(const VbkChainParams& vbkp, const BtcChainParams& btcp)
      : VbkTree(vbkp), cmp_(BtcTree(btcp), btcp, vbkp) {}

  VbkBlockTree(const VbkBlockTree& tree) = default;

  VbkBlockTree& operator=(const VbkBlockTree& tree) {
    this->activeChain_ = tree.activeChain_;
    this->block_index_ = tree.block_index_;
    this->fork_chains_ = tree.fork_chains_;
    this->cmp_ = tree.cmp_;
    return *this;
  }

  BtcTree& btc() { return cmp_.getProtectingBlockTree(); }
  const BtcTree& btc() const { return cmp_.getProtectingBlockTree(); }

  const PopForkComparator& getComparator() const { return cmp_; }

  bool bootstrapWithChain(height_t startHeight,
                          const std::vector<block_t>& chain,
                          ValidationState& state) override;

  bool bootstrapWithGenesis(ValidationState& state) override;

  bool acceptBlock(const block_t& block,
                   const std::vector<payloads_t>& payloads,
                   ValidationState& state);

 private:
  void determineBestChain(Chain<index_t>& currentBest,
                          index_t& indexNew,
                          bool isBootstrap = false) override;

  PopForkComparator cmp_;
};

template <>
bool PopStateMachine<VbkBlockTree::BtcTree,
                     BlockIndex<VbkBlock>,
                     VbkChainParams>::applyContext(const BlockIndex<VbkBlock>&
                                                       index,
                                                   ValidationState& state);

template <>
void PopStateMachine<VbkBlockTree::BtcTree,
                     BlockIndex<VbkBlock>,
                     VbkChainParams>::unapplyContext(const BlockIndex<VbkBlock>&
                                                         index);

template <>
void addContextToBlockIndex(BlockIndex<VbkBlock>& index,
                            const typename BlockIndex<VbkBlock>::payloads_t& p,
                            const VbkBlockTree::BtcTree& tree);
/*
template <>
void removeContextFromBlockIndex(BlockIndex<VbkBlock>& index,
                                 const BlockIndex<VbkBlock>::payloads_t& p);
                                 */

}  // namespace altintegration

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_VBK_BLOCK_TREE_HPP_
