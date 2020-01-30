#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_BTC_CHAIN_PARAMS_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_BTC_CHAIN_PARAMS_HPP_

#include <veriblock/strutil.hpp>
#include <veriblock/uint.hpp>

namespace VeriBlock {
namespace btc {

struct BtcChainParams {
  virtual ~BtcChainParams() = default;
  virtual uint256 getPowLimit() const = 0;
  virtual int getPowTargetTimespan() const noexcept = 0;
  virtual int getPowTargetSpacing() const noexcept = 0;
  virtual bool getAllowMinDifficultyBlocks() const noexcept = 0;
  virtual bool getPowNoRetargeting() const noexcept = 0;
};

struct BtcChainParamsMain : public BtcChainParams {
  ~BtcChainParamsMain() override = default;

  uint256 getPowLimit() const override {
    return uint256(ParseHex(
        "00000000ffffffffffffffffffffffffffffffffffffffffffffffffffffffff"));
  }

  int getPowTargetTimespan() const noexcept override {
    return 14 * 24 * 60 * 60;
  }
  int getPowTargetSpacing() const noexcept override { return 10 * 60; }
  bool getAllowMinDifficultyBlocks() const noexcept override { return false; }
  bool getPowNoRetargeting() const noexcept override { return false; }
};

struct BtcChainParamsTest : public BtcChainParams {
  ~BtcChainParamsTest() override = default;

  uint256 getPowLimit() const override {
    return uint256(ParseHex(
        "00000000ffffffffffffffffffffffffffffffffffffffffffffffffffffffff"));
  }

  int getPowTargetTimespan() const noexcept override {
    return 14 * 24 * 60 * 60;
  }
  int getPowTargetSpacing() const noexcept override { return 10 * 60; }
  bool getAllowMinDifficultyBlocks() const noexcept override { return true; }
  bool getPowNoRetargeting() const noexcept override { return false; }
};

struct BtcChainParamsRegTest : public BtcChainParams {
  ~BtcChainParamsRegTest() override = default;

  uint256 getPowLimit() const override {
    return uint256(ParseHex(
        "7fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff"));
  }

  int getPowTargetTimespan() const noexcept override {
    return 14 * 24 * 60 * 60;
  }
  int getPowTargetSpacing() const noexcept override { return 10 * 60; }
  bool getAllowMinDifficultyBlocks() const noexcept override { return true; }
  bool getPowNoRetargeting() const noexcept override { return true; }
};

}  // namespace btc
}  // namespace VeriBlock

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_BTC_CHAIN_PARAMS_HPP_
