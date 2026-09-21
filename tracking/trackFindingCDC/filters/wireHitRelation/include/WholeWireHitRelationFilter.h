/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/trackingUtilities/filters/base/RelationFilter.dcl.h>

#include <vector>
#include <string>
#include <cstddef>

namespace Belle2 {
  class ModuleParamList;

  namespace CDC {
    class CDCWire;
  }
  namespace TrackingUtilities {
    class CDCWireHit;
  }
  namespace TrackFindingCDC {

    /// Class mapping the neighborhood of wires to the neighborhood of wire hits.
    class WholeWireHitRelationFilter : public TrackingUtilities::RelationFilter<TrackingUtilities::CDCWireHit> {

    public:
      /// Constructor form the default neighborhood degree
      explicit WholeWireHitRelationFilter(int neighborhoodDegree = 2);

      /// Default destructor
      ~WholeWireHitRelationFilter() override;

      /// Expose the parameters to a module
      void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) final;

      /**
       *  Returns a vector containing the neighboring wire hits of the given wire hit out of the
       *  sorted range given by the two iterator other arguments.
       */
      std::vector<TrackingUtilities::CDCWireHit*> getPossibleTos(TrackingUtilities::CDCWireHit* from,
                                                                 const std::vector<TrackingUtilities::CDCWireHit*>& wireHits) const final;

      /**
       *  Precompute the wires of the given wire hits to speed up the searches
       *  in the following getPossibleTos calls made with the very same vector.
       *
       *  Optional - getPossibleTos calls with any other vector fall back to
       *  searching the wire hits directly.
       */
      void prepare(const std::vector<TrackingUtilities::CDCWireHit*>& wireHits);

    private:
      /// Degree of the neighbor extend
      int m_param_degree = 2;

      /// Memory for the wires of the prepared wire hit vector.
      std::vector<const CDC::CDCWire*> m_preparedWires;

      /// Data pointer of the prepared wire hit vector used to recognize it in getPossibleTos.
      TrackingUtilities::CDCWireHit* const* m_preparedWireHitsData = nullptr;

      /// Size of the prepared wire hit vector used to recognize it in getPossibleTos.
      std::size_t m_preparedWireHitsSize = 0;
    };
  }
}
