#include "fastkv/fastkv.hpp"
#include <gtest/gtest.h>

TEST(FastKV, BasicPutGet) {
    fastkv::FastKV kv;
    kv.put("a", "1");
    EXPECT_EQ(kv.get("a"), "1");
}
