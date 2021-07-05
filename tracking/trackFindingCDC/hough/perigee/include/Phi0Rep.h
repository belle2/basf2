/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/hough/axes/StandardAxes.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Strategy to construct discrete phi0 points from discrete overlap specifications.
    class Phi0BinsSpec {
    public:
      /**
       *  Constructor from fixed number of accessable bins and overlap specification
       *  in discrete number of positions
       */
      Phi0BinsSpec(long nBins, int nOverlap, int nWidth);

      /// Constuct the array of discrete phi0 positions
      DiscretePhi0::Array constructArray() const;

      /// Getter for the number of bounds
      long getNPositions() const;

      /**
       *  Getter for the bin width in real phi0 to investigate the value
       *  that results from the discrete overlap specification
       */
      double getBinWidth() const;

      /**
       *  Getter for the overlap in real phi0 to investigate the value
       *  that results from the discrete overlap specification
       */
      double getOverlap() const;

      /// Getter for the overlap in discrete number of positions
      int getNOverlap() const
      {
        return m_nOverlap;
      }

    private:
      ///  Number of accessable bins
      long m_nBins;

      /// Overlap of the leaves in phi0 counted in number of discrete values.
      int m_nOverlap = 1;

      /// Width of the leaves at the maximal level in phi0 counted in number of discrete values.
      int m_nWidth = 3;
    };
  }
}
