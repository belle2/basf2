/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <vxd/background/niel_fun.h>
#include <gtest/gtest.h>
#include <iostream>
#include <string>

using namespace std;

namespace Belle2 {
  namespace SVD {

    /**
     * Check correct functioning of the class.
     */
    TEST(TNiel, output)
    {
      // First create a niel function and load it with data.
      TNiel niel("vxd/tests/niel_test.csv");
      // Value from within the range
      EXPECT_EQ(niel.getNielFactor(3.0), 3.126E-2);
      // Underflow value
      EXPECT_EQ(niel.getNielFactor(0.0), 1.543E-2);
      // Overflow value
      EXPECT_EQ(niel.getNielFactor(20.0), 5.160E-2);
    }

  } // namespace SVD
}  // namespace Belle2
