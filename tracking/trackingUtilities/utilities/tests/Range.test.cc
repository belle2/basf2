/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <gtest/gtest.h>

#include <tracking/trackingUtilities/utilities/Range.h>

#include <tracking/trackingUtilities/utilities/GetValueType.h>
#include <tracking/trackingUtilities/utilities/GetIterator.h>
#include <map>

using namespace Belle2;
using namespace TrackingUtilities;

namespace {
  TEST(TrackingUtilitiesTest, utilities_Range_works_with_equal_range)
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
