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

#include <tracking/trackFindingCDC/legendre/quadtree/AxialHitQuadTreeProcessor.h>

#include <tracking/trackFindingCDC/legendre/quadtreetools/QuadTreePassCounter.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    /**
     * Class which controls parameters of quadtree finding basing on finding pass
     */
    class QuadTreeParameters {

    public:
      /// Constructor
      QuadTreeParameters(int maxLevel, LegendreFindingPass& legendreFindingPass)
        : m_maxLevel(maxLevel)
        , m_legendreFindingPass(legendreFindingPass)
        , m_maxTheta(std::pow(2, PrecisionUtil::getLookupGridLevel()))
        , m_rangesFine( { {0, m_maxTheta}, { -0.02, 0.14}})
      , m_rangesRough({{0, m_maxTheta}, {0.00, 0.30}})
      {
      }

      /// Get precision function for quadtree
      PrecisionUtil::PrecisionFunction getPrecisionFunction()
      {
        if (m_legendreFindingPass == LegendreFindingPass::NonCurlers) {
          return &PrecisionUtil::getOriginCurvPrecision;
        } else {
          return &PrecisionUtil::getNonOriginCurvPrecision;
        }
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
      AxialHitQuadTreeProcessor::XYSpans getQTRanges()
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
        if (m_legendreFindingPass == LegendreFindingPass::NonCurlers)
          return m_maxLevel;
        else
          return m_maxLevel - 2;
      }

      /// Get the seed level of the quadtree
      int getSeedLevel()
      {
        if (getPass() != LegendreFindingPass::FullRange) {
          return 4;
        } else {
          return 1;
        }
      }

      /// Create quadtree processor object
      std::unique_ptr<AxialHitQuadTreeProcessor> constructQTProcessor()
      {
        return makeUnique<AxialHitQuadTreeProcessor>(getMaxLevel(),
                                                     getSeedLevel(),
                                                     getQTRanges(),
                                                     getPrecisionFunction());
      }

    private:
      int m_maxLevel; /**< Maximal deepness (level) of the QuadTree */
      LegendreFindingPass& m_legendreFindingPass; /**< Reference to the current pass */

      /// Maximal index in the theta range.
      int m_maxTheta;

      AxialHitQuadTreeProcessor::XYSpans m_rangesFine; /**< Ranges which correspond to more fine binning*/
      AxialHitQuadTreeProcessor::XYSpans m_rangesRough; /**< Ranges which correspond to more rough binning*/
    };

  }

}
