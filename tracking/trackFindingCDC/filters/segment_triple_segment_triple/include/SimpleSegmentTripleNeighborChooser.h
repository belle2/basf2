/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#ifndef SIMPLESEGMENTTRIPLENEIGHBORCHOOSER_H_
#define SIMPLESEGMENTTRIPLENEIGHBORCHOOSER_H_

#include "BaseSegmentTripleNeighborChooser.h"
#include <tracking/trackFindingCDC/rootification/IfNotCint.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Class filtering the neighborhood of segment triples based on simple criterions.
    class SimpleSegmentTripleNeighborChooser : public Filter<Relation<CDCSegmentTriple>> {

    public:
      /// Implementation currently accepts all combinations
      virtual NeighborWeight operator()(const CDCSegmentTriple&,
                                        const CDCSegmentTriple& neighborTriple) IF_NOT_CINT(override final);

    }; // end class

  } //end namespace TrackFindingCDC
} //end namespace Belle2

#endif //SIMPLESEGMENTTRIPLENEIGHBORCHOOSER_H_
