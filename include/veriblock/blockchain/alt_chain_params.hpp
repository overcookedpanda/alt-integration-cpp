// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_ALT_CHAIN_PARAMS_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_ALT_CHAIN_PARAMS_HPP_

#include <vector>

#include "veriblock/entities/altblock.hpp"

namespace altintegration {

struct PopRewardsCurveParams {
  virtual ~PopRewardsCurveParams() = default;

  // we start decreasing rewards after this score
  virtual double startOfSlope() const noexcept { return 1.0; }

  // we decrease reward coefficient for this value for
  // each additional score point above startOfDecreasingLine
  virtual double slopeNormal() const noexcept { return 0.2; }

  // slope for keystone rounds
  virtual double slopeKeystone() const noexcept { return 0.21325; }
};

struct PopRewardsParams {
  virtual ~PopRewardsParams() = default;

  // we use this round number to detect keystones
  virtual uint32_t keystoneRound() const noexcept { return 3; }

  // we have this number of rounds eg rounds 0, 1, 2, 3
  virtual uint32_t payoutRounds() const noexcept { return 4; }

  // we use this round number to pay flat reward (does not depend on pop
  // difficulty)
  virtual uint32_t flatScoreRound() const noexcept { return 2; }

  // should we use flat rewards at all
  virtual bool flatScoreRoundUse() const noexcept { return true; }

  // we have these payout modifiers for different rounds. Keystone round has
  // the highest multiplier
  virtual std::vector<double> roundRatios() const noexcept {
    return roundRatios_;
  }

  // limit block score to this value
  virtual double maxScoreThresholdNormal() const noexcept { return 2.0; }

  // limit block with keystones score to this value
  virtual double maxScoreThresholdKeystone() const noexcept { return 3.0; }

  // collect this amount of blocks BEFORE the block to calculate pop difficulty
  virtual uint32_t difficultyAveragingInterval() const noexcept { return 50; }

  // wait for this number of blocks before calculating and paying pop reward
  virtual uint32_t rewardSettlementInterval() const noexcept { return 400; }

  // getter for reward curve parameters
  virtual const PopRewardsCurveParams& getCurveParams() const noexcept {
    return *curveParams;
  }

  // reward score table
  // we score each VeriBlock and lower the reward for late blocks
  virtual const std::vector<double>& relativeScoreLookupTable() const noexcept {
    return lookupTable_;
  }

 protected:
  std::shared_ptr<PopRewardsCurveParams> curveParams =
      std::make_shared<PopRewardsCurveParams>();

  std::vector<double> roundRatios_{0.97, 1.03, 1.07, 3.00};

  std::vector<double> lookupTable_{
      1.00000000, 1.00000000, 1.00000000, 1.00000000, 1.00000000, 1.00000000,
      1.00000000, 1.00000000, 1.00000000, 1.00000000, 1.00000000, 1.00000000,
      0.48296816, 0.31551694, 0.23325824, 0.18453616, 0.15238463, 0.12961255,
      0.11265630, 0.09955094, 0.08912509, 0.08063761, 0.07359692, 0.06766428,
      0.06259873, 0.05822428, 0.05440941, 0.05105386, 0.04807993, 0.04542644,
      0.04304458, 0.04089495, 0.03894540, 0.03716941, 0.03554497, 0.03405359,
      0.03267969, 0.03141000, 0.03023319, 0.02913950, 0.02812047, 0.02716878,
      0.02627801, 0.02544253, 0.02465739, 0.02391820, 0.02322107, 0.02256255,
      0.02193952, 0.02134922};
};

struct AltChainParams {
  virtual ~AltChainParams() = default;

  virtual uint32_t getKeystoneInterval() const noexcept { return 5; }

  ///! number of blocks in VBK for finalization
  virtual uint32_t getFinalityDelay() const noexcept { return 100; }

  virtual std::vector<uint32_t> getForkResolutionLookUpTable() const noexcept {
    // TODO(warchant): this should be recalculated. see paper.
    return {100, 100, 95, 89, 80, 69, 56, 40, 21};
  }

  virtual int32_t getEndorsementSettlementInterval() const noexcept {
    return 500;
  }

  // unique POP id for the chain
  virtual uint32_t getIdentifier() const noexcept = 0;

  virtual AltBlock getBootstrapBlock() const noexcept = 0;

  // getter for reward parameters
  virtual const PopRewardsParams& getRewardParams() const noexcept {
    return *popRewardsParams;
  }

 private:
  std::shared_ptr<PopRewardsParams> popRewardsParams =
      std::make_shared<PopRewardsParams>();
};

}  // namespace altintegration

#endif
