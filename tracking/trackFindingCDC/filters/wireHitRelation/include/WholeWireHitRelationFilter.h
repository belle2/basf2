/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/filters/base/RelationFilter.dcl.h>

#include <vector>
#include <string>

namespace Belle2 {
  class ModuleParamList;

  namespace TrackFindingCDC {
    class CDCWireHit;

    /// Class mapping the neighborhood of wires to the neighborhood of wire hits.
    class WholeWireHitRelationFilter : public RelationFilter<CDCWireHit> {

    public:
      /// Constructor form the default neighborhood degree
      explicit WholeWireHitRelationFilter(int neighborhoodDegree = 2);

      /// Default destructor
      ~WholeWireHitRelationFilter();

      /// Expose the parameters to a module
      void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) final;

      /**
       *  Returns a vector containing the neighboring wire hits of the given wire hit out of the
       *  sorted range given by the two iterator other argumets.
       */
      std::vector<CDCWireHit*> getPossibleTos(CDCWireHit* from,
                                              const std::vector<CDCWireHit*>& wireHits) const final;

    private:
      /// Degree of the neighbor extend
      int m_param_degree = 2;
    };
  }
}
