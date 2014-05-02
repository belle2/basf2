#include <framework/datastore/RelationIndex.h>
#include <framework/dataobjects/EventMetaData.h>
#include <framework/dataobjects/ProfileInfo.h>
#include <framework/datastore/RelationsObject.h>
#include <framework/utilities/TestHelpers.h>

#include <gtest/gtest.h>

#include <boost/foreach.hpp>

#include <iostream>
using namespace std;
using namespace Belle2;

namespace {
  /** test relations. */
  class RelationTest : public ::testing::Test {
  protected:
    /** fill StoreArrays with entries from 0..9 */
    virtual void SetUp() {
      DataStore::Instance().setInitializeActive(true);
      evtData.registerPersistent();
      profileData.registerPersistent();
      relObjData.registerPersistent();
      DataStore::Instance().setInitializeActive(false);

      for (int i = 0; i < 10; ++i) {
        evtData.appendNew();
        profileData.appendNew();
        relObjData.appendNew();
      }
    }

    /** clear datastore */
    virtual void TearDown() {
      DataStore::Instance().reset();
    }

    void findRelationsCheckContents();

    StoreArray<EventMetaData> evtData; /**< event data array */
    StoreArray<ProfileInfo> profileData; /**< run data array */
    StoreArray<RelationsObject> relObjData; /**< some objects to test RelationsInterface. */
  };

  /** Tests the creation of a Relation. */
  TEST_F(RelationTest, RelationCreate)
  {
    DataStore::Instance().setInitializeActive(true);
    RelationArray relation(evtData, profileData);
    relation.registerAsPersistent();
    DataStore::Instance().setInitializeActive(false);

    EXPECT_FALSE(relation); //creation only happens on write access or explicitly
    //since we provided arguments to constructor, these should be available even before create()
    EXPECT_TRUE(relation.getFromAccessorParams() == evtData.getAccessorParams());
    EXPECT_TRUE(relation.getToAccessorParams() == profileData.getAccessorParams());

    relation.create();
    EXPECT_TRUE(relation);
    EXPECT_TRUE(relation.getFromAccessorParams() == evtData.getAccessorParams());
    EXPECT_TRUE(relation.getToAccessorParams() == profileData.getAccessorParams());
  }

  /** Check finding of relations. */
  TEST_F(RelationTest, RelationFind)
  {
    EXPECT_FALSE(RelationArray::required(DataStore::relationName(evtData.getName(), profileData.getName())));

    DataStore::Instance().setInitializeActive(true);
    RelationArray relation(evtData, profileData);
    relation.registerAsPersistent();
    StoreArray<EventMetaData>::registerPersistent("OwnName");
    RelationArray::registerPersistent("OwnName", profileData.getName());
    DataStore::Instance().setInitializeActive(false);

    EXPECT_FALSE(RelationArray(DataStore::relationName(evtData.getName(), profileData.getName())));
    relation.create();
    EXPECT_TRUE(RelationArray(evtData, profileData, "", DataStore::c_Event));
    string name = relation.getName();

    RelationArray relationAttachedUsingName(name);
    //trying to get the accessor params should cause the array to attach (and thus get the appropriate data)
    EXPECT_TRUE(relationAttachedUsingName.getFromAccessorParams() == evtData.getAccessorParams());
    EXPECT_TRUE(relationAttachedUsingName.getToAccessorParams() == profileData.getAccessorParams());
    EXPECT_TRUE(relationAttachedUsingName);

    StoreArray<EventMetaData> evtData2("OwnName");
    //check for OwnNameToProfileInfos
    EXPECT_FALSE(RelationArray(DataStore::relationName(evtData2.getName(), profileData.getName()), DataStore::c_Event));
    EXPECT_FALSE(RelationArray("OwnNameToProfileInfos", DataStore::c_Event));
    RelationArray relation2(evtData2, profileData);
    relation2.create();
    EXPECT_TRUE(relation2.getName() == "OwnNameToProfileInfos");
    EXPECT_TRUE(RelationArray(evtData2, profileData));
  }

  /** Test that Relations wich points to the wrong arrays yields a FATAL. */
  TEST_F(RelationTest, RelationWrongDeathTest)
  {
    DataStore::Instance().setInitializeActive(true);
    RelationArray relation1(evtData, profileData, "test");
    relation1.registerAsPersistent();
    DataStore::Instance().setInitializeActive(false);

    relation1.create();
    EXPECT_B2FATAL(RelationArray(profileData, evtData, "test").isValid());
    EXPECT_B2FATAL(RelationArray(profileData, evtData, "test").add(0, 0, 1.0));
    EXPECT_B2FATAL(RelationArray(profileData, evtData, "test")[0]);
    EXPECT_B2FATAL(RelationArray(profileData, evtData, "test").getModified());
  }

  /** Some events may have default constructed relations (i.e. nothing
   *  was inserted and create() was never called). For those, we don't
   *  want the construction of the array fail with a FATAL.
   */
  TEST_F(RelationTest, RelationDefaultConstructed)
  {
    DataStore::Instance().setInitializeActive(true);
    RelationArray array(evtData, profileData, "somethingnew");
    array.registerAsPersistent();
    DataStore::Instance().setInitializeActive(false);

    RelationContainer* rel = new RelationContainer(); //default constructed object, as written to file
    ASSERT_TRUE(DataStore::Instance().createObject(rel, false, array));

    EXPECT_FALSE(array.isValid());

    //shouldn't die here
    RelationIndex<EventMetaData, ProfileInfo> index(evtData, profileData, "somethingnew");
  }

  /** Check creation of an index. */
  TEST_F(RelationTest, BuildIndex)
  {
    DataStore::Instance().setInitializeActive(true);
    RelationArray relation(evtData, profileData);
    relation.registerAsPersistent();
    DataStore::Instance().setInitializeActive(false);

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
    const EventMetaData* first_from_obj = (evtData)[0];
    const EventMetaData* from_obj = (evtData)[1];
    const ProfileInfo* to_obj = (profileData)[0];
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
    EXPECT_TRUE(relIndex.getFirstElementFrom((evtData)[4]) == NULL);
    EXPECT_TRUE(relIndex.getFirstElementTo((profileData)[3]) == NULL);

    //check size of found element lists
    {
      int size(0);
      double allweights(0);
      //for backwards compatibility: BOOST_FOREACH should work
      BOOST_FOREACH(el_t & e, relIndex.getElementsFrom((evtData)[0])) {
        ++size;
        allweights += e.weight;
      }
      EXPECT_EQ(size, 3);
      EXPECT_DOUBLE_EQ(allweights, 6.0);
    }
    {
      int size(0);
      double allweights(0);
      for (el_t & e : relIndex.getElementsTo((profileData)[0])) {
        ++size;
        allweights += e.weight;
      }
      EXPECT_EQ(size, 2);
      EXPECT_DOUBLE_EQ(allweights, 2.0);
    }
    {
      int size(0);
      double allweights(0);
      for (const el_t & e : relIndex.getElementsTo((profileData)[4])) {
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
    RelationArray relation(evtData, profileData);
    relation.registerAsPersistent();
    DataStore::Instance().setInitializeActive(false);

    relation.add(0, 10, 1.0);
    typedef RelationIndex<EventMetaData, ProfileInfo> rel_t;
    EXPECT_B2FATAL(rel_t relIndex);
    relation.clear();
    relation.add(10, 0, 1.0);
    EXPECT_B2FATAL(rel_t relIndex);
  }


  /** Check behaviour when attaching to non-existing(=empty) relation */
  TEST_F(RelationTest, EmptyIndex)
  {
    DataStore::Instance().setInitializeActive(true);
    RelationArray relation(evtData, profileData);
    relation.registerAsPersistent();
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
    RelationArray relation(profileData, evtData, "test");
    relation.registerAsPersistent();
    RelationArray(evtData, profileData).registerAsPersistent();
    StoreArray<EventMetaData>::registerPersistent("evts");
    RelationArray relation2(evtData, profileData, "test2");
    relation2.registerAsPersistent();
    DataStore::Instance().setInitializeActive(false);

    relation.create();
    typedef RelationIndex<EventMetaData, ProfileInfo> rel_t;
    EXPECT_B2FATAL(rel_t(evtData, profileData, "test"));
    EXPECT_B2FATAL(rel_t("test"));

    StoreArray<EventMetaData> eventData("evts");
    relation2.create();
    EXPECT_B2FATAL(rel_t(eventData, profileData, "test2"));

    //This relation works and points to evtData, not eventData.
    //no check is performed, user is responsible to check
    //using getFromAccessorParams and getToAccessorParams
    EXPECT_TRUE(rel_t("test2"));
  }

  /** Check contents of previously created relations. */
  void RelationTest::findRelationsCheckContents()
  {
    const EventMetaData* fromObj = (evtData)[0];
    RelationVector<ProfileInfo> toRels = DataStore::getRelationsFromObj<ProfileInfo>(fromObj);
    EXPECT_EQ(toRels.size(), 3u);
    //this is assuming stable order, correct?
    EXPECT_DOUBLE_EQ(toRels.weight(0), 1.0);
    EXPECT_DOUBLE_EQ(toRels.weight(1), 2.0);
    EXPECT_DOUBLE_EQ(toRels.weight(2), -3.0);

    EXPECT_TRUE(toRels.object(0) == (profileData)[0]);
    EXPECT_TRUE(toRels.object(1) == (profileData)[1]);
    EXPECT_TRUE(toRels.object(2) == (profileData)[2]);

    //test range-based for over RelationVector
    int i = 0;
    for (const ProfileInfo & prof : toRels) {
      (void)prof; //variable unused
      i++;
    }
    EXPECT_EQ(i, 3);

    const ProfileInfo* toObj = (profileData)[2];
    RelationVector<EventMetaData> fromRels = DataStore::getRelationsToObj<EventMetaData>(toObj);
    EXPECT_EQ(fromRels.size(), 1u);
    EXPECT_DOUBLE_EQ(fromRels.weight(0), -3.0);
    EXPECT_TRUE(fromRels.object(0) == fromObj);
    EXPECT_TRUE(fromRels[0] == fromObj);

    //some things that shouldn't return anything
    EXPECT_EQ(DataStore::getRelationsFromObj<EventMetaData>(fromObj).size(), 0u);
    EXPECT_EQ(DataStore::getRelationsToObj<ProfileInfo>(fromObj).size(), 0u);
    EXPECT_EQ(DataStore::getRelationsToObj<EventMetaData>(fromObj).size(), 0u);
  }

  /** Test DataStore members for finding relations. */
  TEST_F(RelationTest, FindRelations)
  {
    DataStore::Instance().setInitializeActive(true);
    RelationArray relation(evtData, profileData);
    relation.registerAsPersistent();
    DataStore::Instance().setInitializeActive(false);

    //check non-existing relations (registered)
    const EventMetaData* fromObj = (evtData)[0];
    RelationVector<ProfileInfo> toRels = DataStore::getRelationsFromObj<ProfileInfo>(fromObj);
    EXPECT_EQ(toRels.size(), 0u);
    const ProfileInfo* toObj = (profileData)[2];
    RelationVector<EventMetaData> fromRels = DataStore::getRelationsToObj<EventMetaData>(toObj);
    EXPECT_EQ(fromRels.size(), 0u);

    //check non-existing relations (unregistered)
    RelationVector<EventMetaData> toRels2 = DataStore::getRelationsFromObj<EventMetaData>(fromObj);
    EXPECT_EQ(toRels2.size(), 0u);
    RelationVector<ProfileInfo> fromRels2 = DataStore::getRelationsToObj<ProfileInfo>(toObj);
    EXPECT_EQ(fromRels2.size(), 0u);

    relation.add(0, 0, 1.0);
    relation.add(0, 1, 2.0);
    relation.add(0, 2, -3.0);

    findRelationsCheckContents();


    //check that results don't change after consolidation
    relation.consolidate();
    findRelationsCheckContents();
  }

  /** Test DataStore members for adding relations. */
  TEST_F(RelationTest, AddRelations)
  {
    DataStore::Instance().setInitializeActive(true);
    RelationArray relation(evtData, profileData);
    relation.registerAsPersistent();
    DataStore::Instance().setInitializeActive(false);

    DataStore::Instance().addRelationFromTo((evtData)[0], (profileData)[0], 1.0);
    DataStore::Instance().addRelationFromTo((evtData)[0], (profileData)[1], 2.0);
    DataStore::Instance().addRelationFromTo((evtData)[0], (profileData)[2], -3.0);

    findRelationsCheckContents();
  }

  /** Test DataStore::getRelationsWith. */
  TEST_F(RelationTest, GetRelationsWith)
  {
    DataStore::Instance().setInitializeActive(true);
    RelationArray relation(evtData, profileData);
    relation.registerAsPersistent();
    DataStore::Instance().setInitializeActive(false);

    DataStore::Instance().addRelationFromTo((evtData)[0], (profileData)[0], 1.0);
    DataStore::Instance().addRelationFromTo((evtData)[0], (profileData)[1], 2.0);
    DataStore::Instance().addRelationFromTo((evtData)[0], (profileData)[2], 3.0);

    //some objects with no relations to given type
    EXPECT_EQ(DataStore::Instance().getRelationsWithObj<EventMetaData>((evtData)[0]).size(), 0u);
    EXPECT_EQ(DataStore::Instance().getRelationsWithObj<EventMetaData>((profileData)[3]).size(), 0u);

    RelationVector<ProfileInfo> profRels = DataStore::Instance().getRelationsWithObj<ProfileInfo>((evtData)[0]);
    EXPECT_EQ(profRels.size(), 3u);
    EXPECT_EQ(profRels.weight(0), 1.0); //should be positive

    RelationVector<EventMetaData> eventRels = DataStore::Instance().getRelationsWithObj<EventMetaData>((profileData)[0]);
    EXPECT_EQ(eventRels.size(), 1u);
    EXPECT_EQ(eventRels.weight(0), 1.0); //points to given object, same weight
  }

  /** Test searching all "ALL" storearrays for objects. */
  TEST_F(RelationTest, SearchAll)
  {
    //2nd array of this type
    StoreArray<ProfileInfo> profileData2("ProfileInfos2");

    DataStore::Instance().setInitializeActive(true);
    profileData2.registerAsPersistent();
    RelationArray relation(evtData, profileData);
    relation.registerAsPersistent();
    RelationArray relation2(evtData, profileData2);
    relation2.registerAsPersistent();
    DataStore::Instance().setInitializeActive(false);

    DataStore::Instance().addRelationFromTo((evtData)[0], (profileData)[0], 1.0);
    DataStore::Instance().addRelationFromTo((evtData)[0], (profileData)[1], 2.0);
    DataStore::Instance().addRelationFromTo((evtData)[0], (profileData)[2], -3.0);

    //add one object (plus relation) to the other array
    profileData2.appendNew();
    DataStore::Instance().addRelationFromTo((evtData)[0], profileData2[0], 42.0);

    //profileData2 shouldn't be searched by default when searching or EventMetaData objects
    findRelationsCheckContents();

    //actually test "ALL" option
    const EventMetaData* fromObj = (evtData)[0];
    RelationVector<ProfileInfo> toRels = DataStore::getRelationsFromObj<ProfileInfo>(fromObj, "ALL");
    EXPECT_EQ(toRels.size(), 4u);
    //order might be anything, check sum of weights
    double sum = 0.0;
    for (int i = 0; i < (int)toRels.size(); i++) {
      sum += toRels.weight(i);
    }
    EXPECT_DOUBLE_EQ(sum, 42.0 + 1 + 2 - 3);

    //finding with default TO name
    EXPECT_EQ(DataStore::getRelationsFromObj<ProfileInfo>(fromObj, profileData.getName()).size(), 3u);
    //finding with TO name of 2nd array
    EXPECT_EQ(DataStore::getRelationsFromObj<ProfileInfo>(fromObj, profileData2.getName()).size(), 1u);
    //and something that doesn't exist
    EXPECT_EQ(DataStore::getRelationsFromObj<ProfileInfo>(fromObj, "DoesntExist").size(), 0u);

    const ProfileInfo* toObj = profileData2[0];
    //object should also be found without specifying the name
    EXPECT_EQ(DataStore::getRelationsToObj<EventMetaData>(toObj).size(), 1u);
    //or with 'ALL"
    EXPECT_EQ(DataStore::getRelationsToObj<EventMetaData>(toObj, "ALL").size(), 1u);
    //and using a base class
    EXPECT_EQ(DataStore::getRelationsToObj<TObject>(toObj, "ALL").size(), 1u);

    //no relations to this type
    EXPECT_EQ(DataStore::getRelationsFromObj<EventMetaData>(fromObj, "ALL").size(), 0u);
    //should work again
    EXPECT_EQ(DataStore::getRelationsFromObj<TObject>(fromObj, "ALL").size(), 4u);
  }

  /** Test adding/finding using RelationsObject/RelationsInterface. */
  TEST_F(RelationTest, RelationsObject)
  {
    DataStore::Instance().setInitializeActive(true);
    RelationArray(relObjData, profileData).registerAsPersistent();
    DataStore::Instance().setInitializeActive(false);

    bool ret = (relObjData)[0]->addRelationTo((profileData)[0], -42.0);
    EXPECT_TRUE(ret);

    RelationVector<ProfileInfo> rels = (relObjData)[0]->getRelationsTo<ProfileInfo>();
    EXPECT_TRUE(rels.size() == 1);
    EXPECT_TRUE(rels.object(0) == (profileData)[0]);
    EXPECT_DOUBLE_EQ(rels.weight(0), -42.0);

    //should be safe
    EXPECT_FALSE((relObjData)[0]->addRelationTo(static_cast<TObject*>(nullptr)));
    ProfileInfo notInArray;
    EXPECT_FALSE((relObjData)[0]->addRelationTo(&notInArray));
  }

  /** Test updating of index after using addRelation. */
  TEST_F(RelationTest, IndexUpdating)
  {
    DataStore::Instance().setInitializeActive(true);
    RelationArray(relObjData, profileData).registerAsPersistent();
    DataStore::Instance().setInitializeActive(false);

    //not yet set
    EXPECT_FALSE((relObjData)[0]->getRelated<ProfileInfo>() != NULL);

    bool ret = (relObjData)[0]->addRelationTo((profileData)[0], -42.0);
    EXPECT_TRUE(ret);

    //now it should be found (index updated because RelationContainer was just created)
    EXPECT_TRUE((relObjData)[0]->getRelated<ProfileInfo>() != NULL);

    //test again with different object
    EXPECT_FALSE((relObjData)[1]->getRelated<ProfileInfo>() != NULL);

    ret = (relObjData)[1]->addRelationTo((profileData)[0], -42.0);
    EXPECT_TRUE(ret);

    //now it should be found (index updated because addRelation marks RelationContainer as modified)
    EXPECT_TRUE((relObjData)[1]->getRelated<ProfileInfo>() != NULL);
  }

  /** Test getting array name/index from a RelationsObject. */
  TEST_F(RelationTest, RelationsObjectArrayIndex)
  {
    for (int i = 0; i < relObjData.getEntries(); i++) {
      EXPECT_TRUE((relObjData)[i]->getArrayName() == relObjData.getName());
      EXPECT_TRUE((relObjData)[i]->getArrayIndex() == i);
    }

    RelationsObject bla;
    EXPECT_TRUE(bla.getArrayName() == "");
    EXPECT_TRUE(bla.getArrayIndex() == -1);
  }

  /** Check behaviour of duplicate relations. */
  TEST_F(RelationTest, DuplicateRelations)
  {
    DataStore::Instance().setInitializeActive(true);
    RelationArray(evtData, relObjData).registerAsPersistent();
    RelationArray(relObjData, evtData).registerAsPersistent();
    DataStore::Instance().setInitializeActive(false);

    //more than a single relation in one direction
    DataStore::Instance().addRelationFromTo((evtData)[0], (relObjData)[1], 1.0);
    DataStore::Instance().addRelationFromTo((evtData)[0], (relObjData)[1], 2.0);

    //since the relation wasn't consolidated, these should still show up as
    //seperate things
    RelationVector<EventMetaData> rels1 = (relObjData)[1]->getRelationsFrom<EventMetaData>();
    EXPECT_EQ(2u, rels1.size());


    //add another one in opposite direction
    DataStore::Instance().addRelationFromTo((relObjData)[1], (evtData)[0], 1.0);
    RelationVector<EventMetaData> rels2 = (relObjData)[1]->getRelationsFrom<EventMetaData>();
    //wasn't _from_ eventmetadata, so no change
    EXPECT_EQ(2u, rels2.size());

    RelationVector<EventMetaData> rels3 = (relObjData)[1]->getRelationsWith<EventMetaData>();
    EXPECT_EQ(3u, rels3.size());
    double sum = 0.0;
    for (int i = 0; i < (int)rels3.size(); i++) {
      sum += rels3.weight(i);
    }
    EXPECT_DOUBLE_EQ(sum, 1.0 + 1.0 + 2.0);
  }


}  // namespace
