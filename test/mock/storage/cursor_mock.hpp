#ifndef ALT_INTEGRATION_TEST_MOCK_STORAGE_CURSOR_MOCK_HPP_
#define ALT_INTEGRATION_TEST_MOCK_STORAGE_CURSOR_MOCK_HPP_

#include <gmock/gmock.h>
#include <veriblock/storage/cursor.hpp>

namespace VeriBlock {

template <typename K, typename V>
 struct CursorMock : public Cursor<K, V> {
   ~CursorMock() override = default;

   MOCK_
 };

}

#endif  // ALT_INTEGRATION_TEST_MOCK_STORAGE_CURSOR_MOCK_HPP_
