/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost <oliver.frost@desy.de>                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <gtest/gtest.h>
#include <tracking/trackFindingCDC/findlets/generic/TreeTraversal.h>
#include <tracking/trackFindingCDC/findlets/generic/WeightedTreeTraversal.h>

#include <tracking/trackFindingCDC/findlets/base/Findlet.h>

#include <tracking/trackFindingCDC/numerics/Weight.h>
#include <tracking/trackFindingCDC/numerics/WithWeight.h>

#include <tracking/trackFindingCDC/utilities/Functional.h>
#include <tracking/trackFindingCDC/utilities/Algorithms.h>

#include <vector>

using namespace Belle2;
using namespace TrackFindingCDC;

namespace {
  using State = int;
  using Result = std::vector<const int*>;

  class AcceptAll : public Findlet<const int* const, int*> {
  public:
    void apply(const std::vector<const int*>& currentPath __attribute__((unused)),
               std::vector<int*>& nextStates __attribute__((unused))) override
    {
      // Do not filter states at all.
    }
  };

  class AcceptHighWeight : public Findlet<const int* const, WithWeight<int*> > {
  public:
    void apply(const std::vector<const int*>& currentPath __attribute__((unused)),
               std::vector<WithWeight<int*>>& nextStates __attribute__((unused)))
    {
      // Remove states with low weight
      erase_remove_if(nextStates, GetWeight() < 0.5);
    }
  };
}


TEST(TrackFindingCDCTest, Findlet_generic_TreeTraversal)
{
  TreeTraversal<AcceptAll, State, Result> testTreeTraversal;

  // Prepare states
  std::vector<int> states({1, 2, 3});

  // Prepare relations
  std::vector<Relation<int> > stateRelations;
  stateRelations.push_back({&states[1], &states[2]});
  stateRelations.push_back({&states[0], &states[2]});
  stateRelations.push_back({&states[0], &states[1]});
  std::sort(stateRelations.begin(), stateRelations.end());

  // Select some seeds
  std::vector<const int*> seedStates;
  seedStates.push_back(&states[0]);

  // Find the paths
  std::vector<std::vector<const int*>> results;
  testTreeTraversal.apply(seedStates, stateRelations, results);

  ASSERT_EQ(2, results.size());

  ASSERT_EQ(3, results[0].size());
  EXPECT_EQ(1, *results[0][0]);
  EXPECT_EQ(2, *results[0][1]);
  EXPECT_EQ(3, *results[0][2]);

  ASSERT_EQ(2, results[1].size());
  EXPECT_EQ(1, *results[1][0]);
  EXPECT_EQ(3, *results[1][1]);
}

TEST(TrackFindingCDCTest, Findlet_generic_WeightedTreeTraversal)
{
  WeightedTreeTraversal<AcceptHighWeight, State> testWeightedTreeTraversal;

  // Prepare states
  std::vector<int> states({1, 2, 3});

  // Prepare relations
  std::vector<WeightedRelation<int> > stateRelations;
  stateRelations.push_back({&states[1], 0.1, &states[2]});
  stateRelations.push_back({&states[0], 1, &states[2]});
  stateRelations.push_back({&states[0], 1, &states[1]});
  std::sort(stateRelations.begin(), stateRelations.end());

  // Select some seeds
  std::vector<const int*> seedStates;
  seedStates.push_back(&states[0]);

  // Find the paths
  std::vector<std::vector<const int*>> results;
  testWeightedTreeTraversal.apply(seedStates, stateRelations, results);

  ASSERT_EQ(2, results.size());

  ASSERT_EQ(2, results[0].size());
  EXPECT_EQ(1, *results[0][0]);
  EXPECT_EQ(2, *results[0][1]);

  ASSERT_EQ(2, results[1].size());
  EXPECT_EQ(1, *results[1][0]);
  EXPECT_EQ(3, *results[1][1]);
}
