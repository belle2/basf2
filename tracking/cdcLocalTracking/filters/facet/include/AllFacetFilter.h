/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef ALLFACETFILTER_H_
#define ALLFACETFILTER_H_

#include <tracking/cdcLocalTracking/eventdata/entities/CDCRecoFacet.h>
#include <tracking/cdcLocalTracking/mockroot/MockRoot.h>

namespace Belle2 {
  namespace CDCLocalTracking {

    /// Filter for the constuction of good facets based on simple criterions.
    class AllFacetFilter : public UsedTObject {



    public:
      /// Constructor using default direction of flight deviation cut off.
      AllFacetFilter();

      /// Main filter method returning the weight of the facet. Returns 3 to accept all facets.
      CellState isGoodFacet(const CDCRecoFacet& facet) const;

      /// Clears all remember information from the last event.
      void clear() const;

      /// Forwards the modules initialize to the filter.
      void initialize();

      /// Forwards the modules initialize to the filter.
      void terminate();


    private:
      /// ROOT Macro to make AllFacetFilter a ROOT class.
      ClassDefInCDCLocalTracking(AllFacetFilter, 1);


    }; //end class AllFacetFilter
  } //end namespace CDCLocalTracking
} //end namespace Belle2

#endif // ALLFACETFILTER_H_
