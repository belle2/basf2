/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Peter Kodys                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <pxd/calibration/PXDClusterShapeCalibrationAlgorithm.h>

#include <TH1F.h>
#include <TH2F.h>
#include <TMath.h>
#include <TRandom.h>
#include <TFile.h>
#include <TEventList.h>
#include <TEntryList.h>

using namespace std;
using namespace Belle2;

PXDClusterShapeCalibrationAlgorithm::PXDClusterShapeCalibrationAlgorithm() : CalibrationAlgorithm("pxdClusterShapeCalibration")
{
  setDescription("Calibration of position/estimated error corrections based on cluster shape in PXD");
}


Belle2::CalibrationAlgorithm::EResult PXDClusterShapeCalibrationAlgorithm::calibrate()
{
// cluster will be check if:
//   is with non-corrected cluster shape, otheway remove from corrections (to prevent multiple-corrections)
//   is not close borders, otheway remove from corrections
//   for givig kind of pixel type, u and v angle direction:
//     is in in-pixel hitmap for giving cluster shape and giving angle, otheway if hitmap is fully empty:
//        do those steps for full range simulation in-pixel hitmap
//        if still not in in-pixel hitmap - remove from corrections
//     look for bias and estimated error to angle in 3 most closes points in calibration tables
//     apply correction with weigh dependent of distances to table points
// Tables for every pixel type (8 types for PXD):
//   2 sets of tables: - for selection of angles and shapes for real data
//                     - for full range of angles and shapes from simulations
//   in every set of tables are:
//       - in-pixel 2D hitmaps for angles and shapes
//       - u+v corrections of bias for angles and shapes
//       - u+v error estimations for angles and shapes
// so we expect u+v directions, 8 kinds of pixels, 18 x 18 angles, 15 shapes = 77760 cases
//   for Bias correction
//   for Error estimation (EstimError/Residual)
// so we expect 8 kinds of pixels, 18 x 18 angles, 15 shapes = 38880 cases
//   for in-pixel positions 2D maps, each map has 9x9 bins with binary value (0/1)
// For ~1000 points per histogram we expect 25 millions events (1 event = 2x sensors, u+v coordinates)
// We expect set of histograms mostly empty so no need any correction for giving parameters
// Finaly we store for 2 sets of tables:
//   - for Bias correction:      77760 short values
//   - for Error estimation:     77760 short values
//   - for in-pixel positions: 3149280 binary values
//
// Storring in database will be in TVectorT format
// Using will be on boost/multi_array format in unordered map and hash table
//
// There are following combinations:
//   1) Real data:              phiTrack, thetaTrack, InPixU, InPixV, ResidUTrack, ResidVTrack, SigmaU, SigmaV, // SigmaUTrack, SigmaVTrack
//   2) Simulation from tracks: phiTrack, thetaTrack, InPixU, InPixV, ResidUTrack, ResidVTrack, SigmaU, SigmaV, // SigmaUTrack, SigmaVTrack
//   3) Simulation from TrueP.: phiTrue, thetaTrue, InPixUTrue, InPixVTrue, ResidUTrue, ResidVTrue, SigmaU, SigmaV
//   4) SpecialSimulation:      phiTrue, thetaTrue, InPixUTrue, InPixVTrue, ResidUTrue, ResidVTrue, SigmaU, SigmaV
//
//  1) m_UseRealData == kTRUE --> m_UseTracks == kTRUE
//  2) m_UseTracks == kTRUE
//  3) m_UseTracks == kFALSE
//  4) m_CalibrationKind == 2 --> m_UseTracks == kFALSE
// so the combination will be use: m_UseTracks == kTRUE / kFALSE
//
// For calibration need to have: InPix, Residual, Sigma
//
// There is general question if we use in-pixel information for selections:
//    - from true prack there is significant and useful information
//    - from tracking information has spread in level sigma 9 microns (preliminary)
//   in-pixel filter will be prepared but not use.
//
//
// ***** DQM AND EXPERT HISTOGRAMS:  *****
//
// All histograms are optional only for request.
//
// Histograms: m_shapes * m_pixelkinds = 15 x 8 = 120 u+v = 240 (1D)
// Binning: angleU x angleV (18 x 18)
// Monitoring values:      BiasCorrection U + V (residual mean)
//                         ResidualRMS U + V (residual RMS)
//                         BiasCorrectionError U + V (error of residual mean)
//                         ErrorEstimation U + V (RMS of (residual / estimated error))
//                         nClusters, nClusterFraction
//
// Special histograms case 2):     2D SigmaUTrack + SigmaVTrack
//                                 2D phiTrack - phiTrue + thetaTrack - thetaTrue
//                                 2D InPixU - InPixUTrue + InPixV - InPixVTrue
//                                 2D ResidUTrack - ResidUTrue + ResidVTrack - ResidVTrue
//
//


  TString name_Case;
  TString name_SourceTree;
  TString name_OutFileCalibrations;
  name_Case = Form("_RealData%i_Track%i_Calib%i_Pixel%i",
                   (int)m_UseRealData, (int)m_UseTracks, m_CalibrationKind, m_PixelKind);
  name_SourceTree = Form("pxdCal");
  name_OutFileCalibrations = Form("pxdClShCal%s.root", name_Case.Data());
  int SummariesInfo[20];
  for (int i = 0; i < 20; i++) SummariesInfo[i] = 0;
  //int SummariesInfoPK[m_pixelkinds];
  std::vector<int> SummariesInfoPK(m_pixelkinds);
  for (int i = 0; i < m_pixelkinds; i++) SummariesInfoPK[i] = 0;
  //int SummariesInfoSh[m_shapes];
  std::vector<int> SummariesInfoSh(m_shapes);
  for (int i = 0; i < m_shapes; i++) SummariesInfoSh[i] = 0;
  //int SummariesInfoAng[m_anglesU * m_anglesV];
  std::vector<int> SummariesInfoAng(m_anglesU * m_anglesV);
  for (int i = 0; i < m_anglesU * m_anglesV; i++) SummariesInfoAng[i] = 0;

  // START - section for variables for DQM and expert histograms
  TString name_OutFileDQM;
  TString name_OutDoExpertHistograms;
  TFile* fDQM = NULL;
  TFile* fExpertHistograms = NULL;
  TString DirPixelKind;
  TString DirShape;
  TH1F* m_histSummariesInfo = NULL;
  TH1F* m_histSummariesInfoSh = NULL;
  TH1F* m_histSummariesInfoPK = NULL;
  TH2F* m_histSummariesInfoAng = NULL;
  TH2F** m_histBiasCorrectionU = NULL;
  TH2F** m_histBiasCorrectionV = NULL;
  TH2F** m_histResidualRMSU = NULL;
  TH2F** m_histResidualRMSV = NULL;
  TH2F** m_histBiasCorrectionErrorU = NULL;
  TH2F** m_histBiasCorrectionErrorV = NULL;
  TH2F** m_histErrorEstimationU = NULL;
  TH2F** m_histErrorEstimationV = NULL;
  TH2F** m_histnClusters = NULL;
  TH2F** m_histnClusterFraction = NULL;

  TH1F** m_histSignal = NULL;
  TH1F** m_histSeed = NULL;
  TH1F** m_histTrackSigmaU = NULL;
  TH1F** m_histTrackSigmaV = NULL;
  TH2F** m_histTrackSigmaUV = NULL;

  m_histTrackSigmaUV = (TH2F**) new TH2F*[m_anglesU * m_anglesV + 1];
  m_histTrackSigmaU = (TH1F**) new TH1F*[m_anglesU * m_anglesV + 1];
  m_histTrackSigmaV = (TH1F**) new TH1F*[m_anglesU * m_anglesV + 1];
  m_histSignal = (TH1F**) new TH1F*[m_shapes + 1];
  m_histSeed = (TH1F**) new TH1F*[m_shapes + 1];
  for (int i_angle = 0; i_angle < m_anglesU * m_anglesV + 1; i_angle++) {
    m_histTrackSigmaU[i_angle] = NULL;
    m_histTrackSigmaV[i_angle] = NULL;
    m_histTrackSigmaUV[i_angle] = NULL;
  }
  for (int i_shape = 0; i_shape < m_shapes + 1; i_shape++) {
    m_histSignal[i_shape] = NULL;
    m_histSeed[i_shape] = NULL;
  }

  m_histBiasCorrectionU = (TH2F**) new TH2F*[m_pixelkinds * m_shapes + 1];
  m_histBiasCorrectionV = (TH2F**) new TH2F*[m_pixelkinds * m_shapes + 1];
  m_histResidualRMSU = (TH2F**) new TH2F*[m_pixelkinds * m_shapes + 1];
  m_histResidualRMSV = (TH2F**) new TH2F*[m_pixelkinds * m_shapes + 1];
  m_histBiasCorrectionErrorU = (TH2F**) new TH2F*[m_pixelkinds * m_shapes + 1];
  m_histBiasCorrectionErrorV = (TH2F**) new TH2F*[m_pixelkinds * m_shapes + 1];
  m_histErrorEstimationU = (TH2F**) new TH2F*[m_pixelkinds * m_shapes + 1];
  m_histErrorEstimationV = (TH2F**) new TH2F*[m_pixelkinds * m_shapes + 1];
  m_histnClusters = (TH2F**) new TH2F*[m_pixelkinds * m_shapes + 1];
  m_histnClusterFraction = (TH2F**) new TH2F*[m_pixelkinds * m_shapes + 1];
  for (int i_pk = 0; i_pk < m_pixelkinds; i_pk++) {
    for (int i_shape = 0; i_shape < m_shapes + 1; i_shape++) {
      m_histBiasCorrectionU[i_pk * m_shapes + i_shape] = NULL;
      m_histBiasCorrectionV[i_pk * m_shapes + i_shape] = NULL;
      m_histResidualRMSU[i_pk * m_shapes + i_shape] = NULL;
      m_histResidualRMSV[i_pk * m_shapes + i_shape] = NULL;
      m_histBiasCorrectionErrorU[i_pk * m_shapes + i_shape] = NULL;
      m_histBiasCorrectionErrorV[i_pk * m_shapes + i_shape] = NULL;
      m_histErrorEstimationU[i_pk * m_shapes + i_shape] = NULL;
      m_histErrorEstimationV[i_pk * m_shapes + i_shape] = NULL;
      m_histnClusters[i_pk * m_shapes + i_shape] = NULL;
      m_histnClusterFraction[i_pk * m_shapes + i_shape] = NULL;
    }
  }

  name_OutFileDQM = Form("pxdClShCalDQM%s.root", name_Case.Data());
  name_OutDoExpertHistograms = Form("pxdClShCalHistos%s.root", name_Case.Data());
  // END - section for variables for DQM and expert histograms


  B2INFO("Name of files: callibration: " << name_OutFileCalibrations.Data() << ", DQM: " << name_OutFileDQM.Data() <<
         ", ExpertHistograms: " << name_OutDoExpertHistograms.Data());

  if (m_DoExpertHistograms) {
    fDQM = new TFile(name_OutFileDQM.Data(), "recreate");
    fDQM->mkdir("NoSorting");
    fDQM->cd("NoSorting");

    m_histSummariesInfo = new TH1F("SummariesInfo", "Summaries Info", 20, 0, 20);
    m_histSummariesInfoSh = new TH1F("SummariesInfoSh", "Summaries Info Over Shapes", m_shapes, 1, m_shapes + 1);
    m_histSummariesInfoPK = new TH1F("SummariesInfoPK", "Summaries Info Over Pixel Kinds", m_pixelkinds, 0, m_pixelkinds);
    m_histSummariesInfoAng = new TH2F("SummariesInfoAng", "Summaries Info Over Angle Distribution", m_anglesU, -m_anglesU / 2,
                                      m_anglesU / 2, m_anglesV, -m_anglesV / 2, m_anglesV / 2);

    m_histSignal[m_shapes] = new TH1F("Signal", "Signal", 100, 0, 100);
    m_histSeed[m_shapes] = new TH1F("Seed", "Seed", 100, 0, 100);
    m_histTrackSigmaU[m_anglesU * m_anglesV] = new TH1F("TrackSigmaU", "Track Sigma U", 200, 0, 100);
    m_histTrackSigmaV[m_anglesU * m_anglesV] = new TH1F("TrackSigmaV", "Track Sigma V", 200, 0, 100);
    m_histTrackSigmaUV[m_anglesU * m_anglesV] = new TH2F("TrackSigmaUV", "Track Sigma UV", 100, 0, 100, 100, 0, 100);

    fDQM->mkdir("TrackSigma");
    fDQM->cd("TrackSigma");
    for (int i_angleU = 0; i_angleU < m_anglesU; i_angleU++) {
      for (int i_angleV = 0; i_angleV < m_anglesV; i_angleV++) {
        TString HistoName = Form("TrackSigmaU_AngU%02i_AngV%02i", i_angleU, i_angleV);
        TString HistoTitle = Form("Track Sigma U, angle U %02i, angle V %02i", i_angleU, i_angleV);
        m_histTrackSigmaU[i_angleU * m_anglesV + i_angleV] = new TH1F(HistoName.Data(), HistoTitle.Data(), 200, 0, 100);
        HistoName = Form("TrackSigmaV_AngU%02i_AngV%02i", i_angleU, i_angleV);
        HistoTitle = Form("Track Sigma V, angle U %02i, angle V %02i", i_angleU, i_angleV);
        m_histTrackSigmaV[i_angleU * m_anglesV + i_angleV] = new TH1F(HistoName.Data(), HistoTitle.Data(), 200, 0, 100);
        HistoName = Form("TrackSigmaUV_AngU%02i_AngV%02i", i_angleU, i_angleV);
        HistoTitle = Form("Track Sigma UV, angle U %02i, angle V %02i", i_angleU, i_angleV);
        m_histTrackSigmaUV[i_angleU * m_anglesV + i_angleV] = new TH2F(HistoName.Data(), HistoTitle.Data(), 100, 0, 100, 100, 0, 100);
      }
    }

    fDQM->mkdir("Energies");
    fDQM->cd("Energies");
    for (int i_shape = 0; i_shape < m_shapes; i_shape++) {
      TString HistoName = Form("Signal_Sh%02i", i_shape + 1);
      TString HistoTitle = Form("Signal, shape %02i", i_shape + 1);
      m_histSignal[i_shape] = new TH1F(HistoName.Data(), HistoTitle.Data(), 100, 0, 100);
      HistoName = Form("Seed_Sh%02i", i_shape + 1);
      HistoTitle = Form("Seed, shape %02i", i_shape + 1);
      m_histSeed[i_shape] = new TH1F(HistoName.Data(), HistoTitle.Data(), 100, 0, 100);
    }

    fExpertHistograms = new TFile(name_OutDoExpertHistograms.Data(), "recreate");
    fExpertHistograms->mkdir("NoSorting");

    for (int i_pk = 0; i_pk < m_pixelkinds; i_pk++) {
      DirPixelKind = Form("PixelKind_%01i_Layer_%i_Sensor_%i_Size_%i", i_pk, (int)((i_pk % 4) / 2) + 1, (int)(i_pk / 4) + 1, i_pk % 2);
      fDQM->mkdir(DirPixelKind.Data());
      fExpertHistograms->mkdir(DirPixelKind.Data());
      for (int i_shape = 0; i_shape < m_shapes; i_shape++) {
        DirShape = Form("%s/Shape_%02i_%s", DirPixelKind.Data(), i_shape + 1,
                        Belle2::PXD::PXDClusterShape::pxdClusterShapeDescription[(Belle2::PXD::pxdClusterShapeType)(i_shape + 1)].c_str());
        DirShape.ReplaceAll(":", "");
        DirShape.ReplaceAll(" ", "_");
        fDQM->mkdir(DirShape.Data());
        fDQM->cd(DirShape.Data());

        // Bias Correction
        TString HistoName = Form("BiasCorrectionU_PK%01i_Sh%02i", i_pk, i_shape + 1);
        TString HistoTitle = Form("Bias Correction U, pixel kind %01i, shape %02i", i_pk, i_shape + 1);
        m_histBiasCorrectionU[i_pk * m_shapes + i_shape] = new TH2F(HistoName.Data(), HistoTitle.Data(),
                                                                    m_anglesU, -TMath::Pi() * 0.5 / Unit::deg, TMath::Pi() * 0.5 / Unit::deg,
                                                                    m_anglesV, -TMath::Pi() * 0.5 / Unit::deg, TMath::Pi() * 0.5 / Unit::deg);
        m_histBiasCorrectionU[i_pk * m_shapes + i_shape]->GetXaxis()->SetTitle("Angle in u [deg]");
        m_histBiasCorrectionU[i_pk * m_shapes + i_shape]->GetYaxis()->SetTitle("Angle in v [deg]");
        HistoName = Form("BiasCorrectionV_PK%01i_Sh%02i", i_pk, i_shape + 1);
        HistoTitle = Form("Bias Correction V, pixel kind %01i, shape %02i", i_pk, i_shape + 1);
        m_histBiasCorrectionV[i_pk * m_shapes + i_shape] = new TH2F(HistoName.Data(), HistoTitle.Data(),
                                                                    m_anglesU, -TMath::Pi() * 0.5 / Unit::deg, TMath::Pi() * 0.5 / Unit::deg,
                                                                    m_anglesV, -TMath::Pi() * 0.5 / Unit::deg, TMath::Pi() * 0.5 / Unit::deg);
        m_histBiasCorrectionV[i_pk * m_shapes + i_shape]->GetXaxis()->SetTitle("Angle in u [deg]");
        m_histBiasCorrectionV[i_pk * m_shapes + i_shape]->GetYaxis()->SetTitle("Angle in v [deg]");

        // Residual RMS
        HistoName = Form("ResidualRMSU_PK%01i_Sh%02i", i_pk, i_shape + 1);
        HistoTitle = Form("Residual RMS U, pixel kind %01i, shape %02i", i_pk, i_shape + 1);
        m_histResidualRMSU[i_pk * m_shapes + i_shape] = new TH2F(HistoName.Data(), HistoTitle.Data(),
                                                                 m_anglesU, -TMath::Pi() * 0.5 / Unit::deg, TMath::Pi() * 0.5 / Unit::deg,
                                                                 m_anglesV, -TMath::Pi() * 0.5 / Unit::deg, TMath::Pi() * 0.5 / Unit::deg);
        m_histResidualRMSU[i_pk * m_shapes + i_shape]->GetXaxis()->SetTitle("Angle in u [deg]");
        m_histResidualRMSU[i_pk * m_shapes + i_shape]->GetYaxis()->SetTitle("Angle in v [deg]");
        HistoName = Form("ResidualRMSV_PK%01i_Sh%02i", i_pk, i_shape + 1);
        HistoTitle = Form("Residual RMS V, pixel kind %01i, shape %02i", i_pk, i_shape + 1);
        m_histResidualRMSV[i_pk * m_shapes + i_shape] = new TH2F(HistoName.Data(), HistoTitle.Data(),
                                                                 m_anglesU, -TMath::Pi() * 0.5 / Unit::deg, TMath::Pi() * 0.5 / Unit::deg,
                                                                 m_anglesV, -TMath::Pi() * 0.5 / Unit::deg, TMath::Pi() * 0.5 / Unit::deg);
        m_histResidualRMSV[i_pk * m_shapes + i_shape]->GetXaxis()->SetTitle("Angle in u [deg]");
        m_histResidualRMSV[i_pk * m_shapes + i_shape]->GetYaxis()->SetTitle("Angle in v [deg]");

        // Bias Correction Error
        HistoName = Form("BiasCorrectionErrorU_PK%01i_Sh%02i", i_pk, i_shape + 1);
        HistoTitle = Form("Bias Correction Error U, pixel kind %01i, shape %02i", i_pk, i_shape + 1);
        m_histBiasCorrectionErrorU[i_pk * m_shapes + i_shape] = new TH2F(HistoName.Data(), HistoTitle.Data(),
            m_anglesU, -TMath::Pi() * 0.5 / Unit::deg, TMath::Pi() * 0.5 / Unit::deg,
            m_anglesV, -TMath::Pi() * 0.5 / Unit::deg, TMath::Pi() * 0.5 / Unit::deg);
        m_histBiasCorrectionErrorU[i_pk * m_shapes + i_shape]->GetXaxis()->SetTitle("Angle in u [deg]");
        m_histBiasCorrectionErrorU[i_pk * m_shapes + i_shape]->GetYaxis()->SetTitle("Angle in v [deg]");
        HistoName = Form("BiasCorrectionErrorV_PK%01i_Sh%02i", i_pk, i_shape + 1);
        HistoTitle = Form("Bias Correction Error V, pixel kind %01i, shape %02i", i_pk, i_shape + 1);
        m_histBiasCorrectionErrorV[i_pk * m_shapes + i_shape] = new TH2F(HistoName.Data(), HistoTitle.Data(),
            m_anglesU, -TMath::Pi() * 0.5 / Unit::deg, TMath::Pi() * 0.5 / Unit::deg,
            m_anglesV, -TMath::Pi() * 0.5 / Unit::deg, TMath::Pi() * 0.5 / Unit::deg);
        m_histBiasCorrectionErrorV[i_pk * m_shapes + i_shape]->GetXaxis()->SetTitle("Angle in u [deg]");
        m_histBiasCorrectionErrorV[i_pk * m_shapes + i_shape]->GetYaxis()->SetTitle("Angle in v [deg]");

        // Error Estimation
        HistoName = Form("ErrorEstimationU_PK%01i_Sh%02i", i_pk, i_shape + 1);
        HistoTitle = Form("Error Estimation U, pixel kind %01i, shape %02i", i_pk, i_shape + 1);
        m_histErrorEstimationU[i_pk * m_shapes + i_shape] = new TH2F(HistoName.Data(), HistoTitle.Data(),
            m_anglesU, -TMath::Pi() * 0.5 / Unit::deg, TMath::Pi() * 0.5 / Unit::deg,
            m_anglesV, -TMath::Pi() * 0.5 / Unit::deg, TMath::Pi() * 0.5 / Unit::deg);
        m_histErrorEstimationU[i_pk * m_shapes + i_shape]->GetXaxis()->SetTitle("Angle in u [deg]");
        m_histErrorEstimationU[i_pk * m_shapes + i_shape]->GetYaxis()->SetTitle("Angle in v [deg]");
        HistoName = Form("ErrorEstimationV_PK%01i_Sh%02i", i_pk, i_shape + 1);
        HistoTitle = Form("Error Estimation V, pixel kind %01i, shape %02i", i_pk, i_shape + 1);
        m_histErrorEstimationV[i_pk * m_shapes + i_shape] = new TH2F(HistoName.Data(), HistoTitle.Data(),
            m_anglesU, -TMath::Pi() * 0.5 / Unit::deg, TMath::Pi() * 0.5 / Unit::deg,
            m_anglesV, -TMath::Pi() * 0.5 / Unit::deg, TMath::Pi() * 0.5 / Unit::deg);
        m_histErrorEstimationV[i_pk * m_shapes + i_shape]->GetXaxis()->SetTitle("Angle in u [deg]");
        m_histErrorEstimationV[i_pk * m_shapes + i_shape]->GetYaxis()->SetTitle("Angle in v [deg]");

        // nCluster, nClusterFraction
        HistoName = Form("nClusters_PK%01i_Sh%02i", i_pk, i_shape + 1);
        HistoTitle = Form("Clusters, pixel kind %01i, shape %02i", i_pk, i_shape + 1);
        m_histnClusters[i_pk * m_shapes + i_shape] = new TH2F(HistoName.Data(), HistoTitle.Data(),
                                                              m_anglesU, -TMath::Pi() * 0.5 / Unit::deg, TMath::Pi() * 0.5 / Unit::deg,
                                                              m_anglesV, -TMath::Pi() * 0.5 / Unit::deg, TMath::Pi() * 0.5 / Unit::deg);
        m_histnClusters[i_pk * m_shapes + i_shape]->GetXaxis()->SetTitle("Angle in u [deg]");
        m_histnClusters[i_pk * m_shapes + i_shape]->GetYaxis()->SetTitle("Angle in v [deg]");
        HistoName = Form("nClusterFraction_PK%01i_Sh%02i", i_pk, i_shape + 1);
        HistoTitle = Form("Cluster Fraction V, pixel kind %01i, shape %02i", i_pk, i_shape + 1);
        m_histnClusterFraction[i_pk * m_shapes + i_shape] = new TH2F(HistoName.Data(), HistoTitle.Data(),
            m_anglesU, -TMath::Pi() * 0.5 / Unit::deg, TMath::Pi() * 0.5 / Unit::deg,
            m_anglesV, -TMath::Pi() * 0.5 / Unit::deg, TMath::Pi() * 0.5 / Unit::deg);
        m_histnClusterFraction[i_pk * m_shapes + i_shape]->GetXaxis()->SetTitle("Angle in u [deg]");
        m_histnClusterFraction[i_pk * m_shapes + i_shape]->GetYaxis()->SetTitle("Angle in v [deg]");

        fExpertHistograms->mkdir(DirShape.Data());
      }
    }
    fDQM->cd("NoSorting");

    // Bias Correction
    TString HistoName = Form("BiasCorrectionU");
    TString HistoTitle = Form("Bias CorrectionU, no sorting");
    m_histBiasCorrectionU[m_pixelkinds * m_shapes] = new TH2F(HistoName.Data(), HistoTitle.Data(),
                                                              m_anglesU, -TMath::Pi() * 0.5 / Unit::deg, TMath::Pi() * 0.5 / Unit::deg,
                                                              m_anglesV, -TMath::Pi() * 0.5 / Unit::deg, TMath::Pi() * 0.5 / Unit::deg);
    m_histBiasCorrectionU[m_pixelkinds * m_shapes]->GetXaxis()->SetTitle("Angle in u [deg]");
    m_histBiasCorrectionU[m_pixelkinds * m_shapes]->GetYaxis()->SetTitle("Angle in v [deg]");
    HistoName = Form("BiasCorrectionV");
    HistoTitle = Form("Bias CorrectionV, no sorting");
    m_histBiasCorrectionV[m_pixelkinds * m_shapes] = new TH2F(HistoName.Data(), HistoTitle.Data(),
                                                              m_anglesU, -TMath::Pi() * 0.5 / Unit::deg, TMath::Pi() * 0.5 / Unit::deg,
                                                              m_anglesV, -TMath::Pi() * 0.5 / Unit::deg, TMath::Pi() * 0.5 / Unit::deg);
    m_histBiasCorrectionV[m_pixelkinds * m_shapes]->GetXaxis()->SetTitle("Angle in u [deg]");
    m_histBiasCorrectionV[m_pixelkinds * m_shapes]->GetYaxis()->SetTitle("Angle in v [deg]");

    // Residual RMS
    HistoName = Form("ResidualRMSU");
    HistoTitle = Form("Residual RMS U, no sorting");
    m_histResidualRMSU[m_pixelkinds * m_shapes] = new TH2F(HistoName.Data(), HistoTitle.Data(),
                                                           m_anglesU, -TMath::Pi() * 0.5 / Unit::deg, TMath::Pi() * 0.5 / Unit::deg,
                                                           m_anglesV, -TMath::Pi() * 0.5 / Unit::deg, TMath::Pi() * 0.5 / Unit::deg);
    m_histResidualRMSU[m_pixelkinds * m_shapes]->GetXaxis()->SetTitle("Angle in u [deg]");
    m_histResidualRMSU[m_pixelkinds * m_shapes]->GetYaxis()->SetTitle("Angle in v [deg]");
    HistoName = Form("ResidualRMSV");
    HistoTitle = Form("Residual RMS V, no sorting");
    m_histResidualRMSV[m_pixelkinds * m_shapes] = new TH2F(HistoName.Data(), HistoTitle.Data(),
                                                           m_anglesU, -TMath::Pi() * 0.5 / Unit::deg, TMath::Pi() * 0.5 / Unit::deg,
                                                           m_anglesV, -TMath::Pi() * 0.5 / Unit::deg, TMath::Pi() * 0.5 / Unit::deg);
    m_histResidualRMSV[m_pixelkinds * m_shapes]->GetXaxis()->SetTitle("Angle in u [deg]");
    m_histResidualRMSV[m_pixelkinds * m_shapes]->GetYaxis()->SetTitle("Angle in v [deg]");

    // Bias Correction Error
    HistoName = Form("BiasCorrectionErrorU");
    HistoTitle = Form("Bias Correction Error U, no sorting");
    m_histBiasCorrectionErrorU[m_pixelkinds * m_shapes] = new TH2F(HistoName.Data(), HistoTitle.Data(),
        m_anglesU, -TMath::Pi() * 0.5 / Unit::deg, TMath::Pi() * 0.5 / Unit::deg,
        m_anglesV, -TMath::Pi() * 0.5 / Unit::deg, TMath::Pi() * 0.5 / Unit::deg);
    m_histBiasCorrectionErrorU[m_pixelkinds * m_shapes]->GetXaxis()->SetTitle("Angle in u [deg]");
    m_histBiasCorrectionErrorU[m_pixelkinds * m_shapes]->GetYaxis()->SetTitle("Angle in v [deg]");
    HistoName = Form("BiasCorrectionErrorV");
    HistoTitle = Form("Bias Correction Error V, no sorting");
    m_histBiasCorrectionErrorV[m_pixelkinds * m_shapes] = new TH2F(HistoName.Data(), HistoTitle.Data(),
        m_anglesU, -TMath::Pi() * 0.5 / Unit::deg, TMath::Pi() * 0.5 / Unit::deg,
        m_anglesV, -TMath::Pi() * 0.5 / Unit::deg, TMath::Pi() * 0.5 / Unit::deg);
    m_histBiasCorrectionErrorV[m_pixelkinds * m_shapes]->GetXaxis()->SetTitle("Angle in u [deg]");
    m_histBiasCorrectionErrorV[m_pixelkinds * m_shapes]->GetYaxis()->SetTitle("Angle in v [deg]");

    // Error Estimation
    HistoName = Form("ErrorEstimationU");
    HistoTitle = Form("Error Estimation U, no sorting");
    m_histErrorEstimationU[m_pixelkinds * m_shapes] = new TH2F(HistoName.Data(), HistoTitle.Data(),
                                                               m_anglesU, -TMath::Pi() * 0.5 / Unit::deg, TMath::Pi() * 0.5 / Unit::deg,
                                                               m_anglesV, -TMath::Pi() * 0.5 / Unit::deg, TMath::Pi() * 0.5 / Unit::deg);
    m_histErrorEstimationU[m_pixelkinds * m_shapes]->GetXaxis()->SetTitle("Angle in u [deg]");
    m_histErrorEstimationU[m_pixelkinds * m_shapes]->GetYaxis()->SetTitle("Angle in v [deg]");
    HistoName = Form("ErrorEstimationV");
    HistoTitle = Form("Error Estimation V, no sorting");
    m_histErrorEstimationV[m_pixelkinds * m_shapes] = new TH2F(HistoName.Data(), HistoTitle.Data(),
                                                               m_anglesU, -TMath::Pi() * 0.5 / Unit::deg, TMath::Pi() * 0.5 / Unit::deg,
                                                               m_anglesV, -TMath::Pi() * 0.5 / Unit::deg, TMath::Pi() * 0.5 / Unit::deg);
    m_histErrorEstimationV[m_pixelkinds * m_shapes]->GetXaxis()->SetTitle("Angle in u [deg]");
    m_histErrorEstimationV[m_pixelkinds * m_shapes]->GetYaxis()->SetTitle("Angle in v [deg]");

    // nCluster, nClusterFraction
    HistoName = Form("nClusters");
    HistoTitle = Form("Clusters, no sorting");
    m_histnClusters[m_pixelkinds * m_shapes] = new TH2F(HistoName.Data(), HistoTitle.Data(),
                                                        m_anglesU, -TMath::Pi() * 0.5 / Unit::deg, TMath::Pi() * 0.5 / Unit::deg,
                                                        m_anglesV, -TMath::Pi() * 0.5 / Unit::deg, TMath::Pi() * 0.5 / Unit::deg);
    m_histnClusters[m_pixelkinds * m_shapes]->GetXaxis()->SetTitle("Angle in u [deg]");
    m_histnClusters[m_pixelkinds * m_shapes]->GetYaxis()->SetTitle("Angle in v [deg]");
    HistoName = Form("nClusterFraction");
    HistoTitle = Form("Cluster Fraction, no sorting");
    m_histnClusterFraction[m_pixelkinds * m_shapes] = new TH2F(HistoName.Data(), HistoTitle.Data(),
                                                               m_anglesU, -TMath::Pi() * 0.5 / Unit::deg, TMath::Pi() * 0.5 / Unit::deg,
                                                               m_anglesV, -TMath::Pi() * 0.5 / Unit::deg, TMath::Pi() * 0.5 / Unit::deg);
    m_histnClusterFraction[m_pixelkinds * m_shapes]->GetXaxis()->SetTitle("Angle in u [deg]");
    m_histnClusterFraction[m_pixelkinds * m_shapes]->GetYaxis()->SetTitle("Angle in v [deg]");

//    fDQM->cd();
//    fExpertHistograms->cd();

    fDQM->cd("NoSorting");
    fExpertHistograms->cd("NoSorting");
  }

  auto tree = getTreeObjectPtr(name_SourceTree.Data());
  int nEntries = tree->GetEntries();

  B2INFO("Entries: " << nEntries);
  SummariesInfo[0] = nEntries;
  B2INFO("UseTracks: " << m_UseTracks <<
         ", UseRealData: " << m_UseRealData <<
         ", CompareTruePointTracks: " << m_CompareTruePointTracks <<
         ", CalibrationKind: " << m_CalibrationKind <<
         ", PixelKind: " << m_PixelKind
        );

  tree->SetBranchAddress("event", &m_evt);
  tree->SetBranchAddress("run", &m_run);
  tree->SetBranchAddress("exp", &m_exp);
  tree->SetBranchAddress("pid", &m_procId);
  tree->SetBranchAddress("layer", &m_layer);
  tree->SetBranchAddress("sensor", &m_sensor);
  tree->SetBranchAddress("pixelKind", &m_pixelKind);
  tree->SetBranchAddress("closeEdge", &m_closeEdge);
  tree->SetBranchAddress("shape", &m_shape);
  tree->SetBranchAddress("phiTrack", &m_phiTrack);
  tree->SetBranchAddress("thetaTrack", &m_thetaTrack);
  tree->SetBranchAddress("phiTrue", &m_phiTrue);
  tree->SetBranchAddress("thetaTrue", &m_thetaTrue);
  tree->SetBranchAddress("signal", &m_signal);
  tree->SetBranchAddress("seed", &m_seed);
  tree->SetBranchAddress("InPixUTrue", &m_InPixUTrue);
  tree->SetBranchAddress("InPixVTrue", &m_InPixVTrue);
  tree->SetBranchAddress("InPixU", &m_InPixU);
  tree->SetBranchAddress("InPixV", &m_InPixV);
  tree->SetBranchAddress("ResidUTrue", &m_ResidUTrue);
  tree->SetBranchAddress("ResidVTrue", &m_ResidVTrue);
  tree->SetBranchAddress("SigmaU", &m_SigmaU);
  tree->SetBranchAddress("SigmaV", &m_SigmaV);
  tree->SetBranchAddress("ResidUTrack", &m_ResidUTrack);
  tree->SetBranchAddress("ResidVTrack", &m_ResidVTrack);
  tree->SetBranchAddress("SigmaUTrack", &m_SigmaUTrack);
  tree->SetBranchAddress("SigmaVTrack", &m_SigmaVTrack);


  // create vector for storing on database:

  TVectorD* Correction_Bias;
//  std::vector<TVectorD*> Correction_Bias(1);
  TVectorD* Correction_ErrorEstimation;
  TVectorD* Correction_BiasErr;
  TVectorD* InPixelPosition;

  TVectorD* PXDShCalibrationBasicSetting;
  PXDShCalibrationBasicSetting = new TVectorD(50);
  double fPXDShCalibrationBasicSetting[50];
  for (int i = 0; i < 50; i++) fPXDShCalibrationBasicSetting[i] = 0.0;
  fPXDShCalibrationBasicSetting[0] = m_shapes;
  fPXDShCalibrationBasicSetting[1] = m_pixelkinds;
  fPXDShCalibrationBasicSetting[2] = m_dimensions;
  fPXDShCalibrationBasicSetting[3] = m_anglesU;
  fPXDShCalibrationBasicSetting[4] = m_anglesV;
  fPXDShCalibrationBasicSetting[5] = m_in_pixelU;
  fPXDShCalibrationBasicSetting[6] = m_in_pixelV;
  PXDShCalibrationBasicSetting->SetElements(fPXDShCalibrationBasicSetting);

  Correction_Bias = new TVectorD(m_shapes * m_pixelkinds * m_dimensions * m_anglesU * m_anglesV);
  Correction_ErrorEstimation = new TVectorD(m_shapes * m_pixelkinds * m_dimensions * m_anglesU * m_anglesV);
  Correction_BiasErr = new TVectorD(m_shapes * m_pixelkinds * m_dimensions * m_anglesU * m_anglesV);
  InPixelPosition = new TVectorD(m_shapes * m_pixelkinds * m_anglesU * m_anglesV * m_in_pixelU * m_in_pixelV);


  // create tables for filling and normal using:

//  typedef boost::multi_array<double, 5> correction_table_type;
//  typedef boost::multi_array<double, 6> inpixel_table_type;

//  correction_table_type TCorrection_Bias(boost::extents[m_shapes][m_pixelkinds][m_dimensions][m_anglesU][m_anglesV]);
//  correction_table_type TCorrection_ErrorEstimation(boost::extents[m_shapes][m_pixelkinds][m_dimensions][m_anglesU][m_anglesV]);
//  correction_table_type TCorrection_BiasErr(boost::extents[m_shapes][m_pixelkinds][m_dimensions][m_anglesU][m_anglesV]);
//  inpixel_table_type TInPixelPosition(boost::extents[m_shapes][m_pixelkinds][m_anglesU][m_anglesV][m_in_pixelU][m_in_pixelV]);


  // presets of vectors and tables:

  Double_t* ValueCors = new Double_t[m_shapes * m_pixelkinds * m_dimensions * m_anglesU * m_anglesV];
  Double_t* ValueInPix = new Double_t[m_shapes * m_pixelkinds * m_anglesU * m_anglesV * m_in_pixelU * m_in_pixelV];
  for (int i = 0; i < m_shapes * m_pixelkinds * m_dimensions * m_anglesU * m_anglesV; i++)
    ValueCors[i] = 0.0;
  Correction_Bias->SetElements(ValueCors);
  Correction_BiasErr->SetElements(ValueCors);
  for (int i = 0; i < m_shapes * m_pixelkinds * m_dimensions * m_anglesU * m_anglesV; i++)
    ValueCors[i] = 1.0;
  Correction_ErrorEstimation->SetElements(ValueCors);
  for (int i = 0; i < m_shapes * m_pixelkinds * m_anglesU * m_anglesV * m_in_pixelU * m_in_pixelV; i++)
    ValueInPix[i] = 0.0;
  InPixelPosition->SetElements(ValueInPix);

  for (int i_shape = 0; i_shape < m_shapes; i_shape++)
    for (int i_pk = 0; i_pk < m_pixelkinds; i_pk++)
      for (int i_axis = 0; i_axis < m_dimensions; i_axis++)
        for (int i_angleU = 0; i_angleU < m_anglesU; i_angleU++)
          for (int i_angleV = 0; i_angleV < m_anglesV; i_angleV++) {
//            TCorrection_Bias[i_shape][i_pk][i_axis][i_angleU][i_angleV] = 0.0;
//            TCorrection_ErrorEstimation[i_shape][i_pk][i_axis][i_angleU][i_angleV] = 1.0;
//            TCorrection_BiasErr[i_shape][i_pk][i_axis][i_angleU][i_angleV] = 0.0;
          }
  for (int i_shape = 0; i_shape < m_shapes; i_shape++)
    for (int i_pk = 0; i_pk < m_pixelkinds; i_pk++)
      for (int i_angleU = 0; i_angleU < m_anglesU; i_angleU++)
        for (int i_angleV = 0; i_angleV < m_anglesV; i_angleV++)
          for (int i_ipU = 0; i_ipU < m_in_pixelU; i_ipU++)
            for (int i_ipV = 0; i_ipV < m_in_pixelV; i_ipV++) {
//              TInPixelPosition[i_shape][i_pk][i_angleU][i_angleV][i_ipU][i_ipV] = 0.0;
            }

  B2DEBUG(30, "--> Presets done. ");

  //std::unordered_map<key_type, double, key_hash, key_equal> TCorrection_BiasMap;
  map_typeCorrs TCorrection_BiasMap;
  map_typeCorrs TCorrection_ErrorEstimationMap;
  map_typeCorrs TCorrection_BiasMapErr;
  std::unordered_map<key_type2, double, key_hash2, key_equal2> TInPixelPositionMap;
  for (int i_shape = 0; i_shape < m_shapes; i_shape++)
    for (int i_pk = 0; i_pk < m_pixelkinds; i_pk++)
      for (int i_axis = 0; i_axis < m_dimensions; i_axis++)
        for (int i_angleU = 0; i_angleU < m_anglesU; i_angleU++)
          for (int i_angleV = 0; i_angleV < m_anglesV; i_angleV++) {
            TCorrection_BiasMap[make_tuple(i_shape, i_pk, i_axis, i_angleU, i_angleV)] = 0;
            TCorrection_ErrorEstimationMap[make_tuple(i_shape, i_pk, i_axis, i_angleU, i_angleV)] = 1.0;
            TCorrection_BiasMapErr[make_tuple(i_shape, i_pk, i_axis, i_angleU, i_angleV)] = 0;
          }
  for (int i_shape = 0; i_shape < m_shapes; i_shape++)
    for (int i_pk = 0; i_pk < m_pixelkinds; i_pk++)
      for (int i_angleU = 0; i_angleU < m_anglesU; i_angleU++)
        for (int i_angleV = 0; i_angleV < m_anglesV; i_angleV++)
          for (int i_ipU = 0; i_ipU < m_in_pixelU; i_ipU++)
            for (int i_ipV = 0; i_ipV < m_in_pixelV; i_ipV++)
              TInPixelPositionMap[make_tuple(i_shape, i_pk, i_angleU, i_angleV, i_ipU, i_ipV)] = 0.0;

  B2DEBUG(30, "--> Ordering done. ");

  if (m_DoExpertHistograms) {
    for (int i_ev = 0; i_ev < nEntries; i_ev++) {
      tree->GetEntry(i_ev);
      int iShp = m_shape;
      if (iShp < 1) iShp = 1;
      if (iShp > m_shapes) iShp = m_shapes;

      m_histSignal[iShp - 1]->Fill(m_signal);
      m_histSignal[m_shapes]->Fill(m_signal);
      m_histSeed[iShp - 1]->Fill(m_seed);
      m_histSeed[m_shapes]->Fill(m_seed);

      int AngU = m_anglesU / 2 + (int)(m_phiTrack * m_anglesU / TMath::Pi());
      int AngV = m_anglesV / 2 + (int)(m_thetaTrack * m_anglesV / TMath::Pi());
      if (AngU < 0) AngU = 0;
      if (AngU >= m_anglesU) AngU = m_anglesU - 1;
      if (AngV < 0) AngV = 0;
      if (AngV >= m_anglesV) AngV = m_anglesV - 1;

      m_histTrackSigmaU[AngU * m_anglesV + AngV]->Fill(m_SigmaUTrack / Unit::um);
      m_histTrackSigmaU[m_anglesU * m_anglesV]->Fill(m_SigmaUTrack / Unit::um);
      m_histTrackSigmaV[AngU * m_anglesV + AngV]->Fill(m_SigmaVTrack / Unit::um);
      m_histTrackSigmaV[m_anglesU * m_anglesV]->Fill(m_SigmaVTrack / Unit::um);
      m_histTrackSigmaUV[AngU * m_anglesV + AngV]->Fill(m_SigmaUTrack / Unit::um, m_SigmaVTrack / Unit::um);
      m_histTrackSigmaUV[m_anglesU * m_anglesV]->Fill(m_SigmaUTrack / Unit::um, m_SigmaVTrack / Unit::um);

    }
    B2DEBUG(30, "--> Filling of histos done. ");
  }


  // presets of vectors and tables:

  freopen("/dev/null", "w", stderr);

  int nSelRowsTemp = 0;
  for (int i_shape = 0; i_shape < m_shapes; i_shape++) {  // TODO nastavit na 0
    TString cCat;
    TCut c1;
    TCut c2;
    TCut c3a;
    TCut c3b;
    TCut c4a;
    TCut c4b;
    TCut c5;
    TCut cFin;
    cCat = Form("shape == %i", i_shape + 1);
    c1.SetTitle(cCat.Data());
    B2INFO("---> Processing cluster shape: " << i_shape + 1 <<
           ", description: " << Belle2::PXD::PXDClusterShape::pxdClusterShapeDescription[(Belle2::PXD::pxdClusterShapeType)(
                 i_shape + 1)].c_str()
          );
    if (!m_DoExpertHistograms) {  // acceleration of calibration process
      tree->Draw("ResidUTrack:ResidVTrack", c1, "goff");
      nSelRowsTemp = (int)tree->GetSelectedRows();
      if (nSelRowsTemp < m_MinClustersCorrections) continue;
    }
    for (int i_pk = 0; i_pk < m_pixelkinds; i_pk++) {  // TODO nastavit na 0
      cCat = Form("pixelKind == %i", i_pk);
      c2.SetTitle(cCat.Data());
      if (!m_DoExpertHistograms) {  // acceleration of calibration process
        tree->Draw("ResidUTrack:ResidVTrack", c1 + c2, "goff");
        nSelRowsTemp = (int)tree->GetSelectedRows();
        if (nSelRowsTemp < m_MinClustersCorrections) continue;
      }
      B2INFO("   ---> Processing pixel kind: " << i_pk <<
             ", Layer: " << (int)((i_pk % 4) / 2) + 1 <<
             ", Sensor: " << (int)(i_pk / 4) + 1 <<
             ", Size: " << i_pk % 2
            );
      for (int i_angleU = 0; i_angleU < m_anglesU; i_angleU++) {  // TODO nastavit na 0
        double PhiMi = ((TMath::Pi() * i_angleU) / m_anglesU) - (TMath::Pi() / 2.0);
        double PhiMa = ((TMath::Pi() * (i_angleU + 1)) / m_anglesU) - (TMath::Pi() / 2.0);
        if ((m_UseRealData == kTRUE) || (m_UseTracks == kTRUE)) {
          //PhiMi = TMath::Tan(PhiMi);  // TODO remeove those two lines - now correction of error in input datas (slope insteed angle)
          //PhiMa = TMath::Tan(PhiMa);  // TODO remeove those two lines - now correction of error in input datas (slope insteed angle)
          cCat = Form("phiTrack > %f", PhiMi);
          c3a.SetTitle(cCat.Data());
          cCat = Form("phiTrack < %f", PhiMa);
          c3b.SetTitle(cCat.Data());
        } else if ((m_CalibrationKind == 2) || (m_UseTracks == kFALSE)) {
          cCat = Form("phiTrue > %f", PhiMi);
          c3a.SetTitle(cCat.Data());
          cCat = Form("phiTrue < %f", PhiMa);
          c3b.SetTitle(cCat.Data());
        }
        if (!m_DoExpertHistograms) {  // acceleration of calibration process
          tree->Draw("ResidUTrack:ResidVTrack", c1 + c2 + c3a + c3b, "goff");
          nSelRowsTemp = (int)tree->GetSelectedRows();
          if (nSelRowsTemp < m_MinClustersCorrections) continue;
        }
        for (int i_angleV = 0; i_angleV < m_anglesV; i_angleV++) {  // TODO nastavit na 0
          SummariesInfo[1]++;
          B2DEBUG(130, "  --> AngleCalibration for: " << i_angleU << ", " << i_angleV);

          double ThetaMi = ((TMath::Pi() * i_angleV) / m_anglesV) - (TMath::Pi() / 2.0);
          double ThetaMa = ((TMath::Pi() * (i_angleV + 1)) / m_anglesV) - (TMath::Pi() / 2.0);
          if ((m_UseRealData == kTRUE) || (m_UseTracks == kTRUE)) {
            //ThetaMi = TMath::Tan(ThetaMi);  // TODO remeove those two lines - now correction of error in input datas (slope insteed angle)
            //ThetaMa = TMath::Tan(ThetaMa);  // TODO remeove those two lines - now correction of error in input datas (slope insteed angle)
            cCat = Form("thetaTrack > %f", ThetaMi);
            c4a.SetTitle(cCat.Data());
            cCat = Form("thetaTrack < %f", ThetaMa);
            c4b.SetTitle(cCat.Data());
          } else if ((m_CalibrationKind == 2) || (m_UseTracks == kFALSE)) {
            cCat = Form("thetaTrue > %f", ThetaMi);
            c4a.SetTitle(cCat.Data());
            cCat = Form("thetaTrue < %f", ThetaMa);
            c4b.SetTitle(cCat.Data());
          }
          if (!m_DoExpertHistograms) {  // acceleration of calibration process
            tree->Draw("ResidUTrack:ResidVTrack", c1 + c2 + c3a + c3b + c4a + c4b, "goff");
            nSelRowsTemp = (int)tree->GetSelectedRows();
            if (nSelRowsTemp < m_MinClustersCorrections) continue;
          }
          cCat = Form("closeEdge == 0");
          c5.SetTitle(cCat.Data());

          cFin = c1 && c2 && c3a && c3b && c4a && c4b && c5;

          tree->Draw(">>selection", cFin);
          TEventList* selectionn = (TEventList*)gDirectory->Get("selection");
          //tree->SetEventList(selectionn);
          //printf("---> GetSelRaws : %i\n",(int)tree->GetSelectedRows() );
          int nSelRows = (int)tree->GetSelectedRows();
          SummariesInfoSh[i_shape] += nSelRows;
          SummariesInfoAng[i_angleU * m_anglesV + i_angleV] += nSelRows;
          SummariesInfoPK[i_pk] += nSelRows;
          if (nSelRows >= m_MinClustersCorrections) {
            std::vector<double> Colm1(nSelRows);
            std::vector<double> Colm2(nSelRows);
            std::vector<double> Colm3(nSelRows);
            std::vector<double> Colm4(nSelRows);
            // std::vector<double> Colm5(nSelRows);
            // std::vector<double> Colm6(nSelRows);
            std::vector<double> Colm7(nSelRows);
            std::vector<double> Colm8(nSelRows);
            for (int i = 0; i < nSelRows; i++) {
              tree->GetEntry(selectionn->GetEntry(i));
              if ((m_UseRealData == kTRUE) || (m_UseTracks == kTRUE)) {
                Colm1[i] = m_ResidUTrack;
                Colm2[i] = m_ResidVTrack;
              } else if ((m_CalibrationKind == 2) || (m_UseTracks == kFALSE)) {
                Colm1[i] = m_ResidUTrue;
                Colm2[i] = m_ResidVTrue;
              }
              Colm3[i] = m_SigmaU;
              Colm4[i] = m_SigmaV;
              //Colm5[i] = m_ResidUTrue;
              //Colm6[i] = m_ResidVTrue;
              Colm7[i] = m_SigmaUTrack;
              Colm8[i] = m_SigmaVTrack;
              //Colm1[i] = Colm5[i];     // TODO true - test
              //Colm2[i] = Colm6[i];

            }

            //          int imax = (int)tree->GetSelectedRows();
            //          if (imax > 50) imax = 50;
            //          tree->Draw("ResidUTrack:ResidVTrack:ResidUTrack/SigmaU:ResidVTrack/SigmaV", cFin, "goff");
            //          for (int i = 0; i < imax; i++ ) {
            //tree->GetEvent(i);
            //            tree->GetEntry(selectionn->GetEntry(i));
            //printf("%lld ", tree->GetEntryNumber(i));
            //            printf("%i %i (%i %i %4.1f) (%f - %f - %f) ", i, m_evt, m_shape, m_pixelKind, m_signal, 22.2, Colm1[i], m_ResidUTrack);
            //printf("%i %i (%i %i %4.1f) (%f - %f - %f) ", i, m_evt, m_shape, m_pixelKind, m_signal, 22, 22, m_ResidUTrack);
            //printf("%i %i %lli (%i %i %4.1f) ", i, m_evt, selection->GetEntry(i), m_shape, m_pixelKind, m_signal);
            //          }
            //          if ((m_UseRealData == kTRUE) || (m_UseTracks == kTRUE)) {
            //tree->Draw("ResidUTrack:ResidVTrack:ResidUTrack/SigmaU:ResidVTrack/SigmaV", cFin, "goff");
            //tree->Draw("ResidUTrack:ResidVTrack:ResidUTrack/SigmaU:ResidVTrack/SigmaV", "", "goff");
            //          } else if ((m_CalibrationKind == 2) || (m_UseTracks == kFALSE)) {
            //tree->Draw("ResidUTrue:ResidVTrue:ResidUTrue/SigmaU:ResidVTrue/SigmaV", cFin, "goff");
            //tree->Draw("ResidUTrue:ResidVTrue:ResidUTrue/SigmaU:ResidVTrue/SigmaV", "", "goff");
            //          }

            //          printf("---> GetSelRaws : %i \n",(int)tree->GetSelectedRows() );
            //          continue;

            // B2DEBUG(10, "2--> Selection criteria: ");
            // cout.rdbuf (old);              // <-- restore
            // B2DEBUG(10, "3--> Selection criteria: ");


            B2DEBUG(30, "--> Selected raws " << nSelRows);
//            double* Col1 = tree->GetV1();
//            double* Col2 = tree->GetV2();
            //double* Col3 = tree->GetV3();
            //double* Col4 = tree->GetV4();

            double RetVal;
            double RetValError;
            double RetRMS;
            RetVal = 0;
            RetValError = 0;
            RetRMS = 0;

            if (CalculateCorrection(1, nSelRows, Colm1, &RetVal, &RetValError, &RetRMS)) {
              TCorrection_BiasMap[make_tuple(i_shape, i_pk, 0, i_angleU, i_angleV)] = RetVal;
              TCorrection_BiasMapErr[make_tuple(i_shape, i_pk, 0, i_angleU, i_angleV)] = RetValError;
              SummariesInfo[2]++;
              SummariesInfo[4] += nSelRows;
              if (m_DoExpertHistograms) {
                m_histBiasCorrectionU[i_pk * m_shapes + i_shape]->SetBinContent(i_angleU + 1, i_angleV + 1, RetVal);
                m_histBiasCorrectionErrorU[i_pk * m_shapes + i_shape]->SetBinContent(i_angleU + 1, i_angleV + 1, RetValError);
                m_histResidualRMSU[i_pk * m_shapes + i_shape]->SetBinContent(i_angleU + 1, i_angleV + 1, RetRMS);
                m_histBiasCorrectionU[m_pixelkinds * m_shapes]->SetBinContent(i_angleU + 1, i_angleV + 1,
                    m_histBiasCorrectionU[m_pixelkinds * m_shapes]->GetBinContent(i_angleU + 1, i_angleV + 1) + 1);
              }
            }
            if (CalculateCorrection(1, nSelRows, Colm2, &RetVal, &RetValError, &RetRMS)) {
              TCorrection_BiasMap[make_tuple(i_shape, i_pk, 1, i_angleU, i_angleV)] = RetVal;
              TCorrection_BiasMapErr[make_tuple(i_shape, i_pk, 1, i_angleU, i_angleV)] = RetValError;
              SummariesInfo[3]++;
              SummariesInfo[5] += nSelRows;
              if (m_DoExpertHistograms) {
                m_histBiasCorrectionV[i_pk * m_shapes + i_shape]->SetBinContent(i_angleU + 1, i_angleV + 1, RetVal);
                m_histBiasCorrectionErrorV[i_pk * m_shapes + i_shape]->SetBinContent(i_angleU + 1, i_angleV + 1, RetValError);
                m_histResidualRMSV[i_pk * m_shapes + i_shape]->SetBinContent(i_angleU + 1, i_angleV + 1, RetRMS);
                m_histBiasCorrectionV[m_pixelkinds * m_shapes]->SetBinContent(i_angleU + 1, i_angleV + 1,
                    m_histBiasCorrectionV[m_pixelkinds * m_shapes]->GetBinContent(i_angleU + 1, i_angleV + 1) + 1);
              }
            }
//            printf("--> %f EE%f\n", Colm1[1] * 10000.0, Colm1[1] / Colm3[1]);
            for (int i = 0; i < nSelRows; i++) {
              //double co1 = TMath::Sqrt(Colm1[i] * Colm1[i] - Colm7[i] * Colm7[i]);
              //double co2 = TMath::Sqrt(Colm2[i] * Colm2[i] - Colm8[i] * Colm8[i]);
              //Colm3[i] = co1 / Colm3[i];
              //Colm4[i] = co2 / Colm4[i];
              Colm1[i] -= TCorrection_BiasMap[make_tuple(i_shape, i_pk, 0, i_angleU, i_angleV)];
              Colm2[i] -= TCorrection_BiasMap[make_tuple(i_shape, i_pk, 1, i_angleU, i_angleV)];
              if ((m_UseRealData == kTRUE) || (m_UseTracks == kTRUE)) {
                Colm3[i] = TMath::Sqrt(Colm3[i] * Colm3[i] + Colm7[i] * Colm7[i]);
                Colm4[i] = TMath::Sqrt(Colm4[i] * Colm4[i] + Colm8[i] * Colm8[i]);
              }
              Colm3[i] = Colm1[i] / Colm3[i];
              Colm4[i] = Colm2[i] / Colm4[i];
              //Colm3[i] /= 2.5;
              //Colm4[i] /= 2.5;
            }
//            printf("  --> %f %f EE%f\n",TCorrection_BiasMap[make_tuple(i_shape, i_pk, 0, i_angleU, i_angleV)] * 10000.0, Colm1[1] * 10000.0, Colm3[1]);

            if (CalculateCorrection(2, nSelRows, Colm3, &RetVal, &RetValError, &RetRMS)) {
              TCorrection_ErrorEstimationMap[make_tuple(i_shape, i_pk, 0, i_angleU, i_angleV)] = RetRMS;
              SummariesInfo[6]++;
              SummariesInfo[8] += nSelRows;
              if (m_DoExpertHistograms) {
                m_histErrorEstimationU[i_pk * m_shapes + i_shape]->SetBinContent(i_angleU + 1, i_angleV + 1, RetRMS);
                m_histErrorEstimationU[m_pixelkinds * m_shapes]->SetBinContent(i_angleU + 1, i_angleV + 1,
                    m_histErrorEstimationU[m_pixelkinds * m_shapes]->GetBinContent(i_angleU + 1, i_angleV + 1) + 1);
              }
            }
            if (CalculateCorrection(2, nSelRows, Colm4, &RetVal, &RetValError, &RetRMS)) {
              TCorrection_ErrorEstimationMap[make_tuple(i_shape, i_pk, 1, i_angleU, i_angleV)] = RetRMS;
              SummariesInfo[7]++;
              SummariesInfo[9] += nSelRows;
              if (m_DoExpertHistograms) {
                m_histErrorEstimationV[i_pk * m_shapes + i_shape]->SetBinContent(i_angleU + 1, i_angleV + 1, RetRMS);
                m_histErrorEstimationV[m_pixelkinds * m_shapes]->SetBinContent(i_angleU + 1, i_angleV + 1,
                    m_histErrorEstimationV[m_pixelkinds * m_shapes]->GetBinContent(i_angleU + 1, i_angleV + 1) + 1);
              }
            }
            //printf("  --> BCor%6.1f, EECor%6.3f\n",TCorrection_BiasMap[make_tuple(i_shape, i_pk, 0, i_angleU, i_angleV)] * 10000.0, TCorrection_ErrorEstimationMap[make_tuple(i_shape, i_pk, 1, i_angleU, i_angleV)]);

            if (m_DoExpertHistograms) {
              m_histnClusters[i_pk * m_shapes + i_shape]->SetBinContent(i_angleU + 1, i_angleV + 1, nSelRows);
              m_histnClusters[m_pixelkinds * m_shapes]->SetBinContent(i_angleU + 1, i_angleV + 1,
                                                                      m_histnClusters[m_pixelkinds * m_shapes]->GetBinContent(i_angleU + 1, i_angleV + 1) + nSelRows);
              m_histnClusterFraction[i_pk * m_shapes + i_shape]->SetBinContent(i_angleU + 1, i_angleV + 1, (float)nSelRows / nEntries);
              m_histnClusterFraction[m_pixelkinds * m_shapes]->SetBinContent(i_angleU + 1, i_angleV + 1,
                  (float)m_histnClusters[m_pixelkinds * m_shapes]->GetBinContent(i_angleU + 1, i_angleV + 1) / nEntries);
            }

            if (m_MinHitsInPixels > 0) if (nSelRows >= m_MinHitsInPixels) {
                double PixSizeU = 50.0; //Unit::um  (TODO use more general way to obtain clustersize?)
                double PixSizeV = 0;      //Unit::um
                if ((i_pk == 0) || (i_pk == 4)) {
                  PixSizeV = 55.0;
                } else if ((i_pk == 1) || (i_pk == 5)) {
                  PixSizeV = 60.0;
                } else if ((i_pk == 2) || (i_pk == 6)) {
                  PixSizeV = 70.0;
                } else if ((i_pk == 3) || (i_pk == 7)) {
                  PixSizeV = 85.0;
                }
                TString sVarexp;
                if ((m_UseRealData == kTRUE) || (m_UseTracks == kTRUE)) {
                  sVarexp = Form("((InPixU / %f) + 0.5) : ((InPixV / %f) + 0.5)", PixSizeU, PixSizeV);
                } else if ((m_CalibrationKind == 2) || (m_UseTracks == kFALSE)) {
                  sVarexp = Form("((InPixUTrue / %f) + 0.5):((InPixVTrue / %f) + 0.5)", PixSizeU, PixSizeV);
                }

                for (int i_ipU = 0; i_ipU < m_in_pixelU; i_ipU++) {
                  for (int i_ipV = 0; i_ipV < m_in_pixelV; i_ipV++) {
                    B2DEBUG(130, "--> InPixCalibration for: " << i_ipU << ", " << i_ipV);

                    TCut c6a;
                    TCut c6b;
                    TCut c7a;
                    TCut c7b;

                    double MinValU = (double)i_ipU / m_in_pixelU;
                    double MaxValU = (double)(i_ipU + 1) / m_in_pixelU;
                    double MinValV = (double)i_ipV / m_in_pixelV;
                    double MaxValV = (double)(i_ipV + 1) / m_in_pixelV;
                    cCat = Form("((InPixU / %f) + 0.5) > %f", PixSizeU, MinValU);
                    c6a.SetTitle(cCat.Data());
                    cCat = Form("((InPixU / %f) + 0.5) < %f", PixSizeU, MaxValU);
                    c6b.SetTitle(cCat.Data());
                    cCat = Form("((InPixV / %f) + 0.5) > %f", PixSizeV, MinValV);
                    c7a.SetTitle(cCat.Data());
                    cCat = Form("((InPixV / %f) + 0.5) < %f", PixSizeV, MaxValV);
                    c7b.SetTitle(cCat.Data());

                    cFin = c6a && c6b && c7a && c7b && c1 && c2 && c3a && c3b && c4a && c4b && c5;

                    tree->Draw(sVarexp.Data(), cFin, "goff");

                    int nSelRows2 = (int)tree->GetSelectedRows();
                    if (nSelRows2 >= m_MinHitsAcceptInPixels) {
                      TInPixelPositionMap[make_tuple(i_shape, i_pk, i_angleU, i_angleV, i_ipU, i_ipV)] = 1.0;
                    }
                  }
                }
              }
          }
        }
      }
    }
  }

  B2DEBUG(30, "--> bias correction calculation done. ");

//  int n_Events = tree->GetEntries();
  /*
    double *fResidU = new double[n_Events];
    double *fResidV = new double[n_Events];
    for (int i_Ev = 0; i_Ev < n_Events; i_Ev++) {
      tree->GetEntry(i_Ev);
      int iIndexPhi = (m_phiTrue+ (TMath::Pi() / 2.0)) / (TMath::Pi() / m_anglesU);
      int iIndexTheta = (m_thetaTrue + (TMath::Pi() / 2.0)) / (TMath::Pi() / m_anglesV);
      fResidU[i_Ev] = m_ResidUTrue;
      fResidV[i_Ev] = m_ResidVTrue;
      if ((m_UseRealData == kTRUE) || (m_UseTracks == kTRUE)) {
        iIndexPhi = (m_phiTrack+ (TMath::Pi() / 2.0)) / (TMath::Pi() / m_anglesU);
        iIndexTheta = (m_thetaTrack + (TMath::Pi() / 2.0)) / (TMath::Pi() / m_anglesV);
        fResidU[i_Ev] = m_ResidUTrack;
        fResidV[i_Ev] = m_ResidVTrack;
      }
      int iShape = m_shape;
      int iPK = m_pixelKind;
      fResidU[i_Ev] -= TCorrection_BiasMap[make_tuple(iShape, iPK, 0, iIndexPhi, iIndexTheta)];
      fResidV[i_Ev] -= TCorrection_BiasMap[make_tuple(iShape, iPK, 1, iIndexPhi, iIndexTheta)];
    }
  */
//  B2DEBUG(30, "--> bias correction aplication done. ");
  /*
    double* fEEU = new double[n_Events];
    double* fEEV = new double[n_Events];
    int nSelRows = 0;
    for (int i_shape = 0; i_shape < m_shapes; i_shape++) {
      B2INFO("Error estimation---> cluster shape: " << i_shape + 1 <<
             ", description: " << Belle2::PXD::PXDClusterShape::pxdClusterShapeDescription[(Belle2::PXD::pxdClusterShapeType)(
                   i_shape + 1)].c_str()
            );
      for (int i_pk = 0; i_pk < m_pixelkinds; i_pk++) {
        B2INFO("Error estimation   ---> pixel kind: " << i_pk <<
               ", Layer: " << (int)((i_pk % 4) / 2) + 1 <<
               ", Sensor: " << (int)(i_pk / 4) + 1 <<
               ", Size: " << i_pk % 2
              );
        for (int i_angleU = 0; i_angleU < m_anglesU; i_angleU++) {
          for (int i_angleV = 0; i_angleV < m_anglesV; i_angleV++) {
            nSelRows = 0;
            for (int i_Ev = 0; i_Ev < n_Events; i_Ev++) {
              if ((i_Ev == (int)(n_Events / 2)) && (nSelRows < m_MinClustersCorrections / 4)) {  // acceleration
                continue;
              }
              tree->GetEntry(i_Ev);
              int iIndexPhi = (m_phiTrue + (TMath::Pi() / 2.0)) / (TMath::Pi() / m_anglesU);
              int iIndexTheta = (m_thetaTrue + (TMath::Pi() / 2.0)) / (TMath::Pi() / m_anglesV);
              if ((m_UseRealData == kTRUE) || (m_UseTracks == kTRUE)) {
                iIndexPhi = (m_phiTrack + (TMath::Pi() / 2.0)) / (TMath::Pi() / m_anglesU);
                iIndexTheta = (m_thetaTrack + (TMath::Pi() / 2.0)) / (TMath::Pi() / m_anglesV);
              }
              int iShape = m_shape;
              int iPK = m_pixelKind;
              if ((i_shape == iShape) && (i_pk == iPK) && (i_angleU == iIndexPhi) && (i_angleV == iIndexTheta)) {
                double fResidU = m_ResidUTrue;
                double fResidV = m_ResidVTrue;
                if ((m_UseRealData == kTRUE) || (m_UseTracks == kTRUE)) {
                  fResidU = m_ResidUTrack;
                  fResidV = m_ResidVTrack;
                }
                fResidU -= TCorrection_BiasMap[make_tuple(iShape, iPK, 0, iIndexPhi, iIndexTheta)];
                fResidV -= TCorrection_BiasMap[make_tuple(iShape, iPK, 1, iIndexPhi, iIndexTheta)];
                fEEU[nSelRows] = fResidU / m_SigmaU;
                fEEV[nSelRows] = fResidV / m_SigmaV;
                nSelRows++;
              }
            }

            if (nSelRows >= m_MinClustersCorrections) {
              double RetVal;
              double RetValError;
              double RetRMS;
              RetVal = 0;
              RetValError = 0;
              RetRMS = 0;
              if (CalculateCorrection(2, nSelRows, fEEU, &RetVal, &RetValError, &RetRMS)) {
                TCorrection_ErrorEstimationMap[make_tuple(i_shape, i_pk, 0, i_angleU, i_angleV)] = RetRMS;
                SummariesInfo[6]++;
                SummariesInfo[8] += nSelRows;
                if (m_DoExpertHistograms) {
                  m_histErrorEstimationU[i_pk * m_shapes + i_shape]->SetBinContent(i_angleU + 1, i_angleV + 1, RetRMS);
                  m_histErrorEstimationU[m_pixelkinds * m_shapes]->SetBinContent(i_angleU + 1, i_angleV + 1,
                      m_histErrorEstimationU[m_pixelkinds * m_shapes]->GetBinContent(i_angleU + 1, i_angleV + 1) + 1);
                }
              }
              if (CalculateCorrection(2, nSelRows, fEEV, &RetVal, &RetValError, &RetRMS)) {
                TCorrection_ErrorEstimationMap[make_tuple(i_shape, i_pk, 1, i_angleU, i_angleV)] = RetRMS;
                SummariesInfo[7]++;
                SummariesInfo[9] += nSelRows;
                if (m_DoExpertHistograms) {
                  m_histErrorEstimationV[i_pk * m_shapes + i_shape]->SetBinContent(i_angleU + 1, i_angleV + 1, RetRMS);
                  m_histErrorEstimationV[m_pixelkinds * m_shapes]->SetBinContent(i_angleU + 1, i_angleV + 1,
                      m_histErrorEstimationV[m_pixelkinds * m_shapes]->GetBinContent(i_angleU + 1, i_angleV + 1) + 1);
                }
              }
            }
          }
        }
      }
    }
  */

  B2DEBUG(30, "--> error estimation correction calculation done. ");

//  auto key = make_tuple(1, 1, 0, 1, 0);
//  double& data = TCorrection_BiasMap[key];
//  cout << "Test - Old: table " << data << ", vector" << Correction_Bias->GetMatrixArray()[3] << endl;

  Table2Vector(TCorrection_BiasMap, Correction_Bias);
  Table2Vector(TCorrection_ErrorEstimationMap, Correction_ErrorEstimation);
  Table2Vector(TCorrection_BiasMapErr, Correction_BiasErr);
  Table2Vector(TInPixelPositionMap, InPixelPosition);
//  Vector2Table(Correction_Bias[0], TCorrection_BiasMap);
//  Vector2Table(Correction_ErrorEstimation[0], TCorrection_ErrorEstimationMap);
//  Vector2Table(InPixelPosition[0], TInPixelPositionMap);

//  cout << "Test - New: table " << data << " or " << TCorrection_BiasMap[key] << ", vector " << Correction_Bias->GetMatrixArray()[3]
//       << endl;

  B2DEBUG(30, "--> Save calibration to vectors done. ");

  if (m_DoExpertHistograms) {
    for (int i = 0; i < 20; i++) m_histSummariesInfo->SetBinContent(i + 1, SummariesInfo[i]);
    for (int i = 0; i < m_shapes; i++) m_histSummariesInfoSh->SetBinContent(i + 1, SummariesInfoSh[i]);
    for (int i = 0; i < m_pixelkinds; i++) m_histSummariesInfoPK->SetBinContent(i + 1, SummariesInfoPK[i]);

    for (int i_angleU = 0; i_angleU < m_anglesU; i_angleU++) {
      for (int i_angleV = 0; i_angleV < m_anglesV; i_angleV++) {
        m_histSummariesInfoAng->SetBinContent(i_angleU + 1, i_angleV + 1, SummariesInfoAng[i_angleU * m_anglesV + i_angleV]);
      }
    }
  }

  // ******************* Show some statistcs and save to asci file: ********************************
  TString TextSh;
  TextSh = Form("CorrectionsStatistics_CK%i_PK%i.log", m_CalibrationKind, m_PixelKind);
  FILE* AscFile = fopen(TextSh.Data(), "w");

  TextSh = Form("*******************************************************************");
  B2INFO(TextSh.Data()); fprintf(AscFile, "%s\n", TextSh.Data());
  TextSh = Form("**");
  B2INFO(TextSh.Data()); fprintf(AscFile, "%s\n", TextSh.Data());
  TextSh = Form("**            Using Clusters: %i", SummariesInfo[0]);
  B2INFO(TextSh.Data()); fprintf(AscFile, "%s\n", TextSh.Data());
  TextSh = Form("**                     Cases: %i", SummariesInfo[1]);
  B2INFO(TextSh.Data()); fprintf(AscFile, "%s\n", TextSh.Data());
  TextSh = Form("**");
  B2INFO(TextSh.Data()); fprintf(AscFile, "%s\n", TextSh.Data());
  TextSh = Form("**    Corrected bias U cases: %i, V cases: %i", SummariesInfo[2], SummariesInfo[3]);
  B2INFO(TextSh.Data()); fprintf(AscFile, "%s\n", TextSh.Data());
  TextSh = Form("**     Not corrected U cases: %i, V cases: %i", SummariesInfo[1] - SummariesInfo[2],
                SummariesInfo[1] - SummariesInfo[3]);
  B2INFO(TextSh.Data()); fprintf(AscFile, "%s\n", TextSh.Data());
  TextSh = Form("**       Clusters in U cases: %i, V cases: %i", SummariesInfo[4], SummariesInfo[5]);
  B2INFO(TextSh.Data()); fprintf(AscFile, "%s\n", TextSh.Data());
  TextSh = Form("** Fraction of clusters in U: %6.4f, in V: %6.4f", (float)SummariesInfo[4] / SummariesInfo[0],
                (float)SummariesInfo[5] / SummariesInfo[0]);
  B2INFO(TextSh.Data()); fprintf(AscFile, "%s\n", TextSh.Data());
  TextSh = Form("**");
  B2INFO(TextSh.Data()); fprintf(AscFile, "%s\n", TextSh.Data());
  TextSh = Form("**   Estimated Error U cases: %i, V cases: %i", SummariesInfo[6], SummariesInfo[7]);
  B2INFO(TextSh.Data()); fprintf(AscFile, "%s\n", TextSh.Data());
  TextSh = Form("**     Not corrected U cases: %i, V cases: %i", SummariesInfo[1] - SummariesInfo[6],
                SummariesInfo[1] - SummariesInfo[7]);
  B2INFO(TextSh.Data()); fprintf(AscFile, "%s\n", TextSh.Data());
  TextSh = Form("**       Clusters in U cases: %i, V cases: %i", SummariesInfo[8], SummariesInfo[9]);
  B2INFO(TextSh.Data()); fprintf(AscFile, "%s\n", TextSh.Data());
  TextSh = Form("** Fraction of clusters in U: %6.4f, in V: %6.4f", (float)SummariesInfo[8] / SummariesInfo[0],
                (float)SummariesInfo[9] / SummariesInfo[0]);
  B2INFO(TextSh.Data()); fprintf(AscFile, "%s\n", TextSh.Data());
  TextSh = Form("**");
  B2INFO(TextSh.Data()); fprintf(AscFile, "%s\n", TextSh.Data());
  B2INFO(TextSh.Data()); fprintf(AscFile, "%s\n", TextSh.Data());
  TextSh = Form("*******************************************************************");
  B2INFO(TextSh.Data()); fprintf(AscFile, "%s\n", TextSh.Data());
  TextSh = Form("**          Occupancy in Pixel Kinds");
  B2INFO(TextSh.Data()); fprintf(AscFile, "%s\n", TextSh.Data());
  TextSh = Form("**");
  B2INFO(TextSh.Data()); fprintf(AscFile, "%s\n", TextSh.Data());
  for (int i_pk = 0; i_pk < m_pixelkinds; i_pk++) {
    TextSh = Form("**      Pixel kind %i (Layer %i, Sensor %i, Size %i): %7i  (%6.2f %%)", i_pk,
                  (int)((i_pk % 4) / 2) + 1, (int)(i_pk / 4) + 1, i_pk % 2, SummariesInfoPK[i_pk],
                  (float)SummariesInfoPK[i_pk] * 100.0 / SummariesInfo[0]);
    B2INFO(TextSh.Data()); fprintf(AscFile, "%s\n", TextSh.Data());
  }
  TextSh = Form("**");
  B2INFO(TextSh.Data()); fprintf(AscFile, "%s\n", TextSh.Data());
  B2INFO(TextSh.Data()); fprintf(AscFile, "%s\n", TextSh.Data());
  TextSh = Form("*******************************************************************");
  B2INFO(TextSh.Data()); fprintf(AscFile, "%s\n", TextSh.Data());
  TextSh = Form("**          Occupancy in Shapes");
  B2INFO(TextSh.Data()); fprintf(AscFile, "%s\n", TextSh.Data());
  TextSh = Form("**");
  B2INFO(TextSh.Data()); fprintf(AscFile, "%s\n", TextSh.Data());
  for (int i_shape = 0; i_shape < m_shapes; i_shape++) {
    TextSh = Form("**      ShapeID %02i: %7i  (%6.2f %%)   (%s)", i_shape, SummariesInfoSh[i_shape],
                  (float)SummariesInfoSh[i_shape] * 100.0 / SummariesInfo[0],
                  Belle2::PXD::PXDClusterShape::pxdClusterShapeDescription[(Belle2::PXD::pxdClusterShapeType)(i_shape + 1)].c_str());
    B2INFO(TextSh.Data()); fprintf(AscFile, "%s\n", TextSh.Data());
  }
  TextSh = Form("**");
  B2INFO(TextSh.Data()); fprintf(AscFile, "%s\n", TextSh.Data());
  B2INFO(TextSh.Data()); fprintf(AscFile, "%s\n", TextSh.Data());
  TextSh = Form("*******************************************************************");
  B2INFO(TextSh.Data()); fprintf(AscFile, "%s\n", TextSh.Data());
  TextSh = Form("**          Occupancy in Angles");
  B2INFO(TextSh.Data()); fprintf(AscFile, "%s\n", TextSh.Data());
  TextSh = Form("**");
  B2INFO(TextSh.Data()); fprintf(AscFile, "%s\n", TextSh.Data());
  TextSh = Form("**    AngleV: ");
  B2INFO(TextSh.Data()); fprintf(AscFile, "%s\n", TextSh.Data());
  for (int i_angleV = 0; i_angleV < m_anglesV; i_angleV++) {
    TextSh = Form("%s     %02i: ", TextSh.Data(), i_angleV);
  }
  B2INFO(TextSh.Data()); fprintf(AscFile, "%s\n", TextSh.Data());
  for (int i_angleU = 0; i_angleU < m_anglesU; i_angleU++) {
    TextSh = Form("** AngleU %02i: ", i_angleU);
    for (int i_angleV = 0; i_angleV < m_anglesV; i_angleV++) {
      TextSh = Form("%s%7i, ", TextSh.Data(), SummariesInfoAng[i_angleU * m_anglesV + i_angleV]);
    }
    B2INFO(TextSh.Data()); fprintf(AscFile, "%s\n", TextSh.Data());
  }
  TextSh = Form("**");
  B2INFO(TextSh.Data()); fprintf(AscFile, "%s\n", TextSh.Data());
  B2INFO(TextSh.Data()); fprintf(AscFile, "%s\n", TextSh.Data());
  TextSh = Form("*******************************************************************");
  B2INFO(TextSh.Data()); fprintf(AscFile, "%s\n", TextSh.Data());
  TextSh = Form("**          Occupancy in Angles in %%");
  B2INFO(TextSh.Data()); fprintf(AscFile, "%s\n", TextSh.Data());
  TextSh = Form("**");
  B2INFO(TextSh.Data()); fprintf(AscFile, "%s\n", TextSh.Data());
  TextSh = Form("**    AngleV: ");
  B2INFO(TextSh.Data()); fprintf(AscFile, "%s\n", TextSh.Data());
  for (int i_angleV = 0; i_angleV < m_anglesV; i_angleV++) {
    TextSh = Form("%s     %02i: ", TextSh.Data(), i_angleV);
  }
  B2INFO(TextSh.Data()); fprintf(AscFile, "%s\n", TextSh.Data());
  for (int i_angleU = 0; i_angleU < m_anglesU; i_angleU++) {
    TextSh = Form("** AngleU %02i: ", i_angleU);
    for (int i_angleV = 0; i_angleV < m_anglesV; i_angleV++) {
      TextSh = Form("%s%7.2f, ", TextSh.Data(), (float)SummariesInfoAng[i_angleU * m_anglesV + i_angleV] * 100.0 / SummariesInfo[0]);
    }
    B2INFO(TextSh.Data()); fprintf(AscFile, "%s\n", TextSh.Data());
  }
  TextSh = Form("**");
  B2INFO(TextSh.Data()); fprintf(AscFile, "%s\n", TextSh.Data());
  B2INFO(TextSh.Data()); fprintf(AscFile, "%s\n", TextSh.Data());
  TextSh = Form("*******************************************************************");
  B2INFO(TextSh.Data()); fprintf(AscFile, "%s\n", TextSh.Data());
  fclose(AscFile);
  // ******************* END show some statistcs and save to asci file: ********************************


  if (m_DoExpertHistograms) {
    //  getObject<TTree>(name_SourceTree).Write(name_SourceTree);
    //  getObject<shape>("shape");
    //  getObject<TTree>("shape").Branch();
    fDQM->Write();
    fDQM->Close();
    fExpertHistograms->Write();
    fExpertHistograms->Close();

  }

  // Here to save corrections in TVectorT format, could change later
  // Use TFile for temporary use,
  // Final use with saveCalibration and database.
  TString nameBS = Form("PXDShCalibrationBasicSetting");
  TString nameB = Form("Correction_Bias");
  TString nameBE = Form("Correction_BiasErr");
  TString nameEE = Form("Correction_ErrorEstimation");
  TString nameIP = Form("InPixelPosition");
  TFile* f = new TFile(name_OutFileCalibrations.Data(), "recreate");
  PXDShCalibrationBasicSetting->Write(nameBS.Data());
  Correction_Bias->Write(nameB.Data());
  Correction_BiasErr->Write(nameBE.Data());
  Correction_ErrorEstimation->Write(nameEE.Data());
  InPixelPosition->Write(nameIP.Data());
  f->Close();

  saveCalibration(Correction_Bias, nameB.Data());
  saveCalibration(Correction_ErrorEstimation, nameEE.Data());
  saveCalibration(InPixelPosition, nameIP.Data());

  delete[] m_histSignal;
  delete[] m_histSeed;
  delete[] m_histTrackSigmaU;
  delete[] m_histTrackSigmaV;
  delete[] m_histTrackSigmaUV;

  delete[] m_histBiasCorrectionU;
  delete[] m_histBiasCorrectionV;
  delete[] m_histResidualRMSU;
  delete[] m_histResidualRMSV;
  delete[] m_histBiasCorrectionErrorU;
  delete[] m_histBiasCorrectionErrorV;
  delete[] m_histErrorEstimationU;
  delete[] m_histErrorEstimationV;
  delete[] m_histnClusters;
  delete[] m_histnClusterFraction;

  delete[] ValueCors;
  delete[] ValueInPix;
//  delete[] fEEU;
//  delete[] fEEV;
  return c_OK;
}

void PXDClusterShapeCalibrationAlgorithm::Table2Vector(map_typeCorrs& TCorrection, TVectorD* CorVector)
{
  Double_t* ValueCors;
  ValueCors = new Double_t[m_shapes * m_pixelkinds * m_dimensions * m_anglesU * m_anglesV];
  for (int i_shape = 0; i_shape < m_shapes; i_shape++)
    for (int i_pk = 0; i_pk < m_pixelkinds; i_pk++)
      for (int i_axis = 0; i_axis < m_dimensions; i_axis++)
        for (int i_angleU = 0; i_angleU < m_anglesU; i_angleU++)
          for (int i_angleV = 0; i_angleV < m_anglesV; i_angleV++) {
            double data = TCorrection[make_tuple(i_shape, i_pk, i_axis, i_angleU, i_angleV)];
            int i_vector = i_shape * m_pixelkinds * m_dimensions * m_anglesU * m_anglesV;
            i_vector += i_pk * m_dimensions * m_anglesU * m_anglesV;
            i_vector += i_axis * m_anglesU * m_anglesV;
            i_vector += i_angleU * m_anglesV;
            i_vector += i_angleV;
            ValueCors[i_vector] = data;
          }
  CorVector->SetElements(ValueCors);

//  B2DEBUG(30, "--> Inside Table2Vector " << TCorrection[1][0][0][0][8]);
//  B2DEBUG(30, "--> Inside Table2Vector " << CorVector->GetMatrixArray()[3]);

//  delete[] ValueCors;

}

void PXDClusterShapeCalibrationAlgorithm::Table2Vector(map_typeInPics& TInPix, TVectorD* InPixVector)
{
  Double_t* ValueInPix = new Double_t[m_shapes * m_pixelkinds * m_anglesU * m_anglesV * m_in_pixelU * m_in_pixelV];
  for (int i_shape = 0; i_shape < m_shapes; i_shape++)
    for (int i_pk = 0; i_pk < m_pixelkinds; i_pk++)
      for (int i_angleU = 0; i_angleU < m_anglesU; i_angleU++)
        for (int i_angleV = 0; i_angleV < m_anglesV; i_angleV++)
          for (int i_ipU = 0; i_ipU < m_in_pixelU; i_ipU++)
            for (int i_ipV = 0; i_ipV < m_in_pixelV; i_ipV++) {
              auto key = make_tuple(i_shape, i_pk, i_angleU, i_angleV, m_in_pixelU, m_in_pixelV);
              auto& data = TInPix[key];
              int i_vector = i_shape * m_pixelkinds * m_anglesU * m_anglesV * m_in_pixelU * m_in_pixelV;
              i_vector += i_pk * m_anglesU * m_anglesV * m_in_pixelU * m_in_pixelV;
              i_vector += i_angleU * m_anglesV * m_in_pixelU * m_in_pixelV;
              i_vector += i_angleV * m_in_pixelU * m_in_pixelV;
              i_vector += i_ipU * m_in_pixelV;
              i_vector += i_ipV;
              ValueInPix[i_vector] = data;
            }
  InPixVector->SetElements(ValueInPix);
  delete[] ValueInPix;

}

void PXDClusterShapeCalibrationAlgorithm::Vector2Table(TVectorD* CorVector, map_typeCorrs& TCorrection)
{

  for (int i_shape = 0; i_shape < m_shapes; i_shape++)
    for (int i_pk = 0; i_pk < m_pixelkinds; i_pk++)
      for (int i_axis = 0; i_axis < m_dimensions; i_axis++)
        for (int i_angleU = 0; i_angleU < m_anglesU; i_angleU++)
          for (int i_angleV = 0; i_angleV < m_anglesV; i_angleV++) {
            auto key = make_tuple(i_shape, i_pk, i_axis, i_angleU, i_angleV);
            auto& data = TCorrection[key];
            int i_vector = i_shape * m_pixelkinds * m_dimensions * m_anglesU * m_anglesV;
            i_vector += i_pk * m_dimensions * m_anglesU * m_anglesV;
            i_vector += i_axis * m_anglesU * m_anglesV;
            i_vector += i_angleU * m_anglesV;
            i_vector += i_angleV;
            data = CorVector->GetMatrixArray()[i_vector];
          }

  B2DEBUG(30, "--> Inside Vector2Table " << CorVector->GetMatrixArray()[3]);
}

void PXDClusterShapeCalibrationAlgorithm::Vector2Table(TVectorD* InPixVector, map_typeInPics& TInPix)
{

  for (int i_shape = 0; i_shape < m_shapes; i_shape++)
    for (int i_pk = 0; i_pk < m_pixelkinds; i_pk++)
      for (int i_angleU = 0; i_angleU < m_anglesU; i_angleU++)
        for (int i_angleV = 0; i_angleV < m_anglesV; i_angleV++)
          for (int i_ipU = 0; i_ipU < m_in_pixelU; i_ipU++)
            for (int i_ipV = 0; i_ipV < m_in_pixelV; i_ipV++) {
              auto key = make_tuple(i_shape, i_pk, i_angleU, i_angleV, m_in_pixelU, m_in_pixelV);
              auto& data = TInPix[key];
              int i_vector = i_shape * m_pixelkinds * m_anglesU * m_anglesV * m_in_pixelU * m_in_pixelV;
              i_vector += i_pk * m_anglesU * m_anglesV * m_in_pixelU * m_in_pixelV;
              i_vector += i_angleU * m_anglesV * m_in_pixelU * m_in_pixelV;
              i_vector += i_angleV * m_in_pixelU * m_in_pixelV;
              i_vector += i_ipU * m_in_pixelV;
              i_vector += i_ipV;
              data = InPixVector->GetMatrixArray()[i_vector];
            }

  B2DEBUG(30, "--> Inside Vector2Table " << InPixVector->GetMatrixArray()[3]);
}

int PXDClusterShapeCalibrationAlgorithm::getPhiIndex(float phi)
{
  int ind = (int)(phi + TMath::Pi() / 2.0) / (TMath::Pi() / m_anglesU);
  return ind;
}

int PXDClusterShapeCalibrationAlgorithm::getThetaIndex(float theta)
{
  int ind = (int)(theta + TMath::Pi() / 2.0) / (TMath::Pi() / m_anglesV);
  return ind;
}

int PXDClusterShapeCalibrationAlgorithm::CalculateCorrection(int CorCase, int n, std::vector<double> array, double* val,
    double* valError, double* rms)
{
  // CorCase: correction case for:
  //   1: Correction_Bias:            MinSamples: m_MinClustersCorrections,  preset = 0, minCorrection = 0.5 micron, MinDistanceInErrors = 3
  //   2: Correction_ErrorEstimation  MinSamples: m_MinClustersCorrections,  preset = 1, minCorrection = 0.05,       MinDistanceInErrors = 3

  int ret = 1;

  int MinSamples;
  double preset;
  double minCorrection;
  double MinDistanceInErrors;
  double* array2 = new double[n];
  for (int i = 0; i < n; i++)   // TODO it correct
    array2[i] = array[i];
  MinSamples = m_MinClustersCorrections;
  MinDistanceInErrors = 3.0;
  if (CorCase == 1) {
    preset = 0.0;
    minCorrection = 0.5 * Unit::um;
  } else if (CorCase == 2) {
    preset = 1.0;
    minCorrection = 0.05;
  } else {
    delete [] array2;
    return 0;
  }

  *val = preset;
  *valError = 0.0;
  *rms = 1;
//printf("----->n %i MinSamples %i\n",n,MinSamples);

  if (n < MinSamples) {
    delete [] array2;
    return 0;
  }

  float QuantCut = 0.9; // TODO this parameter can be change...
  double quantiles[2];
  double prob[2];
  int nprob = 2;
  Bool_t isSorted = kFALSE;
  prob[0] = (1. - QuantCut) / 2.;
  prob[1] = 1.0 - prob[0];
  TMath::Quantiles(n, nprob, array2, quantiles, prob, isSorted, 0, 7);
  for (int iD = 0; iD < n; iD++) {
    if ((array2[iD] < quantiles[0]) || (array2[iD] > quantiles[1])) {
      for (int iD2 = iD; iD2 < n - 1; iD2++) {
        array2[iD2] = array2[iD2 + 1];
      }
      iD--;
      n--;
    }
  }
  // TODO method of bootstrap resampling is best one, for medium time we use simplification

  double DevNew = TMath::Mean(n, array2);
  double ErrNew = TMath::RMS(n, array2);
  ErrNew /= 0.79; // Correction becouse we cut out 10% of
  double SEM = ErrNew / sqrt(n);

  // condition for using correction:
  if (CorCase == 1) {
    if (fabs(DevNew) < minCorrection) ret = 0;
    if (CorCase == 1) if (fabs(DevNew) < (MinDistanceInErrors * SEM)) ret = 0;  // apply only for bias
  }
  if (CorCase == 2) {
    if (fabs(ErrNew - 1) < minCorrection) ret = 0;
  }
//  if (CorCase == 1) printf("----->dev %f err %f sem %f \n", DevNew / Unit::um, ErrNew / Unit::um, SEM / Unit::um);
//  else printf("----->dev %f err %f sem %f \n", DevNew, ErrNew, SEM);
  *val = DevNew;
  *valError = SEM;
  *rms = ErrNew;

  return ret;
}

