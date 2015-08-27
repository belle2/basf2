/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Hauth <thomas.hauth@kit.edu>                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/trackFindingCDC/legendre/tests_fixtures/CDCLegendreTestFixture.h>

#include <tracking/trackFindingCDC/legendre/quadtree/CDCLegendreQuadTree.h>
#include <tracking/trackFindingCDC/legendre/quadtree/AxialHitQuadTreeProcessor.h>
#include <tracking/trackFindingCDC/legendre/quadtree/QuadTreeItem.h>

#include <set>
#include <cmath>
#include <vector>
#include <memory>
#include <chrono>
#include <algorithm>
#include <gtest/gtest.h>

using namespace std;

using namespace Belle2;
using namespace TrackFindingCDC;

TEST_F(CDCLegendreTestFixture, legendre_QuadTreeTest)
{
  /*
   AxialHitQuadTreeProcessor::ChildRanges ranges(AxialHitQuadTreeProcessor::rangeX(0, std::pow(2, 13)),
                                                 AxialHitQuadTreeProcessor::rangeY(-0.15, 0.15));
   std::vector<AxialHitQuadTreeProcessor::ReturnList> candidates;

   markAllHitsAsUnused();
   std::set<TrackHit*>& hits_set = getHitSet();
   std::vector<TrackHit*> hitVector;

   for (TrackHit* trackHit : hits_set) {
     if (trackHit->getSuperlayerId() % 2 == 0)
       hitVector.push_back(trackHit);
   }

   AxialHitQuadTreeProcessor::CandidateProcessorLambda lmdProcessor = [&candidates](const AxialHitQuadTreeProcessor::ReturnList & hits,
   AxialHitQuadTreeProcessor::QuadTree*) {
     candidates.push_back(hits);
   };

   std::function< double(double) > lmdFunctionResQTOrigin = [&](double r_qt) -> double {
     double res;
     //TODO: bug is here!
     if ((TrackCandidate::convertRhoToPt(fabs(r_qt)) > 3.) && (r_qt != 0))
       r_qt = fabs(TrackCandidate::convertPtToRho(3.)) * r_qt / fabs(r_qt);

     if (r_qt != 0)
       res = exp(-16.1987 * TrackCandidate::convertRhoToPt(fabs(r_qt)) - 5.96206)
       + 0.000190872 - 0.0000739319 * TrackCandidate::convertRhoToPt(fabs(r_qt));

     else
       res = 0.00005;

     B2DEBUG(100, "origin: res = " << res << "; r = " << r_qt);
     return res;
   };

   auto now = std::chrono::high_resolution_clock::now();
   AxialHitQuadTreeProcessor qtProcessor1(13, ranges, lmdFunctionResQTOrigin);
   qtProcessor1.provideItemsSet(hitVector);

   // actual filling of the hits into the quad tree structure
   qtProcessor1.fillGivenTree(lmdProcessor, 30);

   AxialHitQuadTreeProcessor qtProcessor2(11, ranges, lmdFunctionResQTOrigin);
   qtProcessor2.provideItemsSet(hitVector);

   // actual filling of the hits into the quad tree structure
   qtProcessor2.fillGivenTree(lmdProcessor, 30);
   auto later = std::chrono::high_resolution_clock::now();

   ASSERT_EQ(numberOfPossibleTrackCandidate, candidates.size());

   std::chrono::duration<double> time_span = std::chrono::duration_cast<std::chrono::duration<double>>(later - now);
   B2INFO("QuadTree took " << time_span.count() << " seconds, found " << candidates.size() << " candidates");

   // Check for the parameters of the track candidates
   // The actual hit numbers are more than 30, but this is somewhat a lower bound
   EXPECT_GE(candidates[0].size(), 30);
   EXPECT_GE(candidates[1].size(), 30);*/
}

