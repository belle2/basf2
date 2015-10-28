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
#include <tracking/trackFindingCDC/legendre/quadtreetools/QuadTreePassCounter.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    /**
     * Class which controls parameters of quadtree finding basing on finding pass
     */
    class QuadTreeParameters {

    public:

      /// Constructor
      QuadTreeParameters(int maxLevel, LegendreFindingPass& legendreFindingPass):
        m_maxLevel(maxLevel), m_legendreFindingPass(legendreFindingPass),
        m_rangesFine(std::make_pair(AxialHitQuadTreeProcessor::rangeX(0, std::pow(2, 16)), AxialHitQuadTreeProcessor::rangeY(0., 0.15))),
        m_rangesRough(std::make_pair(AxialHitQuadTreeProcessor::rangeX(0, std::pow(2, 16)), AxialHitQuadTreeProcessor::rangeY(0., 0.30))),
        m_originPrecisionFunction(),
        m_nonOriginPrecisionFunction()
      {};

      /// Get precision function for quadtree
      BasePrecisionFunction::PrecisionFunction& getPrecisionFunction()
      {
        if (m_legendreFindingPass == LegendreFindingPass::NonCurlers)
          return m_originPrecisionFunction.getFunction();
        else
          return m_nonOriginPrecisionFunction.getFunction();
      };

      /// Get threshold on number of hits
      int getInitialHitsLimit()
      {
        if ((m_legendreFindingPass == LegendreFindingPass::NonCurlers)
            || (m_legendreFindingPass == LegendreFindingPass::NonCurlersWithIncreasingThreshold))
          return 50;
        else
          return 30;
      }

      /// Get threshold on curvature
      double getCurvThreshold()
      {
        if ((m_legendreFindingPass == LegendreFindingPass::NonCurlers)
            || (m_legendreFindingPass == LegendreFindingPass::NonCurlersWithIncreasingThreshold))
          return 0.07;
        else
          return 0.15;
      }

      /// Get ranges for quadtree processor
      AxialHitQuadTreeProcessor::ChildRanges getQTRanges()
      {
        if ((m_legendreFindingPass == LegendreFindingPass::NonCurlers)
            || (m_legendreFindingPass == LegendreFindingPass::NonCurlersWithIncreasingThreshold))
          return m_rangesFine;
        else
          return m_rangesRough;
      }

      /// Get current pass
      LegendreFindingPass& getPass() {return m_legendreFindingPass;};

      /// Get maxlevel of quadtree
      int getMaxLevel()
      {
        if ((m_legendreFindingPass == LegendreFindingPass::NonCurlers))
          return m_maxLevel;
        else
          return m_maxLevel - 2;
      }

      /// Create quadtree processor object
      AxialHitQuadTreeProcessor constructQTProcessor()
      {
        return AxialHitQuadTreeProcessor(
                 getMaxLevel(),
                 getQTRanges(),
                 getPrecisionFunction());
      }

    private:
      int m_maxLevel;
      LegendreFindingPass& m_legendreFindingPass; /**< Reference to the current pass */


      AxialHitQuadTreeProcessor::ChildRanges m_rangesFine; /**< Ranges which correspond to more fine binning*/
      AxialHitQuadTreeProcessor::ChildRanges m_rangesRough; /**< Ranges which correspond to more rough binning*/

      OriginPrecisionFunction m_originPrecisionFunction; /**< More finer precision function (deeper quadtree). */
      NonOriginPrecisionFunction m_nonOriginPrecisionFunction; /**< More rough precision function. */

    };

  }

}

