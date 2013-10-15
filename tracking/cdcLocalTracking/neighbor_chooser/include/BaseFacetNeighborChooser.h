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

#include <tracking/cdcLocalTracking/typedefs/BasicTypes.h>
#include <tracking/cdcLocalTracking/typedefs/BasicConstants.h>
#include <tracking/cdcLocalTracking/typedefs/UsedDataHolders.h>


namespace Belle2 {
  namespace CDCLocalTracking {
    ///Base class  for filtering the neighborhood of facets
    /** Base implementation providing the getLowestPossibleNeighbor and isStillPossibleNeighbor method
     *  using the geometry of the facet.
     *  Besides that it accepts all facets. The isGoodNeighbor method has to be made more sophisticated by a subclass.
     */
    class BaseFacetNeighborChooser {

    public:

      /// Default constructor
      BaseFacetNeighborChooser() {;}

      /// Empty destructor
      ~BaseFacetNeighborChooser() {;}

      /// Clears information from former events
      inline void clear() const {/*nothing to remember*/;}

      inline const CDCRecoFacet
      getLowestPossibleNeighbor(
        const CDCRecoFacet& facet
      ) const {
        return CDCRecoFacet::getLowerBound(facet.getMiddleWireHit(),
                                           facet.getMiddleRLInfo(),
                                           facet.getEndWireHit(),
                                           facet.getEndRLInfo());
      }

      inline bool
      isStillPossibleNeighbor(
        const CDCRecoFacet& facet,
        const CDCRecoFacet& neighborFacet,
        const CDCRecoFacet& lowestPossibleNeighbor __attribute__((unused))
      ) const {

        return *(facet.getMiddleWireHit()) == *(neighborFacet.getStartWireHit()) and
               facet.getMiddleRLInfo() == neighborFacet.getStartRLInfo() and
               * (facet.getEndWireHit()) == *(neighborFacet.getMiddleWireHit()) and
               facet.getEndRLInfo() == neighborFacet.getMiddleRLInfo();
      }

      inline
      Weight
      isGoodNeighbor(
        const CDCRecoFacet& facet,
        const CDCRecoFacet& neighborFacet,
        const CDCRecoFacet& lowestPossibleNeighbor __attribute__((unused))
      ) const {

        //the last wire of the neighbor should not be the same as the start wire of the facet
        if (facet.getStartWire() == neighborFacet.getEndWire()) {
          return NOT_A_NEIGHBOR;
        }

        //besides that the base implemetation accepts all facets
        //to be specialised by a sub class

        // the weight must be -2 because the overlap of the facets is two points
        // so the amount of two facets is 4 points hence the cellular automat
        // must calculate 3 + (-2) + 3 = 4 as cellstate
        // this can of course be adjusted for a more realistic information measure
        // ( together with the facet creator filter)
        return -2;

      }


    }; // end class


  } //end namespace CDCLocalTracking
} //end namespace Belle2

#endif //BASEFACETNEIGHBORCHOOSER_H_
