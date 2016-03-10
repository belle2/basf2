/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost <oliver.frost@desy.de>                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
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
