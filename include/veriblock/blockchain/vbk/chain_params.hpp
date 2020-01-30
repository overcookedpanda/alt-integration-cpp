#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_VBK_CHAIN_PARAMS_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_VBK_CHAIN_PARAMS_HPP_

#include <memory>
#include <veriblock/uint.hpp>

namespace VeriBlock {
namespace vbk {

//! works as optional<uint8_t>
struct VbkNetworkType {
  //! if hasValue == false, it is mainnet
  bool hasValue = false;
  //! otherwise, use value
  uint8_t value = 0;
};

/**
 * VeriBlock chain parameters.
 */
struct VbkChainParams {
  virtual ~VbkChainParams() = default;
  virtual uint256 getMinimumDifficulty() const = 0;
  virtual VbkNetworkType getTransactionMagicByte() const noexcept = 0;
  virtual bool getPowNoRetargeting() const noexcept = 0;
};

/**
 * MainNet.
 */
struct VbkChainParamsMain : public VbkChainParams {
  ~VbkChainParamsMain() override = default;
  // hex(900000000000) = d18c2e2800
  uint256 getMinimumDifficulty() const override {
    return uint256(ParseHex("d18c2e2800"));
  }
  VbkNetworkType getTransactionMagicByte() const noexcept override {
    return {false, 0};
  }
  bool getPowNoRetargeting() const noexcept override { return false; }
};

/**
 * TestNet
 */
struct VbkChainParamsTest : public VbkChainParams {
  ~VbkChainParamsTest() override = default;
  // hex(100000000) = 5f5e100
  uint256 getMinimumDifficulty() const override {
    return uint256(ParseHex("5f5e100"));
  }
  VbkNetworkType getTransactionMagicByte() const noexcept override {
    return {true, 0xAA};
  }
  bool getPowNoRetargeting() const noexcept override { return false; }
};

/**
 * RegTest
 */
struct VbkChainParamsRegTest : public VbkChainParams {
  ~VbkChainParamsRegTest() override = default;
  // hex(1) = 1
  uint256 getMinimumDifficulty() const override {
    return uint256(ParseHex("1"));
  }
  VbkNetworkType getTransactionMagicByte() const noexcept override {
    return {true, 0xBB};
  }
  bool getPowNoRetargeting() const noexcept override { return true; }
};

/**
 * AlphaNet
 */
struct VbkChainParamsAlpha : public VbkChainParams {
  ~VbkChainParamsAlpha() override = default;
  // hex(9999872) = 989600
  uint256 getMinimumDifficulty() const override {
    return uint256(ParseHex("989600"));
  }
  VbkNetworkType getTransactionMagicByte() const noexcept override {
    return {true, 0xAA};
  }
  bool getPowNoRetargeting() const noexcept override { return false; }
};

}  // namespace vbk
}  // namespace VeriBlock

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_VBK_CHAIN_PARAMS_HPP_
