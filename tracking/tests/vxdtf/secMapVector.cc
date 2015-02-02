#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>
#include <tracking/dataobjects/SecMapVector.h>
#include <tracking/dataobjects/VXDTFRawSecMap.h>
#include <gtest/gtest.h>

#include <string>
#include <utility> // std::pair, std::move(objects)
#include <stdio.h> // remove(file)

#include "TFile.h"
#include "TKey.h"
#include "TCollection.h"

using namespace std;

namespace Belle2 {



  /** Testing SecMapVector to be able to store it in a .root file and reopen/rewrite it */
  class SecMapVectorTest : public ::testing::Test {
  protected:
  };

  /** Filling and reading SecMapVector in/from root file */
  TEST_F(SecMapVectorTest, readAndWriteTObjectToRootFile)
  {
    string fName = "demo.root";

    /// create SecMapVector, fill with some fake VXDTFTemporarySecMaps (setting only the name to check whether everything went fine or not)
    string aSecMapName = "aSecMap";
    SecMapVector aSecMapVector;
    VXDTFRawSecMap aSecMap;
    aSecMap.setMapName(aSecMapName);
    aSecMap.setLowerMomentumThreshold(0.23);
    SecMapVector::MapPack firstPack = make_pair(aSecMapName, aSecMap);
    aSecMapVector.push_back(firstPack);
    B2INFO("creating a secMap named " << aSecMap.getMapName() << " having " << aSecMap.getMaxDistance2origin() << " as maxDistance2Origin- and " << aSecMap.getLowerMomentumThreshold() << " for lowerMomentum-threshold")

    std::string anotherSecMapName = "anotherSecMap";
    VXDTFRawSecMap anotherSecMap;
    anotherSecMap.setMapName(anotherSecMapName);
    anotherSecMap.setLowerMomentumThreshold(0.23);
    SecMapVector::MapPack secondPack = make_pair(anotherSecMapName, anotherSecMap);
    aSecMapVector.push_back(secondPack);
    B2INFO("creating a secMap named " << anotherSecMap.getMapName() << " having " << anotherSecMap.getMaxDistance2origin() << " as maxDistance2Origin- and " << anotherSecMap.getLowerMomentumThreshold() << " for lowerMomentum-threshold")

    for (const SecMapVector::MapPack & aMap : aSecMapVector.getFullVector()) {
      B2INFO("vector stored map with name " << aMap.second.getMapName() << " and lower threshold " << aMap.second.getLowerMomentumThreshold())
    }

    /// create root file, fill SecMapVector into file, store it, close
    B2INFO("opening root file:");
    TFile f(fName.c_str(), "recreate");
    B2INFO("storing data to root file:")
    aSecMapVector.Write();
    B2INFO("closing file:")
    f.Close();

    /// repeat first step to check, whether files can be filled repeatedly
    string aThirdSecMapName = "aThirdSecMap";
    SecMapVector aSecMapVector2;
    VXDTFRawSecMap aThirdSecMap;
    aThirdSecMap.setMapName(aThirdSecMapName);
    aThirdSecMap.setLowerMomentumThreshold(0.23);
    SecMapVector::MapPack thirdPack = make_pair(aThirdSecMapName, aThirdSecMap);
    aSecMapVector2.push_back(thirdPack);
    B2INFO("creating a secMap named " << aThirdSecMap.getMapName() << " having " << aThirdSecMap.getMaxDistance2origin() << " as maxDistance2Origin- and " << aThirdSecMap.getLowerMomentumThreshold() << " for lowerMomentum-threshold")

    string aFourthSecMapName = "aFourthSecMap";
    VXDTFRawSecMap aFourthSecMap;
    aFourthSecMap.setMapName(aFourthSecMapName);
    aFourthSecMap.setLowerMomentumThreshold(0.23);
    SecMapVector::MapPack fourthPack = make_pair(aFourthSecMapName, aFourthSecMap);
    aSecMapVector2.push_back(fourthPack);
    B2INFO("creating a secMap named " << aFourthSecMap.getMapName() << " having " << aFourthSecMap.getMaxDistance2origin() << " as maxDistance2Origin- and " << aFourthSecMap.getLowerMomentumThreshold() << " for lowerMomentum-threshold")

    /// reopen root file, fill SecMapVector into file, store it, close
    B2INFO("opening root file:");
    TFile f3(fName.c_str(), "update");
    B2INFO("storing data to root file:")
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
          B2WARNING("Key was not a SecMapVector, therefore error message: " << e.what() << "\n Skipping this key...")
          continue;
        }

        for (SecMapVector::MapPack & entry : retrievedVector->getFullVector()) {
          EXPECT_FLOAT_EQ(-1, entry.second.getMaxDistance2origin());
          EXPECT_FLOAT_EQ(1.5, entry.second.getMagneticFieldStrength());
          EXPECT_FLOAT_EQ(0.23, entry.second.getLowerMomentumThreshold());
          B2INFO(".first is: " << entry.first << ", .second->getMapName() is:" << entry.second.getMapName())
        }
      }
      f2.Close();
    }

//     /// delete file from disk
//     if (remove(fName.c_str()) != 0)
//     { B2ERROR("could not delete file!") }
//     else
//     { B2INFO("File successfully deleted") }

  }

}  // namespace
