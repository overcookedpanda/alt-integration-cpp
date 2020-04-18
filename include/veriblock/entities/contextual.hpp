#ifndef ALTINTEGRATION_CONTEXTUAL_HPP
#define ALTINTEGRATION_CONTEXTUAL_HPP

#include <vector>
#include <veriblock/entities/vbkblock.hpp>

namespace altintegration {

template <typename T>
struct Contextual {
  T data;
  std::vector<VbkBlock> context;

  static Contextual<T> fromVbkEncoding(ReadStream& stream) {
    Contextual<T> t;
    t.data = T::fromVbkEncoding(stream);
    t.context = readArrayOf<VbkBlock>(
        stream, 0, MAX_CONTEXT_COUNT, VbkBlock::fromVbkEncoding);
    return t;
  }

  void toVbkEncoding(WriteStream& stream) {
    data.toVbkEncoding(stream);
    // write context blocks
    writeSingleBEValue(stream, context.size());
    for (const auto& block : context) {
      block.toVbkEncoding(stream);
    }
  }

  bool friend operator==(const Contextual<T>& a, const Contextual<T>& b) {
    return a.data == b.data && a.context == b.context;
  }
};

}  // namespace altintegration

#endif  // ALTINTEGRATION_CONTEXTUAL_HPP
