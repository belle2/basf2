/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <gtest/gtest.h>

#include <tracking/trackFindingVXD/trackSetEvaluator/OverlapMatrixCreator.h>

using namespace std;
using namespace Belle2;


/// Test the overlap-matrix creator using known input
class OverlapMatrixCreatorTest : public ::testing::Test {
protected:
  /// A hitRelatedTracks matrix for Testing purposes:
  vector<vector <unsigned short> > hitRelatedTracks =
    //hit 0     1        2     3      4      5      6      7      8     9
  {{1, 2, 3}, {0, 5, 7}, {5}, {1, 5, 9}, {1}, {4}, {4}, {7, 8, 9}, {8}, {0, 1}};
  /// Corresponding overlap matrix for testing purposes: [assumes sorting]
  vector<vector <unsigned short> > smallOverlapTestMatrix =
    //track 0   1                2       3       4   5             6   7             8       9
  {{1, 5, 7}, {0, 2, 3, 5, 9}, {1, 3}, {1, 2}, {}, {0, 1, 7, 9}, {}, {0, 5, 8, 9}, {7, 9}, {1, 5, 7, 8}};

  /// Another hitRelatedTracks matrix with multiple double overlaps:
  vector<vector <unsigned short> > hitRelatedTracks2 =
  {{0, 1, 2, 3}, {1, 2, 3}, {3, 4, 5, 6}, {2, 4}, {3, 5}};
  /// and its overlap matrix which allows for 1 cluster overlaps
  vector<vector <unsigned short> > allowOneOverlapTestMatrix =
  {{}, {2, 3}, {1, 3}, {1, 2, 5}, {}, {3}, {}};

  /// This time, we want to check for double overlaps
  vector<vector <unsigned short> > hitRelatedTracks3 =
  {{0, 1, 2, 3}, {1, 2, 3}, {3, 4, 5, 6}, {2, 3, 4}, {3, 5}};
  /// and its overlap matrix which allows for 2 cluster overlaps
  vector<vector <unsigned short> > allowTwoOverlapTestMatrix =
  {{}, {}, {3}, {2}, {}, {}, {}};
};

TEST_F(OverlapMatrixCreatorTest, TestGetter)
{
  OverlapMatrixCreator overlapMatrixCreator(hitRelatedTracks, 10);
  EXPECT_EQ(smallOverlapTestMatrix, overlapMatrixCreator.getOverlapMatrix());

  OverlapMatrixCreator overlapMatrixCreator2(hitRelatedTracks2, 7);
  EXPECT_EQ(allowOneOverlapTestMatrix, overlapMatrixCreator2.getOverlapMatrix(1));

  OverlapMatrixCreator overlapMatrixCreator3(hitRelatedTracks3, 7);
  EXPECT_EQ(allowTwoOverlapTestMatrix, overlapMatrixCreator3.getOverlapMatrix(2));
}
