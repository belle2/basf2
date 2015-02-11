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

#include <tracking/trackFindingCDC/filters/segment_triple/MCSegmentTripleFilter.h>

#include <tracking/trackFindingCDC/rootification/IfNotCint.h>

#include "BaseSegmentTripleNeighborChooser.h"

namespace Belle2 {
  namespace TrackFindingCDC {
    ///Class filtering the neighborhood of segment triples with monte carlo information
    class MCSegmentTripleNeighborChooser: public BaseSegmentTripleNeighborChooser {

    public:
      /** Constructor. */
      MCSegmentTripleNeighborChooser(bool allowReverse = true);

      /** Destructor.*/
      virtual ~MCSegmentTripleNeighborChooser() {;}

      /// Clears stored information for a former event
      virtual void clear() IF_NOT_CINT(override final);

      /// Forwards the initialize method from the module
      virtual void initialize() IF_NOT_CINT(override final);

      /// Forwards the terminate method from the module
      virtual void terminate() IF_NOT_CINT(override final);

      /// Main filter method returning the weight of the neighborhood relation. Return NOT_A_NEIGHBOR if relation shall be rejected.
      virtual NeighborWeight isGoodNeighbor(const CDCSegmentTriple& triple,
                                            const CDCSegmentTriple& neighborTriple) override final {

        CellState mcTripleWeight = m_mcSegmentTripleFilter.isGoodSegmentTriple(triple);
        CellState mcNeighborTripleWeight = m_mcSegmentTripleFilter.isGoodSegmentTriple(neighborTriple);

        bool mcDecision = (not isNotACell(mcTripleWeight)) and (not isNotACell(mcNeighborTripleWeight));

        return mcDecision ? - neighborTriple.getStart()->size() : NOT_A_NEIGHBOR;

      }

    private:
      /// Instance of the Monte Carlo segment triple filter for rejection of false cells.
      MCSegmentTripleFilter m_mcSegmentTripleFilter;

    }; // end class


  } //end namespace TrackFindingCDC
} //end namespace Belle2

#endif //MCSEGMENTTRIPLENEIGHBORCHOOSER_H_
