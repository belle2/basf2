#include <framework/io/RootIOUtilities.h>
#include <framework/logging/Logger.h>
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
    //might produce dangling relations right now
    EXPECT_EQ(set<string>({"A", "C", "AToB", "BToC", "CToA"}), RootIOUtilities::filterBranches(input, {}, {"B"}, 0));
    EXPECT_EQ(set<string>({"A", "C", "AToB", "BToC"}), RootIOUtilities::filterBranches(input, {}, {"B", "CToA"}, 0));
    EXPECT_EQ(set<string>(), RootIOUtilities::filterBranches(input, {"B"}, {"B"}, 0));
  }

  TEST(IOTest, filterBranchesWarnings)
  {
    const set<string> input = {"A", "B", "C"};
    EXPECT_B2WARNING(RootIOUtilities::filterBranches(input, {"A", "A"}, {}, 0));
    EXPECT_B2WARNING(RootIOUtilities::filterBranches(input, {}, {"A", "B", "A"}, 1));
  }

}  // namespace
