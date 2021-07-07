/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <gtest/gtest.h>

#include <tracking/trackFindingVXD/trackSetEvaluator/OverlapNetwork.h>

using namespace std;
using namespace Belle2;


/// Test OverlapNetwork Class
class OverlapNetworkTest : public ::testing::Test {
protected:
  /// An overlap matrix for testing purposes:
  std::vector<std::vector <unsigned short> > smallOverlapTestMatrix =
  {{2, 4, 6, 8}, {3, 5, 7, 9}, {0}, {1}, {0}, {1}, {0}, {1}, {0}, {1}};
};

TEST_F(OverlapNetworkTest, TestSettersGetters)
{
  OverlapNetwork overlapNetwork(smallOverlapTestMatrix);
  EXPECT_EQ(smallOverlapTestMatrix[0], overlapNetwork.getOverlapForTrackIndex(0));
  EXPECT_EQ(smallOverlapTestMatrix[1], overlapNetwork.getOverlapForTrackIndex(1));
}

