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
#include <framework/dataobjects/EventMetaData.h>
#include <framework/datastore/StoreObjPtr.h>
#include <vxd/dataobjects/VxdID.h>
#include <pxd/geometry/SensorInfo.h>
#include <pxd/reconstruction/ClusterCache.h>
#include <pxd/reconstruction/ClusterProjection.h>
#include <pxd/reconstruction/NoiseMap.h>
#include <string>
#include <memory>
#include <stdio.h>

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
   *
   * @see PXDClusterShape
   * @see PXDClusterShapeCalibrationAlgorithm
   * @see pxdMergeClusterShapeCorrections
   * @see PXDDQMClusterShape
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
    StoreObjPtr<EventMetaData> m_eventMetaData; /**< Required input for EventMetaData */
    /** Region close edge where remove cluster shape corrections */
    int m_EdgeClose = 2;
    /** To use real data without simulations or simulations, default=False */
    Bool_t m_UseRealData = kFALSE;
    /** Set pixel kind (pixel size) in range -1 (no selection), 0..3 (smallest to biggest), default=-1 */
    int m_PixelKindCal = -1;
    /** Set Layer ID for special analysis, default=1 */
    int m_SpecialLayerNo = 1;
    /** Set Ladder ID for special analysis, default=3 */
    int m_SpecialLadderNo = 3;
    /** Set Sensor ID for special analysis, default=2 */
    int m_SpecialSensorNo = 2;
    /** Current event id */
    int m_evt = -1;
    /** Current run id */
    int m_run = -1;
    /** Current experiment id */
    int m_exp = -1;
    /** Current process id */
    int m_procId = -1;

    /** Collection to use for the Reco Track */
    std::string m_storeRecoTrackName;
    /** Collection to use for the PXDClusters */
    std::string m_storeClustersName;
    /** Collection to use for the PXDTrueHits */
    std::string m_storeTrueHitsName;
    /** Collection to use for the MCParticles */
    std::string m_storeMCParticlesName;
    /** Relation between PXDClusters and MCParticles */
    std::string m_relClusterMCParticleName;
    /** Relation between PXDClusters and PXDTrueHits */
    std::string m_relClusterTrueHitName;
    /** PXDTrueHits StoreArray name */
    std::string m_storePXDTrueHitsName;
    /** PXDDigits StoreArray name */
    std::string m_storePXDDigitsName;
    /** PXDClusters StoreArray name */
    std::string m_storePXDClustersName;
    /** PXDRecoHits StoreArray name */
    std::string m_storePXDRecoHitName;
    /** PXDClustersToPXDDigits RelationArray name */
    std::string m_relPXDClusterDigitName;
    /** PXDRecoHit to PXDTrueHit RelationArray name */
    std::string m_relPXDRecoHitTrueHitName;

    /** Counter of events */
    int m_iEv;
    /** Counter of clusters */
    int m_iCls;
    /** Export data for Peter Kvasnicka to PQClusters.asc */
    int m_ExportDataForPQ = 0;
    /** File for Peter Kvasnicka */
    FILE* fasc;

    /** layer ID (1, 2) */
    short m_layer = 0;
    /** sensor ID (1, 2) */
    short m_sensor = 0;
    /** pixel kind ID (0..7) */
    short m_pixelKind = 0;
    /** mark, if cluster is touch edge or masked pixel  */
    short m_closeEdge = 0;
    /** ID of Cluster Shape */
    short m_shape = 0;
    /** theta angle of track direction via sensor */
    float m_thetaTrack = 0.0;
    /** phi angle of track direction via sensor */
    float m_phiTrack = 0.0;
    /** theta angle from true hit */
    float m_thetaTrue = 0.0;
    /** phi angle from true hit */
    float m_phiTrue = 0.0;
    /** collected charge - signal */
    float m_signal = 0.0;
    /** seed - maximum of signal */
    float m_seed = 0.0;
    /** in-pixel position U from track (if possible) or simulation true hit */
    float m_InPixUTrue = 0.0;
    /** in-pixel position V from track (if possible) or simulation true hit */
    float m_InPixVTrue = 0.0;
    /** in-pixel recohit position U */
    float m_InPixUReco = 0.0;
    /** in-pixel recohit position V */
    float m_InPixVReco = 0.0;

    /** difference in U: plane - recohit position for biased track */
    float m_ResidUPlaneRHBias = 0.0;
    /** difference in V: plane - recohit position for biased track */
    float m_ResidVPlaneRHBias = 0.0;
    /** difference in U: plane - recohit position for unbiased track*/
    float m_ResidUPlaneRHUnBias = 0.0;
    /** difference in V: plane - recohit position for unbiased track */
    float m_ResidVPlaneRHUnBias = 0.0;
    /** difference in U: plane - cluster position for biased track */
    float m_ResidUPlaneClBias = 0.0;
    /** difference in V: plane - cluster position for biased track */
    float m_ResidVPlaneClBias = 0.0;
    /** difference in U: true - plane position for biased track */
    float m_ResidUTruePlaneBias = 0.0;
    /** difference in V: true - plane position for biased track */
    float m_ResidVTruePlaneBias = 0.0;
    /** difference in U: true - plane position for unbiased track */
    float m_ResidUTruePlaneUnBias = 0.0;
    /** difference in V: true - plane position for unbiased track */
    float m_ResidVTruePlaneUnBias = 0.0;
    /** difference in U: true - recohit position */
    float m_ResidUTrueRH = 0.0;
    /** difference in V: true - recohit position */
    float m_ResidVTrueRH = 0.0;
    /** difference in U: true - cluster position */
    float m_ResidUTrueCl = 0.0;
    /** difference in V: true - cluster position */
    float m_ResidVTrueCl = 0.0;
    /** error estimation covariant element 0,1 for biased track */
    float m_ErrorEstimateCovarUVBias = 0.0;
    /** error estimation covariant element 0,1 for unbiased track */
    float m_ErrorEstimateCovarUVUnBias = 0.0;
    /** estimated sigma in U from recohit */
    float m_SigmaUReco = 0.0;
    /** estimated sigma in V from recohit */
    float m_SigmaVReco = 0.0;
    /** estimated sigma in U of track position for biased track */
    float m_SigmaUTrackBias = 0.0;
    /** estimated sigma in V of track position for biased track */
    float m_SigmaVTrackBias = 0.0;
    /** estimated sigma in U of residuals for biased track */
    float m_SigmaUResBias = 0.0;
    /** estimated sigma in V of residuals for biased track */
    float m_SigmaVResBias = 0.0;
    /** estimated sigma in U of track position for unbiased track */
    float m_SigmaUTrackUnBias = 0.0;
    /** estimated sigma in V of track position for unbiased track */
    float m_SigmaVTrackUnBias = 0.0;
    /** estimated sigma in U of residuals for unbiased track */
    float m_SigmaUResUnBias = 0.0;
    /** estimated sigma in V of residuals for unbiased track */
    float m_SigmaVResUnBias = 0.0;
  };
}

#endif /* pxdClusterShapeCalibrationModule_H */

