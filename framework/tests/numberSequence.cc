/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <framework/utilities/NumberSequence.h>
#include <gtest/gtest.h>

using namespace Belle2;
namespace {

  TEST(NumberSequenceTest, TestSingleNumber)
  {
    auto sequence = generate_number_sequence("3");
    std::set<int64_t> expected = {3};
    EXPECT_EQ(sequence, expected);
  }

  TEST(NumberSequenceTest, TestManyNumbers)
  {
    auto sequence = generate_number_sequence("3,4,5,10,12");
    std::set<int64_t> expected = {3, 4, 5, 10, 12};
    EXPECT_EQ(sequence, expected);
  }

  TEST(NumberSequenceTest, TestNegativeNumbers)
  {
    auto sequence = generate_number_sequence("-3,4,-5,10,12");
    std::set<int64_t> expected = { -5, -3, 4, 10, 12};
    EXPECT_EQ(sequence, expected);
  }

  TEST(NumberSequenceTest, TestOutOfOrderNumbers)
  {
    auto sequence = generate_number_sequence("3,10,4,12,5");
    std::set<int64_t> expected = {3, 4, 5, 10, 12};
    EXPECT_EQ(sequence, expected);
  }

  TEST(NumberSequenceTest, TestSingleInterval)
  {
    auto sequence = generate_number_sequence("3:6");
    std::set<int64_t> expected = {3, 4, 5, 6};
    EXPECT_EQ(sequence, expected);
  }

  TEST(NumberSequenceTest, TestManyIntervals)
  {
    auto sequence = generate_number_sequence("3:6,7:9,12:13");
    std::set<int64_t> expected = {3, 4, 5, 6, 7, 8, 9, 12, 13};
    EXPECT_EQ(sequence, expected);
  }

  TEST(NumberSequenceTest, TestOutOfOrderIntervals)
  {
    auto sequence = generate_number_sequence("12:13,3:6,7:9");
    std::set<int64_t> expected = {3, 4, 5, 6, 7, 8, 9, 12, 13};
    EXPECT_EQ(sequence, expected);
  }

  TEST(NumberSequenceTest, TestOverlappingIntervals)
  {
    auto sequence = generate_number_sequence("3:6,5:7,6:8");
    std::set<int64_t> expected = {3, 4, 5, 6, 7, 8};
    EXPECT_EQ(sequence, expected);
  }

  TEST(NumberSequenceTest, TestNegativeIntervals)
  {
    auto sequence = generate_number_sequence("3:6,-5:7,6:8");
    std::set<int64_t> expected = { -5, -4, -3, -2, -1, 0, 1, 2, 3, 4, 5, 6, 7, 8};
    EXPECT_EQ(sequence, expected);
  }

  TEST(NumberSequenceTest, TestReversedIntervalIsEmpty)
  {
    auto sequence = generate_number_sequence("8:6");
    std::set<int64_t> expected;
    EXPECT_EQ(sequence, expected);
  }

  TEST(NumberSequenceTest, TestMixNumbersAndIntervals)
  {
    auto sequence = generate_number_sequence("1,12:13,3,6,7:9,12");
    std::set<int64_t> expected = {1, 3, 6, 7, 8, 9, 12, 13};
    EXPECT_EQ(sequence, expected);
  }

  TEST(NumberSequenceTest, TestWhitespaces)
  {
    auto sequence = generate_number_sequence("1 , 12 : 13  ,3 ,6  ,7 :9,12 ");
    std::set<int64_t> expected = {1, 3, 6, 7, 8, 9, 12, 13};
    EXPECT_EQ(sequence, expected);
  }

}  // namespace
