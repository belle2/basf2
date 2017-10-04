/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Peter Kodys                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef pxdClusterShapeCalibrationModule_H
#define pxdClusterShapeCalibrationModule_H

#include <framework/core/Module.h>
#include <vxd/dataobjects/VxdID.h>
#include <pxd/geometry/SensorInfo.h>
#include <pxd/reconstruction/ClusterCache.h>
#include <pxd/reconstruction/ClusterProjection.h>
#include <pxd/reconstruction/NoiseMap.h>
#include <string>
#include <memory>

#include <calibration/CalibrationCollectorModule.h>

namespace Belle2 {

  /** The PXDClusterShapeCalibration module.
   *
   * This module is responsible to clibrate cluster position and error estimation
   * base on information from cluster only (shape, signal, seed), or with tracking
   * information (angle of track, in-pixel position).
   *
   * Output of module is source for calculation of matrix of corrections
   * for positiom and error estimation
   * colelcted on TTree root file
   * Need to add details of module after his finalyzing
   *
   * @see PXDClusterShape
   *
   */
  class pxdClusterShapeCalibrationModule : public CalibrationCollectorModule {


  public:

    /**
     * Constructor: Sets the description, the properties and the parameters of the module.
     */
    pxdClusterShapeCalibrationModule();

    /** Initialize the module */
    virtual void prepare();
    /** Extract parameters for pxd cluster shape calibration of cluster reconstruction using true position */
    virtual void collect();
    /** write file */
    virtual void finish();

  private:
    /** Region close edge where remove cluster shape corrections */
    int m_EdgeClose = 3;

    /** To use track information (default) or simulations, default=True */
    Bool_t m_UseTracks = kTRUE;

    /** To use real data without simulations or simulations, default=False */
    Bool_t m_UseRealData = kFALSE;

    /** To compare true point and track position in simulations, default=False */
    Bool_t m_CompareTruePointTracks = kFALSE;

    /** Use excluded residuals (non-biased), default=1 */
    int m_ExcludedResiduals = 1;

    /**
     * 1: standard calibration based on realistic physics or real data (default)
     * 2: special, for full range of angles for every kind of pixel
     */
    int m_CalibrationKind = 1;

    /** For CalibrationKind=2 set pixel kind (pixel size) in range 0..3 (smallest to biggest), default=0 */
    int m_PixelKindCal = 0;

    /** For CalibrationKind=2 set Layer ID for special analysis, default=1 */
    int m_SpecialLayerNo = 1;
    /** For CalibrationKind=2 set Ladder ID for special analysis, default=3 */
    int m_SpecialLadderNo = 3;
    /** For CalibrationKind=2 set Sensor ID for special analysis, default=2 */
    int m_SpecialSensorNo = 2;

    /** Current event id */
    int m_evt = -1;
    /** Current run id */
    int m_run = -1;
    /** Current experiment id */
    int m_exp = -1;
    /** Current process id */
    int m_procId = -1;

    /** Name of the collection to use for the PXDClusters */
    std::string m_storeClustersName;
    /** Name of the collection to use for the PXDTrueHits */
    std::string m_storeTrueHitsName;
    /** Name of the collection to use for the MCParticles */
    std::string m_storeMCParticlesName;
    /** Name of the relation between PXDClusters and MCParticles */
    std::string m_relClusterMCParticleName;
    /** Name of the relation between PXDClusters and PXDTrueHits */
    std::string m_relClusterTrueHitName;

    /** Name of variable for layer ID (1, 2) */
    short m_layer = 0;
    /** Name of variable for sensor ID (1, 2) */
    short m_sensor = 0;
    /** Name of variable for pixel kind ID (0..7) */
    short m_pixelKind = 0;
    /** Name of variable for mark, if cluster is touch edge or masked pixel  */
    short m_closeEdge = 0;
    /** Name of variable for ID of Cluster Shape */
    short m_shape = 0;
    /** Name of variable for theta angle of track direction via sensor */
    float m_thetaTrack = 0.0;
    /** Name of variable for phi angle of track direction via sensor */
    float m_phiTrack = 0.0;
    /** Name of variable for theta angle from true hit */
    float m_thetaTrue = 0.0;
    /** Name of variable for phi angle from true hit */
    float m_phiTrue = 0.0;
    /** Name of variable for collected charge - signal */
    double m_signal = 0.0;
    /** Name of variable for seed - maximum of signal */
    double m_seed = 0.0;
    /** Name of variable for in-pixel position U from track (if possible) or simulation true hit */
    double m_InPixUTrue = 0.0;
    /** Name of variable for in-pixel position V from track (if possible) or simulation true hit */
    double m_InPixVTrue = 0.0;
    /** Name of variable for in-pixel reco position U */
    double m_InPixU = 0.0;
    /** Name of variable for in-pixel reco position V */
    double m_InPixV = 0.0;
    /** Name of variable for difference in U: reco - true position */
    double m_ResidUTrue = 0.0;
    /** Name of variable for difference in V: reco - true position */
    double m_ResidVTrue = 0.0;
    /** Name of variable for estimated sigma in U from reco position */
    double m_SigmaU = 0.0;
    /** Name of variable for estimated sigma in V from reco position */
    double m_SigmaV = 0.0;
    /** Name of variable for defference in U: reco - track position */
    double m_ResidUTrack = 0.0;
    /** Name of variable for defference in V: reco - track position */
    double m_ResidVTrack = 0.0;
    /** Name of variable for estimated sigma in U of track position */
    double m_SigmaUTrack = 0.0;
    /** Name of variable for estimated sigma in V of track position */
    double m_SigmaVTrack = 0.0;

  };
}

#endif /* pxdClusterShapeCalibrationModule_H */

