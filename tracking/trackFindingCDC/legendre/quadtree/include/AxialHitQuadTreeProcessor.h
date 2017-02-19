/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2014 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Viktor Trusov, Thomas Hauth, Nils Braun                  *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/legendre/quadtree/QuadTreeProcessor.h>
#include <tracking/trackFindingCDC/legendre/precisionFunctions/PrecisionUtil.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    /** A QuadTreeProcessor for TrackHits */
    class AxialHitQuadTreeProcessor : public QuadTreeProcessor<long, float, const CDCWireHit> {

    public:
      /// Constructor
      AxialHitQuadTreeProcessor(unsigned char lastLevel,
                                const XYSpans& ranges,
                                PrecisionUtil::PrecisionFunction lmdFunctLevel,
                                bool standartBinning = false);

    protected: // Section of specialized functions
      /**
       * lastLevel depends on curvature of the track candidate
       */
      bool isLeaf(QuadTree* node) const final;

      /**
       * Return the new ranges. We do not use the standard ranges for the lower levels.
       * @param node quadtree node
       * @param i theta index of the child
       * @param j rho index of the child
       * @return returns ranges of the (i;j) child
       */
      XYSpans createChild(QuadTree* node, unsigned int i, unsigned int j) const final;

      /**
       * Check whether hit belongs to the quadtree node:
       * @param node quadtree node
       * @param hit hit being checked
       * @return returns true if sinogram of the hit crosses (geometrically) borders of the node
       */
      bool isInNode(QuadTree* node, const CDCWireHit* wireHit) const final;

    protected: // Implementation details
      /**
       * Check derivative of the sinogram.
       * @param node QuadTree node
       * @param hit pointer to the hit to check
       * @return returns true in cases:
       * @return    - positive derivative and no extremum in the node's ranges or
       * @return    - extremum located in the node's ranges
       * @return returns false in other cases (namely negative derivative
       *
       */
      bool checkDerivative(QuadTree* node, const CDCWireHit* wireHit) const;

      /**
       * Checks whether extremum point is located whithin QuadTree node's ranges
       * @param node QuadTree node
       * @param hit hit to check
       * @return true or false
       */
      bool checkExtremum(QuadTree* node, const CDCWireHit* wireHit) const;

    public: // debug stuff
      /// Draw QuadTree nodes
      void drawNode();

    private:
      /**
       *  Sets whether standard splitting of bins will be used
       *   - in case of standard binning each bin will be splitted into 4 equal bins
       *   - in case of non-standard binning boundaries of each child will be extended
       *     (see AxialHitQuadTreeProcessor::createChildWithParent())
       */
      bool m_standartBinning;

      /**
       *  Indicator whether the two sided phases space insertion check should be used
       *  This option should automatically split back to back tracks in the low curvature regions
       */
      bool m_twoSidedPhaseSpace;

      /// Lambda which holds resolution function for the quadtree
      PrecisionUtil::PrecisionFunction m_lmdFunctLevel;
    };
  }
}
