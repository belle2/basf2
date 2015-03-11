/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef FITLESSFACETFILTER_H_
#define FITLESSFACETFILTER_H_

#include <tracking/trackFindingCDC/filters/facet/BaseFacetFilter.h>

#include <tracking/trackFindingCDC/eventdata/entities/CDCRecoFacet.h>

#include <tracking/trackFindingCDC/rootification/IfNotCint.h>

namespace Belle2 {
  namespace TrackFindingCDC {
    /** Filter for the constuction of good facets investigating the feasability
     *  of the right left passage hypotheses combination.
     *  If the given combination cannot be made by a track reject it.
     */
    class FitlessFacetFilter : public BaseFacetFilter {

    public:
      /// Constructor taking a flag if board line feasable cases should be excluded.
      FitlessFacetFilter(bool hard = false);


      /// Main filter method returning the weight of the facet. Returns NOT_A_CELL if the cell shall be rejected.
      virtual CellWeight isGoodFacet(const CDCRecoFacet& facet) IF_NOT_CINT(override final);

    private:
      /// Switch for hard selection.
      const bool m_hard;

    }; // end class FitlessFacetFilter
  } //end namespace TrackFindingCDC
} //end namespace Belle2

#endif //FITLESSFACETFILTER_H_
