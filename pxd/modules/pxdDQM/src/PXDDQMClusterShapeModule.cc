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

#include <vxd/geometry/GeoCache.h>
#include <pxd/geometry/SensorInfo.h>
#include <genfit/MeasurementOnPlane.h>
#include <pxd/modules/pxdDQM/PXDDQMClusterShapeModule.h>
#include <pxd/reconstruction/PXDClusterShape.h>
#include <pxd/reconstruction/PXDRecoHit.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/RelationArray.h>
#include <pxd/dataobjects/PXDFrame.h>
#include <tracking/dataobjects/RecoTrack.h>
#include <tracking/dataobjects/RecoHitInformation.h>
#include <pxd/dataobjects/PXDTrueHit.h>
#include <TFile.h>
#include <pxd/reconstruction/HitCorrector.h>

#include <framework/database/DBObjPtr.h>

#include <TDirectory.h>
#include <TVectorD.h>

using namespace Belle2;
using namespace Belle2::PXD;
using namespace std;
using boost::format;

class PXDClusterShapeCalibrationAlgorithm;


//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(PXDDQMClusterShape)


//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

PXDDQMClusterShapeModule::PXDDQMClusterShapeModule() : HistoModule()
{
  //Set module properties
  setDescription("PXD DQM of Cluster Shape Correction"
                 "Basic steps for using of corrections are: "
                 " - pxdClusterShapeCalibration - create source for correction calculation "
                 " - PXDClusterShapeCalibrationAlgorithm - calculate corrections "
                 " - pxdMergeClusterShapeCorrections - merge results and create DB object "
                 " - actualization of global condition DB "
                 " - PXDDQMClusterShape - check results on DQM histograms <--- this module "
                 " "
                 " For this run stearing files: "
                 " - pxd/examples/PXDClasterShape_PrepareSources.py (5x) "
                 " - pxd/examples/PXDClasterShape_CalculateCorrections.py (5x) "
                 " - pxd/examples/PXDClasterShape_SourceMergator.py "
                 " - vxd/examples/DQMPXDClusterShape.py "
                 " "
                 "Detail description of precedure is on PXDClusterShapeCalibrationAlgorithmModule "
                 " "
                );
  setPropertyFlags(c_ParallelProcessingCertified);

  addParam("CalFileName", m_CalFileName,
           "Name of file contain output merged calibration, default=PXD-ClasterShapeCorrections", m_CalFileName);
  addParam("SeeDQMOfCalibration", m_SeeDQMOfCalibration,
           "flag <0,1>, create DQM of calibration in detail (log file, histograms), default = 0", m_SeeDQMOfCalibration);
  addParam("UseRealData", m_UseRealData,
           "flag <0,1>, using of real data is skip true points availabilities, default = 0", m_UseRealData);

}


PXDDQMClusterShapeModule::~PXDDQMClusterShapeModule()
{
}

//------------------------------------------------------------------
// Function to define histograms
//-----------------------------------------------------------------

void PXDDQMClusterShapeModule::initialize()
{
  // Register histograms (calls back defineHisto)
  REG_HISTOGRAM

  //Register collections
  StoreArray<PXDDigit> storePXDDigits(m_storePXDDigitsName);
  StoreArray<PXDCluster> storePXDClusters(m_storePXDClustersName);
  RelationArray relPXDClusterDigits(storePXDClusters, storePXDDigits);
  m_storePXDClustersName = storePXDClusters.getName();
  m_relPXDClusterDigitName = relPXDClusterDigits.getName();

  //Store names to speed up creation later
  m_storePXDDigitsName = storePXDDigits.getName();

  StoreArray<RecoTrack> recotracks(m_storeRecoTrackName);
  m_storeRecoTrackName = recotracks.getName();

  StoreArray<PXDRecoHit> pxdrecohit(m_storePXDRecoHitName);
  m_storePXDRecoHitName = pxdrecohit.getName();

  StoreArray<PXDTrueHit> pxdtruehit(m_storePXDTrueHitsName);
  m_storePXDTrueHitsName = pxdtruehit.getName();

  StoreArray<PXDFrame> storeFrames(m_storeFramesName);
  m_storeFramesName = storeFrames.getName();

  RelationArray relClustersTrueHits(pxdrecohit, pxdtruehit);
}

void PXDDQMClusterShapeModule::defineHisto()
{
  // Create a separate histogram directories and cd into it.
  TDirectory* oldDir = gDirectory;

  // save way to create directories and cd to them, even if they exist already.
  // if cd fails, Directory points to oldDir, but never ZERO ptr
  oldDir->mkdir("PXD_ClusterShapeCorrections");
  oldDir->cd("PXD_ClusterShapeCorrections");
  TDirectory* DirPXDClShCorrs = gDirectory;

  oldDir->mkdir("PXD_ClusterShape");
  oldDir->cd("PXD_ClusterShape");
  TDirectory* DirPXDBasic = gDirectory;

  oldDir->mkdir("PXD_ClusterShape_OnlyShapeL");
  oldDir->cd("PXD_ClusterShape_OnlyShapeL");
  TDirectory* DirPXDBasicShapeL = gDirectory;

  oldDir->mkdir("PXD_ClusterShapeEE");
  oldDir->cd("PXD_ClusterShapeEE");
  TDirectory* DirPXDBasicEE = gDirectory;

  oldDir->mkdir("PXD_ClusterShapeEE_OnlyShapeL");
  oldDir->cd("PXD_ClusterShapeEE_OnlyShapeL");
  TDirectory* DirPXDBasicEEShapeL = gDirectory;
  oldDir->cd();

  if (m_SeeDQMOfCalibration) {
    // ************ START preparing of histograms for monitoring of corrections:
    DirPXDClShCorrs->cd();

    // START - load corrections:

    B2INFO("Load correction files:");

    TVectorD* PXDShCalibrationBasicSetting = NULL;
    TVectorD* Correction_Bias = NULL;
    TVectorD* Correction_ErrorEstimation = NULL;
    TVectorD* Correction_ErrorEstimationCovariance = NULL;
    TVectorD* Correction_BiasErr = NULL;
    TVectorD* InPixelPosition = NULL;

    if (m_UseCorrectionsFromFile == 0) {
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
      DBObjPtr<TVectorD> Correction_BiasDB(Name.Data());
      if (!Correction_BiasDB.isValid()) {
        isLoad = 0;
      }
      Name = Form("PXDClSh_Correction_BiasErr");
      DBObjPtr<TVectorD> Correction_BiasErrDB(Name.Data());
      if (!Correction_BiasErrDB.isValid()) {
        isLoad = 0;
      }
      Name = Form("PXDClSh_Correction_ErrorEstimation");
      DBObjPtr<TVectorD> Correction_ErrorEstimationDB(Name.Data());
      if (!Correction_ErrorEstimationDB.isValid()) {
        isLoad = 0;
      }
      Name = Form("PXDClSh_Correction_ErrorEstimationCovariance");
      DBObjPtr<TVectorD> Correction_ErrorEstimationCovarianceDB(Name.Data());
      if (!Correction_ErrorEstimationCovarianceDB.isValid()) {
        isLoad = 0;
      }
      Name = Form("PXDClSh_InPixelPosition");
      DBObjPtr<TVectorD> InPixelPositionDB(Name.Data());
      if (!InPixelPositionDB.isValid()) {
        isLoad = 0;
      }
      if (isLoad == 0) {
        B2INFO("Could not open correction in DB: Shape corrections will not be applied.");
        return;
      }
      Correction_Bias = new TVectorD(m_shapes * m_pixelkinds * m_dimensions * m_anglesU * m_anglesV);
      Correction_ErrorEstimation = new TVectorD(m_shapes * m_pixelkinds * m_dimensions * m_anglesU * m_anglesV);
      Correction_ErrorEstimationCovariance = new TVectorD(m_shapes * m_pixelkinds * m_dimensions * m_anglesU * m_anglesV);
      Correction_BiasErr = new TVectorD(m_shapes * m_pixelkinds * m_dimensions * m_anglesU * m_anglesV);
      InPixelPosition = new TVectorD(m_shapes * m_pixelkinds * m_anglesU * m_anglesV * m_in_pixelU * m_in_pixelV);
      *Correction_Bias = *Correction_BiasDB;
      *Correction_BiasErr = *Correction_BiasErrDB;
      *Correction_ErrorEstimation = *Correction_ErrorEstimationDB;
      *Correction_ErrorEstimationCovariance = *Correction_ErrorEstimationCovarianceDB;
      *InPixelPosition = *InPixelPositionDB;

      B2DEBUG(80, "HitCorrector:  ---> Field dimensions: shapes: " << m_shapes << ", pixelkinds: " << m_pixelkinds <<
              ", dimensions: " << m_dimensions <<
              ", anglesU: " << m_anglesU << ", anglesV: " << m_anglesV << ", in_pixelU: " << m_in_pixelU << ", in_pixelV: " << m_in_pixelV);
    } else {
      TFile* f_CalBasic = new TFile(m_CalFileName.c_str(), "read");
      if (f_CalBasic->IsOpen()) {
        B2INFO("Correction file Basic: " << m_CalFileName.c_str());

        // to fill output corrections with index 6:
        f_CalBasic->GetObject("PXDShCalibrationBasicSetting", PXDShCalibrationBasicSetting);
        f_CalBasic->GetObject("Correction_Bias", Correction_Bias);
        f_CalBasic->GetObject("Correction_ErrorEstimation", Correction_ErrorEstimation);
        f_CalBasic->GetObject("Correction_ErrorEstimationCovariance", Correction_ErrorEstimationCovariance);
        f_CalBasic->GetObject("Correction_BiasErr", Correction_BiasErr);
        f_CalBasic->GetObject("InPixelPosition", InPixelPosition);

        m_shapes = (int)PXDShCalibrationBasicSetting->GetMatrixArray()[0];
        m_pixelkinds = (int)PXDShCalibrationBasicSetting->GetMatrixArray()[1];
        m_dimensions = (int)PXDShCalibrationBasicSetting->GetMatrixArray()[2];
        m_anglesU = (int)PXDShCalibrationBasicSetting->GetMatrixArray()[3];
        m_anglesV = (int)PXDShCalibrationBasicSetting->GetMatrixArray()[4];
        m_in_pixelU = (int)PXDShCalibrationBasicSetting->GetMatrixArray()[5];
        m_in_pixelV = (int)PXDShCalibrationBasicSetting->GetMatrixArray()[6];
        B2INFO("---> Field dimensions: shapes: " << m_shapes << ", pixelkinds: " << m_pixelkinds <<
               ", dimensions: " << m_dimensions <<
               ", anglesU: " << m_anglesU << ", anglesV: " << m_anglesV << ", in_pixelU: " << m_in_pixelU << ", in_pixelV: " << m_in_pixelV);
        f_CalBasic->Close();
      } else {
        B2INFO("File of basic calibration: " << m_CalFileName.c_str() << " is not available, please check it!");
        return;
      }
    }
    B2DEBUG(130, "-----> try to read non-zero elements:");
    for (int i_shape = 0; i_shape < 7; i_shape++) {
      int i_vector = i_shape * m_pixelkinds * m_dimensions * m_anglesU * m_anglesV;
      i_vector += 3 * m_dimensions * m_anglesU * m_anglesV;
      i_vector += 0 * m_anglesU * m_anglesV;
      i_vector += 9 * m_anglesV;
      i_vector += 12;
      float fDifference = 0.1 * Unit::um;
      if (fabs(Correction_Bias->GetMatrixArray()[i_vector]) > fDifference) {
        B2DEBUG(130, "     --> " << Correction_Bias->GetMatrixArray()[i_vector] << " (" << i_vector << ")");
      }
    }

    TString sLogFileName = m_CalFileName.c_str();
    sLogFileName.ReplaceAll(".root", ".log");
    TString sHistoFileName = m_CalFileName.c_str();
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
//              ValueCorsErrEstCov[i_vector] = Correction_ErrorEstimationCovariance->GetMatrixArray()[i_vector];
//              if (std::isnan(Correction_ErrorEstimationCovariance->GetMatrixArray()[i_vector])) ValueCorsErrEstCov[i_vector] = 0.0;
              float cor = Correction_Bias->GetMatrixArray()[i_vector];
              float corErr = Correction_BiasErr->GetMatrixArray()[i_vector];
              if (std::isnan(cor)) cor = 0.0;
              if (std::isnan(corErr)) corErr = 0.0;
              if (cor != 0.0) {
                iBiasCorsSum++;
                hAllB->Fill(i_shape + 1);
                hPKB[corFileIndex - 1]->Fill(i_shape + 1);
                hAllBVal->Fill(cor / Unit::um);
                hPKBVal[corFileIndex - 1]->Fill(cor / Unit::um);
                hClSBVal[i_shape]->Fill(cor / Unit::um);
//                ValueCorsBias[i_vector] = cor;
//                ValueCorsBiasErr[i_vector] = corErr;
              }

              cor = Correction_ErrorEstimation->GetMatrixArray()[i_vector];
              if (std::isnan(cor)) cor = 0.0;
              if (cor != 0.0) {
                iEECorsSum++;
                hAllEE->Fill(i_shape + 1);
                hPKEE[corFileIndex - 1]->Fill(i_shape + 1);
                hAllEEVal->Fill(cor);
                hPKEEVal[corFileIndex - 1]->Fill(cor);
                hClSEEVal[i_shape]->Fill(cor);
//                ValueCorsErrEst[i_vector] = cor;
              }
            }  // Angle V
          }    // Angle U
        }      // Axis
      }        // Pixel kinds
    }          // Shapes

    TString nameHist;
    B2DEBUG(130, "-----> Inspection: try to read non-zero bias elements:");
    B2DEBUG(130, "-----> Corrections of bias, biasError, ErrorEstimation and ErrorEstimationCovariance:");
    for (int i = 0; i < m_shapes * m_pixelkinds * m_dimensions * m_anglesU * m_anglesV; i++) {
      B2DEBUG(130, "     --> (" << i << "=" <<
              (int)((float)i / (m_pixelkinds * m_dimensions * m_anglesU * m_anglesV)) << "_" <<
              (int)((i % (m_pixelkinds * m_dimensions * m_anglesU * m_anglesV)) / (m_dimensions * m_anglesU * m_anglesV)) << "_" <<
              (int)((i % (m_dimensions * m_anglesU * m_anglesV)) / (m_anglesU * m_anglesV)) << "_" <<
              (int)((i % (m_anglesU * m_anglesV)) / m_anglesV) << "_" <<
              (int)(i % (m_anglesV)) << ")" <<
              Correction_Bias->GetMatrixArray()[i] * 10000.0 << " - " <<
              Correction_BiasErr->GetMatrixArray()[i] * 10000.0 << " - " <<
              Correction_ErrorEstimation->GetMatrixArray()[i] << " - " <<
              Correction_ErrorEstimationCovariance->GetMatrixArray()[i]
             );
    }
    B2DEBUG(130, "-----> Corrections of InPixel corrections:");
    for (int i = 0; i < m_shapes * m_pixelkinds * m_anglesU * m_anglesV * m_in_pixelU * m_in_pixelV; i++) {
      if (InPixelPosition->GetMatrixArray()[i] > 0) {
        B2DEBUG(130, "     --> " << InPixelPosition->GetMatrixArray()[i] << " (" << i << ")");
      }
    }

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
  // ************ END preparing of histograms for monitoring of corrections

  const int NoOfPlots = 8;  // pixKind
  m_ResidualsPosInPlUBRH = new TH2F*[NoOfPlots];
  m_ResidualsPosInPlUBCl = new TH2F*[NoOfPlots];
  m_ResidualsTruePosInPlUB = new TH2F*[NoOfPlots];
  m_ResidualsPosInPlBRH = new TH2F*[NoOfPlots];
  m_ResidualsPosInPlBCl = new TH2F*[NoOfPlots];
  m_ResidualsTruePosInPlB = new TH2F*[NoOfPlots];
  m_ResidualsTrueRH = new TH2F*[NoOfPlots];
  m_ResidualsTrueCl = new TH2F*[NoOfPlots];
  m_EERH = new TH2F*[NoOfPlots];
  m_EECl = new TH2F*[NoOfPlots];
  m_EETrackUB = new TH2F*[NoOfPlots];
  m_EETrackB = new TH2F*[NoOfPlots];

  m_ReTrue_EECl = new TH2F*[NoOfPlots];
  m_ReTrue_EERH = new TH2F*[NoOfPlots];
  m_ReUnBiasRes_EECl = new TH2F*[NoOfPlots];
  m_ReUnBiasRes_EERH = new TH2F*[NoOfPlots];

  m_ReTrue_EEClShapeL = new TH2F*[NoOfPlots];
  m_ReTrue_EERHShapeL = new TH2F*[NoOfPlots];
  m_ReUnBiasRes_EEClShapeL = new TH2F*[NoOfPlots];
  m_ReUnBiasRes_EERHShapeL = new TH2F*[NoOfPlots];

  m_ResidualsPosInPlUBRHShapeL = new TH2F*[NoOfPlots];
  m_ResidualsPosInPlUBClShapeL = new TH2F*[NoOfPlots];
  m_ResidualsTruePosInPlUBShapeL = new TH2F*[NoOfPlots];
  m_ResidualsPosInPlBRHShapeL = new TH2F*[NoOfPlots];
  m_ResidualsPosInPlBClShapeL = new TH2F*[NoOfPlots];
  m_ResidualsTruePosInPlBShapeL = new TH2F*[NoOfPlots];
  m_ResidualsTrueRHShapeL = new TH2F*[NoOfPlots];
  m_ResidualsTrueClShapeL = new TH2F*[NoOfPlots];
  m_EERHShapeL = new TH2F*[NoOfPlots];
  m_EEClShapeL = new TH2F*[NoOfPlots];
  m_EETrackUBShapeL = new TH2F*[NoOfPlots];
  m_EETrackBShapeL = new TH2F*[NoOfPlots];

  for (int i = 0; i < NoOfPlots; i++) {
    m_ResidualsPosInPlUBRH[i] = NULL;
    m_ResidualsPosInPlUBCl[i] = NULL;
    m_ResidualsTruePosInPlUB[i] = NULL;
    m_ResidualsPosInPlBRH[i] = NULL;
    m_ResidualsPosInPlBCl[i] = NULL;
    m_ResidualsTruePosInPlB[i] = NULL;
    m_ResidualsTrueRH[i] = NULL;
    m_ResidualsTrueCl[i] = NULL;
    m_EERH[i] = NULL;
    m_EECl[i] = NULL;
    m_EETrackUB[i] = NULL;
    m_EETrackB[i] = NULL;

    m_ReTrue_EECl[i] = NULL;
    m_ReTrue_EERH[i] = NULL;
    m_ReUnBiasRes_EECl[i] = NULL;
    m_ReUnBiasRes_EERH[i] = NULL;

    m_ReTrue_EEClShapeL[i] = NULL;
    m_ReTrue_EERHShapeL[i] = NULL;
    m_ReUnBiasRes_EEClShapeL[i] = NULL;
    m_ReUnBiasRes_EERHShapeL[i] = NULL;

    m_ResidualsPosInPlUBRHShapeL[i] = NULL;
    m_ResidualsPosInPlUBClShapeL[i] = NULL;
    m_ResidualsTruePosInPlUBShapeL[i] = NULL;
    m_ResidualsPosInPlBRHShapeL[i] = NULL;
    m_ResidualsPosInPlBClShapeL[i] = NULL;
    m_ResidualsTruePosInPlBShapeL[i] = NULL;
    m_ResidualsTrueRHShapeL[i] = NULL;
    m_ResidualsTrueClShapeL[i] = NULL;
    m_EERHShapeL[i] = NULL;
    m_EEClShapeL[i] = NULL;
    m_EETrackUBShapeL[i] = NULL;
    m_EETrackBShapeL[i] = NULL;

    DirPXDBasic->cd();
    TString sPixelSize;
    int PlotsRange = 100;
    if (i == 0) sPixelSize = Form("60_Sensor1");
    else if (i == 1) sPixelSize = Form("55_Sensor1");
    else if (i == 2) sPixelSize = Form("55_Sensor2");
    else if (i == 3) sPixelSize = Form("60_Sensor2");
    else if (i == 4) sPixelSize = Form("85_Sensor1");
    else if (i == 5) sPixelSize = Form("70_Sensor1");
    else if (i == 6) sPixelSize = Form("70_Sensor2");
    else if (i == 7) sPixelSize = Form("85_Sensor2");
    string name = str(format("PXDResiduals_PosInPlUBRH_L%1%_Size%2%nm") % (i / 4) % sPixelSize.Data());
    string title = str(format("PXD Residuals Position In Plane UnBias vs. RecoHit, Layer %1%, Pixel Size %2% nm") %
                       (i / 4) % sPixelSize.Data());
    m_ResidualsPosInPlUBRH[i] = new TH2F(name.c_str(), title.c_str(), 2 * PlotsRange, -PlotsRange, PlotsRange, 2 * PlotsRange,
                                         -PlotsRange, PlotsRange);
    m_ResidualsPosInPlUBRH[i]->GetXaxis()->SetTitle("Residual in u [um]");
    m_ResidualsPosInPlUBRH[i]->GetYaxis()->SetTitle("Residual in v [um]");
    m_ResidualsPosInPlUBRH[i]->GetZaxis()->SetTitle("counts");
    name = str(format("PXDResiduals_PosInPlUBCl_L%1%_Size%2%nm") % (i / 4) % sPixelSize.Data());
    title = str(format("PXD Residuals Position In Plane UnBias vs. Cluster, Layer %1%, Pixel Size %2% nm") %
                (i / 4) % sPixelSize.Data());
    m_ResidualsPosInPlUBCl[i] = new TH2F(name.c_str(), title.c_str(), 2 * PlotsRange, -PlotsRange, PlotsRange, 2 * PlotsRange,
                                         -PlotsRange, PlotsRange);
    m_ResidualsPosInPlUBCl[i]->GetXaxis()->SetTitle("Residual in u [um]");
    m_ResidualsPosInPlUBCl[i]->GetYaxis()->SetTitle("Residual in v [um]");
    m_ResidualsPosInPlUBCl[i]->GetZaxis()->SetTitle("counts");
    name = str(format("PXDResiduals_TruePosInPlUB_L%1%_Size%2%nm") % (i / 4) % sPixelSize.Data());
    title = str(format("PXD Residuals TrueHit vs Position In Plane UnBias, Layer %1%, Pixel Size %2% nm") %
                (i / 4) % sPixelSize.Data());
    m_ResidualsTruePosInPlUB[i] = new TH2F(name.c_str(), title.c_str(), 2 * PlotsRange, -PlotsRange, PlotsRange, 2 * PlotsRange,
                                           -PlotsRange, PlotsRange);
    m_ResidualsTruePosInPlUB[i]->GetXaxis()->SetTitle("Residual in u [um]");
    m_ResidualsTruePosInPlUB[i]->GetYaxis()->SetTitle("Residual in v [um]");
    m_ResidualsTruePosInPlUB[i]->GetZaxis()->SetTitle("counts");
    name = str(format("PXDResiduals_PosInPlBRH_L%1%_Size%2%nm") % (i / 4) % sPixelSize.Data());
    title = str(format("PXD Residuals Position In Plane Bias vs. RecoHit, Layer %1%, Pixel Size %2% nm") %
                (i / 4) % sPixelSize.Data());
    m_ResidualsPosInPlBRH[i] = new TH2F(name.c_str(), title.c_str(), 2 * PlotsRange, -PlotsRange, PlotsRange, 2 * PlotsRange,
                                        -PlotsRange, PlotsRange);
    m_ResidualsPosInPlBRH[i]->GetXaxis()->SetTitle("Residual in u [um]");
    m_ResidualsPosInPlBRH[i]->GetYaxis()->SetTitle("Residual in v [um]");
    m_ResidualsPosInPlBRH[i]->GetZaxis()->SetTitle("counts");
    name = str(format("PXDResiduals_PosInPlBCl_L%1%_Size%2%nm") % (i / 4) % sPixelSize.Data());
    title = str(format("PXD Residuals Position In Plane Bias vs. Cluster, Layer %1%, Pixel Size %2% nm") % (i / 4) % sPixelSize.Data());
    m_ResidualsPosInPlBCl[i] = new TH2F(name.c_str(), title.c_str(), 2 * PlotsRange, -PlotsRange, PlotsRange, 2 * PlotsRange,
                                        -PlotsRange, PlotsRange);
    m_ResidualsPosInPlBCl[i]->GetXaxis()->SetTitle("Residual in u [um]");
    m_ResidualsPosInPlBCl[i]->GetYaxis()->SetTitle("Residual in v [um]");
    m_ResidualsPosInPlBCl[i]->GetZaxis()->SetTitle("counts");
    name = str(format("PXDResiduals_TruePosInPlB_L%1%_Size%2%nm") % (i / 4) % sPixelSize.Data());
    title = str(format("PXD Residuals TrueHit vs Position In Plane Bias, Layer %1%, Pixel Size %2% nm") % (i / 4) % sPixelSize.Data());
    m_ResidualsTruePosInPlB[i] = new TH2F(name.c_str(), title.c_str(), 2 * PlotsRange, -PlotsRange, PlotsRange, 2 * PlotsRange,
                                          -PlotsRange, PlotsRange);
    m_ResidualsTruePosInPlB[i]->GetXaxis()->SetTitle("Residual in u [um]");
    m_ResidualsTruePosInPlB[i]->GetYaxis()->SetTitle("Residual in v [um]");
    m_ResidualsTruePosInPlB[i]->GetZaxis()->SetTitle("counts");
    name = str(format("PXDResiduals_TrueRecoHit_L%1%_Size%2%nm") % (i / 4) % sPixelSize.Data());
    title = str(format("PXD Residuals TrueHit vs RecoHit, Layer %1%, Pixel Size %2% nm") % (i / 4) % sPixelSize.Data());
    m_ResidualsTrueRH[i] = new TH2F(name.c_str(), title.c_str(), 2 * PlotsRange, -PlotsRange, PlotsRange, 2 * PlotsRange, -PlotsRange,
                                    PlotsRange);
    m_ResidualsTrueRH[i]->GetXaxis()->SetTitle("Residual in u [um]");
    m_ResidualsTrueRH[i]->GetYaxis()->SetTitle("Residual in v [um]");
    m_ResidualsTrueRH[i]->GetZaxis()->SetTitle("counts");
    name = str(format("PXDResiduals_TrueCluster_L%1%_Size%2%nm") % (i / 4) % sPixelSize.Data());
    title = str(format("PXD Residuals TrueHit vs Cluster, Layer %1%, Pixel Size %2% nm") % (i / 4) % sPixelSize.Data());
    m_ResidualsTrueCl[i] = new TH2F(name.c_str(), title.c_str(), 2 * PlotsRange, -PlotsRange, PlotsRange, 2 * PlotsRange, -PlotsRange,
                                    PlotsRange);
    m_ResidualsTrueCl[i]->GetXaxis()->SetTitle("Residual in u [um]");
    m_ResidualsTrueCl[i]->GetYaxis()->SetTitle("Residual in v [um]");
    m_ResidualsTrueCl[i]->GetZaxis()->SetTitle("counts");

    DirPXDBasicShapeL->cd();
    name = str(format("PXDResiduals_PosInPlUBRH_ShapeL_L%1%_Size%2%nm") % (i / 4) % sPixelSize.Data());
    title = str(format("PXD Residuals Position In Plane UnBias vs. RecoHit for Shape L, Layer %1%, Pixel Size %2% nm") %
                (i / 4) % sPixelSize.Data());
    m_ResidualsPosInPlUBRHShapeL[i] = new TH2F(name.c_str(), title.c_str(), 2 * PlotsRange, -PlotsRange, PlotsRange, 2 * PlotsRange,
                                               -PlotsRange, PlotsRange);
    m_ResidualsPosInPlUBRHShapeL[i]->GetXaxis()->SetTitle("Residual in u [um]");
    m_ResidualsPosInPlUBRHShapeL[i]->GetYaxis()->SetTitle("Residual in v [um]");
    m_ResidualsPosInPlUBRHShapeL[i]->GetZaxis()->SetTitle("counts");
    name = str(format("PXDResiduals_PosInPlUBCl_ShapeL_L%1%_Size%2%nm") % (i / 4) % sPixelSize.Data());
    title = str(format("PXD Residuals Position In Plane UnBias vs. Cluster for Shape L, Layer %1%, Pixel Size %2% nm") %
                (i / 4) % sPixelSize.Data());
    m_ResidualsPosInPlUBClShapeL[i] = new TH2F(name.c_str(), title.c_str(), 2 * PlotsRange, -PlotsRange, PlotsRange, 2 * PlotsRange,
                                               -PlotsRange, PlotsRange);
    m_ResidualsPosInPlUBClShapeL[i]->GetXaxis()->SetTitle("Residual in u [um]");
    m_ResidualsPosInPlUBClShapeL[i]->GetYaxis()->SetTitle("Residual in v [um]");
    m_ResidualsPosInPlUBClShapeL[i]->GetZaxis()->SetTitle("counts");
    name = str(format("PXDResiduals_TruePosInPlUB_ShapeL_L%1%_Size%2%nm") % (i / 4) % sPixelSize.Data());
    title = str(format("PXD Residuals TrueHit vs Position In Plane UnBias for Shape L, Layer %1%, Pixel Size %2% nm") %
                (i / 4) % sPixelSize.Data());
    m_ResidualsTruePosInPlUBShapeL[i] = new TH2F(name.c_str(), title.c_str(), 2 * PlotsRange, -PlotsRange, PlotsRange, 2 * PlotsRange,
                                                 -PlotsRange, PlotsRange);
    m_ResidualsTruePosInPlUBShapeL[i]->GetXaxis()->SetTitle("Residual in u [um]");
    m_ResidualsTruePosInPlUBShapeL[i]->GetYaxis()->SetTitle("Residual in v [um]");
    m_ResidualsTruePosInPlUBShapeL[i]->GetZaxis()->SetTitle("counts");
    name = str(format("PXDResiduals_PosInPlBRH_ShapeL_L%1%_Size%2%nm") % (i / 4) % sPixelSize.Data());
    title = str(format("PXD Residuals Position In Plane Bias vs. RecoHit for Shape L, Layer %1%, Pixel Size %2% nm") %
                (i / 4) % sPixelSize.Data());
    m_ResidualsPosInPlBRHShapeL[i] = new TH2F(name.c_str(), title.c_str(), 2 * PlotsRange, -PlotsRange, PlotsRange, 2 * PlotsRange,
                                              -PlotsRange, PlotsRange);
    m_ResidualsPosInPlBRHShapeL[i]->GetXaxis()->SetTitle("Residual in u [um]");
    m_ResidualsPosInPlBRHShapeL[i]->GetYaxis()->SetTitle("Residual in v [um]");
    m_ResidualsPosInPlBRHShapeL[i]->GetZaxis()->SetTitle("counts");
    name = str(format("PXDResiduals_PosInPlBCl_ShapeL_L%1%_Size%2%nm") % (i / 4) % sPixelSize.Data());
    title = str(format("PXD Residuals Position In Plane Bias vs. Cluster for Shape L, Layer %1%, Pixel Size %2% nm") %
                (i / 4) % sPixelSize.Data());
    m_ResidualsPosInPlBClShapeL[i] = new TH2F(name.c_str(), title.c_str(), 2 * PlotsRange, -PlotsRange, PlotsRange, 2 * PlotsRange,
                                              -PlotsRange, PlotsRange);
    m_ResidualsPosInPlBClShapeL[i]->GetXaxis()->SetTitle("Residual in u [um]");
    m_ResidualsPosInPlBClShapeL[i]->GetYaxis()->SetTitle("Residual in v [um]");
    m_ResidualsPosInPlBClShapeL[i]->GetZaxis()->SetTitle("counts");
    name = str(format("PXDResiduals_TruePosInPlB_ShapeL_L%1%_Size%2%nm") % (i / 4) % sPixelSize.Data());
    title = str(format("PXD Residuals TrueHit vs Position In Plane Bias for Shape L, Layer %1%, Pixel Size %2% nm") %
                (i / 4) % sPixelSize.Data());
    m_ResidualsTruePosInPlBShapeL[i] = new TH2F(name.c_str(), title.c_str(), 2 * PlotsRange, -PlotsRange, PlotsRange, 2 * PlotsRange,
                                                -PlotsRange, PlotsRange);
    m_ResidualsTruePosInPlBShapeL[i]->GetXaxis()->SetTitle("Residual in u [um]");
    m_ResidualsTruePosInPlBShapeL[i]->GetYaxis()->SetTitle("Residual in v [um]");
    m_ResidualsTruePosInPlBShapeL[i]->GetZaxis()->SetTitle("counts");
    name = str(format("PXDResiduals_TrueRecoHit_ShapeL_L%1%_Size%2%nm") % (i / 4) % sPixelSize.Data());
    title = str(format("PXD Residuals TrueHit vs RecoHit for Shape L, Layer %1%, Pixel Size %2% nm") % (i / 4) % sPixelSize.Data());
    m_ResidualsTrueRHShapeL[i] = new TH2F(name.c_str(), title.c_str(), 2 * PlotsRange, -PlotsRange, PlotsRange, 2 * PlotsRange,
                                          -PlotsRange,
                                          PlotsRange);
    m_ResidualsTrueRHShapeL[i]->GetXaxis()->SetTitle("Residual in u [um]");
    m_ResidualsTrueRHShapeL[i]->GetYaxis()->SetTitle("Residual in v [um]");
    m_ResidualsTrueRHShapeL[i]->GetZaxis()->SetTitle("counts");
    name = str(format("PXDResiduals_TrueCluster_ShapeL_L%1%_Size%2%nm") % (i / 4) % sPixelSize.Data());
    title = str(format("PXD Residuals TrueHit vs Cluster for Shape L, Layer %1%, Pixel Size %2% nm") % (i / 4) % sPixelSize.Data());
    m_ResidualsTrueClShapeL[i] = new TH2F(name.c_str(), title.c_str(), 2 * PlotsRange, -PlotsRange, PlotsRange, 2 * PlotsRange,
                                          -PlotsRange,
                                          PlotsRange);
    m_ResidualsTrueClShapeL[i]->GetXaxis()->SetTitle("Residual in u [um]");
    m_ResidualsTrueClShapeL[i]->GetYaxis()->SetTitle("Residual in v [um]");
    m_ResidualsTrueClShapeL[i]->GetZaxis()->SetTitle("counts");

    DirPXDBasicEE->cd();
    name = str(format("PXDEERH_L%1%_Size%2%nm") % (i / 4) % sPixelSize.Data());
    title = str(format("PXD Error Estimation of RecoHit, Layer %1%, Pixel Size %2% nm") % (i / 4) % sPixelSize.Data());
    m_EERH[i] = new TH2F(name.c_str(), title.c_str(), 100, 0.0, 100.0, 100, 0.0, 100.0);
    m_EERH[i]->GetXaxis()->SetTitle("EE in u");
    m_EERH[i]->GetYaxis()->SetTitle("EE in v");
    m_EERH[i]->GetZaxis()->SetTitle("counts");
    name = str(format("PXDEECl_L%1%_Size%2%nm") % (i / 4) % sPixelSize.Data());
    title = str(format("PXD Error Estimation of Cluster, Layer %1%, Pixel Size %2% nm") % (i / 4) % sPixelSize.Data());
    m_EECl[i] = new TH2F(name.c_str(), title.c_str(), 100, 0.0, 100.0, 100, 0.0, 100.0);
    m_EECl[i]->GetXaxis()->SetTitle("EE in u");
    m_EECl[i]->GetYaxis()->SetTitle("EE in v");
    m_EECl[i]->GetZaxis()->SetTitle("counts");
    name = str(format("PXDEETrackUB_L%1%_Size%2%nm") % (i / 4) % sPixelSize.Data());
    title = str(format("PXD Error Estimation of Truck UnBias, Layer %1%, Pixel Size %2% nm") % (i / 4) % sPixelSize.Data());
    m_EETrackUB[i] = new TH2F(name.c_str(), title.c_str(), 100, 0.0, 100.0, 100, 0.0, 100.0);
    m_EETrackUB[i]->GetXaxis()->SetTitle("EE in u");
    m_EETrackUB[i]->GetYaxis()->SetTitle("EE in v");
    m_EETrackUB[i]->GetZaxis()->SetTitle("counts");
    name = str(format("PXDEETrackB_L%1%_Size%2%nm") % (i / 4) % sPixelSize.Data());
    title = str(format("PXD Error Estimation of Truck Bias, Layer %1%, Pixel Size %2% nm") % (i / 4) % sPixelSize.Data());
    m_EETrackB[i] = new TH2F(name.c_str(), title.c_str(), 100, 0.0, 100.0, 100, 0.0, 100.0);
    m_EETrackB[i]->GetXaxis()->SetTitle("EE in u");
    m_EETrackB[i]->GetYaxis()->SetTitle("EE in v");
    m_EETrackB[i]->GetZaxis()->SetTitle("counts");

    DirPXDBasicEEShapeL->cd();
    name = str(format("PXDEERHShapeL_L%1%_Size%2%nm") % (i / 4) % sPixelSize.Data());
    title = str(format("PXD Error Estimation of RecoHit for Shape L, Layer %1%, Pixel Size %2% nm") % (i / 4) % sPixelSize.Data());
    m_EERHShapeL[i] = new TH2F(name.c_str(), title.c_str(), 100, 0.0, 100.0, 100, 0.0, 100.0);
    m_EERHShapeL[i]->GetXaxis()->SetTitle("EE in u");
    m_EERHShapeL[i]->GetYaxis()->SetTitle("EE in v");
    m_EERHShapeL[i]->GetZaxis()->SetTitle("counts");
    name = str(format("PXDEEClShapeL_L%1%_Size%2%nm") % (i / 4) % sPixelSize.Data());
    title = str(format("PXD Error Estimation of Cluster for Shape L, Layer %1%, Pixel Size %2% nm") % (i / 4) % sPixelSize.Data());
    m_EEClShapeL[i] = new TH2F(name.c_str(), title.c_str(), 100, 0.0, 100.0, 100, 0.0, 100.0);
    m_EEClShapeL[i]->GetXaxis()->SetTitle("EE in u");
    m_EEClShapeL[i]->GetYaxis()->SetTitle("EE in v");
    m_EEClShapeL[i]->GetZaxis()->SetTitle("counts");
    name = str(format("PXDEETrackUBShapeL_L%1%_Size%2%nm") % (i / 4) % sPixelSize.Data());
    title = str(format("PXD Error Estimation of Truck UnBias for Shape L, Layer %1%, Pixel Size %2% nm") % (i / 4) % sPixelSize.Data());
    m_EETrackUBShapeL[i] = new TH2F(name.c_str(), title.c_str(), 100, 0.0, 100.0, 100, 0.0, 100.0);
    m_EETrackUBShapeL[i]->GetXaxis()->SetTitle("EE in u");
    m_EETrackUBShapeL[i]->GetYaxis()->SetTitle("EE in v");
    m_EETrackUBShapeL[i]->GetZaxis()->SetTitle("counts");
    name = str(format("PXDEETrackBShapeL_L%1%_Size%2%nm") % (i / 4) % sPixelSize.Data());
    title = str(format("PXD Error Estimation of Truck Bias for Shape L, Layer %1%, Pixel Size %2% nm") % (i / 4) % sPixelSize.Data());
    m_EETrackBShapeL[i] = new TH2F(name.c_str(), title.c_str(), 100, 0.0, 100.0, 100, 0.0, 100.0);
    m_EETrackBShapeL[i]->GetXaxis()->SetTitle("EE in u");
    m_EETrackBShapeL[i]->GetYaxis()->SetTitle("EE in v");
    m_EETrackBShapeL[i]->GetZaxis()->SetTitle("counts");

    DirPXDClShCorrs->cd();

    name = str(format("PXDReTrue_EECl_L%1%_Size%2%nm") % (i / 4) % sPixelSize.Data());
    title = str(format("PXD Ratio of Residual True to Cluster Error Estimation, Layer %1%, Pixel Size %2% nm") %
                (i / 4) % sPixelSize.Data());
    m_ReTrue_EECl[i] = new TH2F(name.c_str(), title.c_str(), 40, -10.0, 10.0, 40, -10.0, 10.0);
    m_ReTrue_EECl[i]->GetXaxis()->SetTitle("Res to EE in u");
    m_ReTrue_EECl[i]->GetYaxis()->SetTitle("Res to EE in v");
    m_ReTrue_EECl[i]->GetZaxis()->SetTitle("counts");
    name = str(format("PXDReTrue_EERH_L%1%_Size%2%nm") % (i / 4) % sPixelSize.Data());
    title = str(format("PXD Ratio of Residual True to RecoHit Error Estimation, Layer %1%, Pixel Size %2% nm") %
                (i / 4) % sPixelSize.Data());
    m_ReTrue_EERH[i] = new TH2F(name.c_str(), title.c_str(), 40, -10.0, 10.0, 40, -10.0, 10.0);
    m_ReTrue_EERH[i]->GetXaxis()->SetTitle("Res to EE in u");
    m_ReTrue_EERH[i]->GetYaxis()->SetTitle("Res to EE in v");
    m_ReTrue_EERH[i]->GetZaxis()->SetTitle("counts");
    name = str(format("PXDReUnBiasRes_EECl_L%1%_Size%2%nm") % (i / 4) % sPixelSize.Data());
    title = str(format("PXD Ratio of UnBias Residual to Cluster Error Estimation, Layer %1%, Pixel Size %2% nm") %
                (i / 4) % sPixelSize.Data());
    m_ReUnBiasRes_EECl[i] = new TH2F(name.c_str(), title.c_str(), 40, -10.0, 10.0, 40, -10.0, 10.0);
    m_ReUnBiasRes_EECl[i]->GetXaxis()->SetTitle("Res to EE in u");
    m_ReUnBiasRes_EECl[i]->GetYaxis()->SetTitle("Res to EE in v");
    m_ReUnBiasRes_EECl[i]->GetZaxis()->SetTitle("counts");
    name = str(format("PXDReUnBiasRes_EERH_L%1%_Size%2%nm") % (i / 4) % sPixelSize.Data());
    title = str(format("PXD Ratio of UnBias Residual to RecoHit Error Estimation, Layer %1%, Pixel Size %2% nm") %
                (i / 4) % sPixelSize.Data());
    m_ReUnBiasRes_EERH[i] = new TH2F(name.c_str(), title.c_str(), 40, -10.0, 10.0, 40, -10.0, 10.0);
    m_ReUnBiasRes_EERH[i]->GetXaxis()->SetTitle("Res to EE in u");
    m_ReUnBiasRes_EERH[i]->GetYaxis()->SetTitle("Res to EE in v");
    m_ReUnBiasRes_EERH[i]->GetZaxis()->SetTitle("counts");

    name = str(format("PXDReTrue_EEClShapeL_L%1%_Size%2%nm") % (i / 4) % sPixelSize.Data());
    title = str(format("PXD Ratio of Residual True to Cluster Error Estimation for Shape L, Layer %1%, Pixel Size %2% nm") %
                (i / 4) % sPixelSize.Data());
    m_ReTrue_EEClShapeL[i] = new TH2F(name.c_str(), title.c_str(), 40, -10.0, 10.0, 40, -10.0, 10.0);
    m_ReTrue_EEClShapeL[i]->GetXaxis()->SetTitle("Res to EE in u");
    m_ReTrue_EEClShapeL[i]->GetYaxis()->SetTitle("Res to EE in v");
    m_ReTrue_EEClShapeL[i]->GetZaxis()->SetTitle("counts");
    name = str(format("PXDReTrue_EERHShapeL_L%1%_Size%2%nm") % (i / 4) % sPixelSize.Data());
    title = str(format("PXD Ratio of Residual True to RecoHit Error Estimation for Shape L, Layer %1%, Pixel Size %2% nm") %
                (i / 4) % sPixelSize.Data());
    m_ReTrue_EERHShapeL[i] = new TH2F(name.c_str(), title.c_str(), 40, -10.0, 10.0, 40, -10.0, 10.0);
    m_ReTrue_EERHShapeL[i]->GetXaxis()->SetTitle("Res to EE in u");
    m_ReTrue_EERHShapeL[i]->GetYaxis()->SetTitle("Res to EE in v");
    m_ReTrue_EERHShapeL[i]->GetZaxis()->SetTitle("counts");
    name = str(format("PXDReUnBiasRes_EEClShapeL_L%1%_Size%2%nm") % (i / 4) % sPixelSize.Data());
    title = str(format("PXD Ratio of UnBias Residual to Cluster Error Estimation for Shape L, Layer %1%, Pixel Size %2% nm") %
                (i / 4) % sPixelSize.Data());
    m_ReUnBiasRes_EEClShapeL[i] = new TH2F(name.c_str(), title.c_str(), 40, -10.0, 10.0, 40, -10.0, 10.0);
    m_ReUnBiasRes_EEClShapeL[i]->GetXaxis()->SetTitle("Res to EE in u");
    m_ReUnBiasRes_EEClShapeL[i]->GetYaxis()->SetTitle("Res to EE in v");
    m_ReUnBiasRes_EEClShapeL[i]->GetZaxis()->SetTitle("counts");
    name = str(format("PXDReUnBiasRes_EERHShapeL_L%1%_Size%2%nm") % (i / 4) % sPixelSize.Data());
    title = str(format("PXD Ratio of UnBias Residual to RecoHit Error Estimation for Shape L, Layer %1%, Pixel Size %2% nm") %
                (i / 4) % sPixelSize.Data());
    m_ReUnBiasRes_EERHShapeL[i] = new TH2F(name.c_str(), title.c_str(), 40, -10.0, 10.0, 40, -10.0, 10.0);
    m_ReUnBiasRes_EERHShapeL[i]->GetXaxis()->SetTitle("Res to EE in u");
    m_ReUnBiasRes_EERHShapeL[i]->GetYaxis()->SetTitle("Res to EE in v");
    m_ReUnBiasRes_EERHShapeL[i]->GetZaxis()->SetTitle("counts");

  }

  oldDir->cd();

}

void PXDDQMClusterShapeModule::beginRun()
{
  for (int i = 0; i < 8; i++) {
    if (m_ResidualsPosInPlUBRH[i] != NULL) m_ResidualsPosInPlUBRH[i]->Reset();
    if (m_ResidualsPosInPlUBCl[i] != NULL) m_ResidualsPosInPlUBCl[i]->Reset();
    if (m_ResidualsTruePosInPlUB[i] != NULL) m_ResidualsTruePosInPlUB[i]->Reset();
    if (m_ResidualsPosInPlBRH[i] != NULL) m_ResidualsPosInPlBRH[i]->Reset();
    if (m_ResidualsPosInPlBCl[i] != NULL) m_ResidualsPosInPlBCl[i]->Reset();
    if (m_ResidualsTruePosInPlB[i] != NULL) m_ResidualsTruePosInPlB[i]->Reset();
    if (m_ResidualsTrueRH[i] != NULL) m_ResidualsTrueRH[i]->Reset();
    if (m_ResidualsTrueCl[i] != NULL) m_ResidualsTrueCl[i]->Reset();
    if (m_EERH[i] != NULL) m_EERH[i]->Reset();
    if (m_EECl[i] != NULL) m_EECl[i]->Reset();
    if (m_EETrackUB[i] != NULL) m_EETrackUB[i]->Reset();
    if (m_EETrackB[i] != NULL) m_EETrackB[i]->Reset();

    if (m_ResidualsPosInPlUBRHShapeL[i] != NULL) m_ResidualsPosInPlUBRHShapeL[i]->Reset();
    if (m_ResidualsPosInPlUBClShapeL[i] != NULL) m_ResidualsPosInPlUBClShapeL[i]->Reset();
    if (m_ResidualsTruePosInPlUBShapeL[i] != NULL) m_ResidualsTruePosInPlUBShapeL[i]->Reset();
    if (m_ResidualsPosInPlBRHShapeL[i] != NULL) m_ResidualsPosInPlBRHShapeL[i]->Reset();
    if (m_ResidualsPosInPlBClShapeL[i] != NULL) m_ResidualsPosInPlBClShapeL[i]->Reset();
    if (m_ResidualsTruePosInPlBShapeL[i] != NULL) m_ResidualsTruePosInPlBShapeL[i]->Reset();
    if (m_ResidualsTrueRHShapeL[i] != NULL) m_ResidualsTrueRHShapeL[i]->Reset();
    if (m_ResidualsTrueClShapeL[i] != NULL) m_ResidualsTrueClShapeL[i]->Reset();
    if (m_EERHShapeL[i] != NULL) m_EERHShapeL[i]->Reset();
    if (m_EEClShapeL[i] != NULL) m_EEClShapeL[i]->Reset();
    if (m_EETrackUBShapeL[i] != NULL) m_EETrackUBShapeL[i]->Reset();
    if (m_EETrackBShapeL[i] != NULL) m_EETrackBShapeL[i]->Reset();

    if (m_ReTrue_EECl[i] != NULL) m_ReTrue_EECl[i]->Reset();
    if (m_ReTrue_EERH[i] != NULL) m_ReTrue_EERH[i]->Reset();
    if (m_ReUnBiasRes_EECl[i] != NULL) m_ReUnBiasRes_EECl[i]->Reset();
    if (m_ReUnBiasRes_EERH[i] != NULL) m_ReUnBiasRes_EERH[i]->Reset();

    if (m_ReTrue_EEClShapeL[i] != NULL) m_ReTrue_EEClShapeL[i]->Reset();
    if (m_ReTrue_EERHShapeL[i] != NULL) m_ReTrue_EERHShapeL[i]->Reset();
    if (m_ReUnBiasRes_EEClShapeL[i] != NULL) m_ReUnBiasRes_EEClShapeL[i]->Reset();
    if (m_ReUnBiasRes_EERHShapeL[i] != NULL) m_ReUnBiasRes_EERHShapeL[i]->Reset();

  }
}


void PXDDQMClusterShapeModule::event()
{
//  StoreObjPtr<EventMetaData> emd;
  const StoreArray<PXDTrueHit> storePXDTrueHit(m_storePXDTrueHitsName);
  const StoreArray<PXDDigit> storePXDDigits(m_storePXDDigitsName);
  const StoreArray<PXDCluster> storePXDClusters(m_storePXDClustersName);
  const RelationArray relPXDClusterDigits(storePXDClusters, storePXDDigits, m_relPXDClusterDigitName);
  const StoreArray<PXDFrame> storeFrames(m_storeFramesName);
  StoreArray<RecoTrack> recotracks(m_storeRecoTrackName);
  const StoreArray<PXDRecoHit> pxdrecohit(m_storePXDRecoHitName);

  // If there are no digits, leave
  if (!storePXDDigits || !storePXDDigits.getEntries()) return;

  // Use GeoCache for sensor parameters and transforms
  VXD::GeoCache& geoCache = VXD::GeoCache::getInstance();
  TVector3 pos;
  TVector3 mom;
  TMatrixDSym cov;

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
      B2DEBUG(230, "See some PXD hit, layer" << sensorID.getLayerNumber());
      auto& genfitTrack = RecoTrackGenfitAccess::getGenfitTrack(recoTrack);
      auto cluster = recoHit->getCluster();
      float m_EEClusterU = cluster->getUSigma();
      float m_EEClusterV = cluster->getVSigma();

      auto state = recoTrack.getMeasuredStateOnPlaneFromRecoHit(recoHitInfo);
      // RecoHit obtaining, in first iteration should contain the same position information as cluster.
      auto measurementOnPlane = recoHit->constructMeasurementsOnPlane(state).at(0);
      state.getPosMomCov(pos, mom, cov);
      // Different way how to obtain tu and tv, not use finally:
      // const TVectorD& state5 = state.getState();
      // double TanAngU = state5[1];
      // double TanAngV = state5[2];
      auto mstate = measurementOnPlane->getState();
      float u_rh = 0;
      float v_rh = 0;
      u_rh = mstate(0);
      v_rh = mstate(1);
      // Different way how to obtain EERHu and EERHv, not use finally:
      // float m_EERecoHitU2 = sqrt(cov(0, 0));
      // float m_EERecoHitV2 = sqrt(cov(1, 1));
      // Convert pos and mom to local coordinates
      TVector3 localPos = info.pointToLocal(pos);
      float u = localPos.X();
      float v = localPos.Y();
      TVector3 localMom = info.vectorToLocal(mom);
      double tu = localMom.X() / localMom.Z();
      double tv = localMom.Y() / localMom.Z();
      // Different way how to obtain angle of tu and tv, not use finally:
      // m_phiTrack = TMath::ATan2(tu, 1.0);
      // m_thetaTrack = TMath::ATan2(tv, 1.0);
      // START do correcetion on cllluster repeat recohit steps:
      float m_EERecoHitU;
      float m_EERecoHitV;
      if (cluster && PXD::HitCorrector::getInstance().isInitialized()) {
        PXDCluster correctedCluster(*cluster);
        correctedCluster = PXD::HitCorrector::getInstance().correctCluster(correctedCluster, tu, tv);
        m_EERecoHitU = correctedCluster.getUSigma();
        m_EERecoHitV = correctedCluster.getVSigma();
        B2DEBUG(130, "Correction bias: \n   u: cl " << cluster->getU() << " clcor " << correctedCluster.getU() <<
                "\n   v: cl " << cluster->getV() << " clcor " << correctedCluster.getV() <<
                "Correction EE: \n   u: cl " << m_EEClusterU << " clcor " << m_EERecoHitU <<
                "\n   v: cl " << m_EEClusterV << " clcor " << m_EERecoHitU);
      } else {
        m_EERecoHitU = m_EEClusterU;
        m_EERecoHitV = m_EEClusterV;
      }
      // END do correcetion on cllluster repeat recohit steps

      // Biased position of track on the sensor and related information in local coordinates.
      bool biased = true;
      TVectorD resBias = genfitTrack.getPointWithMeasurement(iHit)->getFitterInfo()->getResidual(0, biased).getState();
      TMatrixDSym covBias = genfitTrack.getPointWithMeasurement(iHit)->getFitterInfo()->getResidual(0, biased).getCov();
      const TVectorD positBias = genfitTrack.getPointWithMeasurement(iHit)->getFitterInfo()->getFittedState(biased).getState();
      double u_bias = positBias(3);
      double v_bias = positBias(4);
      float m_ResidUPlaneRHBias = resBias.GetMatrixArray()[0];
      float m_ResidVPlaneRHBias = resBias.GetMatrixArray()[1];
      float m_ResidUPlaneClBias = m_ResidUPlaneRHBias + (cluster->getU() - u_rh);
      float m_ResidVPlaneClBias = m_ResidVPlaneRHBias + (cluster->getV() - v_rh);
      float m_SigmaUTrackBias = sqrt(covBias(0, 0));
      float m_SigmaVTrackBias = sqrt(covBias(1, 1));
      float m_SigmaUResBias = 0;
      if (cluster->getUSigma() > m_SigmaUTrackBias) {
        m_SigmaUResBias = sqrt(cluster->getUSigma() * cluster->getUSigma() - m_SigmaUTrackBias * m_SigmaUTrackBias);
      }
      float m_SigmaVResBias = 0;
      if (cluster->getVSigma() > m_SigmaVTrackBias) {
        m_SigmaVResBias = sqrt(cluster->getVSigma() * cluster->getVSigma() - m_SigmaVTrackBias * m_SigmaVTrackBias);
      }
      // m_ErrorEstimateCovarUVBias = covBias(0, 1);
      // Unbiased position of track on the sensor and related information in local coordinates.
      biased = false;
      TVectorD resUnBias = genfitTrack.getPointWithMeasurement(iHit)->getFitterInfo()->getResidual(0, biased).getState();
      TMatrixDSym covUnBias = genfitTrack.getPointWithMeasurement(iHit)->getFitterInfo()->getResidual(0, biased).getCov();
      const TVectorD positUnBias = genfitTrack.getPointWithMeasurement(iHit)->getFitterInfo()->getFittedState(biased).getState();
      double u_unbias = positUnBias(3);
      double v_unbias = positUnBias(4);
      float m_ResidUPlaneRHUnBias = resUnBias.GetMatrixArray()[0];
      float m_ResidVPlaneRHUnBias = resUnBias.GetMatrixArray()[1];
      float m_ResidUPlaneClUnBias = m_ResidUPlaneRHUnBias + (cluster->getU() - u_rh);
      float m_ResidVPlaneClUnBias = m_ResidVPlaneRHUnBias + (cluster->getV() - v_rh);
      float m_SigmaUTrackUnBias = sqrt(covUnBias(0, 0));
      float m_SigmaVTrackUnBias = sqrt(covUnBias(1, 1));
      float m_SigmaUResUnBias = sqrt(cluster->getUSigma() * cluster->getUSigma() + m_SigmaUTrackUnBias * m_SigmaUTrackUnBias);
      float m_SigmaVResUnBias = sqrt(cluster->getVSigma() * cluster->getVSigma() + m_SigmaVTrackUnBias * m_SigmaVTrackUnBias);
      // float m_ErrorEstimateCovarUVUnBias = covUnBias(0, 1);

      TString strDebugInfo = Form("  Point----->%i<------------\n", iHit);
      B2DEBUG(130, strDebugInfo.Data());
      strDebugInfo = Form("Bias[um]->R %5.1f %5.1f err: T %5.1f %5.1f = M %5.1f %5.1f - R %5.1f %5.1f (C%5.1f %5.1f C01 %5.1f)\n",
                          m_ResidUPlaneRHBias * 10000, m_ResidVPlaneClBias * 10000,
                          m_SigmaUTrackBias * 10000, m_SigmaVTrackBias * 10000,
                          cluster->getUSigma() * 10000, cluster->getVSigma() * 10000,
                          m_SigmaUResBias * 10000, m_SigmaVResBias * 10000,
                          sqrt(covBias(0, 0)) * 10000, sqrt(covBias(1, 1)) * 10000, covBias(0, 1) * 10000 * 10000);
      B2DEBUG(130, strDebugInfo.Data());
      strDebugInfo = Form("UnBi[um]->R %5.1f %5.1f err: T %5.1f %5.1f = R %5.1f %5.1f - M %5.1f %5.1f (C01: %5.1f)\n",
                          m_ResidUPlaneRHUnBias * 10000, m_ResidVPlaneRHUnBias * 10000,
                          m_SigmaUTrackUnBias * 10000, m_SigmaVTrackUnBias * 10000,
                          m_SigmaUResUnBias * 10000, m_SigmaVResUnBias * 10000,
                          cluster->getUSigma() * 10000, cluster->getVSigma() * 10000, covUnBias(0, 1) * 10000 * 10000);
      B2DEBUG(130, strDebugInfo.Data());
      iHit++;

      float u_cl = cluster->getU();
      float v_cl = cluster->getV();
      int iPixKind = info.getPixelKind(sensorID, cluster->getV());
      int iShape = cluster->getShape();
      // And finally TrueHit:
      float u_th = 0;
      float v_th = 0;
      float m_ResidUTrueRH = 0;
      float m_ResidVTrueRH = 0;
      float m_ResidUTrueCl = 0;
      float m_ResidVTrueCl = 0;
      float m_ResidUTruePlaneBias = 0;
      float m_ResidVTruePlaneBias = 0;
      float m_ResidUTruePlaneUnBias = 0;
      float m_ResidVTruePlaneUnBias = 0;
      if (m_UseRealData == 0) {  // there exist simulation, so use true points
        PXDTrueHit* truehit = cluster->getRelated<PXDTrueHit>();
        u_th = truehit->getU();
        v_th = truehit->getV();

        // For tuning time to check only one case:
        // double phiTrue = TMath::ATan2(tu, 1.0);
        // double thetaTrue = TMath::ATan2(tv, 1.0);
        // int indphi2 = m_anglesU / 2 + (int)(phiTrue * m_anglesU / TMath::Pi());
        // int indtheta2 = m_anglesV / 2 + (int)(thetaTrue * m_anglesV / TMath::Pi());
        // phiTrue = TMath::ATan2(tu, 1.0);  // use in case tu is Tangent of angle
        // thetaTrue = TMath::ATan2(tv, 1.0);  // use in case tv is Tangent of angle
        // double uNormPos = (phiTrue + M_PI_2) / (M_PI / m_anglesU);
        // double vNormPos = (thetaTrue + M_PI_2) / (M_PI / m_anglesV);
        // int indphi = (int)uNormPos;    // identify index
        // int indtheta = (int)vNormPos;  // identify index
        // if (!((iShape == 3) && (iPixKind == 6) && (indphi == 9) && (indtheta == 12)))
        //   continue;
        // printf("----> u: %f %f (%f) %f v: %f %f (%f) %f\n",
        //        m_EEClusterU, m_EERecoHitU, m_EEClusterU / m_EERecoHitU, m_EERecoHitU / m_EEClusterU,
        //        m_EEClusterV, m_EERecoHitV, m_EEClusterV / m_EERecoHitV, m_EERecoHitV / m_EEClusterV);

        // Different way how to obtain tu and tv, not use finally:
        // mom = truehit->getMomentum();  // information in local coordinates
        // m_phiTrue = TMath::ATan2(mom.X(), mom.Z());
        // m_thetaTrue = TMath::ATan2(mom.Y(), mom.Z());
        m_ResidUTruePlaneBias = u_th - u_bias;
        m_ResidVTruePlaneBias = v_th - v_bias;
        m_ResidUTruePlaneUnBias = u_th - u_unbias;
        m_ResidVTruePlaneUnBias = v_th - v_unbias;
        m_ResidUTrueRH = u_th - u_rh;
        m_ResidVTrueRH = v_th - v_rh;
        m_ResidUTrueCl = u_th - u_cl;
        m_ResidVTrueCl = v_th - v_cl;

        // m_InPixUReco = (cluster->getU() - info.getUCellPosition(info.getUCellID(cluster->getU()))) / info.getUPitch(cluster->getU());
        // m_InPixVReco = (cluster->getV() - info.getVCellPosition(info.getVCellID(cluster->getV()))) / info.getVPitch(cluster->getV());

        // m_InPixUTrue = (truehit->getU() - info.getUCellPosition(info.getUCellID(truehit->getU()))) / info.getUPitch(truehit->getU());
        // m_InPixVTrue = (truehit->getV() - info.getVCellPosition(info.getVCellID(truehit->getV()))) / info.getVPitch(truehit->getV());
        B2DEBUG(130, "sensor " << sensorID.getLayerNumber() <<
                "_" << sensorID.getLadderNumber() << "_" << sensorID.getSensorNumber() <<
                ", PixKind " << iPixKind <<
                ", u/u_rh/u_cl/u_th " << u << "/" << u_rh << "/" << u_cl << "/" << u_th <<
                ", v/v_rh/v_cl/v_th " << v << "/" << v_rh << "/" << v_cl << "/" << v_th);
      }

      m_ResidUTrueRH *= Unit::convertValueToUnit(1.0, "um");
      m_ResidVTrueRH *= Unit::convertValueToUnit(1.0, "um");
      m_ResidUTrueCl *= Unit::convertValueToUnit(1.0, "um");
      m_ResidVTrueCl *= Unit::convertValueToUnit(1.0, "um");
      m_ResidUTruePlaneUnBias *= Unit::convertValueToUnit(1.0, "um");
      m_ResidVTruePlaneUnBias *= Unit::convertValueToUnit(1.0, "um");
      m_ResidUTruePlaneBias *= Unit::convertValueToUnit(1.0, "um");
      m_ResidVTruePlaneBias *= Unit::convertValueToUnit(1.0, "um");
      m_ResidUPlaneRHUnBias *= Unit::convertValueToUnit(1.0, "um");
      m_ResidVPlaneRHUnBias *= Unit::convertValueToUnit(1.0, "um");
      m_ResidUPlaneClUnBias *= Unit::convertValueToUnit(1.0, "um");
      m_ResidVPlaneClUnBias *= Unit::convertValueToUnit(1.0, "um");
      m_ResidUPlaneRHBias *= Unit::convertValueToUnit(1.0, "um");
      m_ResidVPlaneRHBias *= Unit::convertValueToUnit(1.0, "um");
      m_ResidUPlaneClBias *= Unit::convertValueToUnit(1.0, "um");
      m_ResidVPlaneClBias *= Unit::convertValueToUnit(1.0, "um");
      m_EEClusterU *= Unit::convertValueToUnit(1.0, "um");
      m_EEClusterV *= Unit::convertValueToUnit(1.0, "um");
      m_EERecoHitU *= Unit::convertValueToUnit(1.0, "um");
      m_EERecoHitV *= Unit::convertValueToUnit(1.0, "um");
      m_SigmaUTrackUnBias *= Unit::convertValueToUnit(1.0, "um");
      m_SigmaVTrackUnBias *= Unit::convertValueToUnit(1.0, "um");
      m_SigmaUTrackBias *= Unit::convertValueToUnit(1.0, "um");
      m_SigmaVTrackBias *= Unit::convertValueToUnit(1.0, "um");

      if (m_ReTrue_EECl[iPixKind] != NULL) m_ReTrue_EECl[iPixKind]->Fill(m_ResidUTrueCl / m_EEClusterU, m_ResidVTrueCl / m_EEClusterV);
      if (m_ReTrue_EERH[iPixKind] != NULL) m_ReTrue_EERH[iPixKind]->Fill(m_ResidUTrueRH / m_EERecoHitU, m_ResidVTrueRH / m_EERecoHitV);
      if (m_ReUnBiasRes_EECl[iPixKind] != NULL) m_ReUnBiasRes_EECl[iPixKind]->Fill(m_ResidUPlaneClUnBias / m_EEClusterU,
            m_ResidVPlaneClUnBias / m_EEClusterV);
      if (m_ReUnBiasRes_EERH[iPixKind] != NULL) m_ReUnBiasRes_EERH[iPixKind]->Fill(m_ResidUPlaneRHUnBias / m_EERecoHitU,
            m_ResidVPlaneRHUnBias / m_EERecoHitV);

      if (m_ResidualsPosInPlUBRH[iPixKind] != NULL) m_ResidualsPosInPlUBRH[iPixKind]->Fill(m_ResidUPlaneRHUnBias, m_ResidVPlaneRHUnBias);
      if (m_ResidualsPosInPlUBCl[iPixKind] != NULL) m_ResidualsPosInPlUBCl[iPixKind]->Fill(m_ResidUPlaneClUnBias, m_ResidVPlaneClUnBias);
      if (m_ResidualsTruePosInPlUB[iPixKind] != NULL) m_ResidualsTruePosInPlUB[iPixKind]->Fill(m_ResidUTruePlaneUnBias,
            m_ResidVTruePlaneUnBias);
      if (m_ResidualsPosInPlBRH[iPixKind] != NULL) m_ResidualsPosInPlBRH[iPixKind]->Fill(m_ResidUPlaneRHBias, m_ResidVPlaneRHBias);
      if (m_ResidualsPosInPlBCl[iPixKind] != NULL) m_ResidualsPosInPlBCl[iPixKind]->Fill(m_ResidUPlaneClBias, m_ResidVPlaneClBias);
      if (m_ResidualsTruePosInPlB[iPixKind] != NULL) m_ResidualsTruePosInPlB[iPixKind]->Fill(m_ResidUTruePlaneBias,
            m_ResidVTruePlaneBias);
      if (m_ResidualsTrueRH[iPixKind] != NULL) m_ResidualsTrueRH[iPixKind]->Fill(m_ResidUTrueRH, m_ResidVTrueRH);
      if (m_ResidualsTrueCl[iPixKind] != NULL) m_ResidualsTrueCl[iPixKind]->Fill(m_ResidUTrueCl, m_ResidVTrueCl);
      if (m_EERH[iPixKind] != NULL) m_EERH[iPixKind]->Fill(m_EERecoHitU, m_EERecoHitV);
      if (m_EECl[iPixKind] != NULL) m_EECl[iPixKind]->Fill(m_EEClusterU, m_EEClusterV);
      if (m_EETrackUB[iPixKind] != NULL) m_EETrackUB[iPixKind]->Fill(m_SigmaUTrackUnBias, m_SigmaVTrackUnBias);
      if (m_EETrackB[iPixKind] != NULL) m_EETrackB[iPixKind]->Fill(m_SigmaUTrackBias, m_SigmaVTrackBias);


      if ((iShape == (short int)pxdClusterShapeType::shape_3_L) ||
          (iShape == (short int)pxdClusterShapeType::shape_3_L_mirr_u) ||
          (iShape == (short int)pxdClusterShapeType::shape_3_L_mirr_v) ||
          (iShape == (short int)pxdClusterShapeType::shape_3_L_mirr_uv)) {

        if (m_ReTrue_EEClShapeL[iPixKind] != NULL) m_ReTrue_EEClShapeL[iPixKind]->Fill(m_ResidUTrueCl / m_EEClusterU,
              m_ResidVTrueCl / m_EEClusterV);
        if (m_ReTrue_EERHShapeL[iPixKind] != NULL) m_ReTrue_EERHShapeL[iPixKind]->Fill(m_ResidUTrueRH / m_EERecoHitU,
              m_ResidVTrueRH / m_EERecoHitV);
        if (m_ReUnBiasRes_EEClShapeL[iPixKind] != NULL) m_ReUnBiasRes_EEClShapeL[iPixKind]->Fill(m_ResidUPlaneClUnBias / m_EEClusterU,
              m_ResidVPlaneClUnBias / m_EEClusterV);
        if (m_ReUnBiasRes_EERHShapeL[iPixKind] != NULL) m_ReUnBiasRes_EERHShapeL[iPixKind]->Fill(m_ResidUPlaneRHUnBias / m_EERecoHitU,
              m_ResidVPlaneRHUnBias / m_EERecoHitV);

        if (m_ResidualsPosInPlUBRHShapeL[iPixKind] != NULL) m_ResidualsPosInPlUBRHShapeL[iPixKind]->Fill(m_ResidUPlaneRHUnBias,
              m_ResidVPlaneRHUnBias);
        if (m_ResidualsPosInPlUBClShapeL[iPixKind] != NULL) m_ResidualsPosInPlUBClShapeL[iPixKind]->Fill(m_ResidUPlaneClUnBias,
              m_ResidVPlaneClUnBias);
        if (m_ResidualsTruePosInPlUBShapeL[iPixKind] != NULL) m_ResidualsTruePosInPlUBShapeL[iPixKind]->Fill(m_ResidUTruePlaneBias,
              m_ResidVTruePlaneBias);
        if (m_ResidualsPosInPlBRHShapeL[iPixKind] != NULL) m_ResidualsPosInPlBRHShapeL[iPixKind]->Fill(m_ResidUPlaneRHBias,
              m_ResidVPlaneRHBias);
        if (m_ResidualsPosInPlBClShapeL[iPixKind] != NULL) m_ResidualsPosInPlBClShapeL[iPixKind]->Fill(m_ResidUPlaneClBias,
              m_ResidVPlaneClBias);
        if (m_ResidualsTruePosInPlBShapeL[iPixKind] != NULL) m_ResidualsTruePosInPlBShapeL[iPixKind]->Fill(m_ResidUTruePlaneUnBias,
              m_ResidVTruePlaneUnBias);
        if (m_ResidualsTrueRHShapeL[iPixKind] != NULL) m_ResidualsTrueRHShapeL[iPixKind]->Fill(m_ResidUTrueRH, m_ResidVTrueRH);
        if (m_ResidualsTrueClShapeL[iPixKind] != NULL) m_ResidualsTrueClShapeL[iPixKind]->Fill(m_ResidUTrueCl, m_ResidVTrueCl);

        if (m_EERHShapeL[iPixKind] != NULL) m_EERHShapeL[iPixKind]->Fill(m_EERecoHitU, m_EERecoHitV);
        if (m_EEClShapeL[iPixKind] != NULL) m_EEClShapeL[iPixKind]->Fill(m_EEClusterU, m_EEClusterV);
        if (m_EETrackUBShapeL[iPixKind] != NULL) m_EETrackUBShapeL[iPixKind]->Fill(m_SigmaUTrackUnBias, m_SigmaVTrackUnBias);
        if (m_EETrackBShapeL[iPixKind] != NULL) m_EETrackBShapeL[iPixKind]->Fill(m_SigmaUTrackBias, m_SigmaVTrackBias);
      }
    }
  }
}


void PXDDQMClusterShapeModule::endRun()
{
}


void PXDDQMClusterShapeModule::terminate()
{
}
