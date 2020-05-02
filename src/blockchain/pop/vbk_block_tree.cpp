// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <veriblock/blockchain/commands/commands.hpp>
#include <veriblock/blockchain/pop/pop_utils.hpp>
#include <veriblock/blockchain/pop/vbk_block_tree.hpp>
#include <veriblock/finalizer.hpp>

namespace altintegration {

void VbkBlockTree::determineBestChain(Chain<index_t>& currentBest,
                                      index_t& indexNew,
                                      bool isBootstrap) {
  if (currentBest.tip() == &indexNew) {
    return;
  }

  // do not even try to do fork resolution with an invalid chain
  if (!indexNew.isValid()) {
    return;
  }

  if (currentBest.tip() == nullptr) {
    currentBest.setTip(&indexNew);
    onTipChanged(nullptr, indexNew, true);
    return;
  }

  auto ki = param_->getKeystoneInterval();
  const auto* forkKeystone =
      currentBest.findHighestKeystoneAtOrBeforeFork(&indexNew, ki);
  if ((forkKeystone == nullptr) || isBootstrap) {
    // we did not find fork... this can happen only during bootstrap
    return VbkTree::determineBestChain(currentBest, indexNew, isBootstrap);
  }

  int result = 0;
  auto* bestTip = currentBest.tip();
  if (isCrossedKeystoneBoundary(forkKeystone->height, indexNew.height, ki) &&
      isCrossedKeystoneBoundary(forkKeystone->height, bestTip->height, ki)) {
    // [vbk fork point keystone ... current tip]
    Chain<index_t> vbkCurrentSubchain(forkKeystone->height, currentBest.tip());
    // [vbk fork point keystone... new block]
    Chain<index_t> vbkOther(forkKeystone->height, &indexNew);

    result = cmp_.comparePopScore(*bestTip, vbkCurrentSubchain, vbkOther);
  }

  if (result == 0) {
    // pop scores are equal. do PoW fork resolution
    VbkTree::determineBestChain(currentBest, indexNew, isBootstrap);
  } else if (result < 0) {
    // other chain won!
    auto* prevTip = currentBest.tip();
    currentBest.setTip(&indexNew);
    onTipChanged(prevTip, indexNew, isBootstrap);
  } else {
    // current chain is better
  }
}

void VbkBlockTree::onTipChanged(index_t* from, index_t& to, bool isBootstrap) {
  if (from && !isBootstrap) {
    ValidationState state;
    bool ret = cmp_.setState(*from, to, state);
    assert(ret);
    (void)ret;
  }
}

bool VbkBlockTree::bootstrapWithChain(int startHeight,
                                      const std::vector<block_t>& chain,
                                      ValidationState& state) {
  if (!VbkTree::bootstrapWithChain(startHeight, chain, state)) {
    return state.Invalid("vbk-bootstrap-chain");
  }

  return true;
}

bool VbkBlockTree::bootstrapWithGenesis(ValidationState& state) {
  if (!VbkTree::bootstrapWithGenesis(state)) {
    return state.Invalid("vbk-bootstrap-genesis");
  }

  return true;
}

void VbkBlockTree::addPayloads(const VbkBlock& block,
                               const std::vector<payloads_t>& payloads) {
  auto hash = block.getHash();
  auto* index = VbkTree::getBlockIndex(hash);
  if (!index) {
    throw std::logic_error("addPayloads is called on unknown VBK block: " +
                           hash.toHex());
  }

  if (!index->isValid()) {
    // adding payloads to an invalid block will not result in a state change
    return;
  }

  for (const auto& p : payloads) {
    payloadsToCommands(*this, hash, p, index->commands);
  }

  // find all affected tips and do a fork resolution
  for (auto* tip : findValidTips<VbkBlock>(*index)) {
    determineBestChain(activeChain_, *tip);
  }
}

std::string VbkBlockTree::toPrettyString(size_t level) const {
  std::ostringstream ss;
  std::string pad(level, ' ');
  ss << VbkTree::toPrettyString(level) << "\n";
  ss << cmp_.toPrettyString(level + 2);
  ss << "}";
  return ss.str();
}

//void VbkBlockTree::removeSubtree(index_t& toRemove) {
//  VbkTree::removeSubtree(toRemove);
//}
//
//void VbkBlockTree::removeSubtree(const hash_t& hash) {
//  auto* index = VbkTree::getBlockIndex(hash);
//  if (!index) {
//    return;
//  }
//
//  return removeSubtree(*index);
//}

void VbkBlockTree::invalidateSubtree(const hash_t& h) {
  auto* index = VbkTree::getBlockIndex(h);
  if (!index) {
    return;
  }

  return invalidateSubtree(*index);
}

void VbkBlockTree::invalidateSubtree(index_t& toInvalidate,
                                     enum BlockStatus reason) {
  VbkTree::invalidateSubtree(toInvalidate, reason);
}

template <>
void payloadsToCommands<VbkBlockTree>(VbkBlockTree& tree,
                                      const VbkBlock::hash_t& containing,
                                      const VTB& p,
                                      std::vector<CommandPtr>& commands) {
  // process BTC context blocks
  for (const auto& b : p.transaction.blockOfProofContext) {
    addBlock(tree.btc(), b, commands);
  }
  // process block of proof
  addBlock(tree.btc(), p.transaction.blockOfProof, commands);

  // add endorsement
  auto e = BtcEndorsement::fromContainerPtr(p);
  auto cmd = std::make_shared<AddBtcEndorsement>(
      tree.btc(), tree, containing, std::move(e));
  commands.push_back(std::move(cmd));
}
}  // namespace altintegration
