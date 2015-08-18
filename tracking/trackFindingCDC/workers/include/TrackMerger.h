/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <vector>

#include <framework/datastore/StoreArray.h>

#include <tracking/trackFindingCDC/topology/CDCWireTopology.h>
#include <tracking/trackFindingCDC/eventtopology/CDCWireHitTopology.h>

#include <tracking/trackFindingCDC/filters/wirehit_relation/WireHitRelationFilter.h>

#include <tracking/trackFindingCDC/ca/MultipassCellularPathFinder.h>
#include <tracking/trackFindingCDC/ca/Clusterizer.h>

#include <tracking/trackFindingCDC/eventdata/CDCEventData.h>

#include <tracking/trackFindingCDC/creators/FacetCreator.h>


namespace Belle2 {
  namespace TrackFindingCDC {

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
      inline void merge(std::vector<CDCTrack>& tracks)
      {

        copyToDataStoreForDebug();
      }

    private:
      /// Helper function to copy intermediate objects to the data store for analysis from python.
      void copyToDataStoreForDebug() const
      {


        // IO for monitoring in python
#ifdef TRACKFINDINGCDC_USE_ROOT_BASE
#endif

      }

    private:
      /// Memory for the tracks after merging.
      std::vector< CDCTrack >& m_mergedTracks;

    }; // end class FacetSegmentWorker
  } //end namespace TrackFindingCDC
} //end namespace Belle2

