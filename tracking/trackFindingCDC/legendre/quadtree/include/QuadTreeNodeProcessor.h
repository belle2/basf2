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
#include <tracking/trackFindingCDC/processing/TrackProcessor.h>
#include <tracking/trackFindingCDC/legendre/quadtree/AxialHitQuadTreeProcessor.h>
#include <tracking/trackFindingCDC/legendre/quadtree/ConformalExtension.h>

#include <list>
#include <vector>
#include <map>
#include <cmath>


namespace Belle2 {
  namespace TrackFindingCDC {
    class CDCConformalHit;

    /**
     * Class which used as interface between QuadTree processor and TrackCreator
     */
    class QuadTreeNodeProcessor {

    public:

      ///Constructor
      QuadTreeNodeProcessor(AxialHitQuadTreeProcessor& qtProcessor,
                            PrecisionUtil::PrecisionFunction precisionFunct) :
        m_qtProcessor(qtProcessor), m_precisionFunct(precisionFunct)
      {
      };


      /**
       *  this lambda function will forward the found candidates to the CandidateCreate for further processing
       *  hits belonging to found candidates will be marked as used and ignored for further filling iterations
       */
      AxialHitQuadTreeProcessor::CandidateProcessorLambda getLambdaInterface(std::vector<const CDCWireHit*>& allAxialWireHits,
          std::list<CDCTrack>& tracks)
      {
        AxialHitQuadTreeProcessor::CandidateProcessorLambda lmdCandidateProcessingFinal = [&](const AxialHitQuadTreeProcessor::ReturnList &
        hits __attribute__((unused)), AxialHitQuadTreeProcessor::QuadTree * qt) -> void {
          this->candidateProcessingFinal(qt, allAxialWireHits, tracks);
        };

        return lmdCandidateProcessingFinal;

      }


      /// Gets hits from quadtree node, convert to QuadTreeHitWrapper and passes for further processing
      void candidateProcessingFinal(AxialHitQuadTreeProcessor::QuadTree* qt,
                                    std::vector<const CDCWireHit*>& allAxialWireHits,
                                    std::list<CDCTrack>& tracks)
      {
        for (AxialHitQuadTreeProcessor::ItemType* hit : qt->getItemsVector()) {
          hit->setUsedFlag(false);
        }

        /// THIS METHOD IS DISABLED BECACUSE IT CREATES INVALID TREE ITEMS
        /// WHICH ARE IN THETA AREAS ( < 0 or > m_nbinsTheta )
        /// WHICH IS NOT SUPPORTED BY OTHER PARTS OF THE QUAD TREE CODE
        /// NO NEGATIVE IMPACT ON THE EFFICIENCY WHEN THIS METHOD IS DISABLED ....
        //advancedProcessing(qt);

        std::vector<CDCConformalHit*> candidateHits;

        for (AxialHitQuadTreeProcessor::ItemType* hit : qt->getItemsVector()) {
          hit->setUsedFlag(false);
          candidateHits.push_back(hit->getPointer());
        }
        postprocessSingleNode(candidateHits, allAxialWireHits, tracks);
      };


      /// Trying to assing new hits from neighbouring regions of the given node.
      /// THIS METHOD IS DISABLED BECACUSE IT CREATES INVALID TREE ITEMS
      /// WHICH ARE IN THETA AREAS ( < 0 or > m_nbinsTheta )
      /// WHICH IS NOT SUPPORTED BY OTHER PARTS OF THE QUAD TREE CODE
      /// NO NEGATIVE IMPACT ON THE EFFICIENCY WHEN THIS METHOD IS DISABLED ....
      void advancedProcessing(AxialHitQuadTreeProcessor::QuadTree* qt)
      {
        double rRes = m_precisionFunct(qt->getYMean());
        unsigned long thetaRes = std::round(std::fabs(m_nbinsTheta * rRes / 0.3));

        unsigned long meanTheta = qt->getXMean();
        double meanR = qt->getYMean();


        std::vector< AxialHitQuadTreeProcessor::QuadTree*> nodesWithPossibleCands;

        for (int ii = -1; ii <= 1; ii = +2) {
          for (int jj = -1; jj <= 1; jj = +2) {
            const double lowerRangeX = meanTheta + (ii - 1) * thetaRes / 2.0f;
            // make sure there are negative lower borders in x region, because only positive values can be stored
            // in unsigned long
            if (lowerRangeX < 0.0f)
              continue;
            const auto rangeX = AxialHitQuadTreeProcessor::rangeX(static_cast<unsigned long>(lowerRangeX),
                                                                  meanTheta + static_cast<unsigned long>((ii + 1) * thetaRes / 2.0f));
            const auto rangeY = AxialHitQuadTreeProcessor::rangeY(meanR + (jj - 1) * rRes / 2.0f,
                                                                  meanR + (jj + 1) * rRes / 2.0f);

            AxialHitQuadTreeProcessor::ChildRanges rangesTmp(rangeX, rangeY);
            nodesWithPossibleCands.push_back(m_qtProcessor.createSingleNode(rangesTmp));
          }
        }

        for (int ii = -1; ii <= 1; ii++) {
          for (int jj = -1; jj <= 1; jj++) {
            const double lowerRangeX = meanTheta + (2 * ii - 1) * thetaRes / 2.0f;
            // make sure there are negative lower borders in x region, because only positive values can be stored
            // in unsigned long
            if (lowerRangeX < 0.0f)
              continue;
            const auto rangeX = AxialHitQuadTreeProcessor::rangeX(static_cast<unsigned long>(lowerRangeX),
                                                                  meanTheta + static_cast<unsigned long>((2 * ii + 1) * thetaRes / 2.0f));
            const auto rangeY = AxialHitQuadTreeProcessor::rangeY(meanR + (2 * jj - 1) * rRes / 2.0f,
                                                                  meanR + (2 * jj + 1) * rRes / 2.0f);

            AxialHitQuadTreeProcessor::ChildRanges rangesTmp(rangeX, rangeY);
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
      void postprocessSingleNode(std::vector<CDCConformalHit*>& candidateHits,
                                 const std::vector<const CDCWireHit*>& allAxialWireHits,
                                 std::list<CDCTrack>& tracks)
      {

        for (CDCConformalHit* hit : candidateHits) {
          hit->setUsedFlag(false);
        }

        ConformalExtension conformalExtension;

        std::vector<const CDCWireHit*> cdcWireHits;

        for (CDCConformalHit* hit : candidateHits) {
          cdcWireHits.push_back(hit->getWireHit());
        }

        conformalExtension.newRefPoint(cdcWireHits, allAxialWireHits, true);

        TrackProcessor::addCandidateFromHitsWithPostprocessing(cdcWireHits, allAxialWireHits, tracks);
      }

    private:
      AxialHitQuadTreeProcessor& m_qtProcessor; /**< Reference to the quadtree processor. */

      PrecisionUtil::PrecisionFunction m_precisionFunct; /**< Quadtree precision function. */

      const unsigned long m_nbinsTheta = pow(2,
                                             TrackFindingCDC::PrecisionUtil::getLookupGridLevel()); /**< Number of theta bins.*/
    };

  }

}

