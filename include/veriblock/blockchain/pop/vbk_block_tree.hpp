// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

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
  using endorsement_t = typename index_t::endorsement_t;
  using context_t = typename index_t::block_t::context_t;
  using PopForkComparator =
      PopAwareForkResolutionComparator<VbkBlock, VbkChainParams, BtcTree>;

  ~VbkBlockTree() override = default;

  VbkBlockTree(const VbkChainParams& vbkp, const BtcChainParams& btcp)
      : VbkTree(vbkp), cmp_(BtcTree(btcp), btcp, vbkp) {}

  BtcTree& btc() { return cmp_.getProtectingBlockTree(); }
  const BtcTree& btc() const { return cmp_.getProtectingBlockTree(); }

  PopForkComparator& getComparator() { return cmp_; }
  const PopForkComparator& getComparator() const { return cmp_; }

//  void removeSubtree(const hash_t& h);
//  void removeSubtree(index_t&);
  void invalidateSubtree(const hash_t& h);
  void invalidateSubtree(index_t&, enum BlockStatus reason = BLOCK_FAILED_POP);

  bool bootstrapWithChain(height_t startHeight,
                          const std::vector<block_t>& chain,
                          ValidationState& state) override;

  bool bootstrapWithGenesis(ValidationState& state) override;

  void addPayloads(const block_t& block,
                   const std::vector<payloads_t>& payloads);

  void onTipChanged(index_t* from, index_t& to, bool isBootstrap) override;

  bool operator==(const VbkBlockTree& o) const {
    return cmp_ == o.cmp_ && VbkTree::operator==(o);
  }

  std::string toPrettyString(size_t level = 0) const;

 private:
  void determineBestChain(Chain<index_t>& currentBest,
                          index_t& indexNew,
                          bool isBootstrap = false) override;

  PopForkComparator cmp_;
};

template <>
void payloadsToCommands<VbkBlockTree>(
    VbkBlockTree& tree,
    const typename VbkBlockTree::hash_t& containing,
    const typename VbkBlockTree::payloads_t& p,
    std::vector<CommandPtr>& commands);

}  // namespace altintegration

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_VBK_BLOCK_TREE_HPP_
