/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost <oliver.frost@desy.de>                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/trackFindingCDC/utilities/EvalVariadic.h>

#include <gtest/gtest.h>

using namespace Belle2;
using namespace TrackFindingCDC;

namespace {
  TEST(TrackFindingCDCTest, utilities_EvalVariadic_TypeInTuple)
  {
    using TestTuple = std::tuple<float, int>;

    static_assert(TypeInTuple<float, TestTuple>::value, "Type should be in tuple");
    static_assert(not TypeInTuple<size_t, TestTuple>::value, "Type should not be in tuple");
  }
}
