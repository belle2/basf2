/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef SIMPLEFACETNEIGHBORCHOOSER_H_
#define SIMPLEFACETNEIGHBORCHOOSER_H_
#include <tracking/cdcLocalTracking/typedefs/BasicTypes.h>
#include <tracking/cdcLocalTracking/typedefs/BasicConstants.h>
#include <tracking/cdcLocalTracking/typedefs/UsedDataHolders.h>

#include "BaseFacetNeighborChooser.h"

namespace Belle2 {
  namespace CDCLocalTracking {

    ///Class filtering the neighborhood of facets based on simple criterions.
    class SimpleFacetNeighborChooser : public BaseFacetNeighborChooser {

    public:
      /** Constructor. */
      SimpleFacetNeighborChooser() : m_allowedDeviationCos(cos(PI / 180.0 * 180.0))
      {;}
      SimpleFacetNeighborChooser(FloatType allowedDeviationCos) : m_allowedDeviationCos(allowedDeviationCos)
      {;}

      /** Destructor.*/
      ~SimpleFacetNeighborChooser() {;}

      inline NeighborWeight isGoodNeighbor(
        const CDCRecoFacet& facet,
        const CDCRecoFacet& neighborFacet
      ) const {

        if (facet.getStartWire() == neighborFacet.getEndWire()) return NOT_A_NEIGHBOR;

        //the compatibility of the short legs or all?
        //start end to continuation middle end
        //start middle to continuation start end

        const ParameterLine2D& firstStartToMiddle = facet.getStartToMiddleLine();
        const ParameterLine2D& firstStartToEnd    = facet.getStartToEndLine();

        const ParameterLine2D& secondStartToEnd   = neighborFacet.getStartToEndLine();
        const ParameterLine2D& secondMiddleToEnd   = neighborFacet.getMiddleToEndLine();

        //check both
        if (firstStartToMiddle.tangential().cosWith(secondStartToEnd.tangential()) > m_allowedDeviationCos and
            firstStartToEnd.tangential().cosWith(secondMiddleToEnd.tangential()) > m_allowedDeviationCos) {

          // the weight must be -2 because the overlap of the facets is two points
          // so the amount of two facets is 4 points hence the cellular automat must calculate 3 + (-2) + 3 = 4 as cellstate
          // this can of course be adjusted for a more realistic information measure ( together with the facet creator filter)
          return -2;

        } else {

          return NOT_A_NEIGHBOR;

        }

      }

    private:
      FloatType m_allowedDeviationCos;

    }; // end class


  } //end namespace CDCLocalTracking
} //end namespace Belle2

#endif //SIMPLEFACETNEIGHBORCHOOSER_H_
