#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_STATECHANGE_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_STATECHANGE_HPP_

#include <vector>
#include <veriblock/entities/atv.hpp>
#include <veriblock/entities/vtb.hpp>
#include <veriblock/validation_state.hpp>

namespace VeriBlock {
/**
 * @class StateChange
 * @brief Stores temporary changes to current state.
 *
 * To apply changes, execute \p apply.
 * To discard changes, simply destroy instance of StateChange.
 */
struct StateChange {
  virtual ~StateChange() = default;

  virtual bool addPayload(const ATV& atv, ValidationState& state) = 0;

  virtual bool addPayload(const VTB& vtb, ValidationState& state) = 0;

  virtual bool updateContext(const std::vector<BtcBlock>& btc,
                             ValidationState& state) = 0;

  virtual bool updateContext(const std::vector<VbkBlock>& vbk,
                             ValidationState& state) = 0;

  virtual void apply() = 0;
};

}  // namespace VeriBlock

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_STATECHANGE_HPP_
