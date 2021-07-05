/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost <oliver.frost@desy.de>                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/trackFindingCDC/geometry/PerigeeCircle.h>
#include <tracking/trackFindingCDC/geometry/GeneralizedCircle.h>
#include <tracking/trackFindingCDC/geometry/Vector2D.h>

#include <gtest/gtest.h>


using namespace Belle2;
using namespace TrackFindingCDC;

TEST(TrackFindingCDCTest, geometry_PerigeeCircle_inheritance)
{

  double curvature = 1.0;
  double phi0 = M_PI / 4.0;
  double impact = 1.0;

  // Checks if the normal parameters n follow the same sign convention
  PerigeeCircle perigeeCircle(curvature, phi0, impact);

  const GeneralizedCircle generalizedCircle(perigeeCircle.n0(),
                                            perigeeCircle.n12(),
                                            perigeeCircle.n3());

  EXPECT_NEAR(curvature, generalizedCircle.curvature(), 10e-7);
  EXPECT_NEAR(impact, generalizedCircle.impact(), 10e-7);
  EXPECT_NEAR(perigeeCircle.tangential().x(), generalizedCircle.tangential().x(), 10e-7);
  EXPECT_NEAR(perigeeCircle.tangential().y(), generalizedCircle.tangential().y(), 10e-7);

  PerigeeCircle roundTripCircle(generalizedCircle);

  EXPECT_NEAR(curvature, roundTripCircle.curvature(), 10e-7);
  EXPECT_NEAR(impact, roundTripCircle.impact(), 10e-7);
  EXPECT_NEAR(cos(phi0), roundTripCircle.phi0Vec().x(), 10e-7);
  EXPECT_NEAR(sin(phi0), roundTripCircle.phi0Vec().y(), 10e-7);
  EXPECT_NEAR(phi0, roundTripCircle.phi0(), 10e-7);

  PerigeeCircle roundTripCircle2;
  roundTripCircle2.setN(generalizedCircle);

  EXPECT_NEAR(curvature, roundTripCircle2.curvature(), 10e-7);
  EXPECT_NEAR(impact, roundTripCircle2.impact(), 10e-7);
  EXPECT_NEAR(cos(phi0), roundTripCircle2.tangential().x(), 10e-7);
  EXPECT_NEAR(sin(phi0), roundTripCircle2.tangential().y(), 10e-7);
  EXPECT_NEAR(phi0, roundTripCircle2.phi0(), 10e-7);
}

TEST(TrackFindingCDCTest, geometry_PerigeeCircle_isLine)
{
  double radius = 1;
  Vector2D center = Vector2D(2.0, 0.0);
  PerigeeCircle circle = PerigeeCircle::fromCenterAndRadius(center, radius);

  EXPECT_FALSE(circle.isLine());

  float curvature = 0;
  float phi0 = M_PI / 2;
  float impact = -1;
  PerigeeCircle line = PerigeeCircle(curvature, phi0, impact);

  EXPECT_TRUE(line.isLine());
}

TEST(TrackFindingCDCTest, geometry_PerigeeCircle_isCircle)
{
  double radius = 1;
  Vector2D center = Vector2D(2.0, 0.0);
  PerigeeCircle circle = PerigeeCircle::fromCenterAndRadius(center, radius);

  EXPECT_TRUE(circle.isCircle());

  float curvature = 0;
  float phi0 = M_PI / 2;
  float impact = -1;
  PerigeeCircle line = PerigeeCircle(curvature, phi0, impact);

  EXPECT_FALSE(line.isCircle());
}

TEST(TrackFindingCDCTest, geometry_PerigeeCircle_orientation)
{
  double curvature = 1;
  Vector2D phi0 = Vector2D::Phi(1);
  double impact = 1;

  PerigeeCircle circle(curvature, phi0, impact);
  EXPECT_EQ(ERotation::c_CounterClockwise, circle.orientation());

  PerigeeCircle reversedCircle = circle.reversed();
  EXPECT_EQ(ERotation::c_Clockwise, reversedCircle.orientation());

  curvature = 0;
  PerigeeCircle line(curvature, phi0, impact);
  EXPECT_EQ(ERotation::c_CounterClockwise, line.orientation());

  PerigeeCircle reversedLine = line.reversed();
  EXPECT_EQ(ERotation::c_Clockwise, reversedLine.orientation());
}

TEST(TrackFindingCDCTest, geometry_PerigeeCircle_minimalCylindricalR)
{
  double curvature = 1.0 / 2.0;
  double tangtialPhi = M_PI / 4.0;
  double impact = -1.0;

  // Checks if the normal parameters n follow the same sign convention
  PerigeeCircle perigeeCircle(curvature, tangtialPhi, impact);

  EXPECT_EQ(1, perigeeCircle.minimalCylindricalR());
}

TEST(TrackFindingCDCTest, geometry_PerigeeCircle_maximalCylindricalR)
{
  double curvature = 1.0 / 2.0;
  double tangtialPhi = M_PI / 4.0;
  double impact = -1.0;

  // Checks if the normal parameters n follow the same sign convention
  PerigeeCircle perigeeCircle(curvature, tangtialPhi, impact);

  EXPECT_EQ(3, perigeeCircle.maximalCylindricalR());
}

TEST(TrackFindingCDCTest, geometry_PerigeeCircle_setCenterAndRadius)
{
  PerigeeCircle circle;
  Vector2D center(0.5, 0.0);
  double radius = 1.5;
  circle.setCenterAndRadius(center, radius, ERotation::c_CounterClockwise);

  EXPECT_TRUE(circle.isCircle());
  EXPECT_FALSE(circle.isLine());

  EXPECT_NEAR(1.5, circle.radius(), 10e-7);
  EXPECT_NEAR(0.5, circle.center().x(), 10e-7);
  EXPECT_NEAR(0.0, circle.center().y(), 10e-7);
}

TEST(TrackFindingCDCTest, geometry_PerigeeCircle_distance)
{

  double curvature = -1.;
  double phi0 = 3. * M_PI / 4.;
  double impact = 1. - sqrt(2.);

  PerigeeCircle circle(curvature, phi0, impact);

  EXPECT_TRUE(circle.isCircle());
  EXPECT_FALSE(circle.isLine());

  EXPECT_NEAR(-1.0, circle.radius(), 10e-7);
  EXPECT_NEAR(1.0, circle.center().x(), 10e-7);
  EXPECT_NEAR(1.0, circle.center().y(), 10e-7);

  EXPECT_NEAR(curvature, circle.curvature(), 10e-7);
  EXPECT_NEAR(phi0, circle.phi0(), 10e-7);
  EXPECT_NEAR(impact, circle.impact(), 10e-7);

  EXPECT_NEAR(0, circle.distance(Vector2D(1.0, 0.0)), 10e-7);
  EXPECT_NEAR(0, circle.distance(Vector2D(0.0, 1.0)), 10e-7);
  EXPECT_NEAR(impact, circle.distance(Vector2D(0.0, 0.0)), 10e-7);

  EXPECT_NEAR(0.5, circle.distance(Vector2D(1.0, 0.5)), 10e-7);

  EXPECT_NEAR(-0.5, circle.distance(Vector2D(1.0, 2.5)), 10e-7);
  EXPECT_NEAR(-0.5, circle.distance(Vector2D(2.5, 1.0)), 10e-7);
}

TEST(TrackFindingCDCTest, geometry_PerigeeCircle_invalidate)
{
  PerigeeCircle defaultCircle;
  EXPECT_TRUE(defaultCircle.isInvalid());

  double curvature = -1.;
  double phi0 = 3. * M_PI / 4.;
  double impact = 1. - sqrt(2.0);

  PerigeeCircle circle(curvature, phi0, impact);

  circle.invalidate();
  EXPECT_TRUE(circle.isInvalid());

  circle.reverse();
  EXPECT_TRUE(circle.isInvalid());

  circle = circle.reversed();
  EXPECT_TRUE(circle.isInvalid());

  GeneralizedCircle generalizedCircle;
  generalizedCircle.invalidate();
  circle.setN(generalizedCircle);

  EXPECT_TRUE(circle.isInvalid());
}

TEST(TrackFindingCDCTest, geometry_PerigeeCircle_passiveMoveBy)
{
  Vector2D center(4.0, 2.0);
  double radius = 5.0;
  PerigeeCircle circle = PerigeeCircle::fromCenterAndRadius(center, radius);

  circle.passiveMoveBy(Vector2D(4.0, 0.0));

  EXPECT_NEAR(5.0, circle.radius(), 10e-7);
  EXPECT_NEAR(0.0, circle.perigee().x(), 10e-7);
  EXPECT_NEAR(-3.0, circle.perigee().y(), 10e-7);
}

TEST(TrackFindingCDCTest, geometry_PerigeeCircle_conformalTranform)
{
  Vector2D center(1.0, 0.0);
  double radius = 1.0;
  PerigeeCircle circle = PerigeeCircle::fromCenterAndRadius(center, radius);

  // Get two points on the circle to check for the orientation to be correct
  Vector2D firstPos = circle.atArcLength(1);
  Vector2D secondPos = circle.atArcLength(2);

  EXPECT_NEAR(1.0, circle.curvature(), 10e-7);
  EXPECT_NEAR(-M_PI / 2.0, circle.phi0(), 10e-7);
  EXPECT_NEAR(0.0, circle.impact(), 10e-7);
  EXPECT_NEAR(0.0, circle.distance(firstPos), 10e-7);
  EXPECT_NEAR(0.0, circle.distance(secondPos), 10e-7);

  circle.conformalTransform();
  firstPos.conformalTransform();
  secondPos.conformalTransform();

  EXPECT_NEAR(0.0, circle.curvature(), 10e-7);
  EXPECT_NEAR(M_PI / 2.0, circle.phi0(), 10e-7);
  EXPECT_NEAR(-1.0 / 2.0, circle.impact(), 10e-7);

  double firstConformalArcLength = circle.arcLengthTo(firstPos);
  double secondConformalArcLength = circle.arcLengthTo(secondPos);
  EXPECT_LT(firstConformalArcLength, secondConformalArcLength);

  EXPECT_NEAR(0.0, circle.distance(firstPos), 10e-7);
  EXPECT_NEAR(0.0, circle.distance(secondPos), 10e-7);

  // Another conformal transformation goes back to the original circle
  PerigeeCircle conformalCopy = circle.conformalTransformed();
  EXPECT_NEAR(1.0, conformalCopy.curvature(), 10e-7);
  EXPECT_NEAR(-M_PI / 2.0, conformalCopy.phi0(), 10e-7);
  EXPECT_NEAR(0.0, conformalCopy.impact(), 10e-7);
}

TEST(TrackFindingCDCTest, geometry_PerigeeCircle_closest)
{
  PerigeeCircle circle(1.0, Vector2D(0.0, -1.0), 1.0);
  Vector2D up(2.0, 2.0);
  Vector2D far(5.0, 0.0);

  EXPECT_EQ(Vector2D(2.0, 1.0), circle.closest(up));
  EXPECT_EQ(Vector2D(3.0, 0.0), circle.closest(far));

  // This tests for point which is on the circle
  double smallAngle = M_PI / 100;
  Vector2D near(2.0 - cos(smallAngle), sin(smallAngle));

  Vector2D closestOfNear = circle.closest(near);
  EXPECT_NEAR(near.x(), closestOfNear.x(), 10e-7);
  EXPECT_NEAR(near.y(), closestOfNear.y(), 10e-7);
}

TEST(TrackFindingCDCTest, geometry_PerigeeCircle_atArcLength)
{
  double radius = 1;
  Vector2D center = Vector2D(2.0, 0.0);

  PerigeeCircle circle = PerigeeCircle::fromCenterAndRadius(center, radius);

  double smallAngle = M_PI / 100;
  Vector2D near(2.0 - cos(smallAngle), sin(smallAngle));

  double nearArcLength =
    -smallAngle * radius; // Minus because of default counterclockwise orientation

  Vector2D atNear = circle.atArcLength(nearArcLength);

  EXPECT_NEAR(near.x(), atNear.x(), 10e-7);
  EXPECT_NEAR(near.y(), atNear.y(), 10e-7);

  Vector2D down(2.0, -1.0);
  double downArcLength =
    +M_PI / 2.0 * radius; // Plus because of default counterclockwise orientation

  Vector2D atDown = circle.atArcLength(downArcLength);

  EXPECT_NEAR(down.x(), atDown.x(), 10e-7);
  EXPECT_NEAR(down.y(), atDown.y(), 10e-7);
}

TEST(TrackFindingCDCTest, geometry_PerigeeCircle_arcLengthToCylindricalR)
{
  double radius = 1;
  Vector2D center = Vector2D(2.0, 0.0);

  PerigeeCircle circle = PerigeeCircle::fromCenterAndRadius(center, radius);
  {
    double closestArcLength = circle.arcLengthToCylindricalR(1);
    EXPECT_NEAR(0, closestArcLength, 10e-7);

    double widestArcLength = circle.arcLengthToCylindricalR(3);
    EXPECT_NEAR(M_PI, widestArcLength, 10e-7);

    double halfArcLength = circle.arcLengthToCylindricalR(sqrt(5.0));
    EXPECT_NEAR(M_PI / 2, halfArcLength, 10e-7);

    double unreachableHighArcLength = circle.arcLengthToCylindricalR(4);
    EXPECT_TRUE(std::isnan(unreachableHighArcLength));

    double unreachableLowArcLength = circle.arcLengthToCylindricalR(0.5);
    EXPECT_TRUE(std::isnan(unreachableLowArcLength));
  }

  PerigeeCircle reversedCircle = circle.reversed();
  {
    double closestArcLength = reversedCircle.arcLengthToCylindricalR(1);
    EXPECT_NEAR(0, closestArcLength, 10e-7);

    double widestArcLength = reversedCircle.arcLengthToCylindricalR(3);
    EXPECT_NEAR(M_PI, widestArcLength, 10e-7);

    double halfArcLength = reversedCircle.arcLengthToCylindricalR(sqrt(5.0));
    EXPECT_NEAR(M_PI / 2, halfArcLength, 10e-7);

    double unreachableHighArcLength = reversedCircle.arcLengthToCylindricalR(4);
    EXPECT_TRUE(std::isnan(unreachableHighArcLength));

    double unreachableLowArcLength = reversedCircle.arcLengthToCylindricalR(0.5);
    EXPECT_TRUE(std::isnan(unreachableLowArcLength));
  }
}

TEST(TrackFindingCDCTest, geometry_PerigeeCircle_atCylindricalR)
{
  double radius = 1;
  Vector2D center = Vector2D(2.0, 0.0);
  PerigeeCircle circle = PerigeeCircle::fromCenterAndRadius(center, radius);

  std::pair<Vector2D, Vector2D> solutions = circle.atCylindricalR(sqrt(5.0));

  EXPECT_NEAR(2, solutions.first.x(), 10e-7);
  EXPECT_NEAR(1, solutions.first.y(), 10e-7);

  EXPECT_NEAR(2, solutions.second.x(), 10e-7);
  EXPECT_NEAR(-1, solutions.second.y(), 10e-7);
}

TEST(TrackFindingCDCTest, geometry_PerigeeCircle_atCylindricalR_opposite_orientation)
{
  double radius = 1;
  Vector2D center = Vector2D(2.0, 0.0);
  PerigeeCircle circle = PerigeeCircle::fromCenterAndRadius(center, radius, ERotation::c_Clockwise);

  std::pair<Vector2D, Vector2D> solutions = circle.atCylindricalR(sqrt(5.0));

  EXPECT_NEAR(2, solutions.first.x(), 10e-7);
  EXPECT_NEAR(-1, solutions.first.y(), 10e-7);

  EXPECT_NEAR(2, solutions.second.x(), 10e-7);
  EXPECT_NEAR(1, solutions.second.y(), 10e-7);
}

TEST(TrackFindingCDCTest, geometry_PerigeeCircle_OriginCircleFromPointDirection)
{
  double expectedCurvature = 1.0 / 2.0;
  double expectedPhi0 = M_PI / 4.0;
  double impact = 0;

  // Checks if the normal parameters n follow the same sign convention
  const PerigeeCircle perigeeCircle(expectedCurvature, expectedPhi0, impact);
  const Vector2D& expectedPhi0Vec = perigeeCircle.phi0Vec();

  double randomArcLength = 2.0;
  Vector2D pos2D = perigeeCircle.atArcLength(randomArcLength);
  Vector2D phiVec = perigeeCircle.tangential(pos2D);

  double curvature = 2 * pos2D.cross(phiVec) / pos2D.normSquared();
  Vector2D phi0Vec = phiVec.flippedOver(pos2D);

  EXPECT_NEAR(expectedCurvature, curvature, 10e-7);
  EXPECT_NEAR(expectedPhi0Vec.x(), phi0Vec.x(), 10e-7);
  EXPECT_NEAR(expectedPhi0Vec.y(), phi0Vec.y(), 10e-7);
  EXPECT_NEAR(expectedPhi0, phi0Vec.phi(), 10e-7);
  EXPECT_NEAR(1, phi0Vec.norm(), 10e-7);
}
