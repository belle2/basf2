/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2021 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Qingyuan Liu                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>
//#include <framework/database/DBObjPtr.h>

#include <pxd/dataobjects/PXDCluster.h>
#include <mdst/dataobjects/Track.h>
#include <tracking/dataobjects/RecoTrack.h>
#include <tracking/dataobjects/PXDIntercept.h>

#include <pxd/utilities/PXD2TrackEvent.h>

//#include <vxd/dataobjects/VxdID.h>
//#include <vxd/geometry/SensorInfoBase.h>
//#include <pxd/geometry/SensorInfo.h>
//#include <vxd/geometry/GeoCache.h>
//#include <string>

namespace Belle2 {

  //class PXD2TrackEvent;

  //namespace PXD {

  /** PXD Performance module.
   * This module collects data from cDSTs and save them into an event class
   * for performance studies.
   */
  class PXDPerformanceModule: public Module {

  public:


    /** Constructor */
    PXDPerformanceModule();
    /** Destructor */
    virtual ~PXDPerformanceModule();

    /** Initialize module */
    virtual void initialize() override;
    /** Start-of-run initializations */
    virtual void beginRun() override;
    /** Event processing */
    virtual void event() override;
    /** End-of-run tasks */
    virtual void endRun() override;
    /** Final summary and cleanup */
    virtual void terminate() override;

  private:

    /** Required input PXDClustersFromTracks  */
    StoreArray<PXDCluster> m_pxdClustersFromTracks;
    /** Required input Tracks */
    StoreArray<Track> m_tracks;
    /** Required input RecoTracks */
    StoreArray<RecoTrack> m_recoTracks;
    /** Required input PXDIntercepts */
    StoreArray<PXDIntercept> m_pxdIntercepts;
    //[>* Required input EventMetaData <]
    //StoreObjPtr<EventMetaData> m_evtMetaData;
    /** Output PXD2TrackEvent */
    StoreArray<PXD2TrackEvent> m_pxd2TrackEvents;

    /** Name of the collection to use for PXDClustersFromTracks */
    std::string m_storeClustersFromTracksName;
    /** Name of the collection to use for Tracks */
    std::string m_storeTracksName;
    /** Name of the collection to use for RecoTracks */
    std::string m_storeRecoTracksName;
    /** Name of the collection to use for PXDIntercepts */
    std::string m_storeInterceptsName;

  };

  //} // namespace PXD
} // namespace Belle2

