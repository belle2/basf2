/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <mdst/dataobjects/ECLCluster.h>
#include <gtest/gtest.h>
#include <cmath>
#include <TMatrixD.h>

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

    EXPECT_EQ(exp(-5.), myECLCluster.getEnergy(ECLCluster::EHypothesisBit::c_nPhotons));
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

    EXPECT_FALSE(myECLCluster.isTrack());
    EXPECT_TRUE(myECLCluster.isNeutral());

    double x = myECLCluster.getClusterPosition().X();
    double y = myECLCluster.getClusterPosition().Y();
    double z = myECLCluster.getClusterPosition().Z();
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
    const ECLCluster::EStatusBit status = ECLCluster::EStatusBit::c_PulseShapeDiscrimination;
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
    myECLCluster.setIsTrack(isTrack);
    myECLCluster.setHypothesis(ECLCluster::EHypothesisBit::c_nPhotons);

    EXPECT_FLOAT_EQ(energy, myECLCluster.getEnergy(ECLCluster::EHypothesisBit::c_nPhotons));
    EXPECT_FLOAT_EQ(E9oE21, myECLCluster.getE9oE21());
    EXPECT_FLOAT_EQ(energyDepSum, myECLCluster.getEnergyRaw());
    EXPECT_FLOAT_EQ(theta, myECLCluster.getTheta());
    EXPECT_FLOAT_EQ(phi, myECLCluster.getPhi());
    EXPECT_FLOAT_EQ(r, myECLCluster.getR());
    EXPECT_FLOAT_EQ(time, myECLCluster.getTime());
    EXPECT_FLOAT_EQ(deltaTime99, myECLCluster.getDeltaTime99());
    EXPECT_FLOAT_EQ(highestEnergy, myECLCluster.getEnergyHighestCrystal());
    EXPECT_TRUE(myECLCluster.hasStatus(status));
    EXPECT_FLOAT_EQ(nOfCrystals, myECLCluster.getNumberOfCrystals());
    EXPECT_FLOAT_EQ(lat, myECLCluster.getLAT());

    EXPECT_FLOAT_EQ(error[0], myECLCluster.getUncertaintyEnergy()*myECLCluster.getUncertaintyEnergy());
    EXPECT_FLOAT_EQ(error[5], myECLCluster.getUncertaintyTheta()*myECLCluster.getUncertaintyTheta());
    EXPECT_FLOAT_EQ(error[2], myECLCluster.getUncertaintyPhi()*myECLCluster.getUncertaintyPhi());

    EXPECT_TRUE(myECLCluster.isTrack());
    EXPECT_FALSE(myECLCluster.isNeutral());

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

  } // default constructor


}  // namespace
