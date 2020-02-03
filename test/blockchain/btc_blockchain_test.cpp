#include <gtest/gtest.h>

#include <veriblock/blockchain/blockchain.hpp>

#include "mock/storage/block_repository_mock.hpp"
#include "veriblock/entities/btcblock.hpp"

using namespace VeriBlock;

struct BtcBlockchainTest : public ::testing::Test {
  using index_t = typename Blockchain<BtcBlock>::index_t;

  std::shared_ptr<BlockRepositoryMock<index_t>> repo;
  std::shared_ptr<Blockchain<BtcBlock>> blockchain;

  BtcBlockchainTest() {
    repo = std::make_shared<BlockRepositoryMock<index_t>>();
    blockchain = std::make_shared<Blockchain<BtcBlock>>(repo);
  };
};

TEST_F(BtcBlockchainTest, BootstrapEmptyStore) {
  BtcBlock block;
  blockchain->bootstrap(block);
}