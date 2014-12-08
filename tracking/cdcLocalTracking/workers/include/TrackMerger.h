/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef TRACKMERGER_H_
#define TRACKMERGER_H_

#include <vector>

#include <framework/datastore/StoreArray.h>

#include <tracking/cdcLocalTracking/topology/CDCWireTopology.h>
#include <tracking/cdcLocalTracking/eventtopology/CDCWireHitTopology.h>

#include <tracking/cdcLocalTracking/filters/wirehit_wirehit/WireHitNeighborChooser.h>

#include <tracking/cdcLocalTracking/algorithms/MultipassCellularPathFinder.h>
#include <tracking/cdcLocalTracking/algorithms/Clusterizer.h>

#include <tracking/cdcLocalTracking/eventdata/CDCEventData.h>

#include <tracking/cdcLocalTracking/creators/FacetCreator.h>


namespace Belle2 {
  namespace CDCLocalTracking {

    class TrackMerger {

    public:
      /** Constructor. */
      TrackMerger() {;}

      /** Destructor.*/
      ~TrackMerger() {;}

      /// Forwards the initialize method of the module
      void initialize() {}

      /// Forwards the terminate method of the module
      void terminate() {}

      /// Generates the segments from the event topology.
      inline void merge(std::vector<CDCTrack>& tracks) {

        copyToDataStoreForDebug();
      }

    private:
      /// Helper function to copy intermediate objects to the data store for analysis from python.
      void copyToDataStoreForDebug() const {


        // IO for monitoring in python
#ifdef CDCLOCALTRACKING_USE_ROOT
#endif

      }

    private:
      /// Memory for the tracks after merging.
      std::vector< CDCTrack >& m_mergedTracks;

    }; // end class FacetSegmentWorker
  } //end namespace CDCLocalTracking
} //end namespace Belle2

#endif //FACETSEGMENTWORKER_H_
