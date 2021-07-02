/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <framework/dataobjects/EventMetaData.h>
#include <framework/dataobjects/ProfileInfo.h>
#include <framework/datastore/RelationArray.h>
#include <framework/datastore/StoreArray.h>

#include <gtest/gtest.h>

using namespace std;
using namespace Belle2;

namespace {
  /** test relation consolidation. */
  class RelationConsolidateTest : public ::testing::Test {
  protected:
    /** fill StoreArrays with entries from 0..9 */
    void SetUp() override
    {
      evtData = new StoreArray<EventMetaData>;
      profileData = new StoreArray<ProfileInfo>;
      DataStore::Instance().setInitializeActive(true);
      evtData->registerInDataStore();
      profileData->registerInDataStore();
      evtData->registerRelationTo(*profileData);
      DataStore::Instance().setInitializeActive(false);

      for (int i = 0; i < 10; ++i) {
        evtData->appendNew();
        profileData->appendNew();
      }

      relArray = new RelationArray(*evtData, *profileData);
      relArray->add(0, 0, 1.0);
      relArray->add(0, 1, 2.0);
      relArray->add(0, 1, 3.0);
      relArray->add(1, 0, 1.0);
      relArray->add(1, 1, 2.0);
      relArray->add(1, 2, 3.0);
    }

    /** clear datastore */
    void TearDown() override
    {
      DataStore::Instance().reset();
      delete evtData;
      delete profileData;
      delete relArray;
    }

    StoreArray<EventMetaData>* evtData; /**< event data array */
    StoreArray<ProfileInfo>* profileData; /**< run data array */
    RelationArray* relArray; /**< relation between those two */
  };

  /** Check consolidation of RelationElements. */
  TEST_F(RelationConsolidateTest, RelationConsolidateDefault)
  {
    RelationArray& relation = *relArray;
    relation.consolidate();
    ASSERT_EQ(relation.getEntries(), 2);
    EXPECT_EQ(relation[0].getSize(), 2u);
    EXPECT_EQ(relation[0].getWeight(0), 1.0);
    EXPECT_EQ(relation[0].getWeight(1), 5.0);
    EXPECT_EQ(relation[1].getSize(), 3u);
    EXPECT_EQ(relation[1].getWeight(0), 1.0);
    EXPECT_EQ(relation[1].getWeight(1), 2.0);
    EXPECT_EQ(relation[1].getWeight(2), 3.0);

    std::map<unsigned int, std::pair<unsigned int, bool> > replace;
    replace[0] = std::make_pair(1, true);
    replace[2] = std::make_pair(3, false);
    RelationArray::ReplaceMap<> replaceMap(replace);
    relation.consolidate(replaceMap, replaceMap);
    ASSERT_EQ(relation.getEntries(), 1);
    EXPECT_EQ(relation[0].getSize(), 2u);
    EXPECT_EQ(relation[0].getWeight(0), 9.0);
    EXPECT_EQ(relation[0].getWeight(1), 3.0);
    EXPECT_EQ(relation[0].getFromIndex(), 1u);
    EXPECT_EQ(relation[0].getToIndex(0), 1u);
    EXPECT_EQ(relation[0].getToIndex(1), 3u);
  }

  /** Check consolidation of RelationElements when some weights should be set to zero */
  TEST_F(RelationConsolidateTest, RelationConsolidateZero)
  {
    RelationArray& relation = *relArray;
    std::map<unsigned int, std::pair<unsigned int, bool> > replace;
    replace[0] = std::make_pair(1, true);
    replace[2] = std::make_pair(3, false);
    RelationArray::ReplaceMap<> replaceMap(replace);
    relation.consolidate(replaceMap, replaceMap, RelationArray::c_zeroWeight);
    ASSERT_EQ(relation.getEntries(), 1);
    EXPECT_EQ(relation[0].getSize(), 2u);
    EXPECT_EQ(relation[0].getWeight(0), 2.0);
    EXPECT_EQ(relation[0].getWeight(1), 3.0);
    EXPECT_EQ(relation[0].getFromIndex(), 1u);
    EXPECT_EQ(relation[0].getToIndex(0), 1u);
    EXPECT_EQ(relation[0].getToIndex(1), 3u);

    //second run, check that weight of the second element changes to zero and index 3->2
    std::map<unsigned int, std::pair<unsigned int, bool> > replace2;
    replace2[3] = std::make_pair(2, true);
    RelationArray::ReplaceMap<> replaceMap2(replace2);
    relation.consolidate(replaceMap2, replaceMap2, RelationArray::c_zeroWeight);
    ASSERT_EQ(relation.getEntries(), 1);
    EXPECT_EQ(relation[0].getSize(), 2u);
    EXPECT_EQ(relation[0].getWeight(0), 2.0);
    EXPECT_EQ(relation[0].getWeight(1), 0.0);
    EXPECT_EQ(relation[0].getFromIndex(), 1u);
    EXPECT_EQ(relation[0].getToIndex(0), 1u);
    EXPECT_EQ(relation[0].getToIndex(1), 2u);
  }

  /** Check consolidation of RelationElements when some should get a negative weight in the process */
  TEST_F(RelationConsolidateTest, RelationConsolidateNegative)
  {
    RelationArray& relation = *relArray;
    std::map<unsigned int, std::pair<unsigned int, bool> > replace;
    replace[0] = std::make_pair(1, true);
    replace[2] = std::make_pair(3, false);
    RelationArray::ReplaceMap<> replaceMap(replace);
    relation.consolidate(replaceMap, replaceMap, RelationArray::c_negativeWeight);
    ASSERT_EQ(relation.getEntries(), 1);
    EXPECT_EQ(relation[0].getSize(), 2u);
    // resulting weight should be -1 + -2 + -3 + -1 + 2 = -5
    EXPECT_EQ(relation[0].getWeight(0), -5.0);
    EXPECT_EQ(relation[0].getWeight(1), 3.0);
    EXPECT_EQ(relation[0].getFromIndex(), 1u);
    EXPECT_EQ(relation[0].getToIndex(0), 1u);
    EXPECT_EQ(relation[0].getToIndex(1), 3u);

    //Second run, check that the weight for the second element goes to -3.0 but -5 should stay -5
    std::map<unsigned int, std::pair<unsigned int, bool> > replace2;
    replace2[1] = std::make_pair(1, true);
    replace2[3] = std::make_pair(2, true);
    RelationArray::ReplaceMap<> replaceMap2(replace2);
    relation.consolidate(replaceMap2, replaceMap2, RelationArray::c_negativeWeight);
    ASSERT_EQ(relation.getEntries(), 1);
    EXPECT_EQ(relation[0].getSize(), 2u);
    EXPECT_EQ(relation[0].getWeight(0), -5.0);
    EXPECT_EQ(relation[0].getWeight(1), -3.0);
    EXPECT_EQ(relation[0].getFromIndex(), 1u);
    EXPECT_EQ(relation[0].getToIndex(0), 1u);
    EXPECT_EQ(relation[0].getToIndex(1), 2u);
  }

  /** Check consolidation of RelationElements if some elements should get deleted in the process */
  TEST_F(RelationConsolidateTest, RelationConsolidateDelete)
  {
    RelationArray& relation = *relArray;
    std::map<unsigned int, std::pair<unsigned int, bool> > replace;
    replace[0] = std::make_pair(1, true);
    replace[2] = std::make_pair(3, false);
    RelationArray::ReplaceMap<> replaceMap(replace);
    relation.consolidate(replaceMap, replaceMap, RelationArray::c_deleteElement);
    ASSERT_EQ(relation.getEntries(), 1);
    EXPECT_EQ(relation[0].getSize(), 2u);
    EXPECT_EQ(relation[0].getWeight(0), 2.0);
    EXPECT_EQ(relation[0].getWeight(1), 3.0);
    EXPECT_EQ(relation[0].getFromIndex(), 1u);
    EXPECT_EQ(relation[0].getToIndex(0), 1u);
    EXPECT_EQ(relation[0].getToIndex(1), 3u);

    //No let's delete the second element
    std::map<unsigned int, std::pair<unsigned int, bool> > replace2;
    replace2[3] = std::make_pair(3, true);
    RelationArray::ReplaceMap<> replaceMap2(replace2);
    relation.consolidate(replaceMap2, replaceMap2, RelationArray::c_deleteElement);
    ASSERT_EQ(relation.getEntries(), 1);
    EXPECT_EQ(relation[0].getSize(), 1u);
    EXPECT_EQ(relation[0].getWeight(0), 2.0);
    EXPECT_EQ(relation[0].getFromIndex(), 1u);
    EXPECT_EQ(relation[0].getToIndex(0), 1u);

    //Check that we get an empty relation if we delete the last one
    std::map<unsigned int, std::pair<unsigned int, bool> > replace3;
    replace3[1] = std::make_pair(2, true);
    RelationArray::ReplaceMap<> replaceMap3(replace3);
    relation.consolidate(replaceMap3, replaceMap3, RelationArray::c_deleteElement);
    ASSERT_EQ(relation.getEntries(), 0);
  }

}  // namespace
