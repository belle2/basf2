/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jincheng Mei                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

/* Own header. */
#include <klm/calibration/KLMTimeAlgorithm.h>

/* KLM headers. */
#include <klm/dataobjects/bklm/BKLMElementNumbers.h>

/* Belle 2 headers. */
#include <framework/database/Database.h>
#include <framework/database/DBObjPtr.h>
#include <framework/database/DBStore.h>
#include <framework/dataobjects/EventMetaData.h>
#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>

/* ROOT headers. */
#include <Math/MinimizerOptions.h>
#include <TFile.h>
#include <TFitResult.h>
#include <TMinuit.h>
#include <TROOT.h>
#include <TString.h>
#include <TTree.h>
#include <TVector3.h>

using namespace Belle2;
using namespace ROOT::Math;

/** Number of time bins. */
const int c_NBinsTime = 100;

/** Number of distance bins. */
const int c_NBinsDistance = 100;

/** Binned data. */
static double s_BinnedData[c_NBinsTime][c_NBinsDistance];

/** Lower time boundary. */
static double s_LowerTimeBoundary = 0;

/** Upper time boundary. */
static double s_UpperTimeBoundary = 0;

/** Maximal propagation distance (strip length). */
static double s_StripLength = 0;

static bool compareEventNumber(const std::pair<uint16_t, unsigned int>& pair1,
                               const std::pair<uint16_t, unsigned int>& pair2)
{
  return pair1.second < pair2.second;
}

static double timeDensity(double x[2], double* par)
{
  double polynomial, t0, gauss;
  polynomial = par[0];
  t0 = par[2] + par[4] * x[1];
  gauss = par[1] / (sqrt(2.0 * M_PI) * par[3]) *
          exp(-0.5 * pow((x[0] - t0) / par[3], 2));
  return fabs(polynomial + gauss);
}

/* cppcheck-suppress constParameter */
static void fcn(int& npar, double* grad, double& fval, double* par, int iflag)
{
  (void)npar;
  (void)grad;
  (void)iflag;
  double x[2];
  fval = 0;
  for (int i = 0; i < c_NBinsTime; ++i) {
    x[0] = s_LowerTimeBoundary +
           (s_UpperTimeBoundary - s_LowerTimeBoundary) *
           (double(i) + 0.5) / c_NBinsTime;
    for (int j = 0; j < c_NBinsDistance; ++j) {
      x[1] = s_StripLength * (double(j) + 0.5) / c_NBinsDistance;
      double f = timeDensity(x, par);
      if (s_BinnedData[i][j] == 0)
        fval = fval + 2.0 * f;
      else
        fval = fval + 2.0 * (f - s_BinnedData[i][j] *
                             (1.0 - log(s_BinnedData[i][j] / f)));
    }
  }
}

KLMTimeAlgorithm::KLMTimeAlgorithm() :
  CalibrationAlgorithm("KLMTimeCollector")
{
  m_ElementNumbers = &(KLMElementNumbers::Instance());
  m_minimizerOptions = ROOT::Math::MinimizerOptions();
}

KLMTimeAlgorithm::~KLMTimeAlgorithm()
{
}

void KLMTimeAlgorithm::setupDatabase()
{
  const std::vector<Calibration::ExpRun>& runs = getRunList();
  int firstExperiment = runs[0].first;
  int lastExperiment = runs[runs.size() - 1].first;
  if (firstExperiment != lastExperiment) {
    B2FATAL("Runs from different experiments are used "
            "for KLM time calibration (single algorithm run).");
  }
  /* DataStore. */
  DataStore::Instance().setInitializeActive(true);
  StoreObjPtr<EventMetaData> eventMetaData;
  eventMetaData.registerInDataStore();
  DataStore::Instance().setInitializeActive(false);
  /* Database. */
  if (eventMetaData.isValid()) {
    if (eventMetaData->getExperiment() != firstExperiment) {
      B2FATAL("Runs from different experiments are used "
              "for KLM time calibration (consecutive algorithm runs).");
    }
    eventMetaData->setExperiment(firstExperiment);
    eventMetaData->setRun(runs[0].second);
  } else {
    eventMetaData.construct(1, runs[0].second, firstExperiment);
  }
  DBStore& dbStore = DBStore::Instance();
  dbStore.update();
  dbStore.updateEvent();
  /*
   * For calibration algorithms, the database is not initialized on class
   * creation. Do not move the database object to class members.
   */
  DBObjPtr<BKLMGeometryPar> bklmGeometry;
  m_BKLMGeometry = bklm::GeometryPar::instance(*bklmGeometry);
  m_EKLMGeometry = &(EKLM::GeometryData::Instance());
}

CalibrationAlgorithm::EResult KLMTimeAlgorithm::readCalibrationData()
{
  B2INFO("Read tree entries and seprate events by module id.");
  Event event;
  std::shared_ptr<TTree> timeCalibrationData;
  timeCalibrationData = getObjectPtr<TTree>("time_calibration_data");
  timeCalibrationData->SetBranchAddress("t0", &event.t0);
  timeCalibrationData->SetBranchAddress("flyTime", &event.flyTime);
  timeCalibrationData->SetBranchAddress("recTime", &event.recTime);
  timeCalibrationData->SetBranchAddress("dist", &event.dist);
  timeCalibrationData->SetBranchAddress("diffDistX", &event.diffDistX);
  timeCalibrationData->SetBranchAddress("diffDistY", &event.diffDistY);
  timeCalibrationData->SetBranchAddress("diffDistZ", &event.diffDistZ);
  timeCalibrationData->SetBranchAddress("eDep", &event.eDep);
  timeCalibrationData->SetBranchAddress("nPE", &event.nPE);
  timeCalibrationData->SetBranchAddress("channelId", &event.channelId);
  timeCalibrationData->SetBranchAddress("inRPC", &event.inRPC);
  timeCalibrationData->SetBranchAddress("isFlipped", &event.isFlipped);

  B2INFO(LogVar("Total number of digits:", timeCalibrationData->GetEntries()));
  m_evts.clear();

  int n = timeCalibrationData->GetEntries();
  if (n < m_MinimalDigitNumber)
    return CalibrationAlgorithm::c_NotEnoughData;
  for (int i = 0; i < n; ++i) {
    timeCalibrationData->GetEntry(i);
    m_evts[event.channelId].push_back(event);
  }
  B2INFO("Events packing finish.");
  return CalibrationAlgorithm::c_OK;
}

void KLMTimeAlgorithm::createHistograms()
{
  if (m_mc) {
    m_LowerTimeBoundaryRPC = -10.0;
    m_UpperTimeBoundaryRPC = 10.0;
    m_LowerTimeBoundaryScintilltorsBKLM = 20.0;
    m_UpperTimeBoundaryScintilltorsBKLM = 70.0;
    m_LowerTimeBoundaryScintilltorsEKLM = 20.0;
    m_UpperTimeBoundaryScintilltorsEKLM = 70.0;
  } else {
    m_LowerTimeBoundaryRPC = -800.0;
    m_UpperTimeBoundaryRPC = -600.0;
    m_LowerTimeBoundaryScintilltorsBKLM = -4800.0;
    m_UpperTimeBoundaryScintilltorsBKLM = -4400.0;
    m_LowerTimeBoundaryScintilltorsEKLM = -4950.0;
    m_UpperTimeBoundaryScintilltorsEKLM = -4650.0;
  }
  int nBin = 200;
  int nBin_scint = 400;

  TString iFstring[2] = {"Backward", "Forward"};
  TString iPstring[2] = {"ZReadout", "PhiReadout"};
  TString hn, ht;

  h_diff = new TH1D("h_diff", "Position difference between bklmHit2d and extHit;position difference", 100, 0, 10);
  h_calibrated = new TH1I("h_calibrated_summary", "h_calibrated_summary;calibrated or not", 3, 0, 3);

  gre_time_channel_scint = new TGraphErrors();
  gre_time_channel_rpc = new TGraphErrors();
  gre_time_channel_scint_end = new TGraphErrors();

  gr_timeShift_channel_scint = new TGraph();
  gr_timeShift_channel_rpc = new TGraph();
  gr_timeShift_channel_scint_end = new TGraph();

  double maximalPhiStripLengthBKLM =
    m_BKLMGeometry->getMaximalPhiStripLength();
  double maximalZStripLengthBKLM =
    m_BKLMGeometry->getMaximalZStripLength();
  double maximalStripLengthEKLM =
    m_EKLMGeometry->getMaximalStripLength() / CLHEP::cm * Unit::cm;

  m_ProfileRpcPhi = new TProfile("hprf_rpc_phi_effC",
                                 "Time over propagation length for RPCs (Phi_Readout); propagation distance[cm]; T_rec-T_0-T_fly-'T_calibration'[ns]", 400, 0.0,
                                 400.0);
  m_ProfileRpcZ = new TProfile("hprf_rpc_z_effC",
                               "Time over propagation length for RPCs (Z_Readout); propagation distance[cm]; T_rec-T_0-T_fly-'T_calibration'[ns]", 400, 0.0,
                               400.0);
  m_ProfileBKLMScintillatorPhi = new TProfile("hprf_scint_phi_effC",
                                              "Time over propagation length for scintillators (Phi_Readout); propagation distance[cm]; T_rec-T_0-T_fly-'T_calibration'[ns]",
                                              200, 0.0, maximalPhiStripLengthBKLM);
  m_ProfileBKLMScintillatorZ = new TProfile("hprf_scint_z_effC",
                                            "Time over propagation length for scintillators (Z_Readout); propagation distance[cm]; T_rec-T_0-T_fly-'T_calibration'[ns]",
                                            200, 0.0, maximalZStripLengthBKLM);
  m_ProfileEKLMScintillatorPlane1 = new TProfile("hprf_scint_plane1_effC_end",
                                                 "Time over propagation length for scintillators (plane1, Endcap); propagation distance[cm]; T_rec-T_0-T_fly-'T_calibration'[ns]",
                                                 200, 0.0, maximalStripLengthEKLM);
  m_ProfileEKLMScintillatorPlane2 = new TProfile("hprf_scint_plane2_effC_end",
                                                 "Time over propagation length for scintillators (plane2, Endcap); propagation distance[cm]; T_rec-T_0-T_fly-'T_calibration'[ns]",
                                                 200, 0.0, maximalStripLengthEKLM);

  m_Profile2RpcPhi = new TProfile("hprf2_rpc_phi_effC",
                                  "Time over propagation length for RPCs (Phi_Readout); propagation distance[cm]; T_rec-T_0-T_fly-'T_calibration'[ns]", 400, 0.0,
                                  400.0);
  m_Profile2RpcZ = new TProfile("hprf2_rpc_z_effC",
                                "Time over propagation length for RPCs (Z_Readout); propagation distance[cm]; T_rec-T_0-T_fly-'T_calibration'[ns]", 400, 0.0,
                                400.0);
  m_Profile2BKLMScintillatorPhi = new TProfile("hprf2_scint_phi_effC",
                                               "Time over propagation length for scintillators (Phi_Readout); propagation distance[cm]; T_rec-T_0-T_fly-'T_calibration'[ns]",
                                               200, 0.0, maximalPhiStripLengthBKLM);
  m_Profile2BKLMScintillatorZ = new TProfile("hprf2_scint_z_effC",
                                             "Time over propagation length for scintillators (Z_Readout); propagation distance[cm]; T_rec-T_0-T_fly-'T_calibration'[ns]",
                                             200, 0.0, maximalZStripLengthBKLM);
  m_Profile2EKLMScintillatorPlane1 = new TProfile("hprf2_scint_plane1_effC_end",
                                                  "Time over propagation length for scintillators (plane1, Endcap); propagation distance[cm]; T_rec-T_0-T_fly-'T_calibration'[ns]",
                                                  200, 0.0, maximalStripLengthEKLM);
  m_Profile2EKLMScintillatorPlane2 = new TProfile("hprf2_scint_plane2_effC_end",
                                                  "Time over propagation length for scintillators (plane2, Endcap); propagation distance[cm]; T_rec-T_0-T_fly-'T_calibration'[ns]",
                                                  200, 0.0, maximalStripLengthEKLM);

  h_time_rpc_tc = new TH1D("h_time_rpc_tc", "time distribtution for RPC", nBin, m_LowerTimeBoundaryRPC, m_UpperTimeBoundaryRPC);
  h_time_scint_tc = new TH1D("h_time_scint_tc", "time distribtution for Scintillator", nBin_scint,
                             m_LowerTimeBoundaryScintilltorsBKLM, m_UpperTimeBoundaryScintilltorsBKLM);
  h_time_scint_tc_end = new TH1D("h_time_scint_tc_end", "time distribtution for Scintillator (Endcap)", nBin_scint,
                                 m_LowerTimeBoundaryScintilltorsEKLM,
                                 m_UpperTimeBoundaryScintilltorsEKLM);

  /** Hist declaration Global time distribution **/
  h_time_rpc = new TH1D("h_time_rpc", "time distribtution for RPC; T_rec-T_0-T_fly-T_propagation[ns]", nBin, m_LowerTimeBoundaryRPC,
                        m_UpperTimeBoundaryRPC);
  h_time_scint = new TH1D("h_time_scint", "time distribtution for Scintillator; T_rec-T_0-T_fly-T_propagation[ns]", nBin_scint,
                          m_LowerTimeBoundaryScintilltorsBKLM, m_UpperTimeBoundaryScintilltorsBKLM);
  h_time_scint_end = new TH1D("h_time_scint_end", "time distribtution for Scintillator (Endcap); T_rec-T_0-T_fly-T_propagation[ns]",
                              nBin_scint, m_LowerTimeBoundaryScintilltorsEKLM, m_UpperTimeBoundaryScintilltorsEKLM);

  hc_time_rpc = new TH1D("hc_time_rpc", "Calibrated time distribtution for RPC; T_rec-T_0-T_fly-T_propagation-T_calibration[ns]",
                         nBin, m_LowerTimeBoundaryCalibratedRPC, m_UpperTimeBoundaryCalibratedRPC);
  hc_time_scint = new TH1D("hc_time_scint",
                           "Calibrated time distribtution for Scintillator; T_rec-T_0-T_fly-T_propagation-T_calibration[ns]", nBin_scint,
                           m_LowerTimeBoundaryCalibratedScintilltorsBKLM,
                           m_UpperTimeBoundaryCalibratedScintilltorsBKLM);
  hc_time_scint_end = new TH1D("hc_time_scint_end",
                               "Calibrated time distribtution for Scintillator (Endcap); T_rec-T_0-T_fly-T_propagation-T_calibration[ns]", nBin_scint,
                               m_LowerTimeBoundaryCalibratedScintilltorsEKLM, m_UpperTimeBoundaryCalibratedScintilltorsEKLM);

  for (int iF = 0; iF < 2; ++iF) {
    hn = Form("h_timeF%d_rpc", iF);
    ht = Form("Time distribtution for RPC of %s; T_rec-T_0-T_fly-T_propagation[ns]", iFstring[iF].Data());
    h_timeF_rpc[iF] = new TH1D(hn.Data(), ht.Data(), nBin, m_LowerTimeBoundaryRPC, m_UpperTimeBoundaryRPC);
    hn = Form("h_timeF%d_scint", iF);
    ht = Form("Time distribtution for Scintillator of %s; T_rec-T_0-T_fly-T_propagation[ns]", iFstring[iF].Data());
    h_timeF_scint[iF] = new TH1D(hn.Data(), ht.Data(), nBin_scint, m_LowerTimeBoundaryScintilltorsBKLM,
                                 m_UpperTimeBoundaryScintilltorsBKLM);
    hn = Form("h_timeF%d_scint_end", iF);
    ht = Form("Time distribtution for Scintillator of %s (Endcap); T_rec-T_0-T_fly-T_propagation[ns]", iFstring[iF].Data());
    h_timeF_scint_end[iF] = new TH1D(hn.Data(), ht.Data(), nBin_scint, m_LowerTimeBoundaryScintilltorsEKLM,
                                     m_UpperTimeBoundaryScintilltorsEKLM);

    hn = Form("h2_timeF%d_rpc", iF);
    ht = Form("Time distribtution for RPC of %s; Sector Index; T_rec-T_0-T_fly-T_propagation[ns]", iFstring[iF].Data());
    h2_timeF_rpc[iF] = new TH2D(hn.Data(), ht.Data(), 8, 0, 8, nBin, m_LowerTimeBoundaryRPC, m_UpperTimeBoundaryRPC);
    hn = Form("h2_timeF%d_scint", iF);
    ht = Form("Time distribtution for Scintillator of %s; Sector Index; T_rec-T_0-T_fly-T_propagation[ns]", iFstring[iF].Data());
    h2_timeF_scint[iF] = new TH2D(hn.Data(), ht.Data(), 8, 0, 8, nBin_scint, m_LowerTimeBoundaryScintilltorsBKLM,
                                  m_UpperTimeBoundaryScintilltorsBKLM);
    hn = Form("h2_timeF%d_scint_end", iF);
    ht = Form("Time distribtution for Scintillator of %s (Endcap); Sector Index; T_rec-T_0-T_fly-T_propagation[ns]",
              iFstring[iF].Data());
    h2_timeF_scint_end[iF] = new TH2D(hn.Data(), ht.Data(), 4, 0, 4, nBin_scint, m_LowerTimeBoundaryScintilltorsEKLM,
                                      m_UpperTimeBoundaryScintilltorsEKLM);

    hn = Form("hc_timeF%d_rpc", iF);
    ht = Form("Calibrated time distribtution for RPC of %s; T_rec-T_0-T_fly-T_propagation-T_calibration[ns]", iFstring[iF].Data());
    hc_timeF_rpc[iF] = new TH1D(hn.Data(), ht.Data(), nBin, m_LowerTimeBoundaryCalibratedRPC, m_UpperTimeBoundaryCalibratedRPC);
    hn = Form("hc_timeF%d_scint", iF);
    ht = Form("Calibrated time distribtution for Scintillator of %s; T_rec-T_0-T_fly-T_propagation-T_calibration[ns]",
              iFstring[iF].Data());
    hc_timeF_scint[iF] = new TH1D(hn.Data(), ht.Data(), nBin_scint, m_LowerTimeBoundaryCalibratedScintilltorsBKLM,
                                  m_UpperTimeBoundaryCalibratedScintilltorsBKLM);
    hn = Form("hc_timeF%d_scint_end", iF);
    ht = Form("Calibrated time distribtution for Scintillator of %s (Endcap); T_rec-T_0-T_fly-T_propagation-T_calibration[ns]",
              iFstring[iF].Data());
    hc_timeF_scint_end[iF] = new TH1D(hn.Data(), ht.Data(), nBin_scint, m_LowerTimeBoundaryCalibratedScintilltorsEKLM,
                                      m_UpperTimeBoundaryCalibratedScintilltorsEKLM);

    hn = Form("h2c_timeF%d_rpc", iF);
    ht = Form("Calibrated time distribtution for RPC of %s; Sector Index; T_rec-T_0-T_fly-T_propagation-T_calibration[ns]",
              iFstring[iF].Data());
    h2c_timeF_rpc[iF] = new TH2D(hn.Data(), ht.Data(), 8, 0, 8, nBin, m_LowerTimeBoundaryCalibratedRPC,
                                 m_UpperTimeBoundaryCalibratedRPC);
    hn = Form("h2c_timeF%d_scint", iF);
    ht = Form("Calibrated time distribtution for Scintillator of %s; Sector Index; T_rec-T_0-T_fly-T_propagation-T_calibration[ns]",
              iFstring[iF].Data());
    h2c_timeF_scint[iF] = new TH2D(hn.Data(), ht.Data(), 8, 0, 8, nBin_scint, m_LowerTimeBoundaryCalibratedScintilltorsBKLM,
                                   m_UpperTimeBoundaryCalibratedScintilltorsBKLM);
    hn = Form("h2c_timeF%d_scint_end", iF);
    ht = Form("Calibrated time distribtution for Scintillator of %s (Endcap) ; Sector Index; T_rec-T_0-T_fly-T_propagation-T_calibration[ns]",
              iFstring[iF].Data());
    h2c_timeF_scint_end[iF] = new TH2D(hn.Data(), ht.Data(), 4, 0, 4, nBin_scint, m_LowerTimeBoundaryCalibratedScintilltorsEKLM,
                                       m_UpperTimeBoundaryCalibratedScintilltorsEKLM);

    for (int iS = 0; iS < 8; ++iS) {
      // Barrel parts
      hn = Form("h_timeF%d_S%d_scint", iF, iS);
      ht = Form("Time distribtution for Scintillator of Sector%d, %s; T_rec-T_0-T_fly-T_propagation[ns]", iS, iFstring[iF].Data());
      h_timeFS_scint[iF][iS] = new TH1D(hn.Data(), ht.Data(), nBin_scint, m_LowerTimeBoundaryScintilltorsBKLM,
                                        m_UpperTimeBoundaryScintilltorsBKLM);
      hn = Form("h_timeF%d_S%d_rpc", iF, iS);
      ht = Form("Time distribtution for RPC of Sector%d, %s; T_rec-T_0-T_fly-T_propagation[ns]", iS, iFstring[iF].Data());
      h_timeFS_rpc[iF][iS] = new TH1D(hn.Data(), ht.Data(), nBin, m_LowerTimeBoundaryRPC, m_UpperTimeBoundaryRPC);
      hn = Form("h2_timeF%d_S%d", iF, iS);
      ht = Form("Time distribtution of Sector%d, %s; Layer Index; T_rec-T_0-T_fly-T_propagation[ns]", iS, iFstring[iF].Data());
      h2_timeFS[iF][iS] = new TH2D(hn.Data(), ht.Data(), 15, 0, 15, nBin_scint, m_LowerTimeBoundaryRPC,
                                   m_UpperTimeBoundaryScintilltorsBKLM);

      hn = Form("hc_timeF%d_S%d_scint", iF, iS);
      ht = Form("Calibrated time distribtution for Scintillator of Sector%d, %s; T_rec-T_0-T_fly-T_propagation-T_calibration[ns]", iS,
                iFstring[iF].Data());
      hc_timeFS_scint[iF][iS] = new TH1D(hn.Data(), ht.Data(), nBin_scint, m_LowerTimeBoundaryCalibratedScintilltorsBKLM,
                                         m_UpperTimeBoundaryCalibratedScintilltorsBKLM);
      hn = Form("hc_timeF%d_S%d_rpc", iF, iS);
      ht = Form("Calibrated time distribtution for RPC of Sector%d, %s; T_rec-T_0-T_fly-T_propagation-T_calibration[ns]", iS,
                iFstring[iF].Data());
      hc_timeFS_rpc[iF][iS] = new TH1D(hn.Data(), ht.Data(), nBin_scint, m_LowerTimeBoundaryCalibratedRPC,
                                       m_UpperTimeBoundaryCalibratedRPC);
      hn = Form("h2c_timeF%d_S%d", iF, iS);
      ht = Form("Calibrated time distribtution of Sector%d, %s; Layer Index; T_rec-T_0-T_fly-T_propagation-T_calibration[ns]", iS,
                iFstring[iF].Data());
      h2c_timeFS[iF][iS] = new TH2D(hn.Data(), ht.Data(), 15, 0, 15, nBin_scint, m_LowerTimeBoundaryCalibratedRPC,
                                    m_UpperTimeBoundaryCalibratedScintilltorsBKLM);

      // Inner 2 layers --> Scintillators
      for (int iL = 0; iL < 2; ++iL) {
        hn = Form("h_timeF%d_S%d_L%d", iF, iS, iL);
        ht = Form("Time distribtution for Scintillator of Layer%d, Sector%d, %s; T_rec-T_0-T_fly-T_propagation[ns]", iL, iS,
                  iFstring[iF].Data());
        h_timeFSL[iF][iS][iL] = new TH1D(hn.Data(), ht.Data(), nBin_scint, m_LowerTimeBoundaryScintilltorsBKLM,
                                         m_UpperTimeBoundaryScintilltorsBKLM);
        hn = Form("hc_timeF%d_S%d_L%d", iF, iS, iL);
        ht = Form("Calibrated time distribtution for Scintillator of Layer%d, Sector%d, %s; T_rec-T_0-T_fly-T_propagation-T_calibration[ns]",
                  iL, iS, iFstring[iF].Data());
        hc_timeFSL[iF][iS][iL] = new TH1D(hn.Data(), ht.Data(), nBin_scint, m_LowerTimeBoundaryCalibratedScintilltorsBKLM,
                                          m_UpperTimeBoundaryCalibratedScintilltorsBKLM);

        for (int iP = 0; iP < 2; ++iP) {
          hn = Form("h_timeF%d_S%d_L%d_P%d", iF, iS, iL, iP);
          ht = Form("Time distribtution for Scintillator of %s, Layer%d, Sector%d, %s; T_rec-T_0-T_fly-T_propagation[ns]",
                    iPstring[iP].Data(), iL, iS, iFstring[iF].Data());
          h_timeFSLP[iF][iS][iL][iP] = new TH1D(hn.Data(), ht.Data(), nBin_scint, m_LowerTimeBoundaryScintilltorsBKLM,
                                                m_UpperTimeBoundaryScintilltorsBKLM);
          hn = Form("h2_timeF%d_S%d_L%d_P%d", iF, iS, iL, iP);
          ht = Form("Time distribtution for Scintillator of %s, Layer%d, Sector%d, %s; Channel Index; T_rec-T_0-T_fly-T_propagation[ns]",
                    iPstring[iP].Data(), iL, iS, iFstring[iF].Data());
          h2_timeFSLP[iF][iS][iL][iP] = new TH2D(hn.Data(), ht.Data(), 54, 0, 54, nBin_scint, m_LowerTimeBoundaryScintilltorsBKLM,
                                                 m_UpperTimeBoundaryScintilltorsBKLM);

          hn = Form("hc_timeF%d_S%d_L%d_P%d", iF, iS, iL, iP);
          ht = Form("Calibrated time distribtution for Scintillator of %s, Layer%d, Sector%d, %s; T_rec-T_0-T_fly-T_propagation-T_calibration[ns]",
                    iPstring[iP].Data(), iL, iS, iFstring[iF].Data());
          hc_timeFSLP[iF][iS][iL][iP] = new TH1D(hn.Data(), ht.Data(), nBin_scint, m_LowerTimeBoundaryCalibratedScintilltorsBKLM,
                                                 m_UpperTimeBoundaryCalibratedScintilltorsBKLM);
          hn = Form("h2c_timeF%d_S%d_L%d_P%d", iF, iS, iL, iP);
          ht = Form("Calibrated time distribtution for Scintillator of %s, Layer%d, Sector%d, %s; Channel Index; T_rec-T_0-T_fly-T_propagation-T_calibration[ns]",
                    iPstring[iP].Data(), iL, iS, iFstring[iF].Data());
          h2c_timeFSLP[iF][iS][iL][iP] = new TH2D(hn.Data(), ht.Data(), 54, 0, 54, nBin_scint, m_LowerTimeBoundaryCalibratedScintilltorsBKLM,
                                                  m_UpperTimeBoundaryCalibratedScintilltorsBKLM);

          int nchannel_max = BKLMElementNumbers::getNStrips(iF, iS + 1, iL + 1, iP);
          for (int iC = 0; iC < nchannel_max; ++iC) {
            hn = Form("h_timeF%d_S%d_L%d_P%d_C%d_tc", iF, iS, iL, iP, iC);
            ht = Form("time distribtution for Scintillator of Channel%d, %s, Layer%d, Sector%d, %s; T_rec-T_0-T_fly-T_propagation[ns]", iC,
                      iPstring[iP].Data(), iL, iS, iFstring[iF].Data());
            h_timeFSLPC_tc[iF][iS][iL][iP][iC] = new TH1D(hn.Data(), ht.Data(), nBin_scint, m_LowerTimeBoundaryScintilltorsBKLM,
                                                          m_UpperTimeBoundaryScintilltorsBKLM);

            hn = Form("h_timeF%d_S%d_L%d_P%d_C%d", iF, iS, iL, iP, iC);
            ht = Form("time distribtution for Scintillator of Channel%d, %s, Layer%d, Sector%d, %s; T_rec-T_0-T_fly-T_propagation[ns]", iC,
                      iPstring[iP].Data(), iL, iS, iFstring[iF].Data());
            h_timeFSLPC[iF][iS][iL][iP][iC] = new TH1D(hn.Data(), ht.Data(), nBin_scint, m_LowerTimeBoundaryScintilltorsBKLM,
                                                       m_UpperTimeBoundaryScintilltorsBKLM);

            hn = Form("hc_timeF%d_S%d_L%d_P%d_C%d", iF, iS, iL, iP, iC);
            ht = Form("Calibrated time distribtution for Scintillator of Channel%d, %s, Layer%d, Sector%d, %s; T_rec-T_0-T_fly-T_propagation-T_calibration[ns]",
                      iC, iPstring[iP].Data(), iL, iS, iFstring[iF].Data());
            hc_timeFSLPC[iF][iS][iL][iP][iC] = new TH1D(hn.Data(), ht.Data(), nBin_scint, m_LowerTimeBoundaryCalibratedScintilltorsBKLM,
                                                        m_UpperTimeBoundaryCalibratedScintilltorsBKLM);
            hn = Form("time_length_bklm_F%d_S%d_L%d_P%d_C%d", iF, iS, iL, iP, iC);
            double stripLength = 200;
            m_HistTimeLengthBKLM[iF][iS][iL][iP][iC] =
              new TH2F(hn.Data(),
                       "Time versus propagation length; "
                       "propagation distance[cm]; "
                       "T_rec-T_0-T_fly-'T_calibration'[ns]",
                       200, 0.0, stripLength,
                       400, m_LowerTimeBoundaryCalibratedScintilltorsBKLM,
                       m_UpperTimeBoundaryCalibratedScintilltorsBKLM);
          }
        }
      }

      for (int iL = 2; iL < 15; ++iL) {
        hn = Form("h_timeF%d_S%d_L%d", iF, iS, iL);
        ht = Form("time distribtution for RPC of Layer%d, Sector%d, %s; T_rec-T_0-T_fly-T_propagation[ns]", iL, iS, iFstring[iF].Data());
        h_timeFSL[iF][iS][iL] = new TH1D(hn.Data(), ht.Data(), nBin, m_LowerTimeBoundaryRPC, m_UpperTimeBoundaryRPC);

        hn = Form("hc_timeF%d_S%d_L%d", iF, iS, iL);
        ht = Form("Calibrated time distribtution for RPC of Layer%d, Sector%d, %s; T_rec-T_0-T_fly-T_propagation-T_calibration[ns]", iL, iS,
                  iFstring[iF].Data());
        hc_timeFSL[iF][iS][iL] = new TH1D(hn.Data(), ht.Data(), nBin, m_LowerTimeBoundaryCalibratedRPC, m_UpperTimeBoundaryCalibratedRPC);

        for (int iP = 0; iP < 2; ++iP) {
          hn = Form("h_timeF%d_S%d_L%d_P%d", iF, iS, iL, iP);
          ht = Form("time distribtution for RPC of %s, Layer%d, Sector%d, %s; T_rec-T_0-T_fly-T_propagation[ns]", iPstring[iP].Data(), iL, iS,
                    iFstring[iF].Data());
          h_timeFSLP[iF][iS][iL][iP] = new TH1D(hn.Data(), ht.Data(), nBin, m_LowerTimeBoundaryRPC, m_UpperTimeBoundaryRPC);

          hn = Form("h2_timeF%d_S%d_L%d_P%d", iF, iS, iL, iP);
          ht = Form("time distribtution for RPC of %s, Layer%d, Sector%d, %s; Channel Index; T_rec-T_0-T_fly-T_propagation[ns]",
                    iPstring[iP].Data(), iL, iS, iFstring[iF].Data());
          h2_timeFSLP[iF][iS][iL][iP] = new TH2D(hn.Data(), ht.Data(), 48, 0, 48, nBin, m_LowerTimeBoundaryRPC, m_UpperTimeBoundaryRPC);

          hn = Form("hc_timeF%d_S%d_L%d_P%d", iF, iS, iL, iP);
          ht = Form("Calibrated time distribtution for RPC of %s, Layer%d, Sector%d, %s; T_rec-T_0-T_fly-T_propagation-T_calibration[ns]",
                    iPstring[iP].Data(), iL, iS, iFstring[iF].Data());
          hc_timeFSLP[iF][iS][iL][iP] = new TH1D(hn.Data(), ht.Data(), nBin, m_LowerTimeBoundaryCalibratedRPC,
                                                 m_UpperTimeBoundaryCalibratedRPC);

          hn = Form("h2c_timeF%d_S%d_L%d_P%d", iF, iS, iL, iP);
          ht = Form("Calibrated time distribtution for RPC of %s, Layer%d, Sector%d, %s; Channel Index; T_rec-T_0-T_fly-T_propagation-T_calibration[ns]",
                    iPstring[iP].Data(), iL, iS, iFstring[iF].Data());
          h2c_timeFSLP[iF][iS][iL][iP] = new TH2D(hn.Data(), ht.Data(), 48, 0, 48, nBin, m_LowerTimeBoundaryCalibratedRPC,
                                                  m_UpperTimeBoundaryCalibratedRPC);

          int nchannel_max = BKLMElementNumbers::getNStrips(iF, iS + 1, iL + 1, iP);
          for (int iC = 0; iC < nchannel_max; ++iC) {
            hn = Form("h_timeF%d_S%d_L%d_P%d_C%d_tc", iF, iS, iL, iP, iC);
            ht = Form("Time distribtution for RPC of Channel%d, %s, Layer%d, Sector%d, %s; T_rec-T_0-T_fly-T_propagation[ns]", iC,
                      iPstring[iP].Data(), iL, iS, iFstring[iF].Data());
            h_timeFSLPC_tc[iF][iS][iL][iP][iC] = new TH1D(hn.Data(), ht.Data(), nBin, m_LowerTimeBoundaryRPC, m_UpperTimeBoundaryRPC);

            hn = Form("h_timeF%d_S%d_L%d_P%d_C%d", iF, iS, iL, iP, iC);
            ht = Form("Time distribtution for RPC of Channel%d, %s, Layer%d, Sector%d, %s; T_rec-T_0-T_fly-T_propagation[ns]", iC,
                      iPstring[iP].Data(), iL, iS, iFstring[iF].Data());
            h_timeFSLPC[iF][iS][iL][iP][iC] = new TH1D(hn.Data(), ht.Data(), nBin, m_LowerTimeBoundaryRPC, m_UpperTimeBoundaryRPC);

            hn = Form("hc_timeF%d_S%d_L%d_P%d_C%d", iF, iS, iL, iP, iC);
            ht = Form("Calibrated time distribtution for RPC of Channel%d, %s, Layer%d, Sector%d, %s; T_rec-T_0-T_fly-T_propagation-T_calibration[ns]",
                      iC, iPstring[iP].Data(), iL, iS, iFstring[iF].Data());
            hc_timeFSLPC[iF][iS][iL][iP][iC] = new TH1D(hn.Data(), ht.Data(), nBin, m_LowerTimeBoundaryCalibratedRPC,
                                                        m_UpperTimeBoundaryCalibratedRPC);
          }
        }
      }
    }
    // Endcap part
    int maxLay = 12 + 2 * iF;
    for (int iS = 0; iS < 4; ++iS) {
      hn = Form("h_timeF%d_S%d_scint_end", iF, iS);
      ht = Form("Time distribtution for Scintillator of Sector%d, %s (Endcap); T_rec-T_0-T_fly-T_propagation[ns]", iS,
                iFstring[iF].Data());
      h_timeFS_scint_end[iF][iS] = new TH1D(hn.Data(), ht.Data(), nBin_scint, m_LowerTimeBoundaryScintilltorsEKLM,
                                            m_UpperTimeBoundaryScintilltorsEKLM);
      hn = Form("h2_timeF%d_S%d_end", iF, iS);
      ht = Form("Time distribtution of Sector%d, %s (Endcap); Layer Index; T_rec-T_0-T_fly-T_propagation[ns]", iS, iFstring[iF].Data());
      h2_timeFS_end[iF][iS] = new TH2D(hn.Data(), ht.Data(), maxLay, 0, maxLay, nBin_scint, m_LowerTimeBoundaryScintilltorsEKLM,
                                       m_UpperTimeBoundaryScintilltorsEKLM);
      hn = Form("hc_timeF%d_S%d_scint_end", iF, iS);
      ht = Form("Calibrated time distribtution for Scintillator of Sector%d (Endcap), %s; T_rec-T_0-T_fly-T_propagation-T_calibration[ns]",
                iS, iFstring[iF].Data());
      hc_timeFS_scint_end[iF][iS] = new TH1D(hn.Data(), ht.Data(), nBin_scint, m_LowerTimeBoundaryCalibratedScintilltorsEKLM,
                                             m_UpperTimeBoundaryCalibratedScintilltorsEKLM);
      hn = Form("h2c_timeF%d_S%d_end", iF, iS);
      ht = Form("Calibrated time distribtution of Sector%d, %s (Endcap); Layer Index; T_rec-T_0-T_fly-T_propagation-T_calibration[ns]",
                iS, iFstring[iF].Data());
      h2c_timeFS_end[iF][iS] = new TH2D(hn.Data(), ht.Data(), maxLay, 0, maxLay, nBin_scint,
                                        m_LowerTimeBoundaryCalibratedScintilltorsEKLM,
                                        m_UpperTimeBoundaryCalibratedScintilltorsEKLM);

      for (int iL = 0; iL < maxLay; ++iL) {
        hn = Form("h_timeF%d_S%d_L%d_end", iF, iS, iL);
        ht = Form("Time distribtution for Scintillator of Layer%d, Sector%d, %s (Endcap); T_rec-T_0-T_fly-T_propagation[ns]", iL, iS,
                  iFstring[iF].Data());
        h_timeFSL_end[iF][iS][iL] = new TH1D(hn.Data(), ht.Data(), nBin_scint, m_LowerTimeBoundaryScintilltorsEKLM,
                                             m_UpperTimeBoundaryScintilltorsEKLM);
        hn = Form("hc_timeF%d_S%d_L%d_end", iF, iS, iL);
        ht = Form("Calibrated time distribtution for Scintillator of Layer%d, Sector%d, %s (Endcap); T_rec-T_0-T_fly-T_propagation-T_calibration[ns]",
                  iL, iS, iFstring[iF].Data());
        hc_timeFSL_end[iF][iS][iL] = new TH1D(hn.Data(), ht.Data(), nBin_scint, m_LowerTimeBoundaryCalibratedScintilltorsEKLM,
                                              m_UpperTimeBoundaryCalibratedScintilltorsEKLM);

        for (int iP = 0; iP < 2; ++iP) {
          hn = Form("h_timeF%d_S%d_L%d_P%d_end", iF, iS, iL, iP);
          ht = Form("Time distribtution for Scintillator of %s, Layer%d, Sector%d, %s (Endcap); T_rec-T_0-T_fly-T_propagation[ns]",
                    iPstring[iP].Data(), iL, iS, iFstring[iF].Data());
          h_timeFSLP_end[iF][iS][iL][iP] = new TH1D(hn.Data(), ht.Data(), nBin_scint, m_LowerTimeBoundaryScintilltorsEKLM,
                                                    m_UpperTimeBoundaryScintilltorsEKLM);

          hn = Form("h2_timeF%d_S%d_L%d_P%d_end", iF, iS, iL, iP);
          ht = Form("Time distribtution for Scintillator of %s, Layer%d, Sector%d, %s (Endcap); Channel Index; T_rec-T_0-T_fly-T_propagation[ns]",
                    iPstring[iP].Data(), iL, iS, iFstring[iF].Data());
          h2_timeFSLP_end[iF][iS][iL][iP] = new TH2D(hn.Data(), ht.Data(), 75, 0, 75, nBin_scint, m_LowerTimeBoundaryScintilltorsEKLM,
                                                     m_UpperTimeBoundaryScintilltorsEKLM);

          hn = Form("hc_timeF%d_S%d_L%d_P%d_end", iF, iS, iL, iP);
          ht = Form("Calibrated time distribtution for Scintillator of %s, Layer%d, Sector%d, %s (Endcap); T_rec-T_0-T_fly-T_propagation-T_calibration[ns]",
                    iPstring[iP].Data(), iL, iS, iFstring[iF].Data());
          hc_timeFSLP_end[iF][iS][iL][iP] = new TH1D(hn.Data(), ht.Data(), nBin_scint, m_LowerTimeBoundaryCalibratedScintilltorsEKLM,
                                                     m_UpperTimeBoundaryCalibratedScintilltorsEKLM);

          hn = Form("h2c_timeF%d_S%d_L%d_P%d_end", iF, iS, iL, iP);
          ht = Form("Calibrated time distribtution for Scintillator of %s, Layer%d, Sector%d, %s (Endcap); Channel Index; T_rec-T_0-T_fly-T_propagation-T_calibration[ns]",
                    iPstring[iP].Data(), iL, iS, iFstring[iF].Data());
          h2c_timeFSLP_end[iF][iS][iL][iP] = new TH2D(hn.Data(), ht.Data(), 75, 0, 75, nBin_scint,
                                                      m_LowerTimeBoundaryCalibratedScintilltorsEKLM,
                                                      m_UpperTimeBoundaryCalibratedScintilltorsEKLM);

          for (int iC = 0; iC < 75; ++iC) {
            hn = Form("h_timeF%d_S%d_L%d_P%d_C%d_tc_end", iF, iS, iL, iP, iC);
            ht = Form("Time distribtution for Scintillator of Channel%d, %s, Layer%d, Sector%d, %s (Endcap); T_rec-T_0-T_fly-T_propagation[ns]",
                      iC, iPstring[iP].Data(), iL, iS, iFstring[iF].Data());
            h_timeFSLPC_tc_end[iF][iS][iL][iP][iC] = new TH1D(hn.Data(), ht.Data(), nBin_scint, m_LowerTimeBoundaryScintilltorsEKLM,
                                                              m_UpperTimeBoundaryScintilltorsEKLM);

            hn = Form("h_timeF%d_S%d_L%d_P%d_C%d_end", iF, iS, iL, iP, iC);
            ht = Form("Time distribtution for Scintillator of Channel%d, %s, Layer%d, Sector%d, %s (Endcap); T_rec-T_0-T_fly-T_propagation[ns]",
                      iC, iPstring[iP].Data(), iL, iS, iFstring[iF].Data());
            h_timeFSLPC_end[iF][iS][iL][iP][iC] = new TH1D(hn.Data(), ht.Data(), nBin_scint, m_LowerTimeBoundaryCalibratedScintilltorsEKLM,
                                                           m_UpperTimeBoundaryCalibratedScintilltorsEKLM);

            hn = Form("hc_timeF%d_S%d_L%d_P%d_C%d_end", iF, iS, iL, iP, iC);
            ht = Form("Calibrated time distribtution for Scintillator of Channel%d, %s, Layer%d, Sector%d, %s (Endcap); T_rec-T_0-T_fly-T_propagation-T_calibration[ns]",
                      iC, iPstring[iP].Data(), iL, iS, iFstring[iF].Data());
            hc_timeFSLPC_end[iF][iS][iL][iP][iC] = new TH1D(hn.Data(), ht.Data(), nBin_scint, m_LowerTimeBoundaryCalibratedScintilltorsEKLM,
                                                            m_UpperTimeBoundaryCalibratedScintilltorsEKLM);
            hn = Form("time_length_eklm_F%d_S%d_L%d_P%d_C%d", iF, iS, iL, iP, iC);
            double stripLength = m_EKLMGeometry->getStripLength(iC + 1) /
                                 CLHEP::cm * Unit::cm;
            m_HistTimeLengthEKLM[iF][iS][iL][iP][iC] =
              new TH2F(hn.Data(),
                       "Time versus propagation length; "
                       "propagation distance[cm]; "
                       "T_rec-T_0-T_fly-'T_calibration'[ns]",
                       200, 0.0, stripLength,
                       400, m_LowerTimeBoundaryCalibratedScintilltorsEKLM,
                       m_UpperTimeBoundaryCalibratedScintilltorsEKLM);
          }
        }
      }
    }
  }
}

void KLMTimeAlgorithm::fillTimeDistanceProfiles(
  TProfile* profileRpcPhi, TProfile* profileRpcZ,
  TProfile* profileBKLMScintillatorPhi, TProfile* profileBKLMScintillatorZ,
  TProfile* profileEKLMScintillatorPlane1,
  TProfile* profileEKLMScintillatorPlane2, bool fill2dHistograms)
{
  for (KLMChannelIndex klmChannel = m_klmChannels.begin(); klmChannel != m_klmChannels.end(); ++klmChannel) {
    uint16_t channel = klmChannel.getKLMChannelNumber();
    if (m_cFlag[channel] == ChannelCalibrationStatus::c_NotEnoughData)
      continue;

    std::vector<struct Event>::iterator it;
    std::vector<struct Event> eventsChannel;
    eventsChannel = m_evts[channel];
    int iSub = klmChannel.getSubdetector();

    for (it = eventsChannel.begin(); it != eventsChannel.end(); ++it) {
      double timeHit = it->time() - m_timeShift[channel];
      if (m_useEventT0)
        timeHit = timeHit - it->t0;
      double distHit = it->dist;

      if (iSub == KLMElementNumbers::c_BKLM) {
        int iF = klmChannel.getSection();
        int iS = klmChannel.getSector() - 1;
        int iL = klmChannel.getLayer() - 1;
        int iP = klmChannel.getPlane();
        int iC = klmChannel.getStrip() - 1;
        if (iL > 1) {
          if (iP) {
            profileRpcPhi->Fill(distHit, timeHit);
          } else {
            profileRpcZ->Fill(distHit, timeHit);
          }
        } else {
          if (fill2dHistograms)
            m_HistTimeLengthBKLM[iF][iS][iL][iP][iC]->Fill(distHit, timeHit);
          if (iP) {
            profileBKLMScintillatorPhi->Fill(distHit, timeHit);
          } else {
            profileBKLMScintillatorZ->Fill(distHit, timeHit);
          }
        }
      } else {
        int iF = klmChannel.getSection() - 1;
        int iS = klmChannel.getSector() - 1;
        int iL = klmChannel.getLayer() - 1;
        int iP = klmChannel.getPlane() - 1;
        int iC = klmChannel.getStrip() - 1;
        if (fill2dHistograms)
          m_HistTimeLengthEKLM[iF][iS][iL][iP][iC]->Fill(distHit, timeHit);
        if (iP) {
          profileEKLMScintillatorPlane1->Fill(distHit, timeHit);
        } else {
          profileEKLMScintillatorPlane2->Fill(distHit, timeHit);
        }
      }
    }
  }
}

void KLMTimeAlgorithm::timeDistance2dFit(
  const std::vector< std::pair<uint16_t, unsigned int> > channels,
  double& delay, double& delayError)
{
  std::vector<struct Event>::iterator it;
  std::vector<struct Event> eventsChannel;
  int nFits = 1000;
  int nConvergedFits = 0;
  delay = 0;
  delayError = 0;
  if (nFits > (int)channels.size())
    nFits = channels.size();
  for (int i = 0; i < nFits; ++i) {
    int subdetector, section, sector, layer, plane, strip;
    m_ElementNumbers->channelNumberToElementNumbers(
      channels[i].first, &subdetector, &section, &sector, &layer, &plane,
      &strip);
    if (subdetector == KLMElementNumbers::c_BKLM) {
      s_LowerTimeBoundary = m_LowerTimeBoundaryScintilltorsBKLM;
      s_UpperTimeBoundary = m_UpperTimeBoundaryScintilltorsBKLM;
      const bklm::Module* module =
        m_BKLMGeometry->findModule(section, sector, layer);
      s_StripLength = module->getStripLength(plane, strip);
    } else {
      s_LowerTimeBoundary = m_LowerTimeBoundaryScintilltorsEKLM;
      s_UpperTimeBoundary = m_UpperTimeBoundaryScintilltorsEKLM;
      s_StripLength = m_EKLMGeometry->getStripLength(strip) /
                      CLHEP::cm * Unit::cm;
    }
    for (int j = 0; j < c_NBinsTime; ++j) {
      for (int k = 0; k < c_NBinsDistance; ++k)
        s_BinnedData[j][k] = 0;
    }
    eventsChannel = m_evts[channels[i].first];
    double averageTime = 0;
    for (it = eventsChannel.begin(); it != eventsChannel.end(); ++it) {
      double timeHit = it->time();
      if (m_useEventT0)
        timeHit = timeHit - it->t0;
      averageTime = averageTime + timeHit;
      int timeBin = std::floor((timeHit - s_LowerTimeBoundary) * c_NBinsTime /
                               (s_UpperTimeBoundary - s_LowerTimeBoundary));
      if (timeBin < 0 || timeBin >= c_NBinsTime)
        continue;
      int distanceBin = std::floor(it->dist * c_NBinsDistance / s_StripLength);
      if (distanceBin < 0 || distanceBin >= c_NBinsDistance) {
        B2ERROR("The distance to SiPM is greater than the strip length.");
        continue;
      }
      s_BinnedData[timeBin][distanceBin] += 1;
    }
    averageTime = averageTime / eventsChannel.size();
    TMinuit minuit(5);
    int minuitResult;
    minuit.mnparm(0, "P0", 1, 0.001, 0, 0, minuitResult);
    minuit.mnparm(1, "N", 10, 0.001, 0, 0, minuitResult);
    minuit.mnparm(2, "T0", averageTime, 0.001, 0, 0, minuitResult);
    minuit.mnparm(3, "SIGMA", 10, 0.001, 0, 0, minuitResult);
    minuit.mnparm(4, "DELAY", 0.0, 0.001, 0, 0, minuitResult);
    minuit.SetFCN(fcn);
    minuit.SetPrintLevel(-1);
    minuit.mncomd("FIX 2 3 4 5", minuitResult);
    minuit.mncomd("MIGRAD 10000", minuitResult);
    minuit.mncomd("RELEASE 2", minuitResult);
    minuit.mncomd("MIGRAD 10000", minuitResult);
    minuit.mncomd("RELEASE 3", minuitResult);
    minuit.mncomd("MIGRAD 10000", minuitResult);
    minuit.mncomd("RELEASE 4", minuitResult);
    minuit.mncomd("MIGRAD 10000", minuitResult);
    minuit.mncomd("RELEASE 5", minuitResult);
    minuit.mncomd("MIGRAD 10000", minuitResult);
    /* Require converged fit with accurate error matrix. */
    if (minuit.fISW[1] != 3)
      continue;
    nConvergedFits++;
    double channelDelay, channelDelayError;
    minuit.GetParameter(4, channelDelay, channelDelayError);
    delay = delay + channelDelay;
    delayError = delayError + channelDelayError * channelDelayError;
  }
  delay = delay / nConvergedFits;
  delayError = sqrt(delayError) / (nConvergedFits - 1);
}

CalibrationAlgorithm::EResult KLMTimeAlgorithm::calibrate()
{
  int channelId;
  gROOT->SetBatch(kTRUE);
  setupDatabase();
  m_timeCableDelay = new KLMTimeCableDelay();
  m_timeConstants = new KLMTimeConstants();

  fcn_gaus = new TF1("fcn_gaus", "gaus");
  fcn_land = new TF1("fcn_land", "landau");
  fcn_pol1 = new TF1("fcn_pol1", "pol1");
  fcn_const = new TF1("fcn_const", "pol0");

  CalibrationAlgorithm::EResult result = readCalibrationData();
  if (result != CalibrationAlgorithm::c_OK)
    return result;

  /* Choose non-existing file name. */
  std::string name = "time_calibration.root";
  int i = 1;
  while (1) {
    struct stat buffer;
    if (stat(name.c_str(), &buffer) != 0)
      break;
    name = "time_calibration_" + std::to_string(i) + ".root";
    i = i + 1;
    /* Overflow. */
    if (i < 0)
      break;
  }
  m_outFile = new TFile(name.c_str(), "recreate");
  createHistograms();

  std::vector<struct Event>::iterator it;
  std::vector<struct Event> eventsChannel;

  eventsChannel.clear();
  m_cFlag.clear();
  m_minimizerOptions.SetDefaultStrategy(2);

  /* Sort channels by number of events. */
  std::vector< std::pair<uint16_t, unsigned int> > channelsBKLM;
  std::vector< std::pair<uint16_t, unsigned int> > channelsEKLM;
  KLMChannelIndex klmChannels;
  for (KLMChannelIndex& klmChannel : klmChannels) {
    uint16_t channel = klmChannel.getKLMChannelNumber();
    m_cFlag[channel] = ChannelCalibrationStatus::c_NotEnoughData;
    if (m_evts.find(channel) == m_evts.end())
      continue;
    int nEvents = m_evts[channel].size();
    if (nEvents < m_lower_limit_counts) {
      B2WARNING("Not enough calibration data collected."
                << LogVar("channel", channel)
                << LogVar("number of digit", nEvents));
      continue;
    }
    m_cFlag[channel] = ChannelCalibrationStatus::c_FailedFit;
    if (klmChannel.getSubdetector() == KLMElementNumbers::c_BKLM &&
        klmChannel.getLayer() < BKLMElementNumbers::c_FirstRPCLayer) {
      channelsBKLM.push_back(
        std::pair<uint16_t, unsigned int>(channel, nEvents));
    }
    if (klmChannel.getSubdetector() == KLMElementNumbers::c_EKLM) {
      channelsEKLM.push_back(
        std::pair<uint16_t, unsigned int>(channel, nEvents));
    }
  }
  std::sort(channelsBKLM.begin(), channelsBKLM.end(), compareEventNumber);
  std::sort(channelsEKLM.begin(), channelsEKLM.end(), compareEventNumber);

  /* Two-dimensional fit for the channel with the maximal number of events. */
  double delayBKLM, delayBKLMError;
  double delayEKLM, delayEKLMError;
  timeDistance2dFit(channelsBKLM, delayBKLM, delayBKLMError);
  timeDistance2dFit(channelsEKLM, delayEKLM, delayEKLMError);

  /**********************************
   * First loop
   * Estimation of effective light speed for Scintillators and RPCs, separately.
   **********************************/
  B2INFO("Effective light speed Estimation.");
  for (KLMChannelIndex klmChannel = m_klmChannels.begin(); klmChannel != m_klmChannels.end(); ++klmChannel) {
    channelId = klmChannel.getKLMChannelNumber();
    if (m_cFlag[channelId] == ChannelCalibrationStatus::c_NotEnoughData)
      continue;

    eventsChannel = m_evts[channelId];
    int iSub = klmChannel.getSubdetector();

    for (it = eventsChannel.begin(); it != eventsChannel.end(); ++it) {
      TVector3 diffD = TVector3(it->diffDistX, it->diffDistY, it->diffDistZ);
      h_diff->Fill(diffD.Mag());
      double timeHit = it->time();
      if (m_useEventT0)
        timeHit = timeHit - it->t0;
      if (iSub == KLMElementNumbers::c_BKLM) {
        int iF = klmChannel.getSection();
        int iS = klmChannel.getSector() - 1;
        int iL = klmChannel.getLayer() - 1;
        int iP = klmChannel.getPlane();
        int iC = klmChannel.getStrip() - 1;
        h_timeFSLPC_tc[iF][iS][iL][iP][iC]->Fill(timeHit);
        if (iL > 1) {
          h_time_rpc_tc->Fill(timeHit);
        } else {
          h_time_scint_tc->Fill(timeHit);
        }
      } else {
        int iF = klmChannel.getSection() - 1;
        int iS = klmChannel.getSector() - 1;
        int iL = klmChannel.getLayer() - 1;
        int iP = klmChannel.getPlane() - 1;
        int iC = klmChannel.getStrip() - 1;
        h_timeFSLPC_tc_end[iF][iS][iL][iP][iC]->Fill(timeHit);
        h_time_scint_tc_end->Fill(timeHit);
      }
    }
  }
  B2INFO("Effective light speed Estimation! Hists and Graph filling done.");

  m_timeShift.clear();

  double tmpMean_rpc_global = h_time_rpc_tc->GetMean();
  double tmpMean_scint_global = h_time_scint_tc->GetMean();
  double tmpMean_scint_global_end = h_time_scint_tc_end->GetMean();

  B2INFO("Global Mean for Raw." << LogVar("RPC", tmpMean_rpc_global) << LogVar("Scint BKLM",
         tmpMean_scint_global) << LogVar("Scint EKLM", tmpMean_scint_global_end));

  for (KLMChannelIndex klmChannel = m_klmChannels.begin(); klmChannel != m_klmChannels.end(); ++klmChannel) {
    channelId = klmChannel.getKLMChannelNumber();
    if (m_cFlag[channelId] == ChannelCalibrationStatus::c_NotEnoughData)
      continue;

    int iSub = klmChannel.getSubdetector();
    if (iSub == KLMElementNumbers::c_BKLM) {
      int iF = klmChannel.getSection();
      int iS = klmChannel.getSector() - 1;
      int iL = klmChannel.getLayer() - 1;
      int iP = klmChannel.getPlane();
      int iC = klmChannel.getStrip() - 1;
      h_timeFSLPC_tc[iF][iS][iL][iP][iC]->Fit(fcn_gaus, "LESQ");
      double tmpMean_channel = fcn_gaus->GetParameter(1);
      if (iL > 1) {
        m_timeShift[channelId] = tmpMean_channel - tmpMean_rpc_global;
      } else {
        m_timeShift[channelId] = tmpMean_channel - tmpMean_scint_global;
      }
    } else {
      int iF = klmChannel.getSection() - 1;
      int iS = klmChannel.getSector() - 1;
      int iL = klmChannel.getLayer() - 1;
      int iP = klmChannel.getPlane() - 1;
      int iC = klmChannel.getStrip() - 1;
      h_timeFSLPC_tc_end[iF][iS][iL][iP][iC]->Fit(fcn_gaus, "LESQ");
      double tmpMean_channel = fcn_gaus->GetParameter(1);
      m_timeShift[channelId] = tmpMean_channel - tmpMean_scint_global_end;
    }
  }

  delete h_time_scint_tc;
  delete h_time_scint_tc_end;
  delete h_time_rpc_tc;
  B2INFO("Effective Light m_timeShift obtained. done.");

  fillTimeDistanceProfiles(
    m_ProfileRpcPhi, m_ProfileRpcZ,
    m_ProfileBKLMScintillatorPhi, m_ProfileBKLMScintillatorZ,
    m_ProfileEKLMScintillatorPlane1, m_ProfileEKLMScintillatorPlane2, false);

  B2INFO("Effective light speed fitting.");
  m_ProfileRpcPhi->Fit("fcn_pol1", "EMQ");
  double delayRPCPhi = fcn_pol1->GetParameter(1);
  double e_slope_rpc_phi = fcn_pol1->GetParError(1);

  m_ProfileRpcZ->Fit("fcn_pol1", "EMQ");
  double delayRPCZ = fcn_pol1->GetParameter(1);
  double e_slope_rpc_z = fcn_pol1->GetParError(1);

  m_ProfileBKLMScintillatorPhi->Fit("fcn_pol1", "EMQ");
  double slope_scint_phi = fcn_pol1->GetParameter(1);
  double e_slope_scint_phi = fcn_pol1->GetParError(1);

  m_ProfileBKLMScintillatorZ->Fit("fcn_pol1", "EMQ");
  double slope_scint_z = fcn_pol1->GetParameter(1);
  double e_slope_scint_z = fcn_pol1->GetParError(1);

  m_ProfileEKLMScintillatorPlane1->Fit("fcn_pol1", "EMQ");
  double slope_scint_plane1_end = fcn_pol1->GetParameter(1);
  double e_slope_scint_plane1_end = fcn_pol1->GetParError(1);

  m_ProfileEKLMScintillatorPlane2->Fit("fcn_pol1", "EMQ");
  double slope_scint_plane2_end = fcn_pol1->GetParameter(1);
  double e_slope_scint_plane2_end = fcn_pol1->GetParError(1);

  TString logStr_phi, logStr_z;
  logStr_phi = Form("%.4f +/- %.4f ns/cm", delayRPCPhi, e_slope_rpc_phi);
  logStr_z = Form("%.4f +/- %.4f ns/cm", delayRPCZ, e_slope_rpc_z);
  B2INFO("Delay in RPCs:"
         << LogVar("Fitted Value (phi readout) ", logStr_phi.Data())
         << LogVar("Fitted Value (z readout) ", logStr_z.Data()));
  logStr_phi = Form("%.4f +/- %.4f ns/cm", slope_scint_phi, e_slope_scint_phi);
  logStr_z = Form("%.4f +/- %.4f ns/cm", slope_scint_z, e_slope_scint_z);
  B2INFO("Delay in BKLM scintillators:"
         << LogVar("Fitted Value (phi readout) ", logStr_phi.Data())
         << LogVar("Fitted Value (z readout) ", logStr_z.Data()));
  logStr_phi = Form("%.4f +/- %.4f ns/cm", slope_scint_plane1_end,
                    e_slope_scint_plane1_end);
  logStr_z = Form("%.4f +/- %.4f ns/cm", slope_scint_plane2_end,
                  e_slope_scint_plane2_end);
  B2INFO("Delay in EKLM scintillators:"
         << LogVar("Fitted Value (plane1 readout) ", logStr_phi.Data())
         << LogVar("Fitted Value (plane2 readout) ", logStr_z.Data()));

  logStr_z = Form("%.4f +/- %.4f ns/cm", delayBKLM, delayBKLMError);
  B2INFO("Delay in BKLM scintillators:"
         << LogVar("Fitted Value (2d fit) ", logStr_z.Data()));
  logStr_z = Form("%.4f +/- %.4f ns/cm", delayEKLM, delayEKLMError);
  B2INFO("Delay in EKLM scintillators:"
         << LogVar("Fitted Value (2d fit) ", logStr_z.Data()));

  // Default Effective light speed in current Database
  //delayEKLM = 0.5 * (slope_scint_plane1_end + slope_scint_plane2_end);
  //delayBKLM = 0.5 * (slope_scint_phi + slope_scint_z);

  m_timeConstants->setDelay(delayEKLM, KLMTimeConstants::c_EKLM);
  m_timeConstants->setDelay(delayBKLM, KLMTimeConstants::c_BKLM);
  m_timeConstants->setDelay(delayRPCPhi, KLMTimeConstants::c_RPCPhi);
  m_timeConstants->setDelay(delayRPCZ, KLMTimeConstants::c_RPCZ);

  /** ======================================================================================= **/
  B2INFO("Time distribution filling begins.");
  for (KLMChannelIndex klmChannel = m_klmChannels.begin(); klmChannel != m_klmChannels.end(); ++klmChannel) {
    channelId = klmChannel.getKLMChannelNumber();
    int iSub = klmChannel.getSubdetector();

    if (m_cFlag[channelId] == ChannelCalibrationStatus::c_NotEnoughData)
      continue;
    eventsChannel = m_evts[channelId];

    for (it = eventsChannel.begin(); it != eventsChannel.end(); ++it) {
      double timeHit = it->time();
      if (m_useEventT0)
        timeHit = timeHit - it->t0;
      if (iSub == KLMElementNumbers::c_BKLM) {
        int iF = klmChannel.getSection();
        int iS = klmChannel.getSector() - 1;
        int iL = klmChannel.getLayer() - 1;
        int iP = klmChannel.getPlane();
        int iC = klmChannel.getStrip() - 1;
        if (iL > 1) {
          double propgationT;
          if (iP == BKLMElementNumbers::c_ZPlane)
            propgationT = it->dist * delayRPCZ;
          else
            propgationT = it->dist * delayRPCPhi;
          double time = timeHit - propgationT;
          h_time_rpc->Fill(time);
          h_timeF_rpc[iF]->Fill(time);
          h_timeFS_rpc[iF][iS]->Fill(time);
          h_timeFSL[iF][iS][iL]->Fill(time);
          h_timeFSLP[iF][iS][iL][iP]->Fill(time);
          h_timeFSLPC[iF][iS][iL][iP][iC]->Fill(time);
          h2_timeF_rpc[iF]->Fill(iS, time);
          h2_timeFS[iF][iS]->Fill(iL, time);
          h2_timeFSLP[iF][iS][iL][iP]->Fill(iC, time);
        } else {
          double propgationT = it->dist * delayBKLM;
          double time = timeHit - propgationT;
          h_time_scint->Fill(time);
          h_timeF_scint[iF]->Fill(time);
          h_timeFS_scint[iF][iS]->Fill(time);
          h_timeFSL[iF][iS][iL]->Fill(time);
          h_timeFSLP[iF][iS][iL][iP]->Fill(time);
          h_timeFSLPC[iF][iS][iL][iP][iC]->Fill(time);
          h2_timeF_scint[iF]->Fill(iS, time);
          h2_timeFS[iF][iS]->Fill(iL, time);
          h2_timeFSLP[iF][iS][iL][iP]->Fill(iC, time);
        }
      } else {
        int iF = klmChannel.getSection() - 1;
        int iS = klmChannel.getSector() - 1;
        int iL = klmChannel.getLayer() - 1;
        int iP = klmChannel.getPlane() - 1;
        int iC = klmChannel.getStrip() - 1;
        double propgationT = it->dist * delayEKLM;
        double time = timeHit - propgationT;
        h_time_scint_end->Fill(time);
        h_timeF_scint_end[iF]->Fill(time);
        h_timeFS_scint_end[iF][iS]->Fill(time);
        h_timeFSL_end[iF][iS][iL]->Fill(time);
        h_timeFSLP_end[iF][iS][iL][iP]->Fill(time);
        h_timeFSLPC_end[iF][iS][iL][iP][iC]->Fill(time);
        h2_timeF_scint_end[iF]->Fill(iS, time);
        h2_timeFS_end[iF][iS]->Fill(iL, time);
        h2_timeFSLP_end[iF][iS][iL][iP]->Fill(iC, time);
      }
    }
  }

  B2INFO("Orignal filling done.");

  int iChannel_rpc = 0;
  int iChannel = 0;
  int iChannel_end = 0;
  for (KLMChannelIndex klmChannel = m_klmChannels.begin(); klmChannel != m_klmChannels.end(); ++klmChannel) {
    channelId = klmChannel.getKLMChannelNumber();
    if (m_cFlag[channelId] == ChannelCalibrationStatus::c_NotEnoughData)
      continue;
    int iSub = klmChannel.getSubdetector();

    if (iSub == KLMElementNumbers::c_BKLM) {
      int iF = klmChannel.getSection();
      int iS = klmChannel.getSector() - 1;
      int iL = klmChannel.getLayer() - 1;
      int iP = klmChannel.getPlane();
      int iC = klmChannel.getStrip() - 1;

      TFitResultPtr r = h_timeFSLPC[iF][iS][iL][iP][iC]->Fit(fcn_gaus, "LESQ");
      if (int(r) != 0)
        continue;
      if (int(r) == 0)
        m_cFlag[channelId] = ChannelCalibrationStatus::c_SuccessfulCalibration;
      m_time_channel[channelId] = fcn_gaus->GetParameter(1);
      m_etime_channel[channelId] = fcn_gaus->GetParError(1);
      if (iL > 1) {
        gre_time_channel_rpc->SetPoint(iChannel, channelId, m_time_channel[channelId]);
        gre_time_channel_rpc->SetPointError(iChannel, 0., m_etime_channel[channelId]);
        iChannel++;
      } else {
        gre_time_channel_scint->SetPoint(iChannel_rpc, channelId, m_time_channel[channelId]);
        gre_time_channel_scint->SetPointError(iChannel_rpc, 0., m_etime_channel[channelId]);
        iChannel_rpc++;
      }
    } else {
      int iF = klmChannel.getSection() - 1;
      int iS = klmChannel.getSector() - 1;
      int iL = klmChannel.getLayer() - 1;
      int iP = klmChannel.getPlane() - 1;
      int iC = klmChannel.getStrip() - 1;

      TFitResultPtr r = h_timeFSLPC_end[iF][iS][iL][iP][iC]->Fit(fcn_gaus, "LESQ");
      if (int(r) != 0)
        continue;
      if (int(r) == 0)
        m_cFlag[channelId] = ChannelCalibrationStatus::c_SuccessfulCalibration;
      m_time_channel[channelId] = fcn_gaus->GetParameter(1);
      m_etime_channel[channelId] = fcn_gaus->GetParError(1);
      gre_time_channel_scint_end->SetPoint(iChannel_end, channelId, m_time_channel[channelId]);
      gre_time_channel_scint_end->SetPointError(iChannel_end, 0., m_etime_channel[channelId]);
      iChannel_end++;
    }
  }

  gre_time_channel_scint->Fit("fcn_const", "EMQ");
  m_time_channelAvg_scint = fcn_const->GetParameter(0);
  m_etime_channelAvg_scint = fcn_const->GetParError(0);

  gre_time_channel_scint_end->Fit("fcn_const", "EMQ");
  m_time_channelAvg_scint_end = fcn_const->GetParameter(0);
  m_etime_channelAvg_scint_end = fcn_const->GetParError(0);

  gre_time_channel_rpc->Fit("fcn_const", "EMQ");
  m_time_channelAvg_rpc = fcn_const->GetParameter(0);
  m_etime_channelAvg_rpc = fcn_const->GetParError(0);

  B2INFO("Channel's time distribution fitting done.");
  B2DEBUG(20, LogVar("Average time (RPC)", m_time_channelAvg_rpc)
          << LogVar("Average time (BKLM scintillators)", m_time_channelAvg_scint)
          << LogVar("Average time (EKLM scintillators)", m_time_channelAvg_scint_end));

  B2INFO("Calibrated channel's time distribution filling begins.");

  m_timeShift.clear();
  for (KLMChannelIndex klmChannel = m_klmChannels.begin(); klmChannel != m_klmChannels.end(); ++klmChannel) {
    channelId = klmChannel.getKLMChannelNumber();
    h_calibrated->Fill(m_cFlag[channelId]);
    if (m_time_channel.find(channelId) == m_time_channel.end())
      continue;
    double timeShift = m_time_channel[channelId];
    int iSub = klmChannel.getSubdetector();
    if (iSub == KLMElementNumbers::c_BKLM) {
      int iL = klmChannel.getLayer() - 1;
      if (iL > 1)
        m_timeShift[channelId] = timeShift;
      else
        m_timeShift[channelId] = timeShift;
    } else {
      m_timeShift[channelId] = timeShift;
    }
    m_timeCableDelay->setTimeDelay(channelId, m_timeShift[channelId]);
  }

  for (KLMChannelIndex klmChannel = m_klmChannels.begin(); klmChannel != m_klmChannels.end(); ++klmChannel) {
    channelId = klmChannel.getKLMChannelNumber();
    if (m_timeShift.find(channelId) != m_timeShift.end())
      continue;
    m_timeShift[channelId] = esti_timeShift(klmChannel);
    m_timeCableDelay->setTimeDelay(channelId, m_timeShift[channelId]);
    B2DEBUG(20, "Uncalibrated Estimation " << LogVar("Channel", channelId) << LogVar("Estimated value", m_timeShift[channelId]));
  }

  iChannel_rpc = 0;
  iChannel = 0;
  iChannel_end = 0;
  for (KLMChannelIndex klmChannel = m_klmChannels.begin(); klmChannel != m_klmChannels.end(); ++klmChannel) {
    channelId = klmChannel.getKLMChannelNumber();
    if (m_timeShift.find(channelId) == m_timeShift.end()) {
      B2ERROR("!!! Not All Channels Calibration Constant Set. Error Happended on " << LogVar("Channel", channelId));
      continue;
    }
    int iSub = klmChannel.getSubdetector();
    if (iSub == KLMElementNumbers::c_BKLM) {
      int iL = klmChannel.getLayer();
      if (iL > 2) {
        gr_timeShift_channel_rpc->SetPoint(iChannel_rpc, channelId, m_timeShift[channelId]);
        iChannel_rpc++;
      } else {
        gr_timeShift_channel_scint->SetPoint(iChannel, channelId, m_timeShift[channelId]);
        iChannel++;
      }
    } else {
      gr_timeShift_channel_scint_end->SetPoint(iChannel_end, channelId, m_timeShift[channelId]);
      iChannel_end++;
    }
  }

  fillTimeDistanceProfiles(
    m_Profile2RpcPhi, m_Profile2RpcZ,
    m_Profile2BKLMScintillatorPhi, m_Profile2BKLMScintillatorZ,
    m_Profile2EKLMScintillatorPlane1,  m_Profile2EKLMScintillatorPlane2, true);
  for (KLMChannelIndex klmChannel = m_klmChannels.begin(); klmChannel != m_klmChannels.end(); ++klmChannel) {
    channelId = klmChannel.getKLMChannelNumber();
    int iSub = klmChannel.getSubdetector();
    eventsChannel = m_evts[channelId];
    for (it = eventsChannel.begin(); it != eventsChannel.end(); ++it) {
      double timeHit = it->time();
      if (m_useEventT0)
        timeHit = timeHit - it->t0;
      if (iSub == KLMElementNumbers::c_BKLM) {
        int iF = klmChannel.getSection();
        int iS = klmChannel.getSector() - 1;
        int iL = klmChannel.getLayer() - 1;
        int iP = klmChannel.getPlane();
        int iC = klmChannel.getStrip() - 1;
        if (iL > 1) {
          double propgationT;
          if (iP == BKLMElementNumbers::c_ZPlane)
            propgationT = it->dist * delayRPCZ;
          else
            propgationT = it->dist * delayRPCPhi;
          double time = timeHit - propgationT - m_timeShift[channelId];
          hc_time_rpc->Fill(time);
          hc_timeF_rpc[iF]->Fill(time);
          hc_timeFS_rpc[iF][iS]->Fill(time);
          hc_timeFSL[iF][iS][iL]->Fill(time);
          hc_timeFSLP[iF][iS][iL][iP]->Fill(time);
          hc_timeFSLPC[iF][iS][iL][iP][iC]->Fill(time);
          h2c_timeF_rpc[iF]->Fill(iS, time);
          h2c_timeFS[iF][iS]->Fill(iL, time);
          h2c_timeFSLP[iF][iS][iL][iP]->Fill(iC, time);
        } else {
          double propgationT = it->dist * delayBKLM;
          double time = timeHit - propgationT - m_timeShift[channelId];
          hc_time_scint->Fill(time);
          hc_timeF_scint[iF]->Fill(time);
          hc_timeFS_scint[iF][iS]->Fill(time);
          hc_timeFSL[iF][iS][iL]->Fill(time);
          hc_timeFSLP[iF][iS][iL][iP]->Fill(time);
          hc_timeFSLPC[iF][iS][iL][iP][iC]->Fill(time);
          h2c_timeF_scint[iF]->Fill(iS, time);
          h2c_timeFS[iF][iS]->Fill(iL, time);
          h2c_timeFSLP[iF][iS][iL][iP]->Fill(iC, time);
        }
      } else {
        int iF = klmChannel.getSection() - 1;
        int iS = klmChannel.getSector() - 1;
        int iL = klmChannel.getLayer() - 1;
        int iP = klmChannel.getPlane() - 1;
        int iC = klmChannel.getStrip() - 1;
        double propgationT = it->dist * delayEKLM;
        double time = timeHit - propgationT - m_timeShift[channelId];
        hc_time_scint_end->Fill(time);
        hc_timeF_scint_end[iF]->Fill(time);
        hc_timeFS_scint_end[iF][iS]->Fill(time);
        hc_timeFSL_end[iF][iS][iL]->Fill(time);
        hc_timeFSLP_end[iF][iS][iL][iP]->Fill(time);
        hc_timeFSLPC_end[iF][iS][iL][iP][iC]->Fill(time);
        h2c_timeF_scint_end[iF]->Fill(iS, time);
        h2c_timeFS_end[iF][iS]->Fill(iL, time);
        h2c_timeFSLP_end[iF][iS][iL][iP]->Fill(iC, time);
      }
    }
  }

  saveHist();

  delete fcn_const;
  m_evts.clear();
  m_timeShift.clear();
  m_cFlag.clear();

  saveCalibration(m_timeCableDelay, "KLMTimeCableDelay");
  saveCalibration(m_timeConstants, "KLMTimeConstants");
  return CalibrationAlgorithm::c_OK;
}


void KLMTimeAlgorithm::saveHist()
{
  m_outFile->cd();
  B2INFO("Save Histograms into Files.");
  TDirectory* dir_monitor = m_outFile->mkdir("monitor_Hists");
  dir_monitor->cd();
  h_calibrated->SetDirectory(dir_monitor);
  h_diff->SetDirectory(dir_monitor);

  m_outFile->cd();
  TDirectory* dir_effC = m_outFile->mkdir("effC_Hists");
  dir_effC->cd();
  m_ProfileRpcPhi->SetDirectory(dir_effC);
  m_ProfileRpcZ->SetDirectory(dir_effC);
  m_ProfileBKLMScintillatorPhi->SetDirectory(dir_effC);
  m_ProfileBKLMScintillatorZ->SetDirectory(dir_effC);
  m_ProfileEKLMScintillatorPlane1->SetDirectory(dir_effC);
  m_ProfileEKLMScintillatorPlane2->SetDirectory(dir_effC);
  m_Profile2RpcPhi->SetDirectory(dir_effC);
  m_Profile2RpcZ->SetDirectory(dir_effC);
  m_Profile2BKLMScintillatorPhi->SetDirectory(dir_effC);
  m_Profile2BKLMScintillatorZ->SetDirectory(dir_effC);
  m_Profile2EKLMScintillatorPlane1->SetDirectory(dir_effC);
  m_Profile2EKLMScintillatorPlane2->SetDirectory(dir_effC);

  m_outFile->cd();
  TDirectory* dir_time = m_outFile->mkdir("time");
  dir_time->cd();

  h_time_scint->SetDirectory(dir_time);
  hc_time_scint->SetDirectory(dir_time);

  h_time_scint_end->SetDirectory(dir_time);
  hc_time_scint_end->SetDirectory(dir_time);

  h_time_rpc->SetDirectory(dir_time);
  hc_time_rpc->SetDirectory(dir_time);

  gre_time_channel_rpc->Write("gre_time_channel_rpc");
  gre_time_channel_scint->Write("gre_time_channel_scint");
  gre_time_channel_scint_end->Write("gre_time_channel_scint_end");
  gr_timeShift_channel_rpc->Write("gr_timeShift_channel_rpc");
  gr_timeShift_channel_scint->Write("gr_timeShift_channel_scint");
  gr_timeShift_channel_scint_end->Write("gr_timeShift_channel_scint_end");

  B2INFO("Top file setup Done.");

  TDirectory* dir_time_F[2];
  TDirectory* dir_time_FS[2][8];
  TDirectory* dir_time_FSL[2][8][15];
  TDirectory* dir_time_FSLP[2][8][15][2];
  TDirectory* dir_time_F_end[2];
  TDirectory* dir_time_FS_end[2][4];
  TDirectory* dir_time_FSL_end[2][4][14];
  TDirectory* dir_time_FSLP_end[2][4][14][2];
  char dirname[50];
  B2INFO("Sub files declare Done.");
  for (int iF = 0; iF < 2; ++iF) {
    h_timeF_rpc[iF]->SetDirectory(dir_time);
    hc_timeF_rpc[iF]->SetDirectory(dir_time);

    h2_timeF_rpc[iF]->SetDirectory(dir_time);
    h2c_timeF_rpc[iF]->SetDirectory(dir_time);

    h_timeF_scint[iF]->SetDirectory(dir_time);
    hc_timeF_scint[iF]->SetDirectory(dir_time);

    h2_timeF_scint[iF]->SetDirectory(dir_time);
    h2c_timeF_scint[iF]->SetDirectory(dir_time);

    h_timeF_scint_end[iF]->SetDirectory(dir_time);
    hc_timeF_scint_end[iF]->SetDirectory(dir_time);

    h2_timeF_scint_end[iF]->SetDirectory(dir_time);
    h2c_timeF_scint_end[iF]->SetDirectory(dir_time);

    sprintf(dirname, "isForward_%d", iF);
    dir_time_F[iF] = dir_time->mkdir(dirname);
    dir_time_F[iF]->cd();

    for (int iS = 0; iS < 8; ++iS) {
      h_timeFS_rpc[iF][iS]->SetDirectory(dir_time_F[iF]);
      hc_timeFS_rpc[iF][iS]->SetDirectory(dir_time_F[iF]);

      h_timeFS_scint[iF][iS]->SetDirectory(dir_time_F[iF]);
      hc_timeFS_scint[iF][iS]->SetDirectory(dir_time_F[iF]);

      h2_timeFS[iF][iS]->SetDirectory(dir_time_F[iF]);
      h2c_timeFS[iF][iS]->SetDirectory(dir_time_F[iF]);

      sprintf(dirname, "Sector_%d", iS + 1);
      dir_time_FS[iF][iS] = dir_time_F[iF]->mkdir(dirname);
      dir_time_FS[iF][iS]->cd();

      for (int iL = 0; iL < 15; ++iL) {
        h_timeFSL[iF][iS][iL]->SetDirectory(dir_time_FS[iF][iS]);
        hc_timeFSL[iF][iS][iL]->SetDirectory(dir_time_FS[iF][iS]);

        sprintf(dirname, "Layer_%d", iL + 1);
        dir_time_FSL[iF][iS][iL] = dir_time_FS[iF][iS]->mkdir(dirname);
        dir_time_FSL[iF][iS][iL]->cd();
        for (int iP = 0; iP < 2; ++iP) {
          h_timeFSLP[iF][iS][iL][iP]->SetDirectory(dir_time_FSL[iF][iS][iL]);
          hc_timeFSLP[iF][iS][iL][iP]->SetDirectory(dir_time_FSL[iF][iS][iL]);
          h2_timeFSLP[iF][iS][iL][iP]->SetDirectory(dir_time_FSL[iF][iS][iL]);
          h2c_timeFSLP[iF][iS][iL][iP]->SetDirectory(dir_time_FSL[iF][iS][iL]);

          sprintf(dirname, "Plane_%d", iP);
          dir_time_FSLP[iF][iS][iL][iP] = dir_time_FSL[iF][iS][iL]->mkdir(dirname);
          dir_time_FSLP[iF][iS][iL][iP]->cd();

          int nchannel_max = BKLMElementNumbers::getNStrips(iF, iS + 1, iL + 1, iP);
          for (int iC = 0; iC < nchannel_max; ++iC) {
            if (iL < 2)
              m_HistTimeLengthBKLM[iF][iS][iL][iP][iC]->SetDirectory(dir_time_FSLP[iF][iS][iL][iP]);
            h_timeFSLPC[iF][iS][iL][iP][iC]->SetDirectory(dir_time_FSLP[iF][iS][iL][iP]);
            hc_timeFSLPC[iF][iS][iL][iP][iC]->SetDirectory(dir_time_FSLP[iF][iS][iL][iP]);
            delete h_timeFSLPC_tc[iF][iS][iL][iP][iC];
          }
        }
      }
    }

    sprintf(dirname, "isForward_%d_end", iF + 1);
    dir_time_F_end[iF] = dir_time->mkdir(dirname);
    dir_time_F_end[iF]->cd();
    int maxLayer = 12 + 2 * iF;
    for (int iS = 0; iS < 4; ++iS) {
      h_timeFS_scint_end[iF][iS]->SetDirectory(dir_time_F_end[iF]);
      hc_timeFS_scint_end[iF][iS]->SetDirectory(dir_time_F_end[iF]);

      h2_timeFS_end[iF][iS]->SetDirectory(dir_time_F_end[iF]);
      h2c_timeFS_end[iF][iS]->SetDirectory(dir_time_F_end[iF]);

      sprintf(dirname, "Sector_%d_end", iS + 1);
      dir_time_FS_end[iF][iS] = dir_time_F_end[iF]->mkdir(dirname);
      dir_time_FS_end[iF][iS]->cd();
      for (int iL = 0; iL < maxLayer; ++iL) {
        h_timeFSL_end[iF][iS][iL]->SetDirectory(dir_time_FS_end[iF][iS]);
        hc_timeFSL_end[iF][iS][iL]->SetDirectory(dir_time_FS_end[iF][iS]);

        sprintf(dirname, "Layer_%d_end", iL + 1);
        dir_time_FSL_end[iF][iS][iL] = dir_time_FS_end[iF][iS]->mkdir(dirname);
        dir_time_FSL_end[iF][iS][iL]->cd();
        for (int iP = 0; iP < 2; ++iP) {
          h_timeFSLP_end[iF][iS][iL][iP]->SetDirectory(dir_time_FSL_end[iF][iS][iL]);
          hc_timeFSLP_end[iF][iS][iL][iP]->SetDirectory(dir_time_FSL_end[iF][iS][iL]);
          h2_timeFSLP_end[iF][iS][iL][iP]->SetDirectory(dir_time_FSL_end[iF][iS][iL]);
          h2c_timeFSLP_end[iF][iS][iL][iP]->SetDirectory(dir_time_FSL_end[iF][iS][iL]);

          sprintf(dirname, "plane_%d_end", iP);
          dir_time_FSLP_end[iF][iS][iL][iP] = dir_time_FSL_end[iF][iS][iL]->mkdir(dirname);
          dir_time_FSLP_end[iF][iS][iL][iP]->cd();

          for (int iC = 0; iC < 75; ++iC) {
            h_timeFSLPC_end[iF][iS][iL][iP][iC]->SetDirectory(dir_time_FSLP_end[iF][iS][iL][iP]);
            hc_timeFSLPC_end[iF][iS][iL][iP][iC]->SetDirectory(dir_time_FSLP_end[iF][iS][iL][iP]);
            m_HistTimeLengthEKLM[iF][iS][iL][iP][iC]->SetDirectory(dir_time_FSLP_end[iF][iS][iL][iP]);
            delete h_timeFSLPC_tc_end[iF][iS][iL][iP][iC];
          }
        }
      }
    }
  }
  m_outFile->cd();
  m_outFile->Write();
  m_outFile->Close();
  B2INFO("File Write and Close. Done.");
}

double KLMTimeAlgorithm::esti_timeShift(const KLMChannelIndex& klmChannel)
{
  double tS = 0.0;
  int iSub = klmChannel.getSubdetector();
  int iF = klmChannel.getSection();
  int iS = klmChannel.getSector();
  int iL = klmChannel.getLayer();
  int iP = klmChannel.getPlane();
  int iC = klmChannel.getStrip();
  int totNStrips = EKLMElementNumbers::getMaximalStripNumber();
  if (iSub == KLMElementNumbers::c_BKLM)
    totNStrips = BKLMElementNumbers::getNStrips(iF, iS, iL, iP);
  if (iC == 1) {
    KLMChannelIndex kCIndex_upper(iSub, iF, iS, iL, iP, iC + 1);
    tS = tS_upperStrip(kCIndex_upper).second;
  } else if (iC == totNStrips) {
    KLMChannelIndex kCIndex_lower(iSub, iF, iS, iL, iP, iC - 1);
    tS = tS_lowerStrip(kCIndex_lower).second;
  } else {
    KLMChannelIndex kCIndex_upper(iSub, iF, iS, iL, iP, iC + 1);
    KLMChannelIndex kCIndex_lower(iSub, iF, iS, iL, iP, iC - 1);
    std::pair<int, double> tS_upper = tS_upperStrip(kCIndex_upper);
    std::pair<int, double> tS_lower = tS_lowerStrip(kCIndex_lower);
    unsigned int td_upper = tS_upper.first - iC;
    unsigned int td_lower = iC - tS_lower.first;
    unsigned int td = tS_upper.first - tS_lower.first;
    tS = (double(td_upper) * tS_lower.second + double(td_lower) * tS_upper.second) / double(td);
  }
  return tS;
}

std::pair<int, double> KLMTimeAlgorithm::tS_upperStrip(const KLMChannelIndex& klmChannel)
{
  std::pair<int, double> tS;
  int cId = klmChannel.getKLMChannelNumber();
  int iSub = klmChannel.getSubdetector();
  int iF = klmChannel.getSection();
  int iS = klmChannel.getSector();
  int iL = klmChannel.getLayer();
  int iP = klmChannel.getPlane();
  int iC = klmChannel.getStrip();
  int totNStrips = EKLMElementNumbers::getMaximalStripNumber();
  if (iSub == KLMElementNumbers::c_BKLM)
    totNStrips = BKLMElementNumbers::getNStrips(iF, iS, iL, iP);
  if (m_timeShift.find(cId) != m_timeShift.end()) {
    tS.first = iC;
    tS.second = m_timeShift[cId];
  } else if (iC == totNStrips) {
    tS.first = iC;
    tS.second = 0.0;
  } else {
    KLMChannelIndex kCIndex(iSub, iF, iS, iL, iP, iC + 1);
    tS = tS_upperStrip(kCIndex);
  }
  return tS;
}

std::pair<int, double> KLMTimeAlgorithm::tS_lowerStrip(const KLMChannelIndex& klmChannel)
{
  std::pair<int, double> tS;
  int cId = klmChannel.getKLMChannelNumber();
  int iSub = klmChannel.getSubdetector();
  int iF = klmChannel.getSection();
  int iS = klmChannel.getSector();
  int iL = klmChannel.getLayer();
  int iP = klmChannel.getPlane();
  int iC = klmChannel.getStrip();
  if (m_timeShift.find(cId) != m_timeShift.end()) {
    tS.first = iC;
    tS.second = m_timeShift[cId];
  } else if (iC == 1) {
    tS.first = iC;
    tS.second = 0.0;
  } else {
    KLMChannelIndex kCIndex(iSub, iF, iS, iL, iP, iC - 1);
    tS = tS_lowerStrip(kCIndex);
  }
  return tS;
}
