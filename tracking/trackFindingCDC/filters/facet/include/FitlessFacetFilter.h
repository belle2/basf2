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

#include <tracking/trackFindingCDC/filters/facet/BaseFacetFilter.h>
#include <tracking/trackFindingCDC/eventdata/hits/CDCFacet.h>


namespace Belle2 {
  namespace TrackFindingCDC {
    /** Filter for the constuction of good facets investigating the feasability
     *  of the right left passage hypotheses combination.
     *  If the given combination cannot be made by a track reject it.
     */
    class FitlessFacetFilter : public Filter<CDCFacet> {

    private:
      /// Type of the super class
      typedef Filter<CDCFacet> Super;

    public:
      /// Constructor taking a flag if boarderline feasable cases should be excluded.
      explicit FitlessFacetFilter(bool hardCut = true);

    public:
      /** Main filter method returning the weight of the facet.
       *  Returns NAN if the cell shall be rejected.
       */
      virtual Weight operator()(const CDCFacet& facet) override final;

    public:
      /** Set the parameter with key to value.
       *
       *  Parameters are:
       *  hard_fitless_cut  - Switch to disallow the boarderline possible hit and
       *                      right left passage information.
       *                      Allowed values "true", "false". Default is "true".
       */
      virtual
      void setParameter(const std::string& key, const std::string& value) override;

      /** Returns a map of keys to descriptions describing the individual parameters of the filter.
       */
      virtual
      std::map<std::string, std::string> getParameterDescription() override;


    public:
      /// Setter for the flag that the boarderline cases should be excluded.
      void setHardCut(bool hardCut)
      {
        m_param_hardCut = hardCut;
      }

      /// Getter for the flag that the boarderline cases should be excluded.
      bool getHardCut() const
      {
        return m_param_hardCut;
      }


    private:
      /// Switch for hard selection.
      bool m_param_hardCut;

    }; // end class FitlessFacetFilter
  } //end namespace TrackFindingCDC
} //end namespace Belle2
