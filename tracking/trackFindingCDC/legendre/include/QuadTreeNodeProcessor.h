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
#include <tracking/trackFindingCDC/creators/QuadTreeHitWrapperCreator.h>
#include <tracking/trackFindingCDC/legendre/TrackProcessor.h>
#include <tracking/trackFindingCDC/eventdata/hits/ConformalCDCWireHit.h>
#include <tracking/trackFindingCDC/legendre/quadtree/AxialHitQuadTreeProcessor.h>
#include <tracking/trackFindingCDC/legendre/quadtree/AxialHitQuadTreeProcessorWithNewReferencePoint.h>
#include <tracking/trackFindingCDC/legendre/quadtree/precision_functions/OriginPrecisionFunction.h>
#include <tracking/trackFindingCDC/legendre/quadtree/precision_functions/NonOriginPrecisionFunction.h>

#include <list>
#include <vector>
#include <map>


namespace Belle2 {
  namespace TrackFindingCDC {

    /**
     * Class which used as interface between QuadTree processor and TrackCreator
     */
    class QuadTreeNodeProcessor {

    public:

      ///Constructor
      QuadTreeNodeProcessor(TrackProcessor& trackProcessor, AxialHitQuadTreeProcessor& qtProcessor,
                            BasePrecisionFunction::PrecisionFunction& precisionFunct) :
        m_trackProcessor(trackProcessor), m_qtProcessor(qtProcessor), m_precisionFunct(precisionFunct)
      {
      };


      /**
       *  this lambda function will forward the found candidates to the CandidateCreate for further processing
       *  hits belonging to found candidates will be marked as used and ignored for further filling iterations
       */
      AxialHitQuadTreeProcessor::CandidateProcessorLambda getLambdaInterface()
      {
        AxialHitQuadTreeProcessor::CandidateProcessorLambda lmdCandidateProcessingFinal = [&](const AxialHitQuadTreeProcessor::ReturnList &
        hits __attribute__((unused)), AxialHitQuadTreeProcessor::QuadTree * qt) -> void {
          this->candidateProcessingFinal(qt);
        };

        return lmdCandidateProcessingFinal;

      }


      /// Gets hits from quadtree node, convert to QuadTreeHitWrapper and passes for further processing
      void candidateProcessingFinal(AxialHitQuadTreeProcessor::QuadTree* qt)
      {
        for (AxialHitQuadTreeProcessor::ItemType* hit : qt->getItemsVector()) {
          hit->setUsedFlag(false);
        }
        advancedProcessing(qt);

        std::vector<ConformalCDCWireHit*> candidateHits;

        for (AxialHitQuadTreeProcessor::ItemType* hit : qt->getItemsVector()) {
          hit->setUsedFlag(false);
          candidateHits.push_back(hit->getPointer());
        }
        postprocessSingleNode(candidateHits);
      };

      /// Trying to assing new hits from neighbouring regions of the given node.
      void advancedProcessing(AxialHitQuadTreeProcessor::QuadTree* qt)
      {
        double rRes = m_precisionFunct(qt->getYMean());
        unsigned long thetaRes = abs(m_nbinsTheta * rRes / 0.3);

        unsigned long meanTheta = qt->getXMean();
        double meanR = qt->getYMean();


        std::vector< AxialHitQuadTreeProcessor::QuadTree*> nodesWithPossibleCands;

        for (int ii = -1; ii <= 1; ii = +2) {
          for (int jj = -1; jj <= 1; jj = +2) {
            AxialHitQuadTreeProcessor::ChildRanges rangesTmp(AxialHitQuadTreeProcessor::rangeX(meanTheta + static_cast<unsigned long>((
                                                               ii - 1)*thetaRes / 2),
                                                             meanTheta + static_cast<unsigned long>((ii + 1)*thetaRes / 2)),
                                                             AxialHitQuadTreeProcessor::rangeY(meanR + (jj - 1)*rRes / 2.,
                                                                 meanR + (jj + 1)*rRes / 2.));

            nodesWithPossibleCands.push_back(m_qtProcessor.createSingleNode(rangesTmp));
          }
        }

        for (int ii = -1; ii <= 1; ii++) {
          for (int jj = -1; jj <= 1; jj++) {
            AxialHitQuadTreeProcessor::ChildRanges rangesTmp(AxialHitQuadTreeProcessor::rangeX(meanTheta + static_cast<unsigned long>((
                                                               2 * ii - 1)*thetaRes / 2),
                                                             meanTheta + static_cast<unsigned long>((2 * ii + 1)*thetaRes / 2)),
                                                             AxialHitQuadTreeProcessor::rangeY(meanR + (2 * jj - 1)*rRes / 2.,
                                                                 meanR + (2 * jj + 1)*rRes / 2.));

            nodesWithPossibleCands.push_back(m_qtProcessor.createSingleNode(rangesTmp));
          }
        }

        std::sort(nodesWithPossibleCands.begin(), nodesWithPossibleCands.end(), [](const AxialHitQuadTreeProcessor::QuadTree * a,
        const AxialHitQuadTreeProcessor::QuadTree * b) {
          return static_cast <bool>(a->getNItems() > b->getNItems());
        });

        qt->clearNode();
        for (AxialHitQuadTreeProcessor::ItemType* hit : nodesWithPossibleCands.front()->getItemsVector()) {
          qt->insertItem(hit);
        }

        for (AxialHitQuadTreeProcessor::QuadTree* node : nodesWithPossibleCands) {
          delete node;
        }
      };


      /// Perform conformal extension for given set of hits and create CDCTrack object of them
      void postprocessSingleNode(std::vector<ConformalCDCWireHit*>& candidateHits)
      {

        for (ConformalCDCWireHit* hit : candidateHits) {
          hit->setUsedFlag(false);
        }

        ConformalExtension conformalExtension(m_trackProcessor.getHitFactory());

        std::vector<const CDCWireHit*> cdcWireHits;

        for (ConformalCDCWireHit* hit : candidateHits) {
          cdcWireHits.push_back(hit->getCDCWireHit());
        }

        conformalExtension.newRefPoint(cdcWireHits, true);

        m_trackProcessor.addCandidateWithHits(cdcWireHits);
      }

    private:
      TrackProcessor& m_trackProcessor; /**< Reference for creating tracks */
      AxialHitQuadTreeProcessor& m_qtProcessor; /**< Reference to the quadtree processor */

      BasePrecisionFunction::PrecisionFunction m_precisionFunct; /**< Quadtree precision function*/

      const unsigned long m_nbinsTheta = pow(2, 16);
    };

  }

}

