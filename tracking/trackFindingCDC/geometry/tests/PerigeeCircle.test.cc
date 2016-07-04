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

using namespace std;

using namespace Belle2;
using namespace TrackFindingCDC;



TEST(TrackFindingCDCTest, geometry_PerigeeCircle_inheritance)
{

  double curvature = 1.0;
  double tangentialPhi = M_PI / 4.0;
  double impact = 1.0;

  // Checks if the normal parameters n follow the same sign convention
  PerigeeCircle perigeeCircle(curvature, tangentialPhi, impact);

  const GeneralizedCircle& generalizedCircle = perigeeCircle;
  //(perigeeCircle.n0(), perigeeCircle.n12(), perigeeCircle.n3());

  EXPECT_NEAR(curvature, generalizedCircle.curvature(), 10e-7);
  EXPECT_NEAR(impact, generalizedCircle.impact(), 10e-7);
  EXPECT_NEAR(perigeeCircle.tangential().x(), generalizedCircle.tangential().x(), 10e-7);
  EXPECT_NEAR(perigeeCircle.tangential().y(), generalizedCircle.tangential().y(), 10e-7);


  PerigeeCircle roundTripCircle(generalizedCircle);

  EXPECT_NEAR(curvature, roundTripCircle.curvature(), 10e-7);
  EXPECT_NEAR(impact, roundTripCircle.impact(), 10e-7);
  EXPECT_NEAR(cos(tangentialPhi), roundTripCircle.tangential().x(), 10e-7);
  EXPECT_NEAR(sin(tangentialPhi), roundTripCircle.tangential().y(), 10e-7);
  EXPECT_NEAR(tangentialPhi, roundTripCircle.tangentialPhi(), 10e-7);


  PerigeeCircle roundTripCircle2;
  roundTripCircle2.setN(generalizedCircle);

  EXPECT_NEAR(curvature, roundTripCircle2.curvature(), 10e-7);
  EXPECT_NEAR(impact, roundTripCircle2.impact(), 10e-7);
  EXPECT_NEAR(cos(tangentialPhi), roundTripCircle2.tangential().x(), 10e-7);
  EXPECT_NEAR(sin(tangentialPhi), roundTripCircle2.tangential().y(), 10e-7);
  EXPECT_NEAR(tangentialPhi, roundTripCircle2.tangentialPhi(), 10e-7);

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
  double tangentialPhi = 3. * M_PI / 4.;
  double impact = 1. - sqrt(2.);

  PerigeeCircle circle(curvature, tangentialPhi, impact);

  EXPECT_TRUE(circle.isCircle());
  EXPECT_FALSE(circle.isLine());

  EXPECT_NEAR(-1.0, circle.radius(), 10e-7);
  EXPECT_NEAR(1.0, circle.center().x(), 10e-7);
  EXPECT_NEAR(1.0, circle.center().y(), 10e-7);

  EXPECT_NEAR(curvature, circle.curvature(), 10e-7);
  EXPECT_NEAR(tangentialPhi, circle.tangentialPhi(), 10e-7);
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

  EXPECT_EQ(0., defaultCircle.n0());
  EXPECT_EQ(0., defaultCircle.n1());
  EXPECT_EQ(0., defaultCircle.n2());
  EXPECT_EQ(0., defaultCircle.n3());

  EXPECT_EQ(0., defaultCircle.curvature());
  EXPECT_TRUE(std::isnan(defaultCircle.tangentialPhi()));
  EXPECT_EQ(0., defaultCircle.tangential().x());
  EXPECT_EQ(0., defaultCircle.tangential().y());
  EXPECT_EQ(0., defaultCircle.impact());


  double curvature = -1.;
  double tangentialPhi = 3. * M_PI / 4.;
  double impact = 1. - sqrt(2.0);

  PerigeeCircle circle(curvature, tangentialPhi, impact);

  circle.invalidate();

  EXPECT_TRUE(circle.isInvalid());

  EXPECT_EQ(0., circle.n0());
  EXPECT_EQ(0., circle.n1());
  EXPECT_EQ(0., circle.n2());
  EXPECT_EQ(0., circle.n3());

  EXPECT_EQ(0., circle.curvature());
  EXPECT_TRUE(std::isnan(circle.tangentialPhi()));
  EXPECT_EQ(0., circle.tangential().x());
  EXPECT_EQ(0., circle.tangential().y());
  EXPECT_EQ(0., circle.impact());


  circle.reverse();

  EXPECT_TRUE(circle.isInvalid());

  EXPECT_EQ(0., circle.n0());
  EXPECT_EQ(0., circle.n1());
  EXPECT_EQ(0., circle.n2());
  EXPECT_EQ(0., circle.n3());

  EXPECT_EQ(0., circle.curvature());
  EXPECT_TRUE(std::isnan(circle.tangentialPhi()));
  EXPECT_EQ(0., circle.tangential().x());
  EXPECT_EQ(0., circle.tangential().y());
  EXPECT_EQ(0., circle.impact());


  circle =  circle.reversed();

  EXPECT_TRUE(circle.isInvalid());

  EXPECT_EQ(0., circle.n0());
  EXPECT_EQ(0., circle.n1());
  EXPECT_EQ(0., circle.n2());
  EXPECT_EQ(0., circle.n3());

  EXPECT_EQ(0., circle.curvature());
  EXPECT_TRUE(std::isnan(circle.tangentialPhi()));
  EXPECT_EQ(0., circle.tangential().x());
  EXPECT_EQ(0., circle.tangential().y());
  EXPECT_EQ(0., circle.impact());


  GeneralizedCircle generalizedCircle;
  generalizedCircle.invalidate();
  circle.setN(generalizedCircle);

  EXPECT_TRUE(circle.isInvalid());

  EXPECT_EQ(0., circle.n0());
  EXPECT_EQ(0., circle.n1());
  EXPECT_EQ(0., circle.n2());
  EXPECT_EQ(0., circle.n3());

  EXPECT_EQ(0., circle.curvature());
  EXPECT_TRUE(std::isnan(circle.tangentialPhi()));
  EXPECT_EQ(0., circle.tangential().x());
  EXPECT_EQ(0., circle.tangential().y());
  EXPECT_EQ(0., circle.impact());
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




TEST(TrackFindingCDCTest, geometry_PerigeeCircle_atArcLength)
{
  double radius = 1;
  Vector2D center = Vector2D(2.0, 0.0);

  PerigeeCircle circle = PerigeeCircle::fromCenterAndRadius(center, radius);

  double smallAngle = M_PI / 100;
  Vector2D near(2.0 - cos(smallAngle), sin(smallAngle));

  double nearArcLength = -smallAngle * radius; //Minus because of default counterclockwise orientation

  Vector2D atNear = circle.atArcLength(nearArcLength);

  EXPECT_NEAR(near.x(), atNear.x(), 10e-7);
  EXPECT_NEAR(near.y(), atNear.y(), 10e-7);


  Vector2D down(2.0, -1.0);
  double downArcLength = +M_PI / 2.0 * radius; //Plus because of default counterclockwise orientation

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


TEST(TrackFindingCDCTest, geometry_PerigeeCircle_OriginCircleFromPointDirection)
{
  double expectedCurvature = 1.0 / 2.0;
  double expectedPhi0 = M_PI / 4.0;
  double impact = 0;

  // Checks if the normal parameters n follow the same sign convention
  const PerigeeCircle perigeeCircle(expectedCurvature,
                                    expectedPhi0,
                                    impact);
  const Vector2D& expectedPhi0Vec = perigeeCircle.tangential();

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
