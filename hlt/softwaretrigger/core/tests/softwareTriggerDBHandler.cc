/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <hlt/softwaretrigger/core/SoftwareTriggerDBHandler.h>
#include <framework/database/Configuration.h>
#include <framework/database/Database.h>

#include <framework/utilities/TestHelpers.h>
#include <gtest/gtest.h>

using namespace std;

namespace Belle2 {
  namespace SoftwareTrigger {

    /// Class to test the down- and upload of trigger cuts to the DB.
    class SoftwareTriggerDBHandlerTest : public ::testing::Test {
      /// Do everything in a temporary dir.
      TestHelpers::TempDirCreator* m_tmpDir = nullptr;

      /// Setup the local DB and the datastore with the event meta data.
      void SetUp()
      {
        m_tmpDir = new TestHelpers::TempDirCreator;

        StoreObjPtr<EventMetaData> evtPtr;
        DataStore::Instance().setInitializeActive(true);
        evtPtr.registerInDataStore();
        DataStore::Instance().setInitializeActive(false);
        evtPtr.construct(1, 0, 0);

        auto& conf = Conditions::Configuration::getInstance();
        conf.setNewPayloadLocation(m_tmpDir->getTempDir() + "/testPayloads/TestDatabase.txt");
        conf.prependTestingPayloadLocation(m_tmpDir->getTempDir() + "/testPayloads/TestDatabase.txt");
      }

      /// Destroy the DB and the DataStore.
      void TearDown()
      {
        Database::reset();
        DBStore::Instance().reset();
        DataStore::Instance().reset();

        delete m_tmpDir;
      }
    };

    /** Download from the DB */
    TEST_F(SoftwareTriggerDBHandlerTest, downloadAndChanged)
    {
      SoftwareTriggerObject preFilledObject;

      IntervalOfValidity iov(0, 0, -1, -1);

      // Create a new cut.
      const auto& cutOne = SoftwareTriggerCut::compile("1 == 1", 1);

      // Upload the first cut
      SoftwareTriggerDBHandler::upload(cutOne, "test", "cutOne", iov);

      // Try to download a missing cut.
      SoftwareTriggerDBHandler::uploadTriggerMenu("test", {"cutOne", "cutTwo"}, true, iov);
      EXPECT_B2FATAL(SoftwareTriggerDBHandler("test"));

      // Download and test the single uploaded cut.
      SoftwareTriggerDBHandler::uploadTriggerMenu("test", {"cutOne"}, true, iov);

      SoftwareTriggerDBHandler dbHandler("test");
      const auto& cutsWithNames = dbHandler.getCutsWithNames();
      EXPECT_EQ(SoftwareTriggerCutResult::c_accept,
                cutsWithNames.at("software_trigger_cut&test&cutOne")->checkPreScaled(preFilledObject));
      EXPECT_THROW(cutsWithNames.at("software_trigger_cut&test&cutTwo"), std::out_of_range);

      // Create the missing cut
      const auto& cutTwo = SoftwareTriggerCut::compile("1 == 2", 1);

      // Upload the second cut
      SoftwareTriggerDBHandler::upload(cutOne, "test2", "cutOne", iov);
      SoftwareTriggerDBHandler::upload(cutTwo, "test2", "cutTwo", iov);

      // Initialize with both cuts uploaded (should not fail)
      SoftwareTriggerDBHandler::uploadTriggerMenu("test2", {"cutOne", "cutTwo"}, true, iov);

      SoftwareTriggerDBHandler otherDBHandler("test2");
      const auto& otherCutsWithNames = otherDBHandler.getCutsWithNames();
      // Try out both cuts
      EXPECT_EQ(SoftwareTriggerCutResult::c_accept,
                otherCutsWithNames.at("software_trigger_cut&test2&cutOne")->checkPreScaled(preFilledObject));
      EXPECT_EQ(SoftwareTriggerCutResult::c_noResult,
                otherCutsWithNames.at("software_trigger_cut&test2&cutTwo")->checkPreScaled(preFilledObject));

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
