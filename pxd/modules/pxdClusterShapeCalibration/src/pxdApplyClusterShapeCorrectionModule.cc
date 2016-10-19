/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Peter Kodys                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <pxd/modules/pxdClusterShapeCalibration/pxdApplyClusterShapeCorrectionModule.h>

#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <framework/logging/Logger.h>

#include <pxd/dataobjects/PXDCluster.h>
#include <pxd/dataobjects/PXDTrueHit.h>
#include <pxd/reconstruction/PXDRecoHit.h>

#include <genfit/Track.h>
#include <genfit/TrackPoint.h>

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

using namespace Belle2;
using namespace Belle2::PXD;
using namespace std;

class PXDClusterShapeCalibrationAlgorithm;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(pxdApplyClusterShapeCorrection);

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

pxdApplyClusterShapeCorrectionModule::pxdApplyClusterShapeCorrectionModule() : Module()
{
  //Set module properties
  setDescription("Apply PXD Cluster Shape Calibration");
  setPropertyFlags(c_ParallelProcessingCertified);

  addParam("PrefereSimulation", m_PrefereSimulation,
           "To use simulations rather than real data calculated corrections, default=False", m_PrefereSimulation);

  addParam("MarkOfLoopForHistogramsFile", m_MarkOfLoopForHistogramsFile,
           "Mark of loop to save monitored data to different file, default=0", m_MarkOfLoopForHistogramsFile);

  addParam("CalFileBasic", m_CalFileBasicName,
           "Name of file contain basic calibration, default=pxdCalibrationBasic", m_CalFileBasicName);
  addParam("CalFilePK0", m_CalFilePK0Name,
           "Name of file contain calibration for pixel kind 0 (PitchV=55um), default=pxdCalibrationPixelKind0", m_CalFilePK0Name);
  addParam("CalFilePK1", m_CalFilePK1Name,
           "Name of file contain calibration for pixel kind 1 (PitchV=60um), default=pxdCalibrationPixelKind1", m_CalFilePK1Name);
  addParam("CalFilePK2", m_CalFilePK2Name,
           "Name of file contain calibration for pixel kind 2 (PitchV=70um), default=pxdCalibrationPixelKind2", m_CalFilePK2Name);
  addParam("CalFilePK3", m_CalFilePK3Name,
           "Name of file contain calibration for pixel kind 3 (PitchV=85um), default=pxdCalibrationPixelKind3", m_CalFilePK3Name);

  addParam("DoExpertHistograms", m_DoExpertHistograms,
           "Do Expert Histograms", m_DoExpertHistograms);

  addParam("ShowDetailStatistics", m_ShowDetailStatistics,
           "Show Detail Statistics", m_ShowDetailStatistics);



}

void pxdApplyClusterShapeCorrectionModule::initialize()
{
  // Only over this limit is correction accepted
  float fDifference = 0.1 * Unit::um;
  // Only under this limit is real bias correction compare to simulation accepted
  float fDifferenceClose = 3 * Unit::um;
  // Only under this limit is real error estimation correction compare to simulation accepted
  float fDifferenceErrEst = 0.1;

  //Mark all StoreArrays as required
  StoreArray<PXDCluster> storeClusters;
  StoreArray<genfit::Track> tracks(m_storeTrackName);
  m_storeTrackName = tracks.getName();

  // START - load corrections:

  B2INFO("pxdApplyClusterShapeCorrection: Load correction files:");

  TVectorD** PXDShCalibrationBasicSetting = NULL;
  TVectorD** Correction_Bias = NULL;
  TVectorD** Correction_ErrorEstimation = NULL;
  TVectorD** Correction_BiasErr = NULL;
  TVectorD** InPixelPosition = NULL;

  PXDShCalibrationBasicSetting = (TVectorD**) new TVectorD*[6];
  Correction_Bias = (TVectorD**) new TVectorD*[6];
  Correction_ErrorEstimation = (TVectorD**) new TVectorD*[6];
  Correction_BiasErr = (TVectorD**) new TVectorD*[6];
  InPixelPosition = (TVectorD**) new TVectorD*[6];
  for (int i = 0; i < 6; i++) {
    PXDShCalibrationBasicSetting[i] = NULL;
    Correction_Bias[i] = NULL;
    Correction_ErrorEstimation[i] = NULL;
    Correction_BiasErr[i] = NULL;
    InPixelPosition[i] = NULL;
  }

  TFile* f_CalBasic = new TFile(m_CalFileBasicName.c_str(), "read");
  int iLoad = 0;
  if (f_CalBasic->IsOpen()) {
    m_ExistCorrectionBasic = 1;
    B2INFO("pxdApplyClusterShapeCorrection: exist correction file Basic: " << m_CalFileBasicName.c_str());
//      getObject<std::vector<TVectorD>(name_SourceTree.Data()).SetBranchAddress("event", &m_evt);
    f_CalBasic->GetObject("PXDShCalibrationBasicSetting", PXDShCalibrationBasicSetting[iLoad]);
    f_CalBasic->GetObject("Correction_Bias", Correction_Bias[iLoad]);
    f_CalBasic->GetObject("Correction_ErrorEstimation", Correction_ErrorEstimation[iLoad]);
    f_CalBasic->GetObject("Correction_BiasErr", Correction_BiasErr[iLoad]);
    f_CalBasic->GetObject("InPixelPosition", InPixelPosition[iLoad]);
    m_shapes = (int)PXDShCalibrationBasicSetting[iLoad]->GetMatrixArray()[0];
    m_pixelkinds = (int)PXDShCalibrationBasicSetting[iLoad]->GetMatrixArray()[1];
    m_dimensions = (int)PXDShCalibrationBasicSetting[iLoad]->GetMatrixArray()[2];
    m_anglesU = (int)PXDShCalibrationBasicSetting[iLoad]->GetMatrixArray()[3];
    m_anglesV = (int)PXDShCalibrationBasicSetting[iLoad]->GetMatrixArray()[4];
    m_in_pixelU = (int)PXDShCalibrationBasicSetting[iLoad]->GetMatrixArray()[5];
    m_in_pixelV = (int)PXDShCalibrationBasicSetting[iLoad]->GetMatrixArray()[6];
    B2INFO("pxdApplyClusterShapeCorrection:  ---> Field dimensions: shapes: " << m_shapes << ", pixelkinds: " << m_pixelkinds <<
           ", dimensions: " << m_dimensions <<
           ", anglesU: " << m_anglesU << ", anglesV: " << m_anglesV << ", in_pixelU: " << m_in_pixelU << ", in_pixelV: " << m_in_pixelV);
    B2DEBUG(130, "-----> try to read non-zero elements:");
    for (int i_shape = 0; i_shape < 3000; i_shape++) {
      if (fabs(Correction_Bias[iLoad]->GetMatrixArray()[i_shape]) > fDifference)
        B2DEBUG(130, "     --> " << Correction_Bias[iLoad]->GetMatrixArray()[i_shape] << " (" << i_shape << ")");
    }
    f_CalBasic->Close();
  }
  f_CalBasic = new TFile(m_CalFilePK0Name.c_str(), "read");
  iLoad = 1;
  if (f_CalBasic->IsOpen()) {
    m_ExistCorrectionPK0 = 1;
    B2INFO("pxdApplyClusterShapeCorrection: exist correction file PK0: " << m_CalFilePK0Name.c_str());
    f_CalBasic->GetObject("PXDShCalibrationBasicSetting", PXDShCalibrationBasicSetting[iLoad]);
    f_CalBasic->GetObject("Correction_Bias", Correction_Bias[iLoad]);
    f_CalBasic->GetObject("Correction_ErrorEstimation", Correction_ErrorEstimation[iLoad]);
    f_CalBasic->GetObject("Correction_BiasErr", Correction_BiasErr[iLoad]);
    f_CalBasic->GetObject("InPixelPosition", InPixelPosition[iLoad]);
    if (fabs(m_shapes - PXDShCalibrationBasicSetting[iLoad]->GetMatrixArray()[0]) > fDifference) m_ExistCorrectionPK0 = 0;
    if (fabs(m_pixelkinds - PXDShCalibrationBasicSetting[iLoad]->GetMatrixArray()[1]) > fDifference) m_ExistCorrectionPK0 = 0;
    if (fabs(m_dimensions - PXDShCalibrationBasicSetting[iLoad]->GetMatrixArray()[2]) > fDifference) m_ExistCorrectionPK0 = 0;
    if (fabs(m_anglesU - PXDShCalibrationBasicSetting[iLoad]->GetMatrixArray()[3]) > fDifference) m_ExistCorrectionPK0 = 0;
    if (fabs(m_anglesV - PXDShCalibrationBasicSetting[iLoad]->GetMatrixArray()[4]) > fDifference) m_ExistCorrectionPK0 = 0;
    if (fabs(m_in_pixelU - PXDShCalibrationBasicSetting[iLoad]->GetMatrixArray()[5]) > fDifference) m_ExistCorrectionPK0 = 0;
    if (fabs(m_in_pixelV - PXDShCalibrationBasicSetting[iLoad]->GetMatrixArray()[6]) > fDifference) m_ExistCorrectionPK0 = 0;
    if (m_ExistCorrectionPK0 == 0) B2INFO("pxdApplyClusterShapeCorrection: ERROR in PK0 file! differences on field dimensions.");
    f_CalBasic->Close();
  }
  f_CalBasic = new TFile(m_CalFilePK1Name.c_str(), "read");
  iLoad = 2;
  if (f_CalBasic->IsOpen()) {
    m_ExistCorrectionPK1 = 1;
    B2INFO("pxdApplyClusterShapeCorrection: exist correction file PK1: " << m_CalFilePK1Name.c_str());
    f_CalBasic->GetObject("PXDShCalibrationBasicSetting", PXDShCalibrationBasicSetting[iLoad]);
    f_CalBasic->GetObject("Correction_Bias", Correction_Bias[iLoad]);
    f_CalBasic->GetObject("Correction_ErrorEstimation", Correction_ErrorEstimation[iLoad]);
    f_CalBasic->GetObject("Correction_BiasErr", Correction_BiasErr[iLoad]);
    f_CalBasic->GetObject("InPixelPosition", InPixelPosition[iLoad]);
    if (fabs(m_shapes - PXDShCalibrationBasicSetting[iLoad]->GetMatrixArray()[0]) > fDifference) m_ExistCorrectionPK1 = 0;
    if (fabs(m_pixelkinds - PXDShCalibrationBasicSetting[iLoad]->GetMatrixArray()[1]) > fDifference) m_ExistCorrectionPK1 = 0;
    if (fabs(m_dimensions - PXDShCalibrationBasicSetting[iLoad]->GetMatrixArray()[2]) > fDifference) m_ExistCorrectionPK1 = 0;
    if (fabs(m_anglesU - PXDShCalibrationBasicSetting[iLoad]->GetMatrixArray()[3]) > fDifference) m_ExistCorrectionPK1 = 0;
    if (fabs(m_anglesV - PXDShCalibrationBasicSetting[iLoad]->GetMatrixArray()[4]) > fDifference) m_ExistCorrectionPK1 = 0;
    if (fabs(m_in_pixelU - PXDShCalibrationBasicSetting[iLoad]->GetMatrixArray()[5]) > fDifference) m_ExistCorrectionPK1 = 0;
    if (fabs(m_in_pixelV - PXDShCalibrationBasicSetting[iLoad]->GetMatrixArray()[6]) > fDifference) m_ExistCorrectionPK1 = 0;
    if (m_ExistCorrectionPK1 == 0) B2INFO("pxdApplyClusterShapeCorrection: ERROR in PK1 file! differences on field dimensions.");
    f_CalBasic->Close();
  }
  f_CalBasic = new TFile(m_CalFilePK2Name.c_str(), "read");
  iLoad = 3;
  if (f_CalBasic->IsOpen()) {
    m_ExistCorrectionPK2 = 1;
    B2INFO("pxdApplyClusterShapeCorrection: exist correction file PK2: " << m_CalFilePK2Name.c_str());
    f_CalBasic->GetObject("PXDShCalibrationBasicSetting", PXDShCalibrationBasicSetting[iLoad]);
    f_CalBasic->GetObject("Correction_Bias", Correction_Bias[iLoad]);
    f_CalBasic->GetObject("Correction_ErrorEstimation", Correction_ErrorEstimation[iLoad]);
    f_CalBasic->GetObject("Correction_BiasErr", Correction_BiasErr[iLoad]);
    f_CalBasic->GetObject("InPixelPosition", InPixelPosition[iLoad]);
    if (fabs(m_shapes - PXDShCalibrationBasicSetting[iLoad]->GetMatrixArray()[0]) > fDifference) m_ExistCorrectionPK2 = 0;
    if (fabs(m_pixelkinds - PXDShCalibrationBasicSetting[iLoad]->GetMatrixArray()[1]) > fDifference) m_ExistCorrectionPK2 = 0;
    if (fabs(m_dimensions - PXDShCalibrationBasicSetting[iLoad]->GetMatrixArray()[2]) > fDifference) m_ExistCorrectionPK2 = 0;
    if (fabs(m_anglesU - PXDShCalibrationBasicSetting[iLoad]->GetMatrixArray()[3]) > fDifference) m_ExistCorrectionPK2 = 0;
    if (fabs(m_anglesV - PXDShCalibrationBasicSetting[iLoad]->GetMatrixArray()[4]) > fDifference) m_ExistCorrectionPK2 = 0;
    if (fabs(m_in_pixelU - PXDShCalibrationBasicSetting[iLoad]->GetMatrixArray()[5]) > fDifference) m_ExistCorrectionPK2 = 0;
    if (fabs(m_in_pixelV - PXDShCalibrationBasicSetting[iLoad]->GetMatrixArray()[6]) > fDifference) m_ExistCorrectionPK2 = 0;
    if (m_ExistCorrectionPK2 == 0) B2INFO("pxdApplyClusterShapeCorrection: ERROR in PK2 file! differences on field dimensions.");
    f_CalBasic->Close();
  }
  f_CalBasic = new TFile(m_CalFilePK3Name.c_str(), "read");
  iLoad = 4;
  if (f_CalBasic->IsOpen()) {
    m_ExistCorrectionPK3 = 1;
    B2INFO("pxdApplyClusterShapeCorrection: exist correction file PK3: " << m_CalFilePK3Name.c_str());
    f_CalBasic->GetObject("PXDShCalibrationBasicSetting", PXDShCalibrationBasicSetting[iLoad]);
    f_CalBasic->GetObject("Correction_Bias", Correction_Bias[iLoad]);
    f_CalBasic->GetObject("Correction_ErrorEstimation", Correction_ErrorEstimation[iLoad]);
    f_CalBasic->GetObject("Correction_BiasErr", Correction_BiasErr[iLoad]);
    f_CalBasic->GetObject("InPixelPosition", InPixelPosition[iLoad]);
    if (fabs(m_shapes - PXDShCalibrationBasicSetting[iLoad]->GetMatrixArray()[0]) > fDifference) m_ExistCorrectionPK3 = 0;
    if (fabs(m_pixelkinds - PXDShCalibrationBasicSetting[iLoad]->GetMatrixArray()[1]) > fDifference) m_ExistCorrectionPK3 = 0;
    if (fabs(m_dimensions - PXDShCalibrationBasicSetting[iLoad]->GetMatrixArray()[2]) > fDifference) m_ExistCorrectionPK3 = 0;
    if (fabs(m_anglesU - PXDShCalibrationBasicSetting[iLoad]->GetMatrixArray()[3]) > fDifference) m_ExistCorrectionPK3 = 0;
    if (fabs(m_anglesV - PXDShCalibrationBasicSetting[iLoad]->GetMatrixArray()[4]) > fDifference) m_ExistCorrectionPK3 = 0;
    if (fabs(m_in_pixelU - PXDShCalibrationBasicSetting[iLoad]->GetMatrixArray()[5]) > fDifference) m_ExistCorrectionPK3 = 0;
    if (fabs(m_in_pixelV - PXDShCalibrationBasicSetting[iLoad]->GetMatrixArray()[6]) > fDifference) m_ExistCorrectionPK3 = 0;
    if (m_ExistCorrectionPK3 == 0) B2INFO("pxdApplyClusterShapeCorrection: ERROR in PK3 file! differences on field dimensions.");
    f_CalBasic->Close();
  }
  for (int i_shape = 0; i_shape < m_shapes; i_shape++)
    for (int i_pk = 0; i_pk < m_pixelkinds; i_pk++)
      for (int i_axis = 0; i_axis < m_dimensions; i_axis++)
        for (int i_angleU = 0; i_angleU < m_anglesU; i_angleU++)
          for (int i_angleV = 0; i_angleV < m_anglesV; i_angleV++) {
            int i_vector = i_shape * m_pixelkinds * m_dimensions * m_anglesU * m_anglesV;
            i_vector += i_pk * m_dimensions * m_anglesU * m_anglesV;
            i_vector += i_axis * m_anglesU * m_anglesV;
            i_vector += i_angleU * m_anglesV;
            i_vector += i_angleV;
            for (int i = 0; i < 5; i++) {
              TCorrection_BiasMap[i][make_tuple(i_shape, i_pk, i_axis, i_angleU, i_angleV)] = Correction_Bias[i]->GetMatrixArray()[i_vector];
              TCorrection_ErrorEstimationMap[i][make_tuple(i_shape, i_pk, i_axis, i_angleU,
                                                           i_angleV)] = Correction_ErrorEstimation[i]->GetMatrixArray()[i_vector];
              TCorrection_BiasMapErr[i][make_tuple(i_shape, i_pk, i_axis, i_angleU,
                                                   i_angleV)] = Correction_BiasErr[i]->GetMatrixArray()[i_vector];
            }
          }

  for (int i_shape = 0; i_shape < m_shapes; i_shape++)
    for (int i_axis = 0; i_axis < m_dimensions; i_axis++)
      for (int i_angleU = 0; i_angleU < m_anglesU; i_angleU++)
        for (int i_angleV = 0; i_angleV < m_anglesV; i_angleV++)
          for (int i_pk = 0; i_pk < 4; i_pk++) {
            TCorrection_BiasMap[5][make_tuple(i_shape, i_pk, i_axis, i_angleU, i_angleV)] = TCorrection_BiasMap[i_pk + 1][make_tuple(i_shape,
                i_pk + 4, i_axis, i_angleU, i_angleV)];
            TCorrection_BiasMap[5][make_tuple(i_shape, i_pk + 4, i_axis, i_angleU,
                                              i_angleV)] = TCorrection_BiasMap[i_pk + 1][make_tuple(i_shape, i_pk + 4, i_axis, i_angleU, i_angleV)];
            TCorrection_ErrorEstimationMap[5][make_tuple(i_shape, i_pk, i_axis, i_angleU,
                                                         i_angleV)] = TCorrection_ErrorEstimationMap[i_pk + 1][make_tuple(i_shape, i_pk + 4, i_axis, i_angleU, i_angleV)];
            TCorrection_ErrorEstimationMap[5][make_tuple(i_shape, i_pk + 4, i_axis, i_angleU,
                                                         i_angleV)] = TCorrection_ErrorEstimationMap[i_pk + 1][make_tuple(i_shape, i_pk + 4, i_axis, i_angleU, i_angleV)];
            TCorrection_BiasMapErr[5][make_tuple(i_shape, i_pk, i_axis, i_angleU,
                                                 i_angleV)] = TCorrection_BiasMapErr[i_pk + 1][make_tuple(i_shape, i_pk + 4, i_axis, i_angleU, i_angleV)];
            TCorrection_BiasMapErr[5][make_tuple(i_shape, i_pk + 4, i_axis, i_angleU,
                                                 i_angleV)] = TCorrection_BiasMapErr[i_pk + 1][make_tuple(i_shape, i_pk + 4, i_axis, i_angleU, i_angleV)];
          }

  if (!m_PrefereSimulation) {
    for (int i_shape = 0; i_shape < m_shapes; i_shape++)
      for (int i_axis = 0; i_axis < m_dimensions; i_axis++)
        for (int i_angleU = 0; i_angleU < m_anglesU; i_angleU++)
          for (int i_angleV = 0; i_angleV < m_anglesV; i_angleV++)
            for (int i_pk = 0; i_pk < m_pixelkinds; i_pk++) {
              if (fabs(TCorrection_BiasMap[0][make_tuple(i_shape, i_pk, i_axis, i_angleU, i_angleV)]) > fDifference) {
                TCorrection_BiasMap[5][make_tuple(i_shape, i_pk, i_axis, i_angleU, i_angleV)] = TCorrection_BiasMap[0][make_tuple(i_shape, i_pk,
                    i_axis, i_angleU, i_angleV)];
                //TCorrection_BiasMapErr[5][make_tuple(i_shape, i_pk, i_axis, i_angleU, i_angleV)] = TCorrection_BiasMapErr[0][make_tuple(i_shape, i_pk, i_axis, i_angleU, i_angleV)];
              }
              if ((fabs(TCorrection_BiasMap[0][make_tuple(i_shape, i_pk, i_axis, i_angleU, i_angleV)]) > fDifference) &&
                  (fabs(TCorrection_BiasMap[5][make_tuple(i_shape, i_pk, i_axis, i_angleU, i_angleV)]) > fDifference)) {
                if (fabs(TCorrection_BiasMap[5][make_tuple(i_shape, i_pk, i_axis, i_angleU, i_angleV)] - TCorrection_BiasMap[0][make_tuple(i_shape,
                         i_pk, i_axis, i_angleU, i_angleV)]) < fDifferenceClose) {
                  //printf("x");
                  //if (TCorrection_BiasMap[0][make_tuple(i_shape, i_pk%4, i_axis, i_angleU, i_angleV)] != TCorrection_BiasMap[0][make_tuple(i_shape, i_pk%4+4, i_axis, i_angleU, i_angleV)])
                  //  printf("-------> %f %f \n",10000.0 * TCorrection_BiasMap[0][make_tuple(i_shape, i_pk%4, i_axis, i_angleU, i_angleV)] ,
                  //         10000.0 * TCorrection_BiasMap[0][make_tuple(i_shape, i_pk%4+4, i_axis, i_angleU, i_angleV)]);
//                  TCorrection_BiasMap[5][make_tuple(i_shape, i_pk, i_axis, i_angleU, i_angleV)] = TCorrection_BiasMap[0][make_tuple(i_shape, i_pk, i_axis, i_angleU, i_angleV)];
//                  TCorrection_BiasMapErr[5][make_tuple(i_shape, i_pk, i_axis, i_angleU, i_angleV)] = TCorrection_BiasMapErr[0][make_tuple(i_shape, i_pk, i_axis, i_angleU, i_angleV)];
                }
              }
              if (fabs(TCorrection_ErrorEstimationMap[0][make_tuple(i_shape, i_pk, i_axis, i_angleU, i_angleV)] - 1.0) > fDifference) {
                TCorrection_ErrorEstimationMap[5][make_tuple(i_shape, i_pk, i_axis, i_angleU,
                                                             i_angleV)] = TCorrection_ErrorEstimationMap[0][make_tuple(i_shape, i_pk, i_axis, i_angleU, i_angleV)];
              }
              if ((fabs(TCorrection_ErrorEstimationMap[0][make_tuple(i_shape, i_pk, i_axis, i_angleU, i_angleV)] - 1.0) > fDifference) &&
                  (fabs(TCorrection_ErrorEstimationMap[5][make_tuple(i_shape, i_pk, i_axis, i_angleU, i_angleV)] - 1.0) > fDifference)) {
                if (fabs(TCorrection_ErrorEstimationMap[5][make_tuple(i_shape, i_pk, i_axis, i_angleU,
                                                                      i_angleV)] - TCorrection_ErrorEstimationMap[0][make_tuple(i_shape, i_pk, i_axis, i_angleU, i_angleV)]) < fDifferenceErrEst) {
                  //TCorrection_ErrorEstimationMap[5][make_tuple(i_shape, i_pk, i_axis, i_angleU,
                  //                                             i_angleV)] = TCorrection_ErrorEstimationMap[0][make_tuple(i_shape, i_pk, i_axis, i_angleU, i_angleV)];
                }
              }
            }
  }
  // Correction inverted counting in v (axis=1) for Sensor = 1:
  for (int i_shape = 0; i_shape < m_shapes; i_shape++)
    for (int i_angleU = 0; i_angleU < m_anglesU; i_angleU++)
      for (int i_angleV = 0; i_angleV < m_anglesV; i_angleV++)
        for (int i_pk = 0; i_pk < 4; i_pk++) {  // only first 4 pixel kinds for Sensor = 1
          //int i_axis = 1;
          //TCorrection_BiasMap[5][make_tuple(i_shape, i_pk, i_axis, i_angleU, i_angleV)] = -TCorrection_BiasMap[5][make_tuple(i_shape, i_pk, i_axis, i_angleU, i_angleV)];
        }

  for (int i_shape = 0; i_shape < m_shapes; i_shape++)
    for (int i_pk = 0; i_pk < m_pixelkinds; i_pk++)
      for (int i_angleU = 0; i_angleU < m_anglesU; i_angleU++)
        for (int i_angleV = 0; i_angleV < m_anglesV; i_angleV++)
          for (int i_ipU = 0; i_ipU < m_in_pixelU; i_ipU++)
            for (int i_ipV = 0; i_ipV < m_in_pixelV; i_ipV++) {
              int i_vector = i_shape * m_pixelkinds * m_anglesU * m_anglesV * m_in_pixelU * m_in_pixelV;
              i_vector += i_pk * m_anglesU * m_anglesV * m_in_pixelU * m_in_pixelV;
              i_vector += i_angleU * m_anglesV * m_in_pixelU * m_in_pixelV;
              i_vector += i_angleV * m_in_pixelU * m_in_pixelV;
              i_vector += i_ipU * m_in_pixelV;
              i_vector += i_ipV;
              for (int i = 0; i < 5; i++) {
                TInPixelPositionMap[i][make_tuple(i_shape, i_pk, i_angleU, i_angleV, i_ipU,
                                                  i_ipV)] = InPixelPosition[i]->GetMatrixArray()[i_vector];
              }
            }

  // END - load corrections:
  TString name_OutDoExpertHistograms;
  TString UseCorrs;
  TString DirPixelKind;
  TString DirShape;

  m_histResidualU = (TH1F**) new TH1F*[2 * (m_pixelkinds * m_shapes + 1)];
  m_histResidualV = (TH1F**) new TH1F*[2 * (m_pixelkinds * m_shapes + 1)];
  m_histResidualUV = (TH2F**) new TH2F*[2 * (m_pixelkinds * m_shapes + 1)];
  m_histNormErrorU = (TH1F**) new TH1F*[2 * (m_pixelkinds * m_shapes + 1)];
  m_histNormErrorV = (TH1F**) new TH1F*[2 * (m_pixelkinds * m_shapes + 1)];
  m_histNormErrorUV = (TH2F**) new TH2F*[2 * (m_pixelkinds * m_shapes + 1)];

  for (int i_pk = 0; i_pk < m_pixelkinds; i_pk++) {
    for (int i_shape = 0; i_shape < m_shapes + 1; i_shape++) {
      for (int i_UseCor = 0; i_UseCor < 2; i_UseCor++) {

        m_histResidualU[i_UseCor * (m_pixelkinds * m_shapes + 1) + i_pk * m_shapes + i_shape] = NULL;
        m_histResidualV[i_UseCor * (m_pixelkinds * m_shapes + 1) + i_pk * m_shapes + i_shape] = NULL;
        m_histResidualUV[i_UseCor * (m_pixelkinds * m_shapes + 1) + i_pk * m_shapes + i_shape] = NULL;
        m_histNormErrorU[i_UseCor * (m_pixelkinds * m_shapes + 1) + i_pk * m_shapes + i_shape] = NULL;
        m_histNormErrorV[i_UseCor * (m_pixelkinds * m_shapes + 1) + i_pk * m_shapes + i_shape] = NULL;
        m_histNormErrorUV[i_UseCor * (m_pixelkinds * m_shapes + 1) + i_pk * m_shapes + i_shape] = NULL;
      }
    }
  }
  name_OutDoExpertHistograms = Form("pxdClShCalApplyHistos_%i.root", m_MarkOfLoopForHistogramsFile);
  if (m_DoExpertHistograms) {  // do experts
    fExpertHistograms = new TFile(name_OutDoExpertHistograms.Data(), "recreate");
    fExpertHistograms->mkdir("NoSorting");
    for (int i_UseCor = 0; i_UseCor < 2; i_UseCor++) {  // before/after
      if (i_UseCor == 0) UseCorrs = Form("BeforeCors");
      if (i_UseCor == 1) UseCorrs = Form("AfterCors");
      for (int i_pk = 0; i_pk < m_pixelkinds; i_pk++) {  // Pixel Kind
        DirPixelKind = Form("PixelKind_%01i_Layer_%i_Sensor_%i_Size_%i", i_pk, (int)((i_pk % 4) / 2) + 1, (int)(i_pk / 4) + 1, i_pk % 2);
        if (i_UseCor == 0)
          fExpertHistograms->mkdir(DirPixelKind.Data());
        for (int i_shape = 0; i_shape < m_shapes; i_shape++) {  // shapes
          DirShape = Form("%s/Shape_%02i_%s", DirPixelKind.Data(), i_shape + 1,
                          Belle2::PXD::PXDClusterShape::pxdClusterShapeDescription[(Belle2::PXD::pxdClusterShapeType)(i_shape + 1)].c_str());
          DirShape.ReplaceAll(":", "");
          DirShape.ReplaceAll(" ", "_");
          if (i_UseCor == 0)
            fExpertHistograms->mkdir(DirShape.Data());
          fExpertHistograms->cd(DirShape.Data());
          TString HistoName = Form("%sResidualsU_PK%01i_Sh%02i", UseCorrs.Data(), i_pk, i_shape + 1);
          TString HistoTitle = Form("%sResiduals U, pixel kind %01i, shape %02i", UseCorrs.Data(), i_pk, i_shape + 1);
          m_histResidualU[i_UseCor * (m_pixelkinds * m_shapes + 1) + i_pk * m_shapes + i_shape] = new TH1F(HistoName.Data(),
              HistoTitle.Data(),
              400, -200, 200);
          m_histResidualU[i_UseCor * (m_pixelkinds * m_shapes + 1) + i_pk * m_shapes +
                          i_shape]->GetXaxis()->SetTitle("Residuals in u [#mum]");
          HistoName = Form("%sResidualsV_PK%01i_Sh%02i", UseCorrs.Data(), i_pk, i_shape + 1);
          HistoTitle = Form("%sResiduals V, pixel kind %01i, shape %02i", UseCorrs.Data(), i_pk, i_shape + 1);
          m_histResidualV[i_UseCor * (m_pixelkinds * m_shapes + 1) + i_pk * m_shapes + i_shape] = new TH1F(HistoName.Data(),
              HistoTitle.Data(),
              400, -200, 200);
          m_histResidualU[i_UseCor * (m_pixelkinds * m_shapes + 1) + i_pk * m_shapes +
                          i_shape]->GetXaxis()->SetTitle("Residuals in v [#mum]");
          HistoName = Form("%sResidualsUV_PK%01i_Sh%02i", UseCorrs.Data(), i_pk, i_shape + 1);
          HistoTitle = Form("%sResiduals U V, pixel kind %01i, shape %02i", UseCorrs.Data(), i_pk, i_shape + 1);
          m_histResidualUV[i_UseCor * (m_pixelkinds * m_shapes + 1) + i_pk * m_shapes + i_shape] = new TH2F(HistoName.Data(),
              HistoTitle.Data(),
              100, -200, 200, 100, -200, 200);
          m_histResidualU[i_UseCor * (m_pixelkinds * m_shapes + 1) + i_pk * m_shapes +
                          i_shape]->GetXaxis()->SetTitle("Residuals in u [#mum]");
          m_histResidualU[i_UseCor * (m_pixelkinds * m_shapes + 1) + i_pk * m_shapes +
                          i_shape]->GetYaxis()->SetTitle("Residuals in v [#mum]");

          HistoName = Form("%sNormErrorsU_PK%01i_Sh%02i", UseCorrs.Data(), i_pk, i_shape + 1);
          HistoTitle = Form("%sNormalised Errors U, pixel kind %01i, shape %02i", UseCorrs.Data(), i_pk, i_shape + 1);
          m_histNormErrorU[i_UseCor * (m_pixelkinds * m_shapes + 1) + i_pk * m_shapes + i_shape] = new TH1F(HistoName.Data(),
              HistoTitle.Data(),
              100, -5, 5);
          m_histNormErrorU[i_UseCor * (m_pixelkinds * m_shapes + 1) + i_pk * m_shapes +
                           i_shape]->GetXaxis()->SetTitle("Normalised Errors in u");
          HistoName = Form("%sNormErrorsV_PK%01i_Sh%02i", UseCorrs.Data(), i_pk, i_shape + 1);
          HistoTitle = Form("%sNormalised Errors V, pixel kind %01i, shape %02i", UseCorrs.Data(), i_pk, i_shape + 1);
          m_histNormErrorV[i_UseCor * (m_pixelkinds * m_shapes + 1) + i_pk * m_shapes + i_shape] = new TH1F(HistoName.Data(),
              HistoTitle.Data(),
              100, -5, 5);
          m_histNormErrorV[i_UseCor * (m_pixelkinds * m_shapes + 1) + i_pk * m_shapes +
                           i_shape]->GetXaxis()->SetTitle("Normalised Errors in v");
          HistoName = Form("%sNormErrorsUV_PK%01i_Sh%02i", UseCorrs.Data(), i_pk, i_shape + 1);
          HistoTitle = Form("%sNormalised Errors U V, pixel kind %01i, shape %02i", UseCorrs.Data(), i_pk, i_shape + 1);
          m_histNormErrorUV[i_UseCor * (m_pixelkinds * m_shapes + 1) + i_pk * m_shapes + i_shape] = new TH2F(HistoName.Data(),
              HistoTitle.Data(),
              100, -5, 5, 100, -5, 5);
          m_histNormErrorUV[i_UseCor * (m_pixelkinds * m_shapes + 1) + i_pk * m_shapes +
                            i_shape]->GetXaxis()->SetTitle("Normalised Errors in u");
          m_histNormErrorUV[i_UseCor * (m_pixelkinds * m_shapes + 1) + i_pk * m_shapes +
                            i_shape]->GetYaxis()->SetTitle("Normalised Errors in v");

        }  // shapes End
      }  // Pixel Kind End
      fExpertHistograms->cd("NoSorting");
      TString HistoName = Form("%sResidualsU", UseCorrs.Data());
      TString HistoTitle = Form("%sResiduals U", UseCorrs.Data());
      m_histResidualU[i_UseCor * (m_pixelkinds * m_shapes + 1) + m_pixelkinds * m_shapes] = new TH1F(HistoName.Data(), HistoTitle.Data(),
          400, -200, 200);
      m_histResidualU[i_UseCor * (m_pixelkinds * m_shapes + 1) + m_pixelkinds * m_shapes]->GetXaxis()->SetTitle("Residuals in u [#mum]");
      HistoName = Form("%sResidualsV", UseCorrs.Data());
      HistoTitle = Form("%sResiduals V", UseCorrs.Data());
      m_histResidualV[i_UseCor * (m_pixelkinds * m_shapes + 1) + m_pixelkinds * m_shapes] = new TH1F(HistoName.Data(), HistoTitle.Data(),
          400, -200, 200);
      m_histResidualU[i_UseCor * (m_pixelkinds * m_shapes + 1) + m_pixelkinds * m_shapes]->GetXaxis()->SetTitle("Residuals in v [#mum]");
      HistoName = Form("%sResidualsUV", UseCorrs.Data());
      HistoTitle = Form("%sResiduals U V", UseCorrs.Data());
      m_histResidualUV[i_UseCor * (m_pixelkinds * m_shapes + 1) + m_pixelkinds * m_shapes] = new TH2F(HistoName.Data(), HistoTitle.Data(),
          100, -200, 200, 100, -200, 200);
      m_histResidualU[i_UseCor * (m_pixelkinds * m_shapes + 1) + m_pixelkinds * m_shapes]->GetXaxis()->SetTitle("Residuals in u [#mum]");
      m_histResidualU[i_UseCor * (m_pixelkinds * m_shapes + 1) + m_pixelkinds * m_shapes]->GetYaxis()->SetTitle("Residuals in v [#mum]");

      HistoName = Form("%sNormErrorsU", UseCorrs.Data());
      HistoTitle = Form("%sNormalised Errors U", UseCorrs.Data());
      m_histNormErrorU[i_UseCor * (m_pixelkinds * m_shapes + 1) + m_pixelkinds * m_shapes] = new TH1F(HistoName.Data(), HistoTitle.Data(),
          100, -5, 5);
      m_histNormErrorU[i_UseCor * (m_pixelkinds * m_shapes + 1) + m_pixelkinds *
                       m_shapes]->GetXaxis()->SetTitle("Normalised Errors in u");
      HistoName = Form("%sNormErrorsV", UseCorrs.Data());
      HistoTitle = Form("%sNormalised Errors V", UseCorrs.Data());
      m_histNormErrorV[i_UseCor * (m_pixelkinds * m_shapes + 1) + m_pixelkinds * m_shapes] = new TH1F(HistoName.Data(), HistoTitle.Data(),
          100, -5, 5);
      m_histNormErrorV[i_UseCor * (m_pixelkinds * m_shapes + 1) + m_pixelkinds *
                       m_shapes]->GetXaxis()->SetTitle("Normalised Errors in v");
      HistoName = Form("%sNormErrorsUV", UseCorrs.Data());
      HistoTitle = Form("%sNormalised Errors U V", UseCorrs.Data());
      m_histNormErrorUV[i_UseCor * (m_pixelkinds * m_shapes + 1) + m_pixelkinds * m_shapes] = new TH2F(HistoName.Data(),
          HistoTitle.Data(),
          100, -5, 5, 100, -5, 5);
      m_histNormErrorUV[i_UseCor * (m_pixelkinds * m_shapes + 1) + m_pixelkinds *
                        m_shapes]->GetXaxis()->SetTitle("Normalised Errors in u");
      m_histNormErrorUV[i_UseCor * (m_pixelkinds * m_shapes + 1) + m_pixelkinds *
                        m_shapes]->GetYaxis()->SetTitle("Normalised Errors in v");
    }  // before/after End
  }  // do experts End


  /*
  // delete seems...
    // ******************* Save corrections to asci file: ********************************
    FILE* AscFile = fopen("Corrections.txt", "w");
    fprintf(AscFile,
            "Storring of all nonZero corrections for Bias and its error (in microns), and ErrorEstimation Corrections different from 1.0\n\n");
  //    fprintf(AscFile, "\n  ***********  Correction case %i:  ***********\n", i);
  //  for (int i_pk = 0; i_pk < m_pixelkinds / 2; i_pk++) {
    for (int i_pk = 0; i_pk < m_pixelkinds; i_pk++) {
      for (int i_shape = 0; i_shape < m_shapes; i_shape++) {
        for (int i_angleU = 0; i_angleU < m_anglesU; i_angleU++) {
          for (int i_angleV = 0; i_angleV < m_anglesV; i_angleV++) {
            for (int i_axis = 0; i_axis < m_dimensions; i_axis++) {
  //            if ((TCorrection_BiasMap[0][make_tuple(i_shape, i_pk, i_axis, i_angleU, i_angleV)] == 0.0) ||
  //              (TCorrection_BiasMap[0][make_tuple(i_shape, i_pk + 4, i_axis, i_angleU, i_angleV)] == 0.0)) {
  //              continue;
  //            }
              if (!((TCorrection_BiasMap[0][make_tuple(i_shape, i_pk, i_axis, i_angleU, i_angleV)]  != 0.0) && (TCorrection_BiasMap[5][make_tuple(i_shape, i_pk, i_axis, i_angleU, i_angleV)]  != 0.0))) {
            //    continue;
              }
              if (TCorrection_BiasMap[0][make_tuple(i_shape, i_pk, i_axis, i_angleU, i_angleV)]  == 0.0) {
                continue;
              }
              if (TCorrection_BiasMap[5][make_tuple(i_shape, i_pk, i_axis, i_angleU, i_angleV)]  == 0.0) {
            //    continue;
              }
              if (TCorrection_BiasMap[i_pk + 1][make_tuple(i_shape, i_pk + 4, i_axis, i_angleU, i_angleV)]  == 0.0) {
                continue;
              }
              if ((fabs(TCorrection_BiasMap[0][make_tuple(i_shape, i_pk, i_axis, i_angleU,
                                                          i_angleV)] - TCorrection_BiasMap[i_pk + 1][make_tuple(i_shape, i_pk + 4, i_axis, i_angleU, i_angleV)]) < .0003) ||
                  (fabs(TCorrection_BiasMap[0][make_tuple(i_shape, i_pk + 4, i_axis, i_angleU,
                                                          i_angleV)] - TCorrection_BiasMap[i_pk + 1][make_tuple(i_shape, i_pk + 4, i_axis, i_angleU, i_angleV)]) < .0003)) {
                continue;
              }
              fprintf(AscFile, "PixKind %1i Sh %2i AngU %2i AngV %2i Dir %1i : Bias", i_pk, i_shape, i_angleU, i_angleV, i_axis);
              for (int i = 0; i < 6; i++) {
                //if (TCorrection_BiasMap[i][make_tuple(i_shape, i_pk, i_axis, i_angleU, i_angleV)] != 0) {
                if (i == 0) {
                  fprintf(AscFile, " case %i : %7.2f +- %7.2f ,", i,
                          TCorrection_BiasMap[i][make_tuple(i_shape, i_pk % 4, i_axis, i_angleU, i_angleV)] / Unit::um,
                          TCorrection_BiasMapErr[i][make_tuple(i_shape, i_pk % 4, i_axis, i_angleU, i_angleV)] / Unit::um);
                  fprintf(AscFile, " case %i : %7.2f +- %7.2f ,", i,
                          TCorrection_BiasMap[i][make_tuple(i_shape, i_pk % 4 + 4, i_axis, i_angleU, i_angleV)] / Unit::um,
                          TCorrection_BiasMapErr[i][make_tuple(i_shape, i_pk % 4 + 4, i_axis, i_angleU, i_angleV)] / Unit::um);
                }
                if ((i > 0) && (i < 5)) {
                  if (i == i_pk - 4 + 1) {
                    fprintf(AscFile, " case %i : %7.2f +- %7.2f ,", i,
                            TCorrection_BiasMap[i][make_tuple(i_shape, i_pk, i_axis, i_angleU, i_angleV)] / Unit::um,
                            TCorrection_BiasMapErr[i][make_tuple(i_shape, i_pk, i_axis, i_angleU, i_angleV)] / Unit::um);
                  } else if (i == i_pk + 1) {
                    fprintf(AscFile, " case %i : %7.2f +- %7.2f ,", i,
                            TCorrection_BiasMap[i][make_tuple(i_shape, i_pk + 4, i_axis, i_angleU, i_angleV)] / Unit::um,
                            TCorrection_BiasMapErr[i][make_tuple(i_shape, i_pk + 4, i_axis, i_angleU, i_angleV)] / Unit::um);
                  }
                }
                if (i == 5) {
                  fprintf(AscFile, " case %i : %7.2f +- %7.2f ,", i,
                          TCorrection_BiasMap[i][make_tuple(i_shape, i_pk % 4, i_axis, i_angleU, i_angleV)] / Unit::um,
                          TCorrection_BiasMapErr[i][make_tuple(i_shape, i_pk % 4, i_axis, i_angleU, i_angleV)] / Unit::um);
                  fprintf(AscFile, " case %i : %7.2f +- %7.2f ,", i,
                          TCorrection_BiasMap[i][make_tuple(i_shape, i_pk % 4 + 4, i_axis, i_angleU, i_angleV)] / Unit::um,
                          TCorrection_BiasMapErr[i][make_tuple(i_shape, i_pk % 4 + 4, i_axis, i_angleU, i_angleV)] / Unit::um);
                }
              }
              fprintf(AscFile, "\n");
              fprintf(AscFile, "                                      : ErEs");
              for (int i = 0; i < 6; i++) {
                if (i == 0) {
                  fprintf(AscFile, " case %i : %7.2f            ,", i,
                          TCorrection_ErrorEstimationMap[i][make_tuple(i_shape, i_pk, i_axis, i_angleU, i_angleV)]);
                }
                if ((i > 0) && (i < 5)) {
                  if (i == i_pk - 4 + 1) {
                    fprintf(AscFile, " case %i : %7.2f            ,", i,
                            TCorrection_ErrorEstimationMap[i][make_tuple(i_shape, i_pk, i_axis, i_angleU, i_angleV)]);
                  } else if (i == i_pk + 1) {
                    fprintf(AscFile, " case %i : %7.2f            ,", i,
                            TCorrection_ErrorEstimationMap[i][make_tuple(i_shape, i_pk, i_axis, i_angleU, i_angleV)]);
                  }
                }
                if (i == 5) {
                  fprintf(AscFile, " case %i : %7.2f            ,", i,
                          TCorrection_ErrorEstimationMap[i][make_tuple(i_shape, i_pk, i_axis, i_angleU, i_angleV)]);
                }
              }
              fprintf(AscFile, "\n");
            }  // dimension (axis U, V)
          }  // angle V
        }  // angle U
      }  // pixel kind
    }  // shape
    fclose(AscFile);
    */
}

void pxdApplyClusterShapeCorrectionModule::event()
{
  //Obtain all StoreArrays
  //const StoreArray<PXDCluster> storeClusters;
  if (m_ExistCorrectionBasic == 0) return;
  StoreArray<genfit::Track> tracks(m_storeTrackName);
  VxdID sensorID(0);

  for (auto track : tracks) {  // over tracks
    for (unsigned int ipoint = 0; ipoint < track.getNumPointsWithMeasurement(); ++ipoint) {  // over track points
      if (PXDRecoHit* pxdhit = dynamic_cast<PXDRecoHit*>(track.getPointWithMeasurement(ipoint)->getRawMeasurement(0))) { // cluster
        //const PXDCluster& cluster = * pxdhit->getCluster();
        //const PXDCluster* cluster = pxdhit->getCluster();
        PXDCluster* cluster = (PXDCluster*)pxdhit->getCluster();
        if (!m_DoExpertHistograms) if (cluster->getShape() < 0) continue;
        //if (m_MarkOfLoopForHistogramsFile == 1) {
        //  TString TextSh = Form("ERROR!!! - Shape should be negative - but is = %i !!!!", cluster->getShape());
        //  B2INFO(TextSh.Data());
        //}
        int DoCorrection = 1;
        if (cluster->getShape() < 0) DoCorrection = 0; // do only monitoring histograms
        //TVectorD state = track.getPointWithMeasurement(ipoint)->getFitterInfo()->getFittedState(true).getState();
        //double f_TrackUIncluded;
        //f_TrackUIncluded = state[3];
        //double f_TrackVIncluded;
        //f_TrackVIncluded = state[4];
        bool biased = false; // excluded residuals
        //biased = true;       // included residuals
        //TVectorD residual = track.getPointWithMeasurement(ipoint)->getFitterInfo()->getResidual(0, biased).getState();
        TVectorD state2 = track.getPointWithMeasurement(ipoint)->getFitterInfo()->getFittedState(biased).getState();
        double f_phiTrack = state2[1];
        double f_thetaTrack = state2[2];
        f_phiTrack = TMath::ATan2(f_phiTrack, 1.0);
        f_thetaTrack = TMath::ATan2(f_thetaTrack, 1.0);
        double f_TrackU = state2[3];
        double f_TrackV = state2[4];

        TMatrixDSym covarianceTR = track.getPointWithMeasurement(ipoint)->getFitterInfo()->getResidual(0, biased).getCov();
        double SigmaUTrack = sqrt(covarianceTR(0, 0) - (cluster->getUSigma() * cluster->getUSigma()));
        double SigmaVTrack = sqrt(covarianceTR(1, 1) - (cluster->getVSigma() * cluster->getVSigma()));
        //TMatrixDSym covarianceTRIncluded = track.getPointWithMeasurement(ipoint)->getFitterInfo()->getResidual(0, true).getCov();
        //double SigmaUTrackIncl = sqrt(covarianceTRIncluded(0, 0) - cluster->getUSigma() * cluster->getUSigma());
        //double SigmaVTrackIncl = sqrt(covarianceTRIncluded(1, 1) - cluster->getVSigma() * cluster->getVSigma());
        //TVectorD residual = track.getPointWithMeasurement(ipoint)->getFitterInfo()->getResidual(0, true).getState();
        // m_ResidUTrack = cluster.getU() - state[3];
        // m_ResidVTrack = cluster.getV() - state[4];
        //ResUIncl = f_TrackUIncluded - cluster->getU();
        //m_ResidUTrack = residual.GetMatrixArray()[0];
        //m_ResidVTrack = residual.GetMatrixArray()[1];
        //printf("---> %f %f --- %f %f  (Tr: excl:%f incl: %f)\n",
        //       cluster->getU() - state[3], cluster->getV() - state[4], residual.GetMatrixArray()[0], residual.GetMatrixArray()[1],
//               SigmaUTrack, TMath::Sqrt(f_TrackU*f_TrackU - f_TrackUIncluded*f_TrackUIncluded)
//               f_TrackU, TMath::Sqrt(SigmaUTrack*SigmaUTrack + f_TrackUIncluded*f_TrackUIncluded)
//               f_TrackU, SigmaUTrack + f_TrackUIncluded
//               f_TrackU - cluster->getU(), TMath::Sqrt(SigmaUTrack*SigmaUTrack + (f_TrackUIncluded - cluster->getU())*(f_TrackUIncluded - cluster->getU()))
        //       SigmaUTrack, SigmaUTrackIncl
        //      );

//        TVectorD track = track.getPointWithMeasurement(ipoint)->getFitterInfo()->getResidual(0, biased).getState();

        // TODO remeove those two lines - now correction of error in input datas (slope insteed angle)
        //f_phiTrack = TMath::ATan2(f_phiTrack, 1.0);
        //f_thetaTrack = TMath::ATan2(f_thetaTrack, 1.0);

        //if ((fabs(m_phiTrack / Unit::deg) > 90.0) || (fabs(m_thetaTrack / Unit::deg) > 90.0))printf("--------------------> %f %f %f %f slope --> %f %f deg\n",m_phiTrack / Unit::deg,m_thetaTrack / Unit::deg,
        //       TMath::Tan(TMath::ATan2(m_phiTrack,1.0)) / Unit::deg, TMath::Tan(TMath::ATan2(m_thetaTrack,1.0)) / Unit::deg,
        //       TMath::ATan2(m_phiTrack,1.0) / Unit::deg, TMath::ATan2(m_thetaTrack,1.0) / Unit::deg );

        //((TMath::Pi() * i_angleV) / m_anglesV) - (TMath::Pi() / 2.0)
        int iIndexPhi = (f_phiTrack + (TMath::Pi() / 2.0)) / (TMath::Pi() / m_anglesU);
        int iIndexTheta = (f_thetaTrack + (TMath::Pi() / 2.0)) / (TMath::Pi() / m_anglesV);
        //printf("----> %i   %f (%f)   %i   %f (%f) \n", iIndexPhi, f_phiTrack, f_phiTrack / TMath::Pi() * 180.0, iIndexTheta, f_thetaTrack, f_thetaTrack / TMath::Pi() * 180.0);
        //Get Geometry information
        sensorID = cluster->getSensorID();
        const PXD::SensorInfo& Info = dynamic_cast<const PXD::SensorInfo&>(VXD::GeoCache::get(
                                        sensorID));
        int i_layer = Info.getID().getLayerNumber();
        int i_sensor = Info.getID().getSensorNumber();
        //int InspectDets = 0;
        //if ((cluster->getShape() >= 11) && (cluster->getShape() <= 14)) InspectDets = 1;
        //if (!InspectDets) continue;
        //if (InspectDets) printf("------->Shape %i (%s)\n",cluster->getShape(), Belle2::PXD::PXDClusterShape::pxdClusterShapeDescription[(Belle2::PXD::pxdClusterShapeType)cluster->getShape()].c_str());
        int i_shape = cluster->getShape() - 1;
        if (i_shape < 0) i_shape = i_shape + 100;
        if (i_shape < 0) continue;
        //if (i_shape >= m_shapes) continue;

        // TODO set it more systematically? m_closeEdge values
        int EdgePixelSizeV1 = 512;   // TODO assign correctly from geometry/design
        int EdgePixelSizeV2 = 256;   // TODO assign correctly from geometry/design
        int EdgePixelSizeU = 256;    // TODO assign correctly from geometry/design
        int m_EdgeClose = 3;
        int m_closeEdge = 0;  // 0: healthy, far from edge or masked, 1: close edge, 2: close masked pixel
        if ((i_sensor == 2) && (abs(Info.getVCellID(cluster->getV()) - EdgePixelSizeV1) < m_EdgeClose)) m_closeEdge = 1;
        else if ((i_sensor == 1) && (abs(Info.getVCellID(cluster->getV()) - EdgePixelSizeV2) < m_EdgeClose)) m_closeEdge = 1;
        else if (Info.getVCellID(cluster->getV()) < m_EdgeClose) m_closeEdge = 1;
        else if ((EdgePixelSizeV1 + EdgePixelSizeV2 - Info.getVCellID(cluster->getV())) < m_EdgeClose) m_closeEdge = 1;
        else if (Info.getUCellID(cluster->getU()) < m_EdgeClose) m_closeEdge = 1;
        else if (EdgePixelSizeU - Info.getUCellID(cluster->getU()) < m_EdgeClose) m_closeEdge = 1;

        int i_pixelKind = 0;  // 0: smaller pixelsize, 1: larger pixelsize TODO set it more systematically?
        if ((i_sensor == 2) && (Info.getVCellID(cluster->getV()) < EdgePixelSizeV1)) i_pixelKind = 1;
        else if ((i_sensor == 1) && (Info.getVCellID(cluster->getV()) >= EdgePixelSizeV2)) i_pixelKind = 1;
        if (i_layer == 2) i_pixelKind += 2;
        if (i_sensor == 2) i_pixelKind += 4;



        //printf("---> %i: s %i l %i v: %i p: %4.1f, v %f uhelV %f (atan: %f)\n",
        //       i_pixelKind,i_sensor,i_layer,Info.getVCellID(cluster->getV()),Info.getVPitch(cluster->getV()) * 10000.0, cluster->getV(),
        //       f_thetaTrack * 180.0 / TMath::Pi(), TMath::ATan2(f_thetaTrack, 1.0) * 180.0 / TMath::Pi()
        //      );
        //continue;
        //if (i_sensor == 1) continue;  // special condition to check..... TODO remove later

        float ResU = 0;
        float ResV = 0;
        float EEU = 0;
        float EEV = 0;
        float f_SigmaU = cluster->getUSigma();
        float f_SigmaV = cluster->getVSigma();
        float Clsu = cluster->getU();
        float Clsv = cluster->getV();
        if (m_closeEdge  == 0) {
          NClusters++;
          //if (InspectDets) printf("------->Not close edge\n");
          if (m_DoExpertHistograms) {
            ResU = f_TrackU - Clsu;
            ResV = f_TrackV - Clsv;
            EEU = ResU / TMath::Sqrt(f_SigmaU * f_SigmaU + SigmaUTrack * SigmaUTrack);
            EEV = ResV / TMath::Sqrt(f_SigmaV * f_SigmaV + SigmaVTrack * SigmaVTrack);
            //if (InspectDets) printf("------->ResU %f , ResV %f\n", 10000.0 * ResU, 10000.0 * ResV );
            B2DEBUG(130, "--------------------------------> before: " << Clsu << ", ResU " << ResU << ", ResV " << ResV << ", ind1 "
                    <<
                    0 * (m_pixelkinds * m_shapes + 1) + i_pixelKind * m_shapes + i_shape <<
                    ", ind2 " << 0 * (m_pixelkinds * m_shapes + 1) + m_pixelkinds * m_shapes);
            m_histResidualU[0 * (m_pixelkinds * m_shapes + 1) + i_pixelKind * m_shapes + i_shape]->Fill(ResU / Unit::um);
            m_histResidualV[0 * (m_pixelkinds * m_shapes + 1) + i_pixelKind * m_shapes + i_shape]->Fill(ResV / Unit::um);
            m_histResidualUV[0 * (m_pixelkinds * m_shapes + 1) + i_pixelKind * m_shapes + i_shape]->Fill(ResU / Unit::um, ResV / Unit::um);
            m_histNormErrorU[0 * (m_pixelkinds * m_shapes + 1) + i_pixelKind * m_shapes + i_shape]->Fill(EEU);
            m_histNormErrorV[0 * (m_pixelkinds * m_shapes + 1) + i_pixelKind * m_shapes + i_shape]->Fill(EEV);
            m_histNormErrorUV[0 * (m_pixelkinds * m_shapes + 1) + i_pixelKind * m_shapes + i_shape]->Fill(EEU, EEV);

            m_histResidualU[0 * (m_pixelkinds * m_shapes + 1) + m_pixelkinds * m_shapes]->Fill(ResU / Unit::um);
            m_histResidualV[0 * (m_pixelkinds * m_shapes + 1) + m_pixelkinds * m_shapes]->Fill(ResV / Unit::um);
            m_histResidualUV[0 * (m_pixelkinds * m_shapes + 1) + m_pixelkinds * m_shapes]->Fill(ResU / Unit::um, ResV / Unit::um);
            m_histNormErrorU[0 * (m_pixelkinds * m_shapes + 1) + m_pixelkinds * m_shapes]->Fill(EEU);
            m_histNormErrorV[0 * (m_pixelkinds * m_shapes + 1) + m_pixelkinds * m_shapes]->Fill(EEV);
            m_histNormErrorUV[0 * (m_pixelkinds * m_shapes + 1) + m_pixelkinds * m_shapes]->Fill(EEU, EEV);
          }
          if (DoCorrection) {
            for (int i_axis = 0; i_axis < m_dimensions; i_axis++) {
              if (TCorrection_BiasMap[0][make_tuple(i_shape, i_pixelKind, i_axis, iIndexPhi, iIndexTheta)] != 0.0) {  // use basic corrections:
                NClustersBasicCorBias[i_axis]++;
              } else {  // use simulation corrections:
                NClustersSimulationCorBias[i_axis]++;
              }
              if (i_axis == 0) {
                Clsu -= TCorrection_BiasMap[5][make_tuple(i_shape, i_pixelKind, i_axis, iIndexPhi, iIndexTheta)];
                cluster->setU(Clsu);
              }
              if (i_axis == 1) {
                Clsv -= TCorrection_BiasMap[5][make_tuple(i_shape, i_pixelKind, i_axis, iIndexPhi, iIndexTheta)];
                cluster->setV(Clsv);
              }
              if (TCorrection_ErrorEstimationMap[0][make_tuple(i_shape, i_pixelKind, i_axis, iIndexPhi, iIndexTheta)] != 1.0) {
                NClustersBasicCorErEst[i_axis]++;
              } else {  // use simulation corrections:
                NClustersSimulationCorErEst[i_axis]++;
              }
              if (i_axis == 0) {
                f_SigmaU *= TCorrection_ErrorEstimationMap[5][make_tuple(i_shape, i_pixelKind, i_axis, iIndexPhi, iIndexTheta)];
                cluster->setUSigma(f_SigmaU);
              }
              if (i_axis == 1) {
                f_SigmaV *= TCorrection_ErrorEstimationMap[5][make_tuple(i_shape, i_pixelKind, i_axis, iIndexPhi, iIndexTheta)];
                cluster->setVSigma(f_SigmaV);
              }
            }
          }
          if (m_DoExpertHistograms) {
            ResU = f_TrackU - Clsu;
            ResV = f_TrackV - Clsv;
            EEU = ResU / TMath::Sqrt(f_SigmaU * f_SigmaU + SigmaUTrack * SigmaUTrack);
            EEV = ResV / TMath::Sqrt(f_SigmaV * f_SigmaV + SigmaVTrack * SigmaVTrack);
            //ResUIncl = f_TrackUIncluded - cluster->getU();
            //ResVIncl = f_TrackVIncluded - cluster->getV();
            //if (InspectDets) printf("------->ResU %f , ResV %f\n", 10000.0 * ResU, 10000.0 * ResV );
            B2DEBUG(130, "--------------------------------> After: " << Clsu << ", ResU " << ResU << ", ResV " << ResV << ", ind1 "
                    <<
                    1 * (m_pixelkinds * m_shapes + 1) + i_pixelKind * m_shapes + i_shape <<
                    ", ind2 " << 1 * (m_pixelkinds * m_shapes + 1) + m_pixelkinds * m_shapes);
            m_histResidualU[1 * (m_pixelkinds * m_shapes + 1) + i_pixelKind * m_shapes + i_shape]->Fill(ResU / Unit::um);
            m_histResidualV[1 * (m_pixelkinds * m_shapes + 1) + i_pixelKind * m_shapes + i_shape]->Fill(ResV / Unit::um);
            m_histResidualUV[1 * (m_pixelkinds * m_shapes + 1) + i_pixelKind * m_shapes + i_shape]->Fill(ResU / Unit::um, ResV / Unit::um);
            m_histNormErrorU[1 * (m_pixelkinds * m_shapes + 1) + i_pixelKind * m_shapes + i_shape]->Fill(EEU);
            m_histNormErrorV[1 * (m_pixelkinds * m_shapes + 1) + i_pixelKind * m_shapes + i_shape]->Fill(EEV);
            m_histNormErrorUV[1 * (m_pixelkinds * m_shapes + 1) + i_pixelKind * m_shapes + i_shape]->Fill(EEU, EEV);

            m_histResidualU[1 * (m_pixelkinds * m_shapes + 1) + m_pixelkinds * m_shapes]->Fill(ResU / Unit::um);
            m_histResidualV[1 * (m_pixelkinds * m_shapes + 1) + m_pixelkinds * m_shapes]->Fill(ResV / Unit::um);
            m_histResidualUV[1 * (m_pixelkinds * m_shapes + 1) + m_pixelkinds * m_shapes]->Fill(ResU / Unit::um, ResV / Unit::um);
            m_histNormErrorU[1 * (m_pixelkinds * m_shapes + 1) + m_pixelkinds * m_shapes]->Fill(EEU);
            m_histNormErrorV[1 * (m_pixelkinds * m_shapes + 1) + m_pixelkinds * m_shapes]->Fill(EEV);
            m_histNormErrorUV[1 * (m_pixelkinds * m_shapes + 1) + m_pixelkinds * m_shapes]->Fill(EEU, EEV);
          }
        }
        if (DoCorrection) cluster->setShape(-100 + i_shape + 1); // 1 -> -99,...
      }
    }
  }
}

void pxdApplyClusterShapeCorrectionModule::terminate()
{
  if (m_DoExpertHistograms) {

    // ******************* Show some statistcs and save to asci file: ********************************
    TString TextSh;
    TextSh = Form("CorrectionsApplyingStatistics_%i.log", m_MarkOfLoopForHistogramsFile);
    FILE* AscFile = fopen(TextSh.Data(), "w");

    TextSh = Form("*******************************************************************");
    B2INFO(TextSh.Data()); fprintf(AscFile, "%s\n", TextSh.Data());
    TextSh = Form("**");
    B2INFO(TextSh.Data()); fprintf(AscFile, "%s\n", TextSh.Data());
    TextSh = Form("**                        Using Clusters: %i", NClusters);
    B2INFO(TextSh.Data()); fprintf(AscFile, "%s\n", TextSh.Data());
    TextSh = Form("**       Bias corrections from real data: U %i ( %5.1f %%), V %i ( %5.1f %%)",
                  NClustersBasicCorBias[0], (float)NClustersBasicCorBias[0] / NClusters * 100.0,
                  NClustersBasicCorBias[1], (float)NClustersBasicCorBias[1] / NClusters * 100.0);
    B2INFO(TextSh.Data()); fprintf(AscFile, "%s\n", TextSh.Data());
    TextSh = Form("**      Bias corrections from simulation: U %i ( %5.1f %%), V %i ( %5.1f %%)",
                  NClustersSimulationCorBias[0], (float)NClustersSimulationCorBias[0] / NClusters * 100.0,
                  NClustersSimulationCorBias[1], (float)NClustersSimulationCorBias[1] / NClusters * 100.0);
    B2INFO(TextSh.Data()); fprintf(AscFile, "%s\n", TextSh.Data());
    TextSh = Form("**  Err Estim corrections from real data: U %i ( %5.1f %%), V %i ( %5.1f %%)",
                  NClustersBasicCorErEst[0], (float)NClustersBasicCorErEst[0] / NClusters * 100.0,
                  NClustersBasicCorErEst[1], (float)NClustersBasicCorErEst[1] / NClusters * 100.0);
    B2INFO(TextSh.Data()); fprintf(AscFile, "%s\n", TextSh.Data());
    TextSh = Form("** Err Estim corrections from simulation: U %i ( %5.1f %%), V %i ( %5.1f %%)",
                  NClustersSimulationCorErEst[0], (float)NClustersSimulationCorErEst[0] / NClusters * 100.0,
                  NClustersSimulationCorErEst[1], (float)NClustersSimulationCorErEst[1] / NClusters * 100.0);
    B2INFO(TextSh.Data()); fprintf(AscFile, "%s\n", TextSh.Data());

    TextSh = Form("**");
    B2INFO(TextSh.Data()); fprintf(AscFile, "%s\n", TextSh.Data());
    B2INFO(TextSh.Data()); fprintf(AscFile, "%s\n", TextSh.Data());
    TextSh = Form("*******************************************************************");
    B2INFO(TextSh.Data()); fprintf(AscFile, "%s\n", TextSh.Data());
    TextSh = Form("**          Mean and RMS for Pixel Kinds in Shapes");
    if (m_ShowDetailStatistics) B2INFO(TextSh.Data()); fprintf(AscFile, "%s\n", TextSh.Data());
    TextSh = Form("**");
    if (m_ShowDetailStatistics) B2INFO(TextSh.Data()); fprintf(AscFile, "%s\n", TextSh.Data());
    //if (i_UseCor == 0) UseCorrs = Form("BeforeCors");
    //if (i_UseCor == 1) UseCorrs = Form("AfterCors");
    TString UseCorrs;
    for (int i_pk = 0; i_pk < m_pixelkinds; i_pk++) {
      TextSh = Form("**  Pixel Kind %i (Layer %i, Sensor %i, Size %i)", i_pk, (int)((i_pk % 4) / 2) + 1, (int)(i_pk / 4) + 1, i_pk % 2);
      if (m_ShowDetailStatistics) B2INFO(TextSh.Data()); fprintf(AscFile, "%s\n", TextSh.Data());
      for (int i_shape = 0; i_shape < m_shapes + 1; i_shape++) {
        TextSh = Form("**   %s ShapeID %02i: events %7i  (%6.2f %%) (%s)",
                      m_histResidualU[0 * (m_pixelkinds * m_shapes + 1) + i_pk * m_shapes + i_shape]->GetName(),
                      i_shape + 1,
                      (int)m_histResidualU[0 * (m_pixelkinds * m_shapes + 1) + i_pk * m_shapes + i_shape]->GetEntries(),
                      (float)m_histResidualU[0 * (m_pixelkinds * m_shapes + 1) + i_pk * m_shapes + i_shape]->GetEntries() / NClusters,
                      Belle2::PXD::PXDClusterShape::pxdClusterShapeDescription[(Belle2::PXD::pxdClusterShapeType)(i_shape + 1)].c_str());
        if (m_ShowDetailStatistics) B2INFO(TextSh.Data()); fprintf(AscFile, "%s\n", TextSh.Data());
        if ((int)m_histResidualU[0 * (m_pixelkinds * m_shapes + 1) + i_pk * m_shapes + i_shape]->GetEntries() > 1) {
          TextSh = Form("**       RMS Before: U  %5.2f , V %5.2f , After: U %5.2f , V %5.2f",
                        m_histNormErrorU[0 * (m_pixelkinds * m_shapes + 1) + i_pk * m_shapes + i_shape]->GetRMS(),
                        m_histNormErrorV[0 * (m_pixelkinds * m_shapes + 1) + i_pk * m_shapes + i_shape]->GetRMS(),
                        m_histNormErrorU[1 * (m_pixelkinds * m_shapes + 1) + i_pk * m_shapes + i_shape]->GetRMS(),
                        m_histNormErrorV[1 * (m_pixelkinds * m_shapes + 1) + i_pk * m_shapes + i_shape]->GetRMS());
        } else {
          TextSh = Form("**       RMS Before: U  %5.2f , V %5.2f , After: U %5.2f , V %5.2f",
                        0.0, 0.0, 0.0, 0.0);
        }
        if (m_ShowDetailStatistics) B2INFO(TextSh.Data()); fprintf(AscFile, "%s\n", TextSh.Data());
        if ((int)m_histResidualU[0 * (m_pixelkinds * m_shapes + 1) + i_pk * m_shapes + i_shape]->GetEntries() > 1) {
          TextSh = Form("**       Bias Before: U %5.2f +- %5.2f um, V %5.2f +- %5.2f um, After: U %5.2f +- %5.2f um, V %5.2f +- %5.2f um",
                        m_histResidualU[0 * (m_pixelkinds * m_shapes + 1) + i_pk * m_shapes + i_shape]->GetMean(),
                        m_histResidualU[0 * (m_pixelkinds * m_shapes + 1) + i_pk * m_shapes + i_shape]->GetRMS(),
                        m_histResidualV[0 * (m_pixelkinds * m_shapes + 1) + i_pk * m_shapes + i_shape]->GetMean(),
                        m_histResidualV[0 * (m_pixelkinds * m_shapes + 1) + i_pk * m_shapes + i_shape]->GetRMS(),
                        m_histResidualU[1 * (m_pixelkinds * m_shapes + 1) + i_pk * m_shapes + i_shape]->GetMean(),
                        m_histResidualU[1 * (m_pixelkinds * m_shapes + 1) + i_pk * m_shapes + i_shape]->GetRMS(),
                        m_histResidualV[1 * (m_pixelkinds * m_shapes + 1) + i_pk * m_shapes + i_shape]->GetMean(),
                        m_histResidualV[1 * (m_pixelkinds * m_shapes + 1) + i_pk * m_shapes + i_shape]->GetRMS());
        } else {
          TextSh = Form("**       Bias Before: U %5.2f +- %5.2f um, V %5.2f +- %5.2f um, After: U %5.2f +- %5.2f um, V %5.2f +- %5.2f um",
                        0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0);
        }
        if (m_ShowDetailStatistics) B2INFO(TextSh.Data()); fprintf(AscFile, "%s\n", TextSh.Data());
        TextSh = Form("**");
        if (m_ShowDetailStatistics) B2INFO(TextSh.Data()); fprintf(AscFile, "%s\n", TextSh.Data());
      }
      TextSh = Form("**");
      if (m_ShowDetailStatistics) B2INFO(TextSh.Data()); fprintf(AscFile, "%s\n", TextSh.Data());
    }
    TextSh = Form("**");
    if (m_ShowDetailStatistics) B2INFO(TextSh.Data()); fprintf(AscFile, "%s\n", TextSh.Data());
    if (m_ShowDetailStatistics) B2INFO(TextSh.Data()); fprintf(AscFile, "%s\n", TextSh.Data());
    TextSh = Form("*******************************************************************");
    if (m_ShowDetailStatistics) B2INFO(TextSh.Data()); fprintf(AscFile, "%s\n", TextSh.Data());
    fclose(AscFile);
    // ******************* END show some statistcs and save to asci file: ********************************

    fExpertHistograms->Write();
    fExpertHistograms->Close();
  }
}

