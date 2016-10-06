/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Peter Kodys                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <pxd/modules/pxdClusterShapeCalibration/pxdCheckClusterShapeCorrectionModule.h>

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
REG_MODULE(pxdCheckClusterShapeCorrection);

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

pxdCheckClusterShapeCorrectionModule::pxdCheckClusterShapeCorrectionModule() : Module()
{
  //Set module properties
  setDescription("Apply PXD Cluster Shape Calibration");
  setPropertyFlags(c_ParallelProcessingCertified);

  addParam("PrefereSimulation", m_PrefereSimulation,
           "To use simulations rather than real data calculated corrections, default=False", m_PrefereSimulation);

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

}

void pxdCheckClusterShapeCorrectionModule::initialize()
{
  // Only over this limit is correction accepted
  float fDifference = 0.1 * Unit::um;
  // Only under this limit is real bias correction compare to simulation accepted
  float fDifferenceClose = 3 * Unit::um;
  // Only under this limit is real error estimation correction compare to simulation accepted
  float fDifferenceErrEst = 0.1;

  // START - load corrections:

  B2INFO("pxdCheckClusterShapeCorrection: Load correction files:");

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
    B2INFO("pxdCheckClusterShapeCorrection: exist correction file Basic: " << m_CalFileBasicName.c_str());
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
    B2INFO("pxdCheckClusterShapeCorrection:  ---> Field dimensions: shapes: " << m_shapes << ", pixelkinds: " << m_pixelkinds <<
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
    B2INFO("pxdCheckClusterShapeCorrection: exist correction file PK0: " << m_CalFilePK0Name.c_str());
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
    if (m_ExistCorrectionPK0 == 0) B2INFO("pxdCheckClusterShapeCorrection: ERROR in PK0 file! differences on field dimensions.");
    f_CalBasic->Close();
  }
  f_CalBasic = new TFile(m_CalFilePK1Name.c_str(), "read");
  iLoad = 2;
  if (f_CalBasic->IsOpen()) {
    m_ExistCorrectionPK1 = 1;
    B2INFO("pxdCheckClusterShapeCorrection: exist correction file PK1: " << m_CalFilePK1Name.c_str());
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
    if (m_ExistCorrectionPK1 == 0) B2INFO("pxdCheckClusterShapeCorrection: ERROR in PK1 file! differences on field dimensions.");
    f_CalBasic->Close();
  }
  f_CalBasic = new TFile(m_CalFilePK2Name.c_str(), "read");
  iLoad = 3;
  if (f_CalBasic->IsOpen()) {
    m_ExistCorrectionPK2 = 1;
    B2INFO("pxdCheckClusterShapeCorrection: exist correction file PK2: " << m_CalFilePK2Name.c_str());
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
    if (m_ExistCorrectionPK2 == 0) B2INFO("pxdCheckClusterShapeCorrection: ERROR in PK2 file! differences on field dimensions.");
    f_CalBasic->Close();
  }
  f_CalBasic = new TFile(m_CalFilePK3Name.c_str(), "read");
  iLoad = 4;
  if (f_CalBasic->IsOpen()) {
    m_ExistCorrectionPK3 = 1;
    B2INFO("pxdCheckClusterShapeCorrection: exist correction file PK3: " << m_CalFilePK3Name.c_str());
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
    if (m_ExistCorrectionPK3 == 0) B2INFO("pxdCheckClusterShapeCorrection: ERROR in PK3 file! differences on field dimensions.");
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
              if ((fabs(TCorrection_BiasMap[0][make_tuple(i_shape, i_pk, i_axis, i_angleU, i_angleV)]) > fDifference) &&
                  (fabs(TCorrection_BiasMap[5][make_tuple(i_shape, i_pk, i_axis, i_angleU, i_angleV)]) > fDifference)) {
                if (fabs(TCorrection_BiasMap[5][make_tuple(i_shape, i_pk, i_axis, i_angleU, i_angleV)] - TCorrection_BiasMap[0][make_tuple(i_shape,
                         i_pk, i_axis, i_angleU, i_angleV)]) < fDifferenceClose) {
                  TCorrection_BiasMap[5][make_tuple(i_shape, i_pk, i_axis, i_angleU, i_angleV)] = TCorrection_BiasMap[0][make_tuple(i_shape, i_pk,
                      i_axis, i_angleU, i_angleV)];
                  TCorrection_BiasMapErr[5][make_tuple(i_shape, i_pk, i_axis, i_angleU, i_angleV)] = TCorrection_BiasMapErr[0][make_tuple(i_shape,
                      i_pk, i_axis, i_angleU, i_angleV)];
                }
              }
              if ((fabs(TCorrection_ErrorEstimationMap[0][make_tuple(i_shape, i_pk, i_axis, i_angleU, i_angleV)] - 1.0) > fDifference) &&
                  (fabs(TCorrection_ErrorEstimationMap[5][make_tuple(i_shape, i_pk, i_axis, i_angleU, i_angleV)] - 1.0) > fDifference)) {
                if (fabs(TCorrection_ErrorEstimationMap[5][make_tuple(i_shape, i_pk, i_axis, i_angleU,
                                                                      i_angleV)] - TCorrection_ErrorEstimationMap[0][make_tuple(i_shape, i_pk, i_axis, i_angleU, i_angleV)]) < fDifferenceErrEst) {
                  TCorrection_ErrorEstimationMap[5][make_tuple(i_shape, i_pk, i_axis, i_angleU,
                                                               i_angleV)] = TCorrection_ErrorEstimationMap[0][make_tuple(i_shape, i_pk, i_axis, i_angleU, i_angleV)];
                }
              }
            }
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

  // ******************* Save corrections to asci file: ********************************
  FILE* AscFileAll = fopen("pxdClShCorrections_Summary_All.txt", "w");
  FILE* AscFileNonZeroBoth = fopen("pxdClShCorrections_Summary_NonZeroBoth.txt", "w");
  FILE* AscFileCloseValue = fopen("pxdClShCorrections_Summary_CloseValue.txt", "w");
  FILE* AscFileDiffValue = fopen("pxdClShCorrections_Summary_DiffValue.txt", "w");
  TString sSave;
  sSave = Form("Storring of all nonZero corrections for Bias and its error (in microns), and ErrorEstimation Corrections different from 1.0\n\n");
  fprintf(AscFileAll, "%s", sSave.Data());
  fprintf(AscFileNonZeroBoth, "%s", sSave.Data());
  fprintf(AscFileCloseValue, "%s", sSave.Data());
  fprintf(AscFileDiffValue, "%s", sSave.Data());

  for (int i_pk = 0; i_pk < m_pixelkinds; i_pk++) {
    for (int i_shape = 0; i_shape < m_shapes; i_shape++) {
      for (int i_angleU = 0; i_angleU < m_anglesU; i_angleU++) {
        for (int i_angleV = 0; i_angleV < m_anglesV; i_angleV++) {
          for (int i_axis = 0; i_axis < m_dimensions; i_axis++) {
            int NonZeroBoth = 1;
            int CloseValue = 1;
            int DiffValue = 1;
            if (!((TCorrection_BiasMap[0][make_tuple(i_shape, i_pk, i_axis, i_angleU, i_angleV)]  != 0.0)
                  && (TCorrection_BiasMap[5][make_tuple(i_shape, i_pk, i_axis, i_angleU, i_angleV)]  != 0.0))) {
              NonZeroBoth = 0;
            }
            if (TCorrection_BiasMap[0][make_tuple(i_shape, i_pk, i_axis, i_angleU, i_angleV)]  == 0.0) {
              CloseValue = 0;
              DiffValue = 0;
            }
            if (TCorrection_BiasMap[(i_pk % 4) + 1][make_tuple(i_shape, (i_pk % 4) + 4, i_axis, i_angleU, i_angleV)]  == 0.0) {
              CloseValue = 0;
              DiffValue = 0;
            }
            if (CloseValue == 1) {
              float v1 = TCorrection_BiasMap[0][make_tuple(i_shape, (i_pk % 4), i_axis, i_angleU, i_angleV)];
              float v2 = TCorrection_BiasMap[0][make_tuple(i_shape, (i_pk % 4) + 4, i_axis, i_angleU, i_angleV)];
              if (v1 == 0.0) {
                if (fabs(v2 - TCorrection_BiasMap[(i_pk % 4) + 1][make_tuple(i_shape, (i_pk % 4) + 4, i_axis, i_angleU,
                                                                  i_angleV)]) > fDifferenceClose) {
                  CloseValue = 0;
                } else {
                  DiffValue = 0;
                }
              } else if (v2 == 0.0) {
                if (fabs(v1 - TCorrection_BiasMap[(i_pk % 4) + 1][make_tuple(i_shape, (i_pk % 4) + 4, i_axis, i_angleU,
                                                                  i_angleV)]) > fDifferenceClose) {
                  CloseValue = 0;
                } else {
                  DiffValue = 0;
                }
              } else if ((fabs(v1 - TCorrection_BiasMap[(i_pk % 4) + 1][make_tuple(i_shape, (i_pk % 4) + 4, i_axis, i_angleU,
                                                                        i_angleV)]) > fDifferenceClose) ||
                         (fabs(v2 - TCorrection_BiasMap[(i_pk % 4) + 1][make_tuple(i_shape, (i_pk % 4) + 4, i_axis, i_angleU,
                                                                        i_angleV)]) > fDifferenceClose)) {
                CloseValue = 0;
              } else {
                DiffValue = 0;
              }
            }
            sSave = Form("PixKind %1i Sh %2i AngU %2i AngV %2i Dir %1i : Bias", i_pk, i_shape, i_angleU, i_angleV, i_axis);
            fprintf(AscFileAll, "%s", sSave.Data());
            if (NonZeroBoth) fprintf(AscFileNonZeroBoth, "%s", sSave.Data());
            if (CloseValue) fprintf(AscFileCloseValue, "%s", sSave.Data());
            if (DiffValue) fprintf(AscFileDiffValue, "%s", sSave.Data());
            for (int i = 0; i < 6; i++) {
              if (i == 0) {
                sSave = Form(" case %i : %7.2f +- %7.2f ,", i,
                             TCorrection_BiasMap[i][make_tuple(i_shape, i_pk % 4, i_axis, i_angleU, i_angleV)] / Unit::um,
                             TCorrection_BiasMapErr[i][make_tuple(i_shape, i_pk % 4, i_axis, i_angleU, i_angleV)] / Unit::um);
                fprintf(AscFileAll, "%s", sSave.Data());
                if (NonZeroBoth) fprintf(AscFileNonZeroBoth, "%s", sSave.Data());
                if (CloseValue) fprintf(AscFileCloseValue, "%s", sSave.Data());
                if (DiffValue) fprintf(AscFileDiffValue, "%s", sSave.Data());
                sSave = Form(" case %i : %7.2f +- %7.2f ,", i,
                             TCorrection_BiasMap[i][make_tuple(i_shape, i_pk % 4 + 4, i_axis, i_angleU, i_angleV)] / Unit::um,
                             TCorrection_BiasMapErr[i][make_tuple(i_shape, i_pk % 4 + 4, i_axis, i_angleU, i_angleV)] / Unit::um);
                fprintf(AscFileAll, "%s", sSave.Data());
                if (NonZeroBoth) fprintf(AscFileNonZeroBoth, "%s", sSave.Data());
                if (CloseValue) fprintf(AscFileCloseValue, "%s", sSave.Data());
                if (DiffValue) fprintf(AscFileDiffValue, "%s", sSave.Data());
              }
              if ((i > 0) && (i < 5)) {
                if (i == i_pk - 4 + 1) {
                  sSave = Form(" case %i : %7.2f +- %7.2f ,", i,
                               TCorrection_BiasMap[i][make_tuple(i_shape, i_pk, i_axis, i_angleU, i_angleV)] / Unit::um,
                               TCorrection_BiasMapErr[i][make_tuple(i_shape, i_pk, i_axis, i_angleU, i_angleV)] / Unit::um);
                  fprintf(AscFileAll, "%s", sSave.Data());
                  if (NonZeroBoth) fprintf(AscFileNonZeroBoth, "%s", sSave.Data());
                  if (CloseValue) fprintf(AscFileCloseValue, "%s", sSave.Data());
                  if (DiffValue) fprintf(AscFileDiffValue, "%s", sSave.Data());
                } else if (i == i_pk + 1) {
                  sSave = Form(" case %i : %7.2f +- %7.2f ,", i,
                               TCorrection_BiasMap[i][make_tuple(i_shape, i_pk + 4, i_axis, i_angleU, i_angleV)] / Unit::um,
                               TCorrection_BiasMapErr[i][make_tuple(i_shape, i_pk + 4, i_axis, i_angleU, i_angleV)] / Unit::um);
                  fprintf(AscFileAll, "%s", sSave.Data());
                  if (NonZeroBoth) fprintf(AscFileNonZeroBoth, "%s", sSave.Data());
                  if (CloseValue) fprintf(AscFileCloseValue, "%s", sSave.Data());
                  if (DiffValue) fprintf(AscFileDiffValue, "%s", sSave.Data());
                }
              }
              if (i == 5) {
                sSave = Form(" case %i : %7.2f +- %7.2f ,", i,
                             TCorrection_BiasMap[i][make_tuple(i_shape, i_pk % 4, i_axis, i_angleU, i_angleV)] / Unit::um,
                             TCorrection_BiasMapErr[i][make_tuple(i_shape, i_pk % 4, i_axis, i_angleU, i_angleV)] / Unit::um);
                fprintf(AscFileAll, "%s", sSave.Data());
                if (NonZeroBoth) fprintf(AscFileNonZeroBoth, "%s", sSave.Data());
                if (CloseValue) fprintf(AscFileCloseValue, "%s", sSave.Data());
                if (DiffValue) fprintf(AscFileDiffValue, "%s", sSave.Data());
                sSave = Form(" case %i : %7.2f +- %7.2f ,", i,
                             TCorrection_BiasMap[i][make_tuple(i_shape, i_pk % 4 + 4, i_axis, i_angleU, i_angleV)] / Unit::um,
                             TCorrection_BiasMapErr[i][make_tuple(i_shape, i_pk % 4 + 4, i_axis, i_angleU, i_angleV)] / Unit::um);
                fprintf(AscFileAll, "%s", sSave.Data());
                if (NonZeroBoth) fprintf(AscFileNonZeroBoth, "%s", sSave.Data());
                if (CloseValue) fprintf(AscFileCloseValue, "%s", sSave.Data());
                if (DiffValue) fprintf(AscFileDiffValue, "%s", sSave.Data());
              }
            }
            sSave = Form("\n                                      : ErEs");
            fprintf(AscFileAll, "%s", sSave.Data());
            if (NonZeroBoth) fprintf(AscFileNonZeroBoth, "%s", sSave.Data());
            if (CloseValue) fprintf(AscFileCloseValue, "%s", sSave.Data());
            if (DiffValue) fprintf(AscFileDiffValue, "%s", sSave.Data());
            for (int i = 0; i < 6; i++) {
              if (i == 0) {
                sSave = Form(" case %i : %7.2f            ,", i,
                             TCorrection_ErrorEstimationMap[i][make_tuple(i_shape, i_pk, i_axis, i_angleU, i_angleV)]);
                fprintf(AscFileAll, "%s", sSave.Data());
                if (NonZeroBoth) fprintf(AscFileNonZeroBoth, "%s", sSave.Data());
                if (CloseValue) fprintf(AscFileCloseValue, "%s", sSave.Data());
                if (DiffValue) fprintf(AscFileDiffValue, "%s", sSave.Data());
              }
              if ((i > 0) && (i < 5)) {
                if (i == i_pk - 4 + 1) {
                  sSave = Form(" case %i : %7.2f            ,", i,
                               TCorrection_ErrorEstimationMap[i][make_tuple(i_shape, i_pk, i_axis, i_angleU, i_angleV)]);
                  fprintf(AscFileAll, "%s", sSave.Data());
                  if (NonZeroBoth) fprintf(AscFileNonZeroBoth, "%s", sSave.Data());
                  if (CloseValue) fprintf(AscFileCloseValue, "%s", sSave.Data());
                  if (DiffValue) fprintf(AscFileDiffValue, "%s", sSave.Data());
                } else if (i == i_pk + 1) {
                  sSave = Form(" case %i : %7.2f            ,", i,
                               TCorrection_ErrorEstimationMap[i][make_tuple(i_shape, i_pk, i_axis, i_angleU, i_angleV)]);
                  fprintf(AscFileAll, "%s", sSave.Data());
                  if (NonZeroBoth) fprintf(AscFileNonZeroBoth, "%s", sSave.Data());
                  if (CloseValue) fprintf(AscFileCloseValue, "%s", sSave.Data());
                  if (DiffValue) fprintf(AscFileDiffValue, "%s", sSave.Data());
                }
              }
              if (i == 5) {
                sSave = Form(" case %i : %7.2f            ,", i,
                             TCorrection_ErrorEstimationMap[i][make_tuple(i_shape, i_pk, i_axis, i_angleU, i_angleV)]);
                fprintf(AscFileAll, "%s", sSave.Data());
                if (NonZeroBoth) fprintf(AscFileNonZeroBoth, "%s", sSave.Data());
                if (CloseValue) fprintf(AscFileCloseValue, "%s", sSave.Data());
                if (DiffValue) fprintf(AscFileDiffValue, "%s", sSave.Data());
              }
            }
            sSave = Form("\n");
            fprintf(AscFileAll, "%s", sSave.Data());
            if (NonZeroBoth) fprintf(AscFileNonZeroBoth, "%s", sSave.Data());
            if (CloseValue) fprintf(AscFileCloseValue, "%s", sSave.Data());
            if (DiffValue) fprintf(AscFileDiffValue, "%s", sSave.Data());
          }  // dimension (axis U, V)
        }  // angle V
      }  // angle U
    }  // pixel kind
  }  // shape
  fclose(AscFileAll);
  fclose(AscFileNonZeroBoth);
  fclose(AscFileCloseValue);
  fclose(AscFileDiffValue);

  for (int i_shape = 0; i_shape < m_shapes; i_shape++)
    for (int i_pk = 0; i_pk < m_pixelkinds; i_pk++)
      for (int i_angleU = 0; i_angleU < m_anglesU; i_angleU++)
        for (int i_angleV = 0; i_angleV < m_anglesV; i_angleV++)
          for (int i_axis = 0; i_axis < m_dimensions; i_axis++) {
            if (TCorrection_BiasMap[0][make_tuple(i_shape, i_pk, i_axis, i_angleU, i_angleV)] != 0) {
              TString text;
              text = Form("Bias Sh %i PixKind %i AngU %i AngV %i Dir %i : %f\n", i_shape, i_pk, i_angleU, i_angleV, i_axis,
                          TCorrection_BiasMap[0][make_tuple(i_shape, i_pk, i_axis, i_angleU, i_angleV)]);
              B2DEBUG(130, text.Data());
            }
          }
  // END - load corrections:

}

