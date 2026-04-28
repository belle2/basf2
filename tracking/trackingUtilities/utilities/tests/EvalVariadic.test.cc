/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <tracking/trackingUtilities/utilities/EvalVariadic.h>

#include <gtest/gtest.h>

using namespace Belle2;
using namespace TrackingUtilities;

namespace {
  TEST(TrackingUtilitiesTest, utilities_EvalVariadic_TypeInTuple)
  {
    using TestTuple = std::tuple<float, int>;

    static_assert(TypeInTuple<float, TestTuple>::value, "Type should be in tuple");
    static_assert(not TypeInTuple<size_t, TestTuple>::value, "Type should not be in tuple");
  }
}
