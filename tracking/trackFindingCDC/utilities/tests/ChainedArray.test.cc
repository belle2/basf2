/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <gtest/gtest.h>

#include <tracking/trackFindingCDC/utilities/ChainedIterator.h>

using namespace Belle2;
using namespace TrackFindingCDC;

namespace {
  TEST(TrackFindingCDCTest, utilities_ChainedArray_basic_test)
  {
    std::vector<int> a = {1, 2, 3};
    std::vector<int> b = {4, 5, 6};

    std::vector<int> result = {};

    ChainedArray<std::vector<int>> chain({a, b});

    for (const auto& item : chain) {
      result.push_back(item);
    }

    ASSERT_EQ(result.size(), 6);
    ASSERT_EQ(result[0], 1);
    ASSERT_EQ(result[1], 2);
    ASSERT_EQ(result[2], 3);
    ASSERT_EQ(result[3], 4);
    ASSERT_EQ(result[4], 5);
    ASSERT_EQ(result[5], 6);
  }

  TEST(TrackFindingCDCTest, utilities_ChainedArray_empty_test)
  {
    std::vector<int> result = {};

    ChainedArray<std::vector<int>> chain;

    for (const auto& item : chain) {
      result.push_back(item);
    }

    ASSERT_EQ(result.size(), 0);
  }

  TEST(TrackFindingCDCTest, utilities_ChainedArray_has_empty_test)
  {
    std::vector<int> a = {1, 2, 3};
    std::vector<int> b = {};
    std::vector<int> c = {4, 5, 6};

    std::vector<int> result = {};

    ChainedArray<std::vector<int>> chain({a, b, c});

    for (const auto& item : chain) {
      result.push_back(item);
    }

    ASSERT_EQ(result.size(), 6);
    ASSERT_EQ(result[0], 1);
    ASSERT_EQ(result[1], 2);
    ASSERT_EQ(result[2], 3);
    ASSERT_EQ(result[3], 4);
    ASSERT_EQ(result[4], 5);
    ASSERT_EQ(result[5], 6);
  }


  TEST(TrackFindingCDCTest, utilities_ChainedArray_empty_in_beginning_test)
  {
    std::vector<int> a = {};
    std::vector<int> b = {1, 2, 3};
    std::vector<int> c = {4, 5};

    std::vector<int> result = {};

    ChainedArray<std::vector<int>> chain({a, b, c});

    for (const auto& item : chain) {
      result.push_back(item);
    }

    ASSERT_EQ(result.size(), 5);
    ASSERT_EQ(result[0], 1);
    ASSERT_EQ(result[1], 2);
    ASSERT_EQ(result[2], 3);
    ASSERT_EQ(result[3], 4);
    ASSERT_EQ(result[4], 5);
  }

  TEST(TrackFindingCDCTest, utilities_ChainedArray_empty_in_end_test)
  {
    std::vector<int> a = {1, 2, 3};
    std::vector<int> b = {};
    std::vector<int> c = {4, 5};
    std::vector<int> d = {};

    std::vector<int> result = {};

    ChainedArray<std::vector<int>> chain({a, b, c, d});

    for (const auto& item : chain) {
      result.push_back(item);
    }

    ASSERT_EQ(result.size(), 5);
    ASSERT_EQ(result[0], 1);
    ASSERT_EQ(result[1], 2);
    ASSERT_EQ(result[2], 3);
    ASSERT_EQ(result[3], 4);
    ASSERT_EQ(result[4], 5);
  }
}
