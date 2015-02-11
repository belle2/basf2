/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef ALLSEGMENTTRIPLENEIGHBORCHOOSER_H_
#define ALLSEGMENTTRIPLENEIGHBORCHOOSER_H_

#include "BaseSegmentTripleNeighborChooser.h"

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Class excepting all segment triples.
    class AllSegmentTripleNeighborChooser : public BaseSegmentTripleNeighborChooser {

    public:
      /// Default constructor
      AllSegmentTripleNeighborChooser() {;}

      /// Empty destructor
      virtual ~AllSegmentTripleNeighborChooser() {;}

      /// Clears stored information for a former event
      virtual void clear() override final {;}

      /// Forwards the initialize method from the module
      virtual void initialize() override final {;}

      /// Forwards the terminate method from the module
      virtual void terminate() override final {;}

      /// Main filter method returning the weight of the neighborhood relation. Return NOT_A_NEIGHBOR if relation shall be rejected.
      virtual NeighborWeight isGoodNeighbor(const CDCSegmentTriple&,
                                            const CDCSegmentTriple& neighborTriple) override final {
        // Just let all found neighors pass for the base implementation
        // with the default weight
        return  -neighborTriple.getStart()->size();
      }

    }; // end class

  } //end namespace TrackFindingCDC
} //end namespace Belle2

#endif //ALLSEGMENTTRIPLENEIGHBORCHOOSER_H_
