/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef TANGENTSEGMENTCREATOR_H_
#define TANGENTSEGMENTCREATOR_H_

#include<vector>

#include <tracking/cdcLocalTracking/typedefs/UsedDataHolders.h>

namespace Belle2 {
  namespace CDCLocalTracking {
    /// Class providing the reduction form facets sequences to reconstructed tangents
    class TangentSegmentCreator {

    public:

      /** Constructor. */
      TangentSegmentCreator();

      /** Destructor.*/
      ~TangentSegmentCreator();


      typedef Belle2::CDCLocalTracking::CDCRecoFacet Item;

      //in type
      typedef Belle2::CDCLocalTracking::CDCRecoFacetPtrSegment FacetSegment;

      //out type
      typedef Belle2::CDCLocalTracking::CDCRecoTangentCollection TangentSegment;

      void create(const std::vector<FacetSegment>& facetSegments,
                  std::vector<TangentSegment>& tangentSegments) const;


      void create(const FacetSegment& facetSegment, TangentSegment& recoSegment) const;

    private:

    }; // end class TangentSegmentCreator
  } //end namespace CDCLocalTracking
} //end namespace Belle2

#endif //TANGENTSEGMENTCREATOR_H_
