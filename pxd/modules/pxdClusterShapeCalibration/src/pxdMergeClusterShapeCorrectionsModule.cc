/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Peter Kodys                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <pxd/modules/pxdClusterShapeCalibration/pxdMergeClusterShapeCorrectionsModule.h>

#include <framework/database/DBImportObjPtr.h>
// #include <framework/database/DBImportArray.h>
#include <framework/database/IntervalOfValidity.h>
#include <framework/database/DBObjPtr.h>
#include <TH1F.h>
#include <TH1I.h>

using namespace Belle2;
using namespace Belle2::PXD;
using namespace std;

class PXDClusterShapeCalibrationAlgorithm;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(pxdMergeClusterShapeCorrections);

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

pxdMergeClusterShapeCorrectionsModule::pxdMergeClusterShapeCorrectionsModule() : Module()
{
  //Set module properties
  setDescription("Merge PXD Cluster Shape Calibrations from 4 correction files to one"
                 "Basic steps for using of corrections are: "
                 " - pxdClusterShapeCalibration - create source for correction calculation "
                 " - PXDClusterShapeCalibrationAlgorithm - calculate corrections "
                 " - pxdMergeClusterShapeCorrections - merge results and create DB object <--- this module "
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
                 " "
                 " TODO: for temprorary time special calibration is set. "
                 " TODO: after data taking set corrections from real data. "
                );

  setPropertyFlags(c_ParallelProcessingCertified);

  addParam("CalFileOutputName", m_CalFileOutputName,
           "Name of file contain output merged calibration, default=PXD-ClasterShapeCorrections", m_CalFileOutputName);
  addParam("CalFileBasicName", m_CalFileBasicName,
           "Name of file contain basic calibration, default=pxdClShCal_RealData0_Pixel-1", m_CalFileBasicName);
  addParam("CalFilePK0Name", m_CalFilePK0Name,
           "Name of file contain calibration for pixel kind 0 (PitchV=55um), default=pxdClShCal_RealData0_Pixel0", m_CalFilePK0Name);
  addParam("CalFilePK1Name", m_CalFilePK1Name,
           "Name of file contain calibration for pixel kind 1 (PitchV=60um), default=pxdClShCal_RealData0_Pixel1", m_CalFilePK1Name);
  addParam("CalFilePK2Name", m_CalFilePK2Name,
           "Name of file contain calibration for pixel kind 2 (PitchV=70um), default=pxdClShCal_RealData0_Pixel2", m_CalFilePK2Name);
  addParam("CalFilePK3Name", m_CalFilePK3Name,
           "Name of file contain calibration for pixel kind 3 (PitchV=85um), default=pxdClShCal_RealData0_Pixel3", m_CalFilePK3Name);

  addParam("DifferenceBias", m_Difference,
           "Only over this limit is bias correction accepted, default = 1.0 um, in [cm]", m_Difference);
  addParam("DifferenceBiasClose", m_DifferenceClose,
           "Only under this limit is real bias correction compare to simulation accepted, default = 3.0 um, in [cm]", m_DifferenceClose);
  addParam("DifferenceErrEst", m_DifferenceErrEst,
           "Only under this limit is real error estimation correction compare to simulation accepted, default = 0.1 um, in [cm]",
           m_DifferenceErrEst);
  addParam("CreateDB", m_CreateDB,
           "Create and fill reference histograms in DataBase, default=0 ", m_CreateDB);
}

void pxdMergeClusterShapeCorrectionsModule::initialize()
{
  /** Exist file with corrections for PK=0, default=0 */
  int m_ExistCorrectionPK0 = 0;
  /** Exist file with corrections for PK=1, default=0 */
  int m_ExistCorrectionPK1 = 0;
  /** Exist file with corrections for PK=2, default=0 */
  int m_ExistCorrectionPK2 = 0;
  /** Exist file with corrections for PK=3, default=0 */
  int m_ExistCorrectionPK3 = 0;

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

  // START - load corrections:

  B2INFO("Load correction files:");

  TVectorD** PXDShCalibrationBasicSetting = NULL;
  TVectorD** Correction_Bias = NULL;
  TVectorD** Correction_ErrorEstimation = NULL;
  TVectorD** Correction_ErrorEstimationCovariance = NULL;
  TVectorD** Correction_BiasErr = NULL;
  TVectorD** InPixelPosition = NULL;

  PXDShCalibrationBasicSetting = (TVectorD**) new TVectorD*[6];
  Correction_Bias = (TVectorD**) new TVectorD*[6];
  Correction_ErrorEstimation = (TVectorD**) new TVectorD*[6];
  Correction_ErrorEstimationCovariance = (TVectorD**) new TVectorD*[6];
  Correction_BiasErr = (TVectorD**) new TVectorD*[6];
  InPixelPosition = (TVectorD**) new TVectorD*[6];
  for (int i = 0; i < 6; i++) {
    PXDShCalibrationBasicSetting[i] = NULL;
    Correction_Bias[i] = NULL;
    Correction_ErrorEstimation[i] = NULL;
    Correction_ErrorEstimationCovariance[i] = NULL;
    Correction_BiasErr[i] = NULL;
    InPixelPosition[i] = NULL;
  }

  TFile* f_CalBasic = new TFile(m_CalFileBasicName.c_str(), "read");
  int iLoad = 0;
  int IsAllFilesLoaded = 1;
  if (f_CalBasic->IsOpen()) {
    B2INFO("Correction file Basic: " << m_CalFileBasicName.c_str());

    // to fill output corrections with index 6:
    f_CalBasic->GetObject("PXDShCalibrationBasicSetting", PXDShCalibrationBasicSetting[5]);
    f_CalBasic->GetObject("Correction_Bias", Correction_Bias[5]);
    f_CalBasic->GetObject("Correction_ErrorEstimation", Correction_ErrorEstimation[5]);
    f_CalBasic->GetObject("Correction_ErrorEstimationCovariance", Correction_ErrorEstimationCovariance[5]);
    f_CalBasic->GetObject("Correction_BiasErr", Correction_BiasErr[5]);
    f_CalBasic->GetObject("InPixelPosition", InPixelPosition[5]);

    f_CalBasic->GetObject("PXDShCalibrationBasicSetting", PXDShCalibrationBasicSetting[iLoad]);
    f_CalBasic->GetObject("Correction_Bias", Correction_Bias[iLoad]);
    f_CalBasic->GetObject("Correction_ErrorEstimation", Correction_ErrorEstimation[iLoad]);
    f_CalBasic->GetObject("Correction_ErrorEstimationCovariance", Correction_ErrorEstimationCovariance[iLoad]);
    f_CalBasic->GetObject("Correction_BiasErr", Correction_BiasErr[iLoad]);
    f_CalBasic->GetObject("InPixelPosition", InPixelPosition[iLoad]);
    m_shapes = (int)PXDShCalibrationBasicSetting[iLoad]->GetMatrixArray()[0];
    m_pixelkinds = (int)PXDShCalibrationBasicSetting[iLoad]->GetMatrixArray()[1];
    m_dimensions = (int)PXDShCalibrationBasicSetting[iLoad]->GetMatrixArray()[2];
    m_anglesU = (int)PXDShCalibrationBasicSetting[iLoad]->GetMatrixArray()[3];
    m_anglesV = (int)PXDShCalibrationBasicSetting[iLoad]->GetMatrixArray()[4];
    m_in_pixelU = (int)PXDShCalibrationBasicSetting[iLoad]->GetMatrixArray()[5];
    m_in_pixelV = (int)PXDShCalibrationBasicSetting[iLoad]->GetMatrixArray()[6];
    B2INFO("---> Field dimensions: shapes: " << m_shapes << ", pixelkinds: " << m_pixelkinds <<
           ", dimensions: " << m_dimensions <<
           ", anglesU: " << m_anglesU << ", anglesV: " << m_anglesV << ", in_pixelU: " << m_in_pixelU << ", in_pixelV: " << m_in_pixelV);
    f_CalBasic->Close();
  } else {
    IsAllFilesLoaded = 0;
    B2INFO("File of basic calibration: " << m_CalFileBasicName.c_str() << " is not available, please check it!");
    return;
  }
  double* ValueCorsBias = new double[m_shapes * m_pixelkinds * m_dimensions * m_anglesU * m_anglesV];
  double* ValueCorsBiasErr = new double[m_shapes * m_pixelkinds * m_dimensions * m_anglesU * m_anglesV];
  double* ValueCorsErrEst = new double[m_shapes * m_pixelkinds * m_dimensions * m_anglesU * m_anglesV];
  double* ValueCorsErrEstCov = new double[m_shapes * m_pixelkinds * m_dimensions * m_anglesU * m_anglesV];
  double* ValueCorsInPixPos = new double[m_shapes * m_pixelkinds * m_anglesU * m_anglesV * m_in_pixelU * m_in_pixelV];
  for (int i = 0; i < m_shapes * m_pixelkinds * m_dimensions * m_anglesU * m_anglesV; i++) {
    ValueCorsBias[i] = 0.0;
    ValueCorsBiasErr[i] = 0.0;
    ValueCorsErrEst[i] = 0.0;
    ValueCorsErrEstCov[i] = 0.0;
  }
  for (int i = 0; i < m_shapes * m_pixelkinds * m_anglesU * m_anglesV * m_in_pixelU * m_in_pixelV; i++) {
    ValueCorsInPixPos[i] = InPixelPosition[0]->GetMatrixArray()[i];
    if (std::isnan(InPixelPosition[0]->GetMatrixArray()[i])) ValueCorsInPixPos[i] = 0;
  }

  f_CalBasic = new TFile(m_CalFilePK0Name.c_str(), "read");
  iLoad = 1;
  if (f_CalBasic->IsOpen()) {
    m_ExistCorrectionPK0 = 1;
    B2INFO("Correction file PK0: " << m_CalFilePK0Name.c_str());
    f_CalBasic->GetObject("PXDShCalibrationBasicSetting", PXDShCalibrationBasicSetting[iLoad]);
    f_CalBasic->GetObject("Correction_Bias", Correction_Bias[iLoad]);
    f_CalBasic->GetObject("Correction_ErrorEstimation", Correction_ErrorEstimation[iLoad]);
    f_CalBasic->GetObject("Correction_ErrorEstimationCovariance", Correction_ErrorEstimationCovariance[iLoad]);
    f_CalBasic->GetObject("Correction_BiasErr", Correction_BiasErr[iLoad]);
    f_CalBasic->GetObject("InPixelPosition", InPixelPosition[iLoad]);
    if (fabs(m_shapes - PXDShCalibrationBasicSetting[iLoad]->GetMatrixArray()[0]) > m_Difference) m_ExistCorrectionPK0 = 0;
    if (fabs(m_pixelkinds - PXDShCalibrationBasicSetting[iLoad]->GetMatrixArray()[1]) > m_Difference) m_ExistCorrectionPK0 = 0;
    if (fabs(m_dimensions - PXDShCalibrationBasicSetting[iLoad]->GetMatrixArray()[2]) > m_Difference) m_ExistCorrectionPK0 = 0;
    if (fabs(m_anglesU - PXDShCalibrationBasicSetting[iLoad]->GetMatrixArray()[3]) > m_Difference) m_ExistCorrectionPK0 = 0;
    if (fabs(m_anglesV - PXDShCalibrationBasicSetting[iLoad]->GetMatrixArray()[4]) > m_Difference) m_ExistCorrectionPK0 = 0;
    if (fabs(m_in_pixelU - PXDShCalibrationBasicSetting[iLoad]->GetMatrixArray()[5]) > m_Difference) m_ExistCorrectionPK0 = 0;
    if (fabs(m_in_pixelV - PXDShCalibrationBasicSetting[iLoad]->GetMatrixArray()[6]) > m_Difference) m_ExistCorrectionPK0 = 0;
    if (m_ExistCorrectionPK0 == 0) B2INFO("pxdMergeClusterShapeCorrections: ERROR in PK0 file! differences on field dimensions.");
    f_CalBasic->Close();
  } else {
    IsAllFilesLoaded = 0;
    B2INFO("File of PixelKind0 calibration: " << m_CalFilePK0Name.c_str() << " is not available, please check it!");
    return;
  }
  f_CalBasic = new TFile(m_CalFilePK1Name.c_str(), "read");
  iLoad = 2;
  if (f_CalBasic->IsOpen()) {
    m_ExistCorrectionPK1 = 1;
    B2INFO("Correction file PK1: " << m_CalFilePK1Name.c_str());
    f_CalBasic->GetObject("PXDShCalibrationBasicSetting", PXDShCalibrationBasicSetting[iLoad]);
    f_CalBasic->GetObject("Correction_Bias", Correction_Bias[iLoad]);
    f_CalBasic->GetObject("Correction_ErrorEstimation", Correction_ErrorEstimation[iLoad]);
    f_CalBasic->GetObject("Correction_ErrorEstimationCovariance", Correction_ErrorEstimationCovariance[iLoad]);
    f_CalBasic->GetObject("Correction_BiasErr", Correction_BiasErr[iLoad]);
    f_CalBasic->GetObject("InPixelPosition", InPixelPosition[iLoad]);
    if (fabs(m_shapes - PXDShCalibrationBasicSetting[iLoad]->GetMatrixArray()[0]) > m_Difference) m_ExistCorrectionPK1 = 0;
    if (fabs(m_pixelkinds - PXDShCalibrationBasicSetting[iLoad]->GetMatrixArray()[1]) > m_Difference) m_ExistCorrectionPK1 = 0;
    if (fabs(m_dimensions - PXDShCalibrationBasicSetting[iLoad]->GetMatrixArray()[2]) > m_Difference) m_ExistCorrectionPK1 = 0;
    if (fabs(m_anglesU - PXDShCalibrationBasicSetting[iLoad]->GetMatrixArray()[3]) > m_Difference) m_ExistCorrectionPK1 = 0;
    if (fabs(m_anglesV - PXDShCalibrationBasicSetting[iLoad]->GetMatrixArray()[4]) > m_Difference) m_ExistCorrectionPK1 = 0;
    if (fabs(m_in_pixelU - PXDShCalibrationBasicSetting[iLoad]->GetMatrixArray()[5]) > m_Difference) m_ExistCorrectionPK1 = 0;
    if (fabs(m_in_pixelV - PXDShCalibrationBasicSetting[iLoad]->GetMatrixArray()[6]) > m_Difference) m_ExistCorrectionPK1 = 0;
    if (m_ExistCorrectionPK1 == 0) B2INFO("pxdMergeClusterShapeCorrections: ERROR in PK1 file! differences on field dimensions.");
    f_CalBasic->Close();
  } else {
    IsAllFilesLoaded = 0;
    B2INFO("File of PixelKind1 calibration: " << m_CalFilePK1Name.c_str() << " is not available, please check it!");
    return;
  }
  f_CalBasic = new TFile(m_CalFilePK2Name.c_str(), "read");
  iLoad = 3;
  if (f_CalBasic->IsOpen()) {
    m_ExistCorrectionPK2 = 1;
    B2INFO("Correction file PK2: " << m_CalFilePK2Name.c_str());
    f_CalBasic->GetObject("PXDShCalibrationBasicSetting", PXDShCalibrationBasicSetting[iLoad]);
    f_CalBasic->GetObject("Correction_Bias", Correction_Bias[iLoad]);
    f_CalBasic->GetObject("Correction_ErrorEstimation", Correction_ErrorEstimation[iLoad]);
    f_CalBasic->GetObject("Correction_ErrorEstimationCovariance", Correction_ErrorEstimationCovariance[iLoad]);
    f_CalBasic->GetObject("Correction_BiasErr", Correction_BiasErr[iLoad]);
    f_CalBasic->GetObject("InPixelPosition", InPixelPosition[iLoad]);
    if (fabs(m_shapes - PXDShCalibrationBasicSetting[iLoad]->GetMatrixArray()[0]) > m_Difference) m_ExistCorrectionPK2 = 0;
    if (fabs(m_pixelkinds - PXDShCalibrationBasicSetting[iLoad]->GetMatrixArray()[1]) > m_Difference) m_ExistCorrectionPK2 = 0;
    if (fabs(m_dimensions - PXDShCalibrationBasicSetting[iLoad]->GetMatrixArray()[2]) > m_Difference) m_ExistCorrectionPK2 = 0;
    if (fabs(m_anglesU - PXDShCalibrationBasicSetting[iLoad]->GetMatrixArray()[3]) > m_Difference) m_ExistCorrectionPK2 = 0;
    if (fabs(m_anglesV - PXDShCalibrationBasicSetting[iLoad]->GetMatrixArray()[4]) > m_Difference) m_ExistCorrectionPK2 = 0;
    if (fabs(m_in_pixelU - PXDShCalibrationBasicSetting[iLoad]->GetMatrixArray()[5]) > m_Difference) m_ExistCorrectionPK2 = 0;
    if (fabs(m_in_pixelV - PXDShCalibrationBasicSetting[iLoad]->GetMatrixArray()[6]) > m_Difference) m_ExistCorrectionPK2 = 0;
    if (m_ExistCorrectionPK2 == 0) B2INFO("pxdMergeClusterShapeCorrections: ERROR in PK2 file! differences on field dimensions.");
    f_CalBasic->Close();
  } else {
    IsAllFilesLoaded = 0;
    B2INFO("File of PixelKind2 calibration: " << m_CalFilePK2Name.c_str() << " is not available, please check it!");
    return;
  }
  f_CalBasic = new TFile(m_CalFilePK3Name.c_str(), "read");
  iLoad = 4;
  if (f_CalBasic->IsOpen()) {
    m_ExistCorrectionPK3 = 1;
    B2INFO("Correction file PK3: " << m_CalFilePK3Name.c_str());
    f_CalBasic->GetObject("PXDShCalibrationBasicSetting", PXDShCalibrationBasicSetting[iLoad]);
    f_CalBasic->GetObject("Correction_Bias", Correction_Bias[iLoad]);
    f_CalBasic->GetObject("Correction_ErrorEstimation", Correction_ErrorEstimation[iLoad]);
    f_CalBasic->GetObject("Correction_ErrorEstimationCovariance", Correction_ErrorEstimationCovariance[iLoad]);
    f_CalBasic->GetObject("Correction_BiasErr", Correction_BiasErr[iLoad]);
    f_CalBasic->GetObject("InPixelPosition", InPixelPosition[iLoad]);
    if (fabs(m_shapes - PXDShCalibrationBasicSetting[iLoad]->GetMatrixArray()[0]) > m_Difference) m_ExistCorrectionPK3 = 0;
    if (fabs(m_pixelkinds - PXDShCalibrationBasicSetting[iLoad]->GetMatrixArray()[1]) > m_Difference) m_ExistCorrectionPK3 = 0;
    if (fabs(m_dimensions - PXDShCalibrationBasicSetting[iLoad]->GetMatrixArray()[2]) > m_Difference) m_ExistCorrectionPK3 = 0;
    if (fabs(m_anglesU - PXDShCalibrationBasicSetting[iLoad]->GetMatrixArray()[3]) > m_Difference) m_ExistCorrectionPK3 = 0;
    if (fabs(m_anglesV - PXDShCalibrationBasicSetting[iLoad]->GetMatrixArray()[4]) > m_Difference) m_ExistCorrectionPK3 = 0;
    if (fabs(m_in_pixelU - PXDShCalibrationBasicSetting[iLoad]->GetMatrixArray()[5]) > m_Difference) m_ExistCorrectionPK3 = 0;
    if (fabs(m_in_pixelV - PXDShCalibrationBasicSetting[iLoad]->GetMatrixArray()[6]) > m_Difference) m_ExistCorrectionPK3 = 0;
    if (m_ExistCorrectionPK3 == 0) B2INFO("pxdMergeClusterShapeCorrections: ERROR in PK3 file! differences on field dimensions.");
    f_CalBasic->Close();
  } else {
    IsAllFilesLoaded = 0;
    B2INFO("File of PixelKind3 calibration: " << m_CalFilePK3Name.c_str() << " is not available, please check it!");
    return;
  }
  if (!IsAllFilesLoaded) return;

  // ************ START preparing of histograms for monitoring of corrections:
  TString sLogFileName = m_CalFileOutputName.c_str();
  sLogFileName.ReplaceAll(".root", ".log");
  TString sHistoFileName = m_CalFileOutputName.c_str();
  sHistoFileName.ReplaceAll(".root", "Histos.root");
  B2INFO("Files of log: " << sLogFileName.Data() << " and for histos: " << sHistoFileName.Data());
  TH1F* hAllBVal = NULL;
  TH1F* hAllEEVal = NULL;
  TH1I* hAllB = NULL;
  TH1I* hAllEE = NULL;
  TH1F** hPKBVal = NULL;
  TH1F** hPKEEVal = NULL;
  TH1I** hPKB = NULL;
  TH1I** hPKEE = NULL;
  TH1F** hClSBVal = NULL;
  TH1F** hClSEEVal = NULL;
  hPKBVal = (TH1F**) new TH1F*[4];
  hPKEEVal = (TH1F**) new TH1F*[4];
  hPKB = (TH1I**) new TH1I*[4];
  hPKEE = (TH1I**) new TH1I*[4];
  hClSBVal = (TH1F**) new TH1F*[m_shapes];
  hClSEEVal = (TH1F**) new TH1F*[m_shapes];
  for (int i_pk = 0; i_pk < 4; i_pk++) {
    hPKBVal[i_pk] = NULL;
    hPKEEVal[i_pk] = NULL;
    TString HistoName = Form("PKBVal_PK%02i", i_pk);
    TString HistoTitle = Form("Cluster Shape Correction, bias values, pix kind %02i", i_pk);
    hPKBVal[i_pk] = new TH1F(HistoName.Data(), HistoTitle.Data(),
                             100, -50.0, 50.0);
    hPKBVal[i_pk]->GetXaxis()->SetTitle("Correction [#mum]");
    hPKBVal[i_pk]->GetYaxis()->SetTitle("Counts");
    HistoName = Form("PKEEVal_PK%02i", i_pk);
    HistoTitle = Form("Cluster Shape Correction, error estimation values, pix kind %02i", i_pk);
    hPKEEVal[i_pk] = new TH1F(HistoName.Data(), HistoTitle.Data(),
                              100, 0.0, 10.0);
    hPKEEVal[i_pk]->GetXaxis()->SetTitle("EE Correction");
    hPKEEVal[i_pk]->GetYaxis()->SetTitle("Counts");

    hPKB[i_pk] = NULL;
    hPKEE[i_pk] = NULL;
    HistoName = Form("PKB_PK%02i", i_pk);
    HistoTitle = Form("Cluster Shape Correction, bias counts, pix kind %02i", i_pk);
    hPKB[i_pk] = new TH1I(HistoName.Data(), HistoTitle.Data(),
                          m_shapes, 1, m_shapes + 1);
    hPKB[i_pk]->GetXaxis()->SetTitle("Cluster Shape ID");
    hPKB[i_pk]->GetYaxis()->SetTitle("Counts");
    HistoName = Form("PKEE_PK%02i", i_pk);
    HistoTitle = Form("Cluster Shape Correction, error estimation counts, pix kind %02i", i_pk);
    hPKEE[i_pk] = new TH1I(HistoName.Data(), HistoTitle.Data(),
                           m_shapes, 1, m_shapes + 1);
    hPKEE[i_pk]->GetXaxis()->SetTitle("Cluster Shape ID");
    hPKEE[i_pk]->GetYaxis()->SetTitle("Counts");
  }
  hAllB = NULL;
  hAllEE = NULL;
  TString HistoName = Form("PKB_AllPK");
  TString HistoTitle = Form("Cluster Shape Correction, bias counts, AllPK");
  hAllB = new TH1I(HistoName.Data(), HistoTitle.Data(),
                   m_shapes, 1, m_shapes + 1);
  hAllB->GetXaxis()->SetTitle("Cluster Shape ID");
  hAllB->GetYaxis()->SetTitle("Counts");
  HistoName = Form("PKEE_AllPK");
  HistoTitle = Form("Cluster Shape Correction, error estimation counts, AllPK");
  hAllEE = new TH1I(HistoName.Data(), HistoTitle.Data(),
                    m_shapes, 1, m_shapes + 1);
  hAllEE->GetXaxis()->SetTitle("Cluster Shape ID");
  hAllEE->GetYaxis()->SetTitle("Counts");
  for (int iSh = 0; iSh < m_shapes; iSh++) {
    hClSBVal[iSh] = NULL;
    hClSEEVal[iSh] = NULL;
    HistoName = Form("ClSBVal_Sh%02i", iSh + 1);
    HistoTitle = Form("Cluster Shape Correction, bias values, shape %02i", iSh + 1);
    hClSBVal[iSh] = new TH1F(HistoName.Data(), HistoTitle.Data(),
                             100, -50.0, 50.0);
    hClSBVal[iSh]->GetXaxis()->SetTitle("Correction [#mum]");
    hClSBVal[iSh]->GetYaxis()->SetTitle("Counts");
    HistoName = Form("ClSEEVal_Sh%02i", iSh + 1);
    HistoTitle = Form("Cluster Shape Correction, error estimation values, shape %02i", iSh + 1);
    hClSEEVal[iSh] = new TH1F(HistoName.Data(), HistoTitle.Data(),
                              100, 0.0, 10.0);
    hClSEEVal[iSh]->GetXaxis()->SetTitle("EE Correction");
    hClSEEVal[iSh]->GetYaxis()->SetTitle("Counts");
  }
  hAllBVal = NULL;
  hAllEEVal = NULL;
  HistoName = Form("ClSBVal_ShAll");
  HistoTitle = Form("Cluster Shape Correction, bias values, shape all");
  hAllBVal = new TH1F(HistoName.Data(), HistoTitle.Data(),
                      100, -50.0, 50.0);
  hAllBVal->GetXaxis()->SetTitle("Correction [#mum]");
  hAllBVal->GetYaxis()->SetTitle("Counts");
  HistoName = Form("ClSEEVal_ShAll");
  HistoTitle = Form("Cluster Shape Correction, error estimation values, shape all");
  hAllEEVal = new TH1F(HistoName.Data(), HistoTitle.Data(),
                       100, 0.0, 10.0);
  hAllEEVal->GetXaxis()->SetTitle("EE Correction");
  hAllEEVal->GetYaxis()->SetTitle("Counts");
  // ************ END preparing of histograms for monitoring of corrections:

  int iMatchBiasCors = 0;
  int iNotMatchBiasCors = 0;
  int iBiasCorsSum = 0;
  int iBiasAll = 0;
  int iMatchEECors = 0;
  int iNotMatchEECors = 0;
  int iEECorsSum = 0;
  int iEEAll = 0;
  int iMatchInPixCors = 0;
  int iNotMatchInPixCors = 0;
  int iInPixCorsSum = 0;
  int iInPixAll = 0;
  for (int i_shape = 0; i_shape < m_shapes; i_shape++) {
    for (int i_pk = 0; i_pk < m_pixelkinds; i_pk++) {
      for (int i_axis = 0; i_axis < m_dimensions; i_axis++) {
        for (int i_angleU = 0; i_angleU < m_anglesU; i_angleU++) {
          for (int i_angleV = 0; i_angleV < m_anglesV; i_angleV++) {
            iBiasAll++;
            iEEAll++;
            iInPixAll++;
            int i_vector = i_shape * m_pixelkinds * m_dimensions * m_anglesU * m_anglesV;
            i_vector += i_pk * m_dimensions * m_anglesU * m_anglesV;
            i_vector += i_axis * m_anglesU * m_anglesV;
            i_vector += i_angleU * m_anglesV;
            i_vector += i_angleV;
            int i_vectorSpecial[4];  // Vectors to special file with correction only in one i_pk case
            for (int i = 0; i < 4; i++) {
              int i_pkSp = 0;
              if (i == 0) i_pkSp = 2;
              else if (i == 1) i_pkSp = 3;
              else if (i == 2) i_pkSp = 6;
              else if (i == 3) i_pkSp = 7;
              i_vectorSpecial[i] = i_shape * m_pixelkinds * m_dimensions * m_anglesU * m_anglesV;
              i_vectorSpecial[i] += i_pkSp * m_dimensions * m_anglesU * m_anglesV;
              i_vectorSpecial[i] += i_axis * m_anglesU * m_anglesV;
              i_vectorSpecial[i] += i_angleU * m_anglesV;
              i_vectorSpecial[i] += i_angleV;
            }
            // Final correcion [5] derivate from "basic" calibration file [0],
            // empty correction are filled from special calibrations [1..4]
            // i_pk: [1]55um = 1,2, [2]60um = 0,3, [3]70um = 5,6, [4]85um = 4,7
            // TODO: for temprorary time special calibration is set.
            int corFileIndex = 0;
            if ((i_pk == 1) || (i_pk == 2)) {  // pixel pitch v = 55 um, special corection file index: [1]
              corFileIndex = 1;
            } else if ((i_pk == 0) || (i_pk == 3)) { // pixel pitch v = 60 um, special corection file index: [2]
              corFileIndex = 2;
            } else if ((i_pk == 5) || (i_pk == 6)) { // pixel pitch v = 70 um, special corection file index: [3]
              corFileIndex = 3;
            } else if ((i_pk == 4) || (i_pk == 7)) { // pixel pitch v = 85 um, special corection file index: [4]
              corFileIndex = 4;
            }
            if (std::isnan(Correction_ErrorEstimationCovariance[0]->GetMatrixArray()[i_vector])) {
              ValueCorsErrEstCov[i_vector] = 0.0;
            } else {
              ValueCorsErrEstCov[i_vector] = Correction_ErrorEstimationCovariance[0]->GetMatrixArray()[i_vector];
            }
            float cor = Correction_Bias[0]->GetMatrixArray()[i_vector];
            float corErr = Correction_BiasErr[0]->GetMatrixArray()[i_vector];
            float corSpec = Correction_Bias[corFileIndex]->GetMatrixArray()[i_vectorSpecial[corFileIndex - 1]];
            float corSpecErr = Correction_BiasErr[corFileIndex]->GetMatrixArray()[i_vectorSpecial[corFileIndex - 1]];
            if (std::isnan(cor)) cor = 0.0;
            if (std::isnan(corErr)) corErr = 0.0;
            if (std::isnan(corSpec)) corSpec = 0.0;
            if (std::isnan(corSpecErr)) corSpecErr = 0.0;
            if (cor != 0.0)
              if (fabs(cor) - (3.0 * corErr) < 0.0) cor = 0.0;
            if (corSpec != 0.0)
              if (fabs(corSpec) - (3.0 * corSpecErr) < 0.0) corSpec = 0.0;
            if (cor != 0.0) {
//              if (fabs(corSpec - cor) > 1.0 * (corErr + corSpecErr)) {
              if (fabs(corSpec - cor) > m_DifferenceClose) {
                iNotMatchBiasCors++;
                B2DEBUG(110, "ShPkAxAuAv " << i_shape << "_" << i_pk << "_" << i_axis << "_" << i_angleU << "_" << i_angleV <<
                        " Difference of bias corrections bigger than expected: basic: " << cor * 10000.0 << "+-" <<
                        corErr * 10000.0 << ", special" << corFileIndex << ": " <<
                        corSpec * 10000.0 << "+-" <<
                        corSpecErr * 10000.0 << " , difference: " <<
                        (corSpec - cor) * 10000.0 << " [all in microns]"
                       );
              } else {
                iMatchBiasCors++;
                B2DEBUG(90, "ShPkAxAuAv " << i_shape << "_" << i_pk << "_" << i_axis << "_" << i_angleU << "_" << i_angleV <<
                        " Basic and special of bias correction match: " << cor * 10000.0 << "+-" <<
                        corErr * 10000.0 << ", special" << corFileIndex << ": " <<
                        corSpec * 10000.0 << "+-" <<
                        corSpecErr * 10000.0 << " , difference: " <<
                        (corSpec - cor) * 10000.0 << " [all in microns]"
                       );
              }
            }
            if ((cor == 0.0) && (corSpec != 0.0)) {
              //  cor = corSpec;
              //  corErr = corSpecErr;
              B2DEBUG(110, "ShPkAxAuAv " << i_shape << "_" << i_pk << "_" << i_axis << "_" << i_angleU << "_" << i_angleV <<
                      " Bias correction use from special: " << cor * 10000.0 << " [microns]");
            }
            cor = corSpec;        // TODO: for temprorary time special calibration is set.
            corErr = corSpecErr;  // TODO: for temprorary time special calibration is set.
            if (cor != 0.0) {
              iBiasCorsSum++;
              hAllB->Fill(i_shape + 1);
              hPKB[corFileIndex - 1]->Fill(i_shape + 1);
              hAllBVal->Fill(cor / Unit::um);
              hPKBVal[corFileIndex - 1]->Fill(cor / Unit::um);
              hClSBVal[i_shape]->Fill(cor / Unit::um);
              ValueCorsBias[i_vector] = cor;
              ValueCorsBiasErr[i_vector] = corErr;
            }

            // ERROR ESTIMATION part:
            // EE is value arround 1.0 in best case,
            //     saved value is decrease of 1.0 to move to arround 0.0
            // TODO: for temprorary time special calibration is set.
            cor = Correction_ErrorEstimation[0]->GetMatrixArray()[i_vector];
            corSpec = Correction_ErrorEstimation[corFileIndex]->GetMatrixArray()[i_vectorSpecial[corFileIndex - 1]];
            if (std::isnan(cor)) cor = 0.0;
            if (std::isnan(corSpec)) corSpec = 0.0;
            if (cor != 0.0) {
              if (fabs(corSpec - cor) > m_DifferenceErrEst) {
                iNotMatchEECors++;
                B2DEBUG(110, "ShPkAxAuAv " << i_shape << "_" << i_pk << "_" << i_axis << "_" << i_angleU << "_" << i_angleV <<
                        " Difference of error estimations bigger than expected: basic: " << cor <<
                        ", special" << corFileIndex << ": " << corSpec << " , difference: " << corSpec - cor
                       );
              } else {
                iMatchEECors++;
                B2DEBUG(90, "ShPkAxAuAv " << i_shape << "_" << i_pk << "_" << i_axis << "_" << i_angleU << "_" << i_angleV <<
                        " Basic and special of error estimations match: " << cor <<
                        ", special" << corFileIndex << ": " << corSpec << " , difference: " << corSpec - cor
                       );
              }
            }
            if ((cor == 0.0) && (corSpec != 0.0)) {
              //  cor = corSpec;
              B2DEBUG(110, "ShPkAxAuAv " << i_shape << "_" << i_pk << "_" << i_axis << "_" << i_angleU << "_" << i_angleV <<
                      " Error estimation correction use from special: " << cor);
            }
            cor = corSpec;  // TODO: for temprorary time special calibration is set.
            if (cor != 0.0) {
              iEECorsSum++;
              hAllEE->Fill(i_shape + 1);
              hPKEE[corFileIndex - 1]->Fill(i_shape + 1);
              hAllEEVal->Fill(cor);
              hPKEEVal[corFileIndex - 1]->Fill(cor);
              hClSEEVal[i_shape]->Fill(cor);
              ValueCorsErrEst[i_vector] = cor;
            }
          }  // Angle V
        }    // Angle U
      }      // Axis
    }        // Pixel kinds
  }          // Shapes
  Correction_Bias[5]->SetElements(ValueCorsBias);
  Correction_BiasErr[5]->SetElements(ValueCorsBiasErr);
  Correction_ErrorEstimation[5]->SetElements(ValueCorsErrEst);
  Correction_ErrorEstimationCovariance[5]->SetElements(ValueCorsErrEstCov);
  InPixelPosition[5]->SetElements(ValueCorsInPixPos);

  TString nameHist = Form("PXDShCalibrationBasicSetting");
  TString nameB = Form("Correction_Bias");
  TString nameBE = Form("Correction_BiasErr");
  TString nameEE = Form("Correction_ErrorEstimation");
  TString nameEC = Form("Correction_ErrorEstimationCovariance");
  TString nameIP = Form("InPixelPosition");
  if (m_CreateDB == 1) {
    IntervalOfValidity iov(0, 0, -1, -1);

    double BasicSet[7];
    for (int i = 0; i < 7; i++) {
      BasicSet[i] = PXDShCalibrationBasicSetting[0]->GetMatrixArray()[i];
    }

    TString Name = Form("PXDClSh_BasicSetting");
    DBImportObjPtr<TVectorD> PXDShCalibrationBasicSet(Name.Data());
    PXDShCalibrationBasicSet.construct(7);
    PXDShCalibrationBasicSet->SetElements(BasicSet);
    PXDShCalibrationBasicSet.import(iov);

    Name = Form("PXDClSh_Correction_Bias");
    DBImportObjPtr<TVectorD> CorrectionBias(Name.Data());
    CorrectionBias.construct(m_shapes * m_pixelkinds * m_dimensions * m_anglesU * m_anglesV);
    CorrectionBias->SetElements(ValueCorsBias);
    CorrectionBias.import(iov);

    Name = Form("PXDClSh_Correction_BiasErr");
    DBImportObjPtr<TVectorD> CorrectionBiasErr(Name.Data());
    CorrectionBiasErr.construct(m_shapes * m_pixelkinds * m_dimensions * m_anglesU * m_anglesV);
    CorrectionBiasErr->SetElements(ValueCorsBiasErr);
    CorrectionBiasErr.import(iov);

    Name = Form("PXDClSh_Correction_ErrorEstimation");
    DBImportObjPtr<TVectorD> CorrectionErrorEstimation(Name.Data());
    CorrectionErrorEstimation.construct(m_shapes * m_pixelkinds * m_dimensions * m_anglesU * m_anglesV);
    CorrectionErrorEstimation->SetElements(ValueCorsErrEst);
    CorrectionErrorEstimation.import(iov);

    Name = Form("PXDClSh_Correction_ErrorEstimationCovariance");
    DBImportObjPtr<TVectorD> CorrectionErrorEstimationCovariance(Name.Data());
    CorrectionErrorEstimationCovariance.construct(m_shapes * m_pixelkinds * m_dimensions * m_anglesU * m_anglesV);
    CorrectionErrorEstimationCovariance->SetElements(ValueCorsErrEstCov);
    CorrectionErrorEstimationCovariance.import(iov);

    Name = Form("PXDClSh_InPixelPosition");
    DBImportObjPtr<TVectorD> InPixelPositionDB(Name.Data());
    InPixelPositionDB.construct(m_shapes * m_pixelkinds * m_anglesU * m_anglesV * m_in_pixelU * m_in_pixelV);
    InPixelPositionDB->SetElements(ValueCorsInPixPos);
    InPixelPositionDB.import(iov);

    B2INFO("Output local database... done. ");
  } else {
    TFile* f = new TFile(m_CalFileOutputName.c_str(), "recreate");
    PXDShCalibrationBasicSetting[0]->Write(nameHist.Data());
    Correction_Bias[5]->Write(nameB.Data());
    Correction_BiasErr[5]->Write(nameBE.Data());
    Correction_ErrorEstimation[5]->Write(nameEE.Data());
    Correction_ErrorEstimationCovariance[5]->Write(nameEC.Data());
    InPixelPosition[5]->Write(nameIP.Data());
    B2DEBUG(130, "-----> Inspection: try to read non-zero bias elements:");
    B2DEBUG(130, "-----> Corrections of bias, biasError, ErrorEstimation and ErrorEstimationCovariance:");
    for (int i = 0; i < m_shapes * m_pixelkinds * m_dimensions * m_anglesU * m_anglesV; i++) {
      if (fabs(Correction_Bias[5]->GetMatrixArray()[i]) > m_Difference) {
        B2DEBUG(130, "     --> (" << i << "=" <<
                (int)((float)i / (m_pixelkinds * m_dimensions * m_anglesU * m_anglesV)) << "_" <<
                (int)((i % (m_pixelkinds * m_dimensions * m_anglesU * m_anglesV)) / (m_dimensions * m_anglesU * m_anglesV)) << "_" <<
                (int)((i % (m_dimensions * m_anglesU * m_anglesV)) / (m_anglesU * m_anglesV)) << "_" <<
                (int)((i % (m_anglesU * m_anglesV)) / m_anglesV) << "_" <<
                (int)(i % (m_anglesV)) << ")" <<
                Correction_Bias[5]->GetMatrixArray()[i] * 10000.0 << " - " <<
                Correction_BiasErr[5]->GetMatrixArray()[i] * 10000.0 << " - " <<
                Correction_ErrorEstimation[5]->GetMatrixArray()[i] << " - " <<
                Correction_ErrorEstimationCovariance[5]->GetMatrixArray()[i]
               );
      }
    }
    B2DEBUG(130, "-----> Corrections of InPixel corrections:");
    for (int i = 0; i < m_shapes * m_pixelkinds * m_anglesU * m_anglesV * m_in_pixelU * m_in_pixelV; i++) {
      if (InPixelPosition[5]->GetMatrixArray()[i] > 0) {
        B2DEBUG(130, "     --> " << InPixelPosition[5]->GetMatrixArray()[i] << " (" << i << ")");
      }
    }
    f->Close();
    B2INFO("Output correction file: " << m_CalFileOutputName.c_str() << " ... done.");
  }

  delete[] ValueCorsBias;
  delete[] ValueCorsBiasErr;
  delete[] ValueCorsErrEst;
  delete[] ValueCorsErrEstCov;
  delete[] ValueCorsInPixPos;
  // ******************* START show some statistcs and save to asci and root file: **********
  TFile* froot = new TFile(sHistoFileName.Data(), "recreate");

  // values:
  nameHist = Form("PXDShCalSummary_BiasValues_All");
  hAllBVal->Write(nameHist.Data());
  nameHist = Form("PXDShCalSummary_BiasValues_PixKind0");
  hPKBVal[0]->Write(nameHist.Data());
  nameHist = Form("PXDShCalSummary_BiasValues_PixKind1");
  hPKBVal[1]->Write(nameHist.Data());
  nameHist = Form("PXDShCalSummary_BiasValues_PixKind2");
  hPKBVal[2]->Write(nameHist.Data());
  nameHist = Form("PXDShCalSummary_BiasValues_PixKind3");
  hPKBVal[3]->Write(nameHist.Data());
  for (int iSh = 0; iSh < m_shapes; iSh++) {
    nameHist = Form("PXDShCalSummary_BiasValues_Shape%i", iSh + 1);
    hClSBVal[iSh]->Write(nameHist.Data());
  }

  nameHist = Form("PXDShCalSummary_ErrEstValues_All");
  hAllEEVal->Write(nameHist.Data());
  nameHist = Form("PXDShCalSummary_ErrEstValues_PixKind0");
  hPKEEVal[0]->Write(nameHist.Data());
  nameHist = Form("PXDShCalSummary_ErrEstValues_PixKind1");
  hPKEEVal[1]->Write(nameHist.Data());
  nameHist = Form("PXDShCalSummary_ErrEstValues_PixKind2");
  hPKEEVal[2]->Write(nameHist.Data());
  nameHist = Form("PXDShCalSummary_ErrEstValues_PixKind3");
  hPKEEVal[3]->Write(nameHist.Data());
  for (int iSh = 0; iSh < m_shapes; iSh++) {
    nameHist = Form("PXDShCalSummary_ErrEstValues_Shape%i", iSh + 1);
    hClSEEVal[iSh]->Write(nameHist.Data());
  }

  // Counts:
  nameHist = Form("PXDShCalSummary_Bias_All");
  hAllB->Write(nameHist.Data());
  nameHist = Form("PXDShCalSummary_Bias_PixKind0");
  hPKB[0]->Write(nameHist.Data());
  nameHist = Form("PXDShCalSummary_Bias_PixKind1");
  hPKB[1]->Write(nameHist.Data());
  nameHist = Form("PXDShCalSummary_Bias_PixKind2");
  hPKB[2]->Write(nameHist.Data());
  nameHist = Form("PXDShCalSummary_Bias_PixKind3");
  hPKB[3]->Write(nameHist.Data());

  nameHist = Form("PXDShCalSummary_ErrEst_All");
  hAllEE->Write(nameHist.Data());
  nameHist = Form("PXDShCalSummary_ErrEst_PixKind0");
  hPKEE[0]->Write(nameHist.Data());
  nameHist = Form("PXDShCalSummary_ErrEst_PixKind1");
  hPKEE[1]->Write(nameHist.Data());
  nameHist = Form("PXDShCalSummary_ErrEst_PixKind2");
  hPKEE[2]->Write(nameHist.Data());
  nameHist = Form("PXDShCalSummary_ErrEst_PixKind3");
  hPKEE[3]->Write(nameHist.Data());

  froot->Close();

  TString TextSh;
  FILE* fLogFile = fopen(sLogFileName.Data(), "w");
  TextSh = Form("There are match corrections for Bias:\n     %i from %i , not match %i (all= %i )\n",
                iMatchBiasCors, iBiasCorsSum, iNotMatchBiasCors, iBiasAll);
  TextSh = Form("%sfor Error Estimation:\n     %i from %i , not match %i (all= %i )\n",
                TextSh.Data(), iMatchEECors, iEECorsSum, iNotMatchEECors, iEEAll);
  TextSh = Form("%sfor In Pixel Corrections:\n     %i from %i , not match %i (all= %i )\n",
                TextSh.Data(), iMatchInPixCors, iInPixCorsSum, iNotMatchInPixCors, iInPixAll);
  B2INFO(TextSh.Data());
  fprintf(fLogFile, "%s", TextSh.Data());
  TextSh = Form("No. of Bias corrections in shapes:");
  B2INFO(TextSh.Data());
  fprintf(fLogFile, "%s\n", TextSh.Data());
  for (int iSh = 0; iSh < m_shapes; iSh++) {
    TextSh = Form(" Shape %i All %i PK0 %i PK1 %i PK2 %i PK3 %i",
                  iSh + 1, (int)hAllB->GetBinContent(iSh + 1),
                  (int)hPKB[0]->GetBinContent(iSh + 1), (int)hPKB[1]->GetBinContent(iSh + 1),
                  (int)hPKB[2]->GetBinContent(iSh + 1), (int)hPKB[3]->GetBinContent(iSh + 1)
                 );
    B2INFO(TextSh.Data());
    fprintf(fLogFile, "%s\n", TextSh.Data());
  }
  TextSh = Form("No. of Error Estimation corrections in shapes:");
  B2INFO(TextSh.Data());
  fprintf(fLogFile, "%s\n", TextSh.Data());
  for (int iSh = 0; iSh < m_shapes; iSh++) {
    TextSh = Form(" Shape %i All %i PK0 %i PK1 %i PK2 %i PK3 %i",
                  iSh + 1, (int)hAllEE->GetBinContent(iSh + 1),
                  (int)hPKEE[0]->GetBinContent(iSh + 1), (int)hPKEE[1]->GetBinContent(iSh + 1),
                  (int)hPKEE[2]->GetBinContent(iSh + 1), (int)hPKEE[3]->GetBinContent(iSh + 1)
                 );
    B2INFO(TextSh.Data());
    fprintf(fLogFile, "%s\n", TextSh.Data());
  }
  fclose(fLogFile);
  // ******************* END show some statistcs and save to asci and root file: ************
  B2INFO("Output result files saved.");
}

void pxdMergeClusterShapeCorrectionsModule::event()
{
}

void pxdMergeClusterShapeCorrectionsModule::terminate()
{
}

