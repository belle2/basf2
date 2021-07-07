/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <framework/logging/Logger.h>
#include <tracking/dataobjects/FullSecID.h>
#include <iostream>
#include <gtest/gtest.h>
#include <vector>

namespace Belle2 {

  /** Testing everything from FullSecID */
  class FullSecIDTest : public ::testing::Test {
  protected:
  };

  /** Test simple Setters and Getters. */
  TEST_F(FullSecIDTest, constructorAndGetterTests)
  {
    // first, we need a usefull vxdID:
    VxdID vxdID = VxdID(34624); // this should be a sensor on layer 4
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
    std::stringstream aSecIDString;
    aSecIDString << aFullSecID.getLayerID() << aFullSecID.getSubLayerID() << "_" << int(aFullSecID.getVxdID()) << "_" <<
                 aFullSecID.getSecID();

    FullSecID aThirdFullSecID = FullSecID(aSecIDString.str());

    EXPECT_EQ(4, aThirdFullSecID.getLayerID());

    EXPECT_EQ(subLayerID, aThirdFullSecID.getSubLayerID());

    EXPECT_EQ(vxdID, aThirdFullSecID.getVxdID());

    EXPECT_EQ(vxdIDInt, aThirdFullSecID.getUniID());

    EXPECT_EQ(aFullSecID.getSecID(), aThirdFullSecID.getSecID());

    EXPECT_EQ(aFullSecID.getFullSecID(), aThirdFullSecID.getFullSecID());

    EXPECT_EQ(aSecIDString.str(), aThirdFullSecID.getFullSecString());

    // now we are using the third constructor again using an encoded short fullSecID (string, used by filterCalculator) as input:
    std::stringstream aSecIDString2;
    aSecIDString2 << aFullSecID.getLayerID() << "_" << int(aFullSecID.getVxdID()) << "_" << aFullSecID.getSecID();

    FullSecID aFourthFullSecID = FullSecID(aSecIDString2.str());

    EXPECT_EQ(4, aFourthFullSecID.getLayerID());

    EXPECT_FALSE(aFourthFullSecID.getSubLayerID());

    EXPECT_EQ(vxdID, aFourthFullSecID.getVxdID());

    EXPECT_EQ(vxdIDInt, aFourthFullSecID.getUniID());

    EXPECT_EQ(FullSecID(vxdID, false, sectorID).getSecID(), aFourthFullSecID.getSecID());

    EXPECT_NE(aSecIDString2.str(), aFourthFullSecID.getFullSecString()); // they should not be the same any more...


    // testing copy constructor (and C++11 range based for-loops):
    std::vector<FullSecID> testVector;
    for (int i = 0; i < 5; ++i) {
      testVector.push_back(aFullSecID);
    }
    for (auto aSecID : testVector) {
      EXPECT_EQ(aFullSecID, aSecID);
    }
  }


  /** testing the overloaded operators of the FullSecID-class */
  TEST_F(FullSecIDTest, overloadedOperatorTests)
  {
    //preparing stuff (procedures copied from constructorAndGetterTest):
    VxdID vxdID = VxdID(34624); // this should be a sensor on layer 4
    bool subLayerID = true;
    unsigned short sectorID = 15;
    FullSecID aFullSecID = FullSecID(vxdID, subLayerID, sectorID);
    std::stringstream aSecIDString;
    aSecIDString << aFullSecID.getLayerID() << aFullSecID.getSubLayerID() << "_" << int(aFullSecID.getVxdID()) << "_" <<
                 aFullSecID.getSecID();

    FullSecID aFullSecID2 = FullSecID(aSecIDString.str());

    // now the checks:
    EXPECT_EQ(static_cast<unsigned int>(aFullSecID), aFullSecID2);  // directly comparing to an int

    EXPECT_EQ(aSecIDString.str(), std::string(aFullSecID2)); // directly comparing to an string - testing string cast

    EXPECT_EQ(aFullSecID, aFullSecID2); // direct comparison


    std::stringstream aSecIDStream, aSecIDStream2;
    aSecIDStream << aFullSecID2;
    aSecIDStream2 << aSecIDString.str();

    EXPECT_EQ(aSecIDString.str(), aSecIDStream.str()); // testing stream operator overloading after string-conversion

    FullSecID aFullSecID3 = FullSecID(vxdID, false, sectorID); // same ID as above but now, the sublayerID is false instead of true

    EXPECT_GT(aFullSecID2, aFullSecID3); // aFullSecID2 > aFullSecID3

    for (int l1 = 7; l1 != 0; --l1) { // testing layerIDs
      int l2 = l1;
      l2--;
      FullSecID biggerOne = FullSecID(l1, false, 0, 0);
      FullSecID smallerOne = FullSecID(l2, false, 0, 0);
      EXPECT_GT(biggerOne, smallerOne);
    }
    for (int s1 = 255; s1 != 0; --s1) { // testing sectorIDs
      int s2 = s1;
      s2--;
      FullSecID biggerOne = FullSecID(6, false, 0, s1);
      FullSecID smallerOne = FullSecID(6, false, 0, s2);
      EXPECT_GT(biggerOne, smallerOne); // aFullSecID2 > aFullSecID3
      int equalOne = smallerOne;
      equalOne++;
      EXPECT_EQ(int(biggerOne), equalOne);
    }

    EXPECT_GT(int(aFullSecID2), int(aFullSecID3));


    FullSecID aFullSecID4 = aFullSecID;

    EXPECT_EQ(aFullSecID4, aFullSecID); // testing assignment operator;
  }


  /** Unfinished test - shall test bufferOverflows... */
  TEST_F(FullSecIDTest, bufferOverflowTest)
  {
    B2WARNING("TODO: FullSecIDTest:bufferOverflowTest should catch cases of bad user input");
    // WARNING TODO should catch cases of bad user input
  }
}  // namespace
