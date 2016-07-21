/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <skim/softwaretrigger/core/SoftwareTriggerDBHandler.h>
#include <framework/database/LocalDatabase.h>
#include <framework/database/DBImportObjPtr.h>
#include <boost/filesystem.hpp>

#include <framework/utilities/TestHelpers.h>
#include <gtest/gtest.h>

using namespace std;

namespace Belle2 {
  namespace SoftwareTrigger {

    /// Class to test the down- and upload of trigger cuts to the DB.
    class SoftwareTriggerDBHandlerTest : public ::testing::Test {

      /// Do everything in a temporary dir.
      TestHelpers::TempDirCreator tmp_dir;

      /// Setup the local DB and the datastore with the event meta data.
      void SetUp()
      {
        StoreObjPtr<EventMetaData> evtPtr;
        DataStore::Instance().setInitializeActive(true);
        evtPtr.registerInDataStore();
        DataStore::Instance().setInitializeActive(false);
        evtPtr.construct(0, 0, 0);

        LocalDatabase::createInstance("testPayloads/TestDatabase.txt");
      }

      /// Destroy the DB and the DataStore.
      void TearDown()
      {
        boost::filesystem::remove_all("testPayloads");
        Database::reset();
        DataStore::Instance().reset();
      }
    };

    /** Download from the DB */
    TEST_F(SoftwareTriggerDBHandlerTest, downloadAndChanged)
    {
      SoftwareTriggerDBHandler dbHandler;
      SoftwareTriggerObject preFilledObject;

      IntervalOfValidity iov(0, 0, -1, -1);

      // Create a new cut.
      const auto& cutOne = SoftwareTriggerCut::compile("1 == 1", 1);

      // Upload the first cut
      SoftwareTriggerDBHandler::upload(cutOne, "test", "cutOne", iov);

      // Try to download a missing cut.
      EXPECT_B2FATAL(dbHandler.initialize("test", {"cutOne", "cutTwo"}));

      // Download and test the single uploaded cut.
      dbHandler.initialize("test", {"cutOne"});
      const auto& cutsWithNames = dbHandler.getCutsWithNames();
      EXPECT_TRUE(cutsWithNames.at("software_trigger_cut_test_cutOne")->checkPreScaled(preFilledObject));
      EXPECT_THROW(cutsWithNames.at("software_trigger_cut_test_cutTwo"), std::out_of_range);

      // Create the missing cut
      const auto& cutTwo = SoftwareTriggerCut::compile("1 == 2", 1);

      // Upload the second cut
      SoftwareTriggerDBHandler::upload(cutTwo, "test", "cutTwo", iov);

      // Initialize with both cuts uploaded (should not fail)
      dbHandler.initialize("test", {"cutOne", "cutTwo"});

      // Try out both cuts
      EXPECT_TRUE(cutsWithNames.at("software_trigger_cut_test_cutOne")->checkPreScaled(preFilledObject));
      EXPECT_FALSE(cutsWithNames.at("software_trigger_cut_test_cutTwo")->checkPreScaled(preFilledObject));

    }
  }
}
