#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>
#include <tracking/dataobjects/SecMapVector.h>
#include <tracking/dataobjects/VXDTFRawSecMap.h>
#include <gtest/gtest.h>

#include <string>
#include <utility> // std::pair
#include <stdio.h> // remove(file)

#include "TFile.h"
#include "TKey.h"
#include "TCollection.h"

using namespace std;

namespace Belle2 {
  /** command x should exit using B2FATAL. */
#define EXPECT_FATAL(x) EXPECT_EXIT(x,::testing::KilledBySignal(SIGABRT),"");

  /** Testing SecMapVector to be able to store it in a .root file and reopen/rewrite it */
  class SecMapVectorTest : public ::testing::Test {
  protected:
  };

  /** Filling and reading SecMapVector in/from root file */
  TEST_F(SecMapVectorTest, readAndWriteTObjectToRootFile)
  {
    /// create SecMapVector, fill with some fake VXDTFTemporarySecMaps (setting only the name to check whether everything went fine or not)
    B2INFO("creating a secMap named aSecMap:")
    SecMapVector aSecMapVector;
    VXDTFRawSecMap aSecMap;
    aSecMap.setMapName(string("aSecMap"));
    SecMapVector::MapPack firstPack = make_pair(string("aSecMap"), aSecMap);
    aSecMapVector.push_back(firstPack);
    B2INFO("creating a secMap named anotherSecMap:")
    VXDTFRawSecMap anotherSecMap;
    anotherSecMap.setMapName(string("anotherSecMap"));
    SecMapVector::MapPack secondPack = make_pair(string("anotherSecMap"), anotherSecMap);
    aSecMapVector.push_back(secondPack);

    /// create root file, fill SecMapVector into file, store it, close
    B2INFO("opening root file:");
    TFile f("demo.root", "recreate");
    B2INFO("storing data to root file:")
    aSecMapVector.Write();
    f.Close();

    /// repeat first step to check, whether files can be filled repeatedly
    B2INFO("creating a secMap named aThirdSecMap:")
    SecMapVector aSecMapVector2;
    VXDTFRawSecMap aThirdSecMap;
    aThirdSecMap.setMapName(string("aThirdSecMap"));
    SecMapVector::MapPack thirdPack = make_pair(string("aThirdSecMap"), aThirdSecMap);
    aSecMapVector2.push_back(thirdPack);
    B2INFO("creating a secMap named aFourthSecMap:")
    VXDTFRawSecMap aFourthSecMap;
    aFourthSecMap.setMapName(string("aFourthSecMap"));
    SecMapVector::MapPack fourthPack = make_pair(string("aFourthSecMap"), aFourthSecMap);
    aSecMapVector2.push_back(fourthPack);

    /// reopen root file, fill SecMapVector into file, store it, close
    B2INFO("opening root file:");
    TFile f3("demo.root", "update");
    B2INFO("storing data to root file:")
    aSecMapVector2.Write();
    f3.Close();


    /// reopen file, read entries and check them
    TFile f2("demo.root");
    if (f2.IsZombie()) { B2ERROR("file could not be reopened!"); }
    else {
      SecMapVector* retrievedVector;
      f2.GetListOfKeys()->Print();

      TIter next(f2.GetListOfKeys());
      TKey* key;
      while ((key = (TKey*)next())) {

        try {
          retrievedVector = dynamic_cast<SecMapVector*>(key->ReadObj());
        } catch (exception& e) {
          B2WARNING("Key was not a SecMapVector, therefore error message: " << e.what() << "\n Skipping this key...")
          continue;
        }

        for (auto entry : retrievedVector->getFullVector()) {
          B2INFO(".first is: " << entry.first << ", .second->getMapName() is:" << entry.second.getMapName())
        }
      }
      f2.Close();
    }

    /// delete file from disk
    if (remove("demo.root") != 0)
    { B2ERROR("could not delete file!") }
    else
    { B2INFO("File successfully deleted") }

//     EXPECT_DOUBLE_EQ(2.329208242689889, aFilter.calcCircleDist2IP());
  }

}  // namespace
