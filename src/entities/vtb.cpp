#include "veriblock/entities/vtb.hpp"

using namespace altintegration;

ContextualVTB ContextualVTB::fromVbkEncoding(ReadStream& stream) {
  ContextualVTB vtb{};
  vtb.transaction = VbkPopTx::fromVbkEncoding(stream);
  vtb.merklePath = VbkMerklePath::fromVbkEncoding(stream);
  vtb.containingBlock = VbkBlock::fromVbkEncoding(stream);
  vtb.context = readArrayOf<VbkBlock>(
      stream, 0, MAX_CONTEXT_COUNT, [](ReadStream& stream) {
        return VbkBlock::fromVbkEncoding(stream);
      });

  return vtb;
}

ContextualVTB ContextualVTB::fromVbkEncoding(Slice<const uint8_t> bytes) {
  ReadStream stream(bytes);
  return fromVbkEncoding(stream);
}

ContextualVTB ContextualVTB::fromVbkEncoding(const std::string& bytes) {
  ReadStream stream(bytes);
  return fromVbkEncoding(stream);
}

void ContextualVTB::toVbkEncoding(WriteStream& stream) const {
  WriteStream txStream;
  transaction.toVbkEncoding(stream);
  merklePath.toVbkEncoding(stream);
  containingBlock.toVbkEncoding(stream);
  writeSingleBEValue(stream, context.size());
  for (const auto& block : context) {
    block.toVbkEncoding(stream);
  }
}

std::vector<uint8_t> ContextualVTB::toVbkEncoding() const {
  WriteStream stream;
  toVbkEncoding(stream);
  return stream.data();
}

VbkBlock ContextualVTB::getContainingBlock() const { return this->containingBlock; }

VbkBlock ContextualVTB::getEndorsedBlock() const {
  return this->transaction.publishedBlock;
}

ContextualVTB ContextualVTB::fromHex(const std::string& hex) {
  auto data = ParseHex(hex);
  return fromVbkEncoding(data);
}
