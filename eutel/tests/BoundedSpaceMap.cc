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
  typedef unsigned short timestamp_type;
  typedef unsigned short item_type;
  BoundedSpaceMap<timestamp_type, item_type> buf(10);
  typedef CIRC::tag_type tag_type;
  typedef BoundedSpaceMap<timestamp_type, item_type>::collection_type coll_type;

  // The map is empty:
  EXPECT_TRUE(buf.isEmpty());
  EXPECT_EQ(tag_type(0), buf.getTop());
  EXPECT_EQ(tag_type(0), buf.getBottom());
  EXPECT_EQ(tag_type(0), buf.getMedian());
  EXPECT_EQ(size_t(10), buf.getFreeSize());
  // empty map:hasKey should return false, and get should return null pointers.
  for (tag_type i = 0; i < 10; ++i) {
    EXPECT_FALSE(buf.hasKey(i));
    timestamp_type t = buf.getTimeStamp(i);
    EXPECT_EQ(timestamp_type(0), t);
    coll_type v = buf.getData(i);
    EXPECT_EQ(size_t(0), v.size());
  }
}

TEST(BoundedSpaceMap, input)
{
  typedef unsigned short timestamp_type;
  typedef unsigned short item_type;
  BoundedSpaceMap<timestamp_type, item_type> buf(10);
  typedef CIRC::tag_type tag_type;
  typedef BoundedSpaceMap<timestamp_type, item_type>::collection_type coll_type;

  // fill and see what happens
  for (tag_type i = 0; i < 20; i++) {
    timestamp_type t = 30 + i;
    coll_type v;
    for (item_type j = i; j < 30; j++) v.push_back(j);
    buf.put(i, t, v);
    v.clear();
  }
  // Now we look what we've got
  // The bottom should be at 10, and the top at 19.
  EXPECT_EQ(buf.getBottom(), tag_type(10));
  EXPECT_EQ(buf.getTop(), tag_type(19));
  EXPECT_EQ(buf.getMedian(), tag_type(15));
  // No free position
  EXPECT_EQ(size_t(0), buf.getFreeSize());

  // One by one
  for (unsigned int i = 0; i < 20; i++) {
    if (i < 10) {
      EXPECT_FALSE(buf.hasKey(i));
      auto t = buf.getTimeStamp(i);
      EXPECT_EQ(timestamp_type(0), t);
      auto v = buf.getData(i);
      EXPECT_EQ(size_t(0), v.size());
      EXPECT_EQ(size_t(0), buf.getFreeSize());
    } else {
      EXPECT_TRUE(buf.hasKey(i));
      auto t = buf.getTimeStamp(i);
      EXPECT_EQ(timestamp_type(30 + i), t);
      auto v = buf.getData(i);
      EXPECT_EQ(size_t(30 - i), v.size());
      // Once retrieved, the data are gone
      EXPECT_FALSE(buf.hasKey(i));
      EXPECT_EQ(size_t(i - 9), buf.getFreeSize());
    }
  }
}

