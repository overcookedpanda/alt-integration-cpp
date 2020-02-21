#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_CONTEXT_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_CONTEXT_HPP_

#include <memory>
#include <utility>
#include <veriblock/blockchain/btc_blockchain_util.hpp>
#include <veriblock/blockchain/btc_chain_params.hpp>
#include <veriblock/blockchain/pop/vbk_block_tree.hpp>
#include <veriblock/blockchain/vbk_blockchain_util.hpp>
#include <veriblock/blockchain/vbk_chain_params.hpp>
#include <veriblock/statechange.hpp>
#include <veriblock/storage/block_repository_inmem.hpp>
#include <veriblock/storage/endorsements_repository_inmem.hpp>

namespace VeriBlock {

struct Config {
  template <typename Block>
  struct Bootstrap {
    int32_t firstBlockHeight;
    std::vector<Block> blocks;
  };

  Bootstrap<VbkBlock> vbkBootstrap;
  Bootstrap<BtcBlock> btcBootstrap;

  std::shared_ptr<BtcChainParams> btcChainParams;
  std::shared_ptr<VbkChainParams> vbkChainParams;

  std::string databasePath;
};

struct Context {
  std::shared_ptr<StateChange> beginStateChange();

  static Context initInmem(Config config) {
    Context ctx(std::move(config));

    ctx.btc_repo_ =
        std::make_shared<BlockRepositoryInmem<BlockIndex<BtcBlock>>>();
    ctx.vbk_repo_ =
        std::make_shared<BlockRepositoryInmem<BlockIndex<VbkBlock>>>();
    ctx.erepo_ = std::make_shared<EndorsementsRepositoryInmem>();

    ctx.btc_ = std::make_shared<BtcBlockTree>(ctx.btc_repo_,
                                              ctx.config_.btcChainParams);
    ctx.vbk_ = std::make_shared<VbkBlockTree>(
        *ctx.btc_, ctx.erepo_, ctx.vbk_repo_, ctx.config_.vbkChainParams);

    bool res = false;
    ValidationState state;
    res =
        ctx.btc_->bootstrapWithChain(ctx.config_.btcBootstrap.firstBlockHeight,
                                     ctx.config_.btcBootstrap.blocks,
                                     state);
    assert(res && "can not bootstrap btc chain");
    res =
        ctx.vbk_->bootstrapWithChain(ctx.config_.vbkBootstrap.firstBlockHeight,
                                     ctx.config_.vbkBootstrap.blocks,
                                     state);
    assert(res && "can not bootstrap vbk chain");

    return ctx;
  }

 private:
  Context(Config config) : config_(std::move(config)) {}

  using BtcBlockRepo = BlockRepository<BlockIndex<BtcBlock>>;
  using VbkBlockRepo = BlockRepository<BlockIndex<VbkBlock>>;
  using BtcBlockTree = BlockTree<BtcBlock, BtcChainParams>;
  Config config_;

  std::shared_ptr<EndorsementsRepository> erepo_;
  std::shared_ptr<BtcBlockRepo> btc_repo_;
  std::shared_ptr<VbkBlockRepo> vbk_repo_;

  std::shared_ptr<VbkBlockTree> vbk_;
  std::shared_ptr<BtcBlockTree> btc_;
};

}  // namespace VeriBlock

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_CONTEXT_HPP_
