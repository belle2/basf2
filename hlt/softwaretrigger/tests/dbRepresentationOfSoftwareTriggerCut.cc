/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <mdst/dbobjects/DBRepresentationOfSoftwareTriggerCut.h>
#include <hlt/softwaretrigger/core/SoftwareTriggerCut.h>
#include <hlt/softwaretrigger/core/SoftwareTriggerDBHandler.h>
#include <framework/database/LocalDatabase.h>
#include <boost/filesystem.hpp>

#include <framework/utilities/TestHelpers.h>
#include <gtest/gtest.h>

using namespace std;

namespace Belle2 {
  namespace SoftwareTrigger {

    /// Class to test the db representation of the cuts.
    class DBRepresentationOfSoftwareTriggerCutTest : public ::testing::Test {

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

    /** Basic representation */
    TEST_F(DBRepresentationOfSoftwareTriggerCutTest, basic)
    {
      auto cut = SoftwareTriggerCut::compile("1 == 2", 10, true);
      DBRepresentationOfSoftwareTriggerCut representation(cut->getPreScaleFactor(), cut->isRejectCut(), cut->decompile());
      auto returnedCut = SoftwareTriggerDBHandler::createCutFromDB(representation);

      EXPECT_EQ(cut->decompile(), returnedCut->decompile());
      EXPECT_EQ(cut->getPreScaleFactor(), returnedCut->getPreScaleFactor());
      EXPECT_EQ(cut->isRejectCut(), returnedCut->isRejectCut());

      auto cut2 = SoftwareTriggerCut::compile("1 == 2", 10, false);
      DBRepresentationOfSoftwareTriggerCut representation2(cut2->getPreScaleFactor(), cut2->isRejectCut(), cut2->decompile());
      auto returnedCut2 = SoftwareTriggerDBHandler::createCutFromDB(representation2);

      EXPECT_EQ(cut2->decompile(), returnedCut2->decompile());
      EXPECT_EQ(cut2->getPreScaleFactor(), returnedCut2->getPreScaleFactor());
      EXPECT_EQ(cut2->isRejectCut(), returnedCut2->isRejectCut());
    }
  }
}
