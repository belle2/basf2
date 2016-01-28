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

#include <vxd/geometry/GeoCache.h>

#include <pxd/dataobjects/PXDCluster.h>
#include <pxd/dataobjects/PXDTrueHit.h>

//#include <pxd/reconstruction/PXDClusterShape.h>

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

  addParam("CalibrationKind", m_CalibrationKind,
           "1: realistic physics or real data (default), 2: special, for full range of angles", m_CalibrationKind);

  addParam("PixelKind", m_PixelKind,
           "For CalibrationKind=2 set pixel kind (pixel size) in range 1..4, default=1", m_PixelKind);

  addParam("SpecialLayerNo", m_SpecialLayerNo,
           "For CalibrationKind=2 set Layer ID for special analysis, default=1", m_SpecialLayerNo);
  addParam("SpecialLadderNo", m_SpecialLadderNo,
           "For CalibrationKind=2 set Ladder ID for special analysis, default=3", m_SpecialLadderNo);
  addParam("SpecialSensorNo", m_SpecialSensorNo,
           "For CalibrationKind=2 set Sensor ID for special analysis, default=2", m_SpecialSensorNo);

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
  tree->Branch<short int>("segment", &m_segment);
  tree->Branch<short int>("pixelKind", &m_pixelKind);
  tree->Branch<short int>("closeEdge", &m_closeEdge);
  tree->Branch<short>("shape", &m_shape);
  tree->Branch<float>("theta", &m_theta);
  tree->Branch<float>("phi", &m_phi);
  tree->Branch<double>("signal", &m_signal);
  tree->Branch<double>("seed", &m_seed);
  tree->Branch<double>("InPixUTrue", &m_InPixUTrue);
  tree->Branch<double>("InPixVTrue", &m_InPixVTrue);
  tree->Branch<double>("InPixUReco", &m_InPixUReco);
  tree->Branch<double>("InPixVReco", &m_InPixVReco);
  tree->Branch<double>("CorrU", &m_CorrU);
  tree->Branch<double>("CorrV", &m_CorrV);
  registerObject<TTree>("pxdCal", tree);

}

void pxdClusterShapeCalibrationModule::collect()
{
  StoreObjPtr<EventMetaData> emd;

  const StoreArray<PXDCluster> storeClusters;
  VxdID sensorID(0);

  m_procId = ProcHandler::EvtProcID();
  m_evt = emd->getEvent();
  m_run = emd->getRun();
  m_exp = emd->getExperiment();

  for (const PXDCluster& cluster : storeClusters) {
    if (m_UseTracks == 1) {  // using tracks, for real getExperiment
      // TODO need to write...
      printf("Nothing is doing in pxdClusterShapeCalibration module, change UseTracks to False\n");
    } else {
      for (auto truehit : cluster.getRelationsTo<PXDTrueHit>()) {  // TODO: prepare this for track source, not only for true hit.
        //Get Geometry information
        sensorID = cluster.getSensorID();
        const PXD::SensorInfo& Info = dynamic_cast<const PXD::SensorInfo&>(VXD::GeoCache::get(
                                        sensorID));
        m_layer = Info.getID().getLayerNumber();
        m_sensor = Info.getID().getSensorNumber();
        m_segment = Info.getID().getSegmentNumber();
        int Conin = 1;
        if (m_CalibrationKind == 2) {
          int ladder = Info.getID().getLadderNumber();
          if (m_layer != m_SpecialLayerNo) Conin = 0;
          if (ladder != m_SpecialLadderNo) Conin = 0;
          if (m_sensor != m_SpecialSensorNo) Conin = 0;
        }
        if (Conin == 0) continue;

        m_CorrU = cluster.getU() - truehit.getU();
        m_CorrV = cluster.getV() - truehit.getV();
        double EnExWidth = fabs(truehit.getExitW() - truehit.getEntryW());
        if (EnExWidth < 0.00000001) EnExWidth = 0.0075; // in cm, use in case particle is not going thrue
        m_phi = TMath::ATan2(truehit.getExitU() - truehit.getEntryU(), EnExWidth);
        m_theta = TMath::ATan2(truehit.getExitV() - truehit.getEntryV(), EnExWidth);
        m_signal = cluster.getCharge();
        m_seed = cluster.getSeedCharge();
        double InPixUTrue = truehit.getU();
        double InPixVTrue = truehit.getV();
        double InPixUReco = cluster.getU();
        double InPixVReco = cluster.getV();

        int EdgePixelSizeV1 = 512;   // TODO assign correctly from geometry/design
        int EdgePixelSizeV2 = 256;   // TODO assign correctly from geometry/design
        int EdgePixelSizeU = 256;    // TODO assign correctly from geometry/design
        m_pixelKind = 0;  // 0: smaller pixelsize, 1: larger pixelsize TODO set it more systematically?
        if ((m_sensor == 2) && (Info.getVCellID(InPixVTrue) < EdgePixelSizeV1)) m_pixelKind = 1;
        else if ((m_sensor == 1) && (Info.getVCellID(InPixVTrue) >= EdgePixelSizeV2)) m_pixelKind = 1;
        // TODO set it more systematically? m_closeEdge values
        m_closeEdge = 0;  // 0: healthy, far from edge or masked, 1: close edge, 2: close masked pixel
        if ((m_sensor == 2) && (abs(Info.getVCellID(InPixVTrue) - EdgePixelSizeV1) < m_EdgeClose)) m_closeEdge = 1;
        else if ((m_sensor == 1) && (abs(Info.getVCellID(InPixVTrue) - EdgePixelSizeV2) < m_EdgeClose)) m_closeEdge = 1;
        else if (Info.getVCellID(InPixVTrue) < m_EdgeClose) m_closeEdge = 1;
        else if ((EdgePixelSizeV1 + EdgePixelSizeV2 - Info.getVCellID(InPixVTrue)) < m_EdgeClose) m_closeEdge = 1;
        else if (Info.getUCellID(InPixUTrue) < m_EdgeClose) m_closeEdge = 1;
        else if (EdgePixelSizeU - Info.getUCellID(InPixUTrue) < m_EdgeClose) m_closeEdge = 1;

        InPixUTrue = (Info.getUCellPosition(Info.getUCellID(InPixUTrue)) - InPixUTrue) / Info.getUPitch(InPixUTrue);
        InPixVTrue = (Info.getVCellPosition(Info.getVCellID(InPixVTrue)) - InPixVTrue) / Info.getVPitch(InPixVTrue);
        InPixUReco = (Info.getUCellPosition(Info.getUCellID(InPixUReco)) - InPixUReco) / Info.getUPitch(InPixUReco);
        InPixVReco = (Info.getVCellPosition(Info.getVCellID(InPixVReco)) - InPixVReco) / Info.getVPitch(InPixVReco);

        if (InPixUTrue < -0.5) InPixUTrue += 1.0;  // TODO: remove, this kind of correction should not use
        if (InPixUTrue > 0.5) InPixUTrue -= 1.0;
        if (InPixVTrue < -0.5) InPixVTrue += 1.0;
        if (InPixVTrue > 0.5) InPixVTrue -= 1.0;
        if (InPixUReco < -0.5) InPixUReco += 1.0;
        if (InPixUReco > 0.5) InPixUReco -= 1.0;
        if (InPixVReco < -0.5) InPixVReco += 1.0;
        if (InPixVReco > 0.5) InPixVReco -= 1.0;

        // TODO: use standard way for conversion to microns:
        InPixUTrue *= Info.getUPitch(cluster.getU()) * 10000.0;  // [um]
        InPixVTrue *= Info.getVPitch(cluster.getV()) * 10000.0;  // [um]
        InPixUReco *= Info.getUPitch(cluster.getU()) * 10000.0;  // [um]
        InPixVReco *= Info.getVPitch(cluster.getV()) * 10000.0;  // [um]

        m_InPixUTrue = InPixUTrue;
        m_InPixVTrue = InPixVTrue;
        m_InPixUReco = InPixUReco;
        m_InPixVReco = InPixVReco;
        m_shape = cluster.getShape();

        getObject<TTree>("pxdCal").Fill();
      } // end of truehit
    } // end of "if" to split to tracks / true hits
  } // end of cluster

}
