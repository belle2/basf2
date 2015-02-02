/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef BASEFACETFILTER_H_
#define BASEFACETFILTER_H_

#include <tracking/trackFindingCDC/eventdata/entities/CDCRecoFacet.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Base class for all facet filters
    class BaseFacetFilter {

    public:
      /// Constructor using default direction of flight deviation cut off.
      BaseFacetFilter() {;}

      /// Constructor using default direction of flight deviation cut off.
      virtual ~BaseFacetFilter() {;}

      /// Main filter method returning the weight of the facet. Returns NOT_A_CELL to reject all facets.
      virtual CellWeight isGoodFacet(const CDCRecoFacet&) { return NOT_A_CELL; }

      /// Clears all remember information from the last event.
      virtual void clear() {;}

      /// Forwards the modules initialize to the filter.
      virtual void initialize() {;}

      /// Forwards the modules initialize to the filter.
      virtual void terminate() {;}

    }; //end class BaseFacetFilter
  } //end namespace TrackFindingCDC
} //end namespace Belle2

#endif // BASEFACETFILTER_H_
