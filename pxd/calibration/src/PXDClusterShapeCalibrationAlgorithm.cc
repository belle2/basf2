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
  setDescription("Calibration of position/estimated error corrections based on cluster shape in PXD. "
                 "Basic steps for using of corrections are: "
                 " - pxdClusterShapeCalibration - create source for correction calculation "
                 " - PXDClusterShapeCalibrationAlgorithm - calculate corrections <--- this module "
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
                 "Detail description of precedure is follows: "
                 " "
                 "Cluster will be check if: "
                 "  is not close borders, otherway no corrections "
                 "  for giving kind of pixel type, u and v angle direction: "
                 "    is in in-pixel hitmap for giving cluster shape and giving angle, otherway if hitmap is fully empty: "
                 "       do those steps for full range simulation in-pixel hitmap "
                 "Correction is apply in HitCorrector: "
                 "  look for bias and estimated error in 4 most closes points in calibration tables "
                 "  apply correction with weight dependent of distances to table points "
                 "Tables for every pixel pitch type (8 types for PXD, 4 sensors x 2 pitch on each): "
                 "  2 sets of tables: - for selection of angles and shapes for real data "
                 "                    - for full range of angles and shapes from simulations "
                 "  in every set of tables are: "
                 "      - in-pixel 2D hitmaps for angles and shapes "
                 "      - u+v corrections of bias for angles and shapes "
                 "      - u+v error estimations for angles and shapes "
                 "We expect u+v directions, 8 kinds of pixels, 18 x 18 angles, 15 shapes = 77760 cases "
                 "  for Bias correction "
                 "  for Error estimation (EstimError/Residual) "
                 "we expect 8 kinds of pixels, 18 x 18 angles, 15 shapes = 38880 cases "
                 "  for in-pixel positions 2D maps, each map has 9x9 bins with binary value (0/1) "
                 "For ~100 points per histogram we expect 2 millions events (1 event = 2x sensors, u+v coordinates) "
                 "For special histograms of simulations of only one pixelkind and full range of angles: 10 millions events "
                 "We expect set of histograms mostly empty so no need correction for giving parameters "
                 "Finaly we store for 3 sets of tables: "
                 "  - for Bias correction:      77760 short values "
                 "  - for Error estimation:     77760 short values "
                 "  - for in-pixel positions: 3149280 binary values "
                 "I advance there is also: "
                 " table of bias corrections errors "
                 " table of error estimation covariance matrix non diagonal element "
                 " "
                 "Storring in database will be in TVectorD format "
                 "Using will be on boost/multi_array format in unordered map and hash table "
                 " "
                 "For calibration need to have: InPix, Residual, Sigma "
                 " "
                 "There is general question if we use in-pixel information for selections: "
                 "   - from true track there is significant and useful information "
                 "   - from tracking information has spread in level sigma 9 microns (preliminary) "
                 "  in-pixel filter will be prepared but not use. "
                 "In current analysis only true information is use, track information analysis is ongoing. "
                 " "
                 "***** DQM AND EXPERT HISTOGRAMS:  ***** "
                 " "
                 "All histograms are optional only for request. "
                 " "
                 "Histograms: m_shapes * m_pixelkinds = 15 x 8 = 120 u+v = 240 (1D) "
                 "Binning: angleU x angleV (18 x 18) "
                 "Monitoring values:      BiasCorrection U + V (residual mean) "
                 "                        ResidualRMS U + V (residual RMS) "
                 "                        BiasCorrectionError U + V (error of residual mean) "
                 "                        ErrorEstimation U + V (RMS of (residual / estimated error)) "
                 "                        nClusters, nClusterFraction "
                 " "
                 "Special histograms case 2):     2D SigmaUTrack + SigmaVTrack "
                 "                                2D phiTrack - phiTrue + thetaTrack - thetaTrue "
                 "                                2D InPixU - InPixUTrue + InPixV - InPixVTrue "
                 "                                2D ResidUTrack - ResidUTrue + ResidVTrack - ResidVTrue "
                 " "
                 "Correction creation: Bias: "
                 "We use Unbiased Residuals: Sigma(UnbiasResid)^2 = Sigma(Cluster)^2 + Sigma(Track)^2 "
                 "Sigma(UnbiasResid)^2 = Sigma(Cluster)^2 + Sigma(Track)^2 --> "
                 "S(UBR)^2 = S(C)^2 + S(T)^2 --> "
                 "we measure S(UBR) and mean of ratio UBR/UBEE (M(UBR/UBEE) "
                 "(S(UBR)*Cor(S(R/EE))^2 = (S(C)*Cor(S(C)))^2 + S(T)^2 "
                 " "
                 "Correction creation: Error Estimation (EE): "
                 "From plot (Residual/ErrorEstimation) we obtain Sigma, than apply Cor(R/EE) "
                 "CorSig(R/EE) --> CorSigTrue(R/EE) using function SigmaREEToTrueValue "
                 "for obtaining best realistic correction. Than equation: "
                 " "
                 "For corrections could to use two types of hit residuals and errors, where the hit included "
                 "in the track fit, and without the hit included, we call: "
                 "correlated or biassed, and uncorrelated or unbiassed track. "
                 "Next kind of sources in case of simulation is true position of cluster/recoHit. "
                 "Use information about residuals and estimated errors for recohit, track and "
                 "full residual on plane. Used theory: "
                 "CBX 96-20, Robert Kutschke, Anders Ryd, September 02 1996, Billoir Fitter for CLEO II "
                 "Chapter 8 Residuals: "
                 "for unbiassed tracks residual variance: "
                 "  Sigma(residual_unbias)^2 = Sigma(cluster)^2 + Sigma(track_unbias)^2  (eq. 108) "
                 "for biassed tracks residual variance: "
                 "  Sigma(residual_bias)^2 = Sigma(cluster)^2 - Sigma(track_bias)^2  (eq. 112) "
                 "We collect for corrections as much as possible those information. "
                 "More stable and better understand is unbias information so we recommend use it. "
                 "For biassed data the return value of sigma(residual_bias) contain nondiagonal nonzero element. "
                 "There is not clear how to work with this situation for biased tracks. "
                 " "
                 "Default return values of recoTrack are biassed! "
                 "so need use getGenfitTrack(recoTrack) to obtain biased and unbiassed values in local u,v plane coordinates. "
                 "recoTrack contain information of covariance matrix (error estimation) in form: "
                 "getMeasuredStateOnPlaneFromRecoHit.getPosMomCov(pos, mom, cov), "
                 "cov(0,0) --> Sigma(BiasTrack)^2 = Sigma(BiasResidual)^2 - Sigma(Cluster)^2 "
                 "which is right for unbias fit, but not for biased fit. "
                 "For biassed fit right calculation should be: "
                 "Sigma(BiasTrack)^2 = Sigma(Cluster)^2 - Sigma(BiasResidual)^2, "
                 "where: Sigma(BiasTrack) < Sigma(Cluster), Sigma(BiasResidual) < Sigma(Cluster) [eq.2] "
                 "In case conditions are not true there is some missunderstanding of return values from covariant matrices. "
                 " "
                 "Solution is to obtain biased or unbiased information from genfitTrack: "
                 "getGenfitTrack(recoTrack) "
                 "TVectorD res = genfitTrack.getPointWithMeasurement(iHit)->getFitterInfo()->getResidual(0, biased).getState() "
                 "TMatrixDSym cov = genfitTrack.getPointWithMeasurement(iHit)->getFitterInfo()->getResidual(0, biased).getCov() "
                 "where biased sets which kind of track is use. Than: "
                 "covBias(i, i) = Sigma(BiasTrack)^2 "
                 "covUnbias(i, i) = Sigma(UnbiasTrack)^2 "
                 "biassed solution: "
                 "SigmaResidualBias^2 = cluster->getSigma()^2 - covBias(i, i); "
                 "unbiassed solution: "
                 "SigmaResidualUnbias^2 = cluster->getSigma()^2 + covUnbias(i, i); "
                 "and finally true solution: "
                 "sigma of distribution of residuals: ( residual/cluster->getSigma() ). "
                 "Calculated corrections will be obtain from unbiased tracks and confirm from true solution. "
                 " "
                 "Final correction: residual/cluster->getSigma() with cluster->getSigma() variate arround residual sigma "
                 "show that sigma of ( residual/cluster->getSigma() ) is slightly out of 1.0 so further corrrection is need. "
                 "This correction is hardcoded from simulation and is use in module with caclulation of correction."
                 " "
                 "R is residual of reconstructed cluster (= measurement = cluster) and true position ( = recoTrack position). "
                 "EE is preset Error Estimation of cluster. "
                 "TrueEE is Error Estimation of cluster equal of distribution of Residuals = TruePosition - MeasuredPosition "
                 "In case of residuals including Tracking error it is need to corigate using equotion for unbiased tracks: "
                 "Sigma(residuals)^2 = Sigma(clusters)^2 + Sigma(tracks)^2 (+ Sigma(MultipleScattering)^2 >> 0) "
                 " "
                 "For EE can be used distribution of ratio R/EE, where we can expect sigma of this distribution -> 1.0 in case of TrueEE. "
                 "From simulation of Sigma(R/EE), where EE < 1.2 * TrueEE, we know there is need some correction to obtain EE -> TrueEE: "
                 "TrueEE = EE * Sig(R/EE) / Cor(1/Sig(R/EE)) "
                 "for 1/Sig(R/EE) >= 1.2 (Sig(R/EE) < 0.832)(EE > EETrue): Cor(..) = ~1, TrueEE = EE * Sig(R/EE) "
                 "Good aproximation of Correction is: "
                 "z = (1 / Sig(R/EE) - a) / b "
                 "e = exp(-z) "
                 "Cor(1/Sig(R/EE)) = A * e / (1 + e) "
                 "where: a = 0.644261279, b = -0.065866687, A = 1.021089491 "
                 "valid for 1/Sig(R/EE) in range 0.7 .. 1.2 (mean(TrueEE)/sig(R) in range 0.5 .. 2.2) "
                 "and giving result within +- 5 % independent of shape of EE distribution. "
                 "From simulation also we expect at least 100 samples for correction. "
                 "Robustness of calculation is usinf cut of +-5% of hits and than finil RMS correct for this."
                 "For this real acceptance condition is 110 samples. "
                );

}

Belle2::CalibrationAlgorithm::EResult PXDClusterShapeCalibrationAlgorithm::calibrate()
{

  TString name_Case;
  TString name_SourceTree;
  TString name_OutFileCalibrations;
  name_Case = Form("_RealData%i_Pixel%i_Type%i",
                   (int)m_UseRealData, m_UsePixelKind, m_StoreEECorrectionType);
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

  TH2F** m_histnClusters = NULL;
  TH2F** m_histnClusterFraction = NULL;

  TH2F** m_histBiasCorrectionU = NULL;
  TH2F** m_histBiasCorrectionV = NULL;
  TH2F** m_histResidualRMSU = NULL;
  TH2F** m_histResidualRMSV = NULL;
  TH2F** m_histBiasCorrectionErrorU = NULL;
  TH2F** m_histBiasCorrectionErrorV = NULL;

  TH2F** m_histCorrEE_TrueU = NULL;
  TH2F** m_histCorrEE_TrueV = NULL;
  TH2F** m_histCorrEE_TrueTrackU = NULL;
  TH2F** m_histCorrEE_TrueTrackV = NULL;
  TH2F** m_histCorrEE_RealTrackU = NULL;
  TH2F** m_histCorrEE_RealTrackV = NULL;
  TH2F** m_histResRMSEEU = NULL;
  TH2F** m_histResRMSEEV = NULL;
  TH2F** m_histDiffClEETrueOrigEEU = NULL;
  TH2F** m_histDiffClEETrueOrigEEV = NULL;
  TH2F** m_histDiffClEETrueCalcTTEEU = NULL;
  TH2F** m_histDiffClEETrueCalcTTEEV = NULL;
  TH2F** m_histDiffClEETrueCalcRTEEU = NULL;
  TH2F** m_histDiffClEETrueCalcRTEEV = NULL;
  TH2F** m_histClusterTrueEEU = NULL;
  TH2F** m_histClusterTrueEEV = NULL;
  TH2F** m_histClusterOrigEEU = NULL;
  TH2F** m_histClusterOrigEEV = NULL;
  TH2F** m_histClustCalcTTEEU = NULL;
  TH2F** m_histClustCalcTTEEV = NULL;
  TH2F** m_histClustCalcRTEEU = NULL;
  TH2F** m_histClustCalcRTEEV = NULL;

  TH1F** m_histSignal = NULL;
  TH1F** m_histSeed = NULL;
  TH2F** m_histResidTruePlaneUnBiasUV = NULL;
  TH2F** m_histSigmaTrackUnBiasUV = NULL;
  TH2F** m_histResidPlaneRHUnBiasUV = NULL;
  TH2F** m_histSigmaRecoUV = NULL;
  TH2F** m_histResidTrueRHUV = NULL;

  m_histResidPlaneRHUnBiasUV = (TH2F**) new TH2F*[m_anglesU * m_anglesV + 1];
  m_histSigmaRecoUV = (TH2F**) new TH2F*[m_anglesU * m_anglesV + 1];
  m_histResidTrueRHUV = (TH2F**) new TH2F*[m_anglesU * m_anglesV + 1];

  m_histResidTruePlaneUnBiasUV = (TH2F**) new TH2F*[m_anglesU * m_anglesV + 1];
  m_histSigmaTrackUnBiasUV = (TH2F**) new TH2F*[m_anglesU * m_anglesV + 1];
  m_histSignal = (TH1F**) new TH1F*[m_shapes + 1];
  m_histSeed = (TH1F**) new TH1F*[m_shapes + 1];
  // Next expert histos is done for only one pixelkind and cluster shape, preset is:
  int TestPK = 2;
  int TestCS = 10;
  for (int i = 0; i < m_anglesU * m_anglesV + 1; i++) {
    m_histResidPlaneRHUnBiasUV[i] = NULL;
    m_histSigmaRecoUV[i] = NULL;
    m_histResidTrueRHUV[i] = NULL;
  }
  for (int i_angle = 0; i_angle < m_anglesU * m_anglesV + 1; i_angle++) {
    m_histResidTruePlaneUnBiasUV[i_angle] = NULL;
    m_histSigmaTrackUnBiasUV[i_angle] = NULL;
  }
  for (int i_shape = 0; i_shape < m_shapes + 1; i_shape++) {
    m_histSignal[i_shape] = NULL;
    m_histSeed[i_shape] = NULL;
  }

  m_histCorrEE_TrueU = (TH2F**) new TH2F*[m_pixelkinds * m_shapes + 1];
  m_histCorrEE_TrueV = (TH2F**) new TH2F*[m_pixelkinds * m_shapes + 1];
  m_histCorrEE_TrueTrackU = (TH2F**) new TH2F*[m_pixelkinds * m_shapes + 1];
  m_histCorrEE_TrueTrackV = (TH2F**) new TH2F*[m_pixelkinds * m_shapes + 1];
  m_histCorrEE_RealTrackU = (TH2F**) new TH2F*[m_pixelkinds * m_shapes + 1];
  m_histCorrEE_RealTrackV = (TH2F**) new TH2F*[m_pixelkinds * m_shapes + 1];
  m_histResRMSEEU = (TH2F**) new TH2F*[m_pixelkinds * m_shapes + 1];
  m_histResRMSEEV = (TH2F**) new TH2F*[m_pixelkinds * m_shapes + 1];
  m_histDiffClEETrueOrigEEU = (TH2F**) new TH2F*[m_pixelkinds * m_shapes + 1];
  m_histDiffClEETrueOrigEEV = (TH2F**) new TH2F*[m_pixelkinds * m_shapes + 1];
  m_histDiffClEETrueCalcTTEEU = (TH2F**) new TH2F*[m_pixelkinds * m_shapes + 1];
  m_histDiffClEETrueCalcTTEEV = (TH2F**) new TH2F*[m_pixelkinds * m_shapes + 1];
  m_histDiffClEETrueCalcRTEEU = (TH2F**) new TH2F*[m_pixelkinds * m_shapes + 1];
  m_histDiffClEETrueCalcRTEEV = (TH2F**) new TH2F*[m_pixelkinds * m_shapes + 1];
  m_histClusterTrueEEU = (TH2F**) new TH2F*[m_pixelkinds * m_shapes + 1];
  m_histClusterTrueEEV = (TH2F**) new TH2F*[m_pixelkinds * m_shapes + 1];
  m_histClusterOrigEEU = (TH2F**) new TH2F*[m_pixelkinds * m_shapes + 1];
  m_histClusterOrigEEV = (TH2F**) new TH2F*[m_pixelkinds * m_shapes + 1];
  m_histClustCalcTTEEU = (TH2F**) new TH2F*[m_pixelkinds * m_shapes + 1];
  m_histClustCalcTTEEV = (TH2F**) new TH2F*[m_pixelkinds * m_shapes + 1];
  m_histClustCalcRTEEU = (TH2F**) new TH2F*[m_pixelkinds * m_shapes + 1];
  m_histClustCalcRTEEV = (TH2F**) new TH2F*[m_pixelkinds * m_shapes + 1];

  m_histBiasCorrectionU = (TH2F**) new TH2F*[m_pixelkinds * m_shapes + 1];
  m_histBiasCorrectionV = (TH2F**) new TH2F*[m_pixelkinds * m_shapes + 1];
  m_histResidualRMSU = (TH2F**) new TH2F*[m_pixelkinds * m_shapes + 1];
  m_histResidualRMSV = (TH2F**) new TH2F*[m_pixelkinds * m_shapes + 1];
  m_histBiasCorrectionErrorU = (TH2F**) new TH2F*[m_pixelkinds * m_shapes + 1];
  m_histBiasCorrectionErrorV = (TH2F**) new TH2F*[m_pixelkinds * m_shapes + 1];
  m_histnClusters = (TH2F**) new TH2F*[m_pixelkinds * m_shapes + 1];
  m_histnClusterFraction = (TH2F**) new TH2F*[m_pixelkinds * m_shapes + 1];

  TH2F* m_histBiasCounterU = NULL;
  TH2F* m_histBiasCounterV = NULL;
  TH2F* m_histEECounterU = NULL;
  TH2F* m_histEECounterV = NULL;
  for (int i_pk = 0; i_pk < m_pixelkinds; i_pk++) {
    for (int i_shape = 0; i_shape < m_shapes + 1; i_shape++) {
      m_histBiasCorrectionU[i_pk * m_shapes + i_shape] = NULL;
      m_histBiasCorrectionV[i_pk * m_shapes + i_shape] = NULL;
      m_histResidualRMSU[i_pk * m_shapes + i_shape] = NULL;
      m_histResidualRMSV[i_pk * m_shapes + i_shape] = NULL;
      m_histBiasCorrectionErrorU[i_pk * m_shapes + i_shape] = NULL;
      m_histBiasCorrectionErrorV[i_pk * m_shapes + i_shape] = NULL;
      m_histnClusters[i_pk * m_shapes + i_shape] = NULL;
      m_histnClusterFraction[i_pk * m_shapes + i_shape] = NULL;

      m_histCorrEE_TrueU[i_pk * m_shapes + i_shape] = NULL;
      m_histCorrEE_TrueV[i_pk * m_shapes + i_shape] = NULL;
      m_histCorrEE_TrueTrackU[i_pk * m_shapes + i_shape] = NULL;
      m_histCorrEE_TrueTrackV[i_pk * m_shapes + i_shape] = NULL;
      m_histCorrEE_RealTrackU[i_pk * m_shapes + i_shape] = NULL;
      m_histCorrEE_RealTrackV[i_pk * m_shapes + i_shape] = NULL;
      m_histResRMSEEU[i_pk * m_shapes + i_shape] = NULL;
      m_histResRMSEEV[i_pk * m_shapes + i_shape] = NULL;
      m_histDiffClEETrueOrigEEU[i_pk * m_shapes + i_shape] = NULL;
      m_histDiffClEETrueOrigEEV[i_pk * m_shapes + i_shape] = NULL;
      m_histDiffClEETrueCalcTTEEU[i_pk * m_shapes + i_shape] = NULL;
      m_histDiffClEETrueCalcTTEEV[i_pk * m_shapes + i_shape] = NULL;
      m_histDiffClEETrueCalcRTEEU[i_pk * m_shapes + i_shape] = NULL;
      m_histDiffClEETrueCalcRTEEV[i_pk * m_shapes + i_shape] = NULL;
      m_histClusterTrueEEU[i_pk * m_shapes + i_shape] = NULL;
      m_histClusterTrueEEV[i_pk * m_shapes + i_shape] = NULL;
      m_histClusterOrigEEU[i_pk * m_shapes + i_shape] = NULL;
      m_histClusterOrigEEV[i_pk * m_shapes + i_shape] = NULL;
      m_histClustCalcTTEEU[i_pk * m_shapes + i_shape] = NULL;
      m_histClustCalcTTEEV[i_pk * m_shapes + i_shape] = NULL;
      m_histClustCalcRTEEU[i_pk * m_shapes + i_shape] = NULL;
      m_histClustCalcRTEEV[i_pk * m_shapes + i_shape] = NULL;

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
    int iDetailBining = 100;
    m_histResidTruePlaneUnBiasUV[m_anglesU * m_anglesV] = new TH2F("ResidTruePlaneUnBias", "Residuals of True and UnBias Track",
        iDetailBining, -0.5 * iDetailBining, 0.5 * iDetailBining, iDetailBining, -0.5 * iDetailBining, 0.5 * iDetailBining);
    m_histSigmaTrackUnBiasUV[m_anglesU * m_anglesV] = new TH2F("SigmaTrackUnBias", "Sigma of UnBias Track",
                                                               iDetailBining, 0, iDetailBining, iDetailBining, 0, iDetailBining);
    TString StrAdd = Form("PK%iCS%i", TestPK, TestCS);
    TString HistoName0 = Form("ResidPlaneRHUnBias_%s", StrAdd.Data());
    TString HistoTitle0 = Form("Resid of RecoHit in Plane and UnBias Track, %s", StrAdd.Data());
    m_histResidPlaneRHUnBiasUV[m_anglesU * m_anglesV] = new TH2F(HistoName0.Data(), HistoTitle0.Data(),
        iDetailBining, -0.5 * iDetailBining, 0.5 * iDetailBining, iDetailBining, -0.5 * iDetailBining, 0.5 * iDetailBining);
    HistoName0 = Form("SigmaReco_%s", StrAdd.Data());
    HistoTitle0 = Form("Sigma of RecoHit, %s", StrAdd.Data());
    m_histSigmaRecoUV[m_anglesU * m_anglesV] = new TH2F(HistoName0.Data(), HistoTitle0.Data(),
                                                        iDetailBining, 0, iDetailBining, iDetailBining, 0, iDetailBining);
    HistoName0 = Form("ResidTrueRH_%s", StrAdd.Data());
    HistoTitle0 = Form("Residuals of True and RecoHit, %s", StrAdd.Data());
    m_histResidTrueRHUV[m_anglesU * m_anglesV] = new TH2F(HistoName0.Data(), HistoTitle0.Data(),
                                                          iDetailBining, -0.5 * iDetailBining, 0.5 * iDetailBining, iDetailBining, -0.5 * iDetailBining, 0.5 * iDetailBining);

    fDQM->mkdir("Tracks");
    fDQM->cd("Tracks");
    for (int i_angleU = 0; i_angleU < m_anglesU; i_angleU++) {
      for (int i_angleV = 0; i_angleV < m_anglesV; i_angleV++) {
        TString HistoName = Form("ResidTruePlaneUnBias_AngU%02i_AngV%02i", i_angleU, i_angleV);
        TString HistoTitle = Form("Residuals of True and UnBias Track, angle: U %02i, V %02i", i_angleU, i_angleV);
        m_histResidTruePlaneUnBiasUV[i_angleU * m_anglesV + i_angleV] = new TH2F(HistoName.Data(), HistoTitle.Data(),
            iDetailBining, -0.5 * iDetailBining, 0.5 * iDetailBining, iDetailBining, -0.5 * iDetailBining, 0.5 * iDetailBining);
        HistoName = Form("SigmaTrackUnBias_AngU%02i_AngV%02i", i_angleU, i_angleV);
        HistoTitle = Form("Sigma of UnBias Track, angle: U %02i, V %02i", i_angleU, i_angleV);
        m_histSigmaTrackUnBiasUV[i_angleU * m_anglesV + i_angleV] = new TH2F(HistoName.Data(), HistoTitle.Data(),
            iDetailBining, 0, iDetailBining, iDetailBining, 0, iDetailBining);
      }
    }

    fDQM->mkdir("ResidualDetailDistr");
    fDQM->cd("ResidualDetailDistr");
    for (int i_angleU = 0; i_angleU < m_anglesU; i_angleU++) {
      for (int i_angleV = 0; i_angleV < m_anglesV; i_angleV++) {
        TString HistoName = Form("ResidPlaneRHUnBias_%s_AngU%02i_AngV%02i", StrAdd.Data(), i_angleU, i_angleV);
        TString HistoTitle = Form("Resid of RecoHit in Plane and UnBias Track, angle: %s U %02i, V %02i", StrAdd.Data(), i_angleU,
                                  i_angleV);
        m_histResidPlaneRHUnBiasUV[i_angleU * m_anglesV + i_angleV] = new TH2F(HistoName.Data(), HistoTitle.Data(),
            iDetailBining, -0.5 * iDetailBining, 0.5 * iDetailBining, iDetailBining, -0.5 * iDetailBining, 0.5 * iDetailBining);
        HistoName = Form("SigmaReco_%s_AngU%02i_AngV%02i", StrAdd.Data(), i_angleU, i_angleV);
        HistoTitle = Form("Sigma of RecoHit, %s angle: U %02i, V %02i", StrAdd.Data(), i_angleU, i_angleV);
        m_histSigmaRecoUV[i_angleU * m_anglesV + i_angleV] = new TH2F(HistoName.Data(), HistoTitle.Data(),
            iDetailBining, 0, iDetailBining, iDetailBining, 0, iDetailBining);
        HistoName = Form("ResidTrueRH_%s_AngU%02i_AngV%02i", StrAdd.Data(), i_angleU, i_angleV);
        HistoTitle = Form("Residuals of True and RecoHit: %s U %02i, V %02i", StrAdd.Data(), i_angleU, i_angleV);
        m_histResidTrueRHUV[i_angleU * m_anglesV + i_angleV] = new TH2F(HistoName.Data(), HistoTitle.Data(),
            iDetailBining, -0.5 * iDetailBining, 0.5 * iDetailBining, iDetailBining, -0.5 * iDetailBining, 0.5 * iDetailBining);
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
      DirPixelKind = Form("Bias_PixelKind_%01i_Layer_%i_Sensor_%i_Size_%i", i_pk, (int)((i_pk % 4) / 2) + 1, (int)(i_pk / 4) + 1,
                          i_pk % 2);
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
    for (int i_pk = 0; i_pk < m_pixelkinds; i_pk++) {
      DirPixelKind = Form("EE_PixelKind_%01i_Layer_%i_Sensor_%i_Size_%i", i_pk, (int)((i_pk % 4) / 2) + 1, (int)(i_pk / 4) + 1, i_pk % 2);
      fDQM->mkdir(DirPixelKind.Data());
      fExpertHistograms->mkdir(DirPixelKind.Data());
      for (int i_shape = 0; i_shape < m_shapes; i_shape++) {
        DirShape = Form("%s/Shape_%02i_%s", DirPixelKind.Data(), i_shape + 1,
                        Belle2::PXD::PXDClusterShape::pxdClusterShapeDescription[(Belle2::PXD::pxdClusterShapeType)(i_shape + 1)].c_str());
        DirShape.ReplaceAll(":", "");
        DirShape.ReplaceAll(" ", "_");
        fDQM->mkdir(DirShape.Data());
        fDQM->cd(DirShape.Data());

        // Error Estimation
        TString HistoName = Form("CorrTrueErrorEstimationU_PK%01i_Sh%02i", i_pk, i_shape + 1);
        TString HistoTitle = Form("Correction True Error Estimation U, pixel kind %01i, shape %02i", i_pk, i_shape + 1);
        m_histCorrEE_TrueU[i_pk * m_shapes + i_shape] = new TH2F(HistoName.Data(), HistoTitle.Data(),
                                                                 m_anglesU, -TMath::Pi() * 0.5 / Unit::deg, TMath::Pi() * 0.5 / Unit::deg,
                                                                 m_anglesV, -TMath::Pi() * 0.5 / Unit::deg, TMath::Pi() * 0.5 / Unit::deg);
        m_histCorrEE_TrueU[i_pk * m_shapes + i_shape]->GetXaxis()->SetTitle("Angle in u [deg]");
        m_histCorrEE_TrueU[i_pk * m_shapes + i_shape]->GetYaxis()->SetTitle("Angle in v [deg]");
        HistoName = Form("CorrTrueErrorEstimationV_PK%01i_Sh%02i", i_pk, i_shape + 1);
        HistoTitle = Form("Correction True Error Estimation V, pixel kind %01i, shape %02i", i_pk, i_shape + 1);
        m_histCorrEE_TrueV[i_pk * m_shapes + i_shape] = new TH2F(HistoName.Data(), HistoTitle.Data(),
                                                                 m_anglesU, -TMath::Pi() * 0.5 / Unit::deg, TMath::Pi() * 0.5 / Unit::deg,
                                                                 m_anglesV, -TMath::Pi() * 0.5 / Unit::deg, TMath::Pi() * 0.5 / Unit::deg);
        m_histCorrEE_TrueV[i_pk * m_shapes + i_shape]->GetXaxis()->SetTitle("Angle in u [deg]");
        m_histCorrEE_TrueV[i_pk * m_shapes + i_shape]->GetYaxis()->SetTitle("Angle in v [deg]");

        HistoName = Form("CorrRealDataErrorEstimationU_PK%01i_Sh%02i", i_pk, i_shape + 1);
        HistoTitle = Form("Correction Real Data Error Estimation U, pixel kind %01i, shape %02i", i_pk, i_shape + 1);
        m_histCorrEE_TrueTrackU[i_pk * m_shapes + i_shape] = new TH2F(HistoName.Data(), HistoTitle.Data(),
            m_anglesU, -TMath::Pi() * 0.5 / Unit::deg, TMath::Pi() * 0.5 / Unit::deg,
            m_anglesV, -TMath::Pi() * 0.5 / Unit::deg, TMath::Pi() * 0.5 / Unit::deg);
        m_histCorrEE_TrueTrackU[i_pk * m_shapes + i_shape]->GetXaxis()->SetTitle("Angle in u [deg]");
        m_histCorrEE_TrueTrackU[i_pk * m_shapes + i_shape]->GetYaxis()->SetTitle("Angle in v [deg]");
        HistoName = Form("CorrRealDataErrorEstimationV_PK%01i_Sh%02i", i_pk, i_shape + 1);
        HistoTitle = Form("Correction Real Data Error Estimation V, pixel kind %01i, shape %02i", i_pk, i_shape + 1);
        m_histCorrEE_TrueTrackV[i_pk * m_shapes + i_shape] = new TH2F(HistoName.Data(), HistoTitle.Data(),
            m_anglesU, -TMath::Pi() * 0.5 / Unit::deg, TMath::Pi() * 0.5 / Unit::deg,
            m_anglesV, -TMath::Pi() * 0.5 / Unit::deg, TMath::Pi() * 0.5 / Unit::deg);
        m_histCorrEE_TrueTrackV[i_pk * m_shapes + i_shape]->GetXaxis()->SetTitle("Angle in u [deg]");
        m_histCorrEE_TrueTrackV[i_pk * m_shapes + i_shape]->GetYaxis()->SetTitle("Angle in v [deg]");

        HistoName = Form("CorrErrorEstimationFromTrackU_PK%01i_Sh%02i", i_pk, i_shape + 1);
        HistoTitle = Form("Correction Error Estimation From Track U, pixel kind %01i, shape %02i", i_pk, i_shape + 1);
        m_histCorrEE_RealTrackU[i_pk * m_shapes + i_shape] = new TH2F(HistoName.Data(), HistoTitle.Data(),
            m_anglesU, -TMath::Pi() * 0.5 / Unit::deg, TMath::Pi() * 0.5 / Unit::deg,
            m_anglesV, -TMath::Pi() * 0.5 / Unit::deg, TMath::Pi() * 0.5 / Unit::deg);
        m_histCorrEE_RealTrackU[i_pk * m_shapes + i_shape]->GetXaxis()->SetTitle("Angle in u [deg]");
        m_histCorrEE_RealTrackU[i_pk * m_shapes + i_shape]->GetYaxis()->SetTitle("Angle in v [deg]");
        HistoName = Form("CorrErrorEstimationFromTrackV_PK%01i_Sh%02i", i_pk, i_shape + 1);
        HistoTitle = Form("Correction Error Estimation From Track V, pixel kind %01i, shape %02i", i_pk, i_shape + 1);
        m_histCorrEE_RealTrackV[i_pk * m_shapes + i_shape] = new TH2F(HistoName.Data(), HistoTitle.Data(),
            m_anglesU, -TMath::Pi() * 0.5 / Unit::deg, TMath::Pi() * 0.5 / Unit::deg,
            m_anglesV, -TMath::Pi() * 0.5 / Unit::deg, TMath::Pi() * 0.5 / Unit::deg);
        m_histCorrEE_RealTrackV[i_pk * m_shapes + i_shape]->GetXaxis()->SetTitle("Angle in u [deg]");
        m_histCorrEE_RealTrackV[i_pk * m_shapes + i_shape]->GetYaxis()->SetTitle("Angle in v [deg]");

        HistoName = Form("ResidualRMSU_PK%01i_Sh%02i", i_pk, i_shape + 1);
        HistoTitle = Form("Residual RMS U, pixel kind %01i, shape %02i", i_pk, i_shape + 1);
        m_histResRMSEEU[i_pk * m_shapes + i_shape] = new TH2F(HistoName.Data(), HistoTitle.Data(),
                                                              m_anglesU, -TMath::Pi() * 0.5 / Unit::deg, TMath::Pi() * 0.5 / Unit::deg,
                                                              m_anglesV, -TMath::Pi() * 0.5 / Unit::deg, TMath::Pi() * 0.5 / Unit::deg);
        m_histResRMSEEU[i_pk * m_shapes + i_shape]->GetXaxis()->SetTitle("Angle in u [deg]");
        m_histResRMSEEU[i_pk * m_shapes + i_shape]->GetYaxis()->SetTitle("Angle in v [deg]");
        HistoName = Form("ResidualRMSV_PK%01i_Sh%02i", i_pk, i_shape + 1);
        HistoTitle = Form("Residual RMS V, pixel kind %01i, shape %02i", i_pk, i_shape + 1);
        m_histResRMSEEV[i_pk * m_shapes + i_shape] = new TH2F(HistoName.Data(), HistoTitle.Data(),
                                                              m_anglesU, -TMath::Pi() * 0.5 / Unit::deg, TMath::Pi() * 0.5 / Unit::deg,
                                                              m_anglesV, -TMath::Pi() * 0.5 / Unit::deg, TMath::Pi() * 0.5 / Unit::deg);
        m_histResRMSEEV[i_pk * m_shapes + i_shape]->GetXaxis()->SetTitle("Angle in u [deg]");
        m_histResRMSEEV[i_pk * m_shapes + i_shape]->GetYaxis()->SetTitle("Angle in v [deg]");

        HistoName = Form("DiffClTrueOrigEEU_PK%01i_Sh%02i", i_pk, i_shape + 1);
        HistoTitle = Form("Difference ClusterEE True and Orig U, pixel kind %01i, shape %02i", i_pk, i_shape + 1);
        m_histDiffClEETrueOrigEEU[i_pk * m_shapes + i_shape] = new TH2F(HistoName.Data(), HistoTitle.Data(),
            m_anglesU, -TMath::Pi() * 0.5 / Unit::deg, TMath::Pi() * 0.5 / Unit::deg,
            m_anglesV, -TMath::Pi() * 0.5 / Unit::deg, TMath::Pi() * 0.5 / Unit::deg);
        m_histDiffClEETrueOrigEEU[i_pk * m_shapes + i_shape]->GetXaxis()->SetTitle("Angle in u [deg]");
        m_histDiffClEETrueOrigEEU[i_pk * m_shapes + i_shape]->GetYaxis()->SetTitle("Angle in v [deg]");
        HistoName = Form("DiffClTrueOrigEEV_PK%01i_Sh%02i", i_pk, i_shape + 1);
        HistoTitle = Form("Difference ClusterEE True and Orig V, pixel kind %01i, shape %02i", i_pk, i_shape + 1);
        m_histDiffClEETrueOrigEEV[i_pk * m_shapes + i_shape] = new TH2F(HistoName.Data(), HistoTitle.Data(),
            m_anglesU, -TMath::Pi() * 0.5 / Unit::deg, TMath::Pi() * 0.5 / Unit::deg,
            m_anglesV, -TMath::Pi() * 0.5 / Unit::deg, TMath::Pi() * 0.5 / Unit::deg);
        m_histDiffClEETrueOrigEEV[i_pk * m_shapes + i_shape]->GetXaxis()->SetTitle("Angle in u [deg]");
        m_histDiffClEETrueOrigEEV[i_pk * m_shapes + i_shape]->GetYaxis()->SetTitle("Angle in v [deg]");

        HistoName = Form("DiffClEETrueCalcTTEEU_PK%01i_Sh%02i", i_pk, i_shape + 1);
        HistoTitle = Form("Difference ClusterEE True and RealData Calcul U, pixel kind %01i, shape %02i", i_pk, i_shape + 1);
        m_histDiffClEETrueCalcTTEEU[i_pk * m_shapes + i_shape] = new TH2F(HistoName.Data(), HistoTitle.Data(),
            m_anglesU, -TMath::Pi() * 0.5 / Unit::deg, TMath::Pi() * 0.5 / Unit::deg,
            m_anglesV, -TMath::Pi() * 0.5 / Unit::deg, TMath::Pi() * 0.5 / Unit::deg);
        m_histDiffClEETrueCalcTTEEU[i_pk * m_shapes + i_shape]->GetXaxis()->SetTitle("Angle in u [deg]");
        m_histDiffClEETrueCalcTTEEU[i_pk * m_shapes + i_shape]->GetYaxis()->SetTitle("Angle in v [deg]");
        HistoName = Form("DiffClEETrueCalcTTEEV_PK%01i_Sh%02i", i_pk, i_shape + 1);
        HistoTitle = Form("Difference ClusterEE True and RealData Calcul V, pixel kind %01i, shape %02i", i_pk, i_shape + 1);
        m_histDiffClEETrueCalcTTEEV[i_pk * m_shapes + i_shape] = new TH2F(HistoName.Data(), HistoTitle.Data(),
            m_anglesU, -TMath::Pi() * 0.5 / Unit::deg, TMath::Pi() * 0.5 / Unit::deg,
            m_anglesV, -TMath::Pi() * 0.5 / Unit::deg, TMath::Pi() * 0.5 / Unit::deg);
        m_histDiffClEETrueCalcTTEEV[i_pk * m_shapes + i_shape]->GetXaxis()->SetTitle("Angle in u [deg]");
        m_histDiffClEETrueCalcTTEEV[i_pk * m_shapes + i_shape]->GetYaxis()->SetTitle("Angle in v [deg]");

        HistoName = Form("DiffClEETrueCalcRTEEU_PK%01i_Sh%02i", i_pk, i_shape + 1);
        HistoTitle = Form("Difference ClusterEE True and FromTrack Calcul U, pixel kind %01i, shape %02i", i_pk, i_shape + 1);
        m_histDiffClEETrueCalcRTEEU[i_pk * m_shapes + i_shape] = new TH2F(HistoName.Data(), HistoTitle.Data(),
            m_anglesU, -TMath::Pi() * 0.5 / Unit::deg, TMath::Pi() * 0.5 / Unit::deg,
            m_anglesV, -TMath::Pi() * 0.5 / Unit::deg, TMath::Pi() * 0.5 / Unit::deg);
        m_histDiffClEETrueCalcRTEEU[i_pk * m_shapes + i_shape]->GetXaxis()->SetTitle("Angle in u [deg]");
        m_histDiffClEETrueCalcRTEEU[i_pk * m_shapes + i_shape]->GetYaxis()->SetTitle("Angle in v [deg]");
        HistoName = Form("DiffClEETrueCalcRTEEV_PK%01i_Sh%02i", i_pk, i_shape + 1);
        HistoTitle = Form("Difference ClusterEE True and FromTrack Calcul V, pixel kind %01i, shape %02i", i_pk, i_shape + 1);
        m_histDiffClEETrueCalcRTEEV[i_pk * m_shapes + i_shape] = new TH2F(HistoName.Data(), HistoTitle.Data(),
            m_anglesU, -TMath::Pi() * 0.5 / Unit::deg, TMath::Pi() * 0.5 / Unit::deg,
            m_anglesV, -TMath::Pi() * 0.5 / Unit::deg, TMath::Pi() * 0.5 / Unit::deg);
        m_histDiffClEETrueCalcRTEEV[i_pk * m_shapes + i_shape]->GetXaxis()->SetTitle("Angle in u [deg]");
        m_histDiffClEETrueCalcRTEEV[i_pk * m_shapes + i_shape]->GetYaxis()->SetTitle("Angle in v [deg]");

        HistoName = Form("ClusterTrueEEU_PK%01i_Sh%02i", i_pk, i_shape + 1);
        HistoTitle = Form("ClusterEE True U, pixel kind %01i, shape %02i", i_pk, i_shape + 1);
        m_histClusterTrueEEU[i_pk * m_shapes + i_shape] = new TH2F(HistoName.Data(), HistoTitle.Data(),
                                                                   m_anglesU, -TMath::Pi() * 0.5 / Unit::deg, TMath::Pi() * 0.5 / Unit::deg,
                                                                   m_anglesV, -TMath::Pi() * 0.5 / Unit::deg, TMath::Pi() * 0.5 / Unit::deg);
        m_histClusterTrueEEU[i_pk * m_shapes + i_shape]->GetXaxis()->SetTitle("Angle in u [deg]");
        m_histClusterTrueEEU[i_pk * m_shapes + i_shape]->GetYaxis()->SetTitle("Angle in v [deg]");
        HistoName = Form("ClusterTrueEEV_PK%01i_Sh%02i", i_pk, i_shape + 1);
        HistoTitle = Form("ClusterEE True V, pixel kind %01i, shape %02i", i_pk, i_shape + 1);
        m_histClusterTrueEEV[i_pk * m_shapes + i_shape] = new TH2F(HistoName.Data(), HistoTitle.Data(),
                                                                   m_anglesU, -TMath::Pi() * 0.5 / Unit::deg, TMath::Pi() * 0.5 / Unit::deg,
                                                                   m_anglesV, -TMath::Pi() * 0.5 / Unit::deg, TMath::Pi() * 0.5 / Unit::deg);
        m_histClusterTrueEEV[i_pk * m_shapes + i_shape]->GetXaxis()->SetTitle("Angle in u [deg]");
        m_histClusterTrueEEV[i_pk * m_shapes + i_shape]->GetYaxis()->SetTitle("Angle in v [deg]");

        HistoName = Form("ClusterOrigEEU_PK%01i_Sh%02i", i_pk, i_shape + 1);
        HistoTitle = Form("ClusterEE Orig U, pixel kind %01i, shape %02i", i_pk, i_shape + 1);
        m_histClusterOrigEEU[i_pk * m_shapes + i_shape] = new TH2F(HistoName.Data(), HistoTitle.Data(),
                                                                   m_anglesU, -TMath::Pi() * 0.5 / Unit::deg, TMath::Pi() * 0.5 / Unit::deg,
                                                                   m_anglesV, -TMath::Pi() * 0.5 / Unit::deg, TMath::Pi() * 0.5 / Unit::deg);
        m_histClusterOrigEEU[i_pk * m_shapes + i_shape]->GetXaxis()->SetTitle("Angle in u [deg]");
        m_histClusterOrigEEU[i_pk * m_shapes + i_shape]->GetYaxis()->SetTitle("Angle in v [deg]");
        HistoName = Form("ClusterOrigEEV_PK%01i_Sh%02i", i_pk, i_shape + 1);
        HistoTitle = Form("ClusterEE Orig V, pixel kind %01i, shape %02i", i_pk, i_shape + 1);
        m_histClusterOrigEEV[i_pk * m_shapes + i_shape] = new TH2F(HistoName.Data(), HistoTitle.Data(),
                                                                   m_anglesU, -TMath::Pi() * 0.5 / Unit::deg, TMath::Pi() * 0.5 / Unit::deg,
                                                                   m_anglesV, -TMath::Pi() * 0.5 / Unit::deg, TMath::Pi() * 0.5 / Unit::deg);
        m_histClusterOrigEEV[i_pk * m_shapes + i_shape]->GetXaxis()->SetTitle("Angle in u [deg]");
        m_histClusterOrigEEV[i_pk * m_shapes + i_shape]->GetYaxis()->SetTitle("Angle in v [deg]");

        HistoName = Form("ClusterTTEEU_PK%01i_Sh%02i", i_pk, i_shape + 1);
        HistoTitle = Form("ClusterEE from RealData U, pixel kind %01i, shape %02i", i_pk, i_shape + 1);
        m_histClustCalcTTEEU[i_pk * m_shapes + i_shape] = new TH2F(HistoName.Data(), HistoTitle.Data(),
                                                                   m_anglesU, -TMath::Pi() * 0.5 / Unit::deg, TMath::Pi() * 0.5 / Unit::deg,
                                                                   m_anglesV, -TMath::Pi() * 0.5 / Unit::deg, TMath::Pi() * 0.5 / Unit::deg);
        m_histClustCalcTTEEU[i_pk * m_shapes + i_shape]->GetXaxis()->SetTitle("Angle in u [deg]");
        m_histClustCalcTTEEU[i_pk * m_shapes + i_shape]->GetYaxis()->SetTitle("Angle in v [deg]");
        HistoName = Form("ClusterTTEEV_PK%01i_Sh%02i", i_pk, i_shape + 1);
        HistoTitle = Form("ClusterEE from RealData V, pixel kind %01i, shape %02i", i_pk, i_shape + 1);
        m_histClustCalcTTEEV[i_pk * m_shapes + i_shape] = new TH2F(HistoName.Data(), HistoTitle.Data(),
                                                                   m_anglesU, -TMath::Pi() * 0.5 / Unit::deg, TMath::Pi() * 0.5 / Unit::deg,
                                                                   m_anglesV, -TMath::Pi() * 0.5 / Unit::deg, TMath::Pi() * 0.5 / Unit::deg);
        m_histClustCalcTTEEV[i_pk * m_shapes + i_shape]->GetXaxis()->SetTitle("Angle in u [deg]");
        m_histClustCalcTTEEV[i_pk * m_shapes + i_shape]->GetYaxis()->SetTitle("Angle in v [deg]");

        HistoName = Form("ClusterRTEEU_PK%01i_Sh%02i", i_pk, i_shape + 1);
        HistoTitle = Form("ClusterEE from Tracks U, pixel kind %01i, shape %02i", i_pk, i_shape + 1);
        m_histClustCalcRTEEU[i_pk * m_shapes + i_shape] = new TH2F(HistoName.Data(), HistoTitle.Data(),
                                                                   m_anglesU, -TMath::Pi() * 0.5 / Unit::deg, TMath::Pi() * 0.5 / Unit::deg,
                                                                   m_anglesV, -TMath::Pi() * 0.5 / Unit::deg, TMath::Pi() * 0.5 / Unit::deg);
        m_histClustCalcRTEEU[i_pk * m_shapes + i_shape]->GetXaxis()->SetTitle("Angle in u [deg]");
        m_histClustCalcRTEEU[i_pk * m_shapes + i_shape]->GetYaxis()->SetTitle("Angle in v [deg]");
        HistoName = Form("ClusterRTEEV_PK%01i_Sh%02i", i_pk, i_shape + 1);
        HistoTitle = Form("ClusterEE from Tracks V, pixel kind %01i, shape %02i", i_pk, i_shape + 1);
        m_histClustCalcRTEEV[i_pk * m_shapes + i_shape] = new TH2F(HistoName.Data(), HistoTitle.Data(),
                                                                   m_anglesU, -TMath::Pi() * 0.5 / Unit::deg, TMath::Pi() * 0.5 / Unit::deg,
                                                                   m_anglesV, -TMath::Pi() * 0.5 / Unit::deg, TMath::Pi() * 0.5 / Unit::deg);
        m_histClustCalcRTEEV[i_pk * m_shapes + i_shape]->GetXaxis()->SetTitle("Angle in u [deg]");
        m_histClustCalcRTEEV[i_pk * m_shapes + i_shape]->GetYaxis()->SetTitle("Angle in v [deg]");
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
    HistoName = Form("ResidualRMSU_Bias");
    HistoTitle = Form("Residual RMS U, no sorting");
    m_histResidualRMSU[m_pixelkinds * m_shapes] = new TH2F(HistoName.Data(), HistoTitle.Data(),
                                                           m_anglesU, -TMath::Pi() * 0.5 / Unit::deg, TMath::Pi() * 0.5 / Unit::deg,
                                                           m_anglesV, -TMath::Pi() * 0.5 / Unit::deg, TMath::Pi() * 0.5 / Unit::deg);
    m_histResidualRMSU[m_pixelkinds * m_shapes]->GetXaxis()->SetTitle("Angle in u [deg]");
    m_histResidualRMSU[m_pixelkinds * m_shapes]->GetYaxis()->SetTitle("Angle in v [deg]");
    HistoName = Form("ResidualRMSV_Bias");
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


    m_histBiasCounterU = new TH2F("BiasCounterU", "Bias Counter in U",
                                  m_anglesU, -TMath::Pi() * 0.5 / Unit::deg, TMath::Pi() * 0.5 / Unit::deg,
                                  m_anglesV, -TMath::Pi() * 0.5 / Unit::deg, TMath::Pi() * 0.5 / Unit::deg);
    m_histBiasCounterU->GetXaxis()->SetTitle("Angle in u [deg]");
    m_histBiasCounterU->GetYaxis()->SetTitle("Angle in v [deg]");
    m_histBiasCounterV = new TH2F("BiasCounterV", "Bias Counter in V",
                                  m_anglesU, -TMath::Pi() * 0.5 / Unit::deg, TMath::Pi() * 0.5 / Unit::deg,
                                  m_anglesV, -TMath::Pi() * 0.5 / Unit::deg, TMath::Pi() * 0.5 / Unit::deg);
    m_histBiasCounterV->GetXaxis()->SetTitle("Angle in u [deg]");
    m_histBiasCounterV->GetYaxis()->SetTitle("Angle in v [deg]");
    m_histEECounterU = new TH2F("EECounterU", "Error Estimation Counter in U",
                                m_anglesU, -TMath::Pi() * 0.5 / Unit::deg, TMath::Pi() * 0.5 / Unit::deg,
                                m_anglesV, -TMath::Pi() * 0.5 / Unit::deg, TMath::Pi() * 0.5 / Unit::deg);
    m_histEECounterU->GetXaxis()->SetTitle("Angle in u [deg]");
    m_histEECounterU->GetYaxis()->SetTitle("Angle in v [deg]");
    m_histEECounterV = new TH2F("EECounterV", "Error Estimation Counter in V",
                                m_anglesU, -TMath::Pi() * 0.5 / Unit::deg, TMath::Pi() * 0.5 / Unit::deg,
                                m_anglesV, -TMath::Pi() * 0.5 / Unit::deg, TMath::Pi() * 0.5 / Unit::deg);
    m_histEECounterV->GetXaxis()->SetTitle("Angle in u [deg]");
    m_histEECounterV->GetYaxis()->SetTitle("Angle in v [deg]");

    // Error Estimation
    HistoName = Form("CorrTrueErrorEstimationU");
    HistoTitle = Form("Correction True Error Estimation U, no sorting");
    m_histCorrEE_TrueU[m_pixelkinds * m_shapes] = new TH2F(HistoName.Data(), HistoTitle.Data(),
                                                           m_anglesU, -TMath::Pi() * 0.5 / Unit::deg, TMath::Pi() * 0.5 / Unit::deg,
                                                           m_anglesV, -TMath::Pi() * 0.5 / Unit::deg, TMath::Pi() * 0.5 / Unit::deg);
    m_histCorrEE_TrueU[m_pixelkinds * m_shapes]->GetXaxis()->SetTitle("Angle in u [deg]");
    m_histCorrEE_TrueU[m_pixelkinds * m_shapes]->GetYaxis()->SetTitle("Angle in v [deg]");
    HistoName = Form("CorrTrueErrorEstimationV");
    HistoTitle = Form("Correction True Error Estimation V, no sorting");
    m_histCorrEE_TrueV[m_pixelkinds * m_shapes] = new TH2F(HistoName.Data(), HistoTitle.Data(),
                                                           m_anglesU, -TMath::Pi() * 0.5 / Unit::deg, TMath::Pi() * 0.5 / Unit::deg,
                                                           m_anglesV, -TMath::Pi() * 0.5 / Unit::deg, TMath::Pi() * 0.5 / Unit::deg);
    m_histCorrEE_TrueV[m_pixelkinds * m_shapes]->GetXaxis()->SetTitle("Angle in u [deg]");
    m_histCorrEE_TrueV[m_pixelkinds * m_shapes]->GetYaxis()->SetTitle("Angle in v [deg]");

    HistoName = Form("CorrRealDataErrorEstimationU");
    HistoTitle = Form("Correction Real Data Error Estimation U, no sorting");
    m_histCorrEE_TrueTrackU[m_pixelkinds * m_shapes] = new TH2F(HistoName.Data(), HistoTitle.Data(),
                                                                m_anglesU, -TMath::Pi() * 0.5 / Unit::deg, TMath::Pi() * 0.5 / Unit::deg,
                                                                m_anglesV, -TMath::Pi() * 0.5 / Unit::deg, TMath::Pi() * 0.5 / Unit::deg);
    m_histCorrEE_TrueTrackU[m_pixelkinds * m_shapes]->GetXaxis()->SetTitle("Angle in u [deg]");
    m_histCorrEE_TrueTrackU[m_pixelkinds * m_shapes]->GetYaxis()->SetTitle("Angle in v [deg]");
    HistoName = Form("CorrRealDataErrorEstimationV");
    HistoTitle = Form("Correction Real Data Error Estimation V, no sorting");
    m_histCorrEE_TrueTrackV[m_pixelkinds * m_shapes] = new TH2F(HistoName.Data(), HistoTitle.Data(),
                                                                m_anglesU, -TMath::Pi() * 0.5 / Unit::deg, TMath::Pi() * 0.5 / Unit::deg,
                                                                m_anglesV, -TMath::Pi() * 0.5 / Unit::deg, TMath::Pi() * 0.5 / Unit::deg);
    m_histCorrEE_TrueTrackV[m_pixelkinds * m_shapes]->GetXaxis()->SetTitle("Angle in u [deg]");
    m_histCorrEE_TrueTrackV[m_pixelkinds * m_shapes]->GetYaxis()->SetTitle("Angle in v [deg]");

    HistoName = Form("CorrErrorEstimationFromTrackU");
    HistoTitle = Form("Correction Error Estimation From Track U, no sorting");
    m_histCorrEE_RealTrackU[m_pixelkinds * m_shapes] = new TH2F(HistoName.Data(), HistoTitle.Data(),
                                                                m_anglesU, -TMath::Pi() * 0.5 / Unit::deg, TMath::Pi() * 0.5 / Unit::deg,
                                                                m_anglesV, -TMath::Pi() * 0.5 / Unit::deg, TMath::Pi() * 0.5 / Unit::deg);
    m_histCorrEE_RealTrackU[m_pixelkinds * m_shapes]->GetXaxis()->SetTitle("Angle in u [deg]");
    m_histCorrEE_RealTrackU[m_pixelkinds * m_shapes]->GetYaxis()->SetTitle("Angle in v [deg]");
    HistoName = Form("CorrErrorEstimationFromTrackV");
    HistoTitle = Form("Correction Error Estimation From Track V, no sorting");
    m_histCorrEE_RealTrackV[m_pixelkinds * m_shapes] = new TH2F(HistoName.Data(), HistoTitle.Data(),
                                                                m_anglesU, -TMath::Pi() * 0.5 / Unit::deg, TMath::Pi() * 0.5 / Unit::deg,
                                                                m_anglesV, -TMath::Pi() * 0.5 / Unit::deg, TMath::Pi() * 0.5 / Unit::deg);
    m_histCorrEE_RealTrackV[m_pixelkinds * m_shapes]->GetXaxis()->SetTitle("Angle in u [deg]");
    m_histCorrEE_RealTrackV[m_pixelkinds * m_shapes]->GetYaxis()->SetTitle("Angle in v [deg]");

    HistoName = Form("ResidualRMSU");
    HistoTitle = Form("Residual RMS U, no sorting");
    m_histResRMSEEU[m_pixelkinds * m_shapes] = new TH2F(HistoName.Data(), HistoTitle.Data(),
                                                        m_anglesU, -TMath::Pi() * 0.5 / Unit::deg, TMath::Pi() * 0.5 / Unit::deg,
                                                        m_anglesV, -TMath::Pi() * 0.5 / Unit::deg, TMath::Pi() * 0.5 / Unit::deg);
    m_histResRMSEEU[m_pixelkinds * m_shapes]->GetXaxis()->SetTitle("Angle in u [deg]");
    m_histResRMSEEU[m_pixelkinds * m_shapes]->GetYaxis()->SetTitle("Angle in v [deg]");
    HistoName = Form("ResidualRMSV");
    HistoTitle = Form("Residual RMS V, no sorting");
    m_histResRMSEEV[m_pixelkinds * m_shapes] = new TH2F(HistoName.Data(), HistoTitle.Data(),
                                                        m_anglesU, -TMath::Pi() * 0.5 / Unit::deg, TMath::Pi() * 0.5 / Unit::deg,
                                                        m_anglesV, -TMath::Pi() * 0.5 / Unit::deg, TMath::Pi() * 0.5 / Unit::deg);
    m_histResRMSEEV[m_pixelkinds * m_shapes]->GetXaxis()->SetTitle("Angle in u [deg]");
    m_histResRMSEEV[m_pixelkinds * m_shapes]->GetYaxis()->SetTitle("Angle in v [deg]");

    HistoName = Form("DiffClTrueOrigEEU");
    HistoTitle = Form("Difference ClusterEE True and Orig U, no sorting");
    m_histDiffClEETrueOrigEEU[m_pixelkinds * m_shapes] = new TH2F(HistoName.Data(), HistoTitle.Data(),
        m_anglesU, -TMath::Pi() * 0.5 / Unit::deg, TMath::Pi() * 0.5 / Unit::deg,
        m_anglesV, -TMath::Pi() * 0.5 / Unit::deg, TMath::Pi() * 0.5 / Unit::deg);
    m_histDiffClEETrueOrigEEU[m_pixelkinds * m_shapes]->GetXaxis()->SetTitle("Angle in u [deg]");
    m_histDiffClEETrueOrigEEU[m_pixelkinds * m_shapes]->GetYaxis()->SetTitle("Angle in v [deg]");
    HistoName = Form("DiffClTrueOrigEEV");
    HistoTitle = Form("Difference ClusterEE True and Orig V, no sorting");
    m_histDiffClEETrueOrigEEV[m_pixelkinds * m_shapes] = new TH2F(HistoName.Data(), HistoTitle.Data(),
        m_anglesU, -TMath::Pi() * 0.5 / Unit::deg, TMath::Pi() * 0.5 / Unit::deg,
        m_anglesV, -TMath::Pi() * 0.5 / Unit::deg, TMath::Pi() * 0.5 / Unit::deg);
    m_histDiffClEETrueOrigEEV[m_pixelkinds * m_shapes]->GetXaxis()->SetTitle("Angle in u [deg]");
    m_histDiffClEETrueOrigEEV[m_pixelkinds * m_shapes]->GetYaxis()->SetTitle("Angle in v [deg]");

    HistoName = Form("DiffClEETrueCalcTTEEU");
    HistoTitle = Form("Difference ClusterEE True and RealData Calcul U, no sorting");
    m_histDiffClEETrueCalcTTEEU[m_pixelkinds * m_shapes] = new TH2F(HistoName.Data(), HistoTitle.Data(),
        m_anglesU, -TMath::Pi() * 0.5 / Unit::deg, TMath::Pi() * 0.5 / Unit::deg,
        m_anglesV, -TMath::Pi() * 0.5 / Unit::deg, TMath::Pi() * 0.5 / Unit::deg);
    m_histDiffClEETrueCalcTTEEU[m_pixelkinds * m_shapes]->GetXaxis()->SetTitle("Angle in u [deg]");
    m_histDiffClEETrueCalcTTEEU[m_pixelkinds * m_shapes]->GetYaxis()->SetTitle("Angle in v [deg]");
    HistoName = Form("DiffClEETrueCalcTTEEV");
    HistoTitle = Form("Difference ClusterEE True and RealData Calcul V, no sorting");
    m_histDiffClEETrueCalcTTEEV[m_pixelkinds * m_shapes] = new TH2F(HistoName.Data(), HistoTitle.Data(),
        m_anglesU, -TMath::Pi() * 0.5 / Unit::deg, TMath::Pi() * 0.5 / Unit::deg,
        m_anglesV, -TMath::Pi() * 0.5 / Unit::deg, TMath::Pi() * 0.5 / Unit::deg);
    m_histDiffClEETrueCalcTTEEV[m_pixelkinds * m_shapes]->GetXaxis()->SetTitle("Angle in u [deg]");
    m_histDiffClEETrueCalcTTEEV[m_pixelkinds * m_shapes]->GetYaxis()->SetTitle("Angle in v [deg]");

    HistoName = Form("DiffClEETrueCalcRTEEU");
    HistoTitle = Form("Difference ClusterEE True and FromTrack Calcul U, no sorting");
    m_histDiffClEETrueCalcRTEEU[m_pixelkinds * m_shapes] = new TH2F(HistoName.Data(), HistoTitle.Data(),
        m_anglesU, -TMath::Pi() * 0.5 / Unit::deg, TMath::Pi() * 0.5 / Unit::deg,
        m_anglesV, -TMath::Pi() * 0.5 / Unit::deg, TMath::Pi() * 0.5 / Unit::deg);
    m_histDiffClEETrueCalcRTEEU[m_pixelkinds * m_shapes]->GetXaxis()->SetTitle("Angle in u [deg]");
    m_histDiffClEETrueCalcRTEEU[m_pixelkinds * m_shapes]->GetYaxis()->SetTitle("Angle in v [deg]");
    HistoName = Form("DiffClEETrueCalcRTEEV");
    HistoTitle = Form("Difference ClusterEE True and FromTrack Calcul V, no sorting");
    m_histDiffClEETrueCalcRTEEV[m_pixelkinds * m_shapes] = new TH2F(HistoName.Data(), HistoTitle.Data(),
        m_anglesU, -TMath::Pi() * 0.5 / Unit::deg, TMath::Pi() * 0.5 / Unit::deg,
        m_anglesV, -TMath::Pi() * 0.5 / Unit::deg, TMath::Pi() * 0.5 / Unit::deg);
    m_histDiffClEETrueCalcRTEEV[m_pixelkinds * m_shapes]->GetXaxis()->SetTitle("Angle in u [deg]");
    m_histDiffClEETrueCalcRTEEV[m_pixelkinds * m_shapes]->GetYaxis()->SetTitle("Angle in v [deg]");

    HistoName = Form("ClusterTrueEEU");
    HistoTitle = Form("ClusterEE True U, no sorting");
    m_histClusterTrueEEU[m_pixelkinds * m_shapes] = new TH2F(HistoName.Data(), HistoTitle.Data(),
                                                             m_anglesU, -TMath::Pi() * 0.5 / Unit::deg, TMath::Pi() * 0.5 / Unit::deg,
                                                             m_anglesV, -TMath::Pi() * 0.5 / Unit::deg, TMath::Pi() * 0.5 / Unit::deg);
    m_histClusterTrueEEU[m_pixelkinds * m_shapes]->GetXaxis()->SetTitle("Angle in u [deg]");
    m_histClusterTrueEEU[m_pixelkinds * m_shapes]->GetYaxis()->SetTitle("Angle in v [deg]");
    HistoName = Form("ClusterTrueEEV");
    HistoTitle = Form("ClusterEE True V, no sorting");
    m_histClusterTrueEEV[m_pixelkinds * m_shapes] = new TH2F(HistoName.Data(), HistoTitle.Data(),
                                                             m_anglesU, -TMath::Pi() * 0.5 / Unit::deg, TMath::Pi() * 0.5 / Unit::deg,
                                                             m_anglesV, -TMath::Pi() * 0.5 / Unit::deg, TMath::Pi() * 0.5 / Unit::deg);
    m_histClusterTrueEEV[m_pixelkinds * m_shapes]->GetXaxis()->SetTitle("Angle in u [deg]");
    m_histClusterTrueEEV[m_pixelkinds * m_shapes]->GetYaxis()->SetTitle("Angle in v [deg]");

    HistoName = Form("ClusterOrigEEU");
    HistoTitle = Form("ClusterEE Orig U, no sorting");
    m_histClusterOrigEEU[m_pixelkinds * m_shapes] = new TH2F(HistoName.Data(), HistoTitle.Data(),
                                                             m_anglesU, -TMath::Pi() * 0.5 / Unit::deg, TMath::Pi() * 0.5 / Unit::deg,
                                                             m_anglesV, -TMath::Pi() * 0.5 / Unit::deg, TMath::Pi() * 0.5 / Unit::deg);
    m_histClusterOrigEEU[m_pixelkinds * m_shapes]->GetXaxis()->SetTitle("Angle in u [deg]");
    m_histClusterOrigEEU[m_pixelkinds * m_shapes]->GetYaxis()->SetTitle("Angle in v [deg]");
    HistoName = Form("ClusterOrigEEV");
    HistoTitle = Form("ClusterEE Orig V, no sorting");
    m_histClusterOrigEEV[m_pixelkinds * m_shapes] = new TH2F(HistoName.Data(), HistoTitle.Data(),
                                                             m_anglesU, -TMath::Pi() * 0.5 / Unit::deg, TMath::Pi() * 0.5 / Unit::deg,
                                                             m_anglesV, -TMath::Pi() * 0.5 / Unit::deg, TMath::Pi() * 0.5 / Unit::deg);
    m_histClusterOrigEEV[m_pixelkinds * m_shapes]->GetXaxis()->SetTitle("Angle in u [deg]");
    m_histClusterOrigEEV[m_pixelkinds * m_shapes]->GetYaxis()->SetTitle("Angle in v [deg]");

    HistoName = Form("ClusterTTEEU");
    HistoTitle = Form("ClusterEE from RealData U, no sorting");
    m_histClustCalcTTEEU[m_pixelkinds * m_shapes] = new TH2F(HistoName.Data(), HistoTitle.Data(),
                                                             m_anglesU, -TMath::Pi() * 0.5 / Unit::deg, TMath::Pi() * 0.5 / Unit::deg,
                                                             m_anglesV, -TMath::Pi() * 0.5 / Unit::deg, TMath::Pi() * 0.5 / Unit::deg);
    m_histClustCalcTTEEU[m_pixelkinds * m_shapes]->GetXaxis()->SetTitle("Angle in u [deg]");
    m_histClustCalcTTEEU[m_pixelkinds * m_shapes]->GetYaxis()->SetTitle("Angle in v [deg]");
    HistoName = Form("ClusterTTEEV");
    HistoTitle = Form("ClusterEE from RealData V, no sorting");
    m_histClustCalcTTEEV[m_pixelkinds * m_shapes] = new TH2F(HistoName.Data(), HistoTitle.Data(),
                                                             m_anglesU, -TMath::Pi() * 0.5 / Unit::deg, TMath::Pi() * 0.5 / Unit::deg,
                                                             m_anglesV, -TMath::Pi() * 0.5 / Unit::deg, TMath::Pi() * 0.5 / Unit::deg);
    m_histClustCalcTTEEV[m_pixelkinds * m_shapes]->GetXaxis()->SetTitle("Angle in u [deg]");
    m_histClustCalcTTEEV[m_pixelkinds * m_shapes]->GetYaxis()->SetTitle("Angle in v [deg]");

    HistoName = Form("ClusterRTEEU");
    HistoTitle = Form("ClusterEE from Tracks U, no sorting");
    m_histClustCalcRTEEU[m_pixelkinds * m_shapes] = new TH2F(HistoName.Data(), HistoTitle.Data(),
                                                             m_anglesU, -TMath::Pi() * 0.5 / Unit::deg, TMath::Pi() * 0.5 / Unit::deg,
                                                             m_anglesV, -TMath::Pi() * 0.5 / Unit::deg, TMath::Pi() * 0.5 / Unit::deg);
    m_histClustCalcRTEEU[m_pixelkinds * m_shapes]->GetXaxis()->SetTitle("Angle in u [deg]");
    m_histClustCalcRTEEU[m_pixelkinds * m_shapes]->GetYaxis()->SetTitle("Angle in v [deg]");
    HistoName = Form("ClusterRTEEV");
    HistoTitle = Form("ClusterEE from Tracks V, no sorting");
    m_histClustCalcRTEEV[m_pixelkinds * m_shapes] = new TH2F(HistoName.Data(), HistoTitle.Data(),
                                                             m_anglesU, -TMath::Pi() * 0.5 / Unit::deg, TMath::Pi() * 0.5 / Unit::deg,
                                                             m_anglesV, -TMath::Pi() * 0.5 / Unit::deg, TMath::Pi() * 0.5 / Unit::deg);
    m_histClustCalcRTEEV[m_pixelkinds * m_shapes]->GetXaxis()->SetTitle("Angle in u [deg]");
    m_histClustCalcRTEEV[m_pixelkinds * m_shapes]->GetYaxis()->SetTitle("Angle in v [deg]");

//    fDQM->cd();
//    fExpertHistograms->cd();

    fDQM->cd("NoSorting");
    fExpertHistograms->cd("NoSorting");
  }

  auto tree = getObjectPtr<TTree>(name_SourceTree.Data());
  int nEntries = tree->GetEntries();

  B2INFO("Entries: " << nEntries);
  SummariesInfo[0] = nEntries;
  B2INFO("UseRealData: " << m_UseRealData <<
         ", PixelKind: " << m_UsePixelKind
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
  tree->SetBranchAddress("phiTrue", &m_phiTrue);
  tree->SetBranchAddress("thetaTrue", &m_thetaTrue);
  tree->SetBranchAddress("signal", &m_signal);
  tree->SetBranchAddress("seed", &m_seed);
  tree->SetBranchAddress("InPixUTrue", &m_InPixUTrue);
  tree->SetBranchAddress("InPixVTrue", &m_InPixVTrue);
  tree->SetBranchAddress("InPixUReco", &m_InPixUReco);
  tree->SetBranchAddress("InPixVReco", &m_InPixVReco);
  tree->SetBranchAddress("ResidUTrueCl", &m_ResidUTrueCl);
  tree->SetBranchAddress("ResidVTrueCl", &m_ResidVTrueCl);
  tree->SetBranchAddress("SigmaUReco", &m_SigmaUReco);
  tree->SetBranchAddress("SigmaVReco", &m_SigmaVReco);

  if (m_UsePixelKind < 0) {  // only tracking used
    tree->SetBranchAddress("phiTrack", &m_phiTrack);
    tree->SetBranchAddress("thetaTrack", &m_thetaTrack);
    tree->SetBranchAddress("ResidUPlaneRHBias", &m_ResidUPlaneRHBias);
    tree->SetBranchAddress("ResidVPlaneRHBias", &m_ResidVPlaneRHBias);
    tree->SetBranchAddress("ResidUPlaneRHUnBias", &m_ResidUPlaneRHUnBias);
    tree->SetBranchAddress("ResidVPlaneRHUnBias", &m_ResidVPlaneRHUnBias);
    tree->SetBranchAddress("ResidUPlaneClBias", &m_ResidUPlaneClBias);
    tree->SetBranchAddress("ResidVPlaneClBias", &m_ResidVPlaneClBias);
    tree->SetBranchAddress("ResidUTruePlaneBias", &m_ResidUTruePlaneBias);
    tree->SetBranchAddress("ResidVTruePlaneBias", &m_ResidVTruePlaneBias);
    tree->SetBranchAddress("ResidUTruePlaneUnBias", &m_ResidUTruePlaneUnBias);
    tree->SetBranchAddress("ResidVTruePlaneUnBias", &m_ResidVTruePlaneUnBias);
    tree->SetBranchAddress("ResidUTrueRH", &m_ResidUTrueRH);
    tree->SetBranchAddress("ResidVTrueRH", &m_ResidVTrueRH);
    tree->SetBranchAddress("ErrorEstimateCovarUVBias", &m_ErrorEstimateCovarUVBias);
    tree->SetBranchAddress("ErrorEstimateCovarUVUnBias", &m_ErrorEstimateCovarUVUnBias);
    tree->SetBranchAddress("SigmaUTrackBias", &m_SigmaUTrackBias);
    tree->SetBranchAddress("SigmaVTrackBias", &m_SigmaVTrackBias);
    tree->SetBranchAddress("SigmaUResBias", &m_SigmaUResBias);
    tree->SetBranchAddress("SigmaVResBias", &m_SigmaVResBias);
    tree->SetBranchAddress("SigmaUTrackUnBias", &m_SigmaUTrackUnBias);
    tree->SetBranchAddress("SigmaVTrackUnBias", &m_SigmaVTrackUnBias);
    tree->SetBranchAddress("SigmaUResUnBias", &m_SigmaUResUnBias);
    tree->SetBranchAddress("SigmaVResUnBias", &m_SigmaVResUnBias);
  }
  // create vector for storing on database:

  TVectorD* Correction_Bias;
//  std::vector<TVectorD*> Correction_Bias(1);
  TVectorD* Correction_ErrorEstimation;
  TVectorD* Correction_ErrorEstimationCovariance;
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
  Correction_ErrorEstimationCovariance = new TVectorD(m_shapes * m_pixelkinds * m_dimensions * m_anglesU * m_anglesV);
  Correction_BiasErr = new TVectorD(m_shapes * m_pixelkinds * m_dimensions * m_anglesU * m_anglesV);
  InPixelPosition = new TVectorD(m_shapes * m_pixelkinds * m_anglesU * m_anglesV * m_in_pixelU * m_in_pixelV);

  // presets of vectors and tables:

  Double_t* ValueCors = new Double_t[m_shapes * m_pixelkinds * m_dimensions * m_anglesU * m_anglesV];
  Double_t* ValueInPix = new Double_t[m_shapes * m_pixelkinds * m_anglesU * m_anglesV * m_in_pixelU * m_in_pixelV];
  for (int i = 0; i < m_shapes * m_pixelkinds * m_dimensions * m_anglesU * m_anglesV; i++)
    ValueCors[i] = 0.0;
  Correction_Bias->SetElements(ValueCors);
  Correction_BiasErr->SetElements(ValueCors);
  Correction_ErrorEstimation->SetElements(ValueCors);
  Correction_ErrorEstimationCovariance->SetElements(ValueCors);
  for (int i = 0; i < m_shapes * m_pixelkinds * m_anglesU * m_anglesV * m_in_pixelU * m_in_pixelV; i++)
    ValueInPix[i] = 0.0;
  InPixelPosition->SetElements(ValueInPix);
  delete[] ValueCors;
  delete[] ValueInPix;

  B2DEBUG(30, "--> Presets done. ");

  //std::unordered_map<key_type, double, key_hash, key_equal> TCorrection_BiasMap;
  map_typeCorrs TCorrection_BiasMap;
  map_typeCorrs TCorrection_ErrorEstimationMap;
  map_typeCorrs TCorrection_ErrorEstimationCovarianceMap;
  map_typeCorrs TCorrection_BiasMapErr;
  std::unordered_map<key_type2, double, key_hash2, key_equal2> TInPixelPositionMap;
  for (int i_shape = 0; i_shape < m_shapes; i_shape++)
    for (int i_pk = 0; i_pk < m_pixelkinds; i_pk++)
      for (int i_axis = 0; i_axis < m_dimensions; i_axis++)
        for (int i_angleU = 0; i_angleU < m_anglesU; i_angleU++)
          for (int i_angleV = 0; i_angleV < m_anglesV; i_angleV++) {
            // TCorrection_Bias[i_shape][i_pk][i_axis][i_angleU][i_angleV] = 0.0;
            TCorrection_BiasMap[make_tuple(i_shape, i_pk, i_axis, i_angleU, i_angleV)] = 0.0;
            TCorrection_ErrorEstimationMap[make_tuple(i_shape, i_pk, i_axis, i_angleU, i_angleV)] = 0.0;
            TCorrection_ErrorEstimationCovarianceMap[make_tuple(i_shape, i_pk, i_axis, i_angleU, i_angleV)] = 0.0;
            TCorrection_BiasMapErr[make_tuple(i_shape, i_pk, i_axis, i_angleU, i_angleV)] = 0.0;
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

      int AngU = getPhiIndex(m_phiTrack);
      int AngV = getThetaIndex(m_thetaTrack);
      if (AngU < 0) AngU = 0;
      if (AngU >= m_anglesU) AngU = m_anglesU - 1;
      if (AngV < 0) AngV = 0;
      if (AngV >= m_anglesV) AngV = m_anglesV - 1;

      m_histResidTruePlaneUnBiasUV[AngU * m_anglesV + AngV]->Fill(m_ResidUTruePlaneUnBias / Unit::um, m_ResidVTruePlaneUnBias / Unit::um);
      m_histResidTruePlaneUnBiasUV[m_anglesU * m_anglesV]->Fill(m_ResidUTruePlaneUnBias / Unit::um, m_ResidVTruePlaneUnBias / Unit::um);
      m_histSigmaTrackUnBiasUV[AngU * m_anglesV + AngV]->Fill(m_SigmaUTrackUnBias / Unit::um, m_SigmaVTrackUnBias / Unit::um);
      m_histSigmaTrackUnBiasUV[m_anglesU * m_anglesV]->Fill(m_SigmaUTrackUnBias / Unit::um, m_SigmaVTrackUnBias / Unit::um);

    }
    B2DEBUG(30, "--> Filling of histos done. ");
  }
  // To be fun show first few entries:
  for (int i_ev = 0; i_ev < 3; i_ev++) {
    tree->Show(i_ev);
  }

  // presets of vectors and tables:
  freopen("/dev/null", "w", stderr);
  int nSelRowsTemp = 0;
  int iDebugCalculation = 0;  // set 1 if you need some visible case for corrections, default=0
  int iFrom1 = 0;
  int iTo1 = m_shapes;
  if (iDebugCalculation) {
    iFrom1 = 10;
    iTo1 = 11;
    iFrom1 = 0;
    iTo1 = 1;
    iFrom1 = 1;
    iTo1 = 2;
    iFrom1 = 2;
    iTo1 = 3;
  }
  for (int i_shape = iFrom1; i_shape < iTo1; i_shape++) {
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
      tree->Draw("ResidUPlaneRHUnBias:ResidUPlaneRHUnBias", c1, "goff");
      nSelRowsTemp = (int)tree->GetSelectedRows();
      if (nSelRowsTemp < m_MinClustersCorrections) continue;
    }
    int iFrom2 = 0;
    int iTo2 = m_pixelkinds;
    if (iDebugCalculation) {
      iFrom2 = 2;
      iFrom2 = 7;
      iFrom2 = 6;
      iTo2 = m_pixelkinds;
    }
    for (int i_pk = iFrom2; i_pk < iTo2; i_pk++) {
      cCat = Form("pixelKind == %i", i_pk);
      c2.SetTitle(cCat.Data());
      if (!m_DoExpertHistograms) {  // acceleration of calibration process
        tree->Draw("ResidUPlaneRHUnBias:ResidUPlaneRHUnBias", c1 + c2, "goff");
        nSelRowsTemp = (int)tree->GetSelectedRows();
        if (nSelRowsTemp < m_MinClustersCorrections) continue;
      }
      B2INFO("   ---> Processing pixel kind: " << i_pk <<
             ", Layer: " << (int)(i_pk / 4) + 1 <<
             ", Sensor: " << (int)((i_pk % 4) / 2) + 1 <<
             ", Size: " << (1 - i_pk % 2) * (1 - (int)((i_pk % 4) / 2)) + i_pk % 2 * (int)((i_pk % 4) / 2)
            );
      int iFrom3 = 0;
      int iTo3 = m_anglesU;
      if (iDebugCalculation) {
        iFrom3 = 8;
        iTo3 = 9;
        iFrom3 = 9;
        iTo3 = 10;
      }
      for (int i_angleU = iFrom3; i_angleU < iTo3; i_angleU++) {
        double PhiMi = getPhiFromIndex(i_angleU);
        double PhiMa = getPhiFromIndex(i_angleU + 1);
        if (m_UsePixelKind == -1) {
          cCat = Form("phiTrack > %f", PhiMi);
          c3a.SetTitle(cCat.Data());
          cCat = Form("phiTrack < %f", PhiMa);
          c3b.SetTitle(cCat.Data());
        } else {
          cCat = Form("phiTrue > %f", PhiMi);
          c3a.SetTitle(cCat.Data());
          cCat = Form("phiTrue < %f", PhiMa);
          c3b.SetTitle(cCat.Data());
        }
        if (!m_DoExpertHistograms) {  // acceleration of calibration process
          tree->Draw("ResidUPlaneRHUnBias:ResidUPlaneRHUnBias", c1 + c2 + c3a + c3b, "goff");
          nSelRowsTemp = (int)tree->GetSelectedRows();
          if (nSelRowsTemp < m_MinClustersCorrections) continue;
        }
        int iFrom4 = 0;
        int iTo4 = m_anglesV;
        if (iDebugCalculation) {
          iFrom4 = 12;
          iTo4 = 15;
          iFrom4 = 8;
          iTo4 = 9;
          iFrom4 = 9;
          iTo4 = 10;
          iFrom4 = 12;
          iTo4 = 13;
        }
        for (int i_angleV = iFrom4; i_angleV < iTo4; i_angleV++) {
          SummariesInfo[1]++;
          B2DEBUG(200, "  --> AngleCalibration for: " << i_angleU << ", " << i_angleV);

          double ThetaMi = getThetaFromIndex(i_angleV);
          double ThetaMa = getThetaFromIndex(i_angleV + 1);
          if (m_UsePixelKind == -1) {
            cCat = Form("thetaTrack > %f", ThetaMi);
            c4a.SetTitle(cCat.Data());
            cCat = Form("thetaTrack < %f", ThetaMa);
            c4b.SetTitle(cCat.Data());
          } else {
            cCat = Form("thetaTrue > %f", ThetaMi);
            c4a.SetTitle(cCat.Data());
            cCat = Form("thetaTrue < %f", ThetaMa);
            c4b.SetTitle(cCat.Data());
          }
          if (!m_DoExpertHistograms) {  // acceleration of calibration process
            tree->Draw("ResidUPlaneRHUnBias:ResidUPlaneRHUnBias", c1 + c2 + c3a + c3b + c4a + c4b, "goff");
            nSelRowsTemp = (int)tree->GetSelectedRows();
            if (nSelRowsTemp < m_MinClustersCorrections) continue;
          }
          cCat = Form("closeEdge == 0");
          c5.SetTitle(cCat.Data());

          // Create final cut condition and selection:
          cFin = c1 && c2 && c3a && c3b && c4a && c4b && c5;  // full selection
          // cFin = c1 && c2 && c5;  // For tuning time: no angle selection...
          // cFin = c2 && c5;  // For tuning time: no shape and angle selection...
          tree->Draw(">>selection", cFin);
          TEventList* selectionn = (TEventList*)gDirectory->Get("selection");
          int nSelRows = (int)tree->GetSelectedRows();
          B2DEBUG(130, "  --> Selection criteria: " << cFin.GetTitle() << ", rows: " << nSelRows);
          SummariesInfoSh[i_shape] += nSelRows;
          SummariesInfoAng[i_angleU * m_anglesV + i_angleV] += nSelRows;
          SummariesInfoPK[i_pk] += nSelRows;

          if (iDebugCalculation) {
            B2DEBUG(190, "nSelRows: " << nSelRows << "\n");
            for (int i = 0; i < nSelRows; i++) {
              tree->GetEntry(selectionn->GetEntry(i));
              B2DEBUG(190, "Shape: " << m_shape << ", ClEE_u: " << m_SigmaUReco * 10000 <<
                      ", ClEE_v: " << m_SigmaVReco * 10000 << "\n");
            }
            B2DEBUG(190, "\n\n");
          }
          // Acceptation:
          if (nSelRows >= m_MinClustersCorrections) {
            std::vector<float> Colm1(nSelRows);
            std::vector<float> Colm2(nSelRows);
            std::vector<float> Colm3(nSelRows);
            std::vector<float> Colm4(nSelRows);
            std::vector<float> Colm5(nSelRows);
            std::vector<float> Colm6(nSelRows);
            std::vector<float> Colm9(nSelRows);
            std::vector<float> Colm10(nSelRows);
            std::vector<float> Colm13(nSelRows);
            std::vector<float> Colm14(nSelRows);
            for (int i = 0; i < nSelRows; i++) {
              tree->GetEntry(selectionn->GetEntry(i));
              if (m_UsePixelKind == -1) {
                Colm1[i] = m_ResidUPlaneRHUnBias;
                Colm2[i] = m_ResidVPlaneRHUnBias;
                Colm3[i] = m_SigmaUReco;
                Colm4[i] = m_SigmaVReco;
                Colm5[i] = m_SigmaUTrackUnBias;
                Colm6[i] = m_SigmaVTrackUnBias;
                Colm9[i] = m_ResidUTrueCl;
                Colm10[i] = m_ResidVTrueCl;
                Colm13[i] = m_ResidUTruePlaneUnBias;
                Colm14[i] = m_ResidVTruePlaneUnBias;
              } else {
                Colm1[i] = m_ResidUTrueCl;
                Colm2[i] = m_ResidVTrueCl;
                Colm3[i] = m_SigmaUReco;
                Colm4[i] = m_SigmaVReco;
              }
              if ((m_DoExpertHistograms) && (TestPK == i_pk) && (TestCS == i_shape)) {
                m_histResidPlaneRHUnBiasUV[i_angleU * m_anglesV + i_angleV]->Fill(m_ResidUPlaneRHUnBias / Unit::um,
                    m_ResidVPlaneRHUnBias / Unit::um);
                m_histResidPlaneRHUnBiasUV[m_anglesU * m_anglesV]->Fill(m_ResidUPlaneRHUnBias / Unit::um, m_ResidVPlaneRHUnBias / Unit::um);
                m_histSigmaRecoUV[i_angleU * m_anglesV + i_angleV]->Fill(m_SigmaUReco / Unit::um, m_SigmaVReco / Unit::um);
                m_histSigmaRecoUV[m_anglesU * m_anglesV]->Fill(m_SigmaUReco / Unit::um, m_SigmaVReco / Unit::um);
                m_histResidTrueRHUV[i_angleU * m_anglesV + i_angleV]->Fill(m_ResidUTrueCl / Unit::um, m_ResidVTrueCl / Unit::um);
                m_histResidTrueRHUV[m_anglesU * m_anglesV]->Fill(m_ResidUTrueCl / Unit::um, m_ResidVTrueCl / Unit::um);
              }
            }

            double RetVal = 0.0;       // return value of mean of selection
            double RetValError = 0.0;  // return value of meanError of selection
            double RetRMS = 0.0;       // return value of RMS of selection

            // Calculation of bias = mean of distribution:
            if (CalculateCorrection(1, nSelRows, Colm1, &RetVal, &RetValError, &RetRMS)) {
              TCorrection_BiasMap[make_tuple(i_shape, i_pk, 0, i_angleU, i_angleV)] = RetVal;
              TCorrection_BiasMapErr[make_tuple(i_shape, i_pk, 0, i_angleU, i_angleV)] = RetValError;
              SummariesInfo[2]++;
              SummariesInfo[4] += nSelRows;
              if (m_DoExpertHistograms) {
                m_histBiasCounterU->SetBinContent(i_angleU + 1, i_angleV + 1,
                                                  m_histBiasCounterU->GetBinContent(i_angleU + 1, i_angleV + 1) + 1);
                m_histBiasCorrectionU[i_pk * m_shapes + i_shape]->SetBinContent(i_angleU + 1, i_angleV + 1, RetVal);
                m_histBiasCorrectionErrorU[i_pk * m_shapes + i_shape]->SetBinContent(i_angleU + 1, i_angleV + 1, RetValError);
                m_histResidualRMSU[i_pk * m_shapes + i_shape]->SetBinContent(i_angleU + 1, i_angleV + 1, RetRMS);
                m_histBiasCorrectionU[m_pixelkinds * m_shapes]->SetBinContent(i_angleU + 1, i_angleV + 1,
                    m_histBiasCorrectionU[m_pixelkinds * m_shapes]->GetBinContent(i_angleU + 1, i_angleV + 1) + RetVal);
                m_histBiasCorrectionErrorU[m_pixelkinds * m_shapes]->SetBinContent(i_angleU + 1, i_angleV + 1,
                    m_histBiasCorrectionErrorU[m_pixelkinds * m_shapes]->GetBinContent(i_angleU + 1, i_angleV + 1) + RetValError);
                m_histResidualRMSU[m_pixelkinds * m_shapes]->SetBinContent(i_angleU + 1, i_angleV + 1,
                                                                           m_histResidualRMSU[m_pixelkinds * m_shapes]->GetBinContent(i_angleU + 1, i_angleV + 1) + RetRMS);
              }
            }
            if (CalculateCorrection(1, nSelRows, Colm2, &RetVal, &RetValError, &RetRMS)) {
              TCorrection_BiasMap[make_tuple(i_shape, i_pk, 1, i_angleU, i_angleV)] = RetVal;
              TCorrection_BiasMapErr[make_tuple(i_shape, i_pk, 1, i_angleU, i_angleV)] = RetValError;
              SummariesInfo[3]++;
              SummariesInfo[5] += nSelRows;
              if (m_DoExpertHistograms) {
                m_histBiasCounterV->SetBinContent(i_angleU + 1, i_angleV + 1,
                                                  m_histBiasCounterV->GetBinContent(i_angleU + 1, i_angleV + 1) + 1);
                m_histBiasCorrectionV[i_pk * m_shapes + i_shape]->SetBinContent(i_angleU + 1, i_angleV + 1, RetVal);
                m_histBiasCorrectionErrorV[i_pk * m_shapes + i_shape]->SetBinContent(i_angleU + 1, i_angleV + 1, RetValError);
                m_histResidualRMSV[i_pk * m_shapes + i_shape]->SetBinContent(i_angleU + 1, i_angleV + 1, RetRMS);
                m_histBiasCorrectionV[m_pixelkinds * m_shapes]->SetBinContent(i_angleU + 1, i_angleV + 1,
                    m_histBiasCorrectionV[m_pixelkinds * m_shapes]->GetBinContent(i_angleU + 1, i_angleV + 1) + RetVal);
                m_histBiasCorrectionErrorV[m_pixelkinds * m_shapes]->SetBinContent(i_angleU + 1, i_angleV + 1,
                    m_histBiasCorrectionErrorV[m_pixelkinds * m_shapes]->GetBinContent(i_angleU + 1, i_angleV + 1) + RetValError);
                m_histResidualRMSV[m_pixelkinds * m_shapes]->SetBinContent(i_angleU + 1, i_angleV + 1,
                                                                           m_histResidualRMSV[m_pixelkinds * m_shapes]->GetBinContent(i_angleU + 1, i_angleV + 1) + RetRMS);
              }
            }
            B2DEBUG(30, "BIAS: Selected raws " << nSelRows << " sh" <<
                    i_shape << " pk" << i_pk << " angU" << i_angleU << " angV" << i_angleV << " biasU " <<
                    TCorrection_BiasMap[make_tuple(i_shape, i_pk, 0, i_angleU, i_angleV)] * 10000.0 << " biasV " <<
                    TCorrection_BiasMap[make_tuple(i_shape, i_pk, 1, i_angleU, i_angleV)] * 10000.0 << " um"
                   );

            // Error Estimation corrections:
            if (m_UsePixelKind > -1) {  // EE calculation from true points only
              for (int i = 0; i < nSelRows; i++) {
                // Remove bias from residual:
                Colm1[i] += TCorrection_BiasMap[make_tuple(i_shape, i_pk, 0, i_angleU, i_angleV)];
                Colm2[i] += TCorrection_BiasMap[make_tuple(i_shape, i_pk, 1, i_angleU, i_angleV)];
                // Create most realistic ratio of residual / residError --> should go to ~1
                Colm5[i] = Colm1[i] / Colm3[i];
                Colm6[i] = Colm2[i] / Colm4[i];
              }
              // 1:rTrueCl, 3: sCl, 5:ratioResEE
              for (int iAxis = 0; iAxis < 2; iAxis++) {
                TString sAxis = "u";
                if (iAxis == 1) sAxis = Form("v");
                int DoCalcul = 0;
                if (iAxis == 0) DoCalcul = CalculateCorrection(2, nSelRows, Colm5, &RetVal, &RetValError, &RetRMS);
                else DoCalcul = CalculateCorrection(2, nSelRows, Colm6, &RetVal, &RetValError, &RetRMS);
                if (DoCalcul) {
                  double SigREE = RetRMS;  // calculation of RMS of R/EE distribution
                  double CorSigREE = GetCorrectionResErrEst(1.0 / SigREE) / SigREE;  // Correction for SigREE
                  if (iAxis == 0) CalculateCorrection(0, nSelRows, Colm3, &RetVal, &RetValError, &RetRMS);
                  else CalculateCorrection(0, nSelRows, Colm4, &RetVal, &RetValError, &RetRMS);
                  double OrigEE = RetVal;  // original EE
                  double OrigEEErr = RetValError;  // original EE
                  double GoodEE = OrigEE * SigREE / GetCorrectionResErrEst(1.0 / SigREE);  // Corrected EE
                  double CorEE = SigREE / GetCorrectionResErrEst(1.0 / SigREE);  // Correction EE
                  double ApplyCorEE = OrigEE * CorEE;  // Test Correction EE
                  // this RMS should be real spread of hits so should meet with ErroEstimation:
                  if (iAxis == 0) DoCalcul = (CalculateCorrection(2, nSelRows, Colm1, &RetVal, &RetValError, &RetRMS));
                  else DoCalcul = CalculateCorrection(2, nSelRows, Colm2, &RetVal, &RetValError, &RetRMS);
                  double TrueEE = RetRMS;
                  double UseFinCorrection = TrueEE / OrigEE;  // CorEE
                  B2DEBUG(30, "ERROR ESTIMATION in " << sAxis.Data() << ": Selected raws " << nSelRows << " sh" <<
                          i_shape << " pk" << i_pk << " angU" << i_angleU << " angV" << i_angleV <<
                          " sigREE " << SigREE << " cor " << CorSigREE <<  " orig " << OrigEE <<
                          "+-" << OrigEEErr <<  " good " << GoodEE << " = true " << TrueEE <<
                          " FinCorr " << CorEE << " ApplyCorEE " << ApplyCorEE
                         );
                  if (fabs(ApplyCorEE - TrueEE) > 200000000.0 * Unit::um) {  // Not use yet
                    B2INFO("Discrepancy between corrected and true value of ERROR ESTIMATION! " <<
                           "Selected raws " << nSelRows << " sh" <<
                           i_shape << " pk" << i_pk << " angU" << i_angleU << " angV" << i_angleV <<
                           " sigREE " << SigREE << " cor " << CorSigREE <<  " orig " << OrigEE <<
                           "+-" << OrigEEErr <<  " good " << GoodEE << " = true " << TrueEE <<
                           " FinCorr " << CorEE << " ApplyCorEE " << ApplyCorEE
                          );
                  } else {
                    if (fabs(UseFinCorrection - 1.0) > m_ApplyEECorrectionCut) {
                      TCorrection_ErrorEstimationMap[make_tuple(i_shape, i_pk, iAxis, i_angleU, i_angleV)] = UseFinCorrection - 1.0;
                      SummariesInfo[6 + iAxis]++;
                      SummariesInfo[8 + iAxis] += nSelRows;
                      if (m_DoExpertHistograms) {
                        if (iAxis == 0) {
                          m_histEECounterU->SetBinContent(i_angleU + 1, i_angleV + 1,
                                                          m_histEECounterU->GetBinContent(i_angleU + 1, i_angleV + 1) + 1);
                          m_histCorrEE_TrueU[i_pk * m_shapes + i_shape]->SetBinContent(i_angleU + 1, i_angleV + 1, CorEE);
                          m_histCorrEE_TrueU[m_pixelkinds * m_shapes]->SetBinContent(i_angleU + 1, i_angleV + 1,
                                                                                     m_histCorrEE_TrueU[m_pixelkinds * m_shapes]->GetBinContent(i_angleU + 1, i_angleV + 1) + CorEE);
                          m_histDiffClEETrueOrigEEU[i_pk * m_shapes + i_shape]->SetBinContent(i_angleU + 1, i_angleV + 1, TrueEE - OrigEE);
                          m_histDiffClEETrueOrigEEU[m_pixelkinds * m_shapes]->SetBinContent(i_angleU + 1, i_angleV + 1,
                              m_histDiffClEETrueOrigEEU[m_pixelkinds * m_shapes]->GetBinContent(i_angleU + 1, i_angleV + 1) + TrueEE - OrigEE);
                          m_histClusterTrueEEU[i_pk * m_shapes + i_shape]->SetBinContent(i_angleU + 1, i_angleV + 1, TrueEE);
                          m_histClusterTrueEEU[m_pixelkinds * m_shapes]->SetBinContent(i_angleU + 1, i_angleV + 1,
                              m_histClusterTrueEEU[m_pixelkinds * m_shapes]->GetBinContent(i_angleU + 1, i_angleV + 1) + TrueEE);
                          m_histClusterOrigEEU[i_pk * m_shapes + i_shape]->SetBinContent(i_angleU + 1, i_angleV + 1, OrigEE);
                          m_histClusterOrigEEU[m_pixelkinds * m_shapes]->SetBinContent(i_angleU + 1, i_angleV + 1,
                              m_histClusterOrigEEU[m_pixelkinds * m_shapes]->GetBinContent(i_angleU + 1, i_angleV + 1) + OrigEE);
                        } else {
                          m_histEECounterV->SetBinContent(i_angleU + 1, i_angleV + 1,
                                                          m_histEECounterV->GetBinContent(i_angleU + 1, i_angleV + 1) + 1);
                          m_histCorrEE_TrueV[i_pk * m_shapes + i_shape]->SetBinContent(i_angleU + 1, i_angleV + 1, CorEE);
                          m_histCorrEE_TrueV[m_pixelkinds * m_shapes]->SetBinContent(i_angleU + 1, i_angleV + 1,
                                                                                     m_histCorrEE_TrueV[m_pixelkinds * m_shapes]->GetBinContent(i_angleU + 1, i_angleV + 1) + CorEE);
                          m_histDiffClEETrueOrigEEV[i_pk * m_shapes + i_shape]->SetBinContent(i_angleU + 1, i_angleV + 1, TrueEE - OrigEE);
                          m_histDiffClEETrueOrigEEV[m_pixelkinds * m_shapes]->SetBinContent(i_angleU + 1, i_angleV + 1,
                              m_histDiffClEETrueOrigEEV[m_pixelkinds * m_shapes]->GetBinContent(i_angleU + 1, i_angleV + 1) + TrueEE - OrigEE);
                          m_histClusterTrueEEV[i_pk * m_shapes + i_shape]->SetBinContent(i_angleU + 1, i_angleV + 1, TrueEE);
                          m_histClusterTrueEEV[m_pixelkinds * m_shapes]->SetBinContent(i_angleU + 1, i_angleV + 1,
                              m_histClusterTrueEEV[m_pixelkinds * m_shapes]->GetBinContent(i_angleU + 1, i_angleV + 1) + TrueEE);
                          m_histClusterOrigEEV[i_pk * m_shapes + i_shape]->SetBinContent(i_angleU + 1, i_angleV + 1, OrigEE);
                          m_histClusterOrigEEV[m_pixelkinds * m_shapes]->SetBinContent(i_angleU + 1, i_angleV + 1,
                              m_histClusterOrigEEV[m_pixelkinds * m_shapes]->GetBinContent(i_angleU + 1, i_angleV + 1) + OrigEE);
                        }
                      }
                    }
                  }
                }
              }
            } else if (m_UsePixelKind == -1) { // calculation from tracks only:
              for (int i = 0; i < nSelRows; i++) {
                // Remove bias from residual:
                Colm1[i] += TCorrection_BiasMap[make_tuple(i_shape, i_pk, 0, i_angleU, i_angleV)];
                Colm2[i] += TCorrection_BiasMap[make_tuple(i_shape, i_pk, 1, i_angleU, i_angleV)];
                Colm9[i] += TCorrection_BiasMap[make_tuple(i_shape, i_pk, 0, i_angleU, i_angleV)];
                Colm10[i] += TCorrection_BiasMap[make_tuple(i_shape, i_pk, 1, i_angleU, i_angleV)];
                //    Colm9[i] *= 3.0;
                //    Colm10[i] *= 3.0;
              }
              // 1:rPlRH, 3: sCl, 5:sTrack, 9:rTrueCl, 13:rTruePl
              for (int iAxis = 0; iAxis < 2; iAxis++) {
                TString sAxis = "u";
                if (iAxis == 1) sAxis = Form("v");
                int DoCalcul = 0;
                if (m_StoreEECorrectionType == 0) {
                  if (iAxis == 0) DoCalcul = CalculateCorrection(0, nSelRows, Colm9, &RetVal, &RetValError, &RetRMS);
                  else DoCalcul = CalculateCorrection(0, nSelRows, Colm10, &RetVal, &RetValError, &RetRMS);
                } else {
                  if (iAxis == 0) DoCalcul = CalculateCorrection(0, nSelRows, Colm1, &RetVal, &RetValError, &RetRMS);
                  else DoCalcul = CalculateCorrection(0, nSelRows, Colm2, &RetVal, &RetValError, &RetRMS);
                }
                if (iAxis == 0) CalculateCorrection(0, nSelRows, Colm1, &RetVal, &RetValError, &RetRMS);
                else CalculateCorrection(0, nSelRows, Colm2, &RetVal, &RetValError, &RetRMS);
                if (DoCalcul) {
                  double ResRMSEE = RetRMS;  // RMS residuals
                  if (iAxis == 0) CalculateCorrection(0, nSelRows, Colm9, &RetVal, &RetValError, &RetRMS);
                  else CalculateCorrection(0, nSelRows, Colm10, &RetVal, &RetValError, &RetRMS);
                  double ClusterTrueEE = RetRMS;  // True-Cluster RMS=EE
                  if (iAxis == 0) CalculateCorrection(0, nSelRows, Colm3, &RetVal, &RetValError, &RetRMS);
                  else CalculateCorrection(0, nSelRows, Colm4, &RetVal, &RetValError, &RetRMS);
                  double ClusterOrigEE = RetVal;  // EE from clusters
                  if (iAxis == 0) CalculateCorrection(0, nSelRows, Colm13, &RetVal, &RetValError, &RetRMS);
                  else CalculateCorrection(0, nSelRows, Colm14, &RetVal, &RetValError, &RetRMS);
                  double TrackRMSResEE = RetRMS;  // RMS of Resids of tracks only
                  if (iAxis == 0) CalculateCorrection(0, nSelRows, Colm5, &RetVal, &RetValError, &RetRMS);
                  else CalculateCorrection(0, nSelRows, Colm6, &RetVal, &RetValError, &RetRMS);
                  double TrackMeanEE = RetVal;  // EE of tracks
                  double ClustCalcTTEE = 0.0;
                  if (ResRMSEE > TrackMeanEE)
                    ClustCalcTTEE = sqrt(ResRMSEE * ResRMSEE - TrackMeanEE * TrackMeanEE);
                  double ClustCalcRTEE = 0.0;
                  if (ResRMSEE > TrackRMSResEE)
                    ClustCalcRTEE = sqrt(ResRMSEE * ResRMSEE - TrackRMSResEE * TrackRMSResEE);

                  double DiffClEETrueOrigEE = ClusterTrueEE - ClusterOrigEE;
                  double DiffClEETrueCalcTTEE = ClusterTrueEE - ClustCalcTTEE;
                  double DiffClEETrueCalcRTEE = ClusterTrueEE - ClustCalcRTEE;
                  double CorrEE_True = ClusterTrueEE / ClusterOrigEE;
                  double CorrEE_TrueTrack = ClustCalcTTEE / ClusterOrigEE;
                  double CorrEE_RealTrack = ClustCalcRTEE / ClusterOrigEE;

                  double UseFinCorrection = CorrEE_True;
                  if (m_StoreEECorrectionType == 1) UseFinCorrection = CorrEE_TrueTrack;
                  if (m_StoreEECorrectionType == 2) UseFinCorrection = CorrEE_RealTrack;

                  UseFinCorrection = UseFinCorrection - 1.0;  // to set correction to arround 0.0
                  // Apply only in case correction is higher than 0.1
                  if (fabs(UseFinCorrection) > m_ApplyEECorrectionCut) {
                    TCorrection_ErrorEstimationMap[make_tuple(i_shape, i_pk, iAxis, i_angleU, i_angleV)] = UseFinCorrection;
                    SummariesInfo[6 + iAxis]++;
                    SummariesInfo[8 + iAxis] += nSelRows;
                    if (m_DoExpertHistograms) {
                      if (iAxis == 0) {
                        m_histEECounterU->SetBinContent(i_angleU + 1, i_angleV + 1,
                                                        m_histEECounterU->GetBinContent(i_angleU + 1, i_angleV + 1) + 1);
                        m_histCorrEE_TrueU[i_pk * m_shapes + i_shape]->SetBinContent(i_angleU + 1, i_angleV + 1, CorrEE_True);
                        m_histCorrEE_TrueU[m_pixelkinds * m_shapes]->SetBinContent(i_angleU + 1, i_angleV + 1,
                                                                                   m_histCorrEE_TrueU[m_pixelkinds * m_shapes]->GetBinContent(i_angleU + 1, i_angleV + 1) + CorrEE_True);
                        m_histCorrEE_TrueTrackU[i_pk * m_shapes + i_shape]->SetBinContent(i_angleU + 1, i_angleV + 1, CorrEE_TrueTrack);
                        m_histCorrEE_RealTrackU[i_pk * m_shapes + i_shape]->SetBinContent(i_angleU + 1, i_angleV + 1, CorrEE_RealTrack);
                        m_histResRMSEEU[i_pk * m_shapes + i_shape]->SetBinContent(i_angleU + 1, i_angleV + 1, ResRMSEE);
                        m_histDiffClEETrueOrigEEU[i_pk * m_shapes + i_shape]->SetBinContent(i_angleU + 1, i_angleV + 1, DiffClEETrueOrigEE);
                        m_histDiffClEETrueCalcTTEEU[i_pk * m_shapes + i_shape]->SetBinContent(i_angleU + 1, i_angleV + 1, DiffClEETrueCalcTTEE);
                        m_histDiffClEETrueCalcRTEEU[i_pk * m_shapes + i_shape]->SetBinContent(i_angleU + 1, i_angleV + 1, DiffClEETrueCalcRTEE);
                        m_histClusterTrueEEU[i_pk * m_shapes + i_shape]->SetBinContent(i_angleU + 1, i_angleV + 1, ClusterTrueEE);
                        m_histClusterOrigEEU[i_pk * m_shapes + i_shape]->SetBinContent(i_angleU + 1, i_angleV + 1, ClusterOrigEE);
                        m_histClustCalcTTEEU[i_pk * m_shapes + i_shape]->SetBinContent(i_angleU + 1, i_angleV + 1, ClustCalcTTEE);
                        m_histClustCalcRTEEU[i_pk * m_shapes + i_shape]->SetBinContent(i_angleU + 1, i_angleV + 1, ClustCalcRTEE);

                        m_histCorrEE_TrueTrackU[m_pixelkinds * m_shapes]->SetBinContent(i_angleU + 1, i_angleV + 1,
                            m_histCorrEE_TrueTrackU[m_pixelkinds * m_shapes]->GetBinContent(i_angleU + 1, i_angleV + 1) + CorrEE_TrueTrack);
                        m_histCorrEE_RealTrackU[m_pixelkinds * m_shapes]->SetBinContent(i_angleU + 1, i_angleV + 1,
                            m_histCorrEE_RealTrackU[m_pixelkinds * m_shapes]->GetBinContent(i_angleU + 1, i_angleV + 1) + CorrEE_RealTrack);
                        m_histResRMSEEU[m_pixelkinds * m_shapes]->SetBinContent(i_angleU + 1, i_angleV + 1,
                                                                                m_histResRMSEEU[m_pixelkinds * m_shapes]->GetBinContent(i_angleU + 1, i_angleV + 1) + ResRMSEE);
                        m_histDiffClEETrueOrigEEU[m_pixelkinds * m_shapes]->SetBinContent(i_angleU + 1, i_angleV + 1,
                            m_histDiffClEETrueOrigEEU[m_pixelkinds * m_shapes]->GetBinContent(i_angleU + 1, i_angleV + 1) + DiffClEETrueOrigEE);
                        m_histDiffClEETrueCalcTTEEU[m_pixelkinds * m_shapes]->SetBinContent(i_angleU + 1, i_angleV + 1,
                            m_histDiffClEETrueCalcTTEEU[m_pixelkinds * m_shapes]->GetBinContent(i_angleU + 1, i_angleV + 1) + DiffClEETrueCalcTTEE);
                        m_histDiffClEETrueCalcRTEEU[m_pixelkinds * m_shapes]->SetBinContent(i_angleU + 1, i_angleV + 1,
                            m_histDiffClEETrueCalcRTEEU[m_pixelkinds * m_shapes]->GetBinContent(i_angleU + 1, i_angleV + 1) + DiffClEETrueCalcRTEE);
                        m_histClusterTrueEEU[m_pixelkinds * m_shapes]->SetBinContent(i_angleU + 1, i_angleV + 1,
                            m_histClusterTrueEEU[m_pixelkinds * m_shapes]->GetBinContent(i_angleU + 1, i_angleV + 1) + ClusterTrueEE);
                        m_histClusterOrigEEU[m_pixelkinds * m_shapes]->SetBinContent(i_angleU + 1, i_angleV + 1,
                            m_histClusterOrigEEU[m_pixelkinds * m_shapes]->GetBinContent(i_angleU + 1, i_angleV + 1) + ClusterOrigEE);
                        m_histClustCalcTTEEU[m_pixelkinds * m_shapes]->SetBinContent(i_angleU + 1, i_angleV + 1,
                            m_histClustCalcTTEEU[m_pixelkinds * m_shapes]->GetBinContent(i_angleU + 1, i_angleV + 1) + ClustCalcTTEE);
                        m_histClustCalcRTEEU[m_pixelkinds * m_shapes]->SetBinContent(i_angleU + 1, i_angleV + 1,
                            m_histClustCalcRTEEU[m_pixelkinds * m_shapes]->GetBinContent(i_angleU + 1, i_angleV + 1) + ClustCalcRTEE);
                      } else {
                        m_histEECounterV->SetBinContent(i_angleU + 1, i_angleV + 1,
                                                        m_histEECounterV->GetBinContent(i_angleU + 1, i_angleV + 1) + 1);
                        m_histCorrEE_TrueV[i_pk * m_shapes + i_shape]->SetBinContent(i_angleU + 1, i_angleV + 1, CorrEE_True);
                        m_histCorrEE_TrueV[m_pixelkinds * m_shapes]->SetBinContent(i_angleU + 1, i_angleV + 1,
                                                                                   m_histCorrEE_TrueV[m_pixelkinds * m_shapes]->GetBinContent(i_angleU + 1, i_angleV + 1) + CorrEE_True);
                        m_histCorrEE_TrueTrackV[i_pk * m_shapes + i_shape]->SetBinContent(i_angleU + 1, i_angleV + 1, CorrEE_TrueTrack);
                        m_histCorrEE_RealTrackV[i_pk * m_shapes + i_shape]->SetBinContent(i_angleU + 1, i_angleV + 1, CorrEE_RealTrack);
                        m_histResRMSEEV[i_pk * m_shapes + i_shape]->SetBinContent(i_angleU + 1, i_angleV + 1, ResRMSEE);
                        m_histDiffClEETrueOrigEEV[i_pk * m_shapes + i_shape]->SetBinContent(i_angleU + 1, i_angleV + 1, DiffClEETrueOrigEE);
                        m_histDiffClEETrueCalcTTEEV[i_pk * m_shapes + i_shape]->SetBinContent(i_angleU + 1, i_angleV + 1, DiffClEETrueCalcTTEE);
                        m_histDiffClEETrueCalcRTEEV[i_pk * m_shapes + i_shape]->SetBinContent(i_angleU + 1, i_angleV + 1, DiffClEETrueCalcRTEE);
                        m_histClusterTrueEEV[i_pk * m_shapes + i_shape]->SetBinContent(i_angleU + 1, i_angleV + 1, ClusterTrueEE);
                        m_histClusterOrigEEV[i_pk * m_shapes + i_shape]->SetBinContent(i_angleU + 1, i_angleV + 1, ClusterOrigEE);
                        m_histClustCalcTTEEV[i_pk * m_shapes + i_shape]->SetBinContent(i_angleU + 1, i_angleV + 1, ClustCalcTTEE);
                        m_histClustCalcRTEEV[i_pk * m_shapes + i_shape]->SetBinContent(i_angleU + 1, i_angleV + 1, ClustCalcRTEE);

                        m_histCorrEE_TrueTrackV[m_pixelkinds * m_shapes]->SetBinContent(i_angleU + 1, i_angleV + 1,
                            m_histCorrEE_TrueTrackV[m_pixelkinds * m_shapes]->GetBinContent(i_angleU + 1, i_angleV + 1) + CorrEE_TrueTrack);
                        m_histCorrEE_RealTrackV[m_pixelkinds * m_shapes]->SetBinContent(i_angleU + 1, i_angleV + 1,
                            m_histCorrEE_RealTrackV[m_pixelkinds * m_shapes]->GetBinContent(i_angleU + 1, i_angleV + 1) + CorrEE_RealTrack);
                        m_histResRMSEEV[m_pixelkinds * m_shapes]->SetBinContent(i_angleU + 1, i_angleV + 1,
                                                                                m_histResRMSEEV[m_pixelkinds * m_shapes]->GetBinContent(i_angleU + 1, i_angleV + 1) + ResRMSEE);
                        m_histDiffClEETrueOrigEEV[m_pixelkinds * m_shapes]->SetBinContent(i_angleU + 1, i_angleV + 1,
                            m_histDiffClEETrueOrigEEV[m_pixelkinds * m_shapes]->GetBinContent(i_angleU + 1, i_angleV + 1) + DiffClEETrueOrigEE);
                        m_histDiffClEETrueCalcTTEEV[m_pixelkinds * m_shapes]->SetBinContent(i_angleU + 1, i_angleV + 1,
                            m_histDiffClEETrueCalcTTEEV[m_pixelkinds * m_shapes]->GetBinContent(i_angleU + 1, i_angleV + 1) + DiffClEETrueCalcTTEE);
                        m_histDiffClEETrueCalcRTEEV[m_pixelkinds * m_shapes]->SetBinContent(i_angleU + 1, i_angleV + 1,
                            m_histDiffClEETrueCalcRTEEV[m_pixelkinds * m_shapes]->GetBinContent(i_angleU + 1, i_angleV + 1) + DiffClEETrueCalcRTEE);
                        m_histClusterTrueEEV[m_pixelkinds * m_shapes]->SetBinContent(i_angleU + 1, i_angleV + 1,
                            m_histClusterTrueEEV[m_pixelkinds * m_shapes]->GetBinContent(i_angleU + 1, i_angleV + 1) + ClusterTrueEE);
                        m_histClusterOrigEEV[m_pixelkinds * m_shapes]->SetBinContent(i_angleU + 1, i_angleV + 1,
                            m_histClusterOrigEEV[m_pixelkinds * m_shapes]->GetBinContent(i_angleU + 1, i_angleV + 1) + ClusterOrigEE);
                        m_histClustCalcTTEEV[m_pixelkinds * m_shapes]->SetBinContent(i_angleU + 1, i_angleV + 1,
                            m_histClustCalcTTEEV[m_pixelkinds * m_shapes]->GetBinContent(i_angleU + 1, i_angleV + 1) + ClustCalcTTEE);
                        m_histClustCalcRTEEV[m_pixelkinds * m_shapes]->SetBinContent(i_angleU + 1, i_angleV + 1,
                            m_histClustCalcRTEEV[m_pixelkinds * m_shapes]->GetBinContent(i_angleU + 1, i_angleV + 1) + ClustCalcRTEE);
                      }
                    }
                  }
                  B2DEBUG(30, "ERROR ESTIMATION: Selected raws " << nSelRows << " axis " << sAxis.Data() << " sh" <<
                          i_shape << " pk" << i_pk << " angU" << i_angleU << " angV" << i_angleV <<
                          "\n    Track:   meanEE " << TrackMeanEE << " rmsRes " << TrackRMSResEE <<
                          "\n    Cluster: origEE " << ClusterOrigEE << " trueEE " << ClusterTrueEE <<
                          " calcEETracks " << ClustCalcTTEE << " calcResTracks " << ClustCalcRTEE <<
                          "\n    Differ:  TOrig " << DiffClEETrueOrigEE <<
                          " TEETr " << DiffClEETrueCalcTTEE << " TEERes " << DiffClEETrueCalcRTEE <<
                          "\n    Correct: TrueO " << CorrEE_True << " EETrO(data) " << CorrEE_TrueTrack <<
                          " EEResO " << CorrEE_RealTrack << " (data from true track dispersion - simuls)\n"
                         );
                }
              }
            }

            if (m_DoExpertHistograms) {
              m_histnClusters[i_pk * m_shapes + i_shape]->SetBinContent(i_angleU + 1, i_angleV + 1, nSelRows);
              m_histnClusters[m_pixelkinds * m_shapes]->SetBinContent(i_angleU + 1, i_angleV + 1,
                                                                      m_histnClusters[m_pixelkinds * m_shapes]->GetBinContent(i_angleU + 1, i_angleV + 1) + nSelRows);
              m_histnClusterFraction[i_pk * m_shapes + i_shape]->SetBinContent(i_angleU + 1, i_angleV + 1, (float)nSelRows / nEntries);
              m_histnClusterFraction[m_pixelkinds * m_shapes]->SetBinContent(i_angleU + 1, i_angleV + 1,
                  (float)m_histnClusters[m_pixelkinds * m_shapes]->GetBinContent(i_angleU + 1, i_angleV + 1) / nEntries);
            }

            if (m_MinHitsInPixels > 0) if (nSelRows >= m_MinHitsInPixels) {
                TString InPixCaseName;
                if (m_UsePixelKind == -1) {
                  InPixCaseName = Form("Reco");
                } else {
                  InPixCaseName = Form("True");
                }
                TString sVarexp = Form("(InPixU%s + 0.5) : (InPixV%s + 0.5)", InPixCaseName.Data(), InPixCaseName.Data());
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
                    cCat = Form("(InPixU%s + 0.5) > %f", InPixCaseName.Data(), MinValU);
                    c6a.SetTitle(cCat.Data());
                    cCat = Form("(InPixU%s + 0.5) < %f", InPixCaseName.Data(), MaxValU);
                    c6b.SetTitle(cCat.Data());
                    cCat = Form("(InPixV%s + 0.5) > %f", InPixCaseName.Data(), MinValV);
                    c7a.SetTitle(cCat.Data());
                    cCat = Form("(InPixV%s + 0.5) < %f", InPixCaseName.Data(), MaxValV);
                    c7b.SetTitle(cCat.Data());

                    cFin = c6a && c6b && c7a && c7b && c1 && c2 && c3a && c3b && c4a && c4b && c5;

                    tree->Draw(sVarexp.Data(), cFin, "goff");
                    int nSelRows2 = (int)tree->GetSelectedRows();
                    if (nSelRows2 >= m_MinHitsAcceptInPixels) {
                      TInPixelPositionMap[make_tuple(i_shape, i_pk, i_angleU, i_angleV, i_ipU, i_ipV)] = 1.0;  // Dummy value
                    }
                  }
                }
              }
          }
        }
      }
    }
  }
  m_histBiasCorrectionU[m_pixelkinds * m_shapes]->Divide(m_histBiasCounterU);
  m_histBiasCorrectionErrorU[m_pixelkinds * m_shapes]->Divide(m_histBiasCounterU);
  m_histResidualRMSU[m_pixelkinds * m_shapes]->Divide(m_histBiasCounterU);
  m_histBiasCorrectionV[m_pixelkinds * m_shapes]->Divide(m_histBiasCounterV);
  m_histBiasCorrectionErrorV[m_pixelkinds * m_shapes]->Divide(m_histBiasCounterV);
  m_histResidualRMSV[m_pixelkinds * m_shapes]->Divide(m_histBiasCounterV);

  m_histCorrEE_TrueU[m_pixelkinds * m_shapes]->Divide(m_histEECounterU);
  m_histCorrEE_TrueTrackU[m_pixelkinds * m_shapes]->Divide(m_histEECounterU);
  m_histCorrEE_RealTrackU[m_pixelkinds * m_shapes]->Divide(m_histEECounterU);
  m_histResRMSEEU[m_pixelkinds * m_shapes]->Divide(m_histEECounterU);
  m_histDiffClEETrueOrigEEU[m_pixelkinds * m_shapes]->Divide(m_histEECounterU);
  m_histDiffClEETrueCalcTTEEU[m_pixelkinds * m_shapes]->Divide(m_histEECounterU);
  m_histDiffClEETrueCalcRTEEU[m_pixelkinds * m_shapes]->Divide(m_histEECounterU);
  m_histClusterTrueEEU[m_pixelkinds * m_shapes]->Divide(m_histEECounterU);
  m_histClusterOrigEEU[m_pixelkinds * m_shapes]->Divide(m_histEECounterU);
  m_histClustCalcTTEEU[m_pixelkinds * m_shapes]->Divide(m_histEECounterU);
  m_histClustCalcRTEEU[m_pixelkinds * m_shapes]->Divide(m_histEECounterU);

  m_histCorrEE_TrueV[m_pixelkinds * m_shapes]->Divide(m_histEECounterV);
  m_histCorrEE_TrueTrackV[m_pixelkinds * m_shapes]->Divide(m_histEECounterV);
  m_histCorrEE_RealTrackV[m_pixelkinds * m_shapes]->Divide(m_histEECounterV);
  m_histResRMSEEV[m_pixelkinds * m_shapes]->Divide(m_histEECounterV);
  m_histDiffClEETrueOrigEEV[m_pixelkinds * m_shapes]->Divide(m_histEECounterV);
  m_histDiffClEETrueCalcTTEEV[m_pixelkinds * m_shapes]->Divide(m_histEECounterV);
  m_histDiffClEETrueCalcRTEEV[m_pixelkinds * m_shapes]->Divide(m_histEECounterV);
  m_histClusterTrueEEV[m_pixelkinds * m_shapes]->Divide(m_histEECounterV);
  m_histClusterOrigEEV[m_pixelkinds * m_shapes]->Divide(m_histEECounterV);
  m_histClustCalcTTEEV[m_pixelkinds * m_shapes]->Divide(m_histEECounterV);
  m_histClustCalcRTEEV[m_pixelkinds * m_shapes]->Divide(m_histEECounterV);

  B2DEBUG(30, "--> bias correction and error estimation correction calculation done. ");

  Table2Vector(TCorrection_BiasMap, Correction_Bias);
  Table2Vector(TCorrection_ErrorEstimationMap, Correction_ErrorEstimation);
  Table2Vector(TCorrection_ErrorEstimationCovarianceMap, Correction_ErrorEstimationCovariance);
  Table2Vector(TCorrection_BiasMapErr, Correction_BiasErr);
  Table2Vector(TInPixelPositionMap, InPixelPosition);

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
  TextSh = Form("CorrectionsStatistics_%s.log", name_Case.Data());
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
                  (int)(i_pk / 4) + 1, (int)((i_pk % 4) / 2) + 1,
                  (1 - i_pk % 2) * (1 - (int)((i_pk % 4) / 2)) + i_pk % 2 * (int)((i_pk % 4) / 2),
                  SummariesInfoPK[i_pk],
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
    TextSh = Form("**      ShapeID %02i: %7i  (%6.2f %%)   (%s)", i_shape + 1, SummariesInfoSh[i_shape],
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
    fDQM->Write();
    fDQM->Close();
    fExpertHistograms->Write();
    fExpertHistograms->Close();

  }

  // Here to save corrections in TVectorT format, could change later
  // Use TFile for temporary use,
  // TODO Final use with saveCalibration and database.
  TString nameBS = Form("PXDShCalibrationBasicSetting");
  TString nameB = Form("Correction_Bias");
  TString nameBE = Form("Correction_BiasErr");
  TString nameEE = Form("Correction_ErrorEstimation");
  TString nameEC = Form("Correction_ErrorEstimationCovariance");
  TString nameIP = Form("InPixelPosition");
  TFile* f = new TFile(name_OutFileCalibrations.Data(), "recreate");
  PXDShCalibrationBasicSetting->Write(nameBS.Data());
  Correction_Bias->Write(nameB.Data());
  Correction_BiasErr->Write(nameBE.Data());
  Correction_ErrorEstimation->Write(nameEE.Data());
  Correction_ErrorEstimationCovariance->Write(nameEC.Data());
  InPixelPosition->Write(nameIP.Data());
  f->Close();

  saveCalibration(Correction_Bias, nameB.Data());
  saveCalibration(Correction_ErrorEstimation, nameEE.Data());
  saveCalibration(Correction_ErrorEstimationCovariance, nameEC.Data());
  saveCalibration(InPixelPosition, nameIP.Data());

  delete[] m_histSignal;
  delete[] m_histSeed;
  delete[] m_histResidTruePlaneUnBiasUV;
  delete[] m_histSigmaTrackUnBiasUV;
  delete[] m_histResidPlaneRHUnBiasUV;
  delete[] m_histSigmaRecoUV;
  delete[] m_histResidTrueRHUV;

  delete[] m_histBiasCorrectionU;
  delete[] m_histBiasCorrectionV;
  delete[] m_histResidualRMSU;
  delete[] m_histResidualRMSV;
  delete[] m_histBiasCorrectionErrorU;
  delete[] m_histBiasCorrectionErrorV;
  delete[] m_histnClusters;
  delete[] m_histnClusterFraction;

  delete[] m_histCorrEE_TrueU;
  delete[] m_histCorrEE_TrueV;
  delete[] m_histCorrEE_TrueTrackU;
  delete[] m_histCorrEE_TrueTrackV;
  delete[] m_histCorrEE_RealTrackU;
  delete[] m_histCorrEE_RealTrackV;
  delete[] m_histResRMSEEU;
  delete[] m_histResRMSEEV;
  delete[] m_histDiffClEETrueOrigEEU;
  delete[] m_histDiffClEETrueOrigEEV;
  delete[] m_histDiffClEETrueCalcTTEEU;
  delete[] m_histDiffClEETrueCalcTTEEV;
  delete[] m_histDiffClEETrueCalcRTEEU;
  delete[] m_histDiffClEETrueCalcRTEEV;
  delete[] m_histClusterTrueEEU;
  delete[] m_histClusterTrueEEV;
  delete[] m_histClusterOrigEEU;
  delete[] m_histClusterOrigEEV;
  delete[] m_histClustCalcTTEEU;
  delete[] m_histClustCalcTTEEV;
  delete[] m_histClustCalcRTEEU;
  delete[] m_histClustCalcRTEEV;

  delete [] m_histBiasCounterU;
  delete [] m_histBiasCounterV;
  delete [] m_histEECounterU;
  delete [] m_histEECounterV;

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
  delete[] ValueCors;
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
}

double PXDClusterShapeCalibrationAlgorithm::getPhiFromIndex(int indPhi)
{
//  double Phi = ((TMath::Pi() * indPhi) / m_anglesU) - (TMath::Pi() / 2.0);
//  return Phi;
  return (M_PI / m_anglesU) * indPhi - M_PI_2;
}

double PXDClusterShapeCalibrationAlgorithm::getThetaFromIndex(int indTheta)
{
//  double Theta = ((TMath::Pi() * indTheta) / m_anglesV) - (TMath::Pi() / 2.0);
//  return Theta;
  return (M_PI / m_anglesV) * indTheta - M_PI_2;
}

int PXDClusterShapeCalibrationAlgorithm::getPhiIndex(float phi)
{
//  int ind = m_anglesU / 2 + (int)(phi * m_anglesU / TMath::Pi());
//  return ind;
  return (int)((phi + M_PI_2) / (M_PI / m_anglesU));
}

int PXDClusterShapeCalibrationAlgorithm::getThetaIndex(float theta)
{
//  int ind = m_anglesV / 2 + (int)(theta * m_anglesV / TMath::Pi());
//  return ind;
  return (int)((theta + M_PI_2) / (M_PI / m_anglesV));
}

int PXDClusterShapeCalibrationAlgorithm::CalculateCorrection(int CorCase, int n,
    std::vector<float> array, double* val, double* valError, double* rms)
{
  if (n < m_MinClustersCorrections) {
    return 0;
  }
  int ret = 1;

  double preset;
  double minCorrection;
  double MinDistanceInErrors;
  MinDistanceInErrors = 3.0;
  if (CorCase == 0) {
    preset = 0.0;
    minCorrection = 0.0;
  } else if (CorCase == 1) {
    preset = 0.0;
    minCorrection = 0.5 * Unit::um;
  } else if (CorCase == 2) {
    preset = 1.0;
    minCorrection = 0.05;
  } else {
    return 0;
  }

  *val = preset;
  *valError = 0.0;
  *rms = 1;

  float QuantCut = 0.9; // TODO this parameter can be change...
  double quantiles[2];
  double prob[2];
  int nprob = 2;
  Bool_t isSorted = kFALSE;
  prob[0] = (1. - QuantCut) / 2.;
  prob[1] = 1.0 - prob[0];
  double* array2 = new double[n];  // medium buffer of array
  for (int i = 0; i < n; i++)   // TODO it correct
    array2[i] = array[i];
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
  delete [] array2;
  ErrNew /= 0.79;                 // Correction becouse we cut out 10% of full distribution
  double SEM = ErrNew / sqrt(n);  // Estimated error of mean

  // condition for using correction:
  if (CorCase == 1) {
    if (fabs(DevNew) < minCorrection) ret = 0;
    if (fabs(DevNew) < (MinDistanceInErrors * SEM)) ret = 0;  // apply only for bias
  }
  if (CorCase == 2) {
    if (fabs(ErrNew - 1) < minCorrection) ret = 0;
  }
  *val = DevNew;
  *valError = SEM;
  *rms = ErrNew;
  return ret;
}

double PXDClusterShapeCalibrationAlgorithm::GetCorrectionResErrEst(double ResErrEst)
{
// z = (x - a)/b
// e = exp(-z)
// f = A * e / (1 + e)
  double a = 0.644261279;
  double b = -0.065866687;
  double A = 1.021089491;
  double out = (ResErrEst - a) / b;
  out = exp(-out);
  return A * out / (1 + out);
}
