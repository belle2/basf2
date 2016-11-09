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
    /**
     *  Filter for the constuction of good facets investigating the feasability
     *  of the right left passage hypotheses combination.
     *  If the given combination cannot be made by a track reject it.
     */
    class FeasibleRLFacetFilter : public Filter<CDCFacet> {

    private:
      /// Type of the super class
      using Super = Filter<CDCFacet>;

    public:
      /// Constructor taking a flag if boarderline feasable cases should be excluded.
      explicit FeasibleRLFacetFilter(bool hardRLCut = true);

    public:
      /// Expose the set of parameters of the filter to the module parameter list.
      virtual void exposeParameters(ModuleParamList* moduleParamList,
                                    const std::string& prefix) override;

    public:
      /**
       *  Main filter method returning the weight of the facet.
       *  Returns NAN if the cell shall be rejected.
       */
      virtual Weight operator()(const CDCFacet& facet) override final;

    public:
      /// Setter for the flag that the boarderline cases should be excluded.
      void setHardRLCut(bool hardRLCut)
      {
        m_param_hardRLCut = hardRLCut;
      }

      /// Getter for the flag that the boarderline cases should be excluded.
      bool getHardRLCut() const
      {
        return m_param_hardRLCut;
      }

    private:
      /// Check if the hit triplet is a feasible combination for shape and rl passage information.
      bool isFeasible(const CDCRLWireHitTriple& rlWireHitTriple) const;

    private:
      /// Switch for hard selection.
      bool m_param_hardRLCut = true;

    };
  }
}
