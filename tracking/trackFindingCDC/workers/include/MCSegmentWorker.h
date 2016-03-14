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

#include <tracking/trackFindingCDC/topology/CDCWireTopology.h>
#include <tracking/trackFindingCDC/eventtopology/CDCWireHitTopology.h>

#include <tracking/trackFindingCDC/eventdata/CDCEventData.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Worker for building reconstructed segments form wirehits using reconstructed facets
    class MCSegmentWorker : public SwitchableRootificationBase {

    public:
      /** Constructor. */
      MCSegmentWorker(bool copyToDataStoreForDebug = true) : m_copyToDataStoreForDebug(copyToDataStoreForDebug) {;}

      /** Destructor.*/
      ~MCSegmentWorker() {;}

      /// Forwards the initialize method of the module to the facet creator and the neighborhood builder
      void initialize() {
#ifdef TRACKFINDINGCDC_USE_ROOT
        StoreArray < Belle2::TrackFindingCDC::CDCRecoSegment2D >::registerTransient();
#endif
      }


      /// Forwards the terminate method of the module
      void terminate() {
      }

      /// Generates the segments from the event topology.
      std::vector<Belle2::TrackFindingCDC::CDCRecoSegment2D>& generate(bool allowBackward = false);

      /// Generates the segments from the event topology.
      void generate(std::vector<Belle2::TrackFindingCDC::CDCRecoSegment2D>& outputSegments,  bool allowBackward = false);

    private:
      /// Helper function to copy intermediate objects to the data store for analysis from python.
      void copyToDataStoreForDebug() const;

    private:
      //typedefs

    private:

      /// Memory for the segments extracted from the paths
      std::vector<Belle2::TrackFindingCDC::CDCRecoSegment2D> m_segments2D;

      /// Switch to write out the intermediate data objects out to DataStore.
      bool m_copyToDataStoreForDebug;

    private:
      /// ROOT Macro to make MCSegmentWorker a ROOT class.
      TRACKFINDINGCDC_SwitchableClassDef(MCSegmentWorker, 1);



    }; // end class MCSegmentWorker

  } //end namespace TrackFindingCDC
} //end namespace Belle2

#endif //MCSEGMENTWORKER_H_
