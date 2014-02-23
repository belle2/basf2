/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef MCSEGMENTTRIPLENEIGHBORCHOOSER_H_
#define MCSEGMENTTRIPLENEIGHBORCHOOSER_H_

#include <tracking/cdcLocalTracking/typedefs/BasicTypes.h>
#include <tracking/cdcLocalTracking/typedefs/BasicConstants.h>
#include <tracking/cdcLocalTracking/typedefs/UsedDataHolders.h>

#include <tracking/cdcLocalTracking/filters/segment_triple/MCSegmentTripleFilter.h>

#include "BaseSegmentTripleNeighborChooser.h"

namespace Belle2 {
  namespace CDCLocalTracking {
    ///Class filtering the neighborhood of segment triples with monte carlo information
    class MCSegmentTripleNeighborChooser: public BaseSegmentTripleNeighborChooser {

    public:
      /** Constructor. */
      MCSegmentTripleNeighborChooser()
      {;}

      /** Destructor.*/
      ~MCSegmentTripleNeighborChooser() {;}


      inline Weight isGoodNeighbor(
        const CDCSegmentTriple& triple,
        const CDCSegmentTriple& neighborTriple
      ) const {

        CellState mcTripleWeight = m_mcSegmentTripleFilter.isGoodSegmentTriple(triple);
        CellState mcNeighborTripleWeight = m_mcSegmentTripleFilter.isGoodSegmentTriple(neighborTriple);

        bool mcDecision = (not isNotACell(mcTripleWeight)) and (not isNotACell(mcNeighborTripleWeight));

        return mcDecision ? - neighborTriple.getStart()->size() : NOT_A_NEIGHBOR;

      }

    private:
      MCSegmentTripleFilter m_mcSegmentTripleFilter;

    }; // end class


  } //end namespace CDCLocalTracking
} //end namespace Belle2

#endif //MCSEGMENTTRIPLENEIGHBORCHOOSER_H_
