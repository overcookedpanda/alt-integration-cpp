#include "veriblock/entities/atv.hpp"

namespace altintegration {

ATV ATV::fromVbkEncoding(Slice<const uint8_t> bytes) {
  ReadStream stream(bytes);
  return fromVbkEncoding(stream);
}

void ATV::toVbkEncoding(WriteStream& stream) const {
  WriteStream txStream;
  transaction.toVbkEncoding(stream);
  merklePath.toVbkEncoding(stream);
  containingBlock.toVbkEncoding(stream);
}

std::vector<uint8_t> ATV::toVbkEncoding() const {
  WriteStream stream;
  toVbkEncoding(stream);
  return stream.data();
}

ATV ATV::fromHex(const std::string& h) {
  auto data = ParseHex(h);
  ReadStream stream(data);
  return ATV::fromVbkEncoding(stream);
}

ATV ATV::fromVbkEncoding(ReadStream& stream) {
  ATV atv;
  atv.transaction = VbkTx::fromVbkEncoding(stream);
  atv.merklePath = VbkMerklePath::fromVbkEncoding(stream);
  atv.containingBlock = VbkBlock::fromVbkEncoding(stream);
  return atv;
}

uint256 ATV::getId() const {
  WriteStream w;
  transaction.toVbkEncoding(w);
  containingBlock.toVbkEncoding(w);
  return sha256(w.data());
}

}  // namespace altintegration