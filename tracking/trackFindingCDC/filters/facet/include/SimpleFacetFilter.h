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

#include <tracking/trackFindingCDC/filters/facet/BaseFacetFilter.h>

#include <tracking/trackFindingCDC/eventdata/hits/CDCFacet.h>


#include <tracking/trackFindingCDC/filters/facet/FitlessFacetFilter.h>

namespace Belle2 {
  namespace TrackFindingCDC {
    /// Filter for the constuction of good facets based on simple criterions.
    class SimpleFacetFilter : public Filter<CDCFacet> {

    private:
      /// Type of the super class
      typedef Filter<CDCFacet> Super;

    public:
      /// Constructor using default direction of flight deviation cut off.
      SimpleFacetFilter();

      /// Constructor using given direction of flight deviation cut off.
      explicit SimpleFacetFilter(double deviationCosCut);

    public:
      /** Set the parameter with key to value.
       *
       *  Parameters are:
       *  hard_fitless_cut  - Switch to disallow the boarderline possible hit and
       *                      right left passage information.
       *                      Allowed values "true", "false". Default is "true".
       *  deviation_cos_cut - Acceptable deviation cosine in the angle of adjacent tangents to the
       *                      drift circles.
       */
      virtual
      void setParameter(const std::string& key, const std::string& value) override;

      /** Returns a map of keys to descriptions describing the individual parameters of the filter.
       */
      virtual
      std::map<std::string, std::string> getParameterDescription() override;

      /** Main filter method returning the weight of the facet.
       *  Returns NAN if the cell shall be rejected.
       */
      virtual Weight operator()(const CDCFacet& facet) override final;

    private:
      /// Basic filter to implement a fitless preselection.
      FitlessFacetFilter m_fitlessFacetFilter;

      /// Memory for the used direction of flight deviation.
      double m_param_deviationCosCut;

    }; // end class SimpleFacetFilter
  } //end namespace TrackFindingCDC
} //end namespace Belle2
