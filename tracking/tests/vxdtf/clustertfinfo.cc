#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>
#include <tracking/trackFindingVXD/displayInterfaceTF/ClusterTFInfo.h>
// #include <TMatrixF.h>
// #include <RKTrackRep.h>
#include <gtest/gtest.h>

using namespace std;

namespace Belle2 {



  /** Set up a few arrays and objects in the datastore */
  class ClusterTFInfoTest : public ::testing::Test {
  protected:
  };


  /** Tests empty ClusterTFInfo */
  TEST_F(ClusterTFInfoTest, testEmptyFilter)
  {
    ClusterTFInfo aCluster = ClusterTFInfo();

    ASSERT_EQ(-1., aCluster.getRealClusterID());
  }

  /** Test simple Setters and Getters of ClusterTFInfo */
  TEST_F(ClusterTFInfoTest, testFilledFilter)
  {
    ClusterTFInfo aCluster = ClusterTFInfo();
    aCluster.setRealClusterID(12);
    aCluster.setPDG(11);
    aCluster.setIsReal(1);
    aCluster.setParticleID(1);
    aCluster.setDetectorType(1);
    aCluster.setRelativeClusterID(2);

    ASSERT_EQ(12., aCluster.getRealClusterID());
    ASSERT_EQ(11., aCluster.getPDG());
    ASSERT_EQ(1., aCluster.getIsReal());
    ASSERT_EQ(1., aCluster.getParticleID());
    ASSERT_EQ(1., aCluster.getDetectorType());
    ASSERT_EQ(2., aCluster.getRelativeClusterID());
  }


  /** Test Overlapped Logic of ClusterTFInfo */
  TEST_F(ClusterTFInfoTest, testOverlapped)
  {
    bool false_item = false;
    bool true_item = true;

    ClusterTFInfo aCluster = ClusterTFInfo();
    aCluster.setRealClusterID(12);
    aCluster.setPDG(11);
    aCluster.setIsReal(1);
    aCluster.setParticleID(1);

    aCluster.setUseCounter(1);
    aCluster.changeUseCounter(1);

    ASSERT_EQ(true_item, aCluster.isOverlapped());

    aCluster.changeUseCounter(-1);

    ASSERT_EQ(false_item, aCluster.isOverlapped());
    ASSERT_EQ(false_item, aCluster.isOverlapped());

    ASSERT_EQ(2., aCluster.getMaxCounter());
    ASSERT_EQ(1., aCluster.getUseCounter());
  }


}
