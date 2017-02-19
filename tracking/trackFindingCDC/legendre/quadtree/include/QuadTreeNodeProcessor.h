/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Viktor Trusov, Nils Braun                                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>
#include <tracking/trackFindingCDC/processing/AxialTrackUtil.h>
#include <tracking/trackFindingCDC/legendre/quadtree/AxialHitQuadTreeProcessor.h>
#include <tracking/trackFindingCDC/legendre/quadtree/ConformalExtension.h>

#include <vector>
#include <map>
#include <cmath>


namespace Belle2 {
  namespace TrackFindingCDC {
    class CDCWireHit;

    /**
     * Class which used as interface between QuadTree processor and TrackCreator
     */
    class QuadTreeNodeProcessor {

    public:
      /// Constructor
      QuadTreeNodeProcessor()
      {
      }

      /**
       *  this lambda function will forward the found candidates to the CandidateCreate for further processing
       *  hits belonging to found candidates will be marked as used and ignored for further filling iterations
       */
      AxialHitQuadTreeProcessor::CandidateProcessorLambda getLambdaInterface(const std::vector<const CDCWireHit*>& allAxialWireHits,
          std::vector<CDCTrack>& tracks)
      {
        AxialHitQuadTreeProcessor::CandidateProcessorLambda lmdCandidateProcessingFinal =
          [&](const AxialHitQuadTreeProcessor::ReturnList & hits __attribute__((unused)),
        AxialHitQuadTreeProcessor::QuadTree * qt) -> void {
          this->candidateProcessingFinal(qt, allAxialWireHits, tracks);
        };
        return lmdCandidateProcessingFinal;
      }

      /// Gets hits from quadtree node, convert to QuadTreeHitWrapper and passes for further processing
      void candidateProcessingFinal(AxialHitQuadTreeProcessor::QuadTree* qt,
                                    const std::vector<const CDCWireHit*>& allAxialWireHits,
                                    std::vector<CDCTrack>& tracks)
      {
        std::vector<const CDCWireHit*> candidateHits;
        for (AxialHitQuadTreeProcessor::Item* hit : qt->getItems()) {
          const CDCWireHit* wireHit = hit->getPointer();
          // Unset the taken flag and let the postprocessing decide
          (*wireHit)->setTakenFlag(false);
          candidateHits.push_back(wireHit);
        }

        ConformalExtension conformalExtension;
        conformalExtension.newRefPoint(candidateHits, allAxialWireHits, true);
        AxialTrackUtil::addCandidateFromHitsWithPostprocessing(candidateHits, allAxialWireHits, tracks);
      }
    };
  }
}
