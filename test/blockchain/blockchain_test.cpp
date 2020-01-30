#include <gtest/gtest.h>

#include "mock/storage/block_repository_mock.hpp"
#include "veriblock/entities/btcblock.hpp"

using namespace VeriBlock;

template <typename Block>
struct BlockchainTest : public ::testing::Test {
  std::shared_ptr<BlockRepositoryMock<Block>> repo =
      std::make_shared<BlockRepositoryMock<Block>>();
};

TYPED_TEST_SUITE_P(BlockchainTest);

TYPED_TEST_P(BlockchainTest, BootstrapWorks) {}

REGISTER_TYPED_TEST_SUITE_P(BlockchainTest, BootstrapWorks);

INSTANTIATE_TYPED_TEST_SUITE_P(Generic, BlockchainTest, BtcBlock);