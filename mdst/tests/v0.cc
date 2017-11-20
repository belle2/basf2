#include <mdst/dataobjects/V0.h>
#include <mdst/dataobjects/TrackFitResult.h>
#include <mdst/dataobjects/Track.h>

#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>

#include <utility>

#include <gtest/gtest.h>

using namespace std;

namespace Belle2 {

  /** Set up a few arrays and objects in the datastore */
  class V0Test : public ::testing::Test {
  protected:
    /** register Particle array */
    virtual void SetUp()
    {
      DataStore::Instance().setInitializeActive(true);
      StoreArray<Track> tracks;
      tracks.registerInDataStore();
      StoreArray<TrackFitResult> trackFitResults;
      trackFitResults.registerInDataStore();
      DataStore::Instance().setInitializeActive(false);
    }

    /** clear datastore */
    virtual void TearDown()
    {
      DataStore::Instance().reset();
    }

    /** Default index in constructor if no tracks/tfrs are given */
    short m_default_index = -1;

  };

  /** Test Index Getter. */
  TEST_F(V0Test, IndexGetters)
  {
    V0 myV0;
    EXPECT_EQ(m_default_index, std::get<0>(myV0.getTrackIndices()));
    EXPECT_EQ(m_default_index, std::get<1>(myV0.getTrackIndices()));
    EXPECT_EQ(m_default_index, std::get<0>(myV0.getTrackFitResultIndices()));
    EXPECT_EQ(m_default_index, std::get<1>(myV0.getTrackFitResultIndices()));

  } // Testcases for Index Getter

  /** Test Pointer Getter. */
  TEST_F(V0Test, PointerGetters)
  {
    V0 myV0;
    EXPECT_ANY_THROW(std::get<0>(myV0.getTracks()));
    EXPECT_ANY_THROW(std::get<1>(myV0.getTracks()));
    EXPECT_ANY_THROW(std::get<0>(myV0.getTrackFitResults()));
    EXPECT_ANY_THROW(std::get<1>(myV0.getTrackFitResults()));
  } // Testcases for Pointer Getter

}  // namespace
