/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <framework/logging/Logger.h>
#include <gtest/gtest.h>
#include <tracking/dataobjects/FullSecID.h>
#include <tracking/trackFindingVXD/sectorMapTools/Sector.h>

namespace Belle2 {

  /** Set up a few arrays and objects in the datastore */
  class SectorTest : public ::testing::Test {
  protected:
  };

  /** Test Constructor,  Setters and Getters. */
  TEST_F(SectorTest, testConstructorSettersAndGetters)
  {
    VxdID vxdID = VxdID(34624); // this should be a sensor on layer 4
    bool subLayerID = true;
    unsigned short sectorID = 15;


    B2INFO("creating FullSecIDs: ");
    FullSecID aFullSecID = FullSecID(vxdID, subLayerID, sectorID);
    unsigned int aFullSecInt = static_cast<unsigned int>(aFullSecID);

    FullSecID aFullSecID2 = FullSecID(vxdID, false, sectorID);
    unsigned int aFullSecInt2 = static_cast<unsigned int>(aFullSecID2);

    FullSecID aFullSecID3 = FullSecID(vxdID, subLayerID, 7);
    unsigned int aFullSecInt3 = static_cast<unsigned int>(aFullSecID3);

    FullSecID aFullSecID4;
    unsigned int aFullSecInt4 = static_cast<unsigned int>(aFullSecID4);


    // producing sectors which can be sorted by secID:
    Sector aSector(aFullSecInt);
    Sector aSector2(aFullSecInt2);
    Sector aSector3(aFullSecInt3);
    Sector aSector4(aFullSecInt4);

    EXPECT_EQ(aFullSecInt, aSector.getSecID());
    EXPECT_EQ(aFullSecInt2, aSector2.getSecID());
    EXPECT_EQ(aFullSecInt3, aSector3.getSecID());
    EXPECT_EQ(aFullSecInt4, aSector4.getSecID());
    EXPECT_EQ(0, aSector.getDistance());
    EXPECT_EQ(0, aSector2.getDistance());
    EXPECT_EQ(0, aSector3.getDistance());
    EXPECT_EQ(0, aSector4.getDistance());


    // producing sectors which can be sorted by secID:
    Sector dSector(aFullSecInt, 3.5);
    Sector dSector2(aFullSecInt2, 3.3, true);
    Sector dSector3(aFullSecInt3, 3.4);
    Sector dSector4(aFullSecInt4, 0, true);
  }
}
