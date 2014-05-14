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

#include <vector>

#include <tracking/cdcLocalTracking/eventdata/entities/CDCRecoFacet.h>
#include <tracking/cdcLocalTracking/eventdata/collections/CDCRecoTangentVector.h>

namespace Belle2 {
  namespace CDCLocalTracking {
    /// Class providing the reduction form facets sequences to reconstructed tangents.
    /** This class is only meant to export facet paths a collections of tangents to the DataStore for monitoring from Python side */
    class TangentSegmentCreator {

    public:

      /** Constructor. */
      TangentSegmentCreator();

      /** Destructor.*/
      ~TangentSegmentCreator();

      /// Converts many paths of facets to collections of tangents.
      void create(const std::vector< std::vector<const CDCRecoFacet*> >& facetPaths,
                  std::vector<CDCRecoTangentVector>& recoTangentSegments) const;

      /// Converts a path of facets to a collection of tangents.
      void create(const std::vector<const CDCRecoFacet*>& facetPath,
                  CDCRecoTangentVector& recoTangentSegment) const;

    private:

    }; // end class TangentSegmentCreator
  } //end namespace CDCLocalTracking
} //end namespace Belle2

#endif //TANGENTSEGMENTCREATOR_H_
