/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <gtest/gtest.h>

#include <tracking/trackFindingVXD/trackSetEvaluator/HopfieldNetwork.h>
#include <tracking/trackFindingVXD/trackSetEvaluator/OverlapResolverNodeInfo.h>

#include <framework/logging/Logger.h>

#include <stdlib.h>

using namespace std;
using namespace Belle2;


/// Test of HopfieldNetwork Class
class HopfieldNetworkTest : public ::testing::Test {
protected:

  /**Container on which the Hopfield Algorithm runs.
   * Output comes in form of tcInfo4Hopfield.neuronValue. Typically > 0.7 signal, < 0.7 bkg;
   */
  vector <OverlapResolverNodeInfo> m_trackCandidateInfos;

  /**Container on which the Greedy Algorithm (Scrooge) runs.
   * Output comes in form of qiTrackOverlap.isActive, which is simply a bool,
   * if the corresponding track was taken or not. */
  vector <OverlapResolverNodeInfo> m_qiTrackOverlap;

  unsigned int myTrueTracks = 10; /**< Number of true tracks */

  /** Create sample for test
   * @return test sample info
   */
  vector<OverlapResolverNodeInfo> getInput()
  {
    vector<OverlapResolverNodeInfo> trackCandidateInfos;
    unsigned int myNTrackCands  = 200;
    unsigned int diff           = myNTrackCands - myTrueTracks;
    unsigned int nOverlaps      = 6;

    //Create competitor IDs
    vector <vector <unsigned short> > competitorIDMatrix(myNTrackCands);
    for (unsigned int ii = 0; ii < myNTrackCands; ii++) {
      for (unsigned int jj = 0; jj < nOverlaps; jj++) {
        unsigned short bkgIndex = (rand() % diff) + myTrueTracks;
        competitorIDMatrix[ii].push_back(bkgIndex);
        competitorIDMatrix[bkgIndex].push_back(ii);
      }
    }

    //Make the actual OverlapResolverNodeInfos
    for (unsigned int ii = 0; ii < myNTrackCands; ii++) {
      float qualityIndicator = 0;
      if (ii < myTrueTracks) {
        qualityIndicator = static_cast<float>(rand() % 100) / 100.;
        B2INFO("Track Index" << ii << ", TrueQI: " << qualityIndicator);
      } else {
        qualityIndicator = 1 / (static_cast<float>(rand() % 100) + 1.2);
        B2INFO("Track Index" << ii << ", FakeQI: " << qualityIndicator);
      }
      trackCandidateInfos.emplace_back(qualityIndicator, ii, competitorIDMatrix[ii], 0.8);
    }
    return trackCandidateInfos;
  }
};

TEST_F(HopfieldNetworkTest, TestPerformance)
{
  HopfieldNetwork hopfieldNetwork;
  m_trackCandidateInfos = getInput();
  bool finished = hopfieldNetwork.doHopfield(m_trackCandidateInfos);
  int countCorrectTracksSurvived  = 0;
  int countWrongTracksSurvived    = 0;
  for (auto&& info : m_trackCandidateInfos) {
    B2INFO("TrackIndex: " << info.trackIndex << ", Neuron Value: " << info.activityState);
    if (info.trackIndex <  myTrueTracks && info.activityState > 0.7) countCorrectTracksSurvived++;
    if (info.trackIndex >= myTrueTracks && info.activityState > 0.7) countWrongTracksSurvived++;
  }
  B2INFO("Correct survivors: " << countCorrectTracksSurvived << ", FakeSurvivors: " << countWrongTracksSurvived);

  EXPECT_EQ(finished, true);
}

//Let's compare the approach with the Scrooge approach
TEST_F(HopfieldNetworkTest, TestScrooge)
{
  /*
  m_trackCandidateInfos = getInput();
  for (auto const && info : m_trackCandidateInfos) {
    m_qiTrackOverlap.emplace_back({});
  }
  */
}
