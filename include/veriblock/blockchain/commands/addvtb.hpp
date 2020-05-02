// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef ALTINTEGRATION_ADDVTB_HPP
#define ALTINTEGRATION_ADDVTB_HPP

#include <vector>
#include <veriblock/blockchain/alt_block_tree.hpp>
#include <veriblock/entities/altblock.hpp>

namespace altintegration {

struct AddVTB : public Command {
  ~AddVTB() override = default;

  AddVTB(AltTree& tree, const VTB& vtb) : tree_(&tree) {
    vbkContaining_ = vtb.containingBlock.getHash();
    // interpret vtb into a vector of commands
    payloadsToCommands(tree.vbk(), vbkContaining_, vtb, containingCmds_);
  }

  bool Execute(ValidationState& state) override {
    auto* index = tree_->vbk().getBlockIndex(vbkContaining_);
    if (!index) {
      return state.Invalid(
          "vbk-containing-not-found",
          "Can not find VTB containing block " + vbkContaining_.toHex());
    }

    // add commands to VBK containing block
    auto& c = index->commands;
    c.insert(c.end(), containingCmds_.begin(), containingCmds_.end());
  }
  void UnExecute() override {
    auto* index = tree_->vbk().getBlockIndex(vbkContaining_);
    if (!index) {
      // this block no longer exists, do not remove anything
      return;
    }

    auto& c = index->commands;

    assert(!containingCmds_.empty());

    // as we add commands
    auto it = std::find(c.begin(), c.end(), containingCmds_[0]);
    if (it != c.end()) {
      // we always add ranges of commands, so try to find first matching command
      // and remove all [it...end()). assert that we remove exactly commands
      // stored in containingCmds
      assert(std::distance(it, c.end()) == (long)containingCmds_.size());
      c.erase(it, c.end());
    }
  };

  //! debug method. returns a string describing this command
  std::string toPrettyString(size_t level = 0) const override { return ""; };

 private:
  AltTree* tree_;
  VbkBlock::hash_t vbkContaining_;
  std::vector<CommandPtr> containingCmds_{};
};

}  // namespace altintegration

#endif  // ALTINTEGRATION_ADDVTB_HPP
