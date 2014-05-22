// #include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>
#include <iostream>
// #include <limits>       // std::numeric_limits
#include <gtest/gtest.h>
#include <framework/utilities/TestHelpers.h> // EXPECT_B2FATAL

#include <tracking/dataobjects/FullSecID.h>
#include <tracking/trackFindingVXD/sectorMapTools/Sector.h>
#include <framework/gearbox/GearDir.h> // needed for reading xml-files, Gearbox::getInstance...
#include <framework/datastore/StoreArray.h>

#include <tracking/trackFindingVXD/sectorMapTools/SectorMapFactory.h>
#include "tracking/dataobjects/VXDTFSecMap.h"

using namespace std;

namespace Belle2 {
  /** command x should exit using B2FATAL. */
#define EXPECT_FATAL(x) EXPECT_EXIT(x,::testing::KilledBySignal(SIGABRT),"");

  /** Tests the production of fully grown sectorMaps */
  class SectorMapFactoryTest : public ::testing::Test {
  protected:
  };

  /** Test Constructor, Setters and Getters. */
  TEST_F(SectorMapFactoryTest, testingConstructors)
  {
    SectorMapFactory emptyFactory; // using standard-settings

    // manually preparing gearbox so that it can work without the rest
    Gearbox& gb = Gearbox::getInstance();
    vector<string> backends = { "file:" };
    gb.setBackends(backends);
    gb.open("geometry/Belle2.xml");

    // throws fatal since the standard setting is not linked to an existing rootSectorMap;
    EXPECT_B2FATAL(emptyFactory.importRootMap());

    // sectorMapFactory prepares the necessities of the storeArray itself
    StoreArray<Sector> emptySectorArray(emptyFactory.getMapName());
    EXPECT_EQ(emptySectorArray.getEntries(), 0);

//     string workingMapName = "sectorList_evtNormSecHIGH_SVD";
    string workingMapName = "secMapEvtGenOnR10501May2014SVDStd-moreThan500MeV_SVD";
    SectorMapFactory workingFactory(workingMapName);

    // loading the same map as the factory:
    string directory = "/Detector/Tracking/CATFParameters/" + workingMapName;
    const VXDTFSecMap* rootMap = dynamic_cast<const VXDTFSecMap*>(gb.getTObject(directory.c_str()));

    workingFactory.importRootMap();
    StoreArray<Sector> workingSectorArray(workingFactory.getMapName());

    EXPECT_EQ(workingSectorArray.getEntries(), rootMap->size());

    B2INFO(" output of first sector: " << workingSectorArray[0]->printSector())

//     VxdID vxdID = VxdID(34624); // this should be a sensor on layer 4
//     bool subLayerID = true;
//     unsigned short sectorID = 15;
//
//
//     B2INFO("creating FullSecIDs: ")
//     FullSecID aFullSecID = FullSecID(vxdID, subLayerID, sectorID);
//     unsigned int aFullSecInt = static_cast<unsigned int>(aFullSecID);
//
//     FullSecID aFullSecID2 = FullSecID(vxdID, false, sectorID);
//     unsigned int aFullSecInt2 = static_cast<unsigned int>(aFullSecID2);
//
//     FullSecID aFullSecID3 = FullSecID(vxdID, subLayerID, 7);
//     unsigned int aFullSecInt3 = static_cast<unsigned int>(aFullSecID3);
//
//     FullSecID aFullSecID4;
//     unsigned int aFullSecInt4 = static_cast<unsigned int>(aFullSecID4);
//
//
//     // producing sectors which can be sorted by secID:
//     Sector aSector(aFullSecInt);
//     Sector aSector2(aFullSecInt2);
//     Sector aSector3(aFullSecInt3);
//     Sector aSector4(aFullSecInt4);
//
//     EXPECT_EQ(aFullSecInt, aSector.getSecID());
//     EXPECT_EQ(aFullSecInt2, aSector2.getSecID());
//     EXPECT_EQ(aFullSecInt3, aSector3.getSecID());
//     EXPECT_EQ(aFullSecInt4, aSector4.getSecID());
//
//
//     // producing sectors which can be sorted by secID:
//     Sector dSector(aFullSecInt, 3.5);
//     Sector dSector2(aFullSecInt2, 3.3, true);
//     Sector dSector3(aFullSecInt3, 3.4);
//     Sector dSector4(aFullSecInt4, 0, true);
  }
}
