/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <mdst/dbobjects/DBRepresentationOfSoftwareTriggerCut.h>
#include <hlt/softwaretrigger/core/SoftwareTriggerCut.h>
#include <hlt/softwaretrigger/core/SoftwareTriggerDBHandler.h>
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
      }

      /// Destroy the DB and the DataStore.
      void TearDown()
      {
        boost::filesystem::remove_all("testPayloads");
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
