#include "fastkv/fastkv.hpp"

#include <gtest/gtest.h>

#include <atomic>
#include <optional>
#include <string>
#include <thread>
#include <vector>

namespace {

using fastkv::FastKV;

std::string key_for(int writer, int i) { return "w" + std::to_string(writer) + "-k" + std::to_string(i); }
std::string value_for(int writer, int i) { return "v" + std::to_string(writer) + "-" + std::to_string(i); }

TEST(FastKV, GetReturnsStoredValue) {
    FastKV kv;
    kv.put("a", "1");
    EXPECT_EQ(kv.get("a"), std::optional<std::string>("1"));
}

TEST(FastKV, GetOnAbsentKeyReturnsNullopt) {
    FastKV kv;
    EXPECT_EQ(kv.get("missing"), std::nullopt);
    kv.put("present", "x");
    EXPECT_EQ(kv.get("missing"), std::nullopt);
}

TEST(FastKV, EmptyValueIsDistinctFromAbsentKey) {
    FastKV kv;
    kv.put("empty", "");
    const auto value = kv.get("empty");
    ASSERT_TRUE(value.has_value());
    EXPECT_EQ(*value, "");
    EXPECT_FALSE(kv.get("never-stored").has_value());
}

TEST(FastKV, EmptyKeyIsAnOrdinaryKey) {
    FastKV kv;
    kv.put("", "value-for-empty-key");
    EXPECT_EQ(kv.get(""), std::optional<std::string>("value-for-empty-key"));
    EXPECT_TRUE(kv.erase(""));
    EXPECT_EQ(kv.get(""), std::nullopt);
}

TEST(FastKV, PutOverwritesExistingValue) {
    FastKV kv;
    kv.put("k", "first");
    kv.put("k", "second");
    EXPECT_EQ(kv.get("k"), std::optional<std::string>("second"));
}

TEST(FastKV, KeysAreIndependent) {
    FastKV kv;
    kv.put("a", "1");
    kv.put("b", "2");
    kv.put("a", "3");
    EXPECT_EQ(kv.get("a"), std::optional<std::string>("3"));
    EXPECT_EQ(kv.get("b"), std::optional<std::string>("2"));
}

TEST(FastKV, EraseRemovesKeyAndReportsIt) {
    FastKV kv;
    kv.put("k", "v");
    EXPECT_TRUE(kv.erase("k"));
    EXPECT_EQ(kv.get("k"), std::nullopt);
}

TEST(FastKV, EraseOfAbsentKeyReturnsFalse) {
    FastKV kv;
    EXPECT_FALSE(kv.erase("missing"));
    kv.put("k", "v");
    EXPECT_TRUE(kv.erase("k"));
    EXPECT_FALSE(kv.erase("k"));
}

TEST(FastKV, PutAfterEraseStoresAgain) {
    FastKV kv;
    kv.put("k", "old");
    ASSERT_TRUE(kv.erase("k"));
    kv.put("k", "new");
    EXPECT_EQ(kv.get("k"), std::optional<std::string>("new"));
}

// Writers fill disjoint key ranges while readers hammer the same keys. Under
// ThreadSanitizer this flags any unsynchronised access; without it, it checks
// that no write is lost and a reader never sees a value that was never written.
TEST(FastKVConcurrency, ManyWritersAndReaders) {
    constexpr int kWriters = 8;
    constexpr int kReaders = 8;
    constexpr int kKeysPerWriter = 500;

    FastKV kv;
    std::atomic<bool> start{false};
    std::atomic<int> torn_reads{0};

    std::vector<std::thread> threads;
    threads.reserve(kWriters + kReaders);
    for (int w = 0; w < kWriters; ++w) {
        threads.emplace_back([&, w] {
            while (!start.load()) {
                std::this_thread::yield();
            }
            for (int i = 0; i < kKeysPerWriter; ++i) {
                kv.put(key_for(w, i), value_for(w, i));
            }
        });
    }
    for (int r = 0; r < kReaders; ++r) {
        threads.emplace_back([&, r] {
            while (!start.load()) {
                std::this_thread::yield();
            }
            const int w = r % kWriters;
            for (int pass = 0; pass < 3; ++pass) {
                for (int i = 0; i < kKeysPerWriter; ++i) {
                    const auto value = kv.get(key_for(w, i));
                    if (value && *value != value_for(w, i)) {
                        torn_reads.fetch_add(1);
                    }
                }
            }
        });
    }
    start.store(true);
    for (auto& t : threads) {
        t.join();
    }

    EXPECT_EQ(torn_reads.load(), 0);
    for (int w = 0; w < kWriters; ++w) {
        for (int i = 0; i < kKeysPerWriter; ++i) {
            EXPECT_EQ(kv.get(key_for(w, i)), std::optional<std::string>(value_for(w, i)));
        }
    }
}

// Every thread overwrites and erases the same small set of keys. The final
// state is not deterministic, but each surviving value must be one that some
// thread actually wrote for that key.
TEST(FastKVConcurrency, ContendedPutEraseOnSameKeys) {
    constexpr int kThreads = 8;
    constexpr int kIterations = 2000;
    constexpr int kKeys = 4;

    FastKV kv;
    std::vector<std::thread> threads;
    threads.reserve(kThreads);
    for (int t = 0; t < kThreads; ++t) {
        threads.emplace_back([&, t] {
            for (int i = 0; i < kIterations; ++i) {
                const std::string key = "shared-" + std::to_string(i % kKeys);
                if (i % 3 == 0) {
                    kv.erase(key);
                } else {
                    kv.put(key, key + "-from-" + std::to_string(t));
                }
                (void)kv.get(key);
            }
        });
    }
    for (auto& t : threads) {
        t.join();
    }

    for (int k = 0; k < kKeys; ++k) {
        const std::string key = "shared-" + std::to_string(k);
        const auto value = kv.get(key);
        if (value) {
            EXPECT_EQ(value->rfind(key + "-from-", 0), 0U) << *value;
        }
    }
}

} // namespace
