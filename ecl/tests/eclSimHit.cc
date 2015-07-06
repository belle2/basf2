#include <ecl/dataobjects/ECLSimHit.h>
#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>

#include <utility>

#include <gtest/gtest.h>

using namespace std;

namespace Belle2 {

  /** Set up a few arrays and objects in the datastore */
  class ECLSimHitTest : public ::testing::Test {
  };

  /** Test Constructors. */
  TEST_F(ECLSimHitTest, Constructors)
  {
    ECLSimHit myECLSimHit;
    EXPECT_EQ(myECLSimHit.getCellId() , 0);
    EXPECT_EQ(myECLSimHit.getTrackId() , 0);
    EXPECT_EQ(myECLSimHit.getPDGCode() , 0);
    EXPECT_EQ(myECLSimHit.getFlightTime() , 0);
    EXPECT_EQ(myECLSimHit.getEnergyDep() , 0);
    const auto momentum = myECLSimHit.getMomentum();
    EXPECT_EQ(momentum.X(), 0);
    EXPECT_EQ(momentum.Y(), 0);
    EXPECT_EQ(momentum.Z(), 0);
    const auto position = myECLSimHit.getPosition();
    EXPECT_EQ(position.X(), 0);
    EXPECT_EQ(position.Y(), 0);
    EXPECT_EQ(position.Z(), 0);

    const int cellId = 1;
    const int trackId = 2;
    const int pdg = 3;
    const float flightTime = 4.1;
    const float edep = 4.2;
    const TVector3 init_momentum(1, 2, 3);
    const TVector3 init_position(4, 5, 6);
    ECLSimHit myECLSimHit2(cellId, trackId, pdg, flightTime, edep, init_momentum, init_position);
    EXPECT_EQ(myECLSimHit2.getCellId(), cellId);
    EXPECT_EQ(myECLSimHit2.getTrackId(), trackId);
    EXPECT_EQ(myECLSimHit2.getPDGCode(), pdg);
    EXPECT_EQ(myECLSimHit2.getFlightTime(), flightTime);
    EXPECT_EQ(myECLSimHit2.getEnergyDep(), edep);
    const auto momentum2 = myECLSimHit2.getMomentum();
    EXPECT_EQ(momentum2.X(), init_momentum.X());
    EXPECT_EQ(momentum2.Y(), init_momentum.Y());
    EXPECT_EQ(momentum2.Z(), init_momentum.Z());
    const auto position2 = myECLSimHit2.getPosition();
    EXPECT_EQ(position2.X(), init_position.X());
    EXPECT_EQ(position2.Y(), init_position.Y());
    EXPECT_EQ(position2.Z(), init_position.Z());

  } // Testcases for Something


  /** Test Setters and Getters */
  TEST_F(ECLSimHitTest, SettersAndGetters)
  {
    const int cellId = 1;
    const int trackId = 2;
    const int pdg = 3;
    const float flightTime = 4.1;
    const float edep = 4.2;
    const TVector3 init_momentum(1, 2, 3);
    const TVector3 init_position(4, 5, 6);
    ECLSimHit myECLSimHit;
    myECLSimHit.setCellId(cellId);
    myECLSimHit.setTrackId(trackId);
    myECLSimHit.setPDGCode(pdg);
    myECLSimHit.setFlightTime(flightTime);
    myECLSimHit.setEnergyDep(edep);
    myECLSimHit.setPosition(init_position);
    myECLSimHit.setMomentum(init_momentum);

    EXPECT_EQ(myECLSimHit.getCellId(), cellId);
    EXPECT_EQ(myECLSimHit.getTrackId(), trackId);
    EXPECT_EQ(myECLSimHit.getPDGCode(), pdg);
    EXPECT_EQ(myECLSimHit.getFlightTime(), flightTime);
    EXPECT_EQ(myECLSimHit.getEnergyDep(), edep);
    const auto momentum = myECLSimHit.getMomentum();
    EXPECT_EQ(momentum.X(), init_momentum.X());
    EXPECT_EQ(momentum.Y(), init_momentum.Y());
    EXPECT_EQ(momentum.Z(), init_momentum.Z());
    const auto position = myECLSimHit.getPosition();
    EXPECT_EQ(position.X(), init_position.X());
    EXPECT_EQ(position.Y(), init_position.Y());
    EXPECT_EQ(position.Z(), init_position.Z());

  } // Testcases for Setters and Getters

}  // namespace
