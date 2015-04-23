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

#ifndef ALLSEGMENTTRIPLENEIGHBORCHOOSER_H_
#define ALLSEGMENTTRIPLENEIGHBORCHOOSER_H_

#include "BaseSegmentTripleNeighborChooser.h"
#include <tracking/trackFindingCDC/rootification/IfNotCint.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Class excepting all segment triples.
    class AllSegmentTripleNeighborChooser : public Filter<Relation<CDCSegmentTriple>> {

      /** Main filter method returning the weight of the neighborhood relation.
       *  Always return the overlap penatlty accepting all relations.*/
      virtual NeighborWeight operator()(const CDCSegmentTriple& from,
                                        const CDCSegmentTriple& to) override final;

    }; // end class

  } //end namespace TrackFindingCDC
} //end namespace Belle2

#endif //ALLSEGMENTTRIPLENEIGHBORCHOOSER_H_
