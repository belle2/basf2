#include <framework/datastore/RelationIndex.h>
#include <framework/dataobjects/EventMetaData.h>
#include <framework/dataobjects/ProfileInfo.h>
#include <framework/datastore/RelationsObject.h>
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
      StoreArray<RelationsObject>::registerPersistent();
      DataStore::Instance().setInitializeActive(false);

      evtData = new StoreArray<EventMetaData>;
      profileData = new StoreArray<ProfileInfo>;
      relObjData = new StoreArray<RelationsObject>;

      for (int i = 0; i < 10; ++i) {
        evtData->appendNew();
        profileData->appendNew();
        relObjData->appendNew();
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
      delete relObjData;
    }

    void findRelationsCheckContents();

    StoreArray<EventMetaData>* evtData; /**< event data array */
    StoreArray<ProfileInfo>* profileData; /**< run data array */
    StoreArray<RelationsObject>* relObjData; /**< some objects to test RelationsInterface. */
  };

  /** Tests the creation of a Relation. */
  TEST_F(RelationTest, RelationCreate)
  {
    DataStore::Instance().setInitializeActive(true);
    RelationArray::registerPersistent(DataStore::relationName(evtData->getName(), profileData->getName()));
    DataStore::Instance().setInitializeActive(false);

    RelationArray relation(*evtData, *profileData);
    EXPECT_FALSE(relation); //creation only happens on write access or explicitly
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

  /** Check contents of previously created relations. */
  void RelationTest::findRelationsCheckContents()
  {
    const EventMetaData* fromObj = (*evtData)[0];
    RelationVector<ProfileInfo> toRels = DataStore::getRelationsFromObj<ProfileInfo>(fromObj);
    EXPECT_EQ(toRels.size(), 3);
    //this is assuming stable order, correct?
    EXPECT_DOUBLE_EQ(toRels.weight(0), 1.0);
    EXPECT_DOUBLE_EQ(toRels.weight(1), 2.0);
    EXPECT_DOUBLE_EQ(toRels.weight(2), 3.0);

    EXPECT_TRUE(toRels.object(0) == (*profileData)[0]);
    EXPECT_TRUE(toRels.object(1) == (*profileData)[1]);
    EXPECT_TRUE(toRels.object(2) == (*profileData)[2]);

    const ProfileInfo* toObj = (*profileData)[2];
    RelationVector<EventMetaData> fromRels = DataStore::getRelationsToObj<EventMetaData>(toObj);
    EXPECT_EQ(fromRels.size(), 1);
    EXPECT_DOUBLE_EQ(fromRels.weight(0), 3.0);
    EXPECT_TRUE(fromRels.object(0) == fromObj);
    EXPECT_TRUE(fromRels[0] == fromObj);

    //some things that shouldn't return anything
    EXPECT_EQ(DataStore::getRelationsFromObj<EventMetaData>(fromObj).size(), 0);
    EXPECT_EQ(DataStore::getRelationsToObj<ProfileInfo>(fromObj).size(), 0);
    EXPECT_EQ(DataStore::getRelationsToObj<EventMetaData>(fromObj).size(), 0);
  }

  /** Test DataStore members for finding relations. */
  TEST_F(RelationTest, FindRelations)
  {
    DataStore::Instance().setInitializeActive(true);
    RelationArray::registerPersistent(DataStore::relationName(evtData->getName(), profileData->getName()));
    DataStore::Instance().setInitializeActive(false);

    //check non-existing relations (registered)
    const EventMetaData* fromObj = (*evtData)[0];
    RelationVector<ProfileInfo> toRels = DataStore::getRelationsFromObj<ProfileInfo>(fromObj);
    EXPECT_EQ(toRels.size(), 0);
    const ProfileInfo* toObj = (*profileData)[2];
    RelationVector<EventMetaData> fromRels = DataStore::getRelationsToObj<EventMetaData>(toObj);
    EXPECT_EQ(fromRels.size(), 0);

    //check non-existing relations (unregistered)
    RelationVector<EventMetaData> toRels2 = DataStore::getRelationsFromObj<EventMetaData>(fromObj);
    EXPECT_EQ(toRels2.size(), 0);
    RelationVector<ProfileInfo> fromRels2 = DataStore::getRelationsToObj<ProfileInfo>(toObj);
    EXPECT_EQ(fromRels2.size(), 0);

    RelationArray relation(*evtData, *profileData);
    relation.add(0, 0, 1.0);
    relation.add(0, 1, 2.0);
    relation.add(0, 2, 3.0);

    findRelationsCheckContents();


    //check that results don't change after consolidation
    relation.consolidate();
    findRelationsCheckContents();
  }

  /** Test DataStore members for adding relations. */
  TEST_F(RelationTest, AddRelations)
  {
    DataStore::Instance().setInitializeActive(true);
    RelationArray::registerPersistent(DataStore::relationName(evtData->getName(), profileData->getName()));
    DataStore::Instance().setInitializeActive(false);

    DataStore::Instance().addRelationFromTo((*evtData)[0], (*profileData)[0], 1.0);
    DataStore::Instance().addRelationFromTo((*evtData)[0], (*profileData)[1], 2.0);
    DataStore::Instance().addRelationFromTo((*evtData)[0], (*profileData)[2], 3.0);

    findRelationsCheckContents();
  }

  /** Test DataStore::getRelationsWith. */
  TEST_F(RelationTest, GetRelationsWith)
  {
    DataStore::Instance().setInitializeActive(true);
    RelationArray::registerPersistent(DataStore::relationName(evtData->getName(), profileData->getName()));
    DataStore::Instance().setInitializeActive(false);

    DataStore::Instance().addRelationFromTo((*evtData)[0], (*profileData)[0], 1.0);
    DataStore::Instance().addRelationFromTo((*evtData)[0], (*profileData)[1], 2.0);
    DataStore::Instance().addRelationFromTo((*evtData)[0], (*profileData)[2], 3.0);

    //some objects with no relations to given type
    EXPECT_EQ(DataStore::Instance().getRelationsWithObj<EventMetaData>((*evtData)[0]).size(), 0);
    EXPECT_EQ(DataStore::Instance().getRelationsWithObj<EventMetaData>((*profileData)[3]).size(), 0);

    RelationVector<ProfileInfo> profRels = DataStore::Instance().getRelationsWithObj<ProfileInfo>((*evtData)[0]);
    EXPECT_EQ(profRels.size(), 3);
    EXPECT_EQ(profRels.weight(0), 1.0); //should be positive

    RelationVector<EventMetaData> eventRels = DataStore::Instance().getRelationsWithObj<EventMetaData>((*profileData)[0]);
    EXPECT_EQ(eventRels.size(), 1);
    EXPECT_EQ(eventRels.weight(0), -1.0); //points to given object, negative weight
  }

  /** Test searching all "ALL" storearrays for objects. */
  TEST_F(RelationTest, SearchAll)
  {
    //2nd array of this type
    StoreArray<ProfileInfo> profileData2("ProfileInfos2");

    DataStore::Instance().setInitializeActive(true);
    profileData2.registerAsPersistent();
    RelationArray::registerPersistent(DataStore::relationName(evtData->getName(), profileData->getName()));
    RelationArray::registerPersistent(DataStore::relationName(evtData->getName(), profileData2.getName()));
    DataStore::Instance().setInitializeActive(false);

    DataStore::Instance().addRelationFromTo((*evtData)[0], (*profileData)[0], 1.0);
    DataStore::Instance().addRelationFromTo((*evtData)[0], (*profileData)[1], 2.0);
    DataStore::Instance().addRelationFromTo((*evtData)[0], (*profileData)[2], 3.0);

    //add one object (plus relation) to the other array
    profileData2.appendNew();
    DataStore::Instance().addRelationFromTo((*evtData)[0], profileData2[0], 42.0);

    //profileData2 shouldn't be searched by default when searching or EventMetaData objects
    findRelationsCheckContents();

    //actually test "ALL" option
    const EventMetaData* fromObj = (*evtData)[0];
    RelationVector<ProfileInfo> toRels = DataStore::getRelationsFromObj<ProfileInfo>(fromObj, "ALL");
    EXPECT_EQ(toRels.size(), 4);
    //order might be anything, check sum of weights
    double sum = 0.0;
    for (int i = 0; i < (int)toRels.size(); i++) {
      sum += toRels.weight(i);
    }
    EXPECT_DOUBLE_EQ(sum, 42.0 + 1 + 2 + 3);

    //finding with default TO name
    EXPECT_EQ(DataStore::getRelationsFromObj<ProfileInfo>(fromObj, profileData->getName()).size(), 3);
    //finding with TO name of 2nd array
    EXPECT_EQ(DataStore::getRelationsFromObj<ProfileInfo>(fromObj, profileData2.getName()).size(), 1);
    //and something that doesn't exist
    EXPECT_EQ(DataStore::getRelationsFromObj<ProfileInfo>(fromObj, "DoesntExist").size(), 0);

    const ProfileInfo* toObj = profileData2[0];
    //object should also be found without specifying the name
    EXPECT_EQ(DataStore::getRelationsToObj<EventMetaData>(toObj).size(), 1);
    //or with 'ALL"
    EXPECT_EQ(DataStore::getRelationsToObj<EventMetaData>(toObj, "ALL").size(), 1);
    //and using a base class
    EXPECT_EQ(DataStore::getRelationsToObj<TObject>(toObj, "ALL").size(), 1);

    //no relations to this type
    EXPECT_EQ(DataStore::getRelationsFromObj<EventMetaData>(fromObj, "ALL").size(), 0);
    //should work again
    EXPECT_EQ(DataStore::getRelationsFromObj<TObject>(fromObj, "ALL").size(), 4);
  }

  /** Test adding/finding using RelationsObject/RelationsInterface. */
  TEST_F(RelationTest, RelationsObject)
  {
    DataStore::Instance().setInitializeActive(true);
    RelationArray::registerPersistent(DataStore::relationName(relObjData->getName(), profileData->getName()));
    DataStore::Instance().setInitializeActive(false);

    (*relObjData)[0]->addRelationTo((*profileData)[0], -42.0);
    RelationVector<ProfileInfo> rels = (*relObjData)[0]->getRelationsTo<ProfileInfo>();
    EXPECT_TRUE(rels.size() == 1);
    EXPECT_TRUE(rels.object(0) == (*profileData)[0]);
    EXPECT_DOUBLE_EQ(rels.weight(0), -42.0);
  }

  /** Test getting array name/index from a RelationsObject. */
  TEST_F(RelationTest, RelationsObjectArrayIndex)
  {
    EXPECT_TRUE((*relObjData)[0]->getArrayName() == relObjData->getName());
    EXPECT_TRUE((*relObjData)[9]->getArrayName() == relObjData->getName());
    EXPECT_TRUE((*relObjData)[0]->getArrayIndex() == 0);
    EXPECT_TRUE((*relObjData)[9]->getArrayIndex() == 9);

    RelationsObject bla;
    EXPECT_TRUE(bla.getArrayName() == "");
    EXPECT_TRUE(bla.getArrayIndex() == -1);
  }

}  // namespace
