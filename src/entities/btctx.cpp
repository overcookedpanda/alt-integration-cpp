// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "veriblock/entities/btctx.hpp"

using namespace altintegration;

BtcTx BtcTx::fromVbkEncoding(ReadStream& stream) {
  return BtcTx(readVarLenValue(stream, 0, BTC_TX_MAX_RAW_SIZE));
}

void BtcTx::toVbkEncoding(WriteStream& stream) const {
  writeVarLenValue(stream, tx);
}

uint256 BtcTx::getHash() const { return sha256twice(tx); }

std::string BtcTx::toHex() const {
  return HexStr(tx);
}