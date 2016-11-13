/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once
#include <vector>
#include <gtest/gtest.h>

#include <tracking/trackFindingCDC/fitting/CDCRiemannFitter.h>

#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>
#include <tracking/trackFindingCDC/eventdata/hits/CDCConformalHit.h>
#include <tracking/trackFindingCDC/testFixtures/TrackFindingCDCTestWithTopology.h>

#include <framework/utilities/TestHelpers.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    /**
     * Test fixture for QuadTree tests
     */
    class CDCLegendreTestFixture : public TrackFindingCDCTestWithTopology {

    public:

      /// Number of generated tracks
      const unsigned int numberOfPossibleTrackCandidate = 2;

      /// Setup test
      void SetUp() override
      {
        prepareWireHits();

        for (unsigned int counter = 0; counter < numberOfPossibleTrackCandidate; counter++) {
          createTrack(counter);
        }

        for (CDCWireHit& hit : m_hitList) {
          m_trackHitWrapperList.push_back(new CDCConformalHit(&hit));
        }

      }

      /// Delete objects
      void TearDown() override
      {
        // clean-up hits. Here the assumption is use that
        // no testing code modifies the trackHitWrapperList

        for (CDCConformalHit* pHit : m_trackHitWrapperList) {
          delete pHit;
        }
        m_trackHitWrapperList.clear();
      }

      /// Get vector of the generated hits
      std::vector<CDCConformalHit*>& getHitVector()
      {
        return m_trackHitWrapperList;
      }

      /// Mark hits as unused (for further QuadTree look-up)
      void markAllHitsAsUnused()
      {
        for (CDCConformalHit* hit : m_trackHitWrapperList) {
          hit->setUsedFlag(false);
        }
      }

      /**
       * Get track candidate
       * @param index index of the track
       * @return the reference to the track candidate
       */
      CDCTrack& getTrackCandidate(unsigned int index)
      {
        return m_trackList[index % numberOfPossibleTrackCandidate];
      }

    private:

      std::vector<CDCWireHit> m_hitList; /**< List of the generated CDCWireHits */
      std::vector<CDCTrack> m_trackList; /**< List of the generated CDCTracks */
      std::vector<CDCConformalHit*> m_trackHitWrapperList; /**< List of the generated CDCConformalHit */

      /**
       * Generate track candidate
       * @param number index of the track
       * @return reference to the generated track
       */
      CDCTrack& createTrack(unsigned int number)
      {
        B2ASSERT("Create CDCTrack: Only 2 tracks can be created whithin the test!", m_trackList.size() < 2);

        std::vector<const CDCWireHit*> trackHitList;
        if (number % 2 == 0) {
          for (std::vector<CDCWireHit>::iterator it = m_hitList.begin();
               it != (m_hitList.begin() + 76);
               ++it) {
            trackHitList.push_back(&(*it));
          }
        } else {
          for (std::vector<CDCWireHit>::iterator it = (m_hitList.begin() + 76);
               it != m_hitList.end();
               ++it) {
            trackHitList.push_back(&(*it));
          }
        }

        const CDCRiemannFitter& fitter = CDCRiemannFitter::getFitter();
        const CDCTrajectory2D& trackTrajectory2D = fitter.fit(trackHitList);
        CDCTrajectory3D trajectory3D(trackTrajectory2D, CDCTrajectorySZ::basicAssumption());

        m_trackList.emplace_back();

        CDCTrack& resultTrackCandidate = m_trackList.back();
        resultTrackCandidate.setStartTrajectory3D(trajectory3D);

        for (const CDCWireHit* item : trackHitList) {
          if (item->getAutomatonCell().hasTakenFlag() || item->getAutomatonCell().hasMaskedFlag()) continue;

          const CDCRecoHit3D& cdcRecoHit3D = CDCRecoHit3D::reconstructNearest(item, trackTrajectory2D);
          resultTrackCandidate.push_back(std::move(cdcRecoHit3D));
        }

        return resultTrackCandidate;
      }

      /**
       * Fill vectors of hits
       */
      void prepareWireHits(/*unsigned int number, CDCTrack& resultTrackCandidate*/)
      {
//          trackHitList.reserve(76);
        m_hitList.reserve(110); //76 hits - first track; 34 - second track
        m_hitList.emplace_back(WireID(6, 4, 251), 0.104);
        EXPECT_ALL_NEAR(m_hitList.back().getWire().getRefPos2D().x(), 18.7478, 1e-3);
        EXPECT_ALL_NEAR(m_hitList.back().getWire().getRefPos2D().y(), -85.4475, 1e-3);
        m_hitList.emplace_back(WireID(6, 4, 250), 0.272);
        EXPECT_ALL_NEAR(m_hitList.back().getWire().getRefPos2D().x(), 17.0665, 1e-3);
        EXPECT_ALL_NEAR(m_hitList.back().getWire().getRefPos2D().y(), -85.7991, 1e-3);
        m_hitList.emplace_back(WireID(6, 4, 249), 0.488);
        EXPECT_ALL_NEAR(m_hitList.back().getWire().getRefPos2D().x(), 15.3787, 1e-3);
        EXPECT_ALL_NEAR(m_hitList.back().getWire().getRefPos2D().y(), -86.1176, 1e-3);
        m_hitList.emplace_back(WireID(6, 4, 248), 0.764);
        EXPECT_ALL_NEAR(m_hitList.back().getWire().getRefPos2D().x(), 13.6849, 1e-3);
        EXPECT_ALL_NEAR(m_hitList.back().getWire().getRefPos2D().y(), -86.403, 1e-3);
        m_hitList.emplace_back(WireID(6, 3, 247), 0.9);
        EXPECT_ALL_NEAR(m_hitList.back().getWire().getRefPos2D().x(), 12.5689, 1e-3);
        EXPECT_ALL_NEAR(m_hitList.back().getWire().getRefPos2D().y(), -84.7329, 1e-3);
        m_hitList.emplace_back(WireID(6, 4, 247), 1.024);
        EXPECT_ALL_NEAR(m_hitList.back().getWire().getRefPos2D().x(), 11.9858, 1e-3);
        EXPECT_ALL_NEAR(m_hitList.back().getWire().getRefPos2D().y(), -86.655, 1e-3);
        m_hitList.emplace_back(WireID(6, 3, 246), 0.64);
        EXPECT_ALL_NEAR(m_hitList.back().getWire().getRefPos2D().x(), 10.9029, 1e-3);
        EXPECT_ALL_NEAR(m_hitList.back().getWire().getRefPos2D().y(), -84.9633, 1e-3);
        m_hitList.emplace_back(WireID(6, 3, 245), 0.304);
        EXPECT_ALL_NEAR(m_hitList.back().getWire().getRefPos2D().x(), 9.23265, 1e-3);
        EXPECT_ALL_NEAR(m_hitList.back().getWire().getRefPos2D().y(), -85.161, 1e-3);
        m_hitList.emplace_back(WireID(6, 3, 244), 0.012);
        EXPECT_ALL_NEAR(m_hitList.back().getWire().getRefPos2D().x(), 7.55884, 1e-3);
        EXPECT_ALL_NEAR(m_hitList.back().getWire().getRefPos2D().y(), -85.3258, 1e-3);
        m_hitList.emplace_back(WireID(6, 3, 243), 0.352);
        EXPECT_ALL_NEAR(m_hitList.back().getWire().getRefPos2D().x(), 5.88212, 1e-3);
        EXPECT_ALL_NEAR(m_hitList.back().getWire().getRefPos2D().y(), -85.4578, 1e-3);
        m_hitList.emplace_back(WireID(6, 3, 242), 0.74);
        EXPECT_ALL_NEAR(m_hitList.back().getWire().getRefPos2D().x(), 4.20314, 1e-3);
        EXPECT_ALL_NEAR(m_hitList.back().getWire().getRefPos2D().y(), -85.5568, 1e-3);
        m_hitList.emplace_back(WireID(6, 2, 241), 0.46);
        EXPECT_ALL_NEAR(m_hitList.back().getWire().getRefPos2D().x(), 1.64609, 1e-3);
        EXPECT_ALL_NEAR(m_hitList.back().getWire().getRefPos2D().y(), -83.8238, 1e-3);
        m_hitList.emplace_back(WireID(6, 2, 240), 0.02);
        EXPECT_ALL_NEAR(m_hitList.back().getWire().getRefPos2D().x(), -1.54006e-14, 1e-3);
        EXPECT_ALL_NEAR(m_hitList.back().getWire().getRefPos2D().y(), -83.84, 1e-3);
        m_hitList.emplace_back(WireID(6, 2, 239), 0.46);
        EXPECT_ALL_NEAR(m_hitList.back().getWire().getRefPos2D().x(), -1.64609, 1e-3);
        EXPECT_ALL_NEAR(m_hitList.back().getWire().getRefPos2D().y(), -83.8238, 1e-3);
        m_hitList.emplace_back(WireID(6, 2, 238), 0.884);
        EXPECT_ALL_NEAR(m_hitList.back().getWire().getRefPos2D().x(), -3.29154, 1e-3);
        EXPECT_ALL_NEAR(m_hitList.back().getWire().getRefPos2D().y(), -83.7754, 1e-3);
        m_hitList.emplace_back(WireID(6, 1, 238), 1.104);
        EXPECT_ALL_NEAR(m_hitList.back().getWire().getRefPos2D().x(), -2.41534, 1e-3);
        EXPECT_ALL_NEAR(m_hitList.back().getWire().getRefPos2D().y(), -81.9844, 1e-3);
        m_hitList.emplace_back(WireID(6, 1, 237), 0.612);
        EXPECT_ALL_NEAR(m_hitList.back().getWire().getRefPos2D().x(), -4.02453, 1e-3);
        EXPECT_ALL_NEAR(m_hitList.back().getWire().getRefPos2D().y(), -81.9212, 1e-3);
        m_hitList.emplace_back(WireID(6, 1, 236), 0.12);
        EXPECT_ALL_NEAR(m_hitList.back().getWire().getRefPos2D().x(), -5.63217, 1e-3);
        EXPECT_ALL_NEAR(m_hitList.back().getWire().getRefPos2D().y(), -81.8264, 1e-3);
        m_hitList.emplace_back(WireID(6, 1, 235), 0.356);
        EXPECT_ALL_NEAR(m_hitList.back().getWire().getRefPos2D().x(), -7.23764, 1e-3);
        EXPECT_ALL_NEAR(m_hitList.back().getWire().getRefPos2D().y(), -81.7, 1e-3);
        m_hitList.emplace_back(WireID(6, 1, 234), 0.884);
        EXPECT_ALL_NEAR(m_hitList.back().getWire().getRefPos2D().x(), -8.84032, 1e-3);
        EXPECT_ALL_NEAR(m_hitList.back().getWire().getRefPos2D().y(), -81.5422, 1e-3);
        m_hitList.emplace_back(WireID(6, 0, 235), 1.032);
        EXPECT_ALL_NEAR(m_hitList.back().getWire().getRefPos2D().x(), -7.86588, 1e-3);
        EXPECT_ALL_NEAR(m_hitList.back().getWire().getRefPos2D().y(), -79.8636, 1e-3);
        m_hitList.emplace_back(WireID(6, 0, 234), 0.52);
        EXPECT_ALL_NEAR(m_hitList.back().getWire().getRefPos2D().x(), -9.43238, 1e-3);
        EXPECT_ALL_NEAR(m_hitList.back().getWire().getRefPos2D().y(), -79.6937, 1e-3);
        m_hitList.emplace_back(WireID(6, 0, 233), 0.06);
        EXPECT_ALL_NEAR(m_hitList.back().getWire().getRefPos2D().x(), -10.9952, 1e-3);
        EXPECT_ALL_NEAR(m_hitList.back().getWire().getRefPos2D().y(), -79.4932, 1e-3);
        m_hitList.emplace_back(WireID(6, 0, 232), 0.62);
        EXPECT_ALL_NEAR(m_hitList.back().getWire().getRefPos2D().x(), -12.5539, 1e-3);
        EXPECT_ALL_NEAR(m_hitList.back().getWire().getRefPos2D().y(), -79.262, 1e-3);
        /*hitList.emplace_back(WireID(5, 5, 206), 1.116);
        EXPECT_ALL_NEAR(hitList.back().getWire().getRefPos2D().x(), -11.2806, 1e-3);
        EXPECT_ALL_NEAR(hitList.back().getWire().getRefPos2D().y(), -77.3568, 1e-3);
        hitList.emplace_back(WireID(5, 5, 205), 0.464);
        EXPECT_ALL_NEAR(hitList.back().getWire().getRefPos2D().x(), -12.9655, 1e-3);
        EXPECT_ALL_NEAR(hitList.back().getWire().getRefPos2D().y(), -77.0923, 1e-3);
        hitList.emplace_back(WireID(5, 5, 204), 0.168);
        EXPECT_ALL_NEAR(hitList.back().getWire().getRefPos2D().x(), -14.6441, 1e-3);
        EXPECT_ALL_NEAR(hitList.back().getWire().getRefPos2D().y(), -76.7911, 1e-3);
        hitList.emplace_back(WireID(5, 4, 204), 1.08);
        EXPECT_ALL_NEAR(hitList.back().getWire().getRefPos2D().x(), -15.142, 1e-3);
        EXPECT_ALL_NEAR(hitList.back().getWire().getRefPos2D().y(), -74.8981, 1e-3);
        hitList.emplace_back(WireID(5, 4, 203), 0.392);
        EXPECT_ALL_NEAR(hitList.back().getWire().getRefPos2D().x(), -16.7723, 1e-3);
        EXPECT_ALL_NEAR(hitList.back().getWire().getRefPos2D().y(), -74.55, 1e-3);
        hitList.emplace_back(WireID(5, 4, 202), 0.304);
        EXPECT_ALL_NEAR(hitList.back().getWire().getRefPos2D().x(), -18.3946, 1e-3);
        EXPECT_ALL_NEAR(hitList.back().getWire().getRefPos2D().y(), -74.1663, 1e-3);
        hitList.emplace_back(WireID(5, 3, 201), 0.968);
        EXPECT_ALL_NEAR(hitList.back().getWire().getRefPos2D().x(), -18.7572, 1e-3);
        EXPECT_ALL_NEAR(hitList.back().getWire().getRefPos2D().y(), -72.2055, 1e-3);
        hitList.emplace_back(WireID(5, 3, 200), 0.252);
        EXPECT_ALL_NEAR(hitList.back().getWire().getRefPos2D().x(), -20.3279, 1e-3);
        EXPECT_ALL_NEAR(hitList.back().getWire().getRefPos2D().y(), -71.7791, 1e-3);
        hitList.emplace_back(WireID(5, 3, 199), 0.476);
        EXPECT_ALL_NEAR(hitList.back().getWire().getRefPos2D().x(), -21.8889, 1e-3);
        EXPECT_ALL_NEAR(hitList.back().getWire().getRefPos2D().y(), -71.3186, 1e-3);
        hitList.emplace_back(WireID(5, 2, 199), 0.736);
        EXPECT_ALL_NEAR(hitList.back().getWire().getRefPos2D().x(), -22.1248, 1e-3);
        EXPECT_ALL_NEAR(hitList.back().getWire().getRefPos2D().y(), -69.3468, 1e-3);
        hitList.emplace_back(WireID(5, 2, 198), 0.008);
        EXPECT_ALL_NEAR(hitList.back().getWire().getRefPos2D().x(), -23.6323, 1e-3);
        EXPECT_ALL_NEAR(hitList.back().getWire().getRefPos2D().y(), -68.8476, 1e-3);
        hitList.emplace_back(WireID(5, 2, 197), 0.788);
        EXPECT_ALL_NEAR(hitList.back().getWire().getRefPos2D().x(), -25.1286, 1e-3);
        EXPECT_ALL_NEAR(hitList.back().getWire().getRefPos2D().y(), -68.3157, 1e-3);
        hitList.emplace_back(WireID(5, 1, 197), 1.188);
        EXPECT_ALL_NEAR(hitList.back().getWire().getRefPos2D().x(), -23.7844, 1e-3);
        EXPECT_ALL_NEAR(hitList.back().getWire().getRefPos2D().y(), -66.8757, 1e-3);
        hitList.emplace_back(WireID(5, 1, 196), 0.404);
        EXPECT_ALL_NEAR(hitList.back().getWire().getRefPos2D().x(), -25.2376, 1e-3);
        EXPECT_ALL_NEAR(hitList.back().getWire().getRefPos2D().y(), -66.3409, 1e-3);
        hitList.emplace_back(WireID(5, 1, 195), 0.356);
        EXPECT_ALL_NEAR(hitList.back().getWire().getRefPos2D().x(), -26.6789, 1e-3);
        EXPECT_ALL_NEAR(hitList.back().getWire().getRefPos2D().y(), -65.7746, 1e-3);
        hitList.emplace_back(WireID(5, 0, 195), 0.74);
        EXPECT_ALL_NEAR(hitList.back().getWire().getRefPos2D().x(), -26.7235, 1e-3);
        EXPECT_ALL_NEAR(hitList.back().getWire().getRefPos2D().y(), -63.8509, 1e-3);
        hitList.emplace_back(WireID(5, 0, 194), 0.04);
        EXPECT_ALL_NEAR(hitList.back().getWire().getRefPos2D().x(), -28.11, 1e-3);
        EXPECT_ALL_NEAR(hitList.back().getWire().getRefPos2D().y(), -63.2527, 1e-3);
        hitList.emplace_back(WireID(5, 0, 193), 0.832);
        EXPECT_ALL_NEAR(hitList.back().getWire().getRefPos2D().x(), -29.4832, 1e-3);
        EXPECT_ALL_NEAR(hitList.back().getWire().getRefPos2D().y(), -62.6244, 1e-3);*/
        m_hitList.emplace_back(WireID(4, 5, 173), 0.692);
        EXPECT_ALL_NEAR(m_hitList.back().getWire().getRefPos2D().x(), -29.5671, 1e-3);
        EXPECT_ALL_NEAR(m_hitList.back().getWire().getRefPos2D().y(), -60.5796, 1e-3);
        m_hitList.emplace_back(WireID(4, 5, 172), 0.22);
        EXPECT_ALL_NEAR(m_hitList.back().getWire().getRefPos2D().x(), -31.0449, 1e-3);
        EXPECT_ALL_NEAR(m_hitList.back().getWire().getRefPos2D().y(), -59.8358, 1e-3);
        m_hitList.emplace_back(WireID(4, 5, 171), 1.132);
        EXPECT_ALL_NEAR(m_hitList.back().getWire().getRefPos2D().x(), -32.504, 1e-3);
        EXPECT_ALL_NEAR(m_hitList.back().getWire().getRefPos2D().y(), -59.0559, 1e-3);
        m_hitList.emplace_back(WireID(4, 4, 172), 0.816);
        EXPECT_ALL_NEAR(m_hitList.back().getWire().getRefPos2D().x(), -30.9425, 1e-3);
        EXPECT_ALL_NEAR(m_hitList.back().getWire().getRefPos2D().y(), -57.8893, 1e-3);
        m_hitList.emplace_back(WireID(4, 4, 171), 0.136);
        EXPECT_ALL_NEAR(m_hitList.back().getWire().getRefPos2D().x(), -32.3538, 1e-3);
        EXPECT_ALL_NEAR(m_hitList.back().getWire().getRefPos2D().y(), -57.1125, 1e-3);
        m_hitList.emplace_back(WireID(4, 4, 170), 1.048);
        EXPECT_ALL_NEAR(m_hitList.back().getWire().getRefPos2D().x(), -33.7457, 1e-3);
        EXPECT_ALL_NEAR(m_hitList.back().getWire().getRefPos2D().y(), -56.3013, 1e-3);
        m_hitList.emplace_back(WireID(4, 3, 170), 0.884);
        EXPECT_ALL_NEAR(m_hitList.back().getWire().getRefPos2D().x(), -32.1358, 1e-3);
        EXPECT_ALL_NEAR(m_hitList.back().getWire().getRefPos2D().y(), -55.1387, 1e-3);
        m_hitList.emplace_back(WireID(4, 3, 169), 0.032);
        EXPECT_ALL_NEAR(m_hitList.back().getWire().getRefPos2D().x(), -33.4793, 1e-3);
        EXPECT_ALL_NEAR(m_hitList.back().getWire().getRefPos2D().y(), -54.3335, 1e-3);
        m_hitList.emplace_back(WireID(4, 3, 168), 0.96);
        EXPECT_ALL_NEAR(m_hitList.back().getWire().getRefPos2D().x(), -34.8026, 1e-3);
        EXPECT_ALL_NEAR(m_hitList.back().getWire().getRefPos2D().y(), -53.4955, 1e-3);
        m_hitList.emplace_back(WireID(4, 2, 169), 0.972);
        EXPECT_ALL_NEAR(m_hitList.back().getWire().getRefPos2D().x(), -33.1699, 1e-3);
        EXPECT_ALL_NEAR(m_hitList.back().getWire().getRefPos2D().y(), -52.3809, 1e-3);
        m_hitList.emplace_back(WireID(4, 2, 168), 0.044);
        EXPECT_ALL_NEAR(m_hitList.back().getWire().getRefPos2D().x(), -34.4454, 1e-3);
        EXPECT_ALL_NEAR(m_hitList.back().getWire().getRefPos2D().y(), -51.5511, 1e-3);
        m_hitList.emplace_back(WireID(4, 2, 167), 0.872);
        EXPECT_ALL_NEAR(m_hitList.back().getWire().getRefPos2D().x(), -35.7001, 1e-3);
        EXPECT_ALL_NEAR(m_hitList.back().getWire().getRefPos2D().y(), -50.6903, 1e-3);
        m_hitList.emplace_back(WireID(4, 1, 167), 1.004);
        EXPECT_ALL_NEAR(m_hitList.back().getWire().getRefPos2D().x(), -34.0457, 1e-3);
        EXPECT_ALL_NEAR(m_hitList.back().getWire().getRefPos2D().y(), -49.6238, 1e-3);
        m_hitList.emplace_back(WireID(4, 1, 166), 0.1);
        EXPECT_ALL_NEAR(m_hitList.back().getWire().getRefPos2D().x(), -35.2533, 1e-3);
        EXPECT_ALL_NEAR(m_hitList.back().getWire().getRefPos2D().y(), -48.7733, 1e-3);
        m_hitList.emplace_back(WireID(4, 1, 165), 0.828);
        EXPECT_ALL_NEAR(m_hitList.back().getWire().getRefPos2D().x(), -36.4397, 1e-3);
        EXPECT_ALL_NEAR(m_hitList.back().getWire().getRefPos2D().y(), -47.8935, 1e-3);
        m_hitList.emplace_back(WireID(4, 0, 166), 1.004);
        EXPECT_ALL_NEAR(m_hitList.back().getWire().getRefPos2D().x(), -34.7948, 1e-3);
        EXPECT_ALL_NEAR(m_hitList.back().getWire().getRefPos2D().y(), -46.9154, 1e-3);
        m_hitList.emplace_back(WireID(4, 0, 165), 0.084);
        EXPECT_ALL_NEAR(m_hitList.back().getWire().getRefPos2D().x(), -35.9357, 1e-3);
        EXPECT_ALL_NEAR(m_hitList.back().getWire().getRefPos2D().y(), -46.0473, 1e-3);
        m_hitList.emplace_back(WireID(4, 0, 164), 0.82);
        EXPECT_ALL_NEAR(m_hitList.back().getWire().getRefPos2D().x(), -37.0549, 1e-3);
        EXPECT_ALL_NEAR(m_hitList.back().getWire().getRefPos2D().y(), -45.1515, 1e-3);
        /*hitList.emplace_back(WireID(3, 5, 145), 0.508);
        EXPECT_ALL_NEAR(hitList.back().getWire().getRefPos2D().x(), -36.497, 1e-3);
        EXPECT_ALL_NEAR(hitList.back().getWire().getRefPos2D().y(), -42.8889, 1e-3);
        hitList.emplace_back(WireID(3, 5, 144), 0.5);
        EXPECT_ALL_NEAR(hitList.back().getWire().getRefPos2D().x(), -37.6855, 1e-3);
        EXPECT_ALL_NEAR(hitList.back().getWire().getRefPos2D().y(), -41.8484, 1e-3);
        hitList.emplace_back(WireID(3, 4, 145), 1.348);
        EXPECT_ALL_NEAR(hitList.back().getWire().getRefPos2D().x(), -35.9299, 1e-3);
        EXPECT_ALL_NEAR(hitList.back().getWire().getRefPos2D().y(), -41.0565, 1e-3);
        hitList.emplace_back(WireID(3, 4, 144), 0.292);
        EXPECT_ALL_NEAR(hitList.back().getWire().getRefPos2D().x(), -37.0672, 1e-3);
        EXPECT_ALL_NEAR(hitList.back().getWire().getRefPos2D().y(), -40.0326, 1e-3);
        hitList.emplace_back(WireID(3, 4, 143), 0.68);
        EXPECT_ALL_NEAR(hitList.back().getWire().getRefPos2D().x(), -38.1754, 1e-3);
        EXPECT_ALL_NEAR(hitList.back().getWire().getRefPos2D().y(), -38.9773, 1e-3);
        hitList.emplace_back(WireID(3, 3, 143), 1.136);
        EXPECT_ALL_NEAR(hitList.back().getWire().getRefPos2D().x(), -36.3716, 1e-3);
        EXPECT_ALL_NEAR(hitList.back().getWire().getRefPos2D().y(), -38.2065, 1e-3);
        hitList.emplace_back(WireID(3, 3, 142), 0.12);
        EXPECT_ALL_NEAR(hitList.back().getWire().getRefPos2D().x(), -37.4288, 1e-3);
        EXPECT_ALL_NEAR(hitList.back().getWire().getRefPos2D().y(), -37.1714, 1e-3);
        hitList.emplace_back(WireID(3, 3, 141), 0.872);
        EXPECT_ALL_NEAR(hitList.back().getWire().getRefPos2D().x(), -38.4566, 1e-3);
        EXPECT_ALL_NEAR(hitList.back().getWire().getRefPos2D().y(), -36.107, 1e-3);
        hitList.emplace_back(WireID(3, 2, 142), 0.96);
        EXPECT_ALL_NEAR(hitList.back().getWire().getRefPos2D().x(), -36.6397, 1e-3);
        EXPECT_ALL_NEAR(hitList.back().getWire().getRefPos2D().y(), -35.3939, 1e-3);
        hitList.emplace_back(WireID(3, 2, 141), 0.036);
        EXPECT_ALL_NEAR(hitList.back().getWire().getRefPos2D().x(), -37.618, 1e-3);
        EXPECT_ALL_NEAR(hitList.back().getWire().getRefPos2D().y(), -34.3524, 1e-3);
        hitList.emplace_back(WireID(3, 1, 140), 0.756);
        EXPECT_ALL_NEAR(hitList.back().getWire().getRefPos2D().x(), -36.7384, 1e-3);
        EXPECT_ALL_NEAR(hitList.back().getWire().getRefPos2D().y(), -32.6281, 1e-3);
        hitList.emplace_back(WireID(3, 1, 139), 0.204);
        EXPECT_ALL_NEAR(hitList.back().getWire().getRefPos2D().x(), -37.639, 1e-3);
        EXPECT_ALL_NEAR(hitList.back().getWire().getRefPos2D().y(), -31.5849, 1e-3);
        hitList.emplace_back(WireID(3, 0, 139), 0.588);
        EXPECT_ALL_NEAR(hitList.back().getWire().getRefPos2D().x(), -36.7106, 1e-3);
        EXPECT_ALL_NEAR(hitList.back().getWire().getRefPos2D().y(), -29.9495, 1e-3);
        hitList.emplace_back(WireID(3, 0, 138), 0.332);
        EXPECT_ALL_NEAR(hitList.back().getWire().getRefPos2D().x(), -37.5361, 1e-3);
        EXPECT_ALL_NEAR(hitList.back().getWire().getRefPos2D().y(), -28.9081, 1e-3);*/
        m_hitList.emplace_back(WireID(2, 5, 116), 1.1);
        EXPECT_ALL_NEAR(m_hitList.back().getWire().getRefPos2D().x(), -35.6944, 1e-3);
        EXPECT_ALL_NEAR(m_hitList.back().getWire().getRefPos2D().y(), -28.3291, 1e-3);
        m_hitList.emplace_back(WireID(2, 5, 115), 0.008);
        EXPECT_ALL_NEAR(m_hitList.back().getWire().getRefPos2D().x(), -36.6022, 1e-3);
        EXPECT_ALL_NEAR(m_hitList.back().getWire().getRefPos2D().y(), -27.146, 1e-3);
        m_hitList.emplace_back(WireID(2, 5, 114), 1.048);
        EXPECT_ALL_NEAR(m_hitList.back().getWire().getRefPos2D().x(), -37.4708, 1e-3);
        EXPECT_ALL_NEAR(m_hitList.back().getWire().getRefPos2D().y(), -25.9339, 1e-3);
        m_hitList.emplace_back(WireID(2, 4, 115), 0.712);
        EXPECT_ALL_NEAR(m_hitList.back().getWire().getRefPos2D().x(), -35.6027, 1e-3);
        EXPECT_ALL_NEAR(m_hitList.back().getWire().getRefPos2D().y(), -25.5125, 1e-3);
        m_hitList.emplace_back(WireID(2, 4, 114), 0.316);
        EXPECT_ALL_NEAR(m_hitList.back().getWire().getRefPos2D().x(), -36.4184, 1e-3);
        EXPECT_ALL_NEAR(m_hitList.back().getWire().getRefPos2D().y(), -24.334, 1e-3);
        m_hitList.emplace_back(WireID(2, 3, 113), 0.428);
        EXPECT_ALL_NEAR(m_hitList.back().getWire().getRefPos2D().x(), -35.282, 1e-3);
        EXPECT_ALL_NEAR(m_hitList.back().getWire().getRefPos2D().y(), -22.7486, 1e-3);
        m_hitList.emplace_back(WireID(2, 3, 112), 0.572);
        EXPECT_ALL_NEAR(m_hitList.back().getWire().getRefPos2D().x(), -36.0074, 1e-3);
        EXPECT_ALL_NEAR(m_hitList.back().getWire().getRefPos2D().y(), -21.582, 1e-3);
        m_hitList.emplace_back(WireID(2, 2, 112), 0.188);
        EXPECT_ALL_NEAR(m_hitList.back().getWire().getRefPos2D().x(), -34.7796, 1e-3);
        EXPECT_ALL_NEAR(m_hitList.back().getWire().getRefPos2D().y(), -20.08, 1e-3);
        m_hitList.emplace_back(WireID(2, 2, 111), 0.776);
        EXPECT_ALL_NEAR(m_hitList.back().getWire().getRefPos2D().x(), -35.418, 1e-3);
        EXPECT_ALL_NEAR(m_hitList.back().getWire().getRefPos2D().y(), -18.9313, 1e-3);
        m_hitList.emplace_back(WireID(2, 1, 111), 0.92);
        EXPECT_ALL_NEAR(m_hitList.back().getWire().getRefPos2D().x(), -33.5126, 1e-3);
        EXPECT_ALL_NEAR(m_hitList.back().getWire().getRefPos2D().y(), -18.6242, 1e-3);
        m_hitList.emplace_back(WireID(2, 1, 110), 0.024);
        EXPECT_ALL_NEAR(m_hitList.back().getWire().getRefPos2D().x(), -34.104, 1e-3);
        EXPECT_ALL_NEAR(m_hitList.back().getWire().getRefPos2D().y(), -17.5177, 1e-3);
        m_hitList.emplace_back(WireID(2, 1, 109), 0.928);
        EXPECT_ALL_NEAR(m_hitList.back().getWire().getRefPos2D().x(), -34.659, 1e-3);
        EXPECT_ALL_NEAR(m_hitList.back().getWire().getRefPos2D().y(), -16.3925, 1e-3);
        m_hitList.emplace_back(WireID(2, 0, 110), 0.776);
        EXPECT_ALL_NEAR(m_hitList.back().getWire().getRefPos2D().x(), -32.7538, 1e-3);
        EXPECT_ALL_NEAR(m_hitList.back().getWire().getRefPos2D().y(), -16.1524, 1e-3);
        m_hitList.emplace_back(WireID(2, 0, 109), 0.116);
        EXPECT_ALL_NEAR(m_hitList.back().getWire().getRefPos2D().x(), -33.2647, 1e-3);
        EXPECT_ALL_NEAR(m_hitList.back().getWire().getRefPos2D().y(), -15.0721, 1e-3);
        m_hitList.emplace_back(WireID(2, 0, 108), 0.992);
        EXPECT_ALL_NEAR(m_hitList.back().getWire().getRefPos2D().x(), -33.7401, 1e-3);
        EXPECT_ALL_NEAR(m_hitList.back().getWire().getRefPos2D().y(), -13.9756, 1e-3);
        /*hitList.emplace_back(WireID(1, 5, 87), 0.664);
        EXPECT_ALL_NEAR(hitList.back().getWire().getRefPos2D().x(), -31.4633, 1e-3);
        EXPECT_ALL_NEAR(hitList.back().getWire().getRefPos2D().y(), -13.7096, 1e-3);
        hitList.emplace_back(WireID(1, 5, 86), 0.3);
        EXPECT_ALL_NEAR(hitList.back().getWire().getRefPos2D().x(), -31.9773, 1e-3);
        EXPECT_ALL_NEAR(hitList.back().getWire().getRefPos2D().y(), -12.4638, 1e-3);
        hitList.emplace_back(WireID(1, 4, 86), 0.504);
        EXPECT_ALL_NEAR(hitList.back().getWire().getRefPos2D().x(), -30.4818, 1e-3);
        EXPECT_ALL_NEAR(hitList.back().getWire().getRefPos2D().y(), -11.3271, 1e-3);
        hitList.emplace_back(WireID(1, 4, 85), 0.424);
        EXPECT_ALL_NEAR(hitList.back().getWire().getRefPos2D().x(), -30.903, 1e-3);
        EXPECT_ALL_NEAR(hitList.back().getWire().getRefPos2D().y(), -10.1217, 1e-3);
        hitList.emplace_back(WireID(1, 3, 85), 1.256);
        EXPECT_ALL_NEAR(hitList.back().getWire().getRefPos2D().x(), -28.9845, 1e-3);
        EXPECT_ALL_NEAR(hitList.back().getWire().getRefPos2D().y(), -10.1821, 1e-3);
        hitList.emplace_back(WireID(1, 3, 84), 0.388);
        EXPECT_ALL_NEAR(hitList.back().getWire().getRefPos2D().x(), -29.3619, 1e-3);
        EXPECT_ALL_NEAR(hitList.back().getWire().getRefPos2D().y(), -9.03631, 1e-3);
        hitList.emplace_back(WireID(1, 3, 83), 0.5);
        EXPECT_ALL_NEAR(hitList.back().getWire().getRefPos2D().x(), -29.6941, 1e-3);
        EXPECT_ALL_NEAR(hitList.back().getWire().getRefPos2D().y(), -7.8766, 1e-3);
        hitList.emplace_back(WireID(1, 2, 84), 1.128);
        EXPECT_ALL_NEAR(hitList.back().getWire().getRefPos2D().x(), -27.8087, 1e-3);
        EXPECT_ALL_NEAR(hitList.back().getWire().getRefPos2D().y(), -7.96407, 1e-3);
        hitList.emplace_back(WireID(1, 2, 83), 0.28);
        EXPECT_ALL_NEAR(hitList.back().getWire().getRefPos2D().x(), -28.0999, 1e-3);
        EXPECT_ALL_NEAR(hitList.back().getWire().getRefPos2D().y(), -6.86616, 1e-3);
        hitList.emplace_back(WireID(1, 2, 82), 0.532);
        EXPECT_ALL_NEAR(hitList.back().getWire().getRefPos2D().x(), -28.3478, 1e-3);
        EXPECT_ALL_NEAR(hitList.back().getWire().getRefPos2D().y(), -5.75767, 1e-3);
        hitList.emplace_back(WireID(1, 1, 82), 1.084);
        EXPECT_ALL_NEAR(hitList.back().getWire().getRefPos2D().x(), -26.4782, 1e-3);
        EXPECT_ALL_NEAR(hitList.back().getWire().getRefPos2D().y(), -5.92152, 1e-3);
        hitList.emplace_back(WireID(1, 1, 81), 0.3);
        EXPECT_ALL_NEAR(hitList.back().getWire().getRefPos2D().x(), -26.6903, 1e-3);
        EXPECT_ALL_NEAR(hitList.back().getWire().getRefPos2D().y(), -4.87743, 1e-3);
        hitList.emplace_back(WireID(1, 1, 80), 0.472);
        EXPECT_ALL_NEAR(hitList.back().getWire().getRefPos2D().x(), -26.8612, 1e-3);
        EXPECT_ALL_NEAR(hitList.back().getWire().getRefPos2D().y(), -3.82581, 1e-3);
        hitList.emplace_back(WireID(1, 0, 81), 1.124);
        EXPECT_ALL_NEAR(hitList.back().getWire().getRefPos2D().x(), -25.0098, 1e-3);
        EXPECT_ALL_NEAR(hitList.back().getWire().getRefPos2D().y(), -4.06461, 1e-3);
        hitList.emplace_back(WireID(1, 0, 80), 0.428);
        EXPECT_ALL_NEAR(hitList.back().getWire().getRefPos2D().x(), -25.1501, 1e-3);
        EXPECT_ALL_NEAR(hitList.back().getWire().getRefPos2D().y(), -3.0796, 1e-3);
        hitList.emplace_back(WireID(1, 0, 79), 0.296);
        EXPECT_ALL_NEAR(hitList.back().getWire().getRefPos2D().x(), -25.2516, 1e-3);
        EXPECT_ALL_NEAR(hitList.back().getWire().getRefPos2D().y(), -2.08984, 1e-3);
        hitList.emplace_back(WireID(1, 0, 78), 0.972);
        EXPECT_ALL_NEAR(hitList.back().getWire().getRefPos2D().x(), -25.3141, 1e-3);
        EXPECT_ALL_NEAR(hitList.back().getWire().getRefPos2D().y(), -1.09686, 1e-3);*/
        m_hitList.emplace_back(WireID(0, 7, 81), 0.192);
        EXPECT_ALL_NEAR(m_hitList.back().getWire().getRefPos2D().x(), -23.7817, 1e-3);
        EXPECT_ALL_NEAR(m_hitList.back().getWire().getRefPos2D().y(), -0.934384, 1e-3);
        m_hitList.emplace_back(WireID(0, 7, 80), 0.452);
        EXPECT_ALL_NEAR(m_hitList.back().getWire().getRefPos2D().x(), -23.8, 1e-3);
        EXPECT_ALL_NEAR(m_hitList.back().getWire().getRefPos2D().y(), 2.91456e-15, 1e-3);
        m_hitList.emplace_back(WireID(0, 6, 80), 0.596);
        EXPECT_ALL_NEAR(m_hitList.back().getWire().getRefPos2D().x(), -22.7956, 1e-3);
        EXPECT_ALL_NEAR(m_hitList.back().getWire().getRefPos2D().y(), -0.447648, 1e-3);
        m_hitList.emplace_back(WireID(0, 6, 79), 0.024);
        EXPECT_ALL_NEAR(m_hitList.back().getWire().getRefPos2D().x(), -22.7956, 1e-3);
        EXPECT_ALL_NEAR(m_hitList.back().getWire().getRefPos2D().y(), 0.447648, 1e-3);
        m_hitList.emplace_back(WireID(0, 6, 78), 0.66);
        EXPECT_ALL_NEAR(m_hitList.back().getWire().getRefPos2D().x(), -22.7605, 1e-3);
        EXPECT_ALL_NEAR(m_hitList.back().getWire().getRefPos2D().y(), 1.34225, 1e-3);
        m_hitList.emplace_back(WireID(0, 5, 79), 0.388);
        EXPECT_ALL_NEAR(m_hitList.back().getWire().getRefPos2D().x(), -21.7832, 1e-3);
        EXPECT_ALL_NEAR(m_hitList.back().getWire().getRefPos2D().y(), 0.855864, 1e-3);
        m_hitList.emplace_back(WireID(0, 5, 78), 0.184);
        EXPECT_ALL_NEAR(m_hitList.back().getWire().getRefPos2D().x(), -21.7328, 1e-3);
        EXPECT_ALL_NEAR(m_hitList.back().getWire().getRefPos2D().y(), 1.71041, 1e-3);
        m_hitList.emplace_back(WireID(0, 4, 77), 0.296);
        EXPECT_ALL_NEAR(m_hitList.back().getWire().getRefPos2D().x(), -20.6998, 1e-3);
        EXPECT_ALL_NEAR(m_hitList.back().getWire().getRefPos2D().y(), 2.03876, 1e-3);
        m_hitList.emplace_back(WireID(0, 4, 76), 0.244);
        EXPECT_ALL_NEAR(m_hitList.back().getWire().getRefPos2D().x(), -20.6038, 1e-3);
        EXPECT_ALL_NEAR(m_hitList.back().getWire().getRefPos2D().y(), 2.84986, 1e-3);
        m_hitList.emplace_back(WireID(0, 3, 76), 0.268);
        EXPECT_ALL_NEAR(m_hitList.back().getWire().getRefPos2D().x(), -19.5562, 1e-3);
        EXPECT_ALL_NEAR(m_hitList.back().getWire().getRefPos2D().y(), 3.0974, 1e-3);
        m_hitList.emplace_back(WireID(0, 3, 75), 0.212);
        EXPECT_ALL_NEAR(m_hitList.back().getWire().getRefPos2D().x(), -19.4195, 1e-3);
        EXPECT_ALL_NEAR(m_hitList.back().getWire().getRefPos2D().y(), 3.86279, 1e-3);
        m_hitList.emplace_back(WireID(0, 2, 74), 0.316);
        EXPECT_ALL_NEAR(m_hitList.back().getWire().getRefPos2D().x(), -18.3632, 1e-3);
        EXPECT_ALL_NEAR(m_hitList.back().getWire().getRefPos2D().y(), 4.02901, 1e-3);
        m_hitList.emplace_back(WireID(0, 2, 73), 0.112);
        EXPECT_ALL_NEAR(m_hitList.back().getWire().getRefPos2D().x(), -18.1909, 1e-3);
        EXPECT_ALL_NEAR(m_hitList.back().getWire().getRefPos2D().y(), 4.74684, 1e-3);
        m_hitList.emplace_back(WireID(0, 2, 72), 0.588);
        EXPECT_ALL_NEAR(m_hitList.back().getWire().getRefPos2D().x(), -17.9905, 1e-3);
        EXPECT_ALL_NEAR(m_hitList.back().getWire().getRefPos2D().y(), 5.45735, 1e-3);
        m_hitList.emplace_back(WireID(0, 1, 73), 0.464);
        EXPECT_ALL_NEAR(m_hitList.back().getWire().getRefPos2D().x(), -17.1317, 1e-3);
        EXPECT_ALL_NEAR(m_hitList.back().getWire().getRefPos2D().y(), 4.83164, 1e-3);
        m_hitList.emplace_back(WireID(0, 1, 72), 0.028);
        EXPECT_ALL_NEAR(m_hitList.back().getWire().getRefPos2D().x(), -16.9288, 1e-3);
        EXPECT_ALL_NEAR(m_hitList.back().getWire().getRefPos2D().y(), 5.5005, 1e-3);
        m_hitList.emplace_back(WireID(0, 0, 70), 0.284);
        EXPECT_ALL_NEAR(m_hitList.back().getWire().getRefPos2D().x(), -15.6444, 1e-3);
        EXPECT_ALL_NEAR(m_hitList.back().getWire().getRefPos2D().y(), 6.1231, 1e-3);
        m_hitList.emplace_back(WireID(0, 0, 69), 0.088);
        EXPECT_ALL_NEAR(m_hitList.back().getWire().getRefPos2D().x(), -15.392, 1e-3);
        EXPECT_ALL_NEAR(m_hitList.back().getWire().getRefPos2D().y(), 6.73258, 1e-3);
        m_hitList.emplace_back(WireID(0, 0, 68), 0.416);
        EXPECT_ALL_NEAR(m_hitList.back().getWire().getRefPos2D().x(), -15.1158, 1e-3);
        EXPECT_ALL_NEAR(m_hitList.back().getWire().getRefPos2D().y(), 7.33168, 1e-3);

        EXPECT_EQ(m_hitList.size(), 76) << "Too few hits in the hit vector.";

//          trackHitList.reserve(34);
        m_hitList.emplace_back(WireID(0, 1, 140), 0.308);
        EXPECT_ALL_NEAR(m_hitList.back().getWire().getRefPos2D().x(), 12.5865, 1e-3);
        EXPECT_ALL_NEAR(m_hitList.back().getWire().getRefPos2D().y(), -12.5865, 1e-3);
        m_hitList.emplace_back(WireID(0, 2, 139), 0.08);
        EXPECT_ALL_NEAR(m_hitList.back().getWire().getRefPos2D().x(), 13.03, 1e-3);
        EXPECT_ALL_NEAR(m_hitList.back().getWire().getRefPos2D().y(), -13.552, 1e-3);
        m_hitList.emplace_back(WireID(0, 3, 139), 0.16);
        EXPECT_ALL_NEAR(m_hitList.back().getWire().getRefPos2D().x(), 13.4403, 1e-3);
        EXPECT_ALL_NEAR(m_hitList.back().getWire().getRefPos2D().y(), -14.5396, 1e-3);
        m_hitList.emplace_back(WireID(0, 4, 139), 0.404);
        EXPECT_ALL_NEAR(m_hitList.back().getWire().getRefPos2D().x(), 14.4162, 1e-3);
        EXPECT_ALL_NEAR(m_hitList.back().getWire().getRefPos2D().y(), -14.9938, 1e-3);
        m_hitList.emplace_back(WireID(0, 4, 138), 0.38);
        EXPECT_ALL_NEAR(m_hitList.back().getWire().getRefPos2D().x(), 13.8165, 1e-3);
        EXPECT_ALL_NEAR(m_hitList.back().getWire().getRefPos2D().y(), -15.5482, 1e-3);
        m_hitList.emplace_back(WireID(0, 5, 139), 0.132);
        EXPECT_ALL_NEAR(m_hitList.back().getWire().getRefPos2D().x(), 14.7979, 1e-3);
        EXPECT_ALL_NEAR(m_hitList.back().getWire().getRefPos2D().y(), -16.0082, 1e-3);
        m_hitList.emplace_back(WireID(0, 6, 138), 0.108);
        EXPECT_ALL_NEAR(m_hitList.back().getWire().getRefPos2D().x(), 15.145, 1e-3);
        EXPECT_ALL_NEAR(m_hitList.back().getWire().getRefPos2D().y(), -17.0432, 1e-3);
        m_hitList.emplace_back(WireID(0, 7, 139), 0.48);
        EXPECT_ALL_NEAR(m_hitList.back().getWire().getRefPos2D().x(), 16.1555, 1e-3);
        EXPECT_ALL_NEAR(m_hitList.back().getWire().getRefPos2D().y(), -17.4769, 1e-3);
        m_hitList.emplace_back(WireID(0, 7, 138), 0.424);
        EXPECT_ALL_NEAR(m_hitList.back().getWire().getRefPos2D().x(), 15.4569, 1e-3);
        EXPECT_ALL_NEAR(m_hitList.back().getWire().getRefPos2D().y(), -18.0977, 1e-3);
        /*hitList.emplace_back(WireID(1, 0, 136), 0.532);
        EXPECT_ALL_NEAR(hitList.back().getWire().getRefPos2D().x(), 17.2743, 1e-3);
        EXPECT_ALL_NEAR(hitList.back().getWire().getRefPos2D().y(), -18.5367, 1e-3);
        hitList.emplace_back(WireID(1, 0, 135), 0.452);
        EXPECT_ALL_NEAR(hitList.back().getWire().getRefPos2D().x(), 16.5332, 1e-3);
        EXPECT_ALL_NEAR(hitList.back().getWire().getRefPos2D().y(), -19.2006, 1e-3);
        hitList.emplace_back(WireID(1, 1, 135), 0.396);
        EXPECT_ALL_NEAR(hitList.back().getWire().getRefPos2D().x(), 18.1043, 1e-3);
        EXPECT_ALL_NEAR(hitList.back().getWire().getRefPos2D().y(), -20.2087, 1e-3);
        hitList.emplace_back(WireID(1, 2, 135), 0.26);
        EXPECT_ALL_NEAR(hitList.back().getWire().getRefPos2D().x(), 18.8749, 1e-3);
        EXPECT_ALL_NEAR(hitList.back().getWire().getRefPos2D().y(), -21.92, 1e-3);
        hitList.emplace_back(WireID(1, 1, 134), 0.64);
        EXPECT_ALL_NEAR(hitList.back().getWire().getRefPos2D().x(), 17.297, 1e-3);
        EXPECT_ALL_NEAR(hitList.back().getWire().getRefPos2D().y(), -20.9039, 1e-3);
        hitList.emplace_back(WireID(1, 3, 134), 0.092);
        EXPECT_ALL_NEAR(hitList.back().getWire().getRefPos2D().x(), 19.5848, 1e-3);
        EXPECT_ALL_NEAR(hitList.back().getWire().getRefPos2D().y(), -23.6688, 1e-3);
        hitList.emplace_back(WireID(1, 4, 134), 0.16);
        EXPECT_ALL_NEAR(hitList.back().getWire().getRefPos2D().x(), 20.1921, 1e-3);
        EXPECT_ALL_NEAR(hitList.back().getWire().getRefPos2D().y(), -25.4896, 1e-3);
        hitList.emplace_back(WireID(1, 5, 133), 0.524);
        EXPECT_ALL_NEAR(hitList.back().getWire().getRefPos2D().x(), 20.6758, 1e-3);
        EXPECT_ALL_NEAR(hitList.back().getWire().getRefPos2D().y(), -27.3935, 1e-3);*/
        m_hitList.emplace_back(WireID(2, 0, 163), 0.228);
        EXPECT_ALL_NEAR(m_hitList.back().getWire().getRefPos2D().x(), 21.2721, 1e-3);
        EXPECT_ALL_NEAR(m_hitList.back().getWire().getRefPos2D().y(), -29.6852, 1e-3);
        m_hitList.emplace_back(WireID(2, 1, 162), 0.356);
        EXPECT_ALL_NEAR(m_hitList.back().getWire().getRefPos2D().x(), 21.8193, 1e-3);
        EXPECT_ALL_NEAR(m_hitList.back().getWire().getRefPos2D().y(), -31.5258, 1e-3);
        m_hitList.emplace_back(WireID(2, 2, 163), 0.776);
        EXPECT_ALL_NEAR(m_hitList.back().getWire().getRefPos2D().x(), 23.3923, 1e-3);
        EXPECT_ALL_NEAR(m_hitList.back().getWire().getRefPos2D().y(), -32.6439, 1e-3);
        m_hitList.emplace_back(WireID(2, 2, 162), 0.46);
        EXPECT_ALL_NEAR(m_hitList.back().getWire().getRefPos2D().x(), 22.3117, 1e-3);
        EXPECT_ALL_NEAR(m_hitList.back().getWire().getRefPos2D().y(), -33.3918, 1e-3);
        m_hitList.emplace_back(WireID(2, 3, 162), 0.744);
        EXPECT_ALL_NEAR(m_hitList.back().getWire().getRefPos2D().x(), 23.8908, 1e-3);
        EXPECT_ALL_NEAR(m_hitList.back().getWire().getRefPos2D().y(), -34.5188, 1e-3);
        m_hitList.emplace_back(WireID(2, 3, 161), 0.58);
        EXPECT_ALL_NEAR(m_hitList.back().getWire().getRefPos2D().x(), 22.7486, 1e-3);
        EXPECT_ALL_NEAR(m_hitList.back().getWire().getRefPos2D().y(), -35.282, 1e-3);
        m_hitList.emplace_back(WireID(2, 4, 162), 0.656);
        EXPECT_ALL_NEAR(m_hitList.back().getWire().getRefPos2D().x(), 24.334, 1e-3);
        EXPECT_ALL_NEAR(m_hitList.back().getWire().getRefPos2D().y(), -36.4184, 1e-3);
        m_hitList.emplace_back(WireID(2, 4, 161), 0.68);
        EXPECT_ALL_NEAR(m_hitList.back().getWire().getRefPos2D().x(), 23.1294, 1e-3);
        EXPECT_ALL_NEAR(m_hitList.back().getWire().getRefPos2D().y(), -37.1951, 1e-3);
        m_hitList.emplace_back(WireID(2, 5, 161), 0.568);
        EXPECT_ALL_NEAR(m_hitList.back().getWire().getRefPos2D().x(), 24.694, 1e-3);
        EXPECT_ALL_NEAR(m_hitList.back().getWire().getRefPos2D().y(), -38.2992, 1e-3);
        m_hitList.emplace_back(WireID(2, 5, 160), 0.812);
        EXPECT_ALL_NEAR(m_hitList.back().getWire().getRefPos2D().x(), 23.4277, 1e-3);
        EXPECT_ALL_NEAR(m_hitList.back().getWire().getRefPos2D().y(), -39.0867, 1e-3);
        /*hitList.emplace_back(WireID(3, 0, 190), 0.54);
        EXPECT_ALL_NEAR(hitList.back().getWire().getRefPos2D().x(), 24.5237, 1e-3);
        EXPECT_ALL_NEAR(hitList.back().getWire().getRefPos2D().y(), -40.5367, 1e-3);
        hitList.emplace_back(WireID(3, 1, 188), 0.688);
        EXPECT_ALL_NEAR(hitList.back().getWire().getRefPos2D().x(), 23.635, 1e-3);
        EXPECT_ALL_NEAR(hitList.back().getWire().getRefPos2D().y(), -43.0777, 1e-3);
        hitList.emplace_back(WireID(3, 2, 188), 0.656);
        EXPECT_ALL_NEAR(hitList.back().getWire().getRefPos2D().x(), 23.8674, 1e-3);
        EXPECT_ALL_NEAR(hitList.back().getWire().getRefPos2D().y(), -45.0061, 1e-3);
        hitList.emplace_back(WireID(3, 3, 188), 0.664);
        EXPECT_ALL_NEAR(hitList.back().getWire().getRefPos2D().x(), 25.357, 1e-3);
        EXPECT_ALL_NEAR(hitList.back().getWire().getRefPos2D().y(), -46.2564, 1e-3);
        hitList.emplace_back(WireID(3, 3, 187), 0.68);
        EXPECT_ALL_NEAR(hitList.back().getWire().getRefPos2D().x(), 24.0497, 1e-3);
        EXPECT_ALL_NEAR(hitList.back().getWire().getRefPos2D().y(), -46.9493, 1e-3);
        hitList.emplace_back(WireID(3, 4, 188), 0.724);
        EXPECT_ALL_NEAR(hitList.back().getWire().getRefPos2D().x(), 25.5436, 1e-3);
        EXPECT_ALL_NEAR(hitList.back().getWire().getRefPos2D().y(), -48.2091, 1e-3);
        hitList.emplace_back(WireID(3, 4, 187), 0.656);
        EXPECT_ALL_NEAR(hitList.back().getWire().getRefPos2D().x(), 24.1815, 1e-3);
        EXPECT_ALL_NEAR(hitList.back().getWire().getRefPos2D().y(), -48.9065, 1e-3);
        hitList.emplace_back(WireID(3, 5, 186), 0.676);
        EXPECT_ALL_NEAR(hitList.back().getWire().getRefPos2D().x(), 24.2413, 1e-3);
        EXPECT_ALL_NEAR(hitList.back().getWire().getRefPos2D().y(), -50.8316, 1e-3);*/
        m_hitList.emplace_back(WireID(4, 0, 211), 0.42);
        EXPECT_ALL_NEAR(m_hitList.back().getWire().getRefPos2D().x(), 26.2618, 1e-3);
        EXPECT_ALL_NEAR(m_hitList.back().getWire().getRefPos2D().y(), -52.1732, 1e-3);
        m_hitList.emplace_back(WireID(4, 0, 210), 0.872);
        EXPECT_ALL_NEAR(m_hitList.back().getWire().getRefPos2D().x(), 24.9735, 1e-3);
        EXPECT_ALL_NEAR(m_hitList.back().getWire().getRefPos2D().y(), -52.802, 1e-3);
        m_hitList.emplace_back(WireID(4, 1, 210), 0.548);
        EXPECT_ALL_NEAR(m_hitList.back().getWire().getRefPos2D().x(), 26.3959, 1e-3);
        EXPECT_ALL_NEAR(m_hitList.back().getWire().getRefPos2D().y(), -54.0822, 1e-3);
        m_hitList.emplace_back(WireID(4, 1, 209), 0.796);
        EXPECT_ALL_NEAR(m_hitList.back().getWire().getRefPos2D().x(), 25.0607, 1e-3);
        EXPECT_ALL_NEAR(m_hitList.back().getWire().getRefPos2D().y(), -54.7137, 1e-3);
        m_hitList.emplace_back(WireID(4, 2, 210), 0.716);
        EXPECT_ALL_NEAR(m_hitList.back().getWire().getRefPos2D().x(), 26.5084, 1e-3);
        EXPECT_ALL_NEAR(m_hitList.back().getWire().getRefPos2D().y(), -56.0473, 1e-3);
        m_hitList.emplace_back(WireID(4, 2, 209), 0.656);
        EXPECT_ALL_NEAR(m_hitList.back().getWire().getRefPos2D().x(), 25.125, 1e-3);
        EXPECT_ALL_NEAR(m_hitList.back().getWire().getRefPos2D().y(), -56.681, 1e-3);
        m_hitList.emplace_back(WireID(4, 3, 209), 0.856);
        EXPECT_ALL_NEAR(m_hitList.back().getWire().getRefPos2D().x(), 26.5765, 1e-3);
        EXPECT_ALL_NEAR(m_hitList.back().getWire().getRefPos2D().y(), -58.0231, 1e-3);
        m_hitList.emplace_back(WireID(4, 4, 209), 1.056);
        EXPECT_ALL_NEAR(m_hitList.back().getWire().getRefPos2D().x(), 26.6, 1e-3);
        EXPECT_ALL_NEAR(m_hitList.back().getWire().getRefPos2D().y(), -60.0087, 1e-3);
        m_hitList.emplace_back(WireID(4, 4, 208), 0.36);
        EXPECT_ALL_NEAR(m_hitList.back().getWire().getRefPos2D().x(), 25.1193, 1e-3);
        EXPECT_ALL_NEAR(m_hitList.back().getWire().getRefPos2D().y(), -60.6435, 1e-3);
        m_hitList.emplace_back(WireID(4, 5, 207), 0.232);
        EXPECT_ALL_NEAR(m_hitList.back().getWire().getRefPos2D().x(), 25.0305, 1e-3);
        EXPECT_ALL_NEAR(m_hitList.back().getWire().getRefPos2D().y(), -62.5906, 1e-3);
        /*hitList.emplace_back(WireID(5, 0, 231), 0.224);
        EXPECT_ALL_NEAR(hitList.back().getWire().getRefPos2D().x(), 26.253, 1e-3);
        EXPECT_ALL_NEAR(hitList.back().getWire().getRefPos2D().y(), -64.0458, 1e-3);
        hitList.emplace_back(WireID(5, 0, 230), 1.088);
        EXPECT_ALL_NEAR(hitList.back().getWire().getRefPos2D().x(), 24.8496, 1e-3);
        EXPECT_ALL_NEAR(hitList.back().getWire().getRefPos2D().y(), -64.6032, 1e-3);
        hitList.emplace_back(WireID(5, 1, 230), 0.452);
        EXPECT_ALL_NEAR(hitList.back().getWire().getRefPos2D().x(), 26.2121, 1e-3);
        EXPECT_ALL_NEAR(hitList.back().getWire().getRefPos2D().y(), -65.962, 1e-3);
        hitList.emplace_back(WireID(5, 1, 229), 0.912);
        EXPECT_ALL_NEAR(hitList.back().getWire().getRefPos2D().x(), 24.7669, 1e-3);
        EXPECT_ALL_NEAR(hitList.back().getWire().getRefPos2D().y(), -66.5181, 1e-3);
        hitList.emplace_back(WireID(5, 2, 230), 0.72);
        EXPECT_ALL_NEAR(hitList.back().getWire().getRefPos2D().x(), 26.151, 1e-3);
        EXPECT_ALL_NEAR(hitList.back().getWire().getRefPos2D().y(), -67.9309, 1e-3);
        hitList.emplace_back(WireID(5, 2, 229), 0.632);
        EXPECT_ALL_NEAR(hitList.back().getWire().getRefPos2D().x(), 24.6629, 1e-3);
        EXPECT_ALL_NEAR(hitList.back().getWire().getRefPos2D().y(), -68.4852, 1e-3);
        hitList.emplace_back(WireID(5, 3, 229), 1.016);
        EXPECT_ALL_NEAR(hitList.back().getWire().getRefPos2D().x(), 26.0504, 1e-3);
        EXPECT_ALL_NEAR(hitList.back().getWire().getRefPos2D().y(), -69.906, 1e-3);
        hitList.emplace_back(WireID(5, 3, 228), 0.34);
        EXPECT_ALL_NEAR(hitList.back().getWire().getRefPos2D().x(), 24.5192, 1e-3);
        EXPECT_ALL_NEAR(hitList.back().getWire().getRefPos2D().y(), -70.4576, 1e-3);
        hitList.emplace_back(WireID(5, 4, 228), 0.04);
        EXPECT_ALL_NEAR(hitList.back().getWire().getRefPos2D().x(), 24.3357, 1e-3);
        EXPECT_ALL_NEAR(hitList.back().getWire().getRefPos2D().y(), -72.4347, 1e-3);
        hitList.emplace_back(WireID(5, 5, 227), 0.22);
        EXPECT_ALL_NEAR(hitList.back().getWire().getRefPos2D().x(), 24.0968, 1e-3);
        EXPECT_ALL_NEAR(hitList.back().getWire().getRefPos2D().y(), -74.3685, 1e-3);
        hitList.emplace_back(WireID(5, 5, 226), 1.196);
        EXPECT_ALL_NEAR(hitList.back().getWire().getRefPos2D().x(), 22.4687, 1e-3);
        EXPECT_ALL_NEAR(hitList.back().getWire().getRefPos2D().y(), -74.8765, 1e-3);*/
        m_hitList.emplace_back(WireID(6, 0, 254), 0.104);
        EXPECT_ALL_NEAR(m_hitList.back().getWire().getRefPos2D().x(), 21.7831, 1e-3);
        EXPECT_ALL_NEAR(m_hitList.back().getWire().getRefPos2D().y(), -77.237, 1e-3);
        m_hitList.emplace_back(WireID(6, 1, 253), 0.504);
        EXPECT_ALL_NEAR(m_hitList.back().getWire().getRefPos2D().x(), 21.4875, 1e-3);
        EXPECT_ALL_NEAR(m_hitList.back().getWire().getRefPos2D().y(), -79.1553, 1e-3);
        m_hitList.emplace_back(WireID(6, 1, 252), 0.78);
        EXPECT_ALL_NEAR(m_hitList.back().getWire().getRefPos2D().x(), 19.9292, 1e-3);
        EXPECT_ALL_NEAR(m_hitList.back().getWire().getRefPos2D().y(), -79.562, 1e-3);
        m_hitList.emplace_back(WireID(6, 2, 253), 0.968);
        EXPECT_ALL_NEAR(m_hitList.back().getWire().getRefPos2D().x(), 21.1689, 1e-3);
        EXPECT_ALL_NEAR(m_hitList.back().getWire().getRefPos2D().y(), -81.1235, 1e-3);
        m_hitList.emplace_back(WireID(6, 2, 252), 0.332);
        EXPECT_ALL_NEAR(m_hitList.back().getWire().getRefPos2D().x(), 19.5721, 1e-3);
        EXPECT_ALL_NEAR(m_hitList.back().getWire().getRefPos2D().y(), -81.5235, 1e-3);

        EXPECT_EQ(m_hitList.size(), 110) << "Too few hits in the hit vector.";

        std::sort(m_hitList.begin(), m_hitList.end());
      }
    };
  }
}
