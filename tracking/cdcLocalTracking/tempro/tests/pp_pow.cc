/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost <oliver.frost@desy.de>                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/cdcLocalTracking/tempro/pp_pow.h>

#include <gtest/gtest.h>

using namespace std;

TEST(TemproTest, tempro_BELLE2_PP_POW)
{

  static_assert(1 == BELLE2_PP_POW(1, 0), "BELLE2_PP_POW gives wrong value for 1**0, should be 1");
  static_assert(1 == BELLE2_PP_POW(0, 0), "BELLE2_PP_POW gives wrong value for 0**0, should be 1");

  static_assert(0 == BELLE2_PP_POW(0, 5), "BELLE2_PP_POW gives wrong value for 0**5, should be 0");

  static_assert(8 == BELLE2_PP_POW(2, 3), "BELLE2_PP_POW gives wrong value for 2**3, should be 8");
  static_assert(27 == BELLE2_PP_POW(3, 3), "BELLE2_PP_POW gives wrong value for 2**3, should be 27");

  static_assert(42 == BOOST_PP_ADD(BELLE2_PP_POW(2, 5), 10), "The answer is not 42");

}


