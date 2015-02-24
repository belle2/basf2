/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <vector>
#include <gtest/gtest.h>

#include <tracking/trackFindingCDC/legendre/CDCLegendreTrackHit.h>
#include <tracking/trackFindingCDC/legendre/CDCLegendreTrackCandidate.h>
#include <tracking/trackFindingCDC/legendre/CDCLegendreTrackFitter.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    class CDCLegendreTestFixture : public ::testing::Test {

    public:

      const unsigned int numberOfPossibleTrackCandidate = 2;

      void SetUp() {
        m_trackCandidates.reserve(numberOfPossibleTrackCandidate);

        for (unsigned int counter = 0; counter < numberOfPossibleTrackCandidate; counter++) {
          TrackCandidate* trackCandidate = createTestTrackCandidate(counter);
          m_trackCandidates.push_back(trackCandidate);

          for (TrackHit * hit : trackCandidate->getTrackHits()) {
            m_hitSet.insert(hit);
          }
        }
      }

      void TearDown() {
        for (TrackCandidate * trackCandidate : m_trackCandidates) {
          deleteTrackCandidate(trackCandidate);
        }

        m_hitSet.clear();
        m_trackCandidates.clear();
      }

      std::set<TrackHit*>& getHitSet() {
        return m_hitSet;
      }

      void markAllHitsAsUnused() {
        for (TrackHit * hit : m_hitSet) {
          hit->setHitUsage(TrackHit::not_used);
        }
      }

      TrackCandidate* getTrackCandidate(unsigned int index) {
        return m_trackCandidates[index % numberOfPossibleTrackCandidate];
      }

      std::vector<TrackHit*> getSomeHits(TrackCandidate* otherTrackCandidate) {
        std::vector<TrackHit*> someHits;

        unsigned int numberOfHitsBefore = otherTrackCandidate->getNHits();

        std::vector<TrackHit*>& trackHitList = otherTrackCandidate->getTrackHits();
        std::vector<TrackHit*> newTrackHitList;
        newTrackHitList.reserve(trackHitList.size() - 3);

        // some random hits
        for (unsigned int hitCounter = 0; hitCounter < trackHitList.size(); hitCounter++) {
          if (hitCounter == 21 || hitCounter == 30 || hitCounter == 42 || hitCounter == 11) {
            someHits.push_back(trackHitList[hitCounter]);
          } else {
            newTrackHitList.push_back(trackHitList[hitCounter]);
          }
        }

        for (TrackHit * trackHit : someHits)  {
          trackHit->setHitUsage(TrackHit::bad);
        }

        trackHitList.clear();
        for (TrackHit * trackHit : newTrackHitList)  {
          trackHitList.push_back(trackHit);
        }

        EXPECT_EQ(otherTrackCandidate->getNHits(), numberOfHitsBefore - someHits.size());

        return someHits;
      }

    private:

      std::vector<TrackCandidate*> m_trackCandidates;
      std::set<TrackHit*> m_hitSet;

      TrackCandidate* createTestTrackCandidate(unsigned int number) {
        std::vector<TrackHit*> emptyHitList;
        TrackCandidate* resultTrackCandidate = new TrackCandidate(0, 0, 1, emptyHitList);

        // create a track. These hits come from a real mcTrackCand.
        std::vector<TrackHit*>& trackHitList = resultTrackCandidate->getTrackHits();

        if (number % 2 == 0) {
          trackHitList.reserve(125);
          trackHitList.push_back(new TrackHit(808, 251, 0.104, 0.000169, 197, 6, 4, TVector3(18.7478, -85.4475, 0)));
          trackHitList.push_back(new TrackHit(807, 250, 0.272, 0.000169, 98, 6, 4, TVector3(17.0665, -85.7991, 0)));
          trackHitList.push_back(new TrackHit(806, 249, 0.488, 0.000169, 243, 6, 4, TVector3(15.3787, -86.1176, 0)));
          trackHitList.push_back(new TrackHit(805, 248, 0.764, 0.000169, 64, 6, 4, TVector3(13.6849, -86.403, 0)));
          trackHitList.push_back(new TrackHit(782, 247, 0.9, 0.000169, 43, 6, 3, TVector3(12.5689, -84.7329, 0)));
          trackHitList.push_back(new TrackHit(804, 247, 1.024, 0.000169, 2, 6, 4, TVector3(11.9858, -86.655, 0)));
          trackHitList.push_back(new TrackHit(781, 246, 0.64, 0.000169, 93, 6, 3, TVector3(10.9029, -84.9633, 0)));
          trackHitList.push_back(new TrackHit(780, 245, 0.304, 0.000169, 159, 6, 3, TVector3(9.23265, -85.161, 0)));
          trackHitList.push_back(new TrackHit(779, 244, 0.012, 0.000169, 92, 6, 3, TVector3(7.55884, -85.3258, 0)));
          trackHitList.push_back(new TrackHit(778, 243, 0.352, 0.000169, 92, 6, 3, TVector3(5.88212, -85.4578, 0)));
          trackHitList.push_back(new TrackHit(777, 242, 0.74, 0.000169, 85, 6, 3, TVector3(4.20314, -85.5568, 0)));
          trackHitList.push_back(new TrackHit(757, 241, 0.46, 0.000169, 81, 6, 2, TVector3(1.64609, -83.8238, 0)));
          trackHitList.push_back(new TrackHit(756, 240, 0.02, 0.000169, 80, 6, 2, TVector3(-1.54006e-14, -83.84, 0)));
          trackHitList.push_back(new TrackHit(755, 239, 0.46, 0.000169, 80, 6, 2, TVector3(-1.64609, -83.8238, 0)));
          trackHitList.push_back(new TrackHit(754, 238, 0.884, 0.000169, 38, 6, 2, TVector3(-3.29154, -83.7754, 0)));
          trackHitList.push_back(new TrackHit(735, 238, 1.104, 0.000169, 1, 6, 1, TVector3(-2.41534, -81.9844, 0)));
          trackHitList.push_back(new TrackHit(734, 237, 0.612, 0.000169, 80, 6, 1, TVector3(-4.02453, -81.9212, 0)));
          trackHitList.push_back(new TrackHit(733, 236, 0.12, 0.000169, 80, 6, 1, TVector3(-5.63217, -81.8264, 0)));
          trackHitList.push_back(new TrackHit(732, 235, 0.356, 0.000169, 80, 6, 1, TVector3(-7.23764, -81.7, 0)));
          trackHitList.push_back(new TrackHit(731, 234, 0.884, 0.000169, 32, 6, 1, TVector3(-8.84032, -81.5422, 0)));
          trackHitList.push_back(new TrackHit(711, 235, 1.032, 0.000169, 9, 6, 0, TVector3(-7.86588, -79.8636, 0)));
          trackHitList.push_back(new TrackHit(710, 234, 0.52, 0.000169, 104, 6, 0, TVector3(-9.43238, -79.6937, 0)));
          trackHitList.push_back(new TrackHit(709, 233, 0.06, 0.000169, 103, 6, 0, TVector3(-10.9952, -79.4932, 0)));
          trackHitList.push_back(new TrackHit(708, 232, 0.62, 0.000169, 82, 6, 0, TVector3(-12.5539, -79.262, 0)));
          trackHitList.push_back(new TrackHit(690, 206, 1.116, 0.000169, 39, 5, 5, TVector3(-11.2806, -77.3568, 0)));
          trackHitList.push_back(new TrackHit(689, 205, 0.464, 0.000169, 92, 5, 5, TVector3(-12.9655, -77.0923, 0)));
          trackHitList.push_back(new TrackHit(688, 204, 0.168, 0.000169, 146, 5, 5, TVector3(-14.6441, -76.7911, 0)));
          trackHitList.push_back(new TrackHit(668, 204, 1.08, 0.000169, 52, 5, 4, TVector3(-15.142, -74.8981, 0)));
          trackHitList.push_back(new TrackHit(667, 203, 0.392, 0.000169, 101, 5, 4, TVector3(-16.7723, -74.55, 0)));
          trackHitList.push_back(new TrackHit(666, 202, 0.304, 0.000169, 86, 5, 4, TVector3(-18.3946, -74.1663, 0)));
          trackHitList.push_back(new TrackHit(650, 201, 0.968, 0.000169, 60, 5, 3, TVector3(-18.7572, -72.2055, 0)));
          trackHitList.push_back(new TrackHit(649, 200, 0.252, 0.000169, 93, 5, 3, TVector3(-20.3279, -71.7791, 0)));
          trackHitList.push_back(new TrackHit(648, 199, 0.476, 0.000169, 170, 5, 3, TVector3(-21.8889, -71.3186, 0)));
          trackHitList.push_back(new TrackHit(626, 199, 0.736, 0.000169, 77, 5, 2, TVector3(-22.1248, -69.3468, 0)));
          trackHitList.push_back(new TrackHit(625, 198, 0.008, 0.000169, 83, 5, 2, TVector3(-23.6323, -68.8476, 0)));
          trackHitList.push_back(new TrackHit(624, 197, 0.788, 0.000169, 17, 5, 2, TVector3(-25.1286, -68.3157, 0)));
          trackHitList.push_back(new TrackHit(604, 197, 1.188, 0.000169, 24, 5, 1, TVector3(-23.7844, -66.8757, 0)));
          trackHitList.push_back(new TrackHit(603, 196, 0.404, 0.000169, 79, 5, 1, TVector3(-25.2376, -66.3409, 0)));
          trackHitList.push_back(new TrackHit(602, 195, 0.356, 0.000169, 57, 5, 1, TVector3(-26.6789, -65.7746, 0)));
          trackHitList.push_back(new TrackHit(583, 195, 0.74, 0.000169, 46, 5, 0, TVector3(-26.7235, -63.8509, 0)));
          trackHitList.push_back(new TrackHit(582, 194, 0.04, 0.000169, 60, 5, 0, TVector3(-28.11, -63.2527, 0)));
          trackHitList.push_back(new TrackHit(581, 193, 0.832, 0.000169, 17, 5, 0, TVector3(-29.4832, -62.6244, 0)));
          trackHitList.push_back(new TrackHit(559, 173, 0.692, 0.000169, 81, 4, 5, TVector3(-29.5671, -60.5796, 0)));
          trackHitList.push_back(new TrackHit(558, 172, 0.22, 0.000169, 111, 4, 5, TVector3(-31.0449, -59.8358, 0)));
          trackHitList.push_back(new TrackHit(557, 171, 1.132, 0.000169, 3, 4, 5, TVector3(-32.504, -59.0559, 0)));
          trackHitList.push_back(new TrackHit(537, 172, 0.816, 0.000169, 25, 4, 4, TVector3(-30.9425, -57.8893, 0)));
          trackHitList.push_back(new TrackHit(536, 171, 0.136, 0.000169, 147, 4, 4, TVector3(-32.3538, -57.1125, 0)));
          trackHitList.push_back(new TrackHit(535, 170, 1.048, 0.000169, 13, 4, 4, TVector3(-33.7457, -56.3013, 0)));
          trackHitList.push_back(new TrackHit(517, 170, 0.884, 0.000169, 43, 4, 3, TVector3(-32.1358, -55.1387, 0)));
          trackHitList.push_back(new TrackHit(516, 169, 0.032, 0.000169, 201, 4, 3, TVector3(-33.4793, -54.3335, 0)));
          trackHitList.push_back(new TrackHit(515, 168, 0.96, 0.000169, 18, 4, 3, TVector3(-34.8026, -53.4955, 0)));
          trackHitList.push_back(new TrackHit(499, 169, 0.972, 0.000169, 13, 4, 2, TVector3(-33.1699, -52.3809, 0)));
          trackHitList.push_back(new TrackHit(498, 168, 0.044, 0.000169, 54, 4, 2, TVector3(-34.4454, -51.5511, 0)));
          trackHitList.push_back(new TrackHit(497, 167, 0.872, 0.000169, 17, 4, 2, TVector3(-35.7001, -50.6903, 0)));
          trackHitList.push_back(new TrackHit(482, 167, 1.004, 0.000169, 21, 4, 1, TVector3(-34.0457, -49.6238, 0)));
          trackHitList.push_back(new TrackHit(481, 166, 0.1, 0.000169, 115, 4, 1, TVector3(-35.2533, -48.7733, 0)));
          trackHitList.push_back(new TrackHit(480, 165, 0.828, 0.000169, 42, 4, 1, TVector3(-36.4397, -47.8935, 0)));
          trackHitList.push_back(new TrackHit(460, 166, 1.004, 0.000169, 12, 4, 0, TVector3(-34.7948, -46.9154, 0)));
          trackHitList.push_back(new TrackHit(459, 165, 0.084, 0.000169, 91, 4, 0, TVector3(-35.9357, -46.0473, 0)));
          trackHitList.push_back(new TrackHit(458, 164, 0.82, 0.000169, 29, 4, 0, TVector3(-37.0549, -45.1515, 0)));
          trackHitList.push_back(new TrackHit(443, 145, 0.508, 0.000169, 166, 3, 5, TVector3(-36.497, -42.8889, 0)));
          trackHitList.push_back(new TrackHit(442, 144, 0.5, 0.000169, 65, 3, 5, TVector3(-37.6855, -41.8484, 0)));
          trackHitList.push_back(new TrackHit(426, 145, 1.348, 0.000169, 8, 3, 4, TVector3(-35.9299, -41.0565, 0)));
          trackHitList.push_back(new TrackHit(425, 144, 0.292, 0.000169, 90, 3, 4, TVector3(-37.0672, -40.0326, 0)));
          trackHitList.push_back(new TrackHit(424, 143, 0.68, 0.000169, 20, 3, 4, TVector3(-38.1754, -38.9773, 0)));
          trackHitList.push_back(new TrackHit(406, 143, 1.136, 0.000169, 19, 3, 3, TVector3(-36.3716, -38.2065, 0)));
          trackHitList.push_back(new TrackHit(405, 142, 0.12, 0.000169, 76, 3, 3, TVector3(-37.4288, -37.1714, 0)));
          trackHitList.push_back(new TrackHit(404, 141, 0.872, 0.000169, 4, 3, 3, TVector3(-38.4566, -36.107, 0)));
          trackHitList.push_back(new TrackHit(385, 142, 0.96, 0.000169, 25, 3, 2, TVector3(-36.6397, -35.3939, 0)));
          trackHitList.push_back(new TrackHit(384, 141, 0.036, 0.000169, 1551, 3, 2, TVector3(-37.618, -34.3524, 0)));
          trackHitList.push_back(new TrackHit(367, 140, 0.756, 0.000169, 193, 3, 1, TVector3(-36.7384, -32.6281, 0)));
          trackHitList.push_back(new TrackHit(366, 139, 0.204, 0.000169, 68, 3, 1, TVector3(-37.639, -31.5849, 0)));
          trackHitList.push_back(new TrackHit(349, 139, 0.588, 0.000169, 68, 3, 0, TVector3(-36.7106, -29.9495, 0)));
          trackHitList.push_back(new TrackHit(348, 138, 0.332, 0.000169, 63, 3, 0, TVector3(-37.5361, -28.9081, 0)));
          trackHitList.push_back(new TrackHit(332, 116, 1.1, 0.000169, 10, 2, 5, TVector3(-35.6944, -28.3291, 0)));
          trackHitList.push_back(new TrackHit(331, 115, 0.008, 0.000169, 59, 2, 5, TVector3(-36.6022, -27.146, 0)));
          trackHitList.push_back(new TrackHit(330, 114, 1.048, 0.000169, 5, 2, 5, TVector3(-37.4708, -25.9339, 0)));
          trackHitList.push_back(new TrackHit(314, 115, 0.712, 0.000169, 52, 2, 4, TVector3(-35.6027, -25.5125, 0)));
          trackHitList.push_back(new TrackHit(313, 114, 0.316, 0.000169, 100, 2, 4, TVector3(-36.4184, -24.334, 0)));
          trackHitList.push_back(new TrackHit(298, 113, 0.428, 0.000169, 47, 2, 3, TVector3(-35.282, -22.7486, 0)));
          trackHitList.push_back(new TrackHit(297, 112, 0.572, 0.000169, 37, 2, 3, TVector3(-36.0074, -21.582, 0)));
          trackHitList.push_back(new TrackHit(281, 112, 0.188, 0.000169, 140, 2, 2, TVector3(-34.7796, -20.08, 0)));
          trackHitList.push_back(new TrackHit(280, 111, 0.776, 0.000169, 51, 2, 2, TVector3(-35.418, -18.9313, 0)));
          trackHitList.push_back(new TrackHit(268, 111, 0.92, 0.000169, 12, 2, 1, TVector3(-33.5126, -18.6242, 0)));
          trackHitList.push_back(new TrackHit(267, 110, 0.024, 0.000169, 82, 2, 1, TVector3(-34.104, -17.5177, 0)));
          trackHitList.push_back(new TrackHit(266, 109, 0.928, 0.000169, 15, 2, 1, TVector3(-34.659, -16.3925, 0)));
          trackHitList.push_back(new TrackHit(254, 110, 0.776, 0.000169, 19, 2, 0, TVector3(-32.7538, -16.1524, 0)));
          trackHitList.push_back(new TrackHit(253, 109, 0.116, 0.000169, 165, 2, 0, TVector3(-33.2647, -15.0721, 0)));
          trackHitList.push_back(new TrackHit(252, 108, 0.992, 0.000169, 2, 2, 0, TVector3(-33.7401, -13.9756, 0)));
          trackHitList.push_back(new TrackHit(237, 87, 0.664, 0.000169, 51, 1, 5, TVector3(-31.4633, -13.7096, 0)));
          trackHitList.push_back(new TrackHit(236, 86, 0.3, 0.000169, 35, 1, 5, TVector3(-31.9773, -12.4638, 0)));
          trackHitList.push_back(new TrackHit(222, 86, 0.504, 0.000169, 125, 1, 4, TVector3(-30.4818, -11.3271, 0)));
          trackHitList.push_back(new TrackHit(221, 85, 0.424, 0.000169, 56, 1, 4, TVector3(-30.903, -10.1217, 0)));
          trackHitList.push_back(new TrackHit(199, 85, 1.256, 0.000169, 9, 1, 3, TVector3(-28.9845, -10.1821, 0)));
          trackHitList.push_back(new TrackHit(198, 84, 0.388, 0.000169, 111, 1, 3, TVector3(-29.3619, -9.03631, 0)));
          trackHitList.push_back(new TrackHit(197, 83, 0.5, 0.000169, 32, 1, 3, TVector3(-29.6941, -7.8766, 0)));
          trackHitList.push_back(new TrackHit(171, 84, 1.128, 0.000169, 20, 1, 2, TVector3(-27.8087, -7.96407, 0)));
          trackHitList.push_back(new TrackHit(170, 83, 0.28, 0.000169, 91, 1, 2, TVector3(-28.0999, -6.86616, 0)));
          trackHitList.push_back(new TrackHit(169, 82, 0.532, 0.000169, 22, 1, 2, TVector3(-28.3478, -5.75767, 0)));
          trackHitList.push_back(new TrackHit(154, 82, 1.084, 0.000169, 15, 1, 1, TVector3(-26.4782, -5.92152, 0)));
          trackHitList.push_back(new TrackHit(153, 81, 0.3, 0.000169, 62, 1, 1, TVector3(-26.6903, -4.87743, 0)));
          trackHitList.push_back(new TrackHit(152, 80, 0.472, 0.000169, 23, 1, 1, TVector3(-26.8612, -3.82581, 0)));
          trackHitList.push_back(new TrackHit(130, 81, 1.124, 0.000169, 9, 1, 0, TVector3(-25.0098, -4.06461, 0)));
          trackHitList.push_back(new TrackHit(129, 80, 0.428, 0.000169, 65, 1, 0, TVector3(-25.1501, -3.0796, 0)));
          trackHitList.push_back(new TrackHit(128, 79, 0.296, 0.000169, 62, 1, 0, TVector3(-25.2516, -2.08984, 0)));
          trackHitList.push_back(new TrackHit(127, 78, 0.972, 0.000169, 10, 1, 0, TVector3(-25.3141, -1.09686, 0)));
          trackHitList.push_back(new TrackHit(108, 81, 0.192, 0.000169, 48, 0, 7, TVector3(-23.7817, -0.934384, 0)));
          trackHitList.push_back(new TrackHit(107, 80, 0.452, 0.000169, 23, 0, 7, TVector3(-23.8, 2.91456e-15, 0)));
          trackHitList.push_back(new TrackHit(96, 80, 0.596, 0.000169, 9, 0, 6, TVector3(-22.7956, -0.447648, 0)));
          trackHitList.push_back(new TrackHit(95, 79, 0.024, 0.000169, 71, 0, 6, TVector3(-22.7956, 0.447648, 0)));
          trackHitList.push_back(new TrackHit(94, 78, 0.66, 0.000169, 3, 0, 6, TVector3(-22.7605, 1.34225, 0)));
          trackHitList.push_back(new TrackHit(82, 79, 0.388, 0.000169, 29, 0, 5, TVector3(-21.7832, 0.855864, 0)));
          trackHitList.push_back(new TrackHit(81, 78, 0.184, 0.000169, 53, 0, 5, TVector3(-21.7328, 1.71041, 0)));
          trackHitList.push_back(new TrackHit(68, 77, 0.296, 0.000169, 40, 0, 4, TVector3(-20.6998, 2.03876, 0)));
          trackHitList.push_back(new TrackHit(67, 76, 0.244, 0.000169, 43, 0, 4, TVector3(-20.6038, 2.84986, 0)));
          trackHitList.push_back(new TrackHit(54, 76, 0.268, 0.000169, 34, 0, 3, TVector3(-19.5562, 3.0974, 0)));
          trackHitList.push_back(new TrackHit(53, 75, 0.212, 0.000169, 95, 0, 3, TVector3(-19.4195, 3.86279, 0)));
          trackHitList.push_back(new TrackHit(42, 74, 0.316, 0.000169, 117, 0, 2, TVector3(-18.3632, 4.02901, 0)));
          trackHitList.push_back(new TrackHit(41, 73, 0.112, 0.000169, 48, 0, 2, TVector3(-18.1909, 4.74684, 0)));
          trackHitList.push_back(new TrackHit(40, 72, 0.588, 0.000169, 4, 0, 2, TVector3(-17.9905, 5.45735, 0)));
          trackHitList.push_back(new TrackHit(27, 73, 0.464, 0.000169, 12, 0, 1, TVector3(-17.1317, 4.83164, 0)));
          trackHitList.push_back(new TrackHit(26, 72, 0.028, 0.000169, 29, 0, 1, TVector3(-16.9288, 5.5005, 0)));
          trackHitList.push_back(new TrackHit(11, 70, 0.284, 0.000169, 33, 0, 0, TVector3(-15.6444, 6.1231, 0)));
          trackHitList.push_back(new TrackHit(10, 69, 0.088, 0.000169, 37, 0, 0, TVector3(-15.392, 6.73258, 0)));
          trackHitList.push_back(new TrackHit(9, 68, 0.416, 0.000169, 18, 0, 0, TVector3(-15.1158, 7.33168, 0)));

          EXPECT_EQ(resultTrackCandidate->getTrackHits().size(), 125) << "Too few hits in the hit vector.";
        } else {
          trackHitList.reserve(61);
          trackHitList.push_back(new TrackHit(31, 140, 0.308, 0.000169, 97, 0, 1, TVector3(12.5865, -12.5865, 0)));
          trackHitList.push_back(new TrackHit(45, 139, 0.08, 0.000169, 64, 0, 2, TVector3(13.03, -13.552, 0)));
          trackHitList.push_back(new TrackHit(58, 139, 0.16, 0.000169, 115, 0, 3, TVector3(13.4403, -14.5396, 0)));
          trackHitList.push_back(new TrackHit(72, 139, 0.404, 0.000169, 62, 0, 4, TVector3(14.4162, -14.9938, 0)));
          trackHitList.push_back(new TrackHit(71, 138, 0.38, 0.000169, 62, 0, 4, TVector3(13.8165, -15.5482, 0)));
          trackHitList.push_back(new TrackHit(85, 139, 0.132, 0.000169, 346, 0, 5, TVector3(14.7979, -16.0082, 0)));
          trackHitList.push_back(new TrackHit(99, 138, 0.108, 0.000169, 186, 0, 6, TVector3(15.145, -17.0432, 0)));
          trackHitList.push_back(new TrackHit(112, 139, 0.48, 0.000169, 19, 0, 7, TVector3(16.1555, -17.4769, 0)));
          trackHitList.push_back(new TrackHit(111, 138, 0.424, 0.000169, 92, 0, 7, TVector3(15.4569, -18.0977, 0)));
          trackHitList.push_back(new TrackHit(143, 136, 0.532, 0.000169, 80, 1, 0, TVector3(17.2743, -18.5367, 0)));
          trackHitList.push_back(new TrackHit(142, 135, 0.452, 0.000169, 153, 1, 0, TVector3(16.5332, -19.2006, 0)));
          trackHitList.push_back(new TrackHit(159, 135, 0.396, 0.000169, 248, 1, 1, TVector3(18.1043, -20.2087, 0)));
          trackHitList.push_back(new TrackHit(176, 135, 0.26, 0.000169, 163, 1, 2, TVector3(18.8749, -21.92, 0)));
          trackHitList.push_back(new TrackHit(158, 134, 0.64, 0.000169, 127, 1, 1, TVector3(17.297, -20.9039, 0)));
          trackHitList.push_back(new TrackHit(203, 134, 0.092, 0.000169, 215, 1, 3, TVector3(19.5848, -23.6688, 0)));
          trackHitList.push_back(new TrackHit(225, 134, 0.16, 0.000169, 325, 1, 4, TVector3(20.1921, -25.4896, 0)));
          trackHitList.push_back(new TrackHit(240, 133, 0.524, 0.000169, 142, 1, 5, TVector3(20.6758, -27.3935, 0)));
          trackHitList.push_back(new TrackHit(257, 163, 0.228, 0.000169, 323, 2, 0, TVector3(21.2721, -29.6852, 0)));
          trackHitList.push_back(new TrackHit(271, 162, 0.356, 0.000169, 294, 2, 1, TVector3(21.8193, -31.5258, 0)));
          trackHitList.push_back(new TrackHit(285, 163, 0.776, 0.000169, 161, 2, 2, TVector3(23.3923, -32.6439, 0)));
          trackHitList.push_back(new TrackHit(284, 162, 0.46, 0.000169, 163, 2, 2, TVector3(22.3117, -33.3918, 0)));
          trackHitList.push_back(new TrackHit(302, 162, 0.744, 0.000169, 70, 2, 3, TVector3(23.8908, -34.5188, 0)));
          trackHitList.push_back(new TrackHit(301, 161, 0.58, 0.000169, 125, 2, 3, TVector3(22.7486, -35.282, 0)));
          trackHitList.push_back(new TrackHit(320, 162, 0.656, 0.000169, 85, 2, 4, TVector3(24.334, -36.4184, 0)));
          trackHitList.push_back(new TrackHit(319, 161, 0.68, 0.000169, 121, 2, 4, TVector3(23.1294, -37.1951, 0)));
          trackHitList.push_back(new TrackHit(337, 161, 0.568, 0.000169, 173, 2, 5, TVector3(24.694, -38.2992, 0)));
          trackHitList.push_back(new TrackHit(336, 160, 0.812, 0.000169, 90, 2, 5, TVector3(23.4277, -39.0867, 0)));
          trackHitList.push_back(new TrackHit(352, 190, 0.54, 0.000169, 189, 3, 0, TVector3(24.5237, -40.5367, 0)));
          trackHitList.push_back(new TrackHit(369, 188, 0.688, 0.000169, 149, 3, 1, TVector3(23.635, -43.0777, 0)));
          trackHitList.push_back(new TrackHit(388, 188, 0.656, 0.000169, 111, 3, 2, TVector3(23.8674, -45.0061, 0)));
          trackHitList.push_back(new TrackHit(410, 188, 0.664, 0.000169, 79, 3, 3, TVector3(25.357, -46.2564, 0)));
          trackHitList.push_back(new TrackHit(409, 187, 0.68, 0.000169, 232, 3, 3, TVector3(24.0497, -46.9493, 0)));
          trackHitList.push_back(new TrackHit(429, 188, 0.724, 0.000169, 201, 3, 4, TVector3(25.5436, -48.2091, 0)));
          trackHitList.push_back(new TrackHit(428, 187, 0.656, 0.000169, 205, 3, 4, TVector3(24.1815, -48.9065, 0)));
          trackHitList.push_back(new TrackHit(445, 186, 0.676, 0.000169, 157, 3, 5, TVector3(24.2413, -50.8316, 0)));
          trackHitList.push_back(new TrackHit(463, 211, 0.42, 0.000169, 205, 4, 0, TVector3(26.2618, -52.1732, 0)));
          trackHitList.push_back(new TrackHit(462, 210, 0.872, 0.000169, 51, 4, 0, TVector3(24.9735, -52.802, 0)));
          trackHitList.push_back(new TrackHit(485, 210, 0.548, 0.000169, 244, 4, 1, TVector3(26.3959, -54.0822, 0)));
          trackHitList.push_back(new TrackHit(484, 209, 0.796, 0.000169, 111, 4, 1, TVector3(25.0607, -54.7137, 0)));
          trackHitList.push_back(new TrackHit(502, 210, 0.716, 0.000169, 138, 4, 2, TVector3(26.5084, -56.0473, 0)));
          trackHitList.push_back(new TrackHit(501, 209, 0.656, 0.000169, 154, 4, 2, TVector3(25.125, -56.681, 0)));
          trackHitList.push_back(new TrackHit(520, 209, 0.856, 0.000169, 71, 4, 3, TVector3(26.5765, -58.0231, 0)));
          trackHitList.push_back(new TrackHit(541, 209, 1.056, 0.000169, 20, 4, 4, TVector3(26.6, -60.0087, 0)));
          trackHitList.push_back(new TrackHit(540, 208, 0.36, 0.000169, 161, 4, 4, TVector3(25.1193, -60.6435, 0)));
          trackHitList.push_back(new TrackHit(561, 207, 0.232, 0.000169, 210, 4, 5, TVector3(25.0305, -62.5906, 0)));
          trackHitList.push_back(new TrackHit(586, 231, 0.224, 0.000169, 266, 5, 0, TVector3(26.253, -64.0458, 0)));
          trackHitList.push_back(new TrackHit(585, 230, 1.088, 0.000169, 34, 5, 0, TVector3(24.8496, -64.6032, 0)));
          trackHitList.push_back(new TrackHit(608, 230, 0.452, 0.000169, 135, 5, 1, TVector3(26.2121, -65.962, 0)));
          trackHitList.push_back(new TrackHit(607, 229, 0.912, 0.000169, 91, 5, 1, TVector3(24.7669, -66.5181, 0)));
          trackHitList.push_back(new TrackHit(629, 230, 0.72, 0.000169, 183, 5, 2, TVector3(26.151, -67.9309, 0)));
          trackHitList.push_back(new TrackHit(628, 229, 0.632, 0.000169, 213, 5, 2, TVector3(24.6629, -68.4852, 0)));
          trackHitList.push_back(new TrackHit(653, 229, 1.016, 0.000169, 2, 5, 3, TVector3(26.0504, -69.906, 0)));
          trackHitList.push_back(new TrackHit(652, 228, 0.34, 0.000169, 189, 5, 3, TVector3(24.5192, -70.4576, 0)));
          trackHitList.push_back(new TrackHit(670, 228, 0.04, 0.000169, 295, 5, 4, TVector3(24.3357, -72.4347, 0)));
          trackHitList.push_back(new TrackHit(693, 227, 0.22, 0.000169, 175, 5, 5, TVector3(24.0968, -74.3685, 0)));
          trackHitList.push_back(new TrackHit(692, 226, 1.196, 0.000169, 31, 5, 5, TVector3(22.4687, -74.8765, 0)));
          trackHitList.push_back(new TrackHit(714, 254, 0.104, 0.000169, 328, 6, 0, TVector3(21.7831, -77.237, 0)));
          trackHitList.push_back(new TrackHit(738, 253, 0.504, 0.000169, 127, 6, 1, TVector3(21.4875, -79.1553, 0)));
          trackHitList.push_back(new TrackHit(737, 252, 0.78, 0.000169, 79, 6, 1, TVector3(19.9292, -79.562, 0)));
          trackHitList.push_back(new TrackHit(760, 253, 0.968, 0.000169, 43, 6, 2, TVector3(21.1689, -81.1235, 0)));
          trackHitList.push_back(new TrackHit(759, 252, 0.332, 0.000169, 167, 6, 2, TVector3(19.5721, -81.5235, 0)));

          EXPECT_EQ(resultTrackCandidate->getTrackHits().size(), 61) << "Too few hits in the hit vector.";
        }

        fitTrackCandidate(resultTrackCandidate);
        return resultTrackCandidate;
      }

      void deleteTrackCandidate(TrackCandidate* trackCandidate) {
        // release TrackHits
        std::for_each(trackCandidate->getTrackHits().begin(), trackCandidate->getTrackHits().end(), [](TrackHit * ht) { delete ht;});
        // release candidate
        delete trackCandidate;
      }

      void fitTrackCandidate(TrackCandidate* trackCandidate) {
        // Fit the track to get good parameter assumptions
        TrackFitter fitter;
        std::pair<double, double> track_par;
        std::pair<double, double> ref_point;

        double chi2 = fitter.fitTrackCandidateFast(trackCandidate->getTrackHits(), track_par, ref_point, false);

        trackCandidate->setTheta(track_par.first);
        trackCandidate->setR(track_par.second);
        trackCandidate->setChi2(chi2);
        trackCandidate->setReferencePoint(ref_point.first, ref_point.second);

        // Recalculate the momentum
        trackCandidate->getMomentumEstimation(true);

        EXPECT_FALSE(std::isnan(trackCandidate->getMomentumEstimation().X()));
        EXPECT_FALSE(std::isnan(trackCandidate->getMomentumEstimation().Y()));
        EXPECT_FALSE(std::isnan(trackCandidate->getMomentumEstimation().Z()));

        EXPECT_FALSE(std::isnan(trackCandidate->getReferencePoint().X()));
        EXPECT_FALSE(std::isnan(trackCandidate->getReferencePoint().Y()));
        EXPECT_FALSE(std::isnan(trackCandidate->getReferencePoint().Z()));

        for (TrackHit * trackHit : trackCandidate->getTrackHits()) {
          trackHit->setHitUsage(TrackHit::used_in_track);
        }
      }

    };
  }
}
