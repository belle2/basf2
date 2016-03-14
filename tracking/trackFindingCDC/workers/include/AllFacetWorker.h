/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#ifndef ALLFACETWORKER_H
#define ALLFACETWORKER_H


#include <tracking/trackFindingCDC/eventdata/CDCEventData.h>

#include <tracking/trackFindingCDC/eventtopology/CDCWireHitTopology.h>
#include <tracking/trackFindingCDC/topology/CDCWireTopology.h>

#include <framework/datastore/StoreArray.h>

#include <vector>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Worker for building all facets without a filter applied. This is mainly a helper to evaluate a filter.
    class AllFacetWorker : public SwitchableRootificationBase {

    public:
      /** Constructor. */
      AllFacetWorker(bool copyToDataStoreForDebug = true) : m_copyToDataStoreForDebug(copyToDataStoreForDebug) {;}

      /** Destructor.*/
      ~AllFacetWorker() {;}

      /// Forwards the initialize method of the module to the facet creator and the neighborhood chooser
      void initialize() {
#ifdef TRACKFINDINGCDC_USE_ROOT
        StoreArray < CDCRecoFacet >::registerTransient();
#endif
      }

      /// Forwards the terminate method of the module to the facet creator and the neighborhood chooser. Does nothing here.
      void terminate() {;}

      /// Generates all possible facets without filtering them from the event topology.
      void generate();



    private:
      /// Helper function to copy intermediate objects to the data store for analysis from python.
      void copyToDataStoreForDebug() const;


    public:
      const CDCRecoFacetCollection& getRecoFacets() const
      { return m_recoFacets; }

    private:
      /// Memory for the constructed facets.
      CDCRecoFacetCollection m_recoFacets;

      /// Switch to write out the intermediate data objects out to DataStore.
      bool m_copyToDataStoreForDebug;

      /// ROOT Macro to make AllFacetWorker a ROOT class.
      TRACKFINDINGCDC_SwitchableClassDef(AllFacetWorker, 1);

    }; // end class AllFacetWorker
  } //end namespace TrackFindingCDC
} //end namespace Belle2

#endif //ALLFACETWORKER_H_
