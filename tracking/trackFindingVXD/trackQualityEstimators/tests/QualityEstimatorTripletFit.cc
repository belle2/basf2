/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Felix Metzner, Jonas Wagner                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/


#include <gtest/gtest.h>

#include "tracking/trackFindingVXD/trackQualityEstimators/QualityEstimatorTripletFit.h"
#include <vector>
#include <utility> // pair

using namespace Belle2;
const static double allowed_chi2_deviation = 1e-20;


namespace QualityEstimatorTests {
  TEST(TripletFitTests, CircleChi2)
  {
    // Setup QualityEstimators Class
    auto qualityEstimator = QualityEstimatorTripletFit(1.5);
    // Some hitpoints that are exactly on a circle in x,y with radius 1 and m=(1,0,0)

    auto position1 = TVector3(0., 0., 0.);
    auto position2 = TVector3(1., 1., 0.);
    auto position3 = TVector3(2., 0., 0.);
    TVector3 sigma;

    std::vector<Measurement> hitlist;
    hitlist.reserve(3);
    hitlist.emplace_back(position1, sigma);
    hitlist.emplace_back(position2, sigma);
    hitlist.emplace_back(position3, sigma);

    // Perform fit
    auto result = qualityEstimator.calcChiSquared(hitlist);

    ASSERT_NEAR(result, 0., allowed_chi2_deviation) << "1st perfect circle";

    // Some hitpoints that are exactly on a circle in rotated 2D-plane with radius 1

    position1 = TVector3(0., 0., 0.);
    position2 = TVector3(1., 1., 1.);
    position3 = TVector3(2., 0., 2.);

    hitlist.clear();
    hitlist.emplace_back(position1, sigma);
    hitlist.emplace_back(position2, sigma);
    hitlist.emplace_back(position3, sigma);

    // Perform fit
    result = qualityEstimator.calcChiSquared(hitlist);

    ASSERT_NEAR(result, 0., allowed_chi2_deviation) << "2nd perfect circle";

    // Move one hit away from circle

    position3 = TVector3(2., 0., 2. + 1e-6);

    hitlist.clear();
    hitlist.emplace_back(position1, sigma);
    hitlist.emplace_back(position2, sigma);
    hitlist.emplace_back(position3, sigma);
    // Perform fit
    result = qualityEstimator.calcChiSquared(hitlist);

    ASSERT_TRUE(result > allowed_chi2_deviation) << "1st imperfect circle";

    // Move one hit away from circle

    position3 = TVector3(2. + 1e-6, 0., 2.);

    hitlist.clear();
    hitlist.emplace_back(position1, sigma);
    hitlist.emplace_back(position2, sigma);
    hitlist.emplace_back(position3, sigma);
    // Perform fit
    result = qualityEstimator.calcChiSquared(hitlist);

    ASSERT_TRUE(result > allowed_chi2_deviation) << "2nd imperfect circle";
  }

  TEST(TripletFitTests, CombinedCircleChi2)
  {
    // Setup QualityEstimators Class
    auto qualityEstimators = QualityEstimatorTripletFit(1.5);
    // Some hitpoints that are exactly on a circle in x,y with radius 1 and m=(1,0,0)

    auto position1 = TVector3(0., 0., 0.);
    auto position2 = TVector3(1., 1., 0.);
    auto position3 = TVector3(2., 0., 0.);
    auto position4 = TVector3(1., -1., 0.);
    TVector3 sigma;

    std::vector<Measurement> hitlist;
    hitlist.reserve(4);
    hitlist.emplace_back(position1, sigma);
    hitlist.emplace_back(position2, sigma);
    hitlist.emplace_back(position3, sigma);
    hitlist.emplace_back(position4, sigma);
    // Perform fit
    auto result = qualityEstimators.calcChiSquared(hitlist);

    ASSERT_NEAR(result, 0., allowed_chi2_deviation) << "1st perfect combined circle";

    // Combine two triplets with different radii;

    position4 = TVector3(1., -2., 0.);
    hitlist.clear();
    hitlist.reserve(4);
    hitlist.emplace_back(position1, sigma);
    hitlist.emplace_back(position2, sigma);
    hitlist.emplace_back(position3, sigma);
    hitlist.emplace_back(position4, sigma);

    result = qualityEstimators.calcChiSquared(hitlist);

    ASSERT_TRUE(result > allowed_chi2_deviation) << "Incompatible circle triplets";
  }
}
