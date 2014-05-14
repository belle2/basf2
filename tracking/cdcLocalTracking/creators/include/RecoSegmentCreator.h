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

#include <vector>

#include <tracking/cdcLocalTracking/eventdata/entities/CDCRecoFacet.h>
#include <tracking/cdcLocalTracking/eventdata/segments/CDCRecoSegment2D.h>


namespace Belle2 {
  namespace CDCLocalTracking {
    /// Class providing the reduction form facets sequences to reconstructed hits
    class RecoSegmentCreator {

    public:

      /** Constructor. */
      RecoSegmentCreator();

      /** Destructor.*/
      ~RecoSegmentCreator();

      /// Converts many paths of facets to tracks of two dimensional reconstructed hits averaging the hits that are present more two or three.
      void create(const std::vector< std::vector<const CDCRecoFacet* > >& facetPaths,
                  std::vector<CDCRecoSegment2D>& recoSegments) const;

      /// Convert a path of facets to a track of two dimensional reconstructed hits averaging the hits that are present more two or three.
      void create(const std::vector<const CDCRecoFacet* >& facetPath, CDCRecoSegment2D& recoSegment) const;

    private:

    }; // end class RecoSegmentCreator
  } //end namespace CDCLocalTracking
} //end namespace Belle2

#endif //RECOSEGMENTCREATOR_H_
