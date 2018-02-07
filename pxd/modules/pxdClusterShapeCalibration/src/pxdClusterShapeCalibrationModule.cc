/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Peter Kodys                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <pxd/modules/pxdClusterShapeCalibration/pxdClusterShapeCalibrationModule.h>

#include <framework/pcore/ProcHandler.h>
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <pxd/dataobjects/PXDFrame.h>
#include <framework/datastore/RelationArray.h>
#include <framework/logging/Logger.h>
#include <framework/gearbox/Unit.h>

#include <vxd/geometry/GeoCache.h>

#include <pxd/dataobjects/PXDCluster.h>
#include <pxd/dataobjects/PXDTrueHit.h>
#include <pxd/reconstruction/HitCorrector.h>

//#include <pxd/reconstruction/PXDClusterShape.h>

#include <genfit/Track.h>
#include <genfit/TrackCand.h>
#include <genfit/RKTrackRep.h>
#include <genfit/AbsKalmanFitter.h>
#include <genfit/KalmanFitter.h>
#include <genfit/KalmanFitterRefTrack.h>
#include <genfit/KalmanFitStatus.h>
#include <genfit/KalmanFitterInfo.h>
#include <genfit/DAF.h>
#include <genfit/Exception.h>
#include <genfit/MeasurementFactory.h>

#include <pxd/reconstruction/PXDRecoHit.h>
#include <svd/reconstruction/SVDRecoHit.h>
#include <svd/reconstruction/SVDRecoHit2D.h>

#include <tracking/gfbfield/GFGeant4Field.h>
#include <genfit/FieldManager.h>
#include <genfit/MaterialEffects.h>
#include <genfit/TGeoMaterialInterface.h>
#include <genfit/MeasurementFactory.h>

#include <tracking/dataobjects/RecoTrack.h>

#include <tracking/trackFitting/fitter/base/TrackFitter.h>

#include <TTree.h>
#include <TMath.h>

using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(pxdClusterShapeCalibration)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

pxdClusterShapeCalibrationModule::pxdClusterShapeCalibrationModule() : CalibrationCollectorModule()
{
  // Set module properties
  setDescription("PXD Cluster Shape Calibration - Preparation of data for corrections. "
                 "Basic steps for using of corrections are: "
                 " - pxdClusterShapeCalibration - create source for correction calculation <--- this module "
                 " - PXDClusterShapeCalibrationAlgorithm - calculate corrections "
                 " - pxdMergeClusterShapeCorrections - merge results and create DB object "
                 " - actualization of global condition DB "
                 " - PXDDQMClusterShape - check results on DQM histograms "
                 " "
                 " For this run stearing files: "
                 " - pxd/examples/PXDClasterShape_PrepareSources.py (5x) "
                 " - pxd/examples/PXDClasterShape_CalculateCorrections.py (5x) "
                 " - pxd/examples/PXDClasterShape_SourceMergator.py "
                 " - vxd/examples/DQMPXDClusterShape.py "
                 " "
                 "Detail description of precedure is on PXDClusterShapeCalibrationAlgorithmModule "
                );

  setPropertyFlags(c_ParallelProcessingCertified);

  // Parameter definitions
  addParam("EdgeClose", m_EdgeClose,
           "Use cluster shape correction", m_EdgeClose);
  addParam("UseRealData", m_UseRealData,
           "To use real data without simulations or simulations, default=False", m_UseRealData);
  addParam("PixelKindCal", m_PixelKindCal,
           "Set pixel kind (pixel size) in range -1 (no selection) 0..3 (smallest to biggest), default=-1", m_PixelKindCal);
  addParam("SpecialLayerNo", m_SpecialLayerNo,
           "Set Layer ID for special analysis, default=1", m_SpecialLayerNo);
  addParam("SpecialLadderNo", m_SpecialLadderNo,
           "Set Ladder ID for special analysis, default=3", m_SpecialLadderNo);
  addParam("SpecialSensorNo", m_SpecialSensorNo,
           "Set Sensor ID for special analysis, default=2", m_SpecialSensorNo);
}

void pxdClusterShapeCalibrationModule::prepare()
{
  //Register collections
  m_eventMetaData.isRequired();
  StoreArray<PXDCluster> storeClusters;
  StoreArray<RecoTrack> recotracks(m_storeRecoTrackName);
  m_storeRecoTrackName = recotracks.getName();

  // Data object creation --------------------------------------------------
  auto m_tree = new TTree("pxdCluserShapeCalibration", "pxd Cluser Shape Calibration Source Data");
  m_tree = new TTree("pxdCluserShapeCalibration", "pxd Cluser Shape Calibration Source Data");
  m_tree->Branch<int>("event", &m_evt);
  m_tree->Branch<int>("run", &m_run);
  m_tree->Branch<int>("exp", &m_exp);
  m_tree->Branch<int>("pid", &m_procId);
  m_tree->Branch<short int>("layer", &m_layer);
  m_tree->Branch<short int>("sensor", &m_sensor);
  m_tree->Branch<short int>("pixelKind", &m_pixelKind);
  m_tree->Branch<short int>("closeEdge", &m_closeEdge);
  m_tree->Branch<short>("shape", &m_shape);
  m_tree->Branch<float>("phiTrue", &m_phiTrue);
  m_tree->Branch<float>("thetaTrue", &m_thetaTrue);
  m_tree->Branch<float>("signal", &m_signal);
  m_tree->Branch<float>("seed", &m_seed);
  m_tree->Branch<float>("InPixUTrue", &m_InPixUTrue);
  m_tree->Branch<float>("InPixVTrue", &m_InPixVTrue);
  m_tree->Branch<float>("InPixUReco", &m_InPixUReco);
  m_tree->Branch<float>("InPixVReco", &m_InPixVReco);
  m_tree->Branch<float>("ResidUTrueCl", &m_ResidUTrueCl);
  m_tree->Branch<float>("ResidVTrueCl", &m_ResidVTrueCl);
  m_tree->Branch<float>("SigmaUReco", &m_SigmaUReco);
  m_tree->Branch<float>("SigmaVReco", &m_SigmaVReco);
  if (m_PixelKindCal < 0) {  // only tracking used
    m_tree->Branch<float>("phiTrack", &m_phiTrack);
    m_tree->Branch<float>("thetaTrack", &m_thetaTrack);
    m_tree->Branch<float>("ResidUPlaneRHBias", &m_ResidUPlaneRHBias);
    m_tree->Branch<float>("ResidVPlaneRHBias", &m_ResidVPlaneRHBias);
    m_tree->Branch<float>("ResidUPlaneRHUnBias", &m_ResidUPlaneRHUnBias);
    m_tree->Branch<float>("ResidVPlaneRHUnBias", &m_ResidVPlaneRHUnBias);
    m_tree->Branch<float>("ResidUPlaneClBias", &m_ResidUPlaneClBias);
    m_tree->Branch<float>("ResidVPlaneClBias", &m_ResidVPlaneClBias);
    m_tree->Branch<float>("ResidUTruePlaneBias", &m_ResidUTruePlaneBias);
    m_tree->Branch<float>("ResidVTruePlaneBias", &m_ResidVTruePlaneBias);
    m_tree->Branch<float>("ResidUTruePlaneUnBias", &m_ResidUTruePlaneUnBias);
    m_tree->Branch<float>("ResidVTruePlaneUnBias", &m_ResidVTruePlaneUnBias);
    m_tree->Branch<float>("ResidUTrueRH", &m_ResidUTrueRH);
    m_tree->Branch<float>("ResidVTrueRH", &m_ResidVTrueRH);
    m_tree->Branch<float>("SigmaUTrackBias", &m_SigmaUTrackBias);
    m_tree->Branch<float>("SigmaVTrackBias", &m_SigmaVTrackBias);
    m_tree->Branch<float>("SigmaUTrackUnBias", &m_SigmaUTrackUnBias);
    m_tree->Branch<float>("SigmaVTrackUnBias", &m_SigmaVTrackUnBias);
    m_tree->Branch<float>("SigmaUResBias", &m_SigmaUResBias);
    m_tree->Branch<float>("SigmaVResBias", &m_SigmaVResBias);
    m_tree->Branch<float>("SigmaUResUnBias", &m_SigmaUResUnBias);
    m_tree->Branch<float>("SigmaVResUnBias", &m_SigmaVResUnBias);
    m_tree->Branch<float>("ErrorEstimateCovarUVBias", &m_ErrorEstimateCovarUVBias);
    m_tree->Branch<float>("ErrorEstimateCovarUVUnBias", &m_ErrorEstimateCovarUVUnBias);
  }
  m_iCls = 0;
  m_iEv = 0;
  registerObject<TTree>("pxdCal", m_tree);
  B2INFO("START: Processed events: " << m_iEv);
  if (m_ExportDataForPQ)
    fasc = fopen("PQClusters.asc", "w");
}

void pxdClusterShapeCalibrationModule::finish()
{
  if (m_ExportDataForPQ)
    fclose(fasc);
  B2INFO("Processed events: " << m_iEv << " and clusters: " << m_iCls);

  // not need other action on the end.
}

void pxdClusterShapeCalibrationModule::collect()
{
  StoreObjPtr<EventMetaData> emd;
  const StoreArray<PXDTrueHit> storePXDTrueHit(m_storePXDTrueHitsName);
  const StoreArray<PXDDigit> storePXDDigits(m_storePXDDigitsName);
  const StoreArray<PXDCluster> storePXDClusters(m_storePXDClustersName);
  const RelationArray relPXDClusterDigits(storePXDClusters, storePXDDigits, m_relPXDClusterDigitName);
  const StoreArray<PXDFrame> storeFrames(m_storeFramesName);
  StoreArray<RecoTrack> recotracks(m_storeRecoTrackName);
  const StoreArray<PXDRecoHit> pxdrecohit(m_storePXDRecoHitName);

  // If there are no digits, leave
  if (!storePXDDigits || !storePXDDigits.getEntries()) return;

  m_iEv++;
  // Use GeoCache for sensor parameters and transforms
  VXD::GeoCache& geoCache = VXD::GeoCache::getInstance();
  TVector3 pos;
  TVector3 mom;
  TMatrixDSym cov;

  m_procId = ProcHandler::EvtProcID();
  m_evt = emd->getEvent();
  m_run = emd->getRun();
  m_exp = emd->getExperiment();

  if (m_PixelKindCal == -1) {  // using tracks, for real experiment
    for (auto& recoTrack : recotracks) {  // over recotracks
      if (!recoTrack.wasFitSuccessful())
        continue;
      if (!recoTrack.getTrackFitStatus())
        continue;
      if (!recoTrack.hasPXDHits())
        continue;
      if ((recoTrack.getNumberOfPXDHits() != 2) || (recoTrack.getNumberOfSVDHits() != 8)) // only good quality of tracks are used
        continue;
      int iHit = 0;
      for (auto recoHitInfo : recoTrack.getRecoHitInformations()) {  // over recohits
        if (!recoHitInfo) {
          B2DEBUG(200, "No genfit::pxd recoHitInfo is missing.");
          continue;
        }
        if (!recoHitInfo->useInFit())
          continue;
        if (recoHitInfo->getTrackingDetector() != RecoHitInformation::c_PXD)
          continue;
        // OK so we have RecoHitInformation related to PXD.
        // First recover the RecoHit.
        const PXDRecoHit* recoHit;
        try {
          recoHit = dynamic_cast<const PXDRecoHit*>(recoTrack.getCreatedTrackPoint(recoHitInfo)->getRawMeasurement());
        } catch (genfit::Exception) {
          break;
        }
        VxdID sensorID = recoHit->getSensorID();
        auto info = dynamic_cast<const PXD::SensorInfo&>(geoCache.get(sensorID));

        if (sensorID.getLayerNumber() > 2)
          continue;
        m_layer = info.getID().getLayerNumber();
        m_sensor = info.getID().getSensorNumber();
        // Next, recover biased and unbiased position of track on the sensor from recoTrack and GenfitTrack information.
        auto& genfitTrack = RecoTrackGenfitAccess::getGenfitTrack(recoTrack);
        auto cluster = recoHit->getCluster();
        float u_cl = cluster->getU();
        float v_cl = cluster->getV();
        auto state = recoTrack.getMeasuredStateOnPlaneFromRecoHit(recoHitInfo);
        // RecoHit obtaining, in first iteration should contain the same position information as cluster.
        auto measurementOnPlane = recoHit->constructMeasurementsOnPlane(state).at(0);
        state.getPosMomCov(pos, mom, cov);
        auto mstate = measurementOnPlane->getState();
        float u_rh = 0;
        float v_rh = 0;
        u_rh = mstate(0);
        v_rh = mstate(1);
        // Convert pos and mom to local coordinates
        TVector3 localPos = info.pointToLocal(pos);
        float u = localPos.X();
        float v = localPos.Y();
        TVector3 localMom = info.vectorToLocal(mom);
        float tu = localMom.X() / localMom.Z();
        float tv = localMom.Y() / localMom.Z();
        m_phiTrack = TMath::ATan2(tu, 1.0);
        m_thetaTrack = TMath::ATan2(tv, 1.0);
        // START do correcetion on cllluster repeat recohit steps:

        if (cluster && PXD::HitCorrector::getInstance().isInitialized()) {
          PXDCluster correctedCluster(*cluster);
          correctedCluster = PXD::HitCorrector::getInstance().correctCluster(correctedCluster, tu, tv);
          m_SigmaUReco = correctedCluster.getUSigma();
          m_SigmaVReco = correctedCluster.getVSigma();
          B2DEBUG(130, "Correction bias: \n   u: cl " << cluster->getU() << " clcor " << correctedCluster.getU() <<
                  "\n   v: cl " << cluster->getV() << " clcor " << correctedCluster.getV() <<
                  "Correction EE: \n   u: cl " << cluster->getUSigma() << " clcor " << m_SigmaUReco <<
                  "\n   v: cl " << cluster->getVSigma() << " clcor " << m_SigmaVReco);
        } else {
          m_SigmaUReco = cluster->getUSigma();
          m_SigmaVReco = cluster->getVSigma();
        }
        // END do correcetion on cllluster repeat recohit steps

        // Biased position of track on the sensor and related information in local coordinates.
        bool biased = true;
        TVectorD resBias = genfitTrack.getPointWithMeasurement(iHit)->getFitterInfo()->getResidual(0, biased).getState();
        TMatrixDSym covBias = genfitTrack.getPointWithMeasurement(iHit)->getFitterInfo()->getResidual(0, biased).getCov();
        const TVectorD positBias = genfitTrack.getPointWithMeasurement(iHit)->getFitterInfo()->getFittedState(biased).getState();
        double u_bias = positBias(3);
        double v_bias = positBias(4);
        m_ResidUPlaneRHBias = resBias.GetMatrixArray()[0];
        m_ResidVPlaneRHBias = resBias.GetMatrixArray()[1];
        m_ResidUPlaneClBias = m_ResidUPlaneRHBias + (cluster->getU() - recoHit->getU());
        m_ResidVPlaneClBias = m_ResidVPlaneRHBias + (cluster->getV() - recoHit->getV());
        m_SigmaUTrackBias = sqrt(covBias(0, 0));
        m_SigmaVTrackBias = sqrt(covBias(1, 1));
        m_SigmaUResBias = 0;

        if (m_SigmaUReco > m_SigmaUTrackBias) {
          m_SigmaUResBias = sqrt(m_SigmaUReco * m_SigmaUReco - m_SigmaUTrackBias * m_SigmaUTrackBias);
        }
        m_SigmaVResBias = 0;
        if (m_SigmaVReco > m_SigmaVTrackBias) {
          m_SigmaVResBias = sqrt(m_SigmaVReco * m_SigmaVReco - m_SigmaVTrackBias * m_SigmaVTrackBias);
        }
        m_ErrorEstimateCovarUVBias = covBias(0, 1);
        // Unbiased position of track on the sensor and related information in local coordinates.
        biased = false;
        TVectorD resUnBias = genfitTrack.getPointWithMeasurement(iHit)->getFitterInfo()->getResidual(0, biased).getState();
        TMatrixDSym covUnBias = genfitTrack.getPointWithMeasurement(iHit)->getFitterInfo()->getResidual(0, biased).getCov();
        const TVectorD positUnBias = genfitTrack.getPointWithMeasurement(iHit)->getFitterInfo()->getFittedState(biased).getState();
        double u_unbias = positUnBias(3);
        double v_unbias = positUnBias(4);
        m_ResidUPlaneRHUnBias = resUnBias.GetMatrixArray()[0];
        m_ResidVPlaneRHUnBias = resUnBias.GetMatrixArray()[1];
        m_SigmaUTrackUnBias = sqrt(covUnBias(0, 0));
        m_SigmaVTrackUnBias = sqrt(covUnBias(1, 1));
        m_SigmaUResUnBias = sqrt(m_SigmaUReco * m_SigmaUReco + m_SigmaUTrackUnBias * m_SigmaUTrackUnBias);
        m_SigmaVResUnBias = sqrt(m_SigmaVReco * m_SigmaVReco + m_SigmaVTrackUnBias * m_SigmaVTrackUnBias);
        m_ErrorEstimateCovarUVUnBias = covUnBias(0, 1);

        TString strDebugInfo = Form("  Point----->%i<------------\n", iHit);
        B2DEBUG(130, strDebugInfo.Data());
        strDebugInfo = Form("Bias[um]->R %5.1f %5.1f err: T %5.1f %5.1f = M %5.1f %5.1f - R %5.1f %5.1f (C%5.1f %5.1f C01 %5.1f)\n",
                            m_ResidUPlaneRHBias * 10000, m_ResidVPlaneClBias * 10000,
                            m_SigmaUTrackBias * 10000, m_SigmaVTrackBias * 10000,
                            m_SigmaUReco * 10000, m_SigmaVReco * 10000,
                            m_SigmaUResBias * 10000, m_SigmaVResBias * 10000,
                            sqrt(covBias(0, 0)) * 10000, sqrt(covBias(1, 1)) * 10000, covBias(0, 1) * 10000 * 10000);
        B2DEBUG(130, strDebugInfo.Data());
        strDebugInfo = Form("UnBi[um]->R %5.1f %5.1f err: T %5.1f %5.1f = R %5.1f %5.1f - M %5.1f %5.1f (C01: %5.1f)\n",
                            m_ResidUPlaneRHUnBias * 10000, m_ResidVPlaneRHUnBias * 10000,
                            m_SigmaUTrackUnBias * 10000, m_SigmaVTrackUnBias * 10000,
                            m_SigmaUResUnBias * 10000, m_SigmaVResUnBias * 10000,
                            m_SigmaUReco * 10000, m_SigmaVReco * 10000, covUnBias(0, 1) * 10000 * 10000);
        B2DEBUG(130, strDebugInfo.Data());
        iHit++;

        m_signal = cluster->getCharge();
        m_seed = cluster->getSeedCharge();
        // And finally TrueHit:
        m_pixelKind = info.getPixelKind(sensorID, cluster->getV());
        m_shape = cluster->getShape();
        if (m_UseRealData == 0) {  // there exist simulation, so use true points
          PXDTrueHit* truehit = cluster->getRelated<PXDTrueHit>();
          float u_th = truehit->getU();
          float v_th = truehit->getV();

          mom = truehit->getMomentum();  // information in local coordinates
          m_phiTrue = TMath::ATan2(mom.X(), mom.Z());
          m_thetaTrue = TMath::ATan2(mom.Y(), mom.Z());
          m_ResidUTruePlaneBias = u_th - u_bias;
          m_ResidVTruePlaneBias = v_th - v_bias;
          m_ResidUTruePlaneUnBias = u_th - u_unbias;
          m_ResidVTruePlaneUnBias = v_th - v_unbias;
          m_ResidUTrueRH = u_th - u_rh;
          m_ResidVTrueRH = v_th - v_rh;
          m_ResidUTrueCl = u_th - u_cl;
          m_ResidVTrueCl = v_th - v_cl;

          m_InPixUReco = (cluster->getU() - info.getUCellPosition(info.getUCellID(cluster->getU()))) / info.getUPitch(cluster->getU());
          m_InPixVReco = (cluster->getV() - info.getVCellPosition(info.getVCellID(cluster->getV()))) / info.getVPitch(cluster->getV());

          m_InPixUTrue = (truehit->getU() - info.getUCellPosition(info.getUCellID(truehit->getU()))) / info.getUPitch(truehit->getU());
          m_InPixVTrue = (truehit->getV() - info.getVCellPosition(info.getVCellID(truehit->getV()))) / info.getVPitch(truehit->getV());
          B2DEBUG(130, "sensor " << sensorID.getLayerNumber() <<
                  "_" << sensorID.getLadderNumber() << "_" << sensorID.getSensorNumber() <<
                  ", PixKind " << m_pixelKind <<
                  ", u/u_rh/u_cl/u_th " << u << "/" << u_rh << "/" << u_cl << "/" << u_th <<
                  ", v/v_rh/v_cl/v_th " << v << "/" << v_rh << "/" << v_cl << "/" << v_th);
        } else {
          m_phiTrue = 0;
          m_thetaTrue = 0;
          m_ResidUTruePlaneBias = 0.0;
          m_ResidVTruePlaneBias = 0.0;
          m_ResidUTruePlaneUnBias = 0.0;
          m_ResidVTruePlaneUnBias = 0.0;
          m_ResidUTrueRH = 0.0;
          m_ResidVTrueRH = 0.0;
          m_ResidUTrueCl = 0.0;
          m_ResidVTrueCl = 0.0;
          m_InPixUTrue = 0.0;
          m_InPixVTrue = 0.0;
          m_InPixUReco = 0.0;
          m_InPixVReco = 0.0;
        }

        const int EdgePixelSizeV1 = info.getVCells2();
        const int EdgePixelSizeV2 = info.getVCells() - EdgePixelSizeV1;
        const int EdgePixelSizeU = info.getUCells();
        m_closeEdge = 0;  // 0: healthy, far from edge or masked, 1: close edge, 2: close masked pixel
        if ((m_sensor == 2) && (abs(info.getVCellID(cluster->getV()) - EdgePixelSizeV1) < m_EdgeClose)) m_closeEdge = 1;
        else if ((m_sensor == 1) && (abs(info.getVCellID(cluster->getV()) - EdgePixelSizeV2) < m_EdgeClose)) m_closeEdge = 1;
        else if (info.getVCellID(cluster->getV()) < m_EdgeClose) m_closeEdge = 1;
        else if ((EdgePixelSizeV1 + EdgePixelSizeV2 - info.getVCellID(cluster->getV())) < m_EdgeClose) m_closeEdge = 1;
        else if (info.getUCellID(cluster->getU()) < m_EdgeClose) m_closeEdge = 1;
        else if (EdgePixelSizeU - info.getUCellID(cluster->getU()) < m_EdgeClose) m_closeEdge = 1;

        strDebugInfo = Form("--->Cov: %f %f %f %f %f %f SigmReco %f %f\n",
                            sqrt(cov(0, 0)) * 10000, sqrt(cov(1, 1)) * 10000, sqrt(cov(2, 2)) * 10000, cov(3, 3), cov(4, 4), cov(5, 5),
                            m_SigmaUReco * 10000, m_SigmaVReco * 10000);
        B2DEBUG(130, strDebugInfo.Data());
        strDebugInfo = Form("--->nediagCov: %f %f %f %f %f %f \n",
                            cov(0, 1) * 10000, cov(1, 0) * 10000, cov(0, 2) * 10000,
                            cov(2, 0) * 10000, cov(1, 2) * 10000, cov(2, 1) * 10000);
        B2DEBUG(130, strDebugInfo.Data());
        strDebugInfo = Form("    --->PlRH %f %f TrPl %f %f TrRH %f %f TrCl %f %f \n",
                            m_ResidUPlaneClBias * 10000, m_ResidVPlaneClBias * 10000, m_ResidUTruePlaneBias * 10000, m_ResidVTruePlaneBias * 10000,
                            m_ResidUTrueRH * 10000, m_ResidVTrueRH * 10000, m_ResidUTrueCl * 10000, m_ResidVTrueCl * 10000);
        B2DEBUG(200, strDebugInfo.Data());

        getObjectPtr<TTree>("pxdCal")->Fill();
        if (m_iCls % 1000 == 0) {
          getObjectPtr<TTree>("pxdCal")->FlushBaskets();
          getObjectPtr<TTree>("pxdCal")->AutoSave();
        }
        m_iCls++;
      }
    }
  }

  VxdID sensorID(0);
  if (m_PixelKindCal >= 0) {  // only selected pixelsize is used and use only truehits
    int iCls = 0;
    for (const PXDCluster& cluster : storePXDClusters) {
      int iTr = 0;
      for (auto truehit : cluster.getRelationsTo<PXDTrueHit>()) {
        //Get Geometry information
        sensorID = cluster.getSensorID();
        const PXD::SensorInfo& info = dynamic_cast<const PXD::SensorInfo&>(VXD::GeoCache::get(
                                        sensorID));
        m_layer = info.getID().getLayerNumber();
        m_sensor = info.getID().getSensorNumber();
        int ladder = info.getID().getLadderNumber();
        if (m_layer != m_SpecialLayerNo) continue;
        if (ladder != m_SpecialLadderNo) continue;
        if (m_sensor != m_SpecialSensorNo) continue;
        double EnExWidth = fabs(truehit.getExitW() - truehit.getEntryW()); // w-distance between entrance and end of path in sensor
        if (EnExWidth < 0.00000001) continue; // in cm, use in case particle is not going thrue

        m_phiTrack = 0.0;    // no track information for this case
        m_thetaTrack = 0.0;
        m_ResidUPlaneRHBias = 0.0;
        m_ResidVPlaneRHBias = 0.0;
        m_ResidUPlaneRHUnBias = 0.0;
        m_ResidVPlaneRHUnBias = 0.0;
        m_ResidUPlaneClBias = 0.0;
        m_ResidVPlaneClBias = 0.0;
        m_ResidUTrueRH = 0.0;
        m_ResidVTrueRH = 0.0;
        m_ResidUTruePlaneBias = 0.0;
        m_ResidVTruePlaneBias = 0.0;
        m_ResidUTruePlaneUnBias = 0.0;
        m_ResidVTruePlaneUnBias = 0.0;
        m_SigmaUTrackBias = 0.0;
        m_SigmaVTrackBias = 0.0;
        m_SigmaUTrackUnBias = 0.0;
        m_SigmaVTrackUnBias = 0.0;
        m_SigmaUResBias = 0.0;
        m_SigmaVResBias = 0.0;
        m_SigmaUResUnBias = 0.0;
        m_SigmaVResUnBias = 0.0;
        m_ErrorEstimateCovarUVUnBias = 0.0;
        m_ErrorEstimateCovarUVBias = 0.0;

        m_SigmaUReco = cluster.getUSigma();
        m_SigmaVReco = cluster.getVSigma();
        m_signal = cluster.getCharge();
        m_seed = cluster.getSeedCharge();
        m_shape = cluster.getShape();
        m_pixelKind = info.getPixelKind(sensorID, cluster.getV());
        m_closeEdge = 0;  // 0: healthy, far from edge or masked, 1: close edge, 2: close masked pixel

        m_phiTrue = TMath::ATan2(truehit.getExitU() - truehit.getEntryU(), EnExWidth);
        m_thetaTrue = TMath::ATan2(truehit.getExitV() - truehit.getEntryV(), EnExWidth);

        // START Special part to produce data for Peter Kvasnicka
        if (m_ExportDataForPQ) {
          int StorePixels[25];
          for (int i = 0; i < 25; i++) StorePixels[i] = 0;
          int iDig = 0;
          float trU = truehit.getU();
          float trV = truehit.getV();
          double uPitch = info.getUPitch();
          double vPitch = info.getVPitch(trV);
          int iTrU = info.getUCellID(trU);
          int iTrV = info.getVCellID(trV);
          double trUmid = info.getUCellPosition(iTrU);
          double trVmid = info.getVCellPosition(iTrV);
          float trUNorm = (trU - trUmid) / uPitch;
          float trVNorm = (trV - trVmid) / vPitch;
          for (auto digit1 : cluster.getRelationsTo<PXDDigit>()) {
            float signal = digit1.getCharge();
            short int idigU = digit1.getUCellID();
            short int idigV = digit1.getVCellID();
            int ind1 = 2 - idigU + iTrU;
            int ind2 = 2 - idigV + iTrV;
            if ((ind1 < 0) || (ind1 > 4)) continue;
            if ((ind2 < 0) || (ind2 > 4)) continue;
            StorePixels[ind1 * 5 + ind2] = (int)signal;
            iDig++;
          }
          iTr++;

          TString out = Form("ev   tr cl phi     theta   u       v       Pixely: u0v0 u0v1 u0v2 ... u4v4");
          if (m_iEv <= 1) {
            fprintf(fasc, "%s\n", out.Data());
          }
          out = Form("%4i %2i %2i %7.4f %7.4f %7.4f %7.4f ",
                     m_iEv, iTr, iCls, m_phiTrue, m_thetaTrue, trUNorm, trVNorm
                    );
          fprintf(fasc, "%s", out.Data());
          for (int i = 0; i < 25; i++) {
            out = Form("%3i ", StorePixels[i]);
            fprintf(fasc, "%s", out.Data());
          }
          fprintf(fasc, "\n");
        }
        // END Special part to produce data for Peter Kvasnicka

        m_ResidUTrueCl = truehit.getU() - cluster.getU();
        m_ResidVTrueCl = truehit.getV() - cluster.getV();

        m_InPixUReco = (cluster.getU() - info.getUCellPosition(info.getUCellID(cluster.getU()))) / info.getUPitch(cluster.getU());
        m_InPixVReco = (cluster.getV() - info.getVCellPosition(info.getVCellID(cluster.getV()))) / info.getVPitch(cluster.getV());

        m_InPixUTrue = (truehit.getU() - info.getUCellPosition(info.getUCellID(truehit.getU()))) / info.getUPitch(truehit.getU());
        m_InPixVTrue = (truehit.getV() - info.getVCellPosition(info.getVCellID(truehit.getV()))) / info.getVPitch(truehit.getV());

        getObjectPtr<TTree>("pxdCal")->Fill();
        if (m_iCls % 1000 == 0) {
          getObjectPtr<TTree>("pxdCal")->FlushBaskets();
          getObjectPtr<TTree>("pxdCal")->AutoSave();
        }
        m_iCls++;
      } // end of truehit
      iCls++;
    } // end of cluster
  }
}


