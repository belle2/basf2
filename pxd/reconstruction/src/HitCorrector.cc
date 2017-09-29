/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Peter Kodys, Peter Kvasnicka                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#define _USE_MATH_DEFINES

#include <pxd/reconstruction/HitCorrector.h>
#include <framework/gearbox/Unit.h>
#include <framework/logging/Logger.h>
#include <vxd/dataobjects/VxdID.h>
#include <pxd/geometry/SensorInfo.h>
#include <vxd/geometry/GeoCache.h>
#include <cmath>
#include <TFile.h>
#include <TVectorD.h>
#include <framework/database/DBObjPtr.h>

using namespace std;
// using boost::format;
using namespace Belle2;
using namespace Belle2::PXD;

/** Clean up internal structures */
void HitCorrector::clear()
{
  m_CorrectionBiasMap.clear();
  m_CorrectionErrorEstimateMap.clear();
  m_CorrectionErrorEstimateCovarianceMap.clear();
  m_CorrectionBiasMapErr.clear();
  if (m_logFile.is_open())
    m_logFile.close();
}

/** Read correction data from DataBase and initialize the corrector.
*/
void HitCorrector::initialize()
{
  int isLoad = 1;
  TString Name = Form("PXDClSh_BasicSetting");
  DBObjPtr<TVectorD> BasicSetting(Name.Data());
  if (BasicSetting.isValid()) {
    m_shapes = (int)BasicSetting->GetMatrixArray()[0];
    m_pixelkinds = (int)BasicSetting->GetMatrixArray()[1];
    m_dimensions = (int)BasicSetting->GetMatrixArray()[2];
    m_anglesU = (int)BasicSetting->GetMatrixArray()[3];
    m_anglesV = (int)BasicSetting->GetMatrixArray()[4];
    m_in_pixelU = (int)BasicSetting->GetMatrixArray()[5];
    m_in_pixelV = (int)BasicSetting->GetMatrixArray()[6];
  } else {
    isLoad = 0;
  }
  Name = Form("PXDClSh_Correction_Bias");
  DBObjPtr<TVectorD> Correction_Bias(Name.Data());
  if (!Correction_Bias.isValid()) {
    isLoad = 0;
  }
  Name = Form("PXDClSh_Correction_BiasErr");
  DBObjPtr<TVectorD> Correction_BiasErr(Name.Data());
  if (!Correction_BiasErr.isValid()) {
    isLoad = 0;
  }
  Name = Form("PXDClSh_Correction_ErrorEstimation");
  DBObjPtr<TVectorD> Correction_ErrorEstimation(Name.Data());
  if (!Correction_ErrorEstimation.isValid()) {
    isLoad = 0;
  }
  Name = Form("PXDClSh_Correction_ErrorEstimationCovariance");
  DBObjPtr<TVectorD> Correction_ErrorEstimationCovariance(Name.Data());
  if (!Correction_ErrorEstimationCovariance.isValid()) {
    isLoad = 0;
  }
  Name = Form("PXDClSh_InPixelPosition");
  DBObjPtr<TVectorD> InPixelPosition(Name.Data());
  if (!InPixelPosition.isValid()) {
    isLoad = 0;
  }
  if (isLoad == 0) {
    B2INFO("Could not open correction in DB: Shape corrections will not be applied.");
    m_isInitialized = false;
    return;
  }

  B2DEBUG(80, "HitCorrector:  ---> Field dimensions: shapes: " << m_shapes << ", pixelkinds: " << m_pixelkinds <<
          ", dimensions: " << m_dimensions <<
          ", anglesU: " << m_anglesU << ", anglesV: " << m_anglesV << ", in_pixelU: " << m_in_pixelU << ", in_pixelV: " << m_in_pixelV);
  for (int i_shape = 0; i_shape < m_shapes; i_shape++) {
    int i_vector_shape = i_shape * m_pixelkinds * m_dimensions * m_anglesU * m_anglesV;
    for (int i_pk = 0; i_pk < m_pixelkinds; i_pk++) {
      int i_vector_pk = i_vector_shape + i_pk * m_dimensions * m_anglesU * m_anglesV;
      for (int i_axis = 0; i_axis < m_dimensions; i_axis++) {
        int i_vector_axis = i_vector_pk + i_axis * m_anglesU * m_anglesV;
        for (int i_angleU = 0; i_angleU < m_anglesU; i_angleU++) {
          int i_vector_angleU = i_vector_axis + i_angleU * m_anglesV;
          for (int i_angleV = 0; i_angleV < m_anglesV; i_angleV++) {
            int i_vector = i_vector_angleU + i_angleV;
            m_CorrectionBiasMap.setValue(i_shape, i_pk, i_axis, i_angleU, i_angleV,
                                         Correction_Bias->GetMatrixArray()[i_vector]);
            m_CorrectionErrorEstimateMap.setValue(i_shape, i_pk, i_axis, i_angleU, i_angleV,
                                                  Correction_ErrorEstimation->GetMatrixArray()[i_vector]);
            m_CorrectionErrorEstimateCovarianceMap.setValue(i_shape, i_pk, i_axis, i_angleU, i_angleV,
                                                            Correction_ErrorEstimationCovariance->GetMatrixArray()[i_vector]);
            m_CorrectionBiasMapErr.setValue(i_shape, i_pk, i_axis, i_angleU, i_angleV,
                                            Correction_BiasErr->GetMatrixArray()[i_vector]);
          }
        }
      }
    }
  }
  B2DEBUG(130, "-----> try to read non-zero elements:");
  for (int i_shape = 0; i_shape < 7; i_shape++) {
    float fDifference = 0.1 * Unit::um;
    if (fabs(Correction_Bias->GetMatrixArray()[i_shape]) > fDifference)
      B2DEBUG(130, "     --> " << Correction_Bias->GetMatrixArray()[i_shape] << " (" << i_shape << ")");
  }
  m_isInitialized = true;
  if (m_logFileName != "") {
    m_logFile.open(m_logFileName, ofstream::out | ofstream::trunc);
    B2INFO("Logging HitCorrector operation into " << m_logFileName);
  }
}

/** Read correction data and initialize the corrector.
  * @filename Name of data file to read correction data.
  */
void HitCorrector::initialize(const string filename)
{
  // Only over this limit is correction accepted
  float fDifference = 0.1 * Unit::um;
  TVectorD* Correction_Bias = NULL;
  TVectorD* Correction_ErrorEstimate = NULL;
  TVectorD* Correction_ErrorEstimateCovariance = NULL;
  TVectorD* Correction_BiasErr = NULL;
  TFile* f_CalBasic = new TFile(filename.c_str(), "read");
  if (f_CalBasic->IsOpen()) {
    TVectorD* PXDShCalibrationBasicSetting = NULL;
    B2INFO("HitCorrector: reading correction file: " << filename.c_str());
    f_CalBasic->GetObject("PXDShCalibrationBasicSetting", PXDShCalibrationBasicSetting);
    f_CalBasic->GetObject("Correction_Bias", Correction_Bias);
    f_CalBasic->GetObject("Correction_ErrorEstimation", Correction_ErrorEstimate);
    f_CalBasic->GetObject("Correction_ErrorEstimationCovariance", Correction_ErrorEstimateCovariance);
    f_CalBasic->GetObject("Correction_BiasErr", Correction_BiasErr);

    m_shapes = (int)PXDShCalibrationBasicSetting->GetMatrixArray()[0];
    m_pixelkinds = (int)PXDShCalibrationBasicSetting->GetMatrixArray()[1];
    m_dimensions = (int)PXDShCalibrationBasicSetting->GetMatrixArray()[2];
    m_anglesU = (int)PXDShCalibrationBasicSetting->GetMatrixArray()[3];
    m_anglesV = (int)PXDShCalibrationBasicSetting->GetMatrixArray()[4];
    m_in_pixelU = (int)PXDShCalibrationBasicSetting->GetMatrixArray()[5];
    m_in_pixelV = (int)PXDShCalibrationBasicSetting->GetMatrixArray()[6];
    B2INFO("HitCorrector:  ---> Field dimensions: shapes: " << m_shapes << ", pixelkinds: " << m_pixelkinds <<
           ", dimensions: " << m_dimensions <<
           ", anglesU: " << m_anglesU << ", anglesV: " << m_anglesV << ", in_pixelU: " << m_in_pixelU << ", in_pixelV: " << m_in_pixelV);
    B2DEBUG(130, "-----> try to read non-zero elements:");
    for (int i_shape = 0; i_shape < 7; i_shape++) {
      if (fabs(Correction_Bias->GetMatrixArray()[i_shape]) > fDifference)
        B2DEBUG(130, "     --> " << Correction_Bias->GetMatrixArray()[i_shape] << " (" << i_shape << ")");
    }

    f_CalBasic->Close();
  } else {
    B2WARNING("Could not open calibration file: Shape corrections will not be applied.");
    m_isInitialized = false;
    return;
  }
  for (int i_shape = 0; i_shape < m_shapes; i_shape++) {
    int i_vector_shape = i_shape * m_pixelkinds * m_dimensions * m_anglesU * m_anglesV;
    for (int i_pk = 0; i_pk < m_pixelkinds; i_pk++) {
      int i_vector_pk = i_vector_shape + i_pk * m_dimensions * m_anglesU * m_anglesV;
      for (int i_axis = 0; i_axis < m_dimensions; i_axis++) {
        int i_vector_axis = i_vector_pk + i_axis * m_anglesU * m_anglesV;
        for (int i_angleU = 0; i_angleU < m_anglesU; i_angleU++) {
          int i_vector_angleU = i_vector_axis + i_angleU * m_anglesV;
          for (int i_angleV = 0; i_angleV < m_anglesV; i_angleV++) {
            int i_vector = i_vector_angleU + i_angleV;
            m_CorrectionBiasMap.setValue(i_shape, i_pk, i_axis, i_angleU, i_angleV,
                                         Correction_Bias->GetMatrixArray()[i_vector]);
            m_CorrectionErrorEstimateMap.setValue(i_shape, i_pk, i_axis, i_angleU, i_angleV,
                                                  Correction_ErrorEstimate->GetMatrixArray()[i_vector]);
            m_CorrectionErrorEstimateCovarianceMap.setValue(i_shape, i_pk, i_axis, i_angleU, i_angleV,
                                                            Correction_ErrorEstimateCovariance->GetMatrixArray()[i_vector]);
            m_CorrectionBiasMapErr.setValue(i_shape, i_pk, i_axis, i_angleU, i_angleV,
                                            Correction_BiasErr->GetMatrixArray()[i_vector]);
          }
        }
      }
    }
  }
  m_isInitialized = true;
  if (m_logFileName != "") {
    m_logFile.open(m_logFileName, ofstream::out | ofstream::trunc);
    B2INFO("Logging HitCorrector operation into " << m_logFileName);
  }
}

/** Main (and only) way to access the HitCorrectior. */
HitCorrector& HitCorrector::getInstance()
{
  static std::unique_ptr<HitCorrector> instance(new HitCorrector());
  return *instance;
}

/** Return position-corrected cluster.
  * @param cluster Pointer to the cluster to be corrected
  * @param tu Track direction in u-coordinate, Tangent of angle
  * @param tv Track direction in v-coordinate, Tangent of angle
  */
PXDCluster& HitCorrector::correctCluster(PXDCluster& cluster, double tu, double tv)
{
  // No correction if no data
  if (!m_isInitialized) return cluster;

  tu = TMath::ATan2(tu, 1.0);  // use in case tu is Tangent of angle
  tv = TMath::ATan2(tv, 1.0);  // use in case tv is Tangent of angle

  double uNormPos = (tu + M_PI_2) / (M_PI / m_anglesU);
  double vNormPos = (tv + M_PI_2) / (M_PI / m_anglesV);
  int iIndexPhi = (int)uNormPos;    // identify index
  int iIndexTheta = (int)vNormPos;  // identify index
  //Get Geometry information
  VxdID sensorID = cluster.getSensorID();
  const SensorInfo& Info =
    dynamic_cast<const SensorInfo&>(VXD::GeoCache::get(sensorID));
  int i_layer = Info.getID().getLayerNumber();
  int i_sensor = Info.getID().getSensorNumber();
  int i_shape = cluster.getShape() - 1;
  if (i_shape < 0) return cluster;
  if (i_shape >= m_shapes) return cluster;

  const int EdgePixelSizeV1 = Info.getVCells2();
  const int EdgePixelSizeV2 = Info.getVCells() - EdgePixelSizeV1;
  const int EdgePixelSizeU = Info.getUCells();
  const int edgeClose = 2;  // TODO set distance of skip correction close edges
  int closeToEdgeOrMasked = 0;  // 0: healthy, far from edge or masked, 1: close edge, 2: close masked pixel
  if ((i_sensor == 2) && (abs(Info.getVCellID(cluster.getV()) - EdgePixelSizeV1) < edgeClose)) closeToEdgeOrMasked = 1;
  else if ((i_sensor == 1) && (abs(Info.getVCellID(cluster.getV()) - EdgePixelSizeV2) < edgeClose)) closeToEdgeOrMasked = 1;
  else if (Info.getVCellID(cluster.getV()) < edgeClose) closeToEdgeOrMasked = 1;
  else if ((EdgePixelSizeV1 + EdgePixelSizeV2 - Info.getVCellID(cluster.getV())) < edgeClose) closeToEdgeOrMasked = 1;
  else if (Info.getUCellID(cluster.getU()) < edgeClose) closeToEdgeOrMasked = 1;
  else if (EdgePixelSizeU - Info.getUCellID(cluster.getU()) < edgeClose) closeToEdgeOrMasked = 1;
  // TODO: add condition if close cluster exist masking pixel.

  int i_pixelKind = Info.getPixelKind(sensorID, cluster.getV());

  if (m_logFile.is_open()) {
    m_logFile << i_layer << "\t" << i_sensor << "\t" << i_shape << "\t" << i_pixelKind << "\t"
              << tu << "\t" << tv << "\t" << cluster.getU() << "\t" << cluster.getV() << "\t"
              << cluster.getUSigma() << "\t" << cluster.getVSigma() << "\t" << cluster.getRho() << "\t";
  }
  if (closeToEdgeOrMasked  == 0) {
    cluster.setU(cluster.getU() +
                 GetWeightedParameter(0, 0, i_shape, i_pixelKind, iIndexPhi, iIndexTheta, uNormPos, vNormPos)
                );
    cluster.setV(cluster.getV() +
                 GetWeightedParameter(0, 1, i_shape, i_pixelKind, iIndexPhi, iIndexTheta, uNormPos, vNormPos)
                );
    double uSigmaFactor =
      GetWeightedParameter(1, 0, i_shape, i_pixelKind, iIndexPhi, iIndexTheta, uNormPos, vNormPos) + 1.0;
    double vSigmaFactor =
      GetWeightedParameter(1, 1, i_shape, i_pixelKind, iIndexPhi, iIndexTheta, uNormPos, vNormPos) + 1.0;
    double uSigma = cluster.getUSigma() * uSigmaFactor;
    cluster.setUSigma(uSigma);
    double vSigma = cluster.getVSigma() * vSigmaFactor;
    cluster.setVSigma(vSigma);
    // Special monitor for tuning time:
    // if (((i_shape == 2) && (i_pixelKind == 6) && (iIndexPhi == 9) && (iIndexTheta == 12))) {
    //   printf("HC----> %i %i %i %i\n", i_shape, i_pixelKind, iIndexPhi, iIndexTheta);
    //   printf("           HC----> B: %f %f EE: %f %f\n",
    //          m_CorrectionBiasMap.getValue(i_shape, i_pixelKind, 0, iIndexPhi, iIndexTheta),
    //          m_CorrectionBiasMap.getValue(i_shape, i_pixelKind, 1, iIndexPhi, iIndexTheta),
    //          uSigmaFactor, vSigmaFactor
    //         );
    // }

    // TODO: Add setter for rho_uv - advance technique, wait for preparing of theory:
    // cluster.setRhoUV( cluster.getRhoUV() *
    //   GetWeightedParameter(2, 0, i_shape, i_pixelKind, iIndexPhi,
    //     iIndexTheta, uNormPos, vNormPos)
    //   );
  }
  if (m_logFile.is_open()) {
    m_logFile <<  cluster.getU() << "\t" << cluster.getV() << "\t"
              << cluster.getUSigma() << "\t" << cluster.getVSigma() << "\t" << cluster.getRho()
              << endl;
  }

  return cluster;
}

/** Function for calculation of waited position of parameter
  * in rectangular net of parameters
  * Return value on position u,v.
  * @param type Type of values:
  * 0: Bias of position (m_CorrectionBiasMap), default = 0.0
  * 1: Correction of Error Estimation (m_CorrectionErrorEstimateMap), default = 1.0
  * 2: Error Estimation Covariance coeficient (m_CorrectionErrorEstimateCovarianceMap), default = 0.0
  * 3: Error of Bias of position (m_CorrectionBiasMapErr), default = 0.0
  * @param direction Direction in which is looking correction: 0: u, 1: v.
  * @param shape Index of shape kind.
  * @param pixelKind Index of pixel kind.
  * @param iu Lowest index in u direction of matrix.
  * @param iv Lowest index in v direction of matrix.
  * @param u Normalized position in u direction in range iu .. iu + 1.
  * @param v Normalized position in v direction in range iv .. iv + 1.
  */
double HitCorrector::GetWeightedParameter(int type, int direction, int shape, int pixelKind, int iu, int iv, double u, double v)
{
  double fVal[4] = {0.0, 0.0, 0.0, 0.0};
  if (type == 0) {
    fVal[0] = m_CorrectionBiasMap.getValue(shape, pixelKind, direction, iu, iv);
    fVal[1] = m_CorrectionBiasMap.getValue(shape, pixelKind, direction, iu + 1, iv);
    fVal[2] = m_CorrectionBiasMap.getValue(shape, pixelKind, direction, iu, iv + 1);
    fVal[3] = m_CorrectionBiasMap.getValue(shape, pixelKind, direction, iu + 1, iv + 1);
  } else if (type == 1) {
    fVal[0] = m_CorrectionErrorEstimateMap.getValue(shape, pixelKind, direction, iu, iv);
    fVal[1] = m_CorrectionErrorEstimateMap.getValue(shape, pixelKind, direction, iu + 1, iv);
    fVal[2] = m_CorrectionErrorEstimateMap.getValue(shape, pixelKind, direction, iu, iv + 1);
    fVal[3] = m_CorrectionErrorEstimateMap.getValue(shape, pixelKind, direction, iu + 1, iv + 1);
  } else if (type == 2) {
    fVal[0] = m_CorrectionErrorEstimateCovarianceMap.getValue(shape, pixelKind, direction, iu, iv);
    fVal[1] = m_CorrectionErrorEstimateCovarianceMap.getValue(shape, pixelKind, direction, iu + 1, iv);
    fVal[2] = m_CorrectionErrorEstimateCovarianceMap.getValue(shape, pixelKind, direction, iu, iv + 1);
    fVal[3] = m_CorrectionErrorEstimateCovarianceMap.getValue(shape, pixelKind, direction, iu + 1, iv + 1);
  } else if (type == 3) {
    fVal[0] = m_CorrectionBiasMapErr.getValue(shape, pixelKind, direction, iu, iv);
    fVal[1] = m_CorrectionBiasMapErr.getValue(shape, pixelKind, direction, iu + 1, iv);
    fVal[2] = m_CorrectionBiasMapErr.getValue(shape, pixelKind, direction, iu, iv + 1);
    fVal[3] = m_CorrectionBiasMapErr.getValue(shape, pixelKind, direction, iu + 1, iv + 1);
  }
  double ret0 = fVal[0] + (fVal[1] - fVal[0]) * (u - (double)iu);
  double ret1 = fVal[2] + (fVal[3] - fVal[2]) * (u - (double)iu);
  double fOutVal = ret0 + (ret1 - ret0) * (v - (double)iv);
  return fOutVal;
}

