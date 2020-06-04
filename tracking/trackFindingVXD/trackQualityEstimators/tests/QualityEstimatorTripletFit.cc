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

#include <vector>

#include <framework/geometry/B2Vector3.h>

#include "tracking/trackFindingVXD/trackQualityEstimators/QualityEstimatorTripletFit.h"


using namespace Belle2;
const static double allowed_chi2_deviation = 1e-20;


namespace QualityEstimatorTests {

  TEST(TripletFitTests, CircleChi2)
  {
    // Setup QualityEstimator
    auto qualityEstimator = QualityEstimatorTripletFit();
    qualityEstimator.setMagneticFieldStrength(1.5);

    // Some hitpoints that are exactly on a circle in x,y with radius 1 and m=(1,0,0)
    auto position1 = B2Vector3<double>(0., 0., 0.);
    auto position2 = B2Vector3<double>(1., 1., 0.);
    auto position3 = B2Vector3<double>(2., 0., 0.);
    B2Vector3<double> sigma;

    auto spacePoint1 = SpacePoint(position1, sigma, {0.5, 0.5}, {false, false}, VxdID(0), Belle2::VXD::SensorInfoBase::VXD);
    auto spacePoint2 = SpacePoint(position2, sigma, {0.5, 0.5}, {false, false}, VxdID(0), Belle2::VXD::SensorInfoBase::VXD);
    auto spacePoint3 = SpacePoint(position3, sigma, {0.5, 0.5}, {false, false}, VxdID(0), Belle2::VXD::SensorInfoBase::VXD);

    std::vector<SpacePoint const*> hitlist;
    hitlist.reserve(3);
    hitlist.push_back(&spacePoint1);
    hitlist.push_back(&spacePoint2);
    hitlist.push_back(&spacePoint3);

    // Perform fit
    auto result = qualityEstimator.estimateQualityAndProperties(hitlist);

    ASSERT_TRUE(result.chiSquared);
    ASSERT_NEAR(*(result.chiSquared), 0., allowed_chi2_deviation) << "1st perfect circle";

    // Some hitpoints that are exactly on a circle in rotated 2D-plane with radius 1
    position1 = B2Vector3<double>(0., 0., 0.);
    position2 = B2Vector3<double>(1., 1., 1.);
    position3 = B2Vector3<double>(2., 0., 2.);

    spacePoint1 = SpacePoint(position1, sigma, {0.5, 0.5}, {false, false}, VxdID(0), Belle2::VXD::SensorInfoBase::VXD);
    spacePoint2 = SpacePoint(position2, sigma, {0.5, 0.5}, {false, false}, VxdID(0), Belle2::VXD::SensorInfoBase::VXD);
    spacePoint3 = SpacePoint(position3, sigma, {0.5, 0.5}, {false, false}, VxdID(0), Belle2::VXD::SensorInfoBase::VXD);

    hitlist.clear();
    hitlist.push_back(&spacePoint1);
    hitlist.push_back(&spacePoint2);
    hitlist.push_back(&spacePoint3);

    // Perform fit
    result = qualityEstimator.estimateQualityAndProperties(hitlist);

    ASSERT_TRUE(result.chiSquared);
    ASSERT_NEAR(*(result.chiSquared), 0., allowed_chi2_deviation) << "2nd perfect circle";

    // Move one hit away from circle
    position3 = B2Vector3<double>(2., 0., 2. + 1e-6);
    spacePoint3 = SpacePoint(position3, sigma, {0.5, 0.5}, {false, false}, VxdID(0), Belle2::VXD::SensorInfoBase::VXD);

    hitlist.clear();
    hitlist.push_back(&spacePoint1);
    hitlist.push_back(&spacePoint2);
    hitlist.push_back(&spacePoint3);
    // Perform fit
    result = qualityEstimator.estimateQualityAndProperties(hitlist);

    ASSERT_TRUE(*(result.chiSquared) > allowed_chi2_deviation) << "1st imperfect circle";

    // Move one hit away from circle
    position3 = B2Vector3<double>(2. + 1e-6, 0., 2.);
    spacePoint3 = SpacePoint(position3, sigma, {0.5, 0.5}, {false, false}, VxdID(0), Belle2::VXD::SensorInfoBase::VXD);

    hitlist.clear();
    hitlist.push_back(&spacePoint1);
    hitlist.push_back(&spacePoint2);
    hitlist.push_back(&spacePoint3);
    // Perform fit
    result = qualityEstimator.estimateQualityAndProperties(hitlist);

    ASSERT_TRUE(result.chiSquared);
    ASSERT_TRUE(*(result.chiSquared) > allowed_chi2_deviation) << "2nd imperfect circle";
  }

  TEST(TripletFitTests, CombinedCircleChi2)
  {
    // Setup QualityEstimator
    auto qualityEstimator = QualityEstimatorTripletFit();
    qualityEstimator.setMagneticFieldStrength(1.5);

    // Some hitpoints that are exactly on a circle in x,y with radius 1 and m=(1,0,0)
    auto position1 = B2Vector3<double>(0., 0., 0.);
    auto position2 = B2Vector3<double>(1., 1., 0.);
    auto position3 = B2Vector3<double>(2., 0., 0.);
    auto position4 = B2Vector3<double>(1., -1., 0.);
    B2Vector3<double> sigma;

    auto spacePoint1 = SpacePoint(position1, sigma, {0.5, 0.5}, {false, false}, VxdID(0), Belle2::VXD::SensorInfoBase::VXD);
    auto spacePoint2 = SpacePoint(position2, sigma, {0.5, 0.5}, {false, false}, VxdID(0), Belle2::VXD::SensorInfoBase::VXD);
    auto spacePoint3 = SpacePoint(position3, sigma, {0.5, 0.5}, {false, false}, VxdID(0), Belle2::VXD::SensorInfoBase::VXD);
    auto spacePoint4 = SpacePoint(position4, sigma, {0.5, 0.5}, {false, false}, VxdID(0), Belle2::VXD::SensorInfoBase::VXD);

    std::vector<SpacePoint const*> hitlist;
    hitlist.reserve(4);
    hitlist.push_back(&spacePoint1);
    hitlist.push_back(&spacePoint2);
    hitlist.push_back(&spacePoint3);
    hitlist.push_back(&spacePoint4);
    // Perform fit
    auto result = qualityEstimator.estimateQualityAndProperties(hitlist);

    ASSERT_TRUE(result.chiSquared);
    ASSERT_NEAR(*(result.chiSquared), 0., allowed_chi2_deviation) << "1st perfect combined circle";

    // Combine two triplets with different radii;
    position4 = B2Vector3<double>(1., -2., 0.);
    spacePoint4 = SpacePoint(position4, sigma, {0.5, 0.5}, {false, false}, VxdID(0), Belle2::VXD::SensorInfoBase::VXD);

    hitlist.clear();
    hitlist.reserve(4);
    hitlist.push_back(&spacePoint1);
    hitlist.push_back(&spacePoint2);
    hitlist.push_back(&spacePoint3);
    hitlist.push_back(&spacePoint4);

    result = qualityEstimator.estimateQualityAndProperties(hitlist);

    ASSERT_TRUE(result.chiSquared);
    ASSERT_TRUE(*(result.chiSquared) > allowed_chi2_deviation) << "Incompatible circle triplets";
  }

  TEST(TripletFitTests, Interface)
  {
    // Setup QualityEstimator Class
    auto qualityEstimator = QualityEstimatorTripletFit();
    qualityEstimator.setMagneticFieldStrength(1.5);

    // Some hitpoints
    auto position1 = B2Vector3<double>(0., 0., 0.);
    auto position2 = B2Vector3<double>(1., 1., 0.);
    auto position3 = B2Vector3<double>(2., 0., 0.);
    B2Vector3<double> sigma;
    auto spacePoint1 = SpacePoint(position1, sigma, {0.5, 0.5}, {false, false}, VxdID(0), Belle2::VXD::SensorInfoBase::VXD);
    auto spacePoint2 = SpacePoint(position2, sigma, {0.5, 0.5}, {false, false}, VxdID(0), Belle2::VXD::SensorInfoBase::VXD);
    auto spacePoint3 = SpacePoint(position3, sigma, {0.5, 0.5}, {false, false}, VxdID(0), Belle2::VXD::SensorInfoBase::VXD);

    std::vector<SpacePoint const*> hitlist;
    hitlist.reserve(4);
    hitlist.push_back(&spacePoint1);
    hitlist.push_back(&spacePoint2);
    hitlist.push_back(&spacePoint3);

    // Test if calcChiSquared and calcCompleteResults return the same values
    auto result = qualityEstimator.estimateQuality(hitlist);
    auto resultObject = qualityEstimator.estimateQualityAndProperties(hitlist);
    ASSERT_EQ(resultObject.qualityIndicator, result);

    // check if the correct optional values are set
    ASSERT_FALSE(resultObject.p);

    ASSERT_TRUE(resultObject.chiSquared);
    ASSERT_TRUE(resultObject.pt);
    ASSERT_TRUE(resultObject.pmag);
    ASSERT_TRUE(resultObject.curvatureSign);

    // Test if tripletFit handles empty measurements vector correct
    hitlist.clear();

    result = qualityEstimator.estimateQuality(hitlist);
    resultObject = qualityEstimator.estimateQualityAndProperties(hitlist);
    ASSERT_EQ(result, resultObject.qualityIndicator);

    // check if the optional values got reset
    ASSERT_FALSE(resultObject.chiSquared);
    ASSERT_FALSE(resultObject.p);
    ASSERT_FALSE(resultObject.pt);
    ASSERT_FALSE(resultObject.pmag);
    ASSERT_FALSE(resultObject.curvatureSign);
  }
}

