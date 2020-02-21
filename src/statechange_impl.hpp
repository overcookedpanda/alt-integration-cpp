#ifndef ALT_INTEGRATION_SRC_STATECHANGE_IMPL_HPP_
#define ALT_INTEGRATION_SRC_STATECHANGE_IMPL_HPP_

#include <veriblock/context.hpp>
#include <veriblock/statechange.hpp>

namespace VeriBlock {

struct StateChangeImpl : public StateChange {
  StateChangeImpl(Context& ctx) : ctx_(ctx) {}

  bool addPayload(const ATV& atv, ValidationState& state) override;

  bool addPayload(const VTB& vtb, ValidationState& state) override;

  bool updateContext(const std::vector<BtcBlock>& btc,
                     ValidationState& state) override;

  bool updateContext(const std::vector<VbkBlock>& vbk,
                     ValidationState& state) override;

  void apply() override;

 private:
  Context& ctx_;
};

}  // namespace VeriBlock

#endif  // ALT_INTEGRATION_SRC_STATECHANGE_IMPL_HPP_
