/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <hlt/softwaretrigger/core/SoftwareTriggerDBHandler.h>
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
      TestHelpers::TempDirCreator* tmp_dir;

      /// Setup the local DB and the datastore with the event meta data.
      void SetUp()
      {
        tmp_dir = new TestHelpers::TempDirCreator;

        StoreObjPtr<EventMetaData> evtPtr;
        DataStore::Instance().setInitializeActive(true);
        evtPtr.registerInDataStore();
        DataStore::Instance().setInitializeActive(false);
        evtPtr.construct(1, 0, 0);

        LocalDatabase::createInstance(tmp_dir->getTempDir() + "/testPayloads/TestDatabase.txt");
      }

      /// Destroy the DB and the DataStore.
      void TearDown()
      {
        Database::reset();
        DBStore::Instance().reset();
        DataStore::Instance().reset();

        delete tmp_dir;
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
      EXPECT_EQ(SoftwareTriggerCutResult::c_accept,
                cutsWithNames.at("software_trigger_cut_test_cutOne")->checkPreScaled(preFilledObject));
      EXPECT_THROW(cutsWithNames.at("software_trigger_cut_test_cutTwo"), std::out_of_range);

      // Create the missing cut
      const auto& cutTwo = SoftwareTriggerCut::compile("1 == 2", 1);

      // Upload the second cut
      SoftwareTriggerDBHandler::upload(cutTwo, "test", "cutTwo", iov);

      // Initialize with both cuts uploaded (should not fail)
      dbHandler.initialize("test", {"cutOne", "cutTwo"});

      // Try out both cuts
      EXPECT_EQ(SoftwareTriggerCutResult::c_accept,
                cutsWithNames.at("software_trigger_cut_test_cutOne")->checkPreScaled(preFilledObject));
      EXPECT_EQ(SoftwareTriggerCutResult::c_noResult,
                cutsWithNames.at("software_trigger_cut_test_cutTwo")->checkPreScaled(preFilledObject));

    }

    /** Python down and upload functions */
    TEST_F(SoftwareTriggerDBHandlerTest, pythonUpAndDownload)
    {
      IntervalOfValidity iov(0, 0, -1, -1);

      // Create a new cut.
      auto cutOne = SoftwareTriggerCut::compile("1 == 1", 1);

      SoftwareTriggerDBHandler::upload(cutOne, "test", "cutOne1", iov);
      auto downloadedCutOne = SoftwareTriggerDBHandler::download("test", "cutOne1");

      ASSERT_NE(downloadedCutOne, nullptr);

      EXPECT_EQ(cutOne->decompile(), downloadedCutOne->decompile());
      EXPECT_EQ(cutOne->getPreScaleFactor(), downloadedCutOne->getPreScaleFactor());
      EXPECT_EQ(cutOne->isRejectCut(), downloadedCutOne->isRejectCut());

      // Create a second cut.
      const auto& cutTwo = SoftwareTriggerCut::compile("1 == 2", 1, true);

      SoftwareTriggerDBHandler::upload(cutTwo, "test", "cutTwo2", iov);
      const auto& downloadedCutTwo = SoftwareTriggerDBHandler::download("test", "cutTwo2");

      ASSERT_NE(downloadedCutTwo, nullptr);

      EXPECT_EQ(cutTwo->decompile(), downloadedCutTwo->decompile());
      EXPECT_EQ(cutTwo->getPreScaleFactor(), downloadedCutTwo->getPreScaleFactor());
      EXPECT_EQ(cutTwo->isRejectCut(), downloadedCutTwo->isRejectCut());
    }
  }
}
