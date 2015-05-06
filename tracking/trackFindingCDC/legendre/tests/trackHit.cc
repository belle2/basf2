/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/trackFindingCDC/legendre/tests_fixtures/CDCLegendreTestFixture.h>
#include <tracking/trackFindingCDC/legendre/TrackHit.h>
#include <framework/utilities/TestHelpers.h>
#include <cdc/geometry/CDCGeometryPar.h>

#include <vector>
#include <gtest/gtest.h>

using namespace std;

using namespace Belle2;
using namespace TrackFindingCDC;


TEST_F(CDCLegendreTestFixture, legendre_TrackHit)
{
  const CDC::CDCGeometryPar& cdcg = CDC::CDCGeometryPar::Instance();
  TrackCandidate* trackCandidate = getTrackCandidate(0);
  const std::vector<TrackHit*>& hits = trackCandidate->getTrackHits();

  TrackHit* testHit = hits[0];
  EXPECT_ALL_NEAR(testHit->getForwardWirePosition().x(), cdcg.wireForwardPosition(testHit->getLayerId(), testHit->getWireId()).x(),
                  1e-4);
  EXPECT_ALL_NEAR(testHit->getForwardWirePosition().y(), cdcg.wireForwardPosition(testHit->getLayerId(), testHit->getWireId()).y(),
                  1e-4);
  EXPECT_ALL_NEAR(testHit->getForwardWirePosition().z(), cdcg.wireForwardPosition(testHit->getLayerId(), testHit->getWireId()).z(),
                  1e-4);
  EXPECT_ALL_NEAR(testHit->getBackwardWirePosition().x(), cdcg.wireBackwardPosition(testHit->getLayerId(), testHit->getWireId()).x(),
                  1e-4);
  EXPECT_ALL_NEAR(testHit->getBackwardWirePosition().y(), cdcg.wireBackwardPosition(testHit->getLayerId(), testHit->getWireId()).y(),
                  1e-4);
  EXPECT_ALL_NEAR(testHit->getBackwardWirePosition().z(), cdcg.wireBackwardPosition(testHit->getLayerId(), testHit->getWireId()).z(),
                  1e-4);

  EXPECT_EQ(testHit->getCurvatureSignWrt(0, 0), -1);
  EXPECT_EQ(testHit->getCurvatureSignWrt(20.3478, -5.4578), -1);
  EXPECT_EQ(testHit->getCurvatureSignWrt(-478, 87.3246234), 1);

  testHit = hits[41];
  EXPECT_ALL_NEAR(testHit->getForwardWirePosition().x(), cdcg.wireForwardPosition(testHit->getLayerId(), testHit->getWireId()).x(),
                  1e-4);
  EXPECT_ALL_NEAR(testHit->getForwardWirePosition().y(), cdcg.wireForwardPosition(testHit->getLayerId(), testHit->getWireId()).y(),
                  1e-4);
  EXPECT_ALL_NEAR(testHit->getForwardWirePosition().z(), cdcg.wireForwardPosition(testHit->getLayerId(), testHit->getWireId()).z(),
                  1e-4);
  EXPECT_ALL_NEAR(testHit->getBackwardWirePosition().x(), cdcg.wireBackwardPosition(testHit->getLayerId(), testHit->getWireId()).x(),
                  1e-4);
  EXPECT_ALL_NEAR(testHit->getBackwardWirePosition().y(), cdcg.wireBackwardPosition(testHit->getLayerId(), testHit->getWireId()).y(),
                  1e-4);
  EXPECT_ALL_NEAR(testHit->getBackwardWirePosition().z(), cdcg.wireBackwardPosition(testHit->getLayerId(), testHit->getWireId()).z(),
                  1e-4);

  EXPECT_EQ(testHit->getCurvatureSignWrt(0, 0), -1);
  EXPECT_EQ(testHit->getCurvatureSignWrt(20.3478, -5.4578), -1);
  EXPECT_EQ(testHit->getCurvatureSignWrt(-478, 87.3246234), 1);
}
