// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef ALTINTEGRATION_ADDVTB_HPP
#define ALTINTEGRATION_ADDVTB_HPP

#include <vector>
#include <veriblock/blockchain/alt_block_tree.hpp>

namespace altintegration {

struct AddVTB : public Command {
  ~AddVTB() override = default;

  AddVTB(BlockIndex<AltBlock>& containing, const VTB& vtb) {
    // interpret vtb into a vector of commands
  }

  bool Execute(ValidationState& state) override {}
  void UnExecute() override{};

  //! debug method. returns a string describing this command
  std::string toPrettyString(size_t level = 0) const override { return ""; };

 private:
  std::vector<CommandPtr> containingCmds_;
};

}  // namespace altintegration

#endif  // ALTINTEGRATION_ADDVTB_HPP
