#include <framework/datastore/RelationIndex.h>
#include <framework/dataobjects/EventMetaData.h>
#include <framework/dataobjects/ProfileInfo.h>
#include <gtest/gtest.h>
#include <boost/foreach.hpp>
#include <iostream>
using namespace std;

namespace Belle2 {
#define EXPECT_FATAL(x) EXPECT_EXIT(x,::testing::KilledBySignal(SIGABRT),"");

  /** The fixture for testing class Foo. */
  class RelationTest : public ::testing::Test {
  protected:
    /** fill StoreArrays with entries from 0..9 */
    virtual void SetUp() {
      DataStore::Instance().setInitializeActive(true);
      StoreArray<EventMetaData>::registerPersistent();
      StoreArray<ProfileInfo>::registerPersistent();
      DataStore::Instance().setInitializeActive(false);

      evtData = new StoreArray<EventMetaData>;
      evtData->create();
      profileData = new StoreArray<ProfileInfo>;
      profileData->create();

      for (int i = 0; i < 10; ++i) {
        evtData->appendNew();
        profileData->appendNew();
      }
    }

    /** clear datastore */
    virtual void TearDown() {
      for (int i = 0; i < DataStore::c_NDurabilityTypes; ++i) {
        DataStore::Instance().clearMaps((DataStore::EDurability) i);
        const DataStore::StoreObjMap& map = DataStore::Instance().getStoreObjectMap(DataStore::EDurability(i));
        const_cast<DataStore::StoreObjMap&>(map).clear();
      }
      delete evtData;
      delete profileData;
    }

    StoreArray<EventMetaData>* evtData; /**< event data array */
    StoreArray<ProfileInfo>* profileData; /**< run data array */
  };

  /** Tests the creation of a Relation. */
  TEST_F(RelationTest, RelationCreate)
  {
    DataStore::Instance().setInitializeActive(true);
    RelationArray::registerPersistent(DataStore::relationName(evtData->getName(), profileData->getName()));
    DataStore::Instance().setInitializeActive(false);

    RelationArray relation(*evtData, *profileData);
    relation.create();
    EXPECT_TRUE(relation);
  }

  /** Check finding of relations. */
  TEST_F(RelationTest, RelationFind)
  {
    EXPECT_FALSE(RelationArray::required(DataStore::relationName(evtData->getName(), profileData->getName())));

    DataStore::Instance().setInitializeActive(true);
    RelationArray::registerPersistent(DataStore::relationName(evtData->getName(), profileData->getName()));
    StoreArray<EventMetaData>::registerPersistent("OwnName");
    RelationArray::registerPersistent(DataStore::relationName("OwnName", profileData->getName()));
    DataStore::Instance().setInitializeActive(false);

    EXPECT_FALSE(RelationArray(DataStore::relationName(evtData->getName(), profileData->getName())));
    RelationArray relation(*evtData, *profileData);
    relation.create();
    EXPECT_TRUE(RelationArray(*evtData, *profileData, "", DataStore::c_Event));
    string name = relation.getName();
    EXPECT_TRUE(RelationArray(name));

    StoreArray<EventMetaData> evtData2("OwnName");
    //check for OwnNameToProfileInfos
    EXPECT_FALSE(RelationArray(DataStore::relationName(evtData2.getName(), profileData->getName()), DataStore::c_Event));
    EXPECT_FALSE(RelationArray("OwnNameToProfileInfos", DataStore::c_Event));
    RelationArray relation2(evtData2, *profileData);
    relation2.create();
    EXPECT_TRUE(relation2.getName() == "OwnNameToProfileInfos");
    EXPECT_TRUE(RelationArray(evtData2, *profileData));
  }

  /** Test that Relations wich points to the wrong arrays yields a FATAL. */
  TEST_F(RelationTest, RelationWrongDeathTest)
  {
    DataStore::Instance().setInitializeActive(true);
    RelationArray::registerPersistent("test");
    DataStore::Instance().setInitializeActive(false);

    RelationArray relation1(*evtData, *profileData, "test");
    relation1.create();
    EXPECT_FATAL(RelationArray(*profileData, *evtData, "test").isValid());
    EXPECT_FATAL(RelationArray(*profileData, *evtData, "test").add(0, 0, 1.0));
    EXPECT_FATAL(RelationArray(*profileData, *evtData, "test")[0]);
    EXPECT_FATAL(RelationArray(*profileData, *evtData, "test").getFromAccessorParams());
    EXPECT_FATAL(RelationArray(*profileData, *evtData, "test").getToAccessorParams());
    EXPECT_FATAL(RelationArray(*profileData, *evtData, "test").getModified());
  }

  /** Some events may have default constructed relations (i.e. nothing
   *  was inserted and create() was never called). For those, we don't
   *  want the construction of the array fail with a FATAL.
   */
  TEST_F(RelationTest, RelationDefaultConstructed)
  {
    DataStore::Instance().setInitializeActive(true);
    RelationArray::registerPersistent("somethingnew");
    DataStore::Instance().setInitializeActive(false);

    RelationContainer* rel = new RelationContainer(); //default constructed object, as written to file
    ASSERT_TRUE(DataStore::Instance().createObject(rel, false, "somethingnew", DataStore::c_Event, RelationContainer::Class(), false));

    RelationArray array(*evtData, *profileData, "somethingnew");
    EXPECT_FALSE(array.isValid());

    //shouldn't die here
    RelationIndex<EventMetaData, ProfileInfo> index(*evtData, *profileData, "somethingnew");
  }

  /** Check consolidation of RelationElements. */
  TEST_F(RelationTest, RelationConsolidate)
  {
    DataStore::Instance().setInitializeActive(true);
    RelationArray::registerPersistent(DataStore::relationName(evtData->getName(), profileData->getName()));
    DataStore::Instance().setInitializeActive(false);

    RelationArray relation(*evtData, *profileData);
    relation.add(0, 0, 1.0);
    relation.add(0, 1, 2.0);
    relation.add(0, 1, 3.0);
    relation.add(1, 0, 1.0);
    ASSERT_EQ(relation.getEntries(), 4);
    relation.consolidate();
    ASSERT_EQ(relation.getEntries(), 2);
    EXPECT_EQ(relation[0].getWeight(0), 1.0);
    EXPECT_EQ(relation[0].getWeight(1), 5.0);
    EXPECT_EQ(relation[1].getWeight(0), 1.0);

    std::map<unsigned int, unsigned int> replace;
    replace[0] = 1;
    RelationArray::ReplaceMap<> replaceMap(replace);
    relation.consolidate(replaceMap, replaceMap);
    ASSERT_EQ(relation.getEntries(), 1);
    EXPECT_EQ(relation[0].getWeight(0), 7.0);
    EXPECT_EQ(relation[0].getFromIndex(), 1u);
    EXPECT_EQ(relation[0].getToIndex(), 1u);
  }

  /** Check creation of an index. */
  TEST_F(RelationTest, BuildIndex)
  {
    DataStore::Instance().setInitializeActive(true);
    RelationArray::registerPersistent(DataStore::relationName(evtData->getName(), profileData->getName()));
    DataStore::Instance().setInitializeActive(false);

    RelationArray relation(*evtData, *profileData);
    relation.add(0, 0, 1.0);
    relation.add(0, 1, 2.0);
    relation.add(0, 2, 3.0);
    relation.consolidate();
    EXPECT_EQ(relation.getEntries(), 1);

    RelationIndex<EventMetaData, ProfileInfo> relIndex;
    EXPECT_EQ(relIndex.size(), 3u);

    relation.add(1, 0, 1.0);
    RelationIndex<EventMetaData, ProfileInfo> relIndex2;
    EXPECT_EQ(relIndex2.size(), 4u);
    //Rebuilding index will affect old index. Should we copy index?
    //Copy could be expensive and this should be a corner-case anyway
    EXPECT_EQ(relIndex.size(), 4u);

    typedef const RelationIndex<EventMetaData, ProfileInfo>::Element el_t;
    //check elements of last relation (both from objects point to to_obj)
    const EventMetaData* first_from_obj = (*evtData)[0];
    const EventMetaData* from_obj = (*evtData)[1];
    const ProfileInfo* to_obj = (*profileData)[0];
    EXPECT_TRUE(first_from_obj == relIndex.getFirstElementTo(to_obj)->from);
    EXPECT_TRUE(to_obj == relIndex.getFirstElementTo(to_obj)->to);
    EXPECT_FLOAT_EQ(1.0, relIndex.getFirstElementTo(to_obj)->weight);
    EXPECT_TRUE(first_from_obj == relIndex.getFirstElementTo(*to_obj)->from);
    EXPECT_TRUE(to_obj == relIndex.getFirstElementFrom(from_obj)->to);
    EXPECT_TRUE(to_obj == relIndex.getFirstElementFrom(*from_obj)->to);
    EXPECT_TRUE(to_obj == relIndex.getFirstElementFrom(first_from_obj)->to);

    //check search for non-existing relations
    EXPECT_TRUE(relIndex.getFirstElementTo(0) == NULL);
    EXPECT_TRUE(relIndex.getFirstElementFrom(0) == NULL);
    EXPECT_TRUE(relIndex.getFirstElementFrom(0) == NULL);
    EXPECT_TRUE(relIndex.getFirstElementFrom((*evtData)[4]) == NULL);
    EXPECT_TRUE(relIndex.getFirstElementTo((*profileData)[3]) == NULL);

    //check size of found element lists
    {
      int size(0);
      double allweights(0);
      BOOST_FOREACH(el_t & e, relIndex.getElementsFrom((*evtData)[0])) {
        ++size;
        allweights += e.weight;
      }
      EXPECT_EQ(size, 3);
      EXPECT_DOUBLE_EQ(allweights, 6.0);
    }
    {
      int size(0);
      double allweights(0);
      BOOST_FOREACH(el_t & e, relIndex.getElementsTo((*profileData)[0])) {
        ++size;
        allweights += e.weight;
      }
      EXPECT_EQ(size, 2);
      EXPECT_DOUBLE_EQ(allweights, 2.0);
    }
    {
      int size(0);
      double allweights(0);
      BOOST_FOREACH(el_t & e, relIndex.getElementsTo((*profileData)[4])) {
        ++size;
        allweights += e.weight;
      }
      EXPECT_EQ(size, 0);
      EXPECT_DOUBLE_EQ(allweights, 0.0);
    }
  }

  /** Check wether out-of-bound indices are caught by RelationIndex. */
  TEST_F(RelationTest, InconsistentIndexDeathTest)
  {
    DataStore::Instance().setInitializeActive(true);
    RelationArray::registerPersistent(DataStore::relationName(evtData->getName(), profileData->getName()));
    DataStore::Instance().setInitializeActive(false);

    RelationArray relation(*evtData, *profileData);
    relation.add(0, 10, 1.0);
    typedef RelationIndex<EventMetaData, ProfileInfo> rel_t;
    EXPECT_FATAL(rel_t relIndex);
    relation.clear();
    relation.add(10, 0, 1.0);
    EXPECT_FATAL(rel_t relIndex);
  }


  /** Check behaviour when attaching to non-existing(=empty) relation */
  TEST_F(RelationTest, EmptyIndex)
  {
    DataStore::Instance().setInitializeActive(true);
    RelationArray::registerPersistent(DataStore::relationName(evtData->getName(), profileData->getName()));
    DataStore::Instance().setInitializeActive(false);

    RelationIndex<EventMetaData, ProfileInfo> index;
    EXPECT_FALSE(index);
    EXPECT_EQ(index.size(), 0u);
    EXPECT_EQ(index.getFromAccessorParams().first, "");
    EXPECT_EQ(index.getToAccessorParams().first, "");
  }

  /** Attaching to relation with from and two swapped, and with different StoreArray of same type. */
  TEST_F(RelationTest, WrongRelationIndexDeathTest)
  {
    DataStore::Instance().setInitializeActive(true);
    RelationArray::registerPersistent("test");
    RelationArray::registerPersistent(DataStore::relationName(evtData->getName(), profileData->getName()));
    StoreArray<EventMetaData>::registerPersistent("evts");
    RelationArray::registerPersistent("test2");
    DataStore::Instance().setInitializeActive(false);

    RelationArray relation(*profileData, *evtData, "test");
    relation.create();
    typedef RelationIndex<EventMetaData, ProfileInfo> rel_t;
    EXPECT_FATAL(rel_t(*evtData, *profileData, "test"));
    EXPECT_FATAL(rel_t("test"));

    StoreArray<EventMetaData> eventData("evts");
    RelationArray relation2(*evtData, *profileData, "test2");
    relation2.create();
    EXPECT_FATAL(rel_t(eventData, *profileData, "test2"));

    //This relation works and points to evtData, not eventData.
    //no check is performed, user is responsible to check
    //using getFromAccessorParams and getToAccessorParams
    EXPECT_TRUE(rel_t("test2"));
  }

}  // namespace
