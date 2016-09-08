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
#include <tracking/trackFindingVXD/trackSetEvaluator/OverlapMatrixCreator.h>

using namespace std;
using namespace Belle2;


class OverlapMatrixCreatorTest : public ::testing::Test {
protected:
  //A hitRelatedTracks matrix for Testing purposes:
  vector<vector <unsigned short> > hitRelatedTracks =
    //hit 0     1        2     3      4      5      6      7      8     9
  {{1, 2, 3}, {0, 5, 7}, {5}, {1, 5, 9}, {1}, {4}, {4}, {7, 8, 9}, {8}, {0, 1}};
  //Corresponding overlap matrix for testing purposes: [assumes sorting]
  vector<vector <unsigned short> > smallOverlapTestMatrix =
    //track 0   1                2       3       4   5             6   7             8       9
  {{1, 5, 7}, {0, 2, 3, 5, 9}, {1, 3}, {1, 2}, {}, {0, 1, 7, 9}, {}, {0, 5, 8, 9}, {7, 9}, {1, 5, 7, 8}};
};

TEST_F(OverlapMatrixCreatorTest, TestGetter)
{
  OverlapMatrixCreator overlapMatrixCreator(hitRelatedTracks, 10);
  EXPECT_EQ(smallOverlapTestMatrix, overlapMatrixCreator.getOverlapMatrix());
}

