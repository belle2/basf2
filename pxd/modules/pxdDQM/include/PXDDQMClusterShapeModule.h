/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Peter Kodys                                              *
 *                                                                        *
 * Prepared for cluster shape correction quality check                    *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/core/HistoModule.h>
#include <framework/core/Module.h>
#include <framework/core/HistoModule.h>
#include <pxd/reconstruction/HitCorrector.h>
#include <pxd/geometry/SensorInfo.h>
#include <vector>
#include <TH2F.h>

namespace Belle2 {

  namespace PXD {

    /** PXD DQM Cluster Shape Module
      *
      * This module is responsible to check DQM of clibrate cluster position and
      * error estimation base on information from cluster only (shape, signal, seed),
      * or with tracking information (angle of track, in-pixel position).
      *
      * @see PXDClusterShape
      * @see PXDClusterShapeCalibration
      * @see PXDClusterShapeCalibrationAlgorithm
      * @see pxdMergeClusterShapeCorrections
      *
      */
    class PXDDQMClusterShapeModule : public HistoModule {  // <- derived from HistoModule class

    public:

      /** Constructor */
      PXDDQMClusterShapeModule();
      /* Destructor */
      virtual ~PXDDQMClusterShapeModule();

      /** Module functions */
      virtual void initialize();
      virtual void beginRun();
      virtual void event();
      virtual void endRun();
      virtual void terminate();

      /**
      * Histogram definitions such as TH1(), TH2(), TNtuple(), TTree().... are supposed
      * to be placed in this function.
      */
      virtual void defineHisto();

    private:
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
      /** Name of the collection to use for the Reco Track */
      std::string m_storeRecoTrackName;
      /** Frames StoreArray name */
      std::string m_storeFramesName;

      /** Name of file contain output merged calibration, default=PXD-ClasterShapeCorrections */
      std::string m_CalFileName = "PXD-ClasterShapeCorrections.root";

      /** flag <0,1>, create DQM of calibration in detail (log file, histograms), default = 0 */
      int m_SeeDQMOfCalibration = 0;

      /** flag <0,1>, using of real data is skip true points availabilities, default = 0 */
      int m_UseRealData = 0;

      /** flag <0,1>, using of corrections from file or from DB, default = 0 = DB */
      int m_UseCorrectionsFromFile = 0;

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

      /** Residuals of PositionOnPlaneUnBias - RecoHit */
      TH2F** m_ResidualsPosInPlUBRH;
      /** Residuals of PositionOnPlaneUnBias - Cluster */
      TH2F** m_ResidualsPosInPlUBCl;
      /** Residuals of TruePos - PositionOnPlaneUnBias */
      TH2F** m_ResidualsTruePosInPlUB;
      /** Residuals of PositionOnPlaneBias - RecoHit */
      TH2F** m_ResidualsPosInPlBRH;
      /** Residuals of PositionOnPlaneBias - Cluster */
      TH2F** m_ResidualsPosInPlBCl;
      /** Residuals of TruePos - PositionOnPlaneBias */
      TH2F** m_ResidualsTruePosInPlB;
      /** Residuals of TruePos - RecoHit */
      TH2F** m_ResidualsTrueRH;
      /** Residuals of TruePos - Cluster */
      TH2F** m_ResidualsTrueCl;
      /** Error Estimation of RecoHit */
      TH2F** m_EERH;
      /** Error Estimation of Cluster */
      TH2F** m_EECl;
      /** Error Estimation of UnBias Truck */
      TH2F** m_EETrackUB;
      /** Error Estimation of Bias Truck */
      TH2F** m_EETrackB;

      /** Ratio True Residual to Error Estimation of Cluster */
      TH2F** m_ReTrue_EECl;
      /** Ratio True Residual to Error Estimation of RecoHit */
      TH2F** m_ReTrue_EERH;
      /** Ratio PositionOnPlaneUnBias Residual to Error Estimation of Cluster */
      TH2F** m_ReUnBiasRes_EECl;
      /** Ratio PositionOnPlaneUnBias Residual to Error Estimation of RecoHit */
      TH2F** m_ReUnBiasRes_EERH;

      /** Residuals of PositionOnPlaneUnBias - RecoHit - for "L" cluster shapes */
      TH2F** m_ResidualsPosInPlUBRHShapeL;
      /** Residuals of PositionOnPlaneUnBias - Cluster - for "L" cluster shapes */
      TH2F** m_ResidualsPosInPlUBClShapeL;
      /** Residuals of TruePos - PositionOnPlaneUnBias - for "L" cluster shapes */
      TH2F** m_ResidualsTruePosInPlUBShapeL;
      /** Residuals of PositionOnPlaneBias - RecoHit - for "L" cluster shapes */
      TH2F** m_ResidualsPosInPlBRHShapeL;
      /** Residuals of PositionOnPlaneBias - Cluster - for "L" cluster shapes */
      TH2F** m_ResidualsPosInPlBClShapeL;
      /** Residuals of TruePos - PositionOnPlaneBias - for "L" cluster shapes */
      TH2F** m_ResidualsTruePosInPlBShapeL;
      /** Residuals of TruePos - RecoHit - for "L" cluster shapes */
      TH2F** m_ResidualsTrueRHShapeL;
      /** Residuals of TruePos - Cluster - for "L" cluster shapes */
      TH2F** m_ResidualsTrueClShapeL;
      /** Error Estimation of RecoHit - for "L" cluster shapes */
      TH2F** m_EERHShapeL;
      /** Error Estimation of Cluster - for "L" cluster shapes */
      TH2F** m_EEClShapeL;
      /** Error Estimation of UnBias Truck - for "L" cluster shapes */
      TH2F** m_EETrackUBShapeL;
      /** Error Estimation of Bias Truck - for "L" cluster shapes */
      TH2F** m_EETrackBShapeL;

      /** Ratio True Residual to Error Estimation of Cluster - for "L" cluster shapes */
      TH2F** m_ReTrue_EEClShapeL;
      /** Ratio True Residual to Error Estimation of RecoHit - for "L" cluster shapes */
      TH2F** m_ReTrue_EERHShapeL;
      /** Ratio PositionOnPlaneUnBias Residual to Error Estimation of Cluster - for "L" cluster shapes */
      TH2F** m_ReUnBiasRes_EEClShapeL;
      /** Ratio PositionOnPlaneUnBias Residual to Error Estimation of RecoHit - for "L" cluster shapes */
      TH2F** m_ReUnBiasRes_EERHShapeL;

    };  //end class declaration

  }  //end PXD namespace;

}  // end namespace Belle2


