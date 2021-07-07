/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/filters/facet/BaseFacetFilter.h>

#include <string>

namespace Belle2 {


  namespace TrackFindingCDC {
    class CDCFacet;
    class CDCRLWireHitTriple;

    /**
     *  Filter for the constuction of good facets investigating the feasability
     *  of the right left passage hypotheses combination.
     *  If the given combination cannot be made by a track reject it.
     */
    class FeasibleRLFacetFilter : public BaseFacetFilter {

    private:
      /// Type of the super class
      using Super = BaseFacetFilter;

    public:
      /// Constructor taking a flag if boarderline feasable cases should be excluded.
      explicit FeasibleRLFacetFilter(bool hardRLCut = true);

    public:
      /// Expose the set of parameters of the filter to the module parameter list.
      void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) final;

    public:
      /**
       *  Main filter method returning the weight of the facet.
       *  Returns NAN if the cell shall be rejected.
       */
      Weight operator()(const CDCFacet& facet) final;

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
