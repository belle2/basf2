#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>
#include <tracking/dataobjects/Track.h>
#include <tracking/dataobjects/TrackFitResult.h>
#include <framework/datastore/StoreArray.h>
#include <gtest/gtest.h>

using namespace std;

namespace Belle2 {
#define EXPECT_FATAL(x) EXPECT_EXIT(x,::testing::KilledBySignal(SIGABRT),"");

  /** Test class for the Track object.
   */
  class TrackTest : public ::testing::Test {
  protected:
  };

  /** Test simple Setters and Getters.
   */
  TEST_F(TrackTest, settersNGetters)
  {
    //Create some TrackFitResults in the the DataStore.
    //PDGCode of the TrackFitResult will be used in the test to identify the TFR.
    DataStore::Instance().setInitializeActive(true);
    StoreArray<TrackFitResult>::registerPersistent();
    StoreArray<TrackFitResult> myResults;

    TrackFitResult result1;
    result1.setParticleType(Const::pion);
    new(myResults.nextFreeAddress()) TrackFitResult(result1);

    TrackFitResult result2;
    result2.setParticleType(Const::kaon);
    new(myResults.nextFreeAddress()) TrackFitResult(result2);

    TrackFitResult result3;
    result3.setParticleType(Const::electron);
    new(myResults.nextFreeAddress()) TrackFitResult(result3);

    TrackFitResult result4;
    result4.setParticleType(Const::muon);
    new(myResults.nextFreeAddress()) TrackFitResult(result4);

    Track mytrack1;
    mytrack1.setTrackFitResultIndex(Const::pion, 0);
    mytrack1.setTrackFitResultIndex(Const::muon, 3);
    mytrack1.setTrackFitResultIndex(Const::kaon, 2);

    // If the index of the corresponding particel is set, the correct particle should be returned.
    EXPECT_EQ(mytrack1.getTrackFitResult(Const::pion)->getParticleType(), Const::pion);

    // make some test of the logic of return statements in case the primary hypothesis is not available.
    Track mytrack2;

  }
}  // namespace
