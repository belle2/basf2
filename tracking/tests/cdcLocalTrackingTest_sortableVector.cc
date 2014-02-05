/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost <oliver.frost@desy.de>                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <gtest/gtest.h>
#include "cdcLocalTrackingTest.h"

#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>

#include <tracking/cdcLocalTracking/eventdata/entities/CDCGenHit.h>
#include <tracking/cdcLocalTracking/eventdata/entities/Compare.h>

#include <tracking/cdcLocalTracking/algorithms/SortableVector.h>

#include <tracking/cdcLocalTracking/topology/CDCWire.h>
#include <tracking/cdcLocalTracking/topology/CDCWireTopology.h>

using namespace std;

using namespace Belle2;
using namespace CDCLocalTracking;

// Tests if SortableVector can handle CDCGenHits
TEST_F(CDCLocalTrackingTest, SortableVector_empty)
{

  const CDCWireTopology& wireTopology  = CDCWireTopology::getInstance();
  const CDCWire& firstWire = wireTopology.first();

  CDCGenHit genHit(firstWire, Vector2D(0.0, 0.0));

  SortableVector<CDCGenHit> collection;

  ASSERT_TRUE(collection.empty());

  collection.push_back(genHit);

  ASSERT_FALSE(collection.empty());

}

TEST_F(CDCLocalTrackingTest, SortableVector_range)
{

  CDCGenHit lowestGenHit;
  CDCGenHit largerGenHitOnFirstWire(WireID(0, 0, 0), Vector2D(0.0, 1.0));
  SortableVector<CDCGenHit> collection;

  collection.push_back(lowestGenHit);
  collection.push_back(lowestGenHit);
  collection.push_back(lowestGenHit);
  collection.push_back(lowestGenHit);
  collection.push_back(largerGenHitOnFirstWire);

  int counter = 0;
  for (CDCGenHit & genHit : collection) {
    ++counter;
  }
  EXPECT_EQ(5, counter);



  counter = 0;

  SortableVector<CDCGenHit>::range range_for_item = collection.equal_range(lowestGenHit);
  EXPECT_EQ(4, std::distance(range_for_item.first, range_for_item.second));

  for (CDCGenHit & genHit : range_for_item) {
    EXPECT_EQ(lowestGenHit, genHit);
  }



  // Test the coalignment of wires and the generic dummy hits
  const CDCWire& firstWire = CDCWire::getLowest();
  range_for_item = collection.equal_range(firstWire);

  EXPECT_EQ(5, std::distance(range_for_item.first, range_for_item.second));

  for (CDCGenHit & genHit : range_for_item) {
    EXPECT_EQ(firstWire, genHit.getWire());
  }


}


TEST_F(CDCLocalTrackingTest, SortableVector_findFast)
{

  const CDCGenHit lowestGenHit;
  const CDCGenHit largerGenHit(WireID(0, 0, 0), Vector2D(0.0, 1.0));
  SortableVector<CDCGenHit> collection;

  collection.push_back(lowestGenHit);
  collection.push_back(largerGenHit);
  collection.push_back(lowestGenHit);

  EXPECT_EQ(3u, collection.size());

  // test the contains pointer mechanism

  EXPECT_EQ(false, collection.containsPointer(&lowestGenHit));
  EXPECT_EQ(false, collection.containsPointer(&largerGenHit));

  const CDCGenHit& firstStoredGenHit = collection[0];
  const CDCGenHit& secondStoredGenHit = collection[1];
  const CDCGenHit& thirdStoredGenHit = collection[2];

  EXPECT_EQ(true, collection.containsPointer(&firstStoredGenHit));
  EXPECT_EQ(true, collection.containsPointer(&secondStoredGenHit));
  EXPECT_EQ(true, collection.containsPointer(&thirdStoredGenHit));

  // test the fast find
  SortableVector<CDCGenHit>::const_iterator itFoundLowest = collection.findFast(lowestGenHit);
  SortableVector<CDCGenHit>::const_iterator itFoundLarger = collection.findFast(largerGenHit);

  // External lookup should yield different object
  EXPECT_NE(&lowestGenHit, &*itFoundLowest);
  EXPECT_NE(&largerGenHit, &*itFoundLarger);

  // But objects should be equal
  EXPECT_EQ(lowestGenHit, *itFoundLowest);
  EXPECT_EQ(largerGenHit, *itFoundLarger);

  // Also check the iterators to point to the correct place inside the vector
  SortableVector<CDCGenHit>::const_iterator itBegin = collection.begin();

  EXPECT_EQ(0, std::distance(itBegin, itFoundLowest));
  EXPECT_EQ(1, std::distance(itBegin, itFoundLarger));

  // Compare addresses since it must be exactly the same
  SortableVector<CDCGenHit>::const_iterator itFoundFirst = collection.findFast(collection[0]);
  SortableVector<CDCGenHit>::const_iterator itFoundSecond = collection.findFast(collection[1]);
  SortableVector<CDCGenHit>::const_iterator itFoundThird = collection.findFast(collection[2]);

  // External lookup should yield exactly the same object
  EXPECT_EQ(&collection[0], &*itFoundFirst);
  EXPECT_EQ(&collection[1], &*itFoundSecond);
  EXPECT_EQ(&collection[2], &*itFoundThird);

  // Also check the iterators to point to the correct place inside the vector
  EXPECT_EQ(0, std::distance(itBegin, itFoundFirst));
  EXPECT_EQ(1, std::distance(itBegin, itFoundSecond));
  EXPECT_EQ(2, std::distance(itBegin, itFoundThird));

}




TEST_F(CDCLocalTrackingTest, SortableVector_sort)
{
  SortableVector<CDCGenHit> collection;
  EXPECT_TRUE(collection.isSorted()) << "Empty collection is not sorted";
  EXPECT_TRUE(collection.checkSorted()) << "Empty collection was checked to be not sorted";
  collection.sort(); //Just check if those run
  collection.ensureSorted(); //Just check if those run

  CDCGenHit lowestGenHit;
  CDCGenHit secondLowestGenHit(WireID(0, 0, 1), Vector2D(0.0, 0.0));
  CDCGenHit thirdLowestGenHit(WireID(0, 0, 1), Vector2D(0.0, 1.0));
  CDCGenHit fourthLowestGenHit(WireID(0, 1, 1), Vector2D(0.0, 1.0));

  //Add the second hit first
  collection.push_back(secondLowestGenHit);
  EXPECT_TRUE(collection.isSorted()) << "Single element collection is not sorted";
  EXPECT_TRUE(collection.checkSorted()) << "Single element collection was check to be not sorted";
  collection.ensureSorted(); //Just check if those run
  collection.sort();  //Just check if those run

  //Add the third hit - collection should still be sorted
  collection.push_back(thirdLowestGenHit);
  EXPECT_TRUE(collection.isSorted()) << "Collection is not sorted after the third hit was added.";
  EXPECT_TRUE(collection.checkSorted()) << "Collection is not sorted after the third hit was added.";;
  collection.ensureSorted(); //Just check if those run
  collection.sort();  //Just check if those run

  //Now add the first hit
  //Collection should not be sorted anymore
  collection.push_back(lowestGenHit);
  EXPECT_FALSE(collection.isSorted());
  EXPECT_FALSE(collection.checkSorted());
  collection.ensureSorted();
  EXPECT_TRUE(collection.isSorted());
  EXPECT_TRUE(collection.checkSorted());

  //Now add the fourth hit - collection should still be sorted
  collection.push_back(fourthLowestGenHit);
  EXPECT_TRUE(collection.isSorted()) << "Collection is not sorted after the fourth hit was added.";
  EXPECT_TRUE(collection.checkSorted()) << "Collection was checked not sorted after the fourth hit was added.";
  collection.ensureSorted();

  collection.push_back(thirdLowestGenHit);
  collection.push_back(secondLowestGenHit);
  EXPECT_FALSE(collection.isSorted()) << "Collection is not sorted after adding two unsorted hits.";
  EXPECT_FALSE(collection.checkSorted()) << "Collection was checked not sorted after adding two unsorted hits.";
  collection.clear();
  EXPECT_TRUE(collection.isSorted()) << "Collection is not sorted after clearing.";
  EXPECT_TRUE(collection.checkSorted()) << "Collection was checked not sorted after clearing.";
}


TEST_F(CDCLocalTrackingTest, SortableVector_unique)
{

  SortableVector<CDCGenHit> collection;

  CDCGenHit lowestGenHit;
  CDCGenHit secondGenHit(WireID(0, 0, 1), Vector2D(0.0, 0.0));

  collection.push_back(lowestGenHit);
  collection.push_back(secondGenHit);
  collection.push_back(lowestGenHit);
  collection.push_back(secondGenHit);
  collection.push_back(lowestGenHit);
  collection.push_back(secondGenHit);
  collection.push_back(lowestGenHit);

  collection.ensureUnique();

  EXPECT_EQ(2u, collection.size());

}




