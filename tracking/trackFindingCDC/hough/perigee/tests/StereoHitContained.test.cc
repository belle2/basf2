/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost, Thomas Hauth <thomas.hauth@kit.edu>        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/trackFindingCDC/hough/algorithms/InPhi0CurvBox.h>
#include <tracking/trackFindingCDC/hough/perigee/StereoHitContained.h>


#include <gtest/gtest.h>

using namespace Belle2;
using namespace TrackFindingCDC;

namespace {

  TEST(TrackFindingCDCTest, hough_perigee_StereoHitContained_compile)
  {
    const float curlCurv = 0.013;
    StereoHitContained<InPhi0CurvBox> stereoHitContained(curlCurv);
    stereoHitContained.setRLWeightGain(0.1);
  }
}
