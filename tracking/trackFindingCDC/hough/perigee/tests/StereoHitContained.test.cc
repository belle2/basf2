/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost, Thomas Hauth <thomas.hauth@kit.edu>        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/test_fixtures/TrackFindingCDCTestWithTopology.h>

#include <tracking/trackFindingCDC/hough/perigee/Phi0Sweeped.h>
#include <tracking/trackFindingCDC/hough/perigee/CurvSweepedXLine.h>
#include <tracking/trackFindingCDC/hough/perigee/StereoHitContained.h>


#include <gtest/gtest.h>

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;

namespace {

  TEST(TrackFindingCDCTest, hough_perigee_StereoHitContained_compile)
  {
    using Phi0SweepedXCircle = Phi0Sweeped<CurvSweepedXLine>;

    const float curlCurv = 0.013;
    StereoHitContained<Phi0SweepedXCircle> stereoHitContained(curlCurv);
    stereoHitContained.setRLWeightGain(0.1);
  }
}
