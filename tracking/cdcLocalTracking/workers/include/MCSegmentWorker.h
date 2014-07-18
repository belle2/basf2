/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef MCSEGMENTWORKER_H_
#define MCSEGMENTWORKER_H_

#include <vector>

#include <framework/datastore/StoreArray.h>

#include <tracking/cdcLocalTracking/topology/CDCWireTopology.h>
#include <tracking/cdcLocalTracking/eventtopology/CDCWireHitTopology.h>

#include <tracking/cdcLocalTracking/eventdata/CDCEventData.h>

namespace Belle2 {
  namespace CDCLocalTracking {

    /// Worker for building reconstructed segments form wirehits using reconstructed facets
    class MCSegmentWorker {

    public:
      /** Constructor. */
      MCSegmentWorker() {;}

      /** Destructor.*/
      ~MCSegmentWorker() {;}

      /// Forwards the initialize method of the module to the facet creator and the neighborhood builder
      void initialize() {
#ifdef CDCLOCALTRACKING_USE_ROOT
        StoreArray < Belle2::CDCLocalTracking::CDCRecoSegment2D >::registerTransient();
#endif
      }


      /// Forwards the terminate method of the module
      void terminate() {
      }

      /// Generates the segments from the event topology.
      void generate(std::vector<Belle2::CDCLocalTracking::CDCRecoSegment2D>& outputSegments);

    private:
      /// Helper function to copy intermediate objects to the data store for analysis from python.
      void copyToDataStoreForDebug() const;

    private:
      //typedefs

    private:

      /// Memory for the segments extracted from the paths
      std::vector<Belle2::CDCLocalTracking::CDCRecoSegment2D> m_segments2D;

    }; // end class MCSegmentWorker

  } //end namespace CDCLocalTracking
} //end namespace Belle2

#endif //MCSEGMENTWORKER_H_
