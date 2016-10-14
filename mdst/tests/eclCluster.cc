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

    EXPECT_EQ(exp(-5.), myECLCluster.getEnergy());
    EXPECT_EQ(exp(-5.), myECLCluster.getEnergyRaw());
    EXPECT_EQ(exp(-5.), myECLCluster.getEnergyHighestCrystal());
    EXPECT_EQ(0, myECLCluster.getTheta());
    EXPECT_EQ(0, myECLCluster.getPhi());
    EXPECT_EQ(0, myECLCluster.getR());
    EXPECT_EQ(0, myECLCluster.getTime());
    EXPECT_EQ(0, myECLCluster.getDeltaTime99());
    EXPECT_EQ(0, myECLCluster.getStatus());
    EXPECT_EQ(0, myECLCluster.getNumberOfCrystals());
    EXPECT_EQ(0, myECLCluster.getLAT());
    EXPECT_EQ(0, myECLCluster.getAbsZernike40());
    EXPECT_EQ(0, myECLCluster.getAbsZernike51());
    EXPECT_EQ(0, myECLCluster.getZernikeMVA());
    EXPECT_EQ(0, myECLCluster.getE1oE9());
    EXPECT_EQ(0, myECLCluster.getE9oE21());

    EXPECT_EQ(0, myECLCluster.getUncertaintyEnergy());
    EXPECT_EQ(0, myECLCluster.getUncertaintyTheta());
    EXPECT_EQ(0, myECLCluster.getUncertaintyPhi());

    EXPECT_EQ(0, myECLCluster.getPx());
    EXPECT_EQ(0, myECLCluster.getPy());
    EXPECT_EQ(exp(-5.), myECLCluster.getPz());

    EXPECT_FALSE(myECLCluster.getisTrack());
    EXPECT_TRUE(myECLCluster.isNeutral());

    double px = myECLCluster.getMomentum().X();
    double py = myECLCluster.getMomentum().Y();
    double pz = myECLCluster.getMomentum().Z();
    EXPECT_EQ(0, px);
    EXPECT_EQ(0, py);
    EXPECT_EQ(exp(-5.), pz);

    px = myECLCluster.get4Vector().X();
    py = myECLCluster.get4Vector().Y();
    pz = myECLCluster.get4Vector().Z();
    double energy = myECLCluster.get4Vector().E();
    EXPECT_EQ(0, px);
    EXPECT_EQ(0, py);
    EXPECT_EQ(exp(-5.), pz);
    EXPECT_EQ(exp(-5.), energy);

    double x = myECLCluster.getClusterPosition().X();
    double y = myECLCluster.getClusterPosition().Y();
    double z = myECLCluster.getClusterPosition().Z();
    EXPECT_EQ(0, x);
    EXPECT_EQ(0, y);
    EXPECT_EQ(0, z);

    x = myECLCluster.getPosition().X();
    y = myECLCluster.getPosition().Y();
    z = myECLCluster.getPosition().Z();
    EXPECT_EQ(0, x);
    EXPECT_EQ(0, y);
    EXPECT_EQ(0, z);

    const auto error3x3 = myECLCluster.getCovarianceMatrix3x3();
    EXPECT_EQ(0, error3x3(0, 0));
    EXPECT_EQ(0, error3x3(0, 1));
    EXPECT_EQ(0, error3x3(0, 2));
    EXPECT_EQ(0, error3x3(1, 0));
    EXPECT_EQ(0, error3x3(1, 1));
    EXPECT_EQ(0, error3x3(1, 2));
    EXPECT_EQ(0, error3x3(2, 0));
    EXPECT_EQ(0, error3x3(2, 1));
    EXPECT_EQ(0, error3x3(2, 2));

    const auto error4x4 = myECLCluster.getCovarianceMatrix4x4();
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

    const auto error7x7 = myECLCluster.getCovarianceMatrix7x7();
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
    const float theta = 1.2;
    const float phi = 1.2;
    const double r = 1.2;
    const double energyDepSum = 12.3;
    const double time = 17.2;
    const double deltaTime99 = 1.1;
    const double E9oE21 = 0.1;
    const double highestEnergy = 32.1;
    const double lat = 0.5;
    const double   nOfCrystals = 4;
    const int   status = 1;
    // Energy->[0], Phi->[2], Theta->[5]
    double error[6] = {0.1, 0.2, 0.3, 0.4, 0.5, 0.6};

    myECLCluster.setEnergy(energy);
    myECLCluster.setE9oE21(E9oE21);
    myECLCluster.setEnergyRaw(energyDepSum);
    myECLCluster.setTheta(theta);
    myECLCluster.setPhi(phi);
    myECLCluster.setR(r);
    myECLCluster.setTime(time);
    myECLCluster.setDeltaTime99(deltaTime99);
    myECLCluster.setEnergyHighestCrystal(highestEnergy);
    myECLCluster.setStatus(status);
    myECLCluster.setNumberOfCrystals(nOfCrystals);
    myECLCluster.setLAT(lat);
    myECLCluster.setCovarianceMatrix(error);
    myECLCluster.setisTrack(isTrack);

    EXPECT_FLOAT_EQ(energy, myECLCluster.getEnergy());
    EXPECT_FLOAT_EQ(E9oE21, myECLCluster.getE9oE21());
    EXPECT_FLOAT_EQ(energyDepSum, myECLCluster.getEnergyRaw());
    EXPECT_FLOAT_EQ(theta, myECLCluster.getTheta());
    EXPECT_FLOAT_EQ(phi, myECLCluster.getPhi());
    EXPECT_FLOAT_EQ(r, myECLCluster.getR());
    EXPECT_FLOAT_EQ(time, myECLCluster.getTime());
    EXPECT_FLOAT_EQ(deltaTime99, myECLCluster.getDeltaTime99());
    EXPECT_FLOAT_EQ(highestEnergy, myECLCluster.getEnergyHighestCrystal());
    EXPECT_EQ(status, myECLCluster.getStatus());
    EXPECT_FLOAT_EQ(nOfCrystals, myECLCluster.getNumberOfCrystals());
    EXPECT_FLOAT_EQ(lat, myECLCluster.getLAT());

    EXPECT_FLOAT_EQ(error[0], myECLCluster.getUncertaintyEnergy()*myECLCluster.getUncertaintyEnergy());
    EXPECT_FLOAT_EQ(error[5], myECLCluster.getUncertaintyTheta()*myECLCluster.getUncertaintyTheta());
    EXPECT_FLOAT_EQ(error[2], myECLCluster.getUncertaintyPhi()*myECLCluster.getUncertaintyPhi());

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

    x = myECLCluster.getClusterPosition().X();
    y = myECLCluster.getClusterPosition().Y();
    z = myECLCluster.getClusterPosition().Z();
    EXPECT_FLOAT_EQ(r * sin(theta) * cos(phi), x);
    EXPECT_FLOAT_EQ(r * sin(theta) * sin(phi), y);
    EXPECT_FLOAT_EQ(r * cos(theta), z);

    x = myECLCluster.getPosition().X();
    y = myECLCluster.getPosition().Y();
    z = myECLCluster.getPosition().Z();
    EXPECT_FLOAT_EQ(0, x);
    EXPECT_FLOAT_EQ(0, y);
    EXPECT_FLOAT_EQ(0, z);

    const auto error3x3 = myECLCluster.getCovarianceMatrix3x3();
    EXPECT_FLOAT_EQ(error[0], error3x3(0, 0));
    EXPECT_FLOAT_EQ(error[1], error3x3(0, 1));
    EXPECT_FLOAT_EQ(error[3], error3x3(0, 2));
    EXPECT_FLOAT_EQ(error[1], error3x3(1, 0));
    EXPECT_FLOAT_EQ(error[2], error3x3(1, 1));
    EXPECT_FLOAT_EQ(error[4], error3x3(1, 2));
    EXPECT_FLOAT_EQ(error[3], error3x3(2, 0));
    EXPECT_FLOAT_EQ(error[4], error3x3(2, 1));
    EXPECT_FLOAT_EQ(error[5], error3x3(2, 2));

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

    const auto error4x4 = myECLCluster.getCovarianceMatrix4x4();
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

    const auto error7x7 = myECLCluster.getCovarianceMatrix7x7();
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
