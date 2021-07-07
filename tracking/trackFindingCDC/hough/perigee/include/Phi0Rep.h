/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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
