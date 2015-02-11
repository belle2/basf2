/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef MCAXIALSTEREOSEGMENTPAIRFILTER_H
#define MCAXIALSTEREOSEGMENTPAIRFILTER_H

#include <tracking/trackFindingCDC/eventdata/tracks/CDCAxialStereoSegmentPair.h>

#include "BaseAxialStereoSegmentPairFilter.h"
#include <tracking/trackFindingCDC/rootification/IfNotCint.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Filter for the constuction of axial to stereo segment pairs based on Monte Carlo information.
    class MCAxialStereoSegmentPairFilter : public BaseAxialStereoSegmentPairFilter {

    public:
      /// Constructor
      MCAxialStereoSegmentPairFilter(bool allowReverse = true) : m_allowReverse(allowReverse) {;}

      /// Checks if a axial stereo segment pair is a good combination.
      virtual CellWeight isGoodAxialStereoSegmentPair(const Belle2::TrackFindingCDC::CDCAxialStereoSegmentPair& axialAxialSegmentPair) IF_NOT_CINT(override final);

    private:
      /// Switch to indicate if the reversed version of the segment pair shall also be accepted (default is true).
      bool m_allowReverse;

    }; // end class MCAxialStereoSegmentPairFilter

  } //end namespace TrackFindingCDC
} //end namespace Belle2

#endif // MCAXIALSTEREOSEGMENTPAIRFILTER_H
