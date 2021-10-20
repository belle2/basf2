/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <calibration/CalibrationCollectorModule.h>
#include <framework/datastore/StoreObjPtr.h>
#include <vxd/geometry/GeoCache.h>
#include <pxd/geometry/SensorInfo.h>
#include <pxd/utilities/PXDPerformanceStructs.h>
#include <pxd/utilities/PXDUtilities.h>
#include <pxd/reconstruction/PXDGainCalibrator.h>
#include <pxd/dbobjects/PXDGainMapPar.h>
#include <pxd/reconstruction/PXDPixelMasker.h>

#include <string>

namespace Belle2 {
  /**
   * Collector module for PXD gain calibration and PXD calibration validation
   *
   * For gain calibration, tracks (particles) are selected using basf2 modular analysis.
   * Further selection based on cluster size/charge is then applied in this module.
   * As in the ClusterChargeCollector, this collector also creates a grid of rectangular
   * regions on each sensor (granularity is steerable with nBinsU and nBinsV) and creates
   * TTree objects for each grid region. Cluster charge values and the expected ones are
   * filled into these trees. The gain is later estimated with the MPV of the ratio,
   * cluster charge / expected value, in the calibration algorithm.
   *
   * Performance variables for estimating PXD efficiency and impact parameter resolution
   * are also collected using related particle lists.
   *
   */
  class PXDPerformanceVariablesCollectorModule : public CalibrationCollectorModule {

  public:

    /**
     * Constructor: Sets the description, the properties and the parameters of the module.
     */
    PXDPerformanceVariablesCollectorModule();
    /** Prepare */
    void prepare() override final;
    /** Collect */
    void collect() override final;
    /** Start run */
    void startRun() override final;

    /**
     * Collect info for impact parameter study on event level
     */
    void collectDeltaIP();
    /**
     * Collect variables for gain calibration
     * @param trackCluster a TrackCluster_t struct as the input
     */
    void collectGainVariables(const PXD::TrackCluster_t& trackCluster);
    /**
     * Collect variables for efficiency monitoring
     * @param trackCluster a TrackCluster_t struct as the input
     */
    void collectEfficiencyVariables(const PXD::TrackCluster_t& trackCluster);

    /**
     * Helper function to get binID, uBin and vBin from Cluster_t struct
     * The binning is derived from PXD::PXDGainCalibrator
     * @param trackCluster a PXD::TrackCluster_t struct
     * @param uBin bin id along u-axis to be updated with binID
     * @param vBin bin id along v-axis to be updated with binID
     * @param useCluster flag to use cluster position instead of track point
     * @return unique binID
     */
    int getBinID(const PXD::TrackCluster_t& trackCluster, int& uBin, int& vBin, bool useCluster = false)
    {

      // Get PXD::TrackPoint_t
      auto const& tPoint = trackCluster.intersection;
      // Get uBin and vBin from a global point.
      VxdID sensorID = PXD::getVxdIDFromPXDModuleID(trackCluster.cluster.pxdID);
      const PXD::SensorInfo& Info = dynamic_cast<const PXD::SensorInfo&>(VXD::GeoCache::get(sensorID));
      float posU(0.), posV(0.);
      if (useCluster) {
        posU = trackCluster.cluster.posU;
        posV = trackCluster.cluster.posV;
      } else {
        auto localPoint = Info.pointToLocal(TVector3(tPoint.x, tPoint.y, tPoint.z), true);
        posU = localPoint.X();
        posV = localPoint.Y();
      }
      auto uID = Info.getUCellID(posU);
      auto vID = Info.getVCellID(posV);
      auto iSensor = VXD::GeoCache::getInstance().getGeoTools()->getPXDSensorIndex(sensorID);
      uBin = PXD::PXDGainCalibrator::getInstance().getBinU(sensorID, uID, vID, m_nBinsU);
      vBin = PXD::PXDGainCalibrator::getInstance().getBinV(sensorID, vID, m_nBinsV);
      assert(uBin < m_nBinsU && vBin < m_nBinsV);

      return iSensor * m_nBinsU * m_nBinsV + uBin * m_nBinsV + vBin;
    }

    /**
     * Helper function to select a track point according to the status of the related pixels.
     * @param sensorID VxdID
     * @param uID uID of the target pixel
     * @param vID vID of the target pixel
     * @return true if the track point/cluster will be used for efficiency study later
     */
    bool isSelected(const VxdID& sensorID, const int& uID, const int& vID)
    {
      // Check a pixel matrix around the track point. Skip counting If any pixel is dead/hot.
      // TODO: Using a dedicated cluster flag (under development).
      if (PXD::isCloseToBorder(uID, vID, m_maskedDistance) ||
          PXD::isDefectivePixelClose(uID, vID, m_maskedDistance, sensorID))
        return false;
      return true;
    }

  private:
    /** Required input EventMetaData */
    StoreObjPtr<EventMetaData> m_evtMetaData;

    /** Minimum cluster charge cut  */
    int m_minClusterCharge;
    /** Minimum cluster size cut */
    int m_minClusterSize;
    /** Maximum cluster size cut */
    int m_maxClusterSize;
    /** Distance inside which no dead pixel or module border is allowed */
    int m_maskedDistance;

    /** Number of corrections per sensor along u side */
    int m_nBinsU;
    /** Number of corrections per sensor along v side */
    int m_nBinsV;
    /** Payload name for Gain to be read from DB */
    std::string m_gainName;
    /** Flag to use cluster position rather than track point to group pixels into bins for gain calibration.*/
    bool m_useClusterPosition;
    /** Flag to fill cluster charge ratio (relative to expected MPV) histograms*/
    bool m_fillChargeRatioHistogram;
    /** Flag to fill cluster charge and its estimated MPV in TTree*/
    bool m_fillChargeTree;

    /** Event selection for efficiency validation */
    bool m_selected4Eff;

    /** (d0_p + d0_m)/sqrt(2) */
    float m_deltaD0oSqrt2;
    /** (z0_p - z0_m)/sqrt(2) */
    float m_deltaZ0oSqrt2;
    /** Cluster charge in ADU */
    int m_signal;
    /** Estimated cluster charge in ADU */
    float m_estimated;
    /** Run number to be stored in dbtree */
    int m_run;
    /** Experiment number to be stored in dbtree */
    int m_exp;
    /** GainMap to be stored in dbtree */
    PXDGainMapPar m_gainMap;

    /** Track struct for holding required variables */
    PXD::Track_t track_struct;
    /** Name of the particle list for gain calibration */
    std::string m_PList4GainName = "";
    /** Name of the particle list for efficiency study */
    std::string m_PList4EffName = "";
    /** Name of the particle list for resolution study */
    std::string m_PList4ResName = "";

  };
}
