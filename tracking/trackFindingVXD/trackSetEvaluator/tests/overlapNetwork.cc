/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Heck                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <gtest/gtest.h>

#include <framework/datastore/StoreArray.h>
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

