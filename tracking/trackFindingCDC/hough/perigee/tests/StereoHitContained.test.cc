/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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
