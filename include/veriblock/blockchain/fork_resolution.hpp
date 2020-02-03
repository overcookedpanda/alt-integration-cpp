#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_BTC_FORK_RESOLUTION_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_BTC_FORK_RESOLUTION_HPP_

#include <cassert>
#include <memory>
#include <veriblock/blockchain/block_index.hpp>
#include <veriblock/entities/btcblock.hpp>
#include <veriblock/entities/vbkblock.hpp>

namespace VeriBlock {

void determineBestChain(std::shared_ptr<BlockIndex<BtcBlock>>* pindexBestHeader,
                        const BlockIndex<BtcBlock>& indexNew) {
  assert(pindexBestHeader != nullptr);
  if (*pindexBestHeader == nullptr ||
      (*pindexBestHeader)->chainWork < indexNew.chainWork) {
    *pindexBestHeader = std::make_shared<BlockIndex<BtcBlock>>(indexNew);
  }
}

//void determineBestChain(std::shared_ptr<BlockIndex<VbkBlock>>* pindexBestHeader,
//                        const BlockIndex<VbkBlock>& indexNew) {
//  (void)pindexBestHeader;
//  (void)indexNew;
//  assert(false && "TODO: implement fork resolution for VBK");
//}

}  // namespace VeriBlock

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_BTC_FORK_RESOLUTION_HPP_
