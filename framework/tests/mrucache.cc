#include <framework/core/MRUCache.h>
#include <gtest/gtest.h>
#include <iostream>
#include <string>

using namespace std;

namespace Belle2 {

  TEST(MRUCache, Constructor)
  {
    MRUCache<string, string> cache(100);
    cache.insert("foo", "bar");
    string result("boo");
    EXPECT_FALSE(cache.retrieve("foobar", result));
    EXPECT_EQ(result, "boo");
    EXPECT_TRUE(cache.retrieve("foo", result));
    EXPECT_EQ(result, "bar");
  }

  TEST(MRUCache, InsertRetrieve)
  {
    MRUCache<int, int> cache(5);
    for (int i = 0; i < 10; ++i) {
      cache.insert(i, -i);
    }

    EXPECT_EQ(cache.size(), 5u);
    MRUCache<int, int>::iterator it = cache.begin();
    for (int i = 9; i >= 5; --i) {
      EXPECT_EQ(it->first, i);
      EXPECT_EQ(it->second, -i);
      ++it;
    }
    int var;
    EXPECT_TRUE(cache.retrieve(5, var));
    EXPECT_EQ(cache.begin()->first, 5);
    EXPECT_EQ(cache.begin()->second, -5);

    cache.clear();
    EXPECT_EQ(cache.size(), 0u);
    var = -1;
    EXPECT_FALSE(cache.retrieve(5, var));
    EXPECT_EQ(var, -1);
  }

  TEST(MRUCache, Statistics)
  {
    MRUCache<int, int> cache(1);
    int var;

    EXPECT_EQ(cache.getHits(), 0u);
    EXPECT_EQ(cache.getMisses(), 0u);
    EXPECT_EQ(cache.getOverflows(), 0u);

    cache.retrieve(0, var);
    EXPECT_EQ(cache.getMisses(), 1u);
    cache.retrieve(0, var);
    EXPECT_EQ(cache.getMisses(), 2u);

    cache.insert(0, 0);
    EXPECT_EQ(cache.getOverflows(), 0u);
    cache.insert(1, 0);
    EXPECT_EQ(cache.getOverflows(), 1u);
    cache.insert(0, 0);
    EXPECT_EQ(cache.getOverflows(), 2u);

    cache.retrieve(0, var);
    EXPECT_EQ(cache.getHits(), 1u);
    cache.retrieve(0, var);
    EXPECT_EQ(cache.getHits(), 2u);

    cache.clear();
    EXPECT_EQ(cache.getHits(), 0u);
    EXPECT_EQ(cache.getMisses(), 0u);
    EXPECT_EQ(cache.getOverflows(), 0u);
  }
}  // namespace
