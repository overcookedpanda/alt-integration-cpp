#include <gtest/gtest.h>

#include <veriblock/storage/block_repository_inmem.hpp>
#include <veriblock/storage/block_repository_rocks.hpp>

using namespace VeriBlock;

struct Block {
  using hash_t = int;
  using height_t = int;

  hash_t getHash() const { return hash; }

  int hash = 0;
  int content = 0;

  bool operator==(const Block& b) const {
    return hash == b.hash && content == b.content;
  }

  std::string toRaw() {
    return std::to_string(hash) + "\n" + std::to_string(content);
  }

  static Block fromRaw(const std::string& s) {
    Block b;
    std::istringstream ss(s);
    ss >> b.hash;
    ss >> b.content;
    return b;
  }
};

struct StorageTest : public ::testing::Test {
  virtual ~StorageTest() = default;
  virtual std::shared_ptr<BlockRepository<Block>> getRepo() const = 0;
};

struct Inmem : public StorageTest {
  std::shared_ptr<BlockRepository<Block>> repo =
      std::make_shared<BlockRepositoryInmem<Block>>();

  ~Inmem() override = default;
  std::shared_ptr<BlockRepository<Block>> getRepo() const override {
    return repo;
  };
};

struct Rocks : public StorageTest {
  std::string name = "__testdb";
  std::shared_ptr<rocksdb::DB> db;
  std::vector<rocksdb::ColumnFamilyDescriptor> descriptors;
  std::shared_ptr<rocksdb::ColumnFamilyHandle> cf;

  std::shared_ptr<BlockRepository<Block>> repo;

  Rocks() {
    rocksdb::Status s;
    db = std::make_shared<rocksdb::DB>();
    rocksdb::ColumnFamilyHandle* handle = cf.get();
    s = db->CreateColumnFamily(rocksdb::ColumnFamilyOptions(), "cf", &handle);
    EXPECT_TRUE(s.ok()) << s.ToString();

    rocksdb::Options options;
    options.create_if_missing = true;
    options.create_missing_column_families = true;
    s = rocksdb::DestroyDB(name, options, descriptors);
    EXPECT_TRUE(s.ok()) << s.ToString();

    rocksdb::DB* dbptr = db.get();
    s = rocksdb::DB::Open(options, name, &dbptr);
    EXPECT_TRUE(s.ok()) << s.ToString();

    repo = std::make_shared<BlockRepositoryRocks<Block>>(db, cf);
  }

  void TearDown() override {
    rocksdb::Status s;
    rocksdb::Options options;
    options.create_if_missing = true;
    options.create_missing_column_families = true;
    s = rocksdb::DestroyDB(name, options, descriptors);
    EXPECT_TRUE(s.ok()) << s.ToString();
  }

  ~Rocks() override = default;
  std::shared_ptr<BlockRepository<Block>> getRepo() const override {
    return repo;
  };
};

struct StorageAcceptanceTest
    : public ::testing::TestWithParam<std::shared_ptr<StorageTest>> {};

static std::vector<std::shared_ptr<StorageTest>> cases = {
    std::make_shared<Inmem>(), std::make_shared<Rocks>()};

TEST_P(StorageAcceptanceTest, Basic) {
  auto repo = GetParam()->getRepo();
  Block b;

  EXPECT_FALSE(repo->put({1, 1}));
  EXPECT_TRUE(repo->getByHash(1, &b));
  EXPECT_EQ(b.hash, 1);
  EXPECT_EQ(b.content, 1);

  EXPECT_FALSE(repo->put({2, 2}));
  EXPECT_FALSE(repo->put({3, 3}));
  EXPECT_TRUE(repo->put({1, 5}));

  // block has been overwritten
  EXPECT_TRUE(repo->getByHash(1, &b));
  EXPECT_EQ(b.hash, 1);
  EXPECT_EQ(b.content, 5);

  EXPECT_TRUE(repo->getByHash(2, &b));
  EXPECT_EQ(b.getHash(), 2);

  EXPECT_TRUE(repo->getByHash(3, &b));
  EXPECT_EQ(b.getHash(), 3);

  EXPECT_FALSE(repo->getByHash(4, &b));

  std::vector<int> keys{1, 2, 4};
  std::vector<Block> blocks;
  size_t size = repo->getManyByHash(keys, &blocks);
  EXPECT_EQ(size, 2);
  EXPECT_EQ(blocks[0], (Block{1, 5}));
  EXPECT_EQ(blocks[1], (Block{2, 2}));

  EXPECT_TRUE(repo->removeByHash(1));
  EXPECT_FALSE(repo->removeByHash(10));
}

void checkContents(BlockRepository<Block>& repo,
                   const std::vector<Block>& blocks) {
  std::set<int> a;
  auto c = repo.newCursor();
  for (c->seekToFirst(); c->isValid(); c->next()) {
    a.insert(c->key());
  }

  std::vector<int> v;
  std::set<int> b;
  for (auto& block : blocks) {
    b.insert(block.hash);
    v.push_back(block.hash);
  }

  std::vector<Block> read;
  size_t size = repo.getManyByHash(v, &read);
  EXPECT_EQ(size, blocks.size()) << "size is not equal";
  EXPECT_EQ(a, b) << "keys are not equal";
  EXPECT_EQ(read, blocks) << "blocks are not equal";
}

// TEST_P(StorageAcceptanceTest, Batch) {
//  auto repo = GetParam()->getRepo();
//
//  EXPECT_FALSE(repo->put({1, 1}));
//  EXPECT_FALSE(repo->put({2, 2}));
//  EXPECT_FALSE(repo->put({3, 3}));
//
//  checkContents(*repo,
//                {
//                    {1, 1},
//                    {2, 2},
//                    {3, 3},
//                });
//
//  // commit empty batch does nothing
//  auto batch = repo->newBatch();
//  batch->commit(*repo);
//
//  checkContents(*repo,
//                {
//                    {1, 1},
//                    {2, 2},
//                    {3, 3},
//                });
//
//  // commit some changes
//  batch->put({4, 4});
//  batch->put({5, 5});
//  batch->put({1, 9});         // overwrite kv
//  batch->removeByHash(2);     // remove existing hash
//  batch->removeByHash(5);     // remove key added in batch
//  batch->removeByHash(1000);  // remove non-existing key
//  batch->commit(*repo);
//
//  checkContents(*repo,
//                {
//                    {1, 9},
//                    {3, 3},
//                    {4, 4},
//                });
//}

TEST_P(StorageAcceptanceTest, Cursor) {
  auto repo = GetParam()->getRepo();

  EXPECT_FALSE(repo->put({1, 1}));
  EXPECT_FALSE(repo->put({2, 2}));
  EXPECT_FALSE(repo->put({3, 3}));
  EXPECT_FALSE(repo->put({4, 4}));

  // save order in which blocks appear in DB
  std::vector<Block> values;
  auto c = repo->newCursor();
  for (c->seekToFirst(); c->isValid(); c->next()) {
    values.push_back(c->value());
  }

  c->seekToFirst();
  EXPECT_TRUE(c->isValid());
  EXPECT_EQ(c->key(), values[0].hash);
  EXPECT_EQ(c->value(), values[0]);

  c->seekToLast();
  EXPECT_TRUE(c->isValid());
  EXPECT_EQ(c->key(), values[values.size() - 1].hash);
  EXPECT_EQ(c->value(), values[values.size() - 1]);

  // read in reversed order
  std::vector<Block> reversedValues;
  for (c->seekToLast(); c->isValid(); c->prev()) {
    reversedValues.push_back(c->value());
  }

  // then reverse it
  std::reverse(reversedValues.begin(), reversedValues.end());
  EXPECT_EQ(reversedValues, values);

  // find some values
  c->seek(1);
  EXPECT_TRUE(c->isValid());  // key found
  EXPECT_EQ(c->value(), (Block{1, 1}));
  c->seek(2);
  EXPECT_TRUE(c->isValid());  // key found
  EXPECT_EQ(c->value(), (Block{2, 2}));
  c->seek(1000);  // non-existing key
  EXPECT_FALSE(c->isValid());
  // user is responsible for maintaining cursor validity
  EXPECT_DEATH(c->value(), "");
}

INSTANTIATE_TEST_SUITE_P(BlockRepo,
                         StorageAcceptanceTest,
                         testing::ValuesIn(cases));
