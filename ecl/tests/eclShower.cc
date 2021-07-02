/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <ecl/dataobjects/ECLShower.h>

#include <gtest/gtest.h>

using namespace std;

namespace Belle2 {

  /** Set up a few arrays and objects in the datastore */
  class ECLShowerTest : public ::testing::Test {
  };

  /** Test Constructors. */
  TEST_F(ECLShowerTest, Constructors)
  {
    ECLShower myECLShower;
    EXPECT_EQ(myECLShower.getIsTrack(), 0);

    EXPECT_EQ(myECLShower.getStatus(), 0);
    EXPECT_EQ(myECLShower.getShowerId(), 0);
    EXPECT_EQ(myECLShower.getConnectedRegionId(), 0);
    EXPECT_EQ(myECLShower.getHypothesisId(), 0);
    EXPECT_EQ(myECLShower.getEnergy(), 0);
    EXPECT_EQ(myECLShower.getTheta(), 0);
    EXPECT_EQ(myECLShower.getPhi(), 0);
    EXPECT_EQ(myECLShower.getR(), 0);
    EXPECT_EQ(myECLShower.getUncertaintyEnergy(), 0);
    EXPECT_EQ(myECLShower.getUncertaintyTheta(), 0);
    EXPECT_EQ(myECLShower.getUncertaintyPhi(), 0);
    EXPECT_EQ(myECLShower.getTime(), 0);
    EXPECT_EQ(myECLShower.getDeltaTime99(), 0);
    EXPECT_EQ(myECLShower.getEnergyHighestCrystal(), 0);
    EXPECT_EQ(myECLShower.getLateralEnergy(), 0);
    EXPECT_EQ(myECLShower.getMinTrkDistance(), 0);
    EXPECT_EQ(myECLShower.getTrkDepth(), 0);
    EXPECT_EQ(myECLShower.getShowerDepth(), 0);
    EXPECT_EQ(myECLShower.getNumberOfCrystals(), 0);
    EXPECT_EQ(myECLShower.getAbsZernike40(), 0);
    EXPECT_EQ(myECLShower.getAbsZernike51(), 0);
    EXPECT_EQ(myECLShower.getZernikeMVA(), 0);
    EXPECT_EQ(myECLShower.getSecondMoment(), 0);
    EXPECT_EQ(myECLShower.getE1oE9(), 0);
    EXPECT_EQ(myECLShower.getE9oE21(), 0);

    double errorArray[6];
    myECLShower.getCovarianceMatrixAsArray(errorArray);
    EXPECT_EQ(errorArray[0], 0);
    EXPECT_EQ(errorArray[1], 0);
    EXPECT_EQ(errorArray[2], 0);
    EXPECT_EQ(errorArray[3], 0);
    EXPECT_EQ(errorArray[4], 0);
    EXPECT_EQ(errorArray[5], 0);

    TVector3 momentum(myECLShower.getMomentum());
    EXPECT_EQ(momentum.X(), 0);
    EXPECT_EQ(momentum.Y(), 0);
    EXPECT_EQ(momentum.Z(), 0);
  } // Testcases for Something

  /** Test Setters and Getter. */
  TEST_F(ECLShowerTest, SettersAndGetters)
  {
    const bool isTrk = true;

    const int status = 5;
    const int showerId = 6;
    const int connectedRegionId = 7;
    const int hypothesisId = 8;

    const double energy = 1.1;
    const double theta = 1.2;
    const double phi = 1.3;
    const double r = 1.4;
    double error[6] = {2.1, 2.2, 2.3, 2.4, 2.5, 2.6};
    const double time = 1.5;
    const double timeResolution = 1.6;
    const double highestEnergy = 1.7;
    const double lateralEnergy = 1.8;
    const double minTrkDistance = 1.9;
    const double trkDepth = 4.1;
    const double showerDepth = 3.1;
    const double NofCrystals = 3.2;
    const double absZernike40 = 1.1;
    const double absZernike51 = 0.1;
    const double zernikeMVA = 0.5;
    const double secondMoment = 5.1;
    const double E1oE9 = 3.8;
    const double E9oE21 = 3.9;

    ECLShower myECLShower;
    myECLShower.setIsTrack(isTrk);
    myECLShower.setStatus(status);
    myECLShower.setShowerId(showerId);
    myECLShower.setConnectedRegionId(connectedRegionId);
    myECLShower.setHypothesisId(hypothesisId);
    myECLShower.setEnergy(energy);
    myECLShower.setTheta(theta);
    myECLShower.setPhi(phi);
    myECLShower.setR(r);
    myECLShower.setCovarianceMatrix(error);
    myECLShower.setTime(time);
    myECLShower.setDeltaTime99(timeResolution);
    myECLShower.setEnergyHighestCrystal(highestEnergy);
    myECLShower.setLateralEnergy(lateralEnergy);
    myECLShower.setMinTrkDistance(minTrkDistance);
    myECLShower.setTrkDepth(trkDepth);
    myECLShower.setShowerDepth(showerDepth);
    myECLShower.setNumberOfCrystals(NofCrystals);
    myECLShower.setAbsZernike40(absZernike40);
    myECLShower.setAbsZernike51(absZernike51);
    myECLShower.setZernikeMVA(zernikeMVA);
    myECLShower.setSecondMoment(secondMoment);
    myECLShower.setE1oE9(E1oE9);
    myECLShower.setE9oE21(E9oE21);

    EXPECT_EQ(myECLShower.getIsTrack(), isTrk);
    EXPECT_EQ(myECLShower.getStatus(), status);
    EXPECT_EQ(myECLShower.getShowerId(), showerId);
    EXPECT_EQ(myECLShower.getConnectedRegionId(), connectedRegionId);
    EXPECT_EQ(myECLShower.getHypothesisId(), hypothesisId);
    EXPECT_EQ(myECLShower.getEnergy(), energy);
    EXPECT_EQ(myECLShower.getTheta(), theta);
    EXPECT_EQ(myECLShower.getPhi(), phi);
    EXPECT_EQ(myECLShower.getUncertaintyEnergy(), sqrt(error[0]));
    EXPECT_EQ(myECLShower.getUncertaintyTheta(), sqrt(error[5]));
    EXPECT_EQ(myECLShower.getUncertaintyPhi(), sqrt(error[2]));
    EXPECT_EQ(myECLShower.getTime(), time);
    EXPECT_EQ(myECLShower.getDeltaTime99(), timeResolution);
    EXPECT_EQ(myECLShower.getEnergyHighestCrystal(), highestEnergy);
    EXPECT_EQ(myECLShower.getLateralEnergy(), lateralEnergy);
    EXPECT_EQ(myECLShower.getMinTrkDistance(), minTrkDistance);
    EXPECT_EQ(myECLShower.getTrkDepth(), trkDepth);
    EXPECT_EQ(myECLShower.getShowerDepth(), showerDepth);
    EXPECT_EQ(myECLShower.getMinTrkDistance(), minTrkDistance);
    EXPECT_EQ(myECLShower.getTrkDepth(), trkDepth);
    EXPECT_EQ(myECLShower.getNumberOfCrystals(), NofCrystals);
    EXPECT_EQ(myECLShower.getAbsZernike40(), absZernike40);
    EXPECT_EQ(myECLShower.getAbsZernike51(), absZernike51);
    EXPECT_EQ(myECLShower.getZernikeMVA(), zernikeMVA);
    EXPECT_EQ(myECLShower.getSecondMoment(), secondMoment);
    EXPECT_EQ(myECLShower.getE1oE9(), E1oE9);
    EXPECT_EQ(myECLShower.getE9oE21(), E9oE21);


    double errorArray[6];
    myECLShower.getCovarianceMatrixAsArray(errorArray);
    EXPECT_EQ(errorArray[0], error[0]);
    EXPECT_EQ(errorArray[1], error[1]);
    EXPECT_EQ(errorArray[2], error[2]);
    EXPECT_EQ(errorArray[3], error[3]);
    EXPECT_EQ(errorArray[4], error[4]);
    EXPECT_EQ(errorArray[5], error[5]);

    TVector3 momentum(myECLShower.getMomentum());
    EXPECT_FLOAT_EQ(momentum.X(), energy * sin(theta) * cos(phi));
    EXPECT_FLOAT_EQ(momentum.Y(), energy * sin(theta) * sin(phi));
    EXPECT_FLOAT_EQ(momentum.Z(), energy * cos(theta));
  } // Testcases for Setters and Getters

}  // namespace
