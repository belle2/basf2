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

#include <tracking/trackFindingCDC/filters/facet/FitlessFacetFilter.h>
#include <tracking/trackFindingCDC/filters/facet/BaseFacetFilter.h>

#include <tracking/trackFindingCDC/eventdata/hits/CDCFacet.h>

namespace Belle2 {
  namespace TrackFindingCDC {
    /// Filter for the constuction of good facets based on simple criterions.
    class RealisticFacetFilter : public Filter<CDCFacet> {

    public:
      /// Constructor using default direction of flight deviation cut off.
      RealisticFacetFilter();

      /// Constructor using given direction of flight deviation cut off.
      explicit RealisticFacetFilter(double phiPullCut);

    public:
      /** Set the parameter with key to value.
       *
       *  Parameters are:
       *  hard_fitless_cut  - Switch to disallow the boarderline possible hit and
       *                      right left passage information.
       *                      Allowed values "true", "false". Default is "true".
       *  phi_pull_cut      - Acceptable deviation cosine in the angle of adjacent tangents to the
       *                      drift circles.
       */
      virtual
      void setParameter(const std::string& key, const std::string& value) override;

      /** Returns a map of keys to descriptions describing the individual parameters of the filter.
       */
      virtual
      std::map<std::string, std::string> getParameterDescription() override;

      /** Main filter method returning the weight of the facet
       *  Returns NAN if the cell shall be rejected.
       */
      virtual
      Weight operator()(const CDCFacet& facet) override final;

    private:
      /// Basic filter to implement a fitless preselection.
      FitlessFacetFilter m_fitlessFacetFilter;

      /// Memory for the pull cu
      double m_param_phiPullCut;

    }; // end class RealisticFacetFilter
  } //end namespace TrackFindingCDC
} //end namespace Belle2
