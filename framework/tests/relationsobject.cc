/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <framework/datastore/StoreArray.h>
#include <framework/dataobjects/EventMetaData.h>
#include <framework/dataobjects/ProfileInfo.h>
#include <framework/datastore/RelationsObject.h>
#include <framework/utilities/TestHelpers.h>

#include <gtest/gtest.h>

using namespace std;
using namespace Belle2;

namespace {
  /** test relations. */
  class RelationsObjectTest : public ::testing::Test {
  protected:
    /** fill StoreArrays with entries from 0..9 */
    void SetUp() override
    {
      evtData.registerInDataStore();
      profileData.registerInDataStore();
      relObjData.registerInDataStore();

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

    StoreArray<EventMetaData> evtData; /**< event data array */
    StoreArray<ProfileInfo> profileData; /**< run data array */
    StoreArray<RelationsObject> relObjData; /**< some objects to test RelationsInterface. */
  };

  /** Test adding/finding using RelationsObject/RelationsInterface. */
  TEST_F(RelationsObjectTest, RelationsObject)
  {
    relObjData.registerRelationTo(profileData);
    DataStore::Instance().setInitializeActive(false);

    (relObjData)[0]->addRelationTo((profileData)[0], -42.0);

    //getRelations
    RelationVector<ProfileInfo> rels = (relObjData)[0]->getRelationsTo<ProfileInfo>();
    EXPECT_TRUE(rels.size() == 1);
    EXPECT_TRUE(rels.object(0) == (profileData)[0]);
    EXPECT_DOUBLE_EQ(rels.weight(0), -42.0);
    EXPECT_EQ(0u, relObjData[1]->getRelationsTo<ProfileInfo>().size());
    EXPECT_EQ(0u, relObjData[0]->getRelationsFrom<ProfileInfo>().size());
    EXPECT_EQ(1u, relObjData[0]->getRelationsWith<ProfileInfo>().size());

    //getRelated
    EXPECT_TRUE(profileData[0] == relObjData[0]->getRelatedTo<ProfileInfo>());
    EXPECT_TRUE(nullptr == relObjData[1]->getRelatedTo<ProfileInfo>());
    EXPECT_TRUE(nullptr == relObjData[0]->getRelatedFrom<ProfileInfo>());
    EXPECT_TRUE(profileData[0] == relObjData[0]->getRelated<ProfileInfo>());

    //getRelatedWithWeight
    EXPECT_TRUE(std::make_pair(profileData[0], -42.0f) == relObjData[0]->getRelatedToWithWeight<ProfileInfo>());
    ProfileInfo* profileNullPtr = nullptr;
    EXPECT_TRUE(std::make_pair(profileNullPtr, 1.0f) == relObjData[1]->getRelatedToWithWeight<ProfileInfo>());
    EXPECT_TRUE(std::make_pair(profileNullPtr, 1.0f) == relObjData[0]->getRelatedFromWithWeight<ProfileInfo>());
    EXPECT_TRUE(std::make_pair(profileData[0], -42.0f) == relObjData[0]->getRelatedWithWeight<ProfileInfo>());


    //adding relations to NULL is safe and doesn't do anything
    (relObjData)[0]->addRelationTo(static_cast<TObject*>(nullptr));
    (relObjData)[0]->addRelationTo(static_cast<ProfileInfo*>(nullptr));

    //if we cannot create a relation to an actual object given, this is obivously wrong
    ProfileInfo notInArray;
    EXPECT_B2FATAL((relObjData)[0]->addRelationTo(&notInArray));
  }

  /** Test updating of index after using addRelation. */
  TEST_F(RelationsObjectTest, IndexUpdating)
  {
    relObjData.registerRelationTo(profileData);
    DataStore::Instance().setInitializeActive(false);

    //not yet set
    EXPECT_FALSE((relObjData)[0]->getRelated<ProfileInfo>() != nullptr);

    (relObjData)[0]->addRelationTo((profileData)[0], -42.0);

    //now it should be found (index updated because RelationContainer was just created)
    EXPECT_TRUE((relObjData)[0]->getRelated<ProfileInfo>() != nullptr);

    //test again with different object
    EXPECT_FALSE((relObjData)[1]->getRelated<ProfileInfo>() != nullptr);

    (relObjData)[1]->addRelationTo((profileData)[0], -42.0);

    //now it should be found (index updated because addRelation marks RelationContainer as modified)
    EXPECT_TRUE((relObjData)[1]->getRelated<ProfileInfo>() != nullptr);
  }

  /** Test getting array name/index from a RelationsObject. */
  TEST_F(RelationsObjectTest, RelationsObjectArrayIndex)
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
  TEST_F(RelationsObjectTest, DuplicateRelations)
  {
    evtData.registerRelationTo(relObjData);
    relObjData.registerRelationTo(evtData);
    DataStore::Instance().setInitializeActive(false);

    //more than a single relation in one direction
    DataStore::Instance().addRelationFromTo((evtData)[0], (relObjData)[1], 1.0);
    DataStore::Instance().addRelationFromTo((evtData)[0], (relObjData)[1], 2.0);

    //since the relation wasn't consolidated, these should still show up as
    //seperate things
    RelationVector<EventMetaData> rels1 = (relObjData)[1]->getRelationsFrom<EventMetaData>();
    EXPECT_EQ(2u, rels1.size());


    //preserve order
    EXPECT_FLOAT_EQ(1.0, rels1.weight(0));
    EXPECT_FLOAT_EQ(2.0, rels1.weight(1));
    EXPECT_FLOAT_EQ(1.0, relObjData[1]->getRelatedFromWithWeight<EventMetaData>().second);


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

  TEST_F(RelationsObjectTest, RelationsToSameArray)
  {
    relObjData.registerRelationTo(relObjData);
    DataStore::Instance().setInitializeActive(false);

    relObjData[0]->addRelationTo(relObjData[1]);
    EXPECT_TRUE(relObjData[0] == relObjData[1]->getRelated<RelationsObject>());
    EXPECT_TRUE(relObjData[0] == relObjData[1]->getRelatedFrom<RelationsObject>());
    EXPECT_TRUE(relObjData[1] == relObjData[0]->getRelated<RelationsObject>());
    EXPECT_TRUE(relObjData[1] == relObjData[0]->getRelatedTo<RelationsObject>());
    EXPECT_TRUE(nullptr == relObjData[2]->getRelated<RelationsObject>());
    EXPECT_TRUE(nullptr == relObjData[2]->getRelatedFrom<RelationsObject>());
    EXPECT_TRUE(nullptr == relObjData[2]->getRelatedTo<RelationsObject>());
    //still in one direction
    EXPECT_TRUE(nullptr == relObjData[1]->getRelatedTo<RelationsObject>());
    EXPECT_TRUE(nullptr == relObjData[0]->getRelatedFrom<RelationsObject>());

    //to same object
    relObjData[3]->addRelationTo(relObjData[3]);
    EXPECT_TRUE(relObjData[3] == relObjData[3]->getRelated<RelationsObject>());
    EXPECT_TRUE(relObjData[3] == relObjData[3]->getRelatedFrom<RelationsObject>());
    EXPECT_TRUE(relObjData[3] == relObjData[3]->getRelatedTo<RelationsObject>());
  }

  TEST_F(RelationsObjectTest, ModifyRelations)
  {
    relObjData.registerRelationTo(profileData);
    DataStore::Instance().setInitializeActive(false);

    (relObjData)[0]->addRelationTo((profileData)[0], -42.0);

    //weights
    RelationVector<ProfileInfo> rels = (relObjData)[0]->getRelationsTo<ProfileInfo>();
    EXPECT_DOUBLE_EQ(rels.weight(0), -42.0);
    rels.setWeight(0, -3.0);
    EXPECT_DOUBLE_EQ(rels.weight(0), -3.0); //updated immediately
    RelationVector<ProfileInfo> rels2 = (relObjData)[0]->getRelationsTo<ProfileInfo>();
    EXPECT_DOUBLE_EQ(rels2.weight(0), -3.0); //and in DataStore

    //removal
    EXPECT_EQ(1u, relObjData[0]->getRelationsTo<ProfileInfo>().size());
    EXPECT_EQ(1u, relObjData[0]->getRelationsWith<ProfileInfo>().size());
    rels2.remove(0);
    EXPECT_EQ(0u, rels2.size());
    EXPECT_EQ(0u, relObjData[0]->getRelationsTo<ProfileInfo>().size());
    EXPECT_EQ(0u, relObjData[0]->getRelationsWith<ProfileInfo>().size());
  }

  /** Test using named relations */
  TEST_F(RelationsObjectTest, NamedRelationsWithInvalidName)
  {
    const std::string relationName = "ExtraRelation WithSpaceInName";
    EXPECT_B2FATAL(relObjData.registerRelationTo(profileData, DataStore::c_Event, DataStore::c_WriteOut, relationName));
  }

  /** Test using named relations */
  TEST_F(RelationsObjectTest, NamedRelations)
  {
    const std::string relationName = "ExtraRelation";
    relObjData.registerRelationTo(profileData, DataStore::c_Event, DataStore::c_WriteOut, relationName);
    DataStore::Instance().setInitializeActive(false);

    //check the hasRelation finder works with and without names
    EXPECT_TRUE(relObjData.hasRelationTo(profileData, DataStore::c_Event, relationName));
    EXPECT_FALSE(profileData.hasRelationTo(relObjData, DataStore::c_Event, relationName));
    EXPECT_FALSE(profileData.hasRelationTo(evtData, DataStore::c_Event, relationName));
    EXPECT_FALSE(relObjData.hasRelationTo(profileData));
    EXPECT_FALSE(profileData.hasRelationTo(relObjData));

    (relObjData)[0]->addRelationTo((profileData)[0], -42.0, relationName);

    //getRelations
    RelationVector<ProfileInfo> rels = (relObjData)[0]->getRelationsTo<ProfileInfo>("", relationName);
    EXPECT_TRUE(rels.size() == 1);
    EXPECT_TRUE(rels.object(0) == (profileData)[0]);
    EXPECT_DOUBLE_EQ(rels.weight(0), -42.0);
    EXPECT_EQ(0u, relObjData[1]->getRelationsTo<ProfileInfo>("", relationName).size());
    EXPECT_EQ(0u, relObjData[0]->getRelationsFrom<ProfileInfo>("", relationName).size());
    EXPECT_EQ(1u, relObjData[0]->getRelationsWith<ProfileInfo>("", relationName).size());

    //getRelated
    EXPECT_TRUE(profileData[0] == relObjData[0]->getRelatedTo<ProfileInfo>("", relationName));
    EXPECT_TRUE(nullptr == relObjData[1]->getRelatedTo<ProfileInfo>("", relationName));
    EXPECT_TRUE(nullptr == relObjData[0]->getRelatedFrom<ProfileInfo>("", relationName));
    EXPECT_TRUE(profileData[0] == relObjData[0]->getRelated<ProfileInfo>("", relationName));

    //getRelatedWithWeight
    EXPECT_TRUE(std::make_pair(profileData[0], -42.0f) == relObjData[0]->getRelatedToWithWeight<ProfileInfo>("", relationName));
    ProfileInfo* profileNullPtr = nullptr;
    EXPECT_TRUE(std::make_pair(profileNullPtr, 1.0f) == relObjData[1]->getRelatedToWithWeight<ProfileInfo>("", relationName));
    EXPECT_TRUE(std::make_pair(profileNullPtr, 1.0f) == relObjData[0]->getRelatedFromWithWeight<ProfileInfo>("", relationName));
    EXPECT_TRUE(std::make_pair(profileData[0], -42.0f) == relObjData[0]->getRelatedWithWeight<ProfileInfo>("", relationName));

    // Check if the "ALL" parameter also works
    StoreEntry* storeEntry = nullptr;
    int index = -1;
    DataStore::Instance().getRelationsWith(DataStore::c_FromSide, (relObjData)[0], storeEntry, index,
                                           TObject::Class(), "ALL", "");

    //adding relations to NULL is safe and doesn't do anything
    (relObjData)[0]->addRelationTo(static_cast<TObject*>(nullptr), 1.0, relationName);
    (relObjData)[0]->addRelationTo(static_cast<ProfileInfo*>(nullptr), 1.0, relationName);

    //if we cannot create a relation to an actual object given, this is obivously wrong
    ProfileInfo notInArray;
    EXPECT_B2FATAL((relObjData)[0]->addRelationTo(&notInArray, 1.0, relationName));
  }

}  // namespace
