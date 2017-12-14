#include <framework/dataobjects/EventMetaData.h>
#include <framework/dataobjects/ProfileInfo.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/RelationsObject.h>
#include <framework/utilities/TestHelpers.h>
#include <framework/logging/Logger.h>

#include <gtest/gtest.h>

#include <algorithm>

using namespace std;
using namespace Belle2;

namespace {
  /** Set up a few arrays and objects in the datastore */
  class DataStoreTest : public ::testing::Test {
  protected:
    /** fill StoreArrays with entries from 0..9 */
    virtual void SetUp()
    {
      StoreObjPtr<EventMetaData> evtPtr;
      StoreArray<EventMetaData> evtData;
      StoreArray<EventMetaData> evtDataDifferentName("EventMetaDatas_2");
      StoreArray<EventMetaData> evtDataEmpty("Empty");
      StoreArray<EventMetaData> evtDataDifferentDurability("", DataStore::c_Persistent);
      StoreArray<ProfileInfo> profileInfo;

      DataStore::Instance().setInitializeActive(true);
      evtPtr.registerInDataStore();
      evtData.registerInDataStore();
      evtDataDifferentName.registerInDataStore();
      evtDataEmpty.registerInDataStore();
      evtDataDifferentDurability.registerInDataStore();
      profileInfo.registerInDataStore();
      DataStore::Instance().setInitializeActive(false);

      evtPtr.create();
      evtPtr->setEvent(42);


      ProfileInfo profileInfoObject(128, 64, 60.0);
      for (int i = 0; i < 10; ++i) {
        //direct construction
        EventMetaData* newobj = evtData.appendNew();
        newobj->setEvent(10 + i);

        //copy-construction
        EventMetaData newObj2;
        newObj2.setEvent(20 + i);
        evtDataDifferentName.appendNew(newObj2);

        //fancy constructors
        // cppcheck-suppress memleak
        newobj = evtDataDifferentDurability.appendNew(30 + i);
        ASSERT_TRUE(newobj != nullptr);

        //copy-construct ProfileInfo objects
        profileInfo.appendNew(profileInfoObject);
      }
    }

    /** clear datastore */
    virtual void TearDown()
    {
      DataStore::Instance().reset();
    }

    /** verify contents created in SetUp(). */
    static void verifyContents()
    {
      StoreObjPtr<EventMetaData> evtPtr;
      EXPECT_TRUE(evtPtr.isValid());
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

        EXPECT_EQ(profileInfo[i]->getVirtualMemory(), 128u);
        EXPECT_FLOAT_EQ(profileInfo[i]->getTimeInSec(), 60.0);
      }
    }

  };

  TEST_F(DataStoreTest, EntryNames)
  {
    EXPECT_EQ("JustSomeStuff", DataStore::defaultObjectName("JustSomeStuff"));
    EXPECT_EQ("JustSomeStuff", DataStore::defaultObjectName("Belle2::Foo::JustSomeStuff"));
    EXPECT_EQ("JustSomeStuffs", DataStore::defaultArrayName("Belle2::Foo::JustSomeStuff"));

    EXPECT_EQ("MyOwnName", DataStore::arrayName<TObject>("MyOwnName"));
    EXPECT_EQ("MyOwnName", DataStore::arrayName(TObject::Class(), "MyOwnName"));
    EXPECT_EQ("TObjects", DataStore::arrayName<TObject>(""));
    EXPECT_EQ("TObjects", DataStore::arrayName(TObject::Class(), ""));

    EXPECT_EQ("MyOwnName", DataStore::objectName<TObject>("MyOwnName"));
    EXPECT_EQ("MyOwnName", DataStore::objectName(TObject::Class(), "MyOwnName"));

    EXPECT_EQ("TObject", DataStore::objectName<TObject>(""));
    EXPECT_EQ("TObject", DataStore::objectName(TObject::Class(), ""));

    EXPECT_EQ("EventMetaDatas", DataStore::arrayName<EventMetaData>(""));
    EXPECT_EQ("GF2Track", DataStore::defaultObjectName("genfit::Track"));

    EXPECT_EQ("AToB", DataStore::relationName("A", "B"));
    auto relname = DataStore::defaultRelationName<EventMetaData, ProfileInfo>();
    EXPECT_EQ("EventMetaDatasToProfileInfos", relname);
  }

  TEST_F(DataStoreTest, GetTClass)
  {
    EXPECT_EQ(Belle2::EventMetaData::Class(), DataStore::getTClassFromDefaultObjectName("Belle2::EventMetaData"));
    EXPECT_EQ(Belle2::EventMetaData::Class(), DataStore::getTClassFromDefaultObjectName("EventMetaData"));

    EXPECT_EQ(Belle2::EventMetaData::Class(), DataStore::getTClassFromDefaultArrayName("Belle2::EventMetaDatas"));
    EXPECT_EQ(Belle2::EventMetaData::Class(), DataStore::getTClassFromDefaultArrayName("EventMetaDatas"));
  }


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
    StoreObjPtr<TObject> emd("EventMetaData");
    EXPECT_TRUE(emd.isValid());
    EXPECT_EQ(std::string("Belle2::EventMetaData"), std::string(emd->GetName()));
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
    verifyContents();

    //test removing data
    StoreArray<EventMetaData> evtData;
    evtData.clear();
    EXPECT_TRUE(evtData.isValid());
    EXPECT_EQ(evtData.getEntries(), 0);

    StoreObjPtr<EventMetaData> evtPtr;
    evtPtr.clear(); //resets to default-constructed object
    EXPECT_TRUE(evtPtr.isValid());
    EXPECT_EQ(evtPtr->getEvent(), 1);
  }
  TEST_F(DataStoreTest, InvalidAccessor)
  {
    StoreArray<EventMetaData> none("doesntexist");
    EXPECT_FALSE(none.isValid());
    EXPECT_THROW(none.getPtr(), std::runtime_error);
    EXPECT_THROW(none.appendNew(), std::runtime_error);
    none.clear();
    EXPECT_THROW(none.getPtr(), std::runtime_error);
    EXPECT_EQ(0, none.getEntries());

    StoreObjPtr<EventMetaData> noobj("doesntexist");
    EXPECT_FALSE(noobj.isValid());
    EXPECT_THROW(*noobj, std::runtime_error);
    EXPECT_THROW(noobj->getEvent(), std::runtime_error);

  }
  TEST_F(DataStoreTest, RawAccess)
  {
    StoreArray<EventMetaData> evtData;
    EXPECT_TRUE(evtData.getPtr() != nullptr);
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
    EXPECT_THROW(evtData[-1], std::out_of_range);
    EXPECT_THROW(evtData[10], std::out_of_range);
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
    DataStore::Instance().setInitializeActive(true);
    EXPECT_TRUE(StoreObjPtr<EventMetaData>().isRequired());
    EXPECT_FALSE(StoreObjPtr<EventMetaData>("", DataStore::c_Persistent).isRequired());
    EXPECT_FALSE(StoreObjPtr<EventMetaData>("nonexisting2").isRequired());
    EXPECT_FALSE(StoreObjPtr<EventMetaData>("", DataStore::c_Persistent).isRequired());
    //check we didn't create one...
    EXPECT_FALSE(StoreObjPtr<EventMetaData>("nonexisting2").isRequired());

    EXPECT_TRUE(StoreArray<EventMetaData>().isRequired());
    EXPECT_TRUE(StoreArray<EventMetaData>("EventMetaDatas_2").isRequired());
    EXPECT_TRUE(StoreArray<EventMetaData>("", DataStore::c_Persistent).isRequired());
    EXPECT_FALSE(StoreArray<EventMetaData>("blah").isRequired());
    EXPECT_FALSE(StoreArray<EventMetaData>("blah").isRequired());
    EXPECT_TRUE(StoreArray<ProfileInfo>().isRequired());
    //check we didn't create one...
    EXPECT_FALSE(StoreArray<EventMetaData>("blah").isRequired());
    DataStore::Instance().setInitializeActive(false);

    EXPECT_B2FATAL(StoreObjPtr<EventMetaData>().isRequired());
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
      ++it2;

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

    int i = 0;
    for (StoreArray<EventMetaData>::iterator it = evtDataNonConst.begin(); it != evtDataNonConst.end(); ++it) {
      EXPECT_TRUE(&(*it) == evtData[i]);
      EXPECT_EQ((int)it->getEvent(), 10 + i);
      i++;
    }
    EXPECT_EQ(i, evtData.getEntries());

    //range-based for
    i = 0;
    for (EventMetaData& emd : evtDataNonConst) {
      EXPECT_TRUE(&emd == evtData[i]);
      i++;
    }
    EXPECT_EQ(i, evtData.getEntries());

    //range-based for (const)
    i = 0;
    for (const EventMetaData& emd : evtData) {
      EXPECT_TRUE(&emd == evtData[i]);
      i++;
    }
    EXPECT_EQ(i, evtData.getEntries());

    //iteration over registered, but empty array
    i = 0;
    StoreArray<EventMetaData> evtDataEmpty("Empty");
    for (const EventMetaData& emd : evtDataEmpty) {
      (void)emd;
      i++;
    }
    EXPECT_EQ(i, 0);
    //iteration over non-existing array
    i = 0;
    StoreArray<EventMetaData> nonExistant("doesntexist");
    for (const EventMetaData& emd : nonExistant) {
      (void)emd;
      i++;
    }
    EXPECT_EQ(i, 0);
  }

  /** test registerInDataStore(), optional() */
  TEST_F(DataStoreTest, DataStoreRegistration)
  {
    //emulate Module::initialize()
    DataStore::Instance().setInitializeActive(true);

    StoreObjPtr<EventMetaData> evtPtr("abc123");
    StoreArray<EventMetaData> evtArray("abc123array");

    //verify that they aren't registered right now
    EXPECT_FALSE(evtPtr.isOptional());
    EXPECT_FALSE(evtArray.isOptional());
    EXPECT_FALSE(evtPtr.isRequired());
    EXPECT_FALSE(evtArray.isRequired());
    EXPECT_FALSE(evtPtr.isValid());
    EXPECT_FALSE(evtArray.isValid());
    EXPECT_FALSE(StoreObjPtr<EventMetaData>(evtPtr.getName()).isRequired());
    EXPECT_FALSE(StoreArray<EventMetaData>(evtArray.getName()).isRequired());

    EXPECT_TRUE(evtPtr.registerInDataStore());
    EXPECT_TRUE(evtArray.registerInDataStore(DataStore::c_DontWriteOut));

    //already registered, ok by default
    EXPECT_TRUE(evtPtr.registerInDataStore());
    EXPECT_TRUE(evtArray.registerInDataStore(DataStore::c_DontWriteOut));

    //test c_ErrorIfAlreadyRegistered (return code=false + B2ERROR)
    EXPECT_B2ERROR(EXPECT_FALSE(evtPtr.registerInDataStore(DataStore::c_ErrorIfAlreadyRegistered)));
    EXPECT_B2ERROR(EXPECT_FALSE(evtArray.registerInDataStore(DataStore::c_DontWriteOut | DataStore::c_ErrorIfAlreadyRegistered)));

    //now they should be available:
    EXPECT_TRUE(evtPtr.isOptional());
    EXPECT_TRUE(evtArray.isOptional());
    EXPECT_TRUE(evtPtr.isRequired());
    EXPECT_TRUE(evtArray.isRequired());
    EXPECT_FALSE(evtPtr.isValid()); // not valid until created
    EXPECT_TRUE(evtArray.isValid());
    EXPECT_TRUE(StoreObjPtr<EventMetaData>(evtPtr.getName()).isRequired());
    EXPECT_TRUE(StoreArray<EventMetaData>(evtArray.getName()).isRequired());
    DataStore::Instance().setInitializeActive(false);
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
    std::vector<std::string> exparrayList = {"Empty", "EventMetaDatas", "EventMetaDatas_2"};
    EXPECT_EQ(exparrayList, arrayList);
  }

  TEST_F(DataStoreTest, ReplaceData)
  {
    StoreObjPtr<EventMetaData> evtPtr;
    StoreObjPtr<EventMetaData> evtPtrB("abc123");
    StoreArray<EventMetaData> evtData;
    StoreArray<EventMetaData> evtDataB("otherArray");
    DataStore::Instance().setInitializeActive(true);
    evtPtrB.registerInDataStore(DataStore::c_DontWriteOut);
    evtDataB.registerInDataStore(DataStore::c_DontWriteOut);
    DataStore::Instance().setInitializeActive(false);

    {
      //objects
      EXPECT_EQ(42, evtPtr->getEvent());
      EXPECT_FALSE(evtPtrB.isValid());
      EXPECT_FALSE(evtPtr.notWrittenOut());
      EXPECT_TRUE(evtPtrB.notWrittenOut());

      DataStore::Instance().replaceData(evtPtr, evtPtrB);
      EXPECT_EQ(42, evtPtrB->getEvent());
      EXPECT_FALSE(evtPtr.isValid());

      //metadata unchanged
      EXPECT_FALSE(evtPtr.notWrittenOut());
      EXPECT_TRUE(evtPtrB.notWrittenOut());

      //move null object into existing one
      DataStore::Instance().replaceData(evtPtr, evtPtrB);
      EXPECT_FALSE(evtPtr.isValid());
      EXPECT_FALSE(evtPtrB.isValid());

      //null object to null object
      DataStore::Instance().replaceData(evtPtr, evtPtrB);
      EXPECT_FALSE(evtPtr.isValid());
      EXPECT_FALSE(evtPtrB.isValid());
    }

    {
      //arrays
      EXPECT_EQ(10, evtData.getEntries());
      EXPECT_EQ(0, evtDataB.getEntries());

      DataStore::Instance().replaceData(evtData, evtDataB);
      EXPECT_EQ(0, evtData.getEntries());
      EXPECT_EQ(10, evtDataB.getEntries());

      DataStore::Instance().replaceData(evtData, evtDataB);
      EXPECT_EQ(0, evtData.getEntries());
      EXPECT_EQ(0, evtDataB.getEntries());

      DataStore::Instance().replaceData(evtData, evtDataB);
      EXPECT_EQ(0, evtData.getEntries());
      EXPECT_EQ(0, evtDataB.getEntries());
    }
  }

  TEST_F(DataStoreTest, SwitchDataStore)
  {
    EXPECT_TRUE("" == DataStore::Instance().currentID());
    //not created yet
    EXPECT_THROW(DataStore::Instance().copyContentsTo("foo"), std::out_of_range);

    //create new DS ID
    DataStore::Instance().createNewDataStoreID("foo");
    EXPECT_TRUE("" == DataStore::Instance().currentID());
    DataStore::Instance().copyContentsTo("foo");
    EXPECT_TRUE("" == DataStore::Instance().currentID());
    verifyContents();
    DataStore::Instance().switchID("foo");
    EXPECT_TRUE("foo" == DataStore::Instance().currentID());
    verifyContents(); //still unmodified

    //change something
    StoreObjPtr<EventMetaData> a;
    a->setEvent(1234567);

    //and restore
    DataStore::Instance().switchID("");
    EXPECT_TRUE("" == DataStore::Instance().currentID());
    verifyContents(); //back to normal

    //register another object in ""
    //and try copying (this catches insufficient checks during copying)
    StoreArray<EventMetaData> evtDataB("otherArray");
    DataStore::Instance().setInitializeActive(true);
    evtDataB.registerInDataStore();
    DataStore::Instance().copyContentsTo("foo");
  }

  TEST_F(DataStoreTest, FindStoreEntry)
  {
    DataStore::StoreEntry* entry = nullptr;
    int index = -1;

    //test TObject-derived arrays
    StoreArray<EventMetaData> evtData;
    StoreArray<EventMetaData> evtDataDifferentName("EventMetaDatas_2");

    EXPECT_TRUE(DataStore::Instance().findStoreEntry(evtData[5], entry, index));
    EXPECT_EQ(index, 5);
    EXPECT_EQ(entry->name, evtData.getName());

    //entry and index are already correct, should return quickly
    EXPECT_TRUE(DataStore::Instance().findStoreEntry(evtData[5], entry, index));
    EXPECT_EQ(index, 5);
    EXPECT_EQ(entry->name, evtData.getName());

    //not resetting entry, index here. this should not usually happen, but ought to be harmless
    EXPECT_TRUE(DataStore::Instance().findStoreEntry(evtDataDifferentName[7], entry, index));
    EXPECT_EQ(index, 7);
    EXPECT_EQ(entry->name, evtDataDifferentName.getName());

    entry = nullptr; index = -1;
    EXPECT_TRUE(DataStore::Instance().findStoreEntry(evtDataDifferentName[7], entry, index));
    EXPECT_EQ(index, 7);
    EXPECT_EQ(entry->name, evtDataDifferentName.getName());

    entry = nullptr; index = -1;
    EventMetaData localObj;
    EXPECT_FALSE(DataStore::Instance().findStoreEntry(&localObj, entry, index));
    EXPECT_EQ(index, -1);
    EXPECT_EQ(entry, nullptr);


    //test RelationsObjects (caches used)
    StoreArray<RelationsObject> relObjs;
    StoreArray<RelationsObject> relObjs2("relobs2");
    DataStore::Instance().setInitializeActive(true);
    relObjs.registerInDataStore();
    relObjs2.registerInDataStore();
    DataStore::Instance().setInitializeActive(false);
    for (int i = 0; i < 6; i++) {
      relObjs.appendNew();
      relObjs2.appendNew();
    }

    entry = nullptr; index = -1;
    EXPECT_TRUE(DataStore::Instance().findStoreEntry(relObjs[5], entry, index));
    EXPECT_EQ(index, 5);
    EXPECT_EQ(entry->name, relObjs.getName());

    //should use cache now
    entry = nullptr; index = -1;
    EXPECT_TRUE(DataStore::Instance().findStoreEntry(relObjs[5], entry, index));
    EXPECT_EQ(index, 5);
    EXPECT_EQ(entry->name, relObjs.getName());

    entry = nullptr; index = -1;
    EXPECT_TRUE(DataStore::Instance().findStoreEntry(relObjs2[2], entry, index));
    EXPECT_EQ(index, 2);
    EXPECT_EQ(entry->name, relObjs2.getName());

    //and caches again
    entry = nullptr; index = -1;
    EXPECT_TRUE(DataStore::Instance().findStoreEntry(relObjs2[2], entry, index));
    EXPECT_EQ(index, 2);
    EXPECT_EQ(entry->name, relObjs2.getName());

    //test finding storeobjptr (not implemented, so nothing found)
    StoreObjPtr<EventMetaData> evtPtr;
    entry = nullptr; index = -1;
    EXPECT_FALSE(DataStore::Instance().findStoreEntry(&(*evtPtr), entry, index));
    EXPECT_EQ(index, -1);
    EXPECT_EQ(entry, nullptr);

    //searching for nullptr is allowed
    EXPECT_FALSE(DataStore::Instance().findStoreEntry(nullptr, entry, index));
  }

  TEST_F(DataStoreTest, ListEntries)
  {
    StoreArray<EventMetaData> evtData;
    EXPECT_EQ(0, DataStore::Instance().getListOfRelatedArrays(evtData).size());

    DataStore::Instance().setInitializeActive(true);
    StoreArray<ProfileInfo> profileInfo;
    evtData.registerRelationTo(profileInfo);
    DataStore::Instance().setInitializeActive(false);

    EXPECT_EQ(1, DataStore::Instance().getListOfRelatedArrays(evtData).size());
    EXPECT_EQ(1, DataStore::Instance().getListOfRelatedArrays(profileInfo).size());

    EXPECT_EQ(1, DataStore::Instance().getListOfArrays(ProfileInfo::Class(), DataStore::c_Event).size());
    EXPECT_EQ(0, DataStore::Instance().getListOfArrays(ProfileInfo::Class(), DataStore::c_Persistent).size());
    EXPECT_EQ(1, DataStore::Instance().getListOfArrays(EventMetaData::Class(), DataStore::c_Persistent).size());
    EXPECT_EQ(3, DataStore::Instance().getListOfArrays(EventMetaData::Class(), DataStore::c_Event).size());

    EXPECT_EQ(1, DataStore::Instance().getListOfArrays(TObject::Class(), DataStore::c_Persistent).size());
    EXPECT_EQ(4, DataStore::Instance().getListOfArrays(TObject::Class(), DataStore::c_Event).size());

    EXPECT_EQ(1, DataStore::Instance().getListOfObjects(EventMetaData::Class(), DataStore::c_Event).size());
    EXPECT_EQ(0, DataStore::Instance().getListOfObjects(EventMetaData::Class(), DataStore::c_Persistent).size());
    EXPECT_EQ(1, DataStore::Instance().getListOfObjects(TObject::Class(), DataStore::c_Event).size());
    EXPECT_EQ(0, DataStore::Instance().getListOfObjects(TObject::Class(), DataStore::c_Persistent).size());
  }

  TEST_F(DataStoreTest, Assign)
  {
    StoreArray<EventMetaData> evtData;
    EXPECT_FALSE(evtData.assign(new EventMetaData(), true));
    EXPECT_FALSE(evtData.assign(new EventMetaData(), false));

    EXPECT_TRUE(evtData.isValid());
    EXPECT_EQ(evtData.getEntries(), 10);

    StoreObjPtr<EventMetaData> evtPtr;
    EventMetaData* newobj = new EventMetaData();
    newobj->setEvent(123);
    EXPECT_FALSE(evtPtr.assign(new EventMetaData(), false));
    EXPECT_FALSE(evtPtr.assign(new ProfileInfo(), true));
    EXPECT_EQ(evtPtr->getEvent(), 42);
    EXPECT_TRUE(evtPtr.assign(newobj, true));
    EXPECT_EQ(evtPtr->getEvent(), 123);
  }
}  // namespace
