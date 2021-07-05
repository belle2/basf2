/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <tracking/trackFindingCDC/utilities/Functional.h>

#include <gtest/gtest.h>

using namespace Belle2;
using namespace TrackFindingCDC;

namespace {
  TEST(TrackFindingCDCTest, utilities_clearIfApplicable)
  {
    float f = 2;
    std::vector<float> v{0, 1};
    EXPECT_FALSE(v.empty());

    clearIfApplicable(f);
    invokeIfApplicable(Clear(), v);


    EXPECT_TRUE(v.empty());
    EXPECT_EQ(2, f);
  }

  struct Back {
    template <class T>
    const typename T::value_type& operator()(const T& container) const
    {
      return container.back();
    }
  };

  TEST(TrackFindingCDCTest, utilities_getIfApplicable)
  {
    const float f = 2;
    const std::vector<float> v{0.0, 1.0};
    EXPECT_FALSE(v.empty());

    // Test if the back getter works without the magic
    Back back;
    EXPECT_EQ(1.0, back(v));

    // Valid get case
    EXPECT_EQ(1.0, Back()(v));
    EXPECT_EQ(1.0, getIfApplicable<float>(Back(), v, -1.0));

    // Default case
    EXPECT_EQ(-1.0, getIfApplicable<float>(Back(), f, -1.0));


  }
}
