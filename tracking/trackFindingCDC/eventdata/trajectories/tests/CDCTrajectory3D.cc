/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost <oliver.frost@desy.de>                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/trackFindingCDC/eventdata/trajectories/CDCTrajectory3D.h>

#include <genfit/TrackCand.h>

#include <boost/range/irange.hpp>

#include <gtest/gtest.h>

using namespace std;

using namespace Belle2;
using namespace TrackFindingCDC;

using boost::irange;

TEST(TrackFindingCDCTest, eventdata_trajectories_CDCTrajectory3D_constructorPosMomCharge)
{
  Vector3D newMom3D(1.0, 2.0, 1.0);
  Vector3D newPos3D(1.0, 2.0, 1.0);
  SignType newChargeSign = PLUS;
  FloatType bZ = 2.0;

  CDCTrajectory3D trajectory(newPos3D, newMom3D, newChargeSign, bZ);;

  Vector3D mom3D = trajectory.getMom3DAtSupport(bZ);
  Vector3D pos3D = trajectory.getSupport();
  SignType chargeSign = trajectory.getChargeSign();


  EXPECT_NEAR(newMom3D.x(), mom3D.x(), 10e-7);
  EXPECT_NEAR(newMom3D.y(), mom3D.y(), 10e-7);
  EXPECT_NEAR(newMom3D.z(), mom3D.z(), 10e-7);

  EXPECT_NEAR(newPos3D.x(), pos3D.x(), 10e-7);
  EXPECT_NEAR(newPos3D.y(), pos3D.y(), 10e-7);
  EXPECT_NEAR(newPos3D.z(), pos3D.z(), 10e-7);

  EXPECT_NEAR(newChargeSign, chargeSign, 10e-7);
}



TEST(TrackFindingCDCTest, CDCTrajectory3D_clear)
{
  Vector3D newMom3D(1.0, 2.0, 1.0);
  Vector3D newPos3D(1.0, 2.0, 1.0);
  SignType newChargeSign = PLUS;

  CDCTrajectory3D trajectory3D(newPos3D, newMom3D, newChargeSign);
  UncertainHelix uncertainHelix = trajectory3D.getLocalHelix();
  Helix helix = uncertainHelix;

  PerigeeCircle perigeeCircle = helix.circleXY();
  GeneralizedCircle generalizedCircle = perigeeCircle;

  Line2D lineSZ = helix.lineSZ();

  trajectory3D.clear();
  EXPECT_TRUE(trajectory3D.isNull());
  EXPECT_FALSE(trajectory3D.isFitted());

  uncertainHelix.setNull();
  EXPECT_TRUE(uncertainHelix.isNull());

  helix.setNull();
  EXPECT_TRUE(helix.isNull());

  perigeeCircle.setNull();
  EXPECT_TRUE(perigeeCircle.isNull());

  generalizedCircle.setNull();
  EXPECT_TRUE(generalizedCircle.isNull());

  lineSZ.setNull();
  EXPECT_TRUE(lineSZ.isNull());
}


TEST(TrackFindingCDCTest, CDCTrajectory3D_GFTrackRoundTrip)
{
  Vector3D expectedMomentum(1.0, 0.0, 0.0);
  Vector3D expectedPosition(0.0, 1.0, 0.0);
  SignType expectedCharge = PLUS;
  FloatType bZ = 2;

  genfit::TrackCand expectedGFTrackCand;
  expectedGFTrackCand.setPosMomSeed(expectedPosition,
                                    expectedMomentum,
                                    expectedCharge);

  TMatrixDSym expectedCov6(6);
  expectedCov6.Zero();
  expectedCov6(0, 0) = 0; // There cannot be a covariance in the x direction since the direction along the track has no constraint.
  expectedCov6(1, 1) = 2;
  expectedCov6(2, 2) = 3;
  expectedCov6(3, 3) = 4;
  expectedCov6(4, 4) = 5;
  expectedCov6(5, 5) = 6;
  expectedGFTrackCand.setCovSeed(expectedCov6);


  CDCTrajectory3D trajectory3D(expectedGFTrackCand, bZ);

  genfit::TrackCand gfTrackCand;
  trajectory3D.fillInto(gfTrackCand, bZ);

  Vector3D position = gfTrackCand.getPosSeed();
  Vector3D momentum = gfTrackCand.getMomSeed();
  SignType charge = gfTrackCand.getChargeSeed();
  TMatrixDSym cov6 = gfTrackCand.getCovSeed();


  EXPECT_NEAR(expectedPosition.x(), position.x(), 10e-7);
  EXPECT_NEAR(expectedPosition.y(), position.y(), 10e-7);
  EXPECT_NEAR(expectedPosition.z(), position.z(), 10e-7);

  EXPECT_NEAR(expectedMomentum.x(), momentum.x(), 10e-7);
  EXPECT_NEAR(expectedMomentum.y(), momentum.y(), 10e-7);
  EXPECT_NEAR(expectedMomentum.z(), momentum.z(), 10e-7);

  EXPECT_EQ(expectedCharge, charge);

  for (int i : irange(0, 6)) {
    for (int j : irange(0, 6)) {
      EXPECT_NEAR(expectedCov6(i, j), cov6(i, j), 10e-7);
    }
  }

}
