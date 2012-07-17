#include <framework/dataobjects/EventMetaData.h>
#include <framework/dataobjects/RunMetaData.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>

#include <gtest/gtest.h>
#include <boost/foreach.hpp>

using namespace std;

namespace Belle2 {
#define EXPECT_FATAL(x) EXPECT_EXIT(x,::testing::KilledBySignal(SIGABRT),"");

  /** Set up a few arrays and objects in the datastore */
  class DataStoreTest : public ::testing::Test {
  protected:
    /** fill StoreArrays with entries from 0..9 */
    virtual void SetUp() {
      StoreObjPtr<EventMetaData> evtPtr;
      evtPtr->setEvent(42);

      StoreArray<EventMetaData> evtData;
      StoreArray<EventMetaData> evtDataDifferentName("EventMetaDatas_2");
      StoreArray<EventMetaData> evtDataDifferentDurability("", DataStore::c_Run);
      StoreArray<RunMetaData> runData;

      RunMetaData runmetadataobject(62.5, 62.5);
      for (int i = 0; i < 10; ++i) {
        EventMetaData* newobj;
        newobj = evtData.appendNew();
        newobj->setEvent(10 + i);
        newobj = evtDataDifferentName.appendNew();
        newobj->setEvent(20 + i);
        newobj = new(evtDataDifferentDurability.nextFreeAddress()) EventMetaData(30 + i);

        //copy-construct RunMetaData objects
        new(runData.nextFreeAddress()) RunMetaData(runmetadataobject);
      }
    }

    /** clear datastore */
    virtual void TearDown() {
      for (int i = 0; i < DataStore::c_NDurabilityTypes; ++i) {
        DataStore::Instance().clearMaps((DataStore::EDurability) i);
      }
    }

  };

  /** Tests the creation of arrays/objects and wether they're attached correctly. */
  TEST_F(DataStoreTest, AttachTest)
  {
    StoreObjPtr<EventMetaData> evtPtr;
    EXPECT_TRUE(evtPtr);

    StoreArray<EventMetaData> evtData;
    EXPECT_TRUE(evtData);
    StoreArray<EventMetaData> evtDataDifferentName("EventMetaDatas_2");
    EXPECT_TRUE(evtDataDifferentName);
    StoreArray<EventMetaData> evtDataDifferentDurability("", DataStore::c_Run);
    EXPECT_TRUE(evtDataDifferentDurability);
    StoreArray<RunMetaData> runData;
    EXPECT_TRUE(runData);
  }

  /** Test attaching with different types */
  TEST_F(DataStoreTest, TypeTest)
  {
    //attach with incompatible type
    EXPECT_FATAL(StoreArray<RunMetaData>("EventMetaDatas"));
    EXPECT_FATAL(StoreObjPtr<RunMetaData>("EventMetaData"));

    //attaching objects to array and vice versa shouldn't work
    //neither should the store allow objects with same name/durability
    //as existing arrays
    EXPECT_FATAL(StoreArray<EventMetaData>("EventMetaData"));
    EXPECT_FATAL(StoreObjPtr<EventMetaData>("EventMetaDatas"));

    //getting a base class is OK
    EXPECT_TRUE(StoreArray<TObject>("EventMetaDatas"));
    EXPECT_TRUE(StoreObjPtr<TObject>("EventMetaData"));
  }

  /** check meta data. */
  TEST_F(DataStoreTest, MetaDataTest)
  {
    StoreObjPtr<EventMetaData> evtPtr;
    EXPECT_EQ(evtPtr.getDurability(), DataStore::c_Event);
    EXPECT_TRUE(evtPtr.getName() == "EventMetaData");

    StoreArray<EventMetaData> evtData;
    StoreArray<EventMetaData> evtData2("EventMetaDatas");
    StoreArray<EventMetaData> evtDataDifferentName("EventMetaDatas_2");
    StoreArray<EventMetaData> evtDataDifferentDurability("", DataStore::c_Run);
    EXPECT_TRUE(evtData.getName() == "EventMetaDatas");
    EXPECT_EQ(evtData.getDurability(), DataStore::c_Event);
    EXPECT_TRUE(evtData == evtData2);
    EXPECT_FALSE(evtData != evtData2);
    EXPECT_FALSE(evtData == evtDataDifferentName);
    EXPECT_FALSE(evtData == evtDataDifferentDurability);

    //different type
    StoreArray<RunMetaData> runData;
    EXPECT_FALSE(runData == evtData);
    EXPECT_TRUE(runData != evtData);
    //note: EXPECT_NE/EQ do things I don't understand. this doesn't work:
    //EXPECT_NE(runData, evtData);
  }

  /** check read-only attaching for StoreObjPtrs */
  TEST_F(DataStoreTest, ReadOnlyAttach)
  {
    StoreObjPtr<EventMetaData> a("", DataStore::c_Event, false);
    EXPECT_TRUE(a);
    EXPECT_EQ(a->getEvent(), (unsigned long)42);
    StoreObjPtr<EventMetaData> b("nonexisting", DataStore::c_Event, false);
    EXPECT_FALSE(b);
    StoreObjPtr<EventMetaData> c("", DataStore::c_Run, false);
    EXPECT_FALSE(c);
    StoreObjPtr<EventMetaData> d("", DataStore::c_Persistent, false);
    EXPECT_FALSE(d);

    //check we didn't insert a new object with 'd'
    StoreObjPtr<EventMetaData> e("", DataStore::c_Persistent, false);
    EXPECT_FALSE(e);
  }

  /** read data */
  TEST_F(DataStoreTest, VerifyContents)
  {
    StoreObjPtr<EventMetaData> evtPtr;
    EXPECT_EQ(evtPtr->getEvent(), (unsigned long)42);

    StoreArray<EventMetaData> evtData;
    StoreArray<EventMetaData> evtDataDifferentName("EventMetaDatas_2");
    StoreArray<EventMetaData> evtDataDifferentDurability("", DataStore::c_Run);
    StoreArray<RunMetaData> runData;
    EXPECT_EQ(evtData.getEntries(), 10);
    EXPECT_EQ(evtDataDifferentName.getEntries(), 10);
    EXPECT_EQ(evtDataDifferentDurability.getEntries(), 10);
    EXPECT_EQ(runData.getEntries(), 10);
    for (int i = 0; i < 10; ++i) {
      EXPECT_EQ((int)evtData[i]->getEvent(), 10 + i);
      EXPECT_EQ((int)evtDataDifferentName[i]->getEvent(), 20 + i);
      EXPECT_EQ((int)evtDataDifferentDurability[i]->getEvent(), 30 + i);

      float energy = runData[i]->getEnergyLER() + runData[i]->getEnergyHER();
      EXPECT_FLOAT_EQ(energy, 125.0);
    }
  }

  /** check TClonesArray consistency (i.e. no gaps) */
  TEST_F(DataStoreTest, ArrayConsistency)
  {
    StoreArray<EventMetaData> evtData;
    StoreArray<EventMetaData> evtDataDifferentName("EventMetaDatas_2");
    StoreArray<EventMetaData> evtDataDifferentDurability("", DataStore::c_Run);
    StoreArray<RunMetaData> runData;
    EXPECT_EQ((int)evtData.getPtr()->GetEntries(), 10);
    EXPECT_EQ((int)evtDataDifferentName.getPtr()->GetEntries(), 10);
    EXPECT_EQ((int)evtDataDifferentDurability.getPtr()->GetEntries(), 10);
    EXPECT_EQ((int)runData.getPtr()->GetEntries(), 10);
  }

  /** check out-of-bounds behaviour for arrays */
  TEST_F(DataStoreTest, RangeCheck)
  {
    StoreArray<EventMetaData> evtData;
    EXPECT_TRUE(evtData[0] != NULL);
    EXPECT_TRUE(evtData[-1] == NULL);
    EXPECT_TRUE(evtData[10] == NULL);
  }

  /** check data store clearing */
  TEST_F(DataStoreTest, ClearMaps)
  {
    StoreObjPtr<EventMetaData> evtPtr;
    EXPECT_EQ(evtPtr->getEvent(), (unsigned long)42);

    //clear event map (evtPtr is now invalid!)
    DataStore::Instance().clearMaps(DataStore::c_Event);

    //right now this is NULL, but it's not actually clearly defined...
    StoreObjPtr<EventMetaData> a("", DataStore::c_Event, false);
    EXPECT_FALSE(a);

    //should be a default constructed object
    StoreObjPtr<EventMetaData> b;
    EXPECT_TRUE(*b == EventMetaData());


    //cleared arrays must be empty
    StoreArray<EventMetaData> evtData;
    StoreArray<EventMetaData> evtDataDifferentName("EventMetaDatas_2");
    StoreArray<EventMetaData> evtDataDifferentDurability("", DataStore::c_Run);
    StoreArray<RunMetaData> runData;
    EXPECT_EQ(evtData.getEntries(), 0);
    EXPECT_EQ(evtDataDifferentName.getEntries(), 0);
    EXPECT_EQ(runData.getEntries(), 0);

    //run durability, should be unaffected
    EXPECT_EQ(evtDataDifferentDurability.getEntries(), 10);
  }

}  // namespace
