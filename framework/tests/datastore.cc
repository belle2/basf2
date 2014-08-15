#include <framework/dataobjects/EventMetaData.h>
#include <framework/dataobjects/ProfileInfo.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/utilities/TestHelpers.h>

#include <gtest/gtest.h>
#include <boost/foreach.hpp>

#include <algorithm>

using namespace std;
using namespace Belle2;

namespace {
  /** Set up a few arrays and objects in the datastore */
  class DataStoreTest : public ::testing::Test {
  protected:
    /** fill StoreArrays with entries from 0..9 */
    virtual void SetUp() {
      StoreObjPtr<EventMetaData> evtPtr;
      StoreArray<EventMetaData> evtData;
      StoreArray<EventMetaData> evtDataDifferentName("EventMetaDatas_2");
      StoreArray<EventMetaData> evtDataDifferentDurability("", DataStore::c_Persistent);
      StoreArray<ProfileInfo> profileInfo;

      DataStore::Instance().setInitializeActive(true);
      evtPtr.registerInDataStore();
      evtData.registerInDataStore();
      evtDataDifferentName.registerInDataStore();
      evtDataDifferentDurability.registerInDataStore();
      profileInfo.registerInDataStore();
      DataStore::Instance().setInitializeActive(false);

      evtPtr.create();
      evtPtr->setEvent(42);

      evtDataDifferentName.create(); //StoreArrays can be explicitly created (can also be omitted)
      evtDataDifferentDurability.create();
      profileInfo.create();

      ProfileInfo profileInfoObject(128, 60.0);
      for (int i = 0; i < 10; ++i) {
        //direct construction
        EventMetaData* newobj = evtData.appendNew();
        newobj->setEvent(10 + i);

        //copy-construction
        EventMetaData newObj2;
        newObj2.setEvent(20 + i);
        evtDataDifferentName.appendNew(newObj2);

        //fancy constructors
        newobj = evtDataDifferentDurability.appendNew(30 + i);

        //copy-construct ProfileInfo objects
        profileInfo.appendNew(profileInfoObject);
      }
    }

    /** clear datastore */
    virtual void TearDown() {
      DataStore::Instance().reset();
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
    EXPECT_B2FATAL(StoreArray<ProfileInfo>("EventMetaDatas").isValid());
    EXPECT_B2FATAL(StoreObjPtr<ProfileInfo>("EventMetaData").isValid());

    //attaching objects to array and vice versa shouldn't work
    //neither should the store allow objects with same name/durability
    //as existing arrays
    EXPECT_B2FATAL(StoreArray<EventMetaData>("EventMetaData").isValid());
    EXPECT_B2FATAL(StoreObjPtr<EventMetaData>("EventMetaDatas").isValid());

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
    EXPECT_FALSE(evtPtr.isArray());
    EXPECT_TRUE(evtPtr.getClass() == EventMetaData::Class());

    StoreArray<EventMetaData> evtData;
    StoreArray<EventMetaData> evtData2("EventMetaDatas");
    StoreArray<EventMetaData> evtDataDifferentName("EventMetaDatas_2");
    StoreArray<EventMetaData> evtDataDifferentDurability("", DataStore::c_Persistent);
    EXPECT_TRUE(evtData.getName() == "EventMetaDatas");
    EXPECT_TRUE(evtData.isArray());
    EXPECT_TRUE(evtData.getClass() == EventMetaData::Class());
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

    //test removing data
    evtData.clear();
    EXPECT_TRUE(evtData.isValid());
    EXPECT_EQ(evtData.getEntries(), 0);
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
    DataStore::Instance().invalidateData(DataStore::c_Event);

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
    StoreArray<EventMetaData> evtDataNonConst;
    //array syntax
    for (int i = 0; i < evtData.getEntries(); i++) {
      EXPECT_TRUE(evtData[i] != NULL);
    }

    //basic iterator features
    {
      //input iterator:
      StoreArray<EventMetaData>::iterator it = evtDataNonConst.begin();
      StoreArray<EventMetaData>::iterator it2(it);
      it2 = it;

      //equality/inequality
      EXPECT_TRUE(it == it2);
      EXPECT_FALSE(it != it2);

      //test postfix
      EXPECT_TRUE((it++) == it2);
      it2++;

      //test prefix
      EXPECT_TRUE(it != (++it2));

      //rvalue deref
      EventMetaData ev = *it;
      it->getEvent();
      it2 = it;

      //output iterator:
      //lvalue deref (just overwrite with same object)
      *it = ev;
      *it++ = ev;

      EXPECT_TRUE(ev == *it2);
      EXPECT_TRUE(it2 != it); //was incremented
      EXPECT_TRUE(ev != *it); //was incremented

      //forward iterator (default constructor and multi-pass):
      StoreArray<EventMetaData>::iterator it3;
      it3 = it;
      ev = *it++;
      EXPECT_TRUE(ev == *it3);
    }

    //algorithm stuff
    {
      StoreArray<EventMetaData> evtDataDifferentName("EventMetaDatas_2");
      EXPECT_EQ((int)evtDataNonConst[0]->getEvent(), 10);
      EXPECT_EQ((int)evtDataDifferentName[0]->getEvent(), 20);
      //swap all members of evtDataNonConst ande evtDataDifferentName (same length!)
      std::swap_ranges(evtDataNonConst.begin(), evtDataNonConst.end(), evtDataDifferentName.begin());
      EXPECT_EQ((int)evtDataDifferentName[9]->getEvent(), 19);
      EXPECT_EQ((int)evtDataNonConst[9]->getEvent(), 29);

      //undo
      std::swap_ranges(evtDataDifferentName.begin(), evtDataDifferentName.end(), evtDataNonConst.begin());
      EXPECT_EQ((int)evtDataNonConst[9]->getEvent(), 19);
      EXPECT_EQ((int)evtDataDifferentName[9]->getEvent(), 29);
    }


    //const_iterator
    int i = 0;
    BOOST_FOREACH(const EventMetaData & emd, evtData) {
      EXPECT_TRUE(&emd == evtData[i]);
      i++;
    }
    EXPECT_EQ(i, evtData.getEntries());


    //iterator
    i = 0;
    BOOST_FOREACH(EventMetaData & emd, evtDataNonConst) {
      EXPECT_TRUE(&emd == evtData[i]);
      i++;
    }
    EXPECT_EQ(i, evtData.getEntries());

    i = 0;
    for (StoreArray<EventMetaData>::iterator it = evtDataNonConst.begin(); it != evtDataNonConst.end(); ++it) {
      EXPECT_TRUE(&(*it) == evtData[i]);
      EXPECT_EQ((int)it->getEvent(), 10 + i);
      i++;
    }
    EXPECT_EQ(i, evtData.getEntries());

    //range-based for
    i = 0;
    for (EventMetaData & emd : evtDataNonConst) {
      EXPECT_TRUE(&emd == evtData[i]);
      i++;
    }
    EXPECT_EQ(i, evtData.getEntries());

    //range-based for (const)
    i = 0;
    for (const EventMetaData & emd : evtData) {
      EXPECT_TRUE(&emd == evtData[i]);
      i++;
    }
    EXPECT_EQ(i, evtData.getEntries());
  }

  /** test registerInDataStore(), optional() */
  TEST_F(DataStoreTest, DataStoreRegistration)
  {
    StoreObjPtr<EventMetaData> evtPtr("abc123");
    StoreArray<EventMetaData> evtArray("abc123array");

    //verify that they aren't registered right now
    EXPECT_FALSE(evtPtr.isOptional());
    EXPECT_FALSE(evtArray.isOptional());
    EXPECT_FALSE(evtPtr.isRequired());
    EXPECT_FALSE(evtArray.isRequired());
    EXPECT_FALSE(StoreObjPtr<EventMetaData>::optional(evtPtr.getName()));
    EXPECT_FALSE(StoreArray<EventMetaData>::optional(evtArray.getName()));
    EXPECT_FALSE(StoreObjPtr<EventMetaData>::required(evtPtr.getName()));
    EXPECT_FALSE(StoreArray<EventMetaData>::required(evtArray.getName()));

    //emulate Module::initialize()
    DataStore::Instance().setInitializeActive(true);
    {
      EXPECT_TRUE(evtPtr.registerInDataStore());
      EXPECT_TRUE(evtArray.registerInDataStore(DataStore::c_DontWriteOut));

      //already registered, ok by default
      EXPECT_TRUE(evtPtr.registerInDataStore());
      EXPECT_TRUE(evtArray.registerInDataStore(DataStore::c_DontWriteOut));

      //test c_ErrorIfAlreadyRegistered (return code=false + B2ERROR)
      EXPECT_B2ERROR(EXPECT_FALSE(evtPtr.registerInDataStore(DataStore::c_ErrorIfAlreadyRegistered)));
      EXPECT_B2ERROR(EXPECT_FALSE(evtArray.registerInDataStore(DataStore::c_DontWriteOut | DataStore::c_ErrorIfAlreadyRegistered)));
    }
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

  TEST_F(DataStoreTest, RegistrationOutsideOfInitializeShouldFail)
  {
    //outside initialize(), registration results in an error
    EXPECT_B2ERROR(StoreArray<EventMetaData>().registerInDataStore("someothernewname"));
    EXPECT_B2ERROR(StoreArray<EventMetaData>().registerInDataStore("someothernewname", DataStore::c_DontWriteOut));
    EXPECT_FALSE(StoreArray<EventMetaData>().isOptional("someothernewname"));

    //accessing unregistered things doesn't work.
    StoreArray<EventMetaData> someothernewname("someothernewname");
    EXPECT_FALSE(someothernewname.isValid());
    //usually one shouldn't call StoreArray::create(), but everything like appendNew() would crash
    EXPECT_B2ERROR(EXPECT_FALSE(someothernewname.create()));
    EXPECT_FALSE(someothernewname.isValid()); //still invalid (as create() failed)
  }

  TEST_F(DataStoreTest, ConstructedBeforeInitializeButWithNonDefaultName)
  {
    //as a class member, the classes get constructed before initialize(), but we may not have the name yet
    StoreArray<EventMetaData> events;
    StoreObjPtr<ProfileInfo> profile;
    StoreArray<EventMetaData> eventsMetaDatas2;

    //inialize(), use names from module paramateres
    DataStore::Instance().setInitializeActive(true);
    {
      EXPECT_TRUE(events.registerInDataStore("ThisBeInterestingNameForEvents"));
      EXPECT_TRUE(profile.registerInDataStore("MyProfileInfoName", DataStore::c_DontWriteOut));

      //also should work with optional / required
      EXPECT_TRUE(eventsMetaDatas2.isOptional("EventMetaDatas_2"));
      EXPECT_TRUE(eventsMetaDatas2.isRequired("EventMetaDatas_2"));
    }
    DataStore::Instance().setInitializeActive(false);

    //ok, our objects should now know their name
    EXPECT_EQ("ThisBeInterestingNameForEvents", events.getName());
    EXPECT_TRUE(profile.getName() == "MyProfileInfoName");
    EXPECT_TRUE(profile.notWrittenOut());
    EXPECT_TRUE(eventsMetaDatas2.getName() == "EventMetaDatas_2");

    //accessing data
    EXPECT_EQ(0, events.getEntries());
    EXPECT_FALSE(profile.isValid());
    EXPECT_EQ(10, eventsMetaDatas2.getEntries());

    //saving data
    profile.create();
    EXPECT_TRUE(profile.isValid());
    StoreObjPtr<ProfileInfo> profileAttachAgain("MyProfileInfoName");
    EXPECT_TRUE(profileAttachAgain.isValid());

    events.appendNew();
    EXPECT_EQ(1, events.getEntries());
    StoreArray<EventMetaData> eventsAttachAgain("ThisBeInterestingNameForEvents");
    EXPECT_EQ(1, eventsAttachAgain.getEntries());
  }

  TEST_F(DataStoreTest, ArrayList)
  {
    std::vector<std::string> arrayList = StoreArray<EventMetaData>::getArrayList();
    std::vector<std::string> exparrayList = {"EventMetaDatas", "EventMetaDatas_2"};
    EXPECT_EQ(exparrayList, arrayList);
  }
}  // namespace
