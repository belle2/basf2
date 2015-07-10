#include <mdst/dataobjects/ECLCluster.h>
#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>
#include <framework/datastore/StoreArray.h>
#include <gtest/gtest.h>

using namespace std;

namespace Belle2 {


  /** Test class for the Track object.
   */
  class ECLClusterTest : public ::testing::Test {
  protected:
  };

  /** Test simple Setters and Getters.
   */
  TEST_F(ECLClusterTest, Constructors)
  {
    ECLCluster myECLCluster;

    EXPECT_EQ(0, myECLCluster.getEnergy());
    EXPECT_EQ(0, myECLCluster.getE9oE25());
    EXPECT_EQ(0, myECLCluster.getEnedepSum());
    EXPECT_EQ(0, myECLCluster.getTheta());
    EXPECT_EQ(0, myECLCluster.getPhi());
    EXPECT_EQ(0, myECLCluster.getR());
    EXPECT_EQ(0, myECLCluster.getTiming());
    EXPECT_EQ(0, myECLCluster.getErrorTiming());
    EXPECT_EQ(0, myECLCluster.getHighestE());
    EXPECT_EQ(0, myECLCluster.getCrystHealth());
    EXPECT_EQ(0, myECLCluster.getNofCrystals());
    EXPECT_EQ(0, myECLCluster.getLAT());
    EXPECT_EQ(0, myECLCluster.getMergedPi0());

    EXPECT_EQ(0, myECLCluster.getErrorEnergy());
    EXPECT_EQ(0, myECLCluster.getErrorTheta());
    EXPECT_EQ(0, myECLCluster.getErrorPhi());

    EXPECT_EQ(0, myECLCluster.getMergedPi0());
    EXPECT_EQ(0, myECLCluster.getPx());
    EXPECT_EQ(0, myECLCluster.getPy());
    EXPECT_EQ(0, myECLCluster.getPz());

    EXPECT_FALSE(myECLCluster.getisTrack());
    EXPECT_TRUE(myECLCluster.isNeutral());

    double x = myECLCluster.getMomentum().X();
    double y = myECLCluster.getMomentum().Y();
    double z = myECLCluster.getMomentum().Z();
    EXPECT_EQ(0, x);
    EXPECT_EQ(0, y);
    EXPECT_EQ(0, z);

    x = myECLCluster.get4Vector().X();
    y = myECLCluster.get4Vector().Y();
    z = myECLCluster.get4Vector().Z();
    double e = myECLCluster.get4Vector().E();
    EXPECT_EQ(0, x);
    EXPECT_EQ(0, y);
    EXPECT_EQ(0, z);
    EXPECT_EQ(0, e);

    x = myECLCluster.getclusterPosition().X();
    y = myECLCluster.getclusterPosition().Y();
    z = myECLCluster.getclusterPosition().Z();
    EXPECT_EQ(0, x);
    EXPECT_EQ(0, y);
    EXPECT_EQ(0, z);

    x = myECLCluster.getPosition().X();
    y = myECLCluster.getPosition().Y();
    z = myECLCluster.getPosition().Z();
    EXPECT_EQ(0, x);
    EXPECT_EQ(0, y);
    EXPECT_EQ(0, z);

    const auto error3x3 = myECLCluster.getError3x3();
    EXPECT_EQ(0, error3x3(0, 0));
    EXPECT_EQ(0, error3x3(0, 1));
    EXPECT_EQ(0, error3x3(0, 2));
    EXPECT_EQ(0, error3x3(1, 0));
    EXPECT_EQ(0, error3x3(1, 1));
    EXPECT_EQ(0, error3x3(1, 2));
    EXPECT_EQ(0, error3x3(2, 0));
    EXPECT_EQ(0, error3x3(2, 1));
    EXPECT_EQ(0, error3x3(2, 2));

    const auto error4x4 = myECLCluster.getError4x4();
    EXPECT_EQ(0, error4x4(0, 0));
    EXPECT_EQ(0, error4x4(0, 1));
    EXPECT_EQ(0, error4x4(0, 2));
    EXPECT_EQ(0, error4x4(0, 3));
    EXPECT_EQ(0, error4x4(1, 0));
    EXPECT_EQ(0, error4x4(1, 1));
    EXPECT_EQ(0, error4x4(1, 2));
    EXPECT_EQ(0, error4x4(1, 3));
    EXPECT_EQ(0, error4x4(2, 0));
    EXPECT_EQ(0, error4x4(2, 1));
    EXPECT_EQ(0, error4x4(2, 2));
    EXPECT_EQ(0, error4x4(2, 3));
    EXPECT_EQ(0, error4x4(3, 0));
    EXPECT_EQ(0, error4x4(3, 1));
    EXPECT_EQ(0, error4x4(3, 2));
    EXPECT_EQ(0, error4x4(3, 3));

    const auto error7x7 = myECLCluster.getError7x7();
    EXPECT_EQ(0, error7x7(0, 0));
    EXPECT_EQ(0, error7x7(0, 1));
    EXPECT_EQ(0, error7x7(0, 2));
    EXPECT_EQ(0, error7x7(0, 3));
    EXPECT_EQ(0, error7x7(0, 4));
    EXPECT_EQ(0, error7x7(0, 5));
    EXPECT_EQ(0, error7x7(0, 6));
    EXPECT_EQ(0, error7x7(1, 0));
    EXPECT_EQ(0, error7x7(1, 1));
    EXPECT_EQ(0, error7x7(1, 2));
    EXPECT_EQ(0, error7x7(1, 3));
    EXPECT_EQ(0, error7x7(1, 4));
    EXPECT_EQ(0, error7x7(1, 5));
    EXPECT_EQ(0, error7x7(1, 6));
    EXPECT_EQ(0, error7x7(2, 0));
    EXPECT_EQ(0, error7x7(2, 1));
    EXPECT_EQ(0, error7x7(2, 2));
    EXPECT_EQ(0, error7x7(2, 3));
    EXPECT_EQ(0, error7x7(2, 4));
    EXPECT_EQ(0, error7x7(2, 5));
    EXPECT_EQ(0, error7x7(2, 6));
    EXPECT_EQ(0, error7x7(3, 0));
    EXPECT_EQ(0, error7x7(3, 1));
    EXPECT_EQ(0, error7x7(3, 2));
    EXPECT_EQ(0, error7x7(3, 3));
    EXPECT_EQ(0, error7x7(3, 4));
    EXPECT_EQ(0, error7x7(3, 5));
    EXPECT_EQ(0, error7x7(3, 6));
    EXPECT_EQ(0, error7x7(4, 0));
    EXPECT_EQ(0, error7x7(4, 1));
    EXPECT_EQ(0, error7x7(4, 2));
    EXPECT_EQ(0, error7x7(4, 3));
    EXPECT_EQ(1, error7x7(4, 4));
    EXPECT_EQ(0, error7x7(4, 5));
    EXPECT_EQ(0, error7x7(4, 6));
    EXPECT_EQ(0, error7x7(5, 0));
    EXPECT_EQ(0, error7x7(5, 1));
    EXPECT_EQ(0, error7x7(5, 2));
    EXPECT_EQ(0, error7x7(5, 3));
    EXPECT_EQ(0, error7x7(5, 4));
    EXPECT_EQ(1, error7x7(5, 5));
    EXPECT_EQ(0, error7x7(5, 6));
    EXPECT_EQ(0, error7x7(6, 0));
    EXPECT_EQ(0, error7x7(6, 1));
    EXPECT_EQ(0, error7x7(6, 2));
    EXPECT_EQ(0, error7x7(6, 3));
    EXPECT_EQ(0, error7x7(6, 4));
    EXPECT_EQ(0, error7x7(6, 5));
    EXPECT_EQ(1, error7x7(6, 6));

  }
}  // namespace
