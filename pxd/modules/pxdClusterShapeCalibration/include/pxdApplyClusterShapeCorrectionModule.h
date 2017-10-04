/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Peter Kodys                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef pxdApplyClusterShapeCorrectionModule_H
#define pxdApplyClusterShapeCorrectionModule_H

#include <framework/core/Module.h>
#include <vxd/dataobjects/VxdID.h>
#include <pxd/geometry/SensorInfo.h>
#include <pxd/reconstruction/ClusterCache.h>
#include <pxd/reconstruction/ClusterProjection.h>
#include <pxd/reconstruction/NoiseMap.h>
#include <string>
#include <memory>
#include <iostream>
#include <stdio.h>
#include <TMath.h>
#include <TH1F.h>
#include <TH2F.h>

#include <calibration/CalibrationCollectorModule.h>
#include <pxd/calibration/PXDClusterShapeCalibrationAlgorithm.h>
//#include <boost/multi_array.hpp>
//#include <boost/functional/hash.hpp>

namespace genfit {
  class Track;
}

namespace Belle2 {

  namespace PXD {

    /** The pxdApplyClusterShapeCorrection module.
     *
     * This module tests whether TrueHits are accessible by following the clusters-to-truehits
     * relation.

     */
    class pxdApplyClusterShapeCorrectionModule : public Module {

    public:
      /** Constructor defining the parameters */
      pxdApplyClusterShapeCorrectionModule();
      /** Initialize the module */
      virtual void initialize();
      /** do the clustering */
      virtual void event();
      /** Terminate the module */
      virtual void terminate();

    private:
      std::string m_storeTrackName;           /**< genfit::Track StoreArray name */

      /** Exist file with basic corrections, default=0 */
      int m_ExistCorrectionBasic = 0;
      /** Exist file with corrections for PK=0, default=0 */
      int m_ExistCorrectionPK0 = 0;
      /** Exist file with corrections for PK=1, default=0 */
      int m_ExistCorrectionPK1 = 0;
      /** Exist file with corrections for PK=2, default=0 */
      int m_ExistCorrectionPK2 = 0;
      /** Exist file with corrections for PK=3, default=0 */
      int m_ExistCorrectionPK3 = 0;

      /** Map of corrections for Bias for 6 cases: 0: realistic data, 1..4: simulation for ful angle range for all pixel types, 5: combine 1..4 */
      map_typeCorrs TCorrection_BiasMap[6];
      /** Map of corrections for Error Estimation for 6 cases: 0: realistic data, 1..4: simulation for ful angle range for all pixel types, 5: combine 1..4 */
      map_typeCorrs TCorrection_ErrorEstimationMap[6];
      /** Map of corrections for Bias errors for 6 cases: 0: realistic data, 1..4: simulation for ful angle range for all pixel types, 5: combine 1..4 */
      map_typeCorrs TCorrection_BiasMapErr[6];
      /** Map of corrections for In-Pixel positions for 6 cases: 0: realistic data, 1..4: simulation for ful angle range for all pixel types, 5: combine 1..4 */
      map_typeInPics TInPixelPositionMap[6];

      /** Expert setting, production of monitoring histograms, default=False */
      Bool_t m_DoExpertHistograms = kFALSE;

      /** Expert setting, production of monitoring histograms, default=False */
      Bool_t m_ShowDetailStatistics = kFALSE;

      /** Mark of loop to save monitored data to different file, default=0 */
      int m_MarkOfLoopForHistogramsFile = 0;

      /** Expert setting, production of monitoring histograms */
      TFile* fExpertHistograms = NULL;

      /** Expert setting, monitoring histograms */
      TH1F** m_histResidualU = NULL;
      TH1F** m_histResidualV = NULL;
      TH2F** m_histResidualUV = NULL;
      TH1F** m_histNormErrorU = NULL;
      TH1F** m_histNormErrorV = NULL;
      TH2F** m_histNormErrorUV = NULL;

      /** Processed Number od clusters */
      int NClusters = 0;

      /** Processed Number od clusters used bias corrections from real data or realistic tracking simulation */
      int NClustersBasicCorBias[2] = {0, 0};

      /** Processed Number od clusters used bias corrections from simulation */
      int NClustersSimulationCorBias[2] = {0, 0};

      /** Processed Number od clusters used Err Estim corrections from real data or realistic tracking simulation */
      int NClustersBasicCorErEst[2] = {0, 0};

      /** Processed Number od clusters used Err Estim corrections from simulation */
      int NClustersSimulationCorErEst[2] = {0, 0};

      /** To use simulations rather than real data calculated corrections, default=False */
      Bool_t m_PrefereSimulation = kFALSE;

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

      /** Name of file contain basic calibration, default="pxdCalibrationBasic */
      std::string m_CalFileBasicName = "pxdCalibrationBasic";
      /** Name of file contain calibration for pixel kind 0 (PitchV=55um), default="pxdCalibrationPixelKind0 */
      std::string m_CalFilePK0Name = "pxdCalibrationPixelKind0";
      /** Name of file contain calibration for pixel kind 1 (PitchV=60um), default="pxdCalibrationPixelKind1 */
      std::string m_CalFilePK1Name = "pxdCalibrationPixelKind1";
      /** Name of file contain calibration for pixel kind 2 (PitchV=70um), default="pxdCalibrationPixelKind2 */
      std::string m_CalFilePK2Name = "pxdCalibrationPixelKind2";
      /** Name of file contain calibration for pixel kind 3 (PitchV=85um), default="pxdCalibrationPixelKind3 */
      std::string m_CalFilePK3Name = "pxdCalibrationPixelKind3";

      /** Presets for usinf of pxd cluster shape calibration - shapes */
      int m_shapes = 0;     // 1 .. 15, shapeID = 0: not setting shape
      /** Presets for usinf of pxd cluster shape calibration - pixelkinds */
      int m_pixelkinds = 0;
      /** Presets for usinf of pxd cluster shape calibration - dimensions */
      int m_dimensions = 0; // cases: 15 x 8 x 2 = 240
      /** Presets for usinf of pxd cluster shape calibration - anglesU */
      int m_anglesU = 0;    // 18 x 18 = 324 (angles)
      /** Presets for usinf of pxd cluster shape calibration - anglesV */
      int m_anglesV = 0;
      /** Presets for usinf of pxd cluster shape calibration - in_pixelU */
      int m_in_pixelU = 0;  // 7 x 7 = 49 (in-pixel positions)
      /** Presets for usinf of pxd cluster shape calibration - in_pixelV */
      int m_in_pixelV = 0;

    };//end class declaration


  } //end PXD namespace;
} // end namespace Belle2

#endif // pxdApplyClusterShapeCorrectionModule_H
