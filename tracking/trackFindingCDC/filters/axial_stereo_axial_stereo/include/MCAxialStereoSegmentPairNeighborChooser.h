/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef MCAXIALSTEREOSEGMENTPAIRNEIGHBORCHOOSER_H
#define MCAXIALSTEREOSEGMENTPAIRNEIGHBORCHOOSER_H

#include <tracking/trackFindingCDC/filters/axial_stereo/MCAxialStereoSegmentPairFilter.h>

#include "BaseAxialStereoSegmentPairNeighborChooser.h"

namespace Belle2 {
  namespace TrackFindingCDC {

    ///Class filtering the neighborhood of axial stereo segment pairs with monte carlo information
    class MCAxialStereoSegmentPairNeighborChooser : public BaseAxialStereoSegmentPairNeighborChooser {

    public:
      /** Constructor. */
      MCAxialStereoSegmentPairNeighborChooser(bool allowReverse = true) : m_mcAxialStereoSegmentPairFilter(allowReverse)
      {;}

      /// Main filter method returning the weight of the neighborhood relation. Return NOT_A_NEIGHBOR if relation shall be rejected.
      virtual NeighborWeight isGoodNeighbor(const CDCAxialStereoSegmentPair& axialStereoSegmentPair,
                                            const CDCAxialStereoSegmentPair& neighborAxialStereoSegmentPair) override final {

        CellWeight mcPairWeight = m_mcAxialStereoSegmentPairFilter.isGoodAxialStereoSegmentPair(axialStereoSegmentPair);
        CellWeight mcNeighborPairWeight = m_mcAxialStereoSegmentPairFilter.isGoodAxialStereoSegmentPair(neighborAxialStereoSegmentPair);

        bool mcDecision = (not isNotACell(mcPairWeight)) and (not isNotACell(mcNeighborPairWeight));

        return mcDecision ? -neighborAxialStereoSegmentPair.getStartSegment()->size() : NOT_A_NEIGHBOR;

      }

    private:
      /// Instance of the Monte Carlo axial stereo segment filter for rejection of false cells.
      MCAxialStereoSegmentPairFilter m_mcAxialStereoSegmentPairFilter;

    }; // end class

  } //end namespace TrackFindingCDC
} //end namespace Belle2

#endif // MCAXIALSTEREOSEGMENTPAIRNEIGHBORCHOOSER_H
