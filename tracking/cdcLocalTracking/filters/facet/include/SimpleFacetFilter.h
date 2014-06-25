/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef SIMPLEFACETFILTER_H_
#define SIMPLEFACETFILTER_H_

#include <tracking/cdcLocalTracking/eventdata/entities/CDCRecoFacet.h>

namespace Belle2 {
  namespace CDCLocalTracking {
    /// Filter for the constuction of good facets based on simple criterions.
    class SimpleFacetFilter {

    public:
      /// Constructor using default direction of flight deviation cut off.
      SimpleFacetFilter();

      /// Constructor using given direction of flight deviation cut off.
      SimpleFacetFilter(FloatType allowedDeviationCos);

      /// Main filter method returning the weight of the facet. Returns NOT_A_CELL if the cell shall be rejected.
      CellState isGoodFacet(const CDCRecoFacet& facet) const;

      /// Clears all remember information from the last event
      void clear() const;

      /// Forwards the modules initialize to the filter.
      void initialize();

      /// Forwards the modules initialize to the filter.
      void terminate();

    private:
      const FloatType m_allowedDeviationCos; ///< Memory for the used direction of flight deviation.

    }; // end class SimpleFacetFilter
  } //end namespace CDCLocalTracking
} //end namespace Belle2

#endif //SIMPLEFACETFILTER_H_
