#include <veriblock/context.hpp>

#include "statechange_impl.hpp"

namespace VeriBlock {

std::shared_ptr<StateChange> Context::beginStateChange() {
  return std::make_shared<StateChangeImpl>(*this);
}

}  // namespace VeriBlock