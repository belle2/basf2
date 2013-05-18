#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>
#include <tracking/vxdCaTracking/FullSecID.h>
#include <iostream>
// #include <TMatrixF.h>
// #include <RKTrackRep.h>
#include <gtest/gtest.h>

using namespace std;

namespace Belle2 {
#define EXPECT_FATAL(x) EXPECT_EXIT(x,::testing::KilledBySignal(SIGABRT),"");

  /** Set up a few arrays and objects in the datastore */
  class FullSecIDTest : public ::testing::Test {
  protected:
  };

  /** Test simple Setters and Getters. */
  TEST_F(FullSecIDTest, simpleTest)
  {
    // first, we need a usefull vxdID:
    VxdID vxdID = VxdID(34624); // this should be a sensor on layer 3
    int vxdIDInt = vxdID;

    EXPECT_EQ(4, vxdID.getLayerNumber());

    // now we are using the constructor with a VxdID, a subLayerID and a sectorID
    bool subLayerID = true;
    unsigned short sectorID = 15;
    FullSecID aFullSecID = FullSecID(vxdID, subLayerID, sectorID);

    EXPECT_EQ(4, aFullSecID.getLayerID());

    EXPECT_EQ(subLayerID, aFullSecID.getSubLayerID());

    EXPECT_EQ(vxdID, aFullSecID.getVxdID());

    EXPECT_EQ(vxdIDInt, aFullSecID.getUniID());

    EXPECT_EQ(sectorID, aFullSecID.getSecID());

    // now we are using the second constructor using an encoded fullSecID (int) as input:

    FullSecID anotherFullSecID = FullSecID(aFullSecID.getFullSecID());

    EXPECT_EQ(4, anotherFullSecID.getLayerID());

    EXPECT_EQ(subLayerID, anotherFullSecID.getSubLayerID());

    EXPECT_EQ(vxdID, anotherFullSecID.getVxdID());

    EXPECT_EQ(vxdIDInt, anotherFullSecID.getUniID());

    EXPECT_EQ(aFullSecID.getSecID(), anotherFullSecID.getSecID());

    EXPECT_EQ(aFullSecID.getFullSecID(), anotherFullSecID.getFullSecID());

    // now we are using the third constructor using an encoded fullSecID (string) as input:
    stringstream aSecIDString;
    aSecIDString << aFullSecID.getLayerID() << aFullSecID.getSubLayerID() << "_" << int(aFullSecID.getVxdID()) << "_" << aFullSecID.getSecID();

    FullSecID aThirdFullSecID = FullSecID(aSecIDString.str());

    EXPECT_EQ(4, aThirdFullSecID.getLayerID());

    EXPECT_EQ(subLayerID, aThirdFullSecID.getSubLayerID());

    EXPECT_EQ(vxdID, aThirdFullSecID.getVxdID());

    EXPECT_EQ(vxdIDInt, aThirdFullSecID.getUniID());

    EXPECT_EQ(aFullSecID.getSecID(), aThirdFullSecID.getSecID());

    EXPECT_EQ(aFullSecID.getFullSecID(), aThirdFullSecID.getFullSecID());

    EXPECT_EQ(aSecIDString.str(), aThirdFullSecID.getFullSecString());
  }
}  // namespace
