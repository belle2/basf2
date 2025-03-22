/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <tracking/trackFindingCDC/utilities/Ptr.h>

#include <gtest/gtest.h>

using namespace Belle2;
using namespace TrackFindingCDC;

namespace {
  TEST(TrackFindingCDCTest, utilities_Ptr_construct)
  {
    int i = 4;
    Ptr<int> ptrIFromPointer {&i};
    EXPECT_EQ(4, *ptrIFromPointer);
  }

  TEST(TrackFindingCDCTest, utilities_Ptr_const_conversion)
  {
    int i = 4;
    Ptr<int> ptrI{&i};
    Ptr<const int> ptrConstI = Ptr<const int>(ptrI);
    EXPECT_EQ(4, *ptrConstI);
  }
}
