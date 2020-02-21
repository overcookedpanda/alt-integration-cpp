#include "statechange_impl.hpp"

namespace VeriBlock {

bool StateChangeImpl::addPayload(const ATV& atv, ValidationState& state) {
  if(!checkATV(atv, state, vbkparams)) {

  }
  return false;
}

bool StateChangeImpl::addPayload(const VTB& vtb, ValidationState& state) {
  return false;
}

bool StateChangeImpl::updateContext(const std::vector<BtcBlock>& btc,
                                    ValidationState& state) {
  return false;
}

bool StateChangeImpl::updateContext(const std::vector<VbkBlock>& vbk,
                                    ValidationState& state) {
  return false;
}

void StateChangeImpl::apply() {}

}  // namespace VeriBlock