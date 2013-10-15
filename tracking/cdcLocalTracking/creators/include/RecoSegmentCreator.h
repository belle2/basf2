/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef RECOSEGMENTCREATOR_H_
#define RECOSEGMENTCREATOR_H_

#include<vector>

#include <tracking/cdcLocalTracking/typedefs/UsedDataHolders.h>

namespace Belle2 {
  namespace CDCLocalTracking {
    /// Class providing the reduction form facets sequences to reconstructed hits
    class RecoSegmentCreator {

    public:

      /** Constructor. */
      RecoSegmentCreator();

      /** Destructor.*/
      ~RecoSegmentCreator();

      typedef Belle2::CDCLocalTracking::CDCRecoFacet Item;

      //in type
      typedef Belle2::CDCLocalTracking::CDCRecoFacetPtrSegment FacetSegment;

      //out type
      typedef Belle2::CDCLocalTracking::CDCRecoSegment2D RecoSegment2D;
      typedef Belle2::CDCLocalTracking::CDCRecoTangentCollection TangentSegment;

      void create(const std::vector<FacetSegment>& facetSegments,
                  std::vector<RecoSegment2D>& recoSegments) const;


      void create(const FacetSegment& facetSegment, RecoSegment2D& recoSegment) const;

    private:

    }; // end class RecoSegmentCreator
  } //end namespace CDCLocalTracking
} //end namespace Belle2

#endif //RECOSEGMENTCREATOR_H_
