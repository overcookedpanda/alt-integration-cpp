// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef ALTINTEGRATION_POP_STATE_MACHINE_HPP
#define ALTINTEGRATION_POP_STATE_MACHINE_HPP

#include <functional>
#include <veriblock/blockchain/chain.hpp>
#include <veriblock/blockchain/command_history.hpp>
#include <veriblock/storage/payloads_repository.hpp>

namespace altintegration {

/// @invariant NOT atomic - given a block tree, if any of functions fail, state
/// is NOT changed back. It you care about the state of your tree, COPY your
/// block tree and use copy in this state machine here.
template <typename ProtectingBlockTree,
          typename ProtectedIndex,
          typename ProtectedChainParams>
struct PopStateMachine {
  using index_t = ProtectedIndex;
  using payloads_t = typename ProtectedIndex::payloads_t;
  using height_t = typename ProtectedIndex::height_t;
  using endorsement_t = typename ProtectedIndex::endorsement_t;

  PopStateMachine(ProtectingBlockTree& tree,
                  const ProtectedChainParams& protectedParams,
                  height_t startHeight = 0)
      : tree_(tree),
        protectedParams_(&protectedParams),
        startHeight_(startHeight) {}

  bool applyBlock(const index_t& index, ValidationState& state) {
    for (const auto& cmd : index.commands) {
      if (!cmd->Execute(state)) {
        return false;
      }
    }
    return true;
  }

  void unapplyBlock(const index_t& index) {
    auto& v = index.commands;
    std::for_each(
        v.rbegin(), v.rend(), [](const CommandPtr& cmd) { cmd->UnExecute(); });
  }

  // unapplies commands in range [from; to)
  void unapply(ProtectedIndex& from, ProtectedIndex& to) {
    assert(from.height > to.height);
    // exclude 'to' by adding 1
    Chain<ProtectedIndex> chain(to.height + 1, from);
    assert(chain.first());
    assert(chain.first()->pprev == &to);

    std::for_each(chain.rbegin(), chain.rend(), [&](ProtectedIndex& current) {
      unapplyBlock(*current);
    });
  }

  // applies commands in range (from; to].
  std::pair<bool, ProtectedIndex*> apply(ProtectedIndex& from, ProtectedIndex& to, ValidationState& state) {
    assert(from.height < to.height);
    // exclude 'from' by adding 1
    Chain<ProtectedIndex> chain(from.height + 1, &to);
    assert(chain.first());
    assert(chain.first()->pprev == &from);

    for (auto it = chain.begin(), end = chain.end(); it != end; ++it) {
      auto* index = *it;
      if (!index->isValid()) {
        unapply(*index, from);
        return {false, nullptr};
      }

      if (!applyBlock(*index, state)) {
        unapply(*index, from);
        return {false, index};
      }
    }

    // this subchain is valid
    return true;
  }

  ProtectingBlockTree& tree() { return tree_; }
  const ProtectingBlockTree& tree() const { return tree_; }
  const ProtectedChainParams& params() const { return *protectedParams_; }

 private:
  ProtectingBlockTree& tree_;
  const ProtectedChainParams* protectedParams_;
  height_t startHeight_ = 0;
};

}  // namespace altintegration

#endif  // ALTINTEGRATION_POP_STATE_MACHINE_HPP
