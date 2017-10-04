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
#include <framework/dataobjects/EventMetaData.h>
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <framework/logging/Logger.h>
#include <framework/gearbox/Unit.h>

#include <vxd/geometry/GeoCache.h>

#include <pxd/dataobjects/PXDCluster.h>
#include <pxd/dataobjects/PXDTrueHit.h>

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
  setDescription("PXD Cluster Shape Calibration");
  setPropertyFlags(c_ParallelProcessingCertified);

  // Parameter definitions
  addParam("EdgeClose", m_EdgeClose,
           "Use cluster shape correction", m_EdgeClose);

  addParam("UseTracks", m_UseTracks,
           "To use track information (default) or simulations, default=True", m_UseTracks);

  addParam("UseRealData", m_UseRealData,
           "To use real data without simulations or simulations, default=False", m_UseRealData);

  addParam("CompareTruePointTracks", m_CompareTruePointTracks,
           "To compare true point and track position in simulations, default=False", m_CompareTruePointTracks);

  addParam("CalibrationKind", m_CalibrationKind,
           "1: realistic physics or real data (default), 2: special, for full range of angles", m_CalibrationKind);

  addParam("PixelKindCal", m_PixelKindCal,
           "For CalibrationKind=2 set pixel kind (pixel size) in range 1..4 (smallest to biggest), default=0", m_PixelKindCal);

  addParam("SpecialLayerNo", m_SpecialLayerNo,
           "For CalibrationKind=2 set Layer ID for special analysis, default=1", m_SpecialLayerNo);
  addParam("SpecialLadderNo", m_SpecialLadderNo,
           "For CalibrationKind=2 set Ladder ID for special analysis, default=3", m_SpecialLadderNo);
  addParam("SpecialSensorNo", m_SpecialSensorNo,
           "For CalibrationKind=2 set Sensor ID for special analysis, default=2", m_SpecialSensorNo);

  addParam("ExcludedResiduals", m_ExcludedResiduals,
           "Use excluded residuals (non-biased), default=1", m_ExcludedResiduals);


}

void pxdClusterShapeCalibrationModule::prepare()
{
  //Register collections
  StoreObjPtr<EventMetaData>::required();
  StoreArray<PXDCluster> storeClusters;

  // Data object creation --------------------------------------------------
  auto tree = new TTree("pxdCluserShapeCalibration", "pxd Cluser Shape Calibration Source Data");
  tree->Branch<int>("event", &m_evt);
  tree->Branch<int>("run", &m_run);
  tree->Branch<int>("exp", &m_exp);
  tree->Branch<int>("pid", &m_procId);
  tree->Branch<short int>("layer", &m_layer);
  tree->Branch<short int>("sensor", &m_sensor);
  tree->Branch<short int>("pixelKind", &m_pixelKind);
  tree->Branch<short int>("closeEdge", &m_closeEdge);
  tree->Branch<short>("shape", &m_shape);
  tree->Branch<float>("phiTrack", &m_phiTrack);
  tree->Branch<float>("thetaTrack", &m_thetaTrack);
  tree->Branch<float>("phiTrue", &m_phiTrue);
  tree->Branch<float>("thetaTrue", &m_thetaTrue);
  tree->Branch<double>("signal", &m_signal);
  tree->Branch<double>("seed", &m_seed);
  tree->Branch<double>("InPixUTrue", &m_InPixUTrue);
  tree->Branch<double>("InPixVTrue", &m_InPixVTrue);
  tree->Branch<double>("InPixU", &m_InPixU);
  tree->Branch<double>("InPixV", &m_InPixV);
  tree->Branch<double>("ResidUTrue", &m_ResidUTrue);
  tree->Branch<double>("ResidVTrue", &m_ResidVTrue);
  tree->Branch<double>("SigmaU", &m_SigmaU);
  tree->Branch<double>("SigmaV", &m_SigmaV);
  tree->Branch<double>("ResidUTrack", &m_ResidUTrack);
  tree->Branch<double>("ResidVTrack", &m_ResidVTrack);
  tree->Branch<double>("SigmaUTrack", &m_SigmaUTrack);
  tree->Branch<double>("SigmaVTrack", &m_SigmaVTrack);
  registerObject<TTree>("pxdCal", tree);

}

void pxdClusterShapeCalibrationModule::finish()
{

  // not need action on the end.
}

void pxdClusterShapeCalibrationModule::collect()
{
  StoreObjPtr<EventMetaData> emd;
  StoreArray<genfit::Track> tracks;

  const StoreArray<PXDCluster> storeClusters;
  VxdID sensorID(0);

  m_procId = ProcHandler::EvtProcID();
  m_evt = emd->getEvent();
  m_run = emd->getRun();
  m_exp = emd->getExperiment();

  TString text;

  if (m_UseTracks == 1) {  // using tracks, for real experiment
    for (auto track : tracks) {  // over tracks
      for (unsigned int ipoint = 0; ipoint < track.getNumPointsWithMeasurement(); ++ipoint) {  // over track points
        if (PXDRecoHit* pxdhit = dynamic_cast<PXDRecoHit*>(track.getPointWithMeasurement(ipoint)->getRawMeasurement(0))) { // cluster
          const PXDCluster& cluster = * pxdhit->getCluster();
          TVectorD state = track.getPointWithMeasurement(ipoint)->getFitterInfo()->getFittedState().getState();
          bool biased = true;
          if (m_ExcludedResiduals == 1) biased = false;
          TVectorD residual = track.getPointWithMeasurement(ipoint)->getFitterInfo()->getResidual(0, biased).getState();
          TMatrixDSym covarianceTR = track.getPointWithMeasurement(ipoint)->getFitterInfo()->getResidual(0, biased).getCov();
          // state 0=q/p, 1= track slope in local - U, 2: in V, 3: u position local, 4: v position local
          m_phiTrack = TMath::ATan2(state[1], 1.0);
          m_thetaTrack = TMath::ATan2(state[2], 1.0);
          // m_ResidUTrack = cluster.getU() - state[3];
          // m_ResidVTrack = cluster.getV() - state[4];
          m_ResidUTrack = residual.GetMatrixArray()[0];
          m_ResidVTrack = residual.GetMatrixArray()[1];
          // TMatrixDSym covariance = track.getPointWithMeasurement(ipoint)->getFitterInfo()->getFittedState().getCov();
          // m_SigmaUTrack = sqrt(covariance(3, 3));
          // m_SigmaVTrack = sqrt(covariance(4, 4));
          m_SigmaUTrack = sqrt(covarianceTR(0, 0) - cluster.getUSigma() * cluster.getUSigma());
          m_SigmaVTrack = sqrt(covarianceTR(1, 1) - cluster.getVSigma() * cluster.getVSigma());
          // printf("----->%f %f | %f %f err: %f %f - %f %f - true %f %f  -- %f %f<------------\n",m_ResidUTrack, m_ResidVTrack, residual.GetMatrixArray()[0], residual.GetMatrixArray()[1],
          //       m_SigmaUTrack, m_SigmaVTrack, sqrt(covarianceTR(0, 0)), sqrt(covarianceTR(1, 1)), cluster.getUSigma(), cluster.getVSigma(),
          //       sqrt(covarianceTR(0, 0) - cluster.getUSigma() * cluster.getUSigma()),
          //       sqrt(covarianceTR(1, 1) - cluster.getVSigma() * cluster.getVSigma()));

          //Get Geometry information
          sensorID = cluster.getSensorID();
          const PXD::SensorInfo& Info = dynamic_cast<const PXD::SensorInfo&>(VXD::GeoCache::get(
                                          sensorID));
          m_layer = Info.getID().getLayerNumber();
          m_sensor = Info.getID().getSensorNumber();

          m_SigmaU = cluster.getUSigma();
          m_SigmaV = cluster.getVSigma();
          m_signal = cluster.getCharge();
          m_seed = cluster.getSeedCharge();
          m_shape = cluster.getShape();

          text = Form("--> phiTrack[deg] %f thetaTrack %f ResidUTrack[um] %f ResidVTrack %f SigmaUTrack %f SigmaVTrack %f SigmaU %f SigmaV %f \n",
                      m_phiTrack / Unit::deg, m_thetaTrack / Unit::deg,
                      m_ResidUTrack / Unit::um, m_ResidVTrack / Unit::um,
                      m_SigmaUTrack / Unit::um, m_SigmaVTrack / Unit::um,
                      m_SigmaU / Unit::um, m_SigmaV / Unit::um
                     );
          B2DEBUG(30, text.Data());

          // TODO set it more systematically? m_closeEdge values
          int EdgePixelSizeV1 = 512;   // TODO assign correctly from geometry/design
          int EdgePixelSizeV2 = 256;   // TODO assign correctly from geometry/design
          int EdgePixelSizeU = 256;    // TODO assign correctly from geometry/design
          m_closeEdge = 0;  // 0: healthy, far from edge or masked, 1: close edge, 2: close masked pixel
          if ((m_sensor == 2) && (abs(Info.getVCellID(cluster.getV()) - EdgePixelSizeV1) < m_EdgeClose)) m_closeEdge = 1;
          else if ((m_sensor == 1) && (abs(Info.getVCellID(cluster.getV()) - EdgePixelSizeV2) < m_EdgeClose)) m_closeEdge = 1;
          else if (Info.getVCellID(cluster.getV()) < m_EdgeClose) m_closeEdge = 1;
          else if ((EdgePixelSizeV1 + EdgePixelSizeV2 - Info.getVCellID(cluster.getV())) < m_EdgeClose) m_closeEdge = 1;
          else if (Info.getUCellID(cluster.getU()) < m_EdgeClose) m_closeEdge = 1;
          else if (EdgePixelSizeU - Info.getUCellID(cluster.getU()) < m_EdgeClose) m_closeEdge = 1;

          m_pixelKind = 0;  // 0: smaller pixelsize, 1: larger pixelsize TODO set it more systematically?
          if ((m_sensor == 2) && (Info.getVCellID(cluster.getV()) < EdgePixelSizeV1)) m_pixelKind = 1;
          else if ((m_sensor == 1) && (Info.getVCellID(cluster.getV()) >= EdgePixelSizeV2)) m_pixelKind = 1;
          if (m_layer == 2) m_pixelKind += 2;
          if (m_sensor == 2) m_pixelKind += 4;
//          printf("----> Layer %i sensor %i UPitch %i VPitch %i --- Vcell %i PixKind %i\n",
//                 m_layer,m_sensor,(int)(Info.getUPitch(cluster.getU()) / Unit::um),(int)(Info.getVPitch(cluster.getV()) / Unit::um),Info.getVCellID(cluster.getV()),m_pixelKind);

          m_InPixU = (Info.getUCellPosition(Info.getUCellID(cluster.getU())) - cluster.getU()) / Info.getUPitch(cluster.getU());
          m_InPixV = (Info.getVCellPosition(Info.getVCellID(cluster.getV())) - cluster.getV()) / Info.getVPitch(cluster.getV());

          if (m_InPixU < -0.5) m_InPixU += 1.0;  // TODO: remove, this kind of correction should not use
          if (m_InPixU > 0.5) m_InPixU -= 1.0;
          if (m_InPixV < -0.5) m_InPixV += 1.0;
          if (m_InPixV > 0.5) m_InPixV -= 1.0;

          m_InPixU *= Info.getUPitch(cluster.getU()) / Unit::um;  // [um]
          m_InPixV *= Info.getVPitch(cluster.getV()) / Unit::um;  // [um]

          if (m_UseRealData == 0) {  // there exist simulation, so use true points
            for (auto truehit : cluster.getRelationsTo<PXDTrueHit>()) {  // true hit
              m_ResidUTrue = cluster.getU() - truehit.getU();
              m_ResidVTrue = cluster.getV() - truehit.getV();

              double EnExWidth = fabs(truehit.getExitW() - truehit.getEntryW()); // w-distance between entrance and end of path in sensor
              if (EnExWidth < 0.00000001) EnExWidth = 0.0075; // in cm, use in case particle is not going thrue
              m_phiTrue = TMath::ATan2(truehit.getExitU() - truehit.getEntryU(), EnExWidth);
              m_thetaTrue = TMath::ATan2(truehit.getExitV() - truehit.getEntryV(), EnExWidth);

              text = Form("--> phiTrue[deg] %f thetaTrue %f ResidUTrue[um] %f ResidVTrue %f \n",
                          m_phiTrue / Unit::deg , m_thetaTrue / Unit::deg,
                          m_ResidUTrue / Unit::um, m_ResidVTrue / Unit::um);
              B2DEBUG(30, text.Data());

              m_InPixUTrue = (Info.getUCellPosition(Info.getUCellID(truehit.getU())) - truehit.getU()) / Info.getUPitch(truehit.getU());
              m_InPixVTrue = (Info.getVCellPosition(Info.getVCellID(truehit.getV())) - truehit.getV()) / Info.getVPitch(truehit.getV());

              if (m_InPixUTrue < -0.5) m_InPixUTrue += 1.0;  // TODO: remove, this kind of correction should not use
              if (m_InPixUTrue > 0.5) m_InPixUTrue -= 1.0;
              if (m_InPixVTrue < -0.5) m_InPixVTrue += 1.0;
              if (m_InPixVTrue > 0.5) m_InPixVTrue -= 1.0;

              m_InPixUTrue *= Info.getUPitch(truehit.getU()) / Unit::um;  // [um]
              m_InPixVTrue *= Info.getVPitch(truehit.getV()) / Unit::um;  // [um]
            }  // end of true hit
          }  // end of exist simulation
          getObject<TTree>("pxdCal").Fill();
        }  // end of cluster
      }  // end of over track points
    }  // end of over tracks
  }  // end of "if" use tracks, for real experiment
  if (m_UseTracks == 0) {  // not use tracks, for simulations only
    for (const PXDCluster& cluster : storeClusters) {
      for (auto truehit : cluster.getRelationsTo<PXDTrueHit>()) {
        //Get Geometry information
        sensorID = cluster.getSensorID();
        const PXD::SensorInfo& Info = dynamic_cast<const PXD::SensorInfo&>(VXD::GeoCache::get(
                                        sensorID));
        m_layer = Info.getID().getLayerNumber();
        m_sensor = Info.getID().getSensorNumber();
        int Conin = 1;
        if (m_CalibrationKind == 2) {
          int ladder = Info.getID().getLadderNumber();
          if (m_layer != m_SpecialLayerNo) Conin = 0;
          if (ladder != m_SpecialLadderNo) Conin = 0;
          if (m_sensor != m_SpecialSensorNo) Conin = 0;
        }
        if (Conin == 0) continue;

        m_phiTrack = 0.0;    // no track information for this case
        m_thetaTrack = 0.0;
        m_ResidUTrack = 0.0;
        m_ResidVTrack = 0.0;
        m_SigmaUTrack = 0.0;
        m_SigmaVTrack = 0.0;

        m_SigmaU = cluster.getUSigma();
        m_SigmaV = cluster.getVSigma();
        m_signal = cluster.getCharge();
        m_seed = cluster.getSeedCharge();
        m_shape = cluster.getShape();

        text = Form("--> SigmaU %f SigmaV %f \n",
                    m_SigmaU / Unit::um, m_SigmaV / Unit::um
                   );
        B2DEBUG(30, text.Data());

        // TODO set it more systematically? m_closeEdge values
        int EdgePixelSizeV1 = 512;   // TODO assign correctly from geometry/design
        int EdgePixelSizeV2 = 256;   // TODO assign correctly from geometry/design
        int EdgePixelSizeU = 256;    // TODO assign correctly from geometry/design
        m_closeEdge = 0;  // 0: healthy, far from edge or masked, 1: close edge, 2: close masked pixel
        if ((m_sensor == 2) && (abs(Info.getVCellID(cluster.getV()) - EdgePixelSizeV1) < m_EdgeClose)) m_closeEdge = 1;
        else if ((m_sensor == 1) && (abs(Info.getVCellID(cluster.getV()) - EdgePixelSizeV2) < m_EdgeClose)) m_closeEdge = 1;
        else if (Info.getVCellID(cluster.getV()) < m_EdgeClose) m_closeEdge = 1;
        else if ((EdgePixelSizeV1 + EdgePixelSizeV2 - Info.getVCellID(cluster.getV())) < m_EdgeClose) m_closeEdge = 1;
        else if (Info.getUCellID(cluster.getU()) < m_EdgeClose) m_closeEdge = 1;
        else if (EdgePixelSizeU - Info.getUCellID(cluster.getU()) < m_EdgeClose) m_closeEdge = 1;

        m_pixelKind = 0;  // 0: smaller pixelsize, 1: larger pixelsize TODO set it more systematically?
        if ((m_sensor == 2) && (Info.getVCellID(cluster.getV()) < EdgePixelSizeV1)) m_pixelKind = 1;
        else if ((m_sensor == 1) && (Info.getVCellID(cluster.getV()) >= EdgePixelSizeV2)) m_pixelKind = 1;
        if (m_layer == 2) m_pixelKind += 2;
        if (m_sensor == 2) m_pixelKind += 4;

        m_InPixU = (Info.getUCellPosition(Info.getUCellID(cluster.getU())) - cluster.getU()) / Info.getUPitch(cluster.getU());
        m_InPixV = (Info.getVCellPosition(Info.getVCellID(cluster.getV())) - cluster.getV()) / Info.getVPitch(cluster.getV());

        if (m_InPixU < -0.5) m_InPixU += 1.0;  // TODO: remove, this kind of correction should not use
        if (m_InPixU > 0.5) m_InPixU -= 1.0;
        if (m_InPixV < -0.5) m_InPixV += 1.0;
        if (m_InPixV > 0.5) m_InPixV -= 1.0;

        m_InPixU *= Info.getUPitch(cluster.getU()) / Unit::um;  // [um]
        m_InPixV *= Info.getVPitch(cluster.getV()) / Unit::um;  // [um]

        for (auto truehit : cluster.getRelationsTo<PXDTrueHit>()) {  // true hit
          m_ResidUTrue = cluster.getU() - truehit.getU();
          m_ResidVTrue = cluster.getV() - truehit.getV();

          double EnExWidth = fabs(truehit.getExitW() - truehit.getEntryW()); // w-distance between entrance and end of path in sensor
          if (EnExWidth < 0.00000001) EnExWidth = 0.0075; // in cm, use in case particle is not going thrue
          m_phiTrue = TMath::ATan2(truehit.getExitU() - truehit.getEntryU(), EnExWidth);
          m_thetaTrue = TMath::ATan2(truehit.getExitV() - truehit.getEntryV(), EnExWidth);

//          B2INFO(" ------INFO---->  useClusterShape:    " << m_phiTrue / Unit::deg);
          text = Form("--> phiTrue[deg] %f thetaTrue %f ResidUTrue[um] %f ResidVTrue %f \n",
                      m_phiTrue / Unit::deg , m_thetaTrue / Unit::deg,
                      m_ResidUTrue / Unit::um, m_ResidVTrue / Unit::um);
          B2DEBUG(30, text.Data());

          m_InPixUTrue = (Info.getUCellPosition(Info.getUCellID(truehit.getU())) - truehit.getU()) / Info.getUPitch(truehit.getU());
          m_InPixVTrue = (Info.getVCellPosition(Info.getVCellID(truehit.getV())) - truehit.getV()) / Info.getVPitch(truehit.getV());

          if (m_InPixUTrue < -0.5) m_InPixUTrue += 1.0;  // TODO: remove, this kind of correction should not use
          if (m_InPixUTrue > 0.5) m_InPixUTrue -= 1.0;
          if (m_InPixVTrue < -0.5) m_InPixVTrue += 1.0;
          if (m_InPixVTrue > 0.5) m_InPixVTrue -= 1.0;

          m_InPixUTrue *= Info.getUPitch(truehit.getU()) / Unit::um;  // [um]
          m_InPixVTrue *= Info.getVPitch(truehit.getV()) / Unit::um;  // [um]
        }  // end of true hit

        getObject<TTree>("pxdCal").Fill();
      } // end of truehit
    } // end of cluster
  } // end of "if" not use tracks, for simulations only

}


