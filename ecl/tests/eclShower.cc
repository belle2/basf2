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
    EXPECT_EQ(myECLShower.getShowerId(), 0);
    EXPECT_EQ(myECLShower.getEnergy(), 0);
    EXPECT_EQ(myECLShower.getTheta(), 0);
    EXPECT_EQ(myECLShower.getPhi(), 0);
    EXPECT_EQ(myECLShower.getR(), 0);
    EXPECT_EQ(myECLShower.getEnergyError(), 0);
    EXPECT_EQ(myECLShower.getPhiError(), 0);
    EXPECT_EQ(myECLShower.getThetaError(), 0);
    EXPECT_EQ(myECLShower.getMass(), 0);
    EXPECT_EQ(myECLShower.getWidth(), 0);
    EXPECT_EQ(myECLShower.getE9oE25(), 0);
    EXPECT_EQ(myECLShower.getE9oE25unf(), 0);
    EXPECT_EQ(myECLShower.getNHits(), 0);
    EXPECT_EQ(myECLShower.getStatus(), 0);
    EXPECT_EQ(myECLShower.getGrade(), 0);
    EXPECT_EQ(myECLShower.getUncEnergy(), 0);
    EXPECT_EQ(myECLShower.getTime(), 0);

    float errorArray[3];
    myECLShower.getError(errorArray);
    EXPECT_EQ(errorArray[0], 0);
    EXPECT_EQ(errorArray[1], 0);
    EXPECT_EQ(errorArray[2], 0);

    TVector3 momentum(myECLShower.getMomentum());
    EXPECT_EQ(momentum.X(), 0);
    EXPECT_EQ(momentum.Y(), 0);
    EXPECT_EQ(momentum.Z(), 0);
  } // Testcases for Something

  /** Test Setters and Getter. */
  TEST_F(ECLShowerTest, SettersAndGetters)
  {
    const int showerId = 1;
    const float energy = 1.1;
    const float theta = 1.2;
    const float phi = 1.3;
    const float r = 1.4;
    float error[3] = {7.7, 7.8, 7.9};
    const float mass = 1.5;
    const float width = 1.6;
    const float E9oE25 = 1.7;
    const float E9oE25unf = 1.8;
    const float nHits = 2;
    const int status = 3;
    const int grade = 4;
    const float uncEnergy = 1.9;
    const float time = 2.1;

    ECLShower myECLShower;
    myECLShower.setShowerId(showerId);
    myECLShower.setEnergy(energy);
    myECLShower.setTheta(theta);
    myECLShower.setPhi(phi);
    myECLShower.setR(r);
    myECLShower.setError(error);
    myECLShower.setMass(mass);
    myECLShower.setWidth(width);
    myECLShower.setE9oE25(E9oE25);
    myECLShower.setE9oE25unf(E9oE25unf);
    myECLShower.setNHits(nHits);
    myECLShower.setStatus(status);
    myECLShower.setGrade(grade);
    myECLShower.setUncEnergy(uncEnergy);
    myECLShower.setTime(time);

    EXPECT_EQ(myECLShower.getShowerId(), showerId);
    EXPECT_EQ(myECLShower.getEnergy(), energy);
    EXPECT_EQ(myECLShower.getTheta(), theta);
    EXPECT_EQ(myECLShower.getPhi(), phi);
    EXPECT_EQ(myECLShower.getR(), r);
    EXPECT_EQ(myECLShower.getEnergyError(), error[0]);
    EXPECT_EQ(myECLShower.getThetaError(), error[1]);
    EXPECT_EQ(myECLShower.getPhiError(), error[2]);
    EXPECT_EQ(myECLShower.getMass(), mass);
    EXPECT_EQ(myECLShower.getWidth(), width);
    EXPECT_EQ(myECLShower.getE9oE25(), E9oE25);
    EXPECT_EQ(myECLShower.getE9oE25unf(), E9oE25unf);
    EXPECT_EQ(myECLShower.getNHits(), nHits);
    EXPECT_EQ(myECLShower.getStatus(), status);
    EXPECT_EQ(myECLShower.getGrade(), grade);
    EXPECT_EQ(myECLShower.getUncEnergy(), uncEnergy);
    EXPECT_EQ(myECLShower.getTime(), time);

    float errorArray[3];
    myECLShower.getError(errorArray);
    EXPECT_EQ(errorArray[0], error[0]);
    EXPECT_EQ(errorArray[1], error[1]);
    EXPECT_EQ(errorArray[2], error[2]);

    TVector3 momentum(myECLShower.getMomentum());
    EXPECT_FLOAT_EQ(momentum.X(), energy * sin(theta) * cos(phi));
    EXPECT_FLOAT_EQ(momentum.Y(), energy * sin(theta) * sin(phi));
    EXPECT_FLOAT_EQ(momentum.Z(), energy * cos(theta));
  } // Testcases for Setters and Getters

}  // namespace
