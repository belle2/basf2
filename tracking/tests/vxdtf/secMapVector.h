/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jakob Lettenbichler                                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>

#include <tracking/dataobjects/FilterID.h>
#include <tracking/dataobjects/FullSecID.h>
#include <tracking/dataobjects/VXDTFRawSecMap.h>
#include <tracking/dataobjects/SecMapVector.h>
#include <tracking/dataobjects/VXDTFRawSecMap.h>
#include <gtest/gtest.h>

#include <string>
#include <utility> // std::pair, std::move(objects)
#include <stdio.h> // remove(file)

//root stuff
#include <TRandom.h>
#include "TFile.h"
#include "TKey.h"
#include "TCollection.h"

using namespace std;

namespace Belle2 {

//   VXDTFRawSecMapTypedef::CutoffValues cutValues; /**< list containing all values calculated during run */
//   cutValues.push_back(rndVal);
//   VXDTFRawSecMapTypedef::CutoffPack cutPack = {filterID, cutValues};
//   VXDTFRawSecMapTypedef::FriendValues frValues = {cutPack};
//   VXDTFRawSecMapTypedef::FriendPack frPack = {friendSecID, frValues};
//   VXDTFRawSecMapTypedef::SectorValues secValues = {frPack};
//   VXDTFRawSecMapTypedef::SectorPack secPack = {mainSecID, secValues};
//   VXDTFRawSecMapTypedef::StrippedRawSecMap strippedRawSecMap = {secPack};
//   newTemporarySecMap.addSectorMap(strippedRawSecMap);

  /** Testing SecMapVector to be able to store it in a .root file and reopen/rewrite it */
  class SecMapVectorTest : public ::testing::Test {

  public:
    /** provides a randomly filled map with some entries.
     *
     * nCuts = how many cuts per filter shall this map contain? not higher than VXDTFRawSecMap::BaseType::max
     * nFilters = how many filters per friend shall this map contain?, not higher than ~15
     * nFriends = how many friends per sector shall this map contain?, not higher than ~6
     * nSectors = how many sectors shall this map contain? not higher than ~10000
     * */
    static VXDTFRawSecMapTypedef::StrippedRawSecMap randomFillRawSecMap(unsigned nCuts = 50, unsigned nFilters = 2,
        unsigned nFriends = 5, unsigned nSectors = 10);
  protected:
  };



  VXDTFRawSecMapTypedef::StrippedRawSecMap SecMapVectorTest::randomFillRawSecMap(unsigned nCuts, unsigned nFilters, unsigned nFriends,
      unsigned nSectors)
  {
    VXDTFRawSecMapTypedef::StrippedRawSecMap strippedRawSecMap;

    for (unsigned iSec = nSectors; iSec > 0; iSec--) {
      unsigned mainSecLayerID = 1 + (iSec % nFriends + 1);
      unsigned mainSecID  = FullSecID(mainSecLayerID,  false , iSec, mainSecLayerID).getFullSecID();

      VXDTFRawSecMapTypedef::SectorValues secValues;

      for (unsigned iFriend = nFriends + 1; iFriend > mainSecLayerID; iFriend--) {
        unsigned friendSecID = FullSecID(iFriend, ((iFriend % 2 == 0) ? false : true) , iFriend, iFriend).getFullSecID();

        VXDTFRawSecMapTypedef::FriendValues frValues;

        for (unsigned iFilter = 0; iFilter < nFilters; iFilter++) {
          unsigned filterID = FilterID::getFilterType(iFilter);

          VXDTFRawSecMapTypedef::CutoffValues cutValues; /**< list containing all values calculated during run */

          for (unsigned iCut = 0; iCut < nCuts; iCut++) {
            unsigned rndVal = gRandom->Uniform(double(iCut));

            cutValues.push_back(rndVal);
          }

          VXDTFRawSecMapTypedef::CutoffPack cutPack = {filterID, cutValues};
          frValues.push_back(cutPack);
        }

        VXDTFRawSecMapTypedef::FriendPack frPack = {friendSecID, frValues};
        secValues.push_back(frPack);
      }

      VXDTFRawSecMapTypedef::SectorPack secPack = {mainSecID, secValues};
      strippedRawSecMap.push_back(secPack);
    }

    return strippedRawSecMap;
  }



  /** Filling and reading SecMapVector in/from root file */
  TEST_F(SecMapVectorTest, readAndWriteTObjectToRootFile)
  {
    string fName = "demo.root";
    unsigned nCuts = 50, nFilters = 2, nFriends = 5, nSectors = 10;

    /// create SecMapVector, fill with some fake VXDTFTemporarySecMaps (setting only the name to check whether everything went fine or not)
    string aSecMapName = "aSecMap";
    SecMapVector aSecMapVector;
    VXDTFRawSecMap aSecMap;
    aSecMap.setMapName(aSecMapName);
    aSecMap.setLowerMomentumThreshold(0.23);
    VXDTFRawSecMapTypedef::StrippedRawSecMap thisMap = SecMapVectorTest::randomFillRawSecMap(nCuts, nFilters, nFriends, nSectors);
    aSecMap.addSectorMap(thisMap);
    SecMapVector::MapPack firstPack = make_pair(aSecMapName, aSecMap);
    aSecMapVector.push_back(firstPack);
    B2INFO("creating a secMap named " << aSecMap.getMapName() << " having " << aSecMap.getMaxDistance2origin() <<
           " as maxDistance2Origin- and " << aSecMap.getLowerMomentumThreshold() << " for lowerMomentum-threshold. nValues: " <<
           aSecMap.getNumOfValues());

    nCuts++; nSectors++;
    std::string anotherSecMapName = "anotherSecMap";
    VXDTFRawSecMap anotherSecMap;
    anotherSecMap.setMapName(anotherSecMapName);
    anotherSecMap.setLowerMomentumThreshold(0.23);
    thisMap = SecMapVectorTest::randomFillRawSecMap(nCuts, nFilters, nFriends, nSectors);
    anotherSecMap.addSectorMap(thisMap);
    SecMapVector::MapPack secondPack = make_pair(anotherSecMapName, anotherSecMap);
    aSecMapVector.push_back(secondPack);
    B2INFO("creating a secMap named " << anotherSecMap.getMapName() << " having " << anotherSecMap.getMaxDistance2origin() <<
           " as maxDistance2Origin- and " << anotherSecMap.getLowerMomentumThreshold() << " for lowerMomentum-threshold. nValues: " <<
           anotherSecMap.getNumOfValues());

    for (SecMapVector::MapPack& aMap : aSecMapVector.getFullVector()) {
      B2INFO("vector stored map with name " << aMap.second.getMapName() << " and lower threshold " <<
             aMap.second.getLowerMomentumThreshold() << " and nValues: " << aMap.second.getNumOfValues());
    }

    /// create root file, fill SecMapVector into file, store it, close
    B2INFO("opening root file:");
    TFile f(fName.c_str(), "recreate");
    B2INFO("storing data to root file:");
    aSecMapVector.Write();
    B2INFO("closing file:");
    f.Close();

    /// repeat first step to check, whether files can be filled repeatedly
    nCuts++; nSectors++;
    string aThirdSecMapName = "aThirdSecMap";
    SecMapVector aSecMapVector2;
    VXDTFRawSecMap aThirdSecMap;
    aThirdSecMap.setMapName(aThirdSecMapName);
    aThirdSecMap.setLowerMomentumThreshold(0.23);
    thisMap = SecMapVectorTest::randomFillRawSecMap(nCuts, nFilters, nFriends, nSectors);
    aThirdSecMap.addSectorMap(thisMap);
    SecMapVector::MapPack thirdPack = make_pair(aThirdSecMapName, aThirdSecMap);
    aSecMapVector2.push_back(thirdPack);
    B2INFO("creating a secMap named " << aThirdSecMap.getMapName() << " having " << aThirdSecMap.getMaxDistance2origin() <<
           " as maxDistance2Origin- and " << aThirdSecMap.getLowerMomentumThreshold() << " for lowerMomentum-threshold. nValues: " <<
           aThirdSecMap.getNumOfValues());

    nCuts++; nSectors++;
    string aFourthSecMapName = "aFourthSecMap";
    VXDTFRawSecMap aFourthSecMap;
    aFourthSecMap.setMapName(aFourthSecMapName);
    aFourthSecMap.setLowerMomentumThreshold(0.23);
    thisMap = SecMapVectorTest::randomFillRawSecMap(nCuts, nFilters, nFriends, nSectors);
    aFourthSecMap.addSectorMap(thisMap);
    SecMapVector::MapPack fourthPack = make_pair(aFourthSecMapName, aFourthSecMap);
    aSecMapVector2.push_back(fourthPack);
    B2INFO("creating a secMap named " << aFourthSecMap.getMapName() << " having " << aFourthSecMap.getMaxDistance2origin() <<
           " as maxDistance2Origin- and " << aFourthSecMap.getLowerMomentumThreshold() << " for lowerMomentum-threshold. nValues: " <<
           aFourthSecMap.getNumOfValues());

    /// reopen root file, fill SecMapVector into file, store it, close
    B2INFO("opening root file:");
    TFile f3(fName.c_str(), "update");
    B2INFO("storing data to root file:");
    aSecMapVector2.Write();
    f3.Close();


    /// reopen file, read entries and check them
    TFile f2(fName.c_str());
    if (f2.IsZombie()) { B2ERROR("file could not be reopened!"); }
    else {
      SecMapVector* retrievedVector;
      f2.GetListOfKeys()->Print();

      TIter next(f2.GetListOfKeys());
      TKey* key;
      while ((key = (TKey*)next())) {

        try {
          retrievedVector = static_cast<SecMapVector*>(key->ReadObj());
        } catch (exception& e) {
          B2WARNING("Key was not a SecMapVector, therefore error message: " << e.what() << "\n Skipping this key...");
          continue;
        }

        for (SecMapVector::MapPack& entry : retrievedVector->getFullVector()) {
          EXPECT_FLOAT_EQ(-1, entry.second.getMaxDistance2origin());
          EXPECT_FLOAT_EQ(1.5, entry.second.getMagneticFieldStrength());
          EXPECT_FLOAT_EQ(0.23, entry.second.getLowerMomentumThreshold());
          B2INFO(".first is: " << entry.first << ", .second->getMapName() is:" << entry.second.getMapName());
          B2INFO("and nValues: " << entry.second.getNumOfValues());
        }
      }
      f2.Close();
    }

    /// delete file from disk
    if (remove(fName.c_str()) != 0)
    { B2ERROR("could not delete file!"); }
    else
    { B2INFO("File successfully deleted"); }

  }

}  // namespace
