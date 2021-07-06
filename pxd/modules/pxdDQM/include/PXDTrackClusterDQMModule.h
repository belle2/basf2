/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/core/HistoModule.h>
#include <vxd/geometry/GeoCache.h>
#include <framework/datastore/StoreArray.h>
#include <mdst/dataobjects/Track.h>
#include <tracking/dataobjects/RecoTrack.h>
#include <TH1F.h>

#include <map>
#include <array>

namespace Belle2 {

  /** DQM of cluster in matched tracks
    */
  class PXDTrackClusterDQMModule : public HistoModule {  // <- derived from HistoModule class

  public:

    /** Constructor */
    PXDTrackClusterDQMModule();


  private:

    /** Initialize */
    void initialize() override final;
    /** Begin run */
    void beginRun() override final;
    /** Event */
    void event() override final;
    /** Define histograms */
    void defineHisto() override final;

    std::string m_histogramDirectoryName; /**< Name of the histogram directory in ROOT file */
    bool m_moreHistos; /**< Fill aditional histograms */
    bool m_ASICHistos; /**< Fill aditional histograms per ASCI region */

    /// the geometry
    VXD::GeoCache& m_vxdGeometry;

    /** StoreArray name where Tracks are written. */
    std::string m_TracksStoreArrayName;
    /** StoreArray name where RecoTracks are written. */
    std::string m_RecoTracksStoreArrayName;

    StoreArray<RecoTrack> m_recoTracks; /**< reco track store array */
    StoreArray<Track> m_tracks; /**< reco track store array */

    /** Cluster Charge for PXD clusters */
    std::map <VxdID, TH1F*> m_trackClusterCharge{};
    /** Cluster Charge for PXD clusters, uncorrected */
    std::map <VxdID, TH1F*> m_trackClusterChargeUC{};
    /** Cluster Charge for ASIC combinations for PXD clusters */
    std::map <VxdID, std::array<std::array<TH1F*, 4>, 6>> m_trackASICClusterCharge{};

    /** Number of Tracked Clusters per module */
    TH1F* m_trackedClusters{};

    /** map vxd id to dhe id */
    std::map<VxdID, int> m_vxd_to_dhe;

  };  //end class declaration

}  // end namespace Belle2

