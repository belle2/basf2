/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost <oliver.frost@desy.de>                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <gtest/gtest.h>

#include <tracking/trackFindingCDC/utilities/Range.h>

#include <tracking/trackFindingCDC/utilities/GetValueType.h>
#include <tracking/trackFindingCDC/utilities/GetIterator.h>
#include <map>

using namespace Belle2;
using namespace TrackFindingCDC;

namespace {
  TEST(TrackFindingCDCTest, utilities_Range_works_with_equal_range)
  {
    using IntMap = std::map<int, int>;
    IntMap myMap;
    myMap.emplace(1, 2);
    myMap.emplace(3, 4);

    using IntMapIterator = GetIterator<IntMap>;
    using IntPair = GetValueType<IntMap>;

    for (const IntPair& intPair : Range<IntMapIterator>(myMap.equal_range(1))) {
      EXPECT_EQ(1, intPair.first);
    }
  }
}
