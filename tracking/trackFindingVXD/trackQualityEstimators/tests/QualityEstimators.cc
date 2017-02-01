/*
 * QualityEstimators.cc
 *
 *  Created on: Jan 31, 2017
 *      Author: jowagner
 */

#include <gtest/gtest.h>

#include "tracking/trackFindingVXD/trackQualityEstimators/QualityEstimators.h"
#include <vector>
#include <utility> // pair

using namespace Belle2;
const static double allowed_chi2_deviation = 1e-20;

namespace QualityEstimatorTests {

  TEST(TripletFitTests, CircleChi2)
  {
    // Setup QualityEstimators Class
    QualityEstimators qualityEstimators = QualityEstimators();
    qualityEstimators.resetMagneticField(1.5);
    // Some hitpoints that are exactly on a circle in x,y with radius 1 and m=(1,0,0)
    PositionInfo hit1, hit2, hit3;

    hit1.hitPosition = TVector3(0., 0., 0.);
    hit2.hitPosition = TVector3(1., 1., 0.);
    hit3.hitPosition = TVector3(2., 0., 0.);

    std::vector<PositionInfo*> hitlist;
    hitlist.reserve(3);
    hitlist.push_back(&hit1);
    hitlist.push_back(&hit2);
    hitlist.push_back(&hit3);
    // Perform fit
    std::pair<double, TVector3> result = qualityEstimators.tripletFit(&hitlist);

    ASSERT_NEAR(result.first, 0., allowed_chi2_deviation) << "1st perfect circle";

    // Some hitpoints that are exactly on a circle in rotated 2D-plane with radius 1

    hit1.hitPosition = TVector3(0., 0., 0.);
    hit2.hitPosition = TVector3(1., 1., 1.);
    hit3.hitPosition = TVector3(2., 0., 2.);
    // Perform fit
    result = qualityEstimators.tripletFit(&hitlist);

    ASSERT_NEAR(result.first, 0., allowed_chi2_deviation) << "2nd perfect circle";

    // Move one hit away from circle

    hit3.hitPosition = TVector3(2., 0., 2. + 1e-6);
    // Perform fit
    result = qualityEstimators.tripletFit(&hitlist);

    ASSERT_TRUE(result.first > allowed_chi2_deviation) << "1st imperfect circle";

    // Move one hit away from circle

    hit3.hitPosition = TVector3(2. + 1e-6, 0., 2.);
    // Perform fit
    result = qualityEstimators.tripletFit(&hitlist);

    ASSERT_TRUE(result.first > allowed_chi2_deviation) << "2nd imperfect circle";
  }

  TEST(TripletFitTests, CombinedCircleChi2)
  {
    // Setup QualityEstimators Class
    QualityEstimators qualityEstimators = QualityEstimators();
    qualityEstimators.resetMagneticField(1.5);
    // Some hitpoints that are exactly on a circle in x,y with radius 1 and m=(1,0,0)
    PositionInfo hit1, hit2, hit3, hit4;

    hit1.hitPosition = TVector3(0., 0., 0.);
    hit2.hitPosition = TVector3(1., 1., 0.);
    hit3.hitPosition = TVector3(2., 0., 0.);
    hit4.hitPosition = TVector3(1., -1., 0.);

    std::vector<PositionInfo*> hitlist;
    hitlist.reserve(4);
    hitlist.push_back(&hit1);
    hitlist.push_back(&hit2);
    hitlist.push_back(&hit3);
    hitlist.push_back(&hit4);
    // Perform fit
    std::pair<double, TVector3> result = qualityEstimators.tripletFit(&hitlist);

    ASSERT_NEAR(result.first, 0., allowed_chi2_deviation) << "1st perfect combined circle";

    // Combine two triplets with different radii;
    hit4.hitPosition = TVector3(1., -2., 0.);

    result = qualityEstimators.tripletFit(&hitlist);

    ASSERT_TRUE(result.first > allowed_chi2_deviation) << "Incompatible circle triplets";
  }

}


