/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jakob Lettenbichler                                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>
#include <tracking/spacePointCreation/SpacePoint.h>
#include <vxd/dataobjects/VxdID.h>
#include <vxd/geometry/SensorInfoBase.h>
#include <vxd/geometry/SensorPlane.h>
#include <gtest/gtest.h>


using namespace std;

namespace Belle2 {
  /** command x should exit using B2FATAL. */
#define EXPECT_FATAL(x) EXPECT_EXIT(x,::testing::KilledBySignal(SIGABRT),"");

  /** Set up a few arrays and objects in the datastore */
  class SpacePointTest : public ::testing::Test {
  protected:
  };

  /** Test constructor for PXDClsuters
   * tests the constructor importing a PXDCluster and tests results by
   * using the getters of the spacePoint...
   */
  TEST_F(SpacePointTest, testConstructorPXD)
  {
    VxdID aVxdID = VxdID(1, 1, 1);

    VXD::SensorInfoBase sensorInfoBase(VXD::SensorInfoBase::PXD, aVxdID, 1.0, 2.0, 3.0, 2, 4);

    TGeoRotation r1;
    r1.SetAngles(45, 20, 30);      // rotation defined by Euler angles
    TGeoTranslation t1(-10, 10, 1);
    TGeoCombiTrans c1(t1, r1);
    TGeoHMatrix transform = c1;
    sensorInfoBase.setTransformation(transform);

    // create new PXDCluster and fill it with Info getting a Hit which is not at the origin (here, first layer)

    PXDCluster aCluster = PXDCluster(aVxdID, 0., 0., 0.1, 0.1, 0, 0, 1, 1, 1, 1, 1, 1);
    SpacePoint testPoint = SpacePoint(aCluster, 3, &sensorInfoBase);
//     SpacePoint testPoint = SpacePoint(aCluster, 3);

    EXPECT_DOUBLE_EQ(aVxdID, testPoint.getVxdID());

    // needing globalized position and error:
    TVector3 aPosition = sensorInfoBase.pointToGlobal(TVector3(aCluster.getU(), aCluster.getV(), 0));
    TVector3 globalizedVariances = sensorInfoBase.vectorToGlobal(
                                     TVector3(
                                       aCluster.getUSigma() * aCluster.getUSigma(),
                                       aCluster.getVSigma() * aCluster.getVSigma(),
                                       0
                                     )
                                   );

    TVector3 globalError;
    for (int i = 0; i < 3; i++) { globalError[i] = sqrt(abs(globalizedVariances[i])); }

    EXPECT_DOUBLE_EQ(aPosition.X(), testPoint.getPosition().X());
    EXPECT_DOUBLE_EQ(aPosition.Y(), testPoint.getPosition().Y());
    EXPECT_DOUBLE_EQ(aPosition.Z(), testPoint.getPosition().Z());
    EXPECT_DOUBLE_EQ(globalError.X(), testPoint.getPositionError().X());
    EXPECT_DOUBLE_EQ(globalError.Y(), testPoint.getPositionError().Y());
    EXPECT_DOUBLE_EQ(globalError.Z(), testPoint.getPositionError().Z());
    // normalized coordinates, center of Plane should be at 0.5:
    EXPECT_DOUBLE_EQ(0.5, testPoint.getNormalizedLocalU());
    EXPECT_DOUBLE_EQ(0.5, testPoint.getNormalizedLocalV());

  }

}  // namespace
