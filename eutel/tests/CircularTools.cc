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
#include <set>
#include <map>
#include <gtest/gtest.h>

using namespace std;

TEST(CircularTools, wrap)
{
  EXPECT_EQ(CIRC::wrap(-3), CIRC::c_tagUpperBound - 3);
  EXPECT_EQ(CIRC::wrap(17000), 17000 % CIRC::c_tagUpperBound);
  // upper bound wraps to lower bound
  EXPECT_EQ(CIRC::wrap(CIRC::c_tagUpperBound), CIRC::c_tagLowerBound);
}

TEST(CircularTools, compare)
{
  EXPECT_TRUE(CIRC::compare(10, 50));
  EXPECT_FALSE(CIRC::compare(50, 10));
  EXPECT_TRUE(CIRC::compare(CIRC::c_tagUpperBound - 20, 30));
  EXPECT_FALSE(CIRC::compare(30, CIRC::c_tagUpperBound - 20));
}

TEST(CircularTools, setWithCompare)
{
  std::set<CIRC::tag_type, CIRC::compare_type > tags(&CIRC::compare);
  CIRC::tag_type tagarray[4] = {CIRC::c_tagUpperBound - 20, 20, CIRC::c_tagUpperBound - 40, 40};
  for (CIRC::tag_type tag : tagarray)
    tags.insert(tag);
  auto ptag = tags.begin();
  EXPECT_EQ(*ptag, tagarray[2]);
  ptag++;
  EXPECT_EQ(*ptag, tagarray[0]);
  ptag++;
  EXPECT_EQ(*ptag, tagarray[1]);
  ptag++;
  EXPECT_EQ(*ptag, tagarray[3]);
}

TEST(CircularTools, mapWithCompare)
{
  std::map<CIRC::tag_type, CIRC::tag_type, CIRC::compare_type > tags(&CIRC::compare);
  CIRC::tag_type tagarray[4] = {CIRC::c_tagUpperBound - 20, 20, CIRC::c_tagUpperBound - 40, 40};
  CIRC::tag_type indexarray[4] = {2, 0, 1, 3};
  for (int i = 0; i < 4; ++i)
    tags.insert(std::make_pair(tagarray[i], indexarray[i]));
  CIRC::tag_type i(0);
  for (auto p : tags) {
    EXPECT_EQ(p.first, tagarray[indexarray[i]]);
    i++;
  }
}


TEST(CircularTools, distance)
{
  // Distance with transition over 0, also check symmetry of arguments
  EXPECT_EQ(size_t(6), CIRC::distance(3, CIRC::c_tagUpperBound - 3));
  EXPECT_EQ(size_t(6), CIRC::distance(CIRC::c_tagUpperBound - 3, 3));
  // Generic case
  EXPECT_EQ(5, CIRC::distance(10, 15));
  EXPECT_EQ(5, CIRC::distance(15, 10));
}

