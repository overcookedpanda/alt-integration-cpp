#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_STORAGE_BLOCK_REPOSITORY_ROCKS_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_STORAGE_BLOCK_REPOSITORY_ROCKS_HPP_

#include <rocksdb/db.h>

#include <set>
#include <utility>

#include "veriblock/blob.hpp"
#include "veriblock/serde.hpp"
#include "veriblock/storage/block_repository.hpp"
#include "veriblock/storage/db_error.hpp"
#include "veriblock/strutil.hpp"

namespace VeriBlock {

namespace {

rocksdb::Slice toSlice(int a) { return std::to_string(a); }

template <size_t N>
rocksdb::Slice toSlice(const Blob<N>& blob) {
  return rocksdb::Slice{blob.data(), blob.size()};
}

}  // namespace

template <typename Block>
class BlockRepositoryRocks : public BlockRepository<Block> {
  //! stored block type
  using stored_block_t = Block;
  //! block has type
  using hash_t = typename Block::hash_t;
  //! block height type
  using height_t = typename Block::height_t;
  //! iterator type
  using cursor_t = Cursor<hash_t, stored_block_t>;

  //! column family type
  using cf_handle_t = rocksdb::ColumnFamilyHandle;

 public:
  BlockRepositoryRocks() = default;

  BlockRepositoryRocks(std::shared_ptr<rocksdb::DB> db,
                       std::shared_ptr<cf_handle_t> hashBlockHandle)
      : _db(std::move(db)), _hashBlockHandle(std::move(hashBlockHandle)) {}

  bool put(const stored_block_t& block) override {
    // add hash -> block record
    auto hash = block.getHash();
    auto blockBytes = block.toRaw();

    auto s = _db->Put(rocksdb::WriteOptions(),
                      _hashBlockHandle.get(),
                      toSlice(hash),
                      blockBytes);
    if (!s.ok()) {
      throw db::DbError(s.ToString());
    }

    // always overwrites
    return true;
  }

  bool getByHash(const hash_t& hash, stored_block_t* out) const override {
    std::string dbValue{};
    rocksdb::Status s = _db->Get(rocksdb::ReadOptions(),
                                 _hashBlockHandle.get(),
                                 toSlice(hash),
                                 &dbValue);
    if (s.IsNotFound()) {
      return false;
    }
    if (!s.ok()) {
      throw db::DbError(s.ToString());
    }

    *out = stored_block_t::fromRaw(dbValue);
    return true;
  }

  size_t getManyByHash(Slice<const hash_t> hashes,
                       std::vector<stored_block_t>* out) const override {
    size_t found = 0;
    for (const hash_t& hash : hashes) {
      stored_block_t outBlock{};
      if (getByHash(hash, &outBlock)) {
        if (out) {
          out->push_back(outBlock);
        }
      }
      found++;
    }
    return found;
  }

  bool removeByHash(const hash_t& hash) override {
    // obtain block information
    auto status = _db->Delete(
        rocksdb::WriteOptions(), _hashBlockHandle.get(), toSlice(hash));
    if (status.IsNotFound()) {
      return false;
    }
    if (!status.ok()) {
      throw db::DbError(status.ToString());
    }

    return true;
  }

  std::unique_ptr<WriteBatch<stored_block_t>> newBatch() override {
    return nullptr;
  }

  std::shared_ptr<Cursor<height_t, stored_block_t>> getCursor() override {
    return std::make_shared<CursorRocks>(*this);
  }

 private:
  std::shared_ptr<rocksdb::DB> _db{};
  std::shared_ptr<cf_handle_t> _hashBlockHandle{};

 private:
  class CursorRocks : public Cursor<hash_t, stored_block_t> {
   private:
    std::shared_ptr<rocksdb::Iterator> it;

   public:
    explicit CursorRocks(const BlockRepositoryRocks<stored_block_t>& repo) {
      auto* iter = repo._db->NewIterator(rocksdb::ReadOptions(),
                                         repo._hashBlockHandle.get());
      it = std::shared_ptr<rocksdb::Iterator>(iter);
    }

    void seekToFirst() override { it->SeekToFirst(); }

    void seek(const hash_t& key) override { it->Seek(std::to_string(key)); }

    void seekToLast() override { it->SeekToLast(); }

    bool isValid() const override { return it->Valid(); }

    void next() override { it->Next(); }

    void prev() override { it->Prev(); }

    hash_t key() const override { return it->key(); }

    stored_block_t value() const override {
      return stored_block_t::fromRaw(it->value().ToString());
    }
  };
};

}  // namespace VeriBlock

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_STORAGE_BLOCK_REPOSITORY_ROCKS_HPP_
