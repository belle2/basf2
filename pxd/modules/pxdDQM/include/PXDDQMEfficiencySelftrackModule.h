/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/


#pragma once

#include <framework/core/HistoModule.h>
#include <framework/datastore/StoreArray.h>

#include <pxd/dataobjects/PXDCluster.h>

#include <vxd/geometry/GeoCache.h>
#include <vxd/geometry/SensorInfoBase.h>

#include <tracking/dataobjects/RecoTrack.h>
#include <tracking/dataobjects/ROIid.h>

#include "TH1F.h"
#include "TH2F.h"
#include "TVector3.h"


namespace Belle2 {

  /**
   * Creates the basic histograms for PXD Efficiency DQM
   * Simplified and adopted version of the testbeam pxd efficiency module
   *
   * This module is doing the tracking itself, thus checking for hits on in and
   * outgoing leg of a track, which work only for cosmics. For tracks originating in IP
   * we will end up at 50%, depending on module order.
   * Do not use this module unless you know what to expect.
   */
  class PXDDQMEfficiencySelftrackModule : public HistoModule {

  public:

    /**
     * Constructor: Sets the description, the properties and the parameters of the module.
     */
    PXDDQMEfficiencySelftrackModule();

  private:

    /**
     * main function which fills trees and histograms
     */
    void event() override final;

    /**
     * begin run function which resets histograms
     */
    void beginRun() override final;

    /**
     * initializes the need store arrays, trees and histograms
     */
    void initialize() override final;

    /**
     * actually defines the trees and histograms
     */
    void defineHisto() override final;



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

    /// Require tracks going through ROIs
    bool m_requireROIs;

    /// if true alignment will be used!
    bool m_useAlignment;
    /// mask dead pixels
    bool m_maskDeadPixels;
    /// cut borders
    bool m_cutBorders;
    /// add some verbose histograms for cuts
    bool m_verboseHistos;

    /// the geometry
    VXD::GeoCache& m_vxdGeometry;

    /// Where to save the histograms too
    std::string m_histogramDirectoryName;

    std::string m_pxdClustersName; ///< name of the store array of pxd clusters
    std::string m_tracksName; ///< name of the store array of tracks
    std::string m_ROIsName; ///< name of the store array of ROIs

    int m_u_bins; ///< the u bins
    int m_v_bins; ///< the v bins

    StoreArray<PXDCluster> m_pxdclusters; ///< store array of pxd clusters
    StoreArray<RecoTrack> m_tracks; ///< store array of tracks
    StoreArray<ROIid> m_ROIs; ///< store array of ROIs

    double m_distcut; ///< distance cut in cm!
    double m_uFactor; ///< factor for track-error on distcut comparison
    double m_vFactor; ///< factor for track-error on distcut comparison
    double m_pcut; ///< pValue-Cut for tracks
    double m_momCut; ///< Cut on fitted track momentum
    double m_pTCut; ///< Cut on fitted track pT
    unsigned int m_minSVDHits; ///< Required hits in SVD strips for tracks
    double m_z0minCut;/**< cut z0 minimum in cm (large negativ value eg -9999 disables)*/
    double m_z0maxCut;/**< cut z0 maximum in cm (large positiv value eg 9999 disables)*/
    double m_d0Cut;/**< cut abs(d0) in cm (large positiv value eg 9999 disables)*/
    int m_maskedDistance; ///< Distance inside which no dead pixel or module border is allowed

    //Histograms to later determine efficiency
    std::map<VxdID, TH2F*> m_h_track_hits; ///< histograms of track hits
    std::map<VxdID, TH2F*> m_h_matched_cluster; ///< histograms of matched clusters
    std::map<VxdID, TH1F*> m_h_p; ///< histograms of momenta
    std::map<VxdID, TH1F*> m_h_pt; ///< histograms of transverse momenta
    std::map<VxdID, TH1F*> m_h_su; ///< histograms of su
    std::map<VxdID, TH1F*> m_h_sv; ///< histograms of sv
    std::map<VxdID, TH1F*> m_h_p2; ///< histograms of p2
    std::map<VxdID, TH1F*> m_h_pt2; ///< histograms of pt2
    std::map<VxdID, TH1F*> m_h_su2; ///< histrograms of su2
    std::map<VxdID, TH1F*> m_h_sv2; ///< histrograms of sv2

  };
}
