/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost, Thomas Hauth <thomas.hauth@kit.edu>        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/trackFindingCDC/hough/perigee/Phi0Rep.h>
#include <tracking/trackFindingCDC/hough/perigee/CurvRep.h>
#include <tracking/trackFindingCDC/hough/boxes/Box.h>

#include <gtest/gtest.h>

using namespace Belle2;
using namespace TrackFindingCDC;

namespace {

  TEST(TrackFindingCDCTest, hough_perigee_getCurvBounds)
  {
    DiscretePhi0::Array phi0Bounds{{0, M_PI}};
    DiscreteCurv::Array discreteCurvBounds{{2.0, 4.0}};
    ContinuousCurv::Array continuousCurvBounds{{2.0, 4.0}};

    Box<DiscretePhi0, DiscreteCurv> phi0DiscreteCurvBox(DiscretePhi0::getRange(phi0Bounds),
                                                        DiscreteCurv::getRange(discreteCurvBounds));

    EXPECT_EQ(2.0, getLowerCurv(phi0DiscreteCurvBox));
    EXPECT_EQ(4.0, getUpperCurv(phi0DiscreteCurvBox));

    Box<DiscretePhi0, ContinuousCurv> phi0ContinuousCurvBox(DiscretePhi0::getRange(phi0Bounds),
                                                            ContinuousCurv::getRange(continuousCurvBounds));

    EXPECT_EQ(2.0, getLowerCurv(phi0ContinuousCurvBox));
    EXPECT_EQ(4.0, getUpperCurv(phi0ContinuousCurvBox));


    Box<DiscretePhi0> phi0Box(DiscretePhi0::getRange(phi0Bounds));

    EXPECT_EQ(0.0, getLowerCurv(phi0Box));
    EXPECT_EQ(0.0, getUpperCurv(phi0Box));

  }
}
