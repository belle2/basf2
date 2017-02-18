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

      ///Constructor
      QuadTreeNodeProcessor(AxialHitQuadTreeProcessor& qtProcessor,
                            PrecisionUtil::PrecisionFunction precisionFunct)
        : m_qtProcessor(qtProcessor)
        , m_precisionFunct(precisionFunct) {};

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
        for (AxialHitQuadTreeProcessor::ItemType* hit : qt->getItems()) {
          hit->setUsedFlag(false);
        }

        std::vector<const CDCWireHit*> candidateHits;

        for (AxialHitQuadTreeProcessor::ItemType* hit : qt->getItems()) {
          hit->setUsedFlag(false);
          candidateHits.push_back(hit->getPointer());
        }
        postprocessSingleNode(candidateHits, allAxialWireHits, tracks);
      };

      /// Perform conformal extension for given set of hits and create CDCTrack object of them
      void postprocessSingleNode(std::vector<const CDCWireHit*>& candidateHits,
                                 const std::vector<const CDCWireHit*>& allAxialWireHits,
                                 std::vector<CDCTrack>& tracks)
      {
        for (const CDCWireHit* hit : candidateHits) {
          (*hit)->setTakenFlag(false);
        }

        ConformalExtension conformalExtension;
        conformalExtension.newRefPoint(candidateHits, allAxialWireHits, true);
        AxialTrackUtil::addCandidateFromHitsWithPostprocessing(candidateHits, allAxialWireHits, tracks);
      }

    private:
      AxialHitQuadTreeProcessor& m_qtProcessor; /**< Reference to the quadtree processor. */

      PrecisionUtil::PrecisionFunction m_precisionFunct; /**< Quadtree precision function. */

      const unsigned long m_nbinsTheta = pow(2,
                                             TrackFindingCDC::PrecisionUtil::getLookupGridLevel()); /**< Number of theta bins.*/
    };

  }
}
