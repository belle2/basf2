/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef BASEFACETNEIGHBORCHOOSER_H_
#define BASEFACETNEIGHBORCHOOSER_H_

#include <boost/range/iterator_range.hpp>

#include <tracking/trackFindingCDC/typedefs/BasicTypes.h>

#include <tracking/trackFindingCDC/algorithms/NeighborWeight.h>

#include <tracking/trackFindingCDC/eventdata/entities/CDCRLWireHitPair.h>
#include <tracking/trackFindingCDC/eventdata/entities/CDCRecoFacet.h>

namespace Belle2 {
  namespace TrackFindingCDC {
    /// Base class for filtering the neighborhood of facets.
    /** Base implementation providing the getLowestPossibleNeighbor and isStillPossibleNeighbor method
     *  using the geometry of the facet.
     *  Besides that it accepts all facets. The isGoodNeighbor method has to be made more sophisticated by a subclass.
     */
    class BaseFacetNeighborChooser {

    public:

      /// Default constructor
      BaseFacetNeighborChooser() {;}

      /// Empty destructor
      virtual ~BaseFacetNeighborChooser() {;}

      /// Clears information from former events
      virtual void clear() {;}

      /// Forwards the initialize method from the module
      virtual void initialize() {;}

      /// Forwards the terminate method from the module
      virtual void terminate() {;}

      /// Returns a two iterator range covering the range of possible neighboring facets of the given facet out of the sorted range given by the two other argumets.
      template<class CDCRecoFacetIterator>
      boost::iterator_range<CDCRecoFacetIterator> getPossibleNeighbors(const CDCRecoFacet& recoFacet, const CDCRecoFacetIterator& itBegin, const CDCRecoFacetIterator& itEnd) const {

        const CDCRLWireHitPair& rearRLWireHitPair = recoFacet.getRearRLWireHitPair();

        std::pair<CDCRecoFacetIterator, CDCRecoFacetIterator> itPairPossibleNeighbors = std::equal_range(itBegin, itEnd, rearRLWireHitPair);
        return boost::iterator_range<CDCRecoFacetIterator>(itPairPossibleNeighbors.first, itPairPossibleNeighbors.second);
      }

      /// Main filter method returning the weight of the neighborhood relation. Return always returns NOT_A_NEIGHBOR to reject all facet neighbors.
      virtual inline NeighborWeight isGoodNeighbor(const CDCRecoFacet&,
                                                   const CDCRecoFacet&) {
        return NOT_A_NEIGHBOR;
      }
    }; // end class

  } //end namespace TrackFindingCDC
} //end namespace Belle2

#endif //BASEFACETNEIGHBORCHOOSER_H_
