#include <mdst/dataobjects/ECLCluster.h>
#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>
#include <framework/datastore/StoreArray.h>
#include <gtest/gtest.h>
#include <cmath>

using namespace std;

namespace Belle2 {


  /** Test class for the Track object.
   */
  class ECLClusterTest : public ::testing::Test {
  protected:
  };

  /** Test default constructor. */
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
  } // default constructor

  /** Test setter and getter. */
  TEST_F(ECLClusterTest, SettersAndGetters)
  {
    ECLCluster myECLCluster;
    // Pick some arbitrary numbers to test with
    const bool isTrack = true;
    const double energy = 1.165;
    const double theta = 1.2;
    const double phi = 1.2;
    const double r = 1.2;
    const double energyDepSum = 123.3;
    const double timing = 17.2;
    const double errorTiming = 1.1;
    const double E9oE25 = 12.2;
    const double highestE = 32.1;
    const double lat = 21.5;
    const int   nOfCrystals = 4;
    const int   crystHealth = 1;
    const double mergedPi0 = 12.2;
    // Energy->[0], Phi->[2], Theta->[5]
    double error[6] = {1.1, 1.2, 1.3, 1.4, 1.5, 1.6};

    myECLCluster.setEnergy(energy);
    myECLCluster.setE9oE25(E9oE25);
    myECLCluster.setEnedepSum(energyDepSum);
    myECLCluster.setTheta(theta);
    myECLCluster.setPhi(phi);
    myECLCluster.setR(r);
    myECLCluster.setTiming(timing);
    myECLCluster.setErrorTiming(errorTiming);
    myECLCluster.setHighestE(highestE);
    myECLCluster.setCrystHealth(crystHealth);
    myECLCluster.setNofCrystals(nOfCrystals);
    myECLCluster.setLAT(lat);
    myECLCluster.setMergedPi0(mergedPi0);
    myECLCluster.setError(error);
    myECLCluster.setisTrack(isTrack);

    EXPECT_FLOAT_EQ(energy, myECLCluster.getEnergy());
    EXPECT_FLOAT_EQ(E9oE25, myECLCluster.getE9oE25());
    EXPECT_FLOAT_EQ(energyDepSum, myECLCluster.getEnedepSum());
    EXPECT_FLOAT_EQ(theta, myECLCluster.getTheta());
    EXPECT_FLOAT_EQ(phi, myECLCluster.getPhi());
    EXPECT_FLOAT_EQ(r, myECLCluster.getR());
    EXPECT_FLOAT_EQ(timing, myECLCluster.getTiming());
    EXPECT_FLOAT_EQ(errorTiming, myECLCluster.getErrorTiming());
    EXPECT_FLOAT_EQ(highestE, myECLCluster.getHighestE());
    EXPECT_EQ(crystHealth, myECLCluster.getCrystHealth());
    EXPECT_EQ(nOfCrystals, myECLCluster.getNofCrystals());
    EXPECT_FLOAT_EQ(lat, myECLCluster.getLAT());
    EXPECT_FLOAT_EQ(mergedPi0, myECLCluster.getMergedPi0());

    EXPECT_FLOAT_EQ(error[0], myECLCluster.getErrorEnergy());
    EXPECT_FLOAT_EQ(error[5], myECLCluster.getErrorTheta());
    EXPECT_FLOAT_EQ(error[2], myECLCluster.getErrorPhi());

    EXPECT_FLOAT_EQ(energy * sin(theta) * cos(phi), myECLCluster.getPx());
    EXPECT_FLOAT_EQ(energy * sin(theta) * sin(phi), myECLCluster.getPy());
    EXPECT_FLOAT_EQ(energy * cos(theta), myECLCluster.getPz());

    EXPECT_TRUE(myECLCluster.getisTrack());
    EXPECT_FALSE(myECLCluster.isNeutral());

    double x = myECLCluster.getMomentum().X();
    double y = myECLCluster.getMomentum().Y();
    double z = myECLCluster.getMomentum().Z();
    EXPECT_FLOAT_EQ(energy * sin(theta) * cos(phi), x);
    EXPECT_FLOAT_EQ(energy * sin(theta) * sin(phi), y);
    EXPECT_FLOAT_EQ(energy * cos(theta), z);

    x = myECLCluster.get4Vector().X();
    y = myECLCluster.get4Vector().Y();
    z = myECLCluster.get4Vector().Z();
    double e = myECLCluster.get4Vector().E();
    EXPECT_FLOAT_EQ(energy * sin(theta) * cos(phi), x);
    EXPECT_FLOAT_EQ(energy * sin(theta) * sin(phi), y);
    EXPECT_FLOAT_EQ(energy * cos(theta), z);
    EXPECT_FLOAT_EQ(energy, e);

    x = myECLCluster.getclusterPosition().X();
    y = myECLCluster.getclusterPosition().Y();
    z = myECLCluster.getclusterPosition().Z();
    EXPECT_FLOAT_EQ(r * sin(theta) * cos(phi), x);
    EXPECT_FLOAT_EQ(r * sin(theta) * sin(phi), y);
    EXPECT_FLOAT_EQ(r * cos(theta), z);

    x = myECLCluster.getPosition().X();
    y = myECLCluster.getPosition().Y();
    z = myECLCluster.getPosition().Z();
    EXPECT_FLOAT_EQ(0, x);
    EXPECT_FLOAT_EQ(0, y);
    EXPECT_FLOAT_EQ(0, z);

    const auto error3x3 = myECLCluster.getError3x3();
    EXPECT_FLOAT_EQ(error[0] * error[0], error3x3(0, 0));
    EXPECT_FLOAT_EQ(error[1], error3x3(0, 1));
    EXPECT_FLOAT_EQ(error[3], error3x3(0, 2));
    EXPECT_FLOAT_EQ(error[1], error3x3(1, 0));
    EXPECT_FLOAT_EQ(error[2] * error[2], error3x3(1, 1));
    EXPECT_FLOAT_EQ(error[4], error3x3(1, 2));
    EXPECT_FLOAT_EQ(error[3], error3x3(2, 0));
    EXPECT_FLOAT_EQ(error[4], error3x3(2, 1));
    EXPECT_FLOAT_EQ(error[5] * error[5], error3x3(2, 2));

    TMatrixDSym errorecl = error3x3;
    TMatrixD  jacobian(4, 3);
    const double cosPhi = cos(phi);
    const double sinPhi = sin(phi);
    const double cosTheta = cos(theta);
    const double sinTheta = sin(theta);
    jacobian(0, 0) = cosPhi * sinTheta;
    jacobian(0, 1) = -1.0 * energy * sinPhi * sinTheta;
    jacobian(0, 2) = energy * cosPhi * cosTheta;
    jacobian(1, 0) = sinPhi * sinTheta;
    jacobian(1, 1) = energy * cosPhi * sinTheta;
    jacobian(1, 2) = energy * sinPhi * cosTheta;
    jacobian(2, 0) = cosTheta;
    jacobian(2, 1) = 0.0;
    jacobian(2, 2) = -1.0 * energy * sinTheta;
    jacobian(3, 0) = 1.0;
    jacobian(3, 1) = 0.0;
    jacobian(3, 2) = 0.0;
    TMatrixDSym error4x4expected(4);
    error4x4expected = errorecl.Similarity(jacobian);

    const auto error4x4 = myECLCluster.getError4x4();
    EXPECT_FLOAT_EQ(error4x4expected(0, 0), error4x4(0, 0));
    EXPECT_FLOAT_EQ(error4x4expected(0, 1), error4x4(0, 1));
    EXPECT_FLOAT_EQ(error4x4expected(0, 2), error4x4(0, 2));
    EXPECT_FLOAT_EQ(error4x4expected(0, 3), error4x4(0, 3));
    EXPECT_FLOAT_EQ(error4x4expected(1, 0), error4x4(1, 0));
    EXPECT_FLOAT_EQ(error4x4expected(1, 1), error4x4(1, 1));
    EXPECT_FLOAT_EQ(error4x4expected(1, 2), error4x4(1, 2));
    EXPECT_FLOAT_EQ(error4x4expected(1, 3), error4x4(1, 3));
    EXPECT_FLOAT_EQ(error4x4expected(2, 0), error4x4(2, 0));
    EXPECT_FLOAT_EQ(error4x4expected(2, 1), error4x4(2, 1));
    EXPECT_FLOAT_EQ(error4x4expected(2, 2), error4x4(2, 2));
    EXPECT_FLOAT_EQ(error4x4expected(2, 3), error4x4(2, 3));
    EXPECT_FLOAT_EQ(error4x4expected(3, 0), error4x4(3, 0));
    EXPECT_FLOAT_EQ(error4x4expected(3, 1), error4x4(3, 1));
    EXPECT_FLOAT_EQ(error4x4expected(3, 2), error4x4(3, 2));
    EXPECT_FLOAT_EQ(error4x4expected(3, 3), error4x4(3, 3));

    const auto error7x7 = myECLCluster.getError7x7();
    EXPECT_FLOAT_EQ(error4x4expected(0, 0), error7x7(0, 0));
    EXPECT_FLOAT_EQ(error4x4expected(0, 1), error7x7(0, 1));
    EXPECT_FLOAT_EQ(error4x4expected(0, 2), error7x7(0, 2));
    EXPECT_FLOAT_EQ(error4x4expected(0, 3), error7x7(0, 3));
    EXPECT_EQ(0, error7x7(0, 4));
    EXPECT_EQ(0, error7x7(0, 5));
    EXPECT_EQ(0, error7x7(0, 6));
    EXPECT_FLOAT_EQ(error4x4expected(1, 0), error7x7(1, 0));
    EXPECT_FLOAT_EQ(error4x4expected(1, 1), error7x7(1, 1));
    EXPECT_FLOAT_EQ(error4x4expected(1, 2), error7x7(1, 2));
    EXPECT_FLOAT_EQ(error4x4expected(1, 3), error7x7(1, 3));
    EXPECT_EQ(0, error7x7(1, 4));
    EXPECT_EQ(0, error7x7(1, 5));
    EXPECT_EQ(0, error7x7(1, 6));
    EXPECT_FLOAT_EQ(error4x4expected(2, 0), error7x7(2, 0));
    EXPECT_FLOAT_EQ(error4x4expected(2, 1), error7x7(2, 1));
    EXPECT_FLOAT_EQ(error4x4expected(2, 2), error7x7(2, 2));
    EXPECT_FLOAT_EQ(error4x4expected(2, 3), error7x7(2, 3));
    EXPECT_EQ(0, error7x7(2, 4));
    EXPECT_EQ(0, error7x7(2, 5));
    EXPECT_EQ(0, error7x7(2, 6));
    EXPECT_FLOAT_EQ(error4x4expected(3, 0), error7x7(3, 0));
    EXPECT_FLOAT_EQ(error4x4expected(3, 1), error7x7(3, 1));
    EXPECT_FLOAT_EQ(error4x4expected(3, 2), error7x7(3, 2));
    EXPECT_FLOAT_EQ(error4x4expected(3, 3), error7x7(3, 3));
    EXPECT_EQ(0, error7x7(3, 4));
    EXPECT_EQ(0, error7x7(3, 5));
    EXPECT_EQ(0, error7x7(3, 6));
    EXPECT_FLOAT_EQ(0, error7x7(4, 0));
    EXPECT_FLOAT_EQ(0, error7x7(4, 1));
    EXPECT_FLOAT_EQ(0, error7x7(4, 2));
    EXPECT_FLOAT_EQ(0, error7x7(4, 3));
    EXPECT_FLOAT_EQ(1, error7x7(4, 4));
    EXPECT_FLOAT_EQ(0, error7x7(4, 5));
    EXPECT_FLOAT_EQ(0, error7x7(4, 6));
    EXPECT_FLOAT_EQ(0, error7x7(5, 0));
    EXPECT_FLOAT_EQ(0, error7x7(5, 1));
    EXPECT_FLOAT_EQ(0, error7x7(5, 2));
    EXPECT_FLOAT_EQ(0, error7x7(5, 3));
    EXPECT_FLOAT_EQ(0, error7x7(5, 4));
    EXPECT_FLOAT_EQ(1, error7x7(5, 5));
    EXPECT_FLOAT_EQ(0, error7x7(5, 6));
    EXPECT_FLOAT_EQ(0, error7x7(6, 0));
    EXPECT_FLOAT_EQ(0, error7x7(6, 1));
    EXPECT_FLOAT_EQ(0, error7x7(6, 2));
    EXPECT_FLOAT_EQ(0, error7x7(6, 3));
    EXPECT_FLOAT_EQ(0, error7x7(6, 4));
    EXPECT_FLOAT_EQ(0, error7x7(6, 5));
    EXPECT_FLOAT_EQ(1, error7x7(6, 6));
  } // default constructor


}  // namespace
