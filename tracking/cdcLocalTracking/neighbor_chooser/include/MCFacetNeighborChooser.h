/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef MCFACETNEIGHBORCHOOSER_H_
#define MCFACETNEIGHBORCHOOSER_H_

#include <tracking/cdcLocalTracking/typedefs/BasicTypes.h>
#include <tracking/cdcLocalTracking/typedefs/BasicConstants.h>
#include <tracking/cdcLocalTracking/typedefs/UsedDataHolders.h>

#include <tracking/cdcLocalTracking/creator_filters/MCFacetFilter.h>

#include "BaseFacetNeighborChooser.h"

namespace Belle2 {
  namespace CDCLocalTracking {
    ///Class filtering the neighborhood of facets with monte carlo information
    class MCFacetNeighborChooser : public BaseFacetNeighborChooser {

    public:

      /** Constructor. */
      MCFacetNeighborChooser() {;}

      /** Destructor.*/
      ~MCFacetNeighborChooser() {;}

      inline
      NeighborWeight
      isGoodNeighbor(
        const CDCRecoFacet& facet,
        const CDCRecoFacet& neighborFacet
      ) const {



        //the last wire of the neighbor should not be the same as the start wire of the facet
        if (facet.getStartWire() == neighborFacet.getEndWire()) {
          return NOT_A_NEIGHBOR;
        }



        // Despite of that two facets are neighbors if both are true facets
        // That also implies the correct tof alignment of the hits not common to both facets
        CellWeight facetWeight = m_mcFacetFilter.isGoodFacet(facet);
        CellWeight neighborWeight = m_mcFacetFilter.isGoodFacet(neighborFacet);

        bool mcDecision = (not isNotACell(facetWeight)) and (not isNotACell(neighborWeight));


        // the weight must be -2 because the overlap of the facets is two points
        // so the amount of two facets is 4 points hence the cellular automat
        // must calculate 3 + (-2) + 3 = 4 as cellstate
        // this can of course be adjusted for a more realistic information measure
        // ( together with the facet creator filter)
        return mcDecision ? -2.0 : NOT_A_NEIGHBOR;


        /*
        const CDCWireHit * startWireHit   = facet.getStartWireHit();

        const CDCWireHit * middleWireHit  = facet.getMiddleWireHit();
              //  == neighborFacet.getStartWireHit();

        const CDCWireHit * endWireHit     = facet.getEndWireHit();
              //  == neighborFacet.getMiddleWireHit();

        const CDCWireHit * neighborStartWireHit   = facet.getStartWireHit();
              //  == facet.getMiddleWireHit();

        const CDCWireHit * neighborMiddleWireHit  = facet.getMiddleWireHit();
              //  == facet.getEndWireHit();

        const CDCWireHit * neighborEndWireHit     = facet.getEndWireHit();


        ITrackType iTrack
            = m_mcLookUp.getMajorMCTrackId(startWireHit,
                                           middleWireHit,
                                           endWireHit);


        ITrackType neighborITrack
            = m_mcLookUp.getMajorMCTrackId(neighborStartWireHit,
                                           neighborMiddleWireHit,
                                           neighborEndWireHit);

        if ( iTrack == INVALID_ITRACK or
             neighborITrack == INVALID_ITRACK or
             neighborITrack != iTrack ){

          //B2DEBUG(200,"Rejected facet neighbor because major track id does not match");

        }


        return -2;
        */
      }

    private:
      const MCFacetFilter m_mcFacetFilter;
      //const CDCMCLookUp & m_mcLookUp;



    }; // end class


  } //end namespace CDCLocalTracking
} //end namespace Belle2

#endif //MCFACETNEIGHBORCHOOSER_H_
