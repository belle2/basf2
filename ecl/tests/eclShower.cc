#include <ecl/dataobjects/ECLShower.h>
#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>

#include <utility>

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
    EXPECT_EQ(myECLShower.getEnergyError(), 0);
    EXPECT_EQ(myECLShower.getThetaError(), 0);
    EXPECT_EQ(myECLShower.getPhiError(), 0);
    EXPECT_EQ(myECLShower.getTime(), 0);
    EXPECT_EQ(myECLShower.getTimeResolution(), 0);
    EXPECT_EQ(myECLShower.getHighestEnergy(), 0);
    EXPECT_EQ(myECLShower.getLateralEnergy(), 0);
    EXPECT_EQ(myECLShower.getMinTrkDistance(), 0);
    EXPECT_EQ(myECLShower.getTrkDepth(), 0);
    EXPECT_EQ(myECLShower.getShowerDepth(), 0);
    EXPECT_EQ(myECLShower.getNofCrystals(), 0);
    EXPECT_EQ(myECLShower.getZernike20(), 0);
    EXPECT_EQ(myECLShower.getZernike40(), 0);
    EXPECT_EQ(myECLShower.getZernike42(), 0);
    EXPECT_EQ(myECLShower.getZernike51(), 0);
    EXPECT_EQ(myECLShower.getZernike53(), 0);
    EXPECT_EQ(myECLShower.getSecondMoment(), 0);
    EXPECT_EQ(myECLShower.getE1oE9(), 0);
    EXPECT_EQ(myECLShower.getE9oE25(), 0);

    double errorArray[6];
    myECLShower.getError(errorArray);
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
    const double zernike20 = 3.3;
    const double zernike40 = 3.4;
    const double zernike42 = 3.5;
    const double zernike51 = 3.6;
    const double zernike53 = 3.7;
    const double secondMoment = 5.1;
    const double E1oE9 = 3.8;
    const double E9oE25 = 3.9;

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
    myECLShower.setError(error);
    myECLShower.setTime(time);
    myECLShower.setTimeResolution(timeResolution);
    myECLShower.setHighestEnergy(highestEnergy);
    myECLShower.setLateralEnergy(lateralEnergy);
    myECLShower.setMinTrkDistance(minTrkDistance);
    myECLShower.setTrkDepth(trkDepth);
    myECLShower.setShowerDepth(showerDepth);
    myECLShower.setNofCrystals(NofCrystals);
    myECLShower.setZernike20(zernike20);
    myECLShower.setZernike40(zernike40);
    myECLShower.setZernike42(zernike42);
    myECLShower.setZernike51(zernike51);
    myECLShower.setZernike53(zernike53);
    myECLShower.setSecondMoment(secondMoment);
    myECLShower.setE1oE9(E1oE9);
    myECLShower.setE9oE25(E9oE25);

    EXPECT_EQ(myECLShower.getIsTrack(), isTrk);
    EXPECT_EQ(myECLShower.getStatus(), status);
    EXPECT_EQ(myECLShower.getShowerId(), showerId);
    EXPECT_EQ(myECLShower.getConnectedRegionId(), connectedRegionId);
    EXPECT_EQ(myECLShower.getHypothesisId(), hypothesisId);
    EXPECT_EQ(myECLShower.getEnergy(), energy);
    EXPECT_EQ(myECLShower.getTheta(), theta);
    EXPECT_EQ(myECLShower.getPhi(), phi);
    EXPECT_EQ(myECLShower.getEnergyError(), error[0]);
    EXPECT_EQ(myECLShower.getThetaError(), error[5]);
    EXPECT_EQ(myECLShower.getPhiError(), error[2]);
    EXPECT_EQ(myECLShower.getTime(), time);
    EXPECT_EQ(myECLShower.getTimeResolution(), timeResolution);
    EXPECT_EQ(myECLShower.getHighestEnergy(), highestEnergy);
    EXPECT_EQ(myECLShower.getLateralEnergy(), lateralEnergy);
    EXPECT_EQ(myECLShower.getMinTrkDistance(), minTrkDistance);
    EXPECT_EQ(myECLShower.getTrkDepth(), trkDepth);
    EXPECT_EQ(myECLShower.getShowerDepth(), showerDepth);
    EXPECT_EQ(myECLShower.getMinTrkDistance(), minTrkDistance);
    EXPECT_EQ(myECLShower.getTrkDepth(), trkDepth);
    EXPECT_EQ(myECLShower.getNofCrystals(), NofCrystals);
    EXPECT_EQ(myECLShower.getZernike20(), zernike20);
    EXPECT_EQ(myECLShower.getZernike40(), zernike40);
    EXPECT_EQ(myECLShower.getZernike42(), zernike42);
    EXPECT_EQ(myECLShower.getZernike51(), zernike51);
    EXPECT_EQ(myECLShower.getZernike53(), zernike53);
    EXPECT_EQ(myECLShower.getSecondMoment(), secondMoment);
    EXPECT_EQ(myECLShower.getE1oE9(), E1oE9);
    EXPECT_EQ(myECLShower.getE9oE25(), E9oE25);


    double errorArray[6];
    myECLShower.getError(errorArray);
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