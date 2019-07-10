/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2019 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Bjoern Spruck                                            *
 * Use: Do some stupid things with matched PXD custers                    *
 * Date: Sternzeit 778378246.65                                           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>
#include <framework/core/HistoModule.h>
#include <vxd/geometry/GeoCache.h>
#include <vxd/geometry/SensorInfoBase.h>
#include <framework/datastore/StoreArray.h>
#include <mdst/dataobjects/Track.h>
#include <tracking/dataobjects/RecoTrack.h>
#include <TH1F.h>
#include <TH2F.h>

#include <map>

namespace Belle2 {

  /** DQM of cluster in matched tracks
    */
  class PXDTrackClusterDQMModule : public HistoModule {  // <- derived from HistoModule class

  public:

    /** Constructor */
    PXDTrackClusterDQMModule();


  private:

    /** Module functions */
    void initialize() override final;
    void beginRun() override final;
    void event() override final;
    void defineHisto() override final;

    std::string m_histogramDirectoryName; /**< Name of the histogram directory in ROOT file */
    bool m_moreHistos; /**< Fill aditional histograms */

    //the geometry
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

  };  //end class declaration

}  // end namespace Belle2

