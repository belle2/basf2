/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <framework/datastore/RelationIndex.h>
#include <framework/dataobjects/EventMetaData.h>
#include <framework/dataobjects/ProfileInfo.h>
#include <framework/datastore/RelationsObject.h>
#include <framework/utilities/TestHelpers.h>

#include <gtest/gtest.h>

using namespace std;
using namespace Belle2;

namespace {
  /** test relations. */
  class RelationsInternal : public ::testing::Test {
  protected:
    /** fill StoreArrays with entries from 0..9 */
    void SetUp() override
    {
      DataStore::Instance().setInitializeActive(true);
      evtData.registerInDataStore();
      profileData.registerInDataStore();
      relObjData.registerInDataStore();
      DataStore::Instance().setInitializeActive(false);

      for (int i = 0; i < 10; ++i) {
        evtData.appendNew();
        profileData.appendNew();
        relObjData.appendNew();
      }
    }

    /** clear datastore */
    void TearDown() override
    {
      DataStore::Instance().reset();
    }

    void findRelationsCheckContents();

    StoreArray<EventMetaData> evtData; /**< event data array */
    StoreArray<ProfileInfo> profileData; /**< run data array */
    StoreArray<RelationsObject> relObjData; /**< some objects to test RelationsInterface. */
  };

  /** Tests the creation of a Relation. */
  TEST_F(RelationsInternal, RelationCreate)
  {
    DataStore::Instance().setInitializeActive(true);
    evtData.registerRelationTo(profileData);
    DataStore::Instance().setInitializeActive(false);

    RelationArray relation(evtData, profileData);
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
  TEST_F(RelationsInternal, RelationFind)
  {
    DataStore::Instance().setInitializeActive(true);
    EXPECT_FALSE(evtData.optionalRelationTo(profileData));
    EXPECT_FALSE(evtData.requireRelationTo(profileData));

    StoreArray<EventMetaData> evtData2;
    evtData.registerRelationTo(profileData);
    evtData2.registerInDataStore("OwnName");
    evtData2.registerRelationTo(profileData);
    DataStore::Instance().setInitializeActive(false);

    RelationArray relation(evtData, profileData);
    EXPECT_FALSE(RelationArray(DataStore::relationName(evtData.getName(), profileData.getName()), DataStore::c_Event));
    relation.create();
    EXPECT_TRUE(RelationArray(evtData, profileData, "", DataStore::c_Event));
    string name = relation.getName();

    EXPECT_EQ("OwnName", evtData2.getName());


    RelationArray relationAttachedUsingName(name, DataStore::c_Event);
    //trying to get the accessor params should cause the array to attach (and thus get the appropriate data)
    EXPECT_TRUE(relationAttachedUsingName.getFromAccessorParams() == evtData.getAccessorParams());
    EXPECT_TRUE(relationAttachedUsingName.getToAccessorParams() == profileData.getAccessorParams());
    EXPECT_TRUE(relationAttachedUsingName);

    //check for OwnNameToProfileInfos
    EXPECT_FALSE(RelationArray(DataStore::relationName(evtData2.getName(), profileData.getName()), DataStore::c_Event));
    EXPECT_FALSE(RelationArray("OwnNameToProfileInfos", DataStore::c_Event));
    RelationArray relation2(evtData2, profileData);
    relation2.create();
    EXPECT_TRUE(relation2.getName() == "OwnNameToProfileInfos");
    EXPECT_TRUE(RelationArray(evtData2, profileData));

    //outside of initialize() this should fail
    EXPECT_B2FATAL(evtData.requireRelationTo(profileData));
  }

  /** Test that Relations wich points to the wrong arrays yields a FATAL. */
  TEST_F(RelationsInternal, RelationWrongDeathTest)
  {
    DataStore::Instance().setInitializeActive(true);
    RelationArray relation1(evtData, profileData, "test");
    relation1.registerInDataStore();
    DataStore::Instance().setInitializeActive(false);

    relation1.create();
    EXPECT_B2FATAL(RelationArray(profileData, evtData, "test").isValid());
    EXPECT_B2FATAL(RelationArray(profileData, evtData, "test").add(0, 0, 1.0));
    EXPECT_B2FATAL(RelationArray(profileData, evtData, "test")[0]);
    EXPECT_B2FATAL(RelationArray(profileData, evtData, "test").getModified());
  }

  /** */
  TEST_F(RelationsInternal, RegistrationWithDefaultNames)
  {
    //not registered yet
    EXPECT_B2FATAL(relObjData[0]->addRelationTo(profileData[3]));
    EXPECT_B2FATAL(DataStore::Instance().addRelationFromTo((evtData)[0], (profileData)[1], 2.0));

    DataStore::Instance().setInitializeActive(true);
    relObjData.registerRelationTo(profileData);
    evtData.registerRelationTo(profileData);
    DataStore::Instance().setInitializeActive(false);

    //profileData has default name, so this should be ok now
    relObjData[0]->addRelationTo(profileData[4]);
    DataStore::Instance().addRelationFromTo((evtData)[0], (profileData)[3], 2.0);
  }

  /** Some events may have default constructed relations (i.e. nothing
   *  was inserted and create() was never called). For those, we don't
   *  want the construction of the array fail with a FATAL.
   */
  TEST_F(RelationsInternal, RelationDefaultConstructed)
  {
    DataStore::Instance().setInitializeActive(true);
    RelationArray array(evtData, profileData);
    array.registerInDataStore();
    DataStore::Instance().setInitializeActive(false);

    auto* rel = new RelationContainer(); //default constructed object, as written to file
    ASSERT_TRUE(DataStore::Instance().createObject(rel, false, array));

    EXPECT_FALSE(array.isValid());

    //shouldn't die here
    RelationIndex<EventMetaData, ProfileInfo> index(evtData, profileData);
  }

  /** Check creation of an index. */
  TEST_F(RelationsInternal, BuildIndex)
  {
    DataStore::Instance().setInitializeActive(true);
    evtData.registerRelationTo(profileData);
    DataStore::Instance().setInitializeActive(false);

    RelationArray relation(evtData, profileData);
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
    EXPECT_TRUE(relIndex.getFirstElementTo(nullptr) == nullptr);
    EXPECT_TRUE(relIndex.getFirstElementFrom(nullptr) == nullptr);
    EXPECT_TRUE(relIndex.getFirstElementFrom(nullptr) == nullptr);
    EXPECT_TRUE(relIndex.getFirstElementFrom((evtData)[4]) == nullptr);
    EXPECT_TRUE(relIndex.getFirstElementTo((profileData)[3]) == nullptr);

    //check size of found element lists
    {
      int size(0);
      double allweights(0);
      for (auto& e : relIndex.getElementsFrom((evtData)[0])) {
        ++size;
        allweights += e.weight;
      }
      EXPECT_EQ(size, 3);
      EXPECT_DOUBLE_EQ(allweights, 6.0);
    }
    {
      int size(0);
      double allweights(0);
      for (auto& e : relIndex.getElementsTo((profileData)[0])) {
        ++size;
        allweights += e.weight;
      }
      EXPECT_EQ(size, 2);
      EXPECT_DOUBLE_EQ(allweights, 2.0);
    }
    {
      int size(0);
      double allweights(0);
      for (const auto& e : relIndex.getElementsTo((profileData)[4])) {
        ++size;
        allweights += e.weight;
      }
      EXPECT_EQ(size, 0);
      EXPECT_DOUBLE_EQ(allweights, 0.0);
    }
  }

  /** Check wether out-of-bound indices are caught by RelationIndex. */
  TEST_F(RelationsInternal, InconsistentIndexDeathTest)
  {
    DataStore::Instance().setInitializeActive(true);
    evtData.registerRelationTo(profileData);
    DataStore::Instance().setInitializeActive(false);

    RelationArray relation(evtData, profileData);
    relation.add(0, 10, 1.0);
    using rel_t =  RelationIndex<EventMetaData, ProfileInfo>;
    EXPECT_B2FATAL(rel_t relIndex);
    relation.clear();
    relation.add(10, 0, 1.0);
    EXPECT_B2FATAL(rel_t relIndex);
  }


  /** Check behaviour when attaching to non-existing(=empty) relation */
  TEST_F(RelationsInternal, EmptyIndex)
  {
    DataStore::Instance().setInitializeActive(true);
    evtData.registerRelationTo(profileData);
    DataStore::Instance().setInitializeActive(false);

    RelationIndex<EventMetaData, ProfileInfo> index;
    EXPECT_FALSE(index);
    EXPECT_EQ(index.size(), 0u);
    EXPECT_EQ(index.getFromAccessorParams().first, "");
    EXPECT_EQ(index.getToAccessorParams().first, "");
  }

  /** Attaching to relation with from and two swapped, and with different StoreArray of same type. */
  TEST_F(RelationsInternal, WrongRelationIndexDeathTest)
  {
    StoreArray<EventMetaData> eventData("evts");

    DataStore::Instance().setInitializeActive(true);
    RelationArray relation(profileData, evtData, "test");
    relation.registerInDataStore();
    RelationArray(evtData, profileData).registerInDataStore();
    eventData.registerInDataStore();
    RelationArray relation2(evtData, profileData, "test2");
    relation2.registerInDataStore();
    DataStore::Instance().setInitializeActive(false);

    relation.create();
    using rel_t =  RelationIndex<EventMetaData, ProfileInfo>;
    EXPECT_B2FATAL(rel_t(evtData, profileData, "test"));
    EXPECT_B2FATAL(rel_t("test"));

    relation2.create();
    EXPECT_B2FATAL(rel_t(eventData, profileData, "test2"));

    //This relation works and points to evtData, not eventData.
    //no check is performed, user is responsible to check
    //using getFromAccessorParams and getToAccessorParams
    EXPECT_TRUE(rel_t("test2"));
  }

  /** Check contents of previously created relations. */
  void RelationsInternal::findRelationsCheckContents()
  {
    const EventMetaData* fromObj = (evtData)[0];
    RelationVector<ProfileInfo> toRels = DataStore::getRelationsWithObj<ProfileInfo>(fromObj);
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
    for (const ProfileInfo& prof : toRels) {
      (void)prof; //variable unused
      i++;
    }
    EXPECT_EQ(i, 3);

    const ProfileInfo* toObj = (profileData)[2];
    RelationVector<EventMetaData> fromRels = DataStore::getRelationsWithObj<EventMetaData>(toObj);
    EXPECT_EQ(fromRels.size(), 1u);
    EXPECT_DOUBLE_EQ(fromRels.weight(0), -3.0);
    EXPECT_TRUE(fromRels.object(0) == fromObj);
    EXPECT_TRUE(fromRels[0] == fromObj);

    //some things that shouldn't return anything
    EXPECT_EQ(DataStore::getRelationsWithObj<EventMetaData>(fromObj).size(), 0u);
  }

  /** Test DataStore members for finding relations. */
  TEST_F(RelationsInternal, FindRelations)
  {
    DataStore::Instance().setInitializeActive(true);
    evtData.registerRelationTo(profileData);
    DataStore::Instance().setInitializeActive(false);

    //check non-existing relations (registered)
    const EventMetaData* fromObj = (evtData)[0];
    RelationVector<ProfileInfo> toRels = DataStore::getRelationsWithObj<ProfileInfo>(fromObj);
    EXPECT_EQ(toRels.size(), 0u);
    const ProfileInfo* toObj = (profileData)[2];
    RelationVector<EventMetaData> fromRels = DataStore::getRelationsWithObj<EventMetaData>(toObj);
    EXPECT_EQ(fromRels.size(), 0u);

    //check non-existing relations (unregistered)
    RelationVector<EventMetaData> toRels2 = DataStore::getRelationsWithObj<EventMetaData>(fromObj);
    EXPECT_EQ(toRels2.size(), 0u);
    RelationVector<ProfileInfo> fromRels2 = DataStore::getRelationsWithObj<ProfileInfo>(toObj);
    EXPECT_EQ(fromRels2.size(), 0u);

    RelationArray relation(evtData, profileData);
    relation.add(0, 0, 1.0);
    relation.add(0, 1, 2.0);
    relation.add(0, 2, -3.0);

    findRelationsCheckContents();


    //check that results don't change after consolidation
    relation.consolidate();
    findRelationsCheckContents();
  }

  /** Test DataStore members for adding relations. */
  TEST_F(RelationsInternal, AddRelations)
  {
    DataStore::Instance().setInitializeActive(true);
    evtData.registerRelationTo(profileData);
    DataStore::Instance().setInitializeActive(false);

    DataStore::Instance().addRelationFromTo((evtData)[0], (profileData)[0], 1.0);
    DataStore::Instance().addRelationFromTo((evtData)[0], (profileData)[1], 2.0);
    DataStore::Instance().addRelationFromTo((evtData)[0], (profileData)[2], -3.0);

    findRelationsCheckContents();
  }

  /** Test DataStore::getRelationsWith. */
  TEST_F(RelationsInternal, GetRelationsWith)
  {
    DataStore::Instance().setInitializeActive(true);
    evtData.registerRelationTo(profileData);
    DataStore::Instance().setInitializeActive(false);

    DataStore::Instance().addRelationFromTo((evtData)[0], (profileData)[0], 1.0);
    DataStore::Instance().addRelationFromTo((evtData)[0], (profileData)[1], 2.0);
    DataStore::Instance().addRelationFromTo((evtData)[0], (profileData)[2], 3.0);

    //some objects with no relations to given type
    EXPECT_EQ(DataStore::getRelationsWithObj<EventMetaData>((evtData)[0]).size(), 0u);
    EXPECT_EQ(DataStore::getRelationsWithObj<EventMetaData>((profileData)[3]).size(), 0u);

    RelationVector<ProfileInfo> profRels = DataStore::getRelationsWithObj<ProfileInfo>((evtData)[0]);
    EXPECT_EQ(profRels.size(), 3u);
    EXPECT_EQ(profRels.weight(0), 1.0); //should be positive

    RelationVector<EventMetaData> eventRels = DataStore::getRelationsWithObj<EventMetaData>((profileData)[0]);
    EXPECT_EQ(eventRels.size(), 1u);
    EXPECT_EQ(eventRels.weight(0), 1.0); //points to given object, same weight
  }

  /** Test searching all "ALL" storearrays for objects. */
  TEST_F(RelationsInternal, SearchAll)
  {
    //2nd array of this type
    StoreArray<ProfileInfo> profileData2("ProfileInfos2");

    DataStore::Instance().setInitializeActive(true);
    profileData2.registerInDataStore();
    evtData.registerRelationTo(profileData);
    evtData.registerRelationTo(profileData2);
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
    RelationVector<ProfileInfo> toRels = DataStore::getRelationsWithObj<ProfileInfo>(fromObj, "ALL");
    EXPECT_EQ(toRels.size(), 4u);
    //order might be anything, check sum of weights
    double sum = 0.0;
    for (int i = 0; i < (int)toRels.size(); i++) {
      sum += toRels.weight(i);
    }
    EXPECT_DOUBLE_EQ(sum, 42.0 + 1 + 2 - 3);

    //finding with default TO name
    EXPECT_EQ(DataStore::getRelationsWithObj<ProfileInfo>(fromObj, profileData.getName()).size(), 3u);
    //finding with TO name of 2nd array
    EXPECT_EQ(DataStore::getRelationsWithObj<ProfileInfo>(fromObj, profileData2.getName()).size(), 1u);
    //and something that doesn't exist
    EXPECT_EQ(DataStore::getRelationsWithObj<ProfileInfo>(fromObj, "DoesntExist").size(), 0u);

    const ProfileInfo* toObj = profileData2[0];
    //object should also be found without specifying the name
    EXPECT_EQ(DataStore::getRelationsWithObj<EventMetaData>(toObj).size(), 1u);
    //or with 'ALL"
    EXPECT_EQ(DataStore::getRelationsWithObj<EventMetaData>(toObj, "ALL").size(), 1u);
    //and using a base class
    EXPECT_EQ(DataStore::getRelationsWithObj<TObject>(toObj, "ALL").size(), 1u);

    //no relations to this type
    EXPECT_EQ(DataStore::getRelationsWithObj<EventMetaData>(fromObj, "ALL").size(), 0u);
    //should work again
    EXPECT_EQ(DataStore::getRelationsWithObj<TObject>(fromObj, "ALL").size(), 4u);

  }


  TEST_F(RelationsInternal, ListOfRelatedArrays)
  {
    //2nd array of this type
    StoreArray<ProfileInfo> profileData2("ProfileInfos2");

    //nothing
    EXPECT_EQ(0u, DataStore::Instance().getListOfRelatedArrays(profileData2).size());
    EXPECT_EQ(0u, DataStore::Instance().getListOfRelatedArrays(profileData).size());
    EXPECT_EQ(0u, DataStore::Instance().getListOfRelatedArrays(evtData).size());

    DataStore::Instance().setInitializeActive(true);
    profileData2.registerInDataStore();
    evtData.registerRelationTo(profileData);
    evtData.registerRelationTo(profileData2);
    DataStore::Instance().setInitializeActive(false);

    EXPECT_EQ(1u, DataStore::Instance().getListOfRelatedArrays(profileData2).size());
    EXPECT_EQ(1u, DataStore::Instance().getListOfRelatedArrays(profileData).size());
    EXPECT_EQ(2u, DataStore::Instance().getListOfRelatedArrays(evtData).size());

    EXPECT_EQ(evtData.getName(), DataStore::Instance().getListOfRelatedArrays(profileData2).at(0));
    EXPECT_EQ(evtData.getName(), DataStore::Instance().getListOfRelatedArrays(profileData).at(0));
  }

  TEST_F(RelationsInternal, StoreArray_clear_cleans_relations)
  {
    DataStore::Instance().setInitializeActive(true);
    evtData.registerRelationTo(relObjData);
    relObjData.registerRelationTo(profileData);
    evtData.registerRelationTo(profileData);
    DataStore::Instance().setInitializeActive(false);

    DataStore::Instance().addRelationFromTo((evtData)[0], (relObjData)[0], 1.0);
    DataStore::Instance().addRelationFromTo((relObjData)[1], (profileData)[9], 1.0);
    DataStore::Instance().addRelationFromTo((evtData)[1], (profileData)[1], 2.0);

    evtData.clear();
    profileData.clear();

    EXPECT_EQ(DataStore::getRelationsWithObj<ProfileInfo>(relObjData[1]).size(), 0u);
    EXPECT_EQ(DataStore::getRelationsWithObj<EventMetaData>(relObjData[0]).size(), 0u);
  }
}  // namespace
