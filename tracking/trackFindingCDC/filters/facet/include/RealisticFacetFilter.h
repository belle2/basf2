/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef REALISTICFACETFILTER_H_
#define REALISTICFACETFILTER_H_

#include <tracking/trackFindingCDC/filters/facet/BaseFacetFilter.h>

#include <tracking/trackFindingCDC/eventdata/entities/CDCRecoFacet.h>

#include <tracking/trackFindingCDC/rootification/IfNotCint.h>

#include "FitlessFacetFilter.h"

namespace Belle2 {
  namespace TrackFindingCDC {
    /// Filter for the constuction of good facets based on simple criterions.
    class RealisticFacetFilter : public BaseFacetFilter {

    public:
      /// Constructor using default direction of flight deviation cut off.
      RealisticFacetFilter();

      /// Constructor using given direction of flight deviation cut off.
      RealisticFacetFilter(FloatType phiPullCut);

      /// Main filter method returning the weight of the facet. Returns NOT_A_CELL if the cell shall be rejected.
      virtual CellWeight isGoodFacet(const CDCRecoFacet& facet) IF_NOT_CINT(override final);

    private:
      /// Basic filter to implement a fitless preselection.
      FitlessFacetFilter m_fitlessFacetFilter;

      /// Memory for the pull cu
      const FloatType m_phiPullCut;

    }; // end class RealisticFacetFilter
  } //end namespace TrackFindingCDC
} //end namespace Belle2

#endif //REALISTICFACETFILTER_H_
