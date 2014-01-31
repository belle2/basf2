/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Peter Kvasnicka                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <eutel/merge/CircularTools.h>
#include <eutel/merge/BoundedSpaceMap.h>
#include <gtest/gtest.h>

using namespace std;

TEST(BoundedSpaceMap, empty)
{
  BoundedSpaceMap<unsigned short> buf(3);
  typedef BoundedSpaceMap<unsigned short>::collection_type coll_type;
  typedef CIRC::tag_type tag_type;
  // The map is empty:
  EXPECT_TRUE(buf.isEmpty());
  EXPECT_EQ(tag_type(0), buf.getTop());
  EXPECT_EQ(tag_type(0), buf.getBottom());
  EXPECT_EQ(tag_type(0), buf.getMedian());
  EXPECT_EQ(size_t(3), buf.getFreeSize());
  // empty map:hasKey should return false, and get should return null pointers.
  for (tag_type i = 0; i < 3; ++i) {
    EXPECT_FALSE(buf.hasKey(i));
    auto v = buf.get(i);
    EXPECT_EQ(size_t(0), v.size());
  }
}

TEST(BoundedSpaceMap, input)
{
  BoundedSpaceMap<unsigned short> buf(3);
  typedef BoundedSpaceMap<unsigned short>::collection_type coll_type;
  typedef CIRC::tag_type tag_type;
  // fill and see what happens
  for (unsigned int i = 0; i < 10; i++) {
    coll_type v;
    for (unsigned short j = i; j < 10; j++) v.push_back(j);
    buf.put(i, v);
    v.clear();
  }
  // Now we look what we've got
  // The bottom should be at 7, and the top at 9.
  EXPECT_EQ(buf.getBottom(), tag_type(7));
  EXPECT_EQ(buf.getTop(), tag_type(9));
  // Median should be at 8
  EXPECT_EQ(buf.getMedian(), tag_type(8));
  // No free position
  EXPECT_EQ(size_t(0), buf.getFreeSize());
  // One by one
  for (unsigned int i = 0; i < 10; i++) {
    if (i < 7) {
      EXPECT_FALSE(buf.hasKey(i));
      auto v = buf.get(i);
      EXPECT_EQ(size_t(0), v.size());
      EXPECT_EQ(size_t(0), buf.getFreeSize());
    } else {
      EXPECT_TRUE(buf.hasKey(i));
      auto v = buf.get(i);
      EXPECT_EQ(size_t(10 - i), v.size());
      for (size_t j = 0; j < v.size(); ++j)
        EXPECT_EQ(i + j, v.at(j));
      EXPECT_FALSE(buf.hasKey(i));
      EXPECT_EQ(size_t(i - 6), buf.getFreeSize());
    }
  }
}

