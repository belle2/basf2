/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <framework/io/RootIOUtilities.h>
#include <framework/utilities/TestHelpers.h>

#include <gtest/gtest.h>

using namespace std;
using namespace Belle2;

namespace {
  TEST(IOTest, filterBranches)
  {
    const set<string> input = {"A", "B", "C", "AToB", "BToC", "CToA"};
    //unfiltered
    EXPECT_EQ(input, RootIOUtilities::filterBranches(input, {}, {}, 0));

    EXPECT_EQ(set<string>({"A", "C", "CToA"}), RootIOUtilities::filterBranches(input, {"A", "C"}, {}, 0));
    EXPECT_EQ(set<string>({"B"}), RootIOUtilities::filterBranches(input, {"B"}, {}, 0));

    //excludeBranches takes precedence over everything
    EXPECT_EQ(set<string>({"A", "C"}), RootIOUtilities::filterBranches(input, {"A", "C"}, {"CToA"}, 0));
    //and also prevents dangling relations
    EXPECT_EQ(set<string>({"A", "C"}), RootIOUtilities::filterBranches(input, {}, {"B", "CToA"}, 0));
    EXPECT_EQ(set<string>({"A", "C", "CToA"}), RootIOUtilities::filterBranches(input, {}, {"B"}, 0));
    //unless one really wants them!
    EXPECT_EQ(set<string>({"AToB"}), RootIOUtilities::filterBranches(input, {"AToB", "B"}, {"B"}, 0));
    EXPECT_EQ(set<string>(), RootIOUtilities::filterBranches(input, {"B"}, {"B"}, 0));
  }

  TEST(IOTest, filterBranchesWarnings)
  {
    const set<string> input = {"A", "B", "C"};
    EXPECT_B2WARNING(RootIOUtilities::filterBranches(input, {"A", "A"}, {}, 0));
    EXPECT_B2WARNING(RootIOUtilities::filterBranches(input, {}, {"A", "B", "A"}, 1));
  }

}  // namespace
