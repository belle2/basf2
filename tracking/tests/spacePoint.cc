#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>
#include <tracking/spacePointCreation/SpacePoint.h>
#include <vxd/dataobjects/VxdID.h>
#include <gtest/gtest.h>
// #include <vxd/geometry/GeoCache.h>
#include <geometry/GeometryManager.h>

using namespace std;

namespace Belle2 {
  /** command x should exit using B2FATAL. */
#define EXPECT_FATAL(x) EXPECT_EXIT(x,::testing::KilledBySignal(SIGABRT),"");

  /** Set up a few arrays and objects in the datastore */
  class SpacePointTest : public ::testing::Test {
  protected:
  };

  /** Test constructorForPXDClsuters */
  TEST_F(SpacePointTest, testConstructorPXD)
  {

    // TODO howto load a geometry or get another working bleh for testing?
// // // //     geometry::GeometryManager& geoManager = geometry::GeometryManager::getInstance();
// // // //     geoManager.createTGeoRepresentation();
//    VXD::GeoCache& aGeometry = VXD::GeoCache::getInstance();

    // create new PXDCluster and fill it with Info getting a Hit which is not at the origin (here, first layer)
    VxdID aVxdID = VxdID(1, 1, 1);
    PXDCluster aCluster = PXDCluster(aVxdID, 0., 0., 0, 0, 1, 1, 1, 1, 1);
    SpacePoint testPoint = SpacePoint(aCluster, 3);

//    EXPECT_DOUBLE_EQ(aVxdID, testPoint.getVxdID());

    // needing globalized position and error:
//    const VXD::SensorInfoBase& sensorInfoBase = aGeometry.getSensorInfo(aVxdID);
//    TVector3 aPosition = sensorInfoBase.pointToGlobal(TVector3(aCluster.getU(), aCluster.getV(), 0));
//    TVector3 globalizedVariances = sensorInfoBase.vectorToGlobal(
//      TVector3(
//        aCluster.getUSigma()*aCluster.getUSigma(),
//        aCluster.getVSigma()*aCluster.getVSigma(),
//        0
//      )
//    );
//    TVector3 globalError;
//    for (int i = 0; i < 3; i++) { globalError[i] = sqrt(globalizedVariances[i]); }
//
//    EXPECT_DOUBLE_EQ(aPosition.X(), testPoint.getPosition().X());
//    EXPECT_DOUBLE_EQ(aPosition.Y(), testPoint.getPosition().Y());
//    EXPECT_DOUBLE_EQ(aPosition.Z(), testPoint.getPosition().Z());
//    EXPECT_DOUBLE_EQ(globalError.X(), testPoint.getPositionError().X());
//    EXPECT_DOUBLE_EQ(globalError.Y(), testPoint.getPositionError().Y());
//    EXPECT_DOUBLE_EQ(globalError.Z(), testPoint.getPositionError().Z());
//    // normalized coordinates, center of Plane should be at 0.5:
//    EXPECT_DOUBLE_EQ(0.5, testPoint.getNormalizedLocalU());
//    EXPECT_DOUBLE_EQ(0.5, testPoint.getNormalizedLocalV());


  }

}  // namespace
