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

#include <pxd/dataobjects/PXDCluster.h>

#include <vxd/geometry/GeoCache.h>
#include <vxd/geometry/SensorInfoBase.h>
#include <mdst/dataobjects/Track.h>
#include <tracking/dataobjects/RecoTrack.h>
#include <tracking/dataobjects/ROIid.h>


//root stuff
#include "TVector3.h"

#include "TFile.h"
#include "TNtuple.h"


namespace Belle2 {

  /**
   * Creates Ntuples for PXD Efficiency analysis
   *
   * This module is doing the tracking itself, thus checking for hits on in and
   * outgoing leg of a track, which work only for cosmics. For tracks originating in IP
   * we will end up at 50%, depending on module order.
   * Do not use this module unless you know what to expect.
   */
  class PXDDQMEfficiencyNtupleSelftrackModule : public Module {

  public:

    /**
     * Constructor: Sets the description, the properties and the parameters of the module.
     */
    PXDDQMEfficiencyNtupleSelftrackModule();

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
    /** helper functions to do some of the calculations*/
    /* returns the space point in local coordinates where the track hits the sensor:
      sensorInfo: info of the sensor under investigation
      aTrack: the track to be tested
      isgood: flag which is false if some error occured (do not use the point if false)
      du and dv are the uncertainties in u and v on the sensor plane of the fit (local coordinates)
     */
    TVector3 getTrackInterSec(const VXD::SensorInfoBase& pxdSensorInfo, const RecoTrack& aTrack, bool& isgood, double& du, double& dv);
    /** find the closest cluster*/
    int findClosestCluster(const VxdID& vxdid, TVector3 intersection);
    /** is it close to the border*/
    bool isCloseToBorder(int u, int v, int checkDistance);
    /** is a dead pixel close*/
    bool isDeadPixelClose(int u, int v, int checkDistance, const VxdID& moduleID);

    /// if true alignment will be used!
    bool m_useAlignment;

    /// the geometry
    VXD::GeoCache& m_vxdGeometry;

    std::string m_pxdClustersName; ///< name of the store array of pxd clusters
    std::string m_tracksName; ///< name of the store array of tracks
    std::string m_recoTracksName; ///< name of the store array of recotracks
    std::string m_ROIsName; ///< name of the store array of ROIs

    StoreArray<PXDCluster> m_pxdclusters; ///< store array of pxd clusters
    StoreArray<Track> m_tracks; ///< store array of tracks
    StoreArray<RecoTrack> m_recoTracks; ///< store array of reco tracks
    StoreArray<ROIid> m_ROIs; ///< store array of ROIs

    double m_pcut; ///< pValue-Cut for tracks
    double m_momCut; ///< Cut on fitted track momentum
    double m_pTCut; ///< Cut on fitted track pT
    unsigned int m_minSVDHits; ///< Required hits in SVD strips for tracks
    int m_maskedDistance; ///< Distance inside which no dead pixel or module border is allowed

    TFile* m_file{}; ///< pointer to opened file
    TNtuple* m_tuple{}; ///< pointer to opened tuple

  };
}
