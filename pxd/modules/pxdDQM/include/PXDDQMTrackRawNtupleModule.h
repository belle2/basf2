/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/


#pragma once

#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>

#include <pxd/dataobjects/PXDRawHit.h>
#include <pxd/dataobjects/PXDDAQStatus.h>

#include <vxd/geometry/GeoCache.h>
#include <vxd/geometry/SensorInfoBase.h>
#include <mdst/dataobjects/Track.h>
#include <tracking/dataobjects/RecoTrack.h>

#include <tracking/pxdDataReductionClasses/PXDInterceptor.h>

#include "TVector3.h"
#include "TFile.h"
#include "TNtuple.h"


namespace Belle2 {

  /**
   * Creates Ntuples for PXD Trigger analysis
   *
   * relies on PXD intercepts from the interceptor module which needs to be run before.´
   */
  class PXDDQMTrackRawNtupleModule : public Module {

  public:

    /**
     * Constructor: Sets the description, the properties and the parameters of the module.
     */
    PXDDQMTrackRawNtupleModule();

  private:

    /**
     * main function which fills trees and histograms
     */
    void event() override final;

    /**
     * initializes the need store arrays, trees and histograms
     */
    void initialize() override final;

    /**
     * terminate , save tuple to file if needed
     */
    void terminate() override final;


  private:

    /// if true alignment will be used!
    bool m_useAlignment;

    /// the geometry
    VXD::GeoCache& m_vxdGeometry;

    std::string m_ntupleName; ///< name output file
    std::string m_pxdHitsName; ///< name of the store array of pxd clusters
    std::string m_tracksName; ///< name of the store array of tracks
    std::string m_recoTracksName; ///< name of the store array of recotracks
    std::string m_PXDInterceptListName; /**< intercept list name*/

    StoreArray<PXDRawHit> m_pxdhits; ///< store array of pxd clusters
    StoreArray<Track> m_tracks; ///< store array of tracks
    StoreArray<RecoTrack> m_recoTracks; ///< store array of reco tracks
    StoreArray<PXDIntercept> m_intercepts; ///< store array of PXD Intercepts
    /** Input array for DAQ Status. */
    StoreObjPtr<PXDDAQStatus> m_storeDAQEvtStats;

    double m_pcut{0}; ///< pValue-Cut for tracks
    double m_momCut{0}; ///< Cut on fitted track momentum
    double m_pTCut{0}; ///< Cut on fitted track pT
    unsigned int m_minSVDHits{0}; ///< Required hits in SVD strips for tracks
    int m_uDist{0}; ///< distance in ucell to intercept to accept hit
    int m_vDist{0}; ///< distance in vcell to intercept to accept hit

    TFile* m_file{nullptr}; ///< pointer to opened file
    TNtuple* m_tuple{nullptr}; ///< pointer to opened tuple

  };
}
