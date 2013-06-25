#include <framework/dataobjects/EventMetaData.h>
#include <framework/dataobjects/ProfileInfo.h>
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
      DataStore::Instance().setInitializeActive(true);
      StoreObjPtr<EventMetaData>::registerPersistent();
      StoreArray<EventMetaData>::registerPersistent();
      StoreArray<EventMetaData>::registerPersistent("EventMetaDatas_2");
      StoreArray<EventMetaData>::registerPersistent("", DataStore::c_Persistent);
      StoreArray<ProfileInfo>::registerPersistent();
      DataStore::Instance().setInitializeActive(false);

      StoreObjPtr<EventMetaData> evtPtr;
      evtPtr.create();
      evtPtr->setEvent(42);

      StoreArray<EventMetaData> evtData;
      StoreArray<EventMetaData> evtDataDifferentName("EventMetaDatas_2");
      evtDataDifferentName.create(); //StoreArrays can be explicitly created (can also be omitted)
      StoreArray<EventMetaData> evtDataDifferentDurability("", DataStore::c_Persistent);
      evtDataDifferentDurability.create();
      StoreArray<ProfileInfo> profileInfo;
      profileInfo.create();

      ProfileInfo profileInfoObject(128, 60.0);
      for (int i = 0; i < 10; ++i) {
        EventMetaData* newobj;
        newobj = evtData.appendNew();
        newobj->setEvent(10 + i);
        newobj = evtDataDifferentName.appendNew();
        newobj->setEvent(20 + i);
        newobj = new(evtDataDifferentDurability.nextFreeAddress()) EventMetaData(30 + i);

        //copy-construct ProfileInfo objects
        new(profileInfo.nextFreeAddress()) ProfileInfo(profileInfoObject);
      }
    }

    /** clear datastore */
    virtual void TearDown() {
      for (int i = 0; i < DataStore::c_NDurabilityTypes; ++i) {
        DataStore::Instance().clearMaps((DataStore::EDurability) i);
        const DataStore::StoreObjMap& map = DataStore::Instance().getStoreObjectMap(DataStore::EDurability(i));
        const_cast<DataStore::StoreObjMap&>(map).clear();
      }
    }

  };

  /** Tests the creation of arrays/objects and whether they're attached correctly. */
  TEST_F(DataStoreTest, AttachTest)
  {
    StoreObjPtr<EventMetaData> evtPtr;
    EXPECT_TRUE(evtPtr);

    StoreArray<EventMetaData> evtData;
    EXPECT_TRUE(evtData);
    StoreArray<EventMetaData> evtDataDifferentName("EventMetaDatas_2");
    EXPECT_TRUE(evtDataDifferentName);
    StoreArray<EventMetaData> evtDataDifferentDurability("", DataStore::c_Persistent);
    EXPECT_TRUE(evtDataDifferentDurability);
    StoreArray<ProfileInfo> profileInfo;
    EXPECT_TRUE(profileInfo);
  }

  /** Test attaching with different types */
  TEST_F(DataStoreTest, TypeTest)
  {
    //attach with incompatible type
    EXPECT_FATAL(StoreArray<ProfileInfo>("EventMetaDatas").isValid());
    EXPECT_FATAL(StoreObjPtr<ProfileInfo>("EventMetaData").isValid());

    //attaching objects to array and vice versa shouldn't work
    //neither should the store allow objects with same name/durability
    //as existing arrays
    EXPECT_FATAL(StoreArray<EventMetaData>("EventMetaData").isValid());
    EXPECT_FATAL(StoreObjPtr<EventMetaData>("EventMetaDatas").isValid());

    //getting a base class is OK
    EXPECT_TRUE(StoreArray<TObject>("EventMetaDatas").isValid());
    EXPECT_TRUE(StoreObjPtr<TObject>("EventMetaData").isValid());
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
    StoreArray<EventMetaData> evtDataDifferentDurability("", DataStore::c_Persistent);
    EXPECT_TRUE(evtData.getName() == "EventMetaDatas");
    EXPECT_EQ(evtData.getDurability(), DataStore::c_Event);
    EXPECT_TRUE(evtData == evtData2);
    EXPECT_FALSE(evtData != evtData2);
    EXPECT_FALSE(evtData == evtDataDifferentName);
    EXPECT_FALSE(evtData == evtDataDifferentDurability);

    //different type
    StoreArray<ProfileInfo> profileInfo;
    EXPECT_FALSE(profileInfo == evtData);
    EXPECT_TRUE(profileInfo != evtData);
    //note: EXPECT_NE/EQ do things I don't understand. this doesn't work:
    //EXPECT_NE(profileInfo, evtData);
  }

  /** check read-only attaching for StoreObjPtrs */
  TEST_F(DataStoreTest, ReadOnlyAttach)
  {
    StoreObjPtr<EventMetaData> a("", DataStore::c_Event);
    EXPECT_TRUE(a);
    EXPECT_EQ(a->getEvent(), (unsigned long)42);
    StoreObjPtr<EventMetaData> b("nonexisting", DataStore::c_Event);
    EXPECT_FALSE(b);
    StoreObjPtr<EventMetaData> c("", DataStore::c_Persistent);
    EXPECT_FALSE(c);
    StoreObjPtr<EventMetaData> d("", DataStore::c_Persistent);
    EXPECT_FALSE(d);

    //check we didn't insert a new object with 'd'
    StoreObjPtr<EventMetaData> e("", DataStore::c_Persistent);
    EXPECT_FALSE(e);
  }

  /** read data */
  TEST_F(DataStoreTest, VerifyContents)
  {
    StoreObjPtr<EventMetaData> evtPtr;
    EXPECT_EQ(evtPtr->getEvent(), (unsigned long)42);

    StoreArray<EventMetaData> evtData;
    StoreArray<EventMetaData> evtDataDifferentName("EventMetaDatas_2");
    StoreArray<EventMetaData> evtDataDifferentDurability("", DataStore::c_Persistent);
    StoreArray<ProfileInfo> profileInfo;
    EXPECT_EQ(evtData.getEntries(), 10);
    EXPECT_EQ(evtDataDifferentName.getEntries(), 10);
    EXPECT_EQ(evtDataDifferentDurability.getEntries(), 10);
    EXPECT_EQ(profileInfo.getEntries(), 10);
    for (int i = 0; i < 10; ++i) {
      EXPECT_EQ((int)evtData[i]->getEvent(), 10 + i);
      EXPECT_EQ((int)evtDataDifferentName[i]->getEvent(), 20 + i);
      EXPECT_EQ((int)evtDataDifferentDurability[i]->getEvent(), 30 + i);

      EXPECT_EQ(profileInfo[i]->getMemory(), 128u);
      EXPECT_FLOAT_EQ(profileInfo[i]->getTimeInSec(), 60.0);
    }
  }

  /** check TClonesArray consistency (i.e. no gaps) */
  TEST_F(DataStoreTest, ArrayConsistency)
  {
    StoreArray<EventMetaData> evtData;
    StoreArray<EventMetaData> evtDataDifferentName("EventMetaDatas_2");
    StoreArray<EventMetaData> evtDataDifferentDurability("", DataStore::c_Persistent);
    StoreArray<ProfileInfo> profileInfo;
    EXPECT_EQ((int)evtData.getPtr()->GetEntries(), 10);
    EXPECT_EQ((int)evtDataDifferentName.getPtr()->GetEntries(), 10);
    EXPECT_EQ((int)evtDataDifferentDurability.getPtr()->GetEntries(), 10);
    EXPECT_EQ((int)profileInfo.getPtr()->GetEntries(), 10);
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

    //right now this is NULL, since no object was actually created yet
    StoreObjPtr<EventMetaData> a;
    EXPECT_FALSE(a);

    StoreObjPtr<EventMetaData> b;
    EXPECT_FALSE(b);

    //create() should produce a default constructed object
    b.create();
    EXPECT_TRUE(*b == EventMetaData());
    //since a should attach to same object...
    EXPECT_TRUE(*a == *b);

    a->setEvent(42);
    //don't replace existing object
    a.create(false);
    EXPECT_EQ(a->getEvent(), (unsigned long)42);
    //replace existing object
    a.create(true);
    EXPECT_NE(a->getEvent(), (unsigned long)42);


    //cleared arrays must be empty
    StoreArray<EventMetaData> evtData;
    StoreArray<EventMetaData> evtDataDifferentName("EventMetaDatas_2");
    StoreArray<ProfileInfo> profileInfo;
    EXPECT_EQ(evtData.getEntries(), 0);
    EXPECT_EQ(evtDataDifferentName.getEntries(), 0);
    EXPECT_EQ(profileInfo.getEntries(), 0);

    //run durability, should be unaffected
    StoreArray<EventMetaData> evtDataDifferentDurability("", DataStore::c_Persistent);
    EXPECT_EQ(evtDataDifferentDurability.getEntries(), 10);
  }

  /** check required() functionality */
  TEST_F(DataStoreTest, RequireObjects)
  {
    EXPECT_TRUE(StoreObjPtr<EventMetaData>::required());
    EXPECT_FALSE(StoreObjPtr<EventMetaData>::required("", DataStore::c_Persistent));
    EXPECT_FALSE(StoreObjPtr<EventMetaData>::required("nonexisting2"));
    EXPECT_FALSE(StoreObjPtr<EventMetaData>::required("", DataStore::c_Persistent));
    //check we didn't create one...
    EXPECT_FALSE(StoreObjPtr<EventMetaData>::required("nonexisting2"));

    EXPECT_TRUE(StoreArray<EventMetaData>::required());
    EXPECT_TRUE(StoreArray<EventMetaData>::required("EventMetaDatas_2"));
    EXPECT_TRUE(StoreArray<EventMetaData>::required("", DataStore::c_Persistent));
    EXPECT_FALSE(StoreArray<EventMetaData>::required("blah"));
    EXPECT_FALSE(StoreArray<EventMetaData>::required("blah"));
    EXPECT_TRUE(StoreArray<ProfileInfo>::required());
    //check we didn't create one...
    EXPECT_FALSE(StoreArray<EventMetaData>::required("blah"));
  }

  /** Test iteration. */
  TEST_F(DataStoreTest, StoreArrayIteration)
  {
    const StoreArray<EventMetaData> evtData;
    //array syntax
    for (int i = 0; i < evtData.getEntries(); i++) {
      EXPECT_TRUE(evtData[i] != NULL);
    }

    //const_iterator
    int i = 0;
    BOOST_FOREACH(const EventMetaData & emd, evtData) {
      EXPECT_TRUE(&emd == evtData[i]);
      i++;
    }
    EXPECT_EQ(i, evtData.getEntries());


    //iterator
    StoreArray<EventMetaData> evtDataNonConst;
    i = 0;
    BOOST_FOREACH(EventMetaData & emd, evtDataNonConst) {
      EXPECT_TRUE(&emd == evtData[i]);
      i++;
    }
    EXPECT_EQ(i, evtData.getEntries());

    i = 0;
    for (StoreArray<EventMetaData>::iterator it = evtDataNonConst.begin(); it != evtDataNonConst.end(); ++it) {
      EXPECT_TRUE(&(*it) == evtData[i]);
      EXPECT_EQ(it->getEvent(), 10 + i);
      i++;
    }
    EXPECT_EQ(i, evtData.getEntries());

    /*
        //range-based for
        i = 0;
        for (EventMetaData& emd : evtDataNonConst) {
          EXPECT_TRUE(&emd == evtData[i]);
          i++;
        }
        EXPECT_EQ(i, evtData.getEntries());
        */
  }

  /** test registerPersistent(), optional() */
  TEST_F(DataStoreTest, DataStoreRegistration)
  {
    StoreObjPtr<EventMetaData> evtPtr("abc123");
    StoreArray<EventMetaData>  evtArray("abc123array");

    //verify that they aren't registered right now
    EXPECT_FALSE(evtPtr.isOptional());
    EXPECT_FALSE(evtArray.isOptional());
    EXPECT_FALSE(evtPtr.isRequired());
    EXPECT_FALSE(evtArray.isRequired());
    EXPECT_FALSE(StoreObjPtr<EventMetaData>::optional(evtPtr.getName()));
    EXPECT_FALSE(StoreArray<EventMetaData>::optional(evtArray.getName()));
    EXPECT_FALSE(StoreObjPtr<EventMetaData>::required(evtPtr.getName()));
    EXPECT_FALSE(StoreArray<EventMetaData>::required(evtArray.getName()));

    DataStore::Instance().setInitializeActive(true);
    EXPECT_TRUE(evtPtr.registerAsPersistent());
    EXPECT_TRUE(evtArray.registerAsTransient());

    //already registered, ok by default
    EXPECT_TRUE(evtPtr.registerAsPersistent());
    EXPECT_TRUE(evtArray.registerAsTransient());

    //test errorIfExisting
    EXPECT_FALSE(evtPtr.registerAsPersistent(true));
    EXPECT_FALSE(evtArray.registerAsTransient(true));
    DataStore::Instance().setInitializeActive(false);

    //now they should be available:
    EXPECT_TRUE(evtPtr.isOptional());
    EXPECT_TRUE(evtArray.isOptional());
    EXPECT_TRUE(evtPtr.isRequired());
    EXPECT_TRUE(evtArray.isRequired());
    EXPECT_TRUE(StoreObjPtr<EventMetaData>::optional(evtPtr.getName()));
    EXPECT_TRUE(StoreArray<EventMetaData>::optional(evtArray.getName()));
    EXPECT_TRUE(StoreObjPtr<EventMetaData>::required(evtPtr.getName()));
    EXPECT_TRUE(StoreArray<EventMetaData>::required(evtArray.getName()));
  }

}  // namespace
