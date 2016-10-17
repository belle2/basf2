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
#include <tracking/trackFindingCDC/legendre/quadtree/QuadTreeNodeProcessor.h>
#include <tracking/trackFindingCDC/legendre/quadtree/AxialHitQuadTreeProcessor.h>
#include <tracking/trackFindingCDC/legendre/quadtree/AxialHitQuadTreeProcessorWithNewReferencePoint.h>
#include <tracking/trackFindingCDC/legendre/precisionFunctions/OriginPrecisionFunction.h>
#include <tracking/trackFindingCDC/legendre/precisionFunctions/NonOriginPrecisionFunction.h>
#include <tracking/trackFindingCDC/legendre/quadtreetools/QuadTreeParameters.h>

#include <list>
#include <vector>
#include <map>


namespace Belle2 {
  namespace TrackFindingCDC {

    /**
     * Class which operates with AxialHitQuadTreeProcessor instance and performs tracks finding with different conditions
     */
    class QuadTreeCandidateFinder {

    public:

      /// Default constructor.
      QuadTreeCandidateFinder() {};

      ~QuadTreeCandidateFinder() = default;

      /**
       * Performs quadtree search
       * @param lmdInterface lambda interface to operate with QuadTree which contains possible track
       * @param parameters pass-dependent parameters of the QuadTree search
       * @param qtProcessor reference to the AxialHitQuadTreeProcessor instance
       */
      void doTreeTrackFinding(AxialHitQuadTreeProcessor::CandidateProcessorLambda& lmdInterface, QuadTreeParameters& parameters,
                              AxialHitQuadTreeProcessor& qtProcessor)
      {

        //radius of the CDC
        double rCDC = 113.;

        if (parameters.getPass() != LegendreFindingPass::FullRange) qtProcessor.seedQuadTree(4);

        //find high-pt tracks (not-curlers: diameter of the track higher than radius of CDC -- 2*Rtrk > rCDC => Rtrk < 2./rCDC, r(legendre) = 1/Rtrk =>  r(legendre) < 2./rCDC)
        if (parameters.getPass() != LegendreFindingPass::FullRange) qtProcessor.fillSeededTree(lmdInterface, 50,
              2. / rCDC); //fillSeededTree
        else qtProcessor.fillGivenTree(lmdInterface, 50, 2. / rCDC);
        //qtProcessor.fillGivenTree(lmdInterface, 50, 2. / rCDC);

        //find curlers with diameter higher than half of radius of CDC (see calculations above)
        if (parameters.getPass() != LegendreFindingPass::FullRange) qtProcessor.fillSeededTree(lmdInterface, 70, 4. / rCDC); //fillGivenTree
        else qtProcessor.fillGivenTree(lmdInterface, 70, 4. / rCDC);
        //qtProcessor.fillGivenTree(lmdInterface, 70, 4. / rCDC);

        // Start loop, where tracks are searched for
        int limit = parameters.getInitialHitsLimit();
        double rThreshold = parameters.getCurvThreshold();
        do {
          if (parameters.getPass() != LegendreFindingPass::FullRange) qtProcessor.fillSeededTree(lmdInterface, limit,
                rThreshold); //fillSeededTree
          else qtProcessor.fillGivenTree(lmdInterface, limit, rThreshold);
          //qtProcessor.fillGivenTree(lmdInterface, limit, rThreshold);

          limit = limit * m_param_stepScale;

          if (parameters.getPass() != LegendreFindingPass::NonCurlers) {
            rThreshold *= 2.;
            if (rThreshold > 0.15/*ranges.second.second*/) rThreshold = 0.15;// ranges.second.second;
          }

          //perform search until found track has too few hits or threshold is too small and no tracks are found
        } while (limit >= m_param_threshold);

        //qtProcessor.clearSeededTree();

      }

    private:
      const double m_param_stepScale = 0.75; /**< Parameter to define multiplier for hits threshold for the next quadtree iteration*/
      const int m_param_threshold = 10; /**< Parameter to define minimal threshold of hit*/
    };

  }

}

