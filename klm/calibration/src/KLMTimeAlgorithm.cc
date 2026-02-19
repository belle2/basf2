/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

/* Own header. */
#include <klm/calibration/KLMTimeAlgorithm.h>

/* KLM headers. */
#include <klm/dataobjects/bklm/BKLMElementNumbers.h>

/* Basf2 headers. */
#include <framework/database/Database.h>
#include <framework/database/DBObjPtr.h>
#include <framework/database/DBStore.h>
#include <framework/dataobjects/EventMetaData.h>
#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>

/* ROOT headers. */
#include <Math/MinimizerOptions.h>
#include <Math/Vector3D.h>
#include <TFile.h>
#include <TFitResult.h>
#include <TMinuit.h>
#include <TROOT.h>
#include <TString.h>
#include <TTree.h>

/* C++ headers. */
#include <functional>
#include <set>

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

static bool compareEventNumber(const std::pair<KLMChannelNumber, unsigned int>& pair1,
                               const std::pair<KLMChannelNumber, unsigned int>& pair2)
{
  return pair1.second < pair2.second;
}

static double timeDensity(const double x[2], const double* par)
{
  double polynomial, t0, gauss;
  polynomial = par[0];
  t0 = par[2] + par[4] * x[1];
  gauss = par[1] / (sqrt(2.0 * M_PI) * par[3]) *
          exp(-0.5 * pow((x[0] - t0) / par[3], 2));
  return fabs(polynomial + gauss);
}

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

// Version 1: Initial check only (don't load data)
CalibrationAlgorithm::EResult KLMTimeAlgorithm::readCalibrationData()
{
  B2INFO("Read tree entries (initial data check only).");
  std::shared_ptr<TTree> timeCalibrationData;
  timeCalibrationData = getObjectPtr<TTree>("time_calibration_data");

  int n = timeCalibrationData->GetEntries();
  B2INFO(LogVar("Total number of digits:", n));

  if (n < m_MinimalDigitNumber)
    return CalibrationAlgorithm::c_NotEnoughData;

  return CalibrationAlgorithm::c_OK;
}

void KLMTimeAlgorithm::readCalibrationDataCounts(std::map<KLMChannelNumber, unsigned int>& eventCounts)
{
  B2INFO("Counting events per channel (lightweight scan)...");
  Event event;
  std::shared_ptr<TTree> timeCalibrationData;
  timeCalibrationData = getObjectPtr<TTree>("time_calibration_data");
  timeCalibrationData->SetBranchAddress("channelId", &event.channelId);

  eventCounts.clear();

  int n = timeCalibrationData->GetEntries();
  for (int i = 0; i < n; ++i) {
    timeCalibrationData->GetEntry(i);
    eventCounts[event.channelId]++;
  }

  B2INFO("Event counting complete." << LogVar("Total events", n) << LogVar("Unique channels", eventCounts.size()));
}

void KLMTimeAlgorithm::readCalibrationDataFor2DFit(
  const std::vector<std::pair<KLMChannelNumber, unsigned int>>& channelsBKLM,
  const std::vector<std::pair<KLMChannelNumber, unsigned int>>& channelsEKLM)
{
  B2INFO("Loading data for 2D fit (top 1000 channels from BKLM and EKLM)...");
  Event event;
  std::shared_ptr<TTree> timeCalibrationData;
  timeCalibrationData = getObjectPtr<TTree>("time_calibration_data");
  timeCalibrationData->SetBranchAddress("Run", &event.Run);
  timeCalibrationData->SetBranchAddress("Event", &event.Events);
  timeCalibrationData->SetBranchAddress("nTrack", &event.nTrack);
  timeCalibrationData->SetBranchAddress("Track_Charge", &event.Track_Charge);

  timeCalibrationData->SetBranchAddress("t0", &event.t0);
  timeCalibrationData->SetBranchAddress("t0_uc", &event.t0_uc);
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
  timeCalibrationData->SetBranchAddress("isGood", &event.isGood);
  timeCalibrationData->SetBranchAddress("getADCcount", &event.getADCcount);

  m_evts.clear();

  // Build set of channels we need for 2D fit (top 1000 from each)
  std::set<KLMChannelNumber> neededChannels;
  int maxChannels = 1000;

  for (size_t i = 0; i < channelsBKLM.size() && i < static_cast<size_t>(maxChannels); ++i) {
    neededChannels.insert(channelsBKLM[i].first);
  }
  for (size_t i = 0; i < channelsEKLM.size() && i < static_cast<size_t>(maxChannels); ++i) {
    neededChannels.insert(channelsEKLM[i].first);
  }

  int n = timeCalibrationData->GetEntries();
  int loadedEvents = 0;

  for (int i = 0; i < n; ++i) {
    timeCalibrationData->GetEntry(i);

    if (neededChannels.find(event.channelId) != neededChannels.end()) {
      m_evts[event.channelId].push_back(event);
      loadedEvents++;
    }
  }

  B2INFO("2D fit data loaded." << LogVar("Events", loadedEvents) << LogVar("Channels", m_evts.size()));
}

void KLMTimeAlgorithm::readCalibrationDataBatch(std::function<bool(const KLMChannelIndex&)> channelFilter)
{
  B2INFO("Loading calibration data batch...");
  Event event;
  std::shared_ptr<TTree> timeCalibrationData;
  timeCalibrationData = getObjectPtr<TTree>("time_calibration_data");
  timeCalibrationData->SetBranchAddress("Run", &event.Run);
  timeCalibrationData->SetBranchAddress("Event", &event.Events);
  timeCalibrationData->SetBranchAddress("nTrack", &event.nTrack);
  timeCalibrationData->SetBranchAddress("Track_Charge", &event.Track_Charge);

  timeCalibrationData->SetBranchAddress("t0", &event.t0);
  timeCalibrationData->SetBranchAddress("t0_uc", &event.t0_uc);
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
  timeCalibrationData->SetBranchAddress("isGood", &event.isGood);
  timeCalibrationData->SetBranchAddress("getADCcount", &event.getADCcount);

  m_evts.clear();

  int n = timeCalibrationData->GetEntries();
  int loadedEvents = 0;

  for (int i = 0; i < n; ++i) {
    timeCalibrationData->GetEntry(i);

    // Convert channel number to KLMChannelIndex using channelNumberToElementNumbers
    int subdetector, section, sector, layer, plane, strip;
    m_ElementNumbers->channelNumberToElementNumbers(
      event.channelId, &subdetector, &section, &sector, &layer, &plane, &strip);
    KLMChannelIndex klmChannel(subdetector, section, sector, layer, plane, strip);

    if (channelFilter(klmChannel)) {
      m_evts[event.channelId].push_back(event);
      loadedEvents++;
    }
  }

  B2INFO("Batch loaded." << LogVar("Events", loadedEvents) << LogVar("Channels", m_evts.size()));
}

void KLMTimeAlgorithm::createHistograms()
{
  if (m_mc) {
    m_LowerTimeBoundaryRPC = -10.0;
    m_UpperTimeBoundaryRPC = 10.0;
    m_LowerTimeBoundaryScintillatorsBKLM = 20.0;
    m_UpperTimeBoundaryScintillatorsBKLM = 70.0;
    m_LowerTimeBoundaryScintillatorsEKLM = 20.0;
    m_UpperTimeBoundaryScintillatorsEKLM = 70.0;
  } else {
    m_LowerTimeBoundaryRPC = -800.0;
    m_UpperTimeBoundaryRPC = -600.0;
    m_LowerTimeBoundaryScintillatorsBKLM = -4800.0;
    m_UpperTimeBoundaryScintillatorsBKLM = -4400.0;
    m_LowerTimeBoundaryScintillatorsEKLM = -4950.0;
    m_UpperTimeBoundaryScintillatorsEKLM = -4650.0;
  }

  // Create directory structure for per-channel histograms
  // This must be done here (not in setupDatabase) because m_outFile is created just before this function is called
  if (m_outFile && m_saveChannelHists) {
    TDirectory* dir_channels = m_outFile->mkdir("channels", "Per-channel histograms", true);

    // BKLM directories
    TDirectory* dir_bklm = dir_channels->mkdir("BKLM", "", true);
    TString sectionName[2] = {"Backward", "Forward"};
    TString planeName[2] = {"Z", "Phi"};

    for (int iF = 0; iF < 2; ++iF) {
      TDirectory* dir_section = dir_bklm->mkdir(sectionName[iF].Data(), "", true);
      for (int iS = 0; iS < 8; ++iS) {
        TDirectory* dir_sector = dir_section->mkdir(Form("Sector_%d", iS + 1), "", true);
        for (int iL = 0; iL < 15; ++iL) {
          TDirectory* dir_layer = dir_sector->mkdir(Form("Layer_%d", iL + 1), "", true);
          for (int iP = 0; iP < 2; ++iP) {
            m_channelHistDir_BKLM[iF][iS][iL][iP] = dir_layer->mkdir(Form("Plane_%s", planeName[iP].Data()), "", true);
          }
        }
      }
    }

    // EKLM directories
    TDirectory* dir_eklm = dir_channels->mkdir("EKLM", "", true);
    for (int iF = 0; iF < 2; ++iF) {
      TDirectory* dir_section = dir_eklm->mkdir(sectionName[iF].Data(), "", true);
      for (int iS = 0; iS < 4; ++iS) {
        TDirectory* dir_sector = dir_section->mkdir(Form("Sector_%d", iS + 1), "", true);
        int maxLayer = 12 + 2 * iF;  // 12 for backward, 14 for forward
        for (int iL = 0; iL < maxLayer; ++iL) {
          TDirectory* dir_layer = dir_sector->mkdir(Form("Layer_%d", iL + 1), "", true);
          for (int iP = 0; iP < 2; ++iP) {
            m_channelHistDir_EKLM[iF][iS][iL][iP] = dir_layer->mkdir(Form("Plane_%d", iP + 1), "", true);
          }
        }
      }
    }

    m_outFile->cd();  // Return to root directory
    B2INFO("Created directory structure for per-channel histograms.");
  }

  int nBin = 80;
  int nBin_scint = 80;

  TString iFstring[2] = {"Backward", "Forward"};
  TString iPstring[2] = {"ZReadout", "PhiReadout"};
  TString hn, ht;

  h_diff = new TH1F("h_diff", "Position difference between bklmHit2d and extHit;position difference", 100, 0, 10);
  h_calibrated = new TH1I("h_calibrated_summary", "h_calibrated_summary;calibrated or not", 3, 0, 3);
  hc_calibrated = new TH1I("hc_calibrated_summary", "hc_calibrated_summary;calibrated or not", 3, 0, 3);

  gre_time_channel_scint = new TGraphErrors();
  gre_time_channel_rpc = new TGraphErrors();
  gre_time_channel_scint_end = new TGraphErrors();

  gr_timeShift_channel_scint = new TGraph();
  gr_timeShift_channel_rpc = new TGraph();
  gr_timeShift_channel_scint_end = new TGraph();

  gre_ctime_channel_scint = new TGraphErrors();
  gre_ctime_channel_rpc = new TGraphErrors();
  gre_ctime_channel_scint_end = new TGraphErrors();

  gr_timeRes_channel_scint = new TGraph();
  gr_timeRes_channel_rpc = new TGraph();
  gr_timeRes_channel_scint_end = new TGraph();

  double maximalPhiStripLengthBKLM =
    m_BKLMGeometry->getMaximalPhiStripLength();
  double maximalZStripLengthBKLM =
    m_BKLMGeometry->getMaximalZStripLength();
  double maximalStripLengthEKLM =
    m_EKLMGeometry->getMaximalStripLength() / CLHEP::cm * Unit::cm;

  m_ProfileRpcPhi = new TProfile("hprf_rpc_phi_effC",
                                 "Time over propagation length for RPCs (Phi_Readout); propagation distance[cm]; T_rec-T_0-T_fly-'T_calibration'[ns]", 50, 0.0,
                                 400.0);
  m_ProfileRpcZ = new TProfile("hprf_rpc_z_effC",
                               "Time over propagation length for RPCs (Z_Readout); propagation distance[cm]; T_rec-T_0-T_fly-'T_calibration'[ns]", 50, 0.0,
                               400.0);
  m_ProfileBKLMScintillatorPhi = new TProfile("hprf_scint_phi_effC",
                                              "Time over propagation length for scintillators (Phi_Readout); propagation distance[cm]; T_rec-T_0-T_fly-'T_calibration'[ns]",
                                              50, 0.0, maximalPhiStripLengthBKLM);
  m_ProfileBKLMScintillatorZ = new TProfile("hprf_scint_z_effC",
                                            "Time over propagation length for scintillators (Z_Readout); propagation distance[cm]; T_rec-T_0-T_fly-'T_calibration'[ns]",
                                            50, 0.0, maximalZStripLengthBKLM);
  m_ProfileEKLMScintillatorPlane1 = new TProfile("hprf_scint_plane1_effC_end",
                                                 "Time over propagation length for scintillators (plane1, Endcap); propagation distance[cm]; T_rec-T_0-T_fly-'T_calibration'[ns]",
                                                 50, 0.0, maximalStripLengthEKLM);
  m_ProfileEKLMScintillatorPlane2 = new TProfile("hprf_scint_plane2_effC_end",
                                                 "Time over propagation length for scintillators (plane2, Endcap); propagation distance[cm]; T_rec-T_0-T_fly-'T_calibration'[ns]",
                                                 50, 0.0, maximalStripLengthEKLM);

  m_Profile2RpcPhi = new TProfile("hprf2_rpc_phi_effC",
                                  "Time over propagation length for RPCs (Phi_Readout); propagation distance[cm]; T_rec-T_0-T_fly-'T_calibration'[ns]", 50, 0.0,
                                  400.0);
  m_Profile2RpcZ = new TProfile("hprf2_rpc_z_effC",
                                "Time over propagation length for RPCs (Z_Readout); propagation distance[cm]; T_rec-T_0-T_fly-'T_calibration'[ns]", 50, 0.0,
                                400.0);
  m_Profile2BKLMScintillatorPhi = new TProfile("hprf2_scint_phi_effC",
                                               "Time over propagation length for scintillators (Phi_Readout); propagation distance[cm]; T_rec-T_0-T_fly-'T_calibration'[ns]",
                                               50, 0.0, maximalPhiStripLengthBKLM);
  m_Profile2BKLMScintillatorZ = new TProfile("hprf2_scint_z_effC",
                                             "Time over propagation length for scintillators (Z_Readout); propagation distance[cm]; T_rec-T_0-T_fly-'T_calibration'[ns]",
                                             50, 0.0, maximalZStripLengthBKLM);
  m_Profile2EKLMScintillatorPlane1 = new TProfile("hprf2_scint_plane1_effC_end",
                                                  "Time over propagation length for scintillators (plane1, Endcap); propagation distance[cm]; T_rec-T_0-T_fly-'T_calibration'[ns]",
                                                  50, 0.0, maximalStripLengthEKLM);
  m_Profile2EKLMScintillatorPlane2 = new TProfile("hprf2_scint_plane2_effC_end",
                                                  "Time over propagation length for scintillators (plane2, Endcap); propagation distance[cm]; T_rec-T_0-T_fly-'T_calibration'[ns]",
                                                  50, 0.0, maximalStripLengthEKLM);

  h_time_rpc_tc = new TH1F("h_time_rpc_tc", "time distribution for RPC", nBin, m_LowerTimeBoundaryRPC, m_UpperTimeBoundaryRPC);
  h_time_scint_tc = new TH1F("h_time_scint_tc", "time distribution for Scintillator", nBin_scint,
                             m_LowerTimeBoundaryScintillatorsBKLM, m_UpperTimeBoundaryScintillatorsBKLM);
  h_time_scint_tc_end = new TH1F("h_time_scint_tc_end", "time distribution for Scintillator (Endcap)", nBin_scint,
                                 m_LowerTimeBoundaryScintillatorsEKLM,
                                 m_UpperTimeBoundaryScintillatorsEKLM);

  /** Hist declaration Global time distribution **/
  h_time_rpc = new TH1F("h_time_rpc", "time distribution for RPC; T_rec-T_0-T_fly-T_propagation[ns]", nBin, m_LowerTimeBoundaryRPC,
                        m_UpperTimeBoundaryRPC);
  h_time_scint = new TH1F("h_time_scint", "time distribution for Scintillator; T_rec-T_0-T_fly-T_propagation[ns]", nBin_scint,
                          m_LowerTimeBoundaryScintillatorsBKLM, m_UpperTimeBoundaryScintillatorsBKLM);
  h_time_scint_end = new TH1F("h_time_scint_end", "time distribution for Scintillator (Endcap); T_rec-T_0-T_fly-T_propagation[ns]",
                              nBin_scint, m_LowerTimeBoundaryScintillatorsEKLM, m_UpperTimeBoundaryScintillatorsEKLM);

  hc_time_rpc = new TH1F("hc_time_rpc", "Calibrated time distribution for RPC; T_rec-T_0-T_fly-T_propagation-T_calibration[ns]",
                         nBin, m_LowerTimeBoundaryCalibratedRPC, m_UpperTimeBoundaryCalibratedRPC);
  hc_time_scint = new TH1F("hc_time_scint",
                           "Calibrated time distribution for Scintillator; T_rec-T_0-T_fly-T_propagation-T_calibration[ns]", nBin_scint,
                           m_LowerTimeBoundaryCalibratedScintillatorsBKLM,
                           m_UpperTimeBoundaryCalibratedScintillatorsBKLM);
  hc_time_scint_end = new TH1F("hc_time_scint_end",
                               "Calibrated time distribution for Scintillator (Endcap); T_rec-T_0-T_fly-T_propagation-T_calibration[ns]", nBin_scint,
                               m_LowerTimeBoundaryCalibratedScintillatorsEKLM, m_UpperTimeBoundaryCalibratedScintillatorsEKLM);

  int nBin_t0 = 100;
  h_eventT0_rpc = new TH1F("h_eventT0_rpc",
                           "RPC: Event T0; T_{0}[ns]", nBin_t0, -100.0, 100.0);
  h_eventT0_scint = new TH1F("h_eventT0_scint",
                             "BKLM scintillator: Event T0; T_{0}[ns]", nBin_t0, -100.0, 100.0);
  h_eventT0_scint_end = new TH1F("h_eventT0_scint_end",
                                 "EKLM scintillator: Event T0; T_{0}[ns]", nBin_t0, -100.0, 100.0);

  // Corrected EventT0 distributions between 2 muon tracks in an event.
  hc_eventT0_rpc = new TH1F("hc_eventT0_rpc",
                            "RPC: corrected Event T0; T_{0}^{+} - T_{0}^{-} [ns]", nBin_t0, -40.0, 40.0);
  hc_eventT0_scint = new TH1F("hc_eventT0_scint",
                              "BKLM scintillator: corrected Event T0; T_{0}^{+} - T_{0}^{-} [ns]", nBin_t0, -40.0, 40.0);
  hc_eventT0_scint_end = new TH1F("hc_eventT0_scint_end",
                                  "EKLM scintillator: corrected Event T0; T_{0}^{+} - T_{0}^{-} [ns]", nBin_t0, -20.0, 20.0);

  // ==== NEW DIAGNOSTIC PLOTS ====

  // Hit multiplicity distributions
  h_nHits_plus_rpc = new TH1F("h_nHits_plus_rpc",
                              "RPC: #mu^{+} hit multiplicity;N_{hits};Events", 100, 0, 100);
  h_nHits_minus_rpc = new TH1F("h_nHits_minus_rpc",
                               "RPC: #mu^{-} hit multiplicity;N_{hits};Events", 100, 0, 100);
  h_nHits_plus_scint = new TH1F("h_nHits_plus_scint",
                                "BKLM Scint: #mu^{+} hit multiplicity;N_{hits};Events", 30, 0, 30);
  h_nHits_minus_scint = new TH1F("h_nHits_minus_scint",
                                 "BKLM Scint: #mu^{-} hit multiplicity;N_{hits};Events", 30, 0, 30);
  h_nHits_plus_scint_end = new TH1F("h_nHits_plus_scint_end",
                                    "EKLM Scint: #mu^{+} hit multiplicity;N_{hits};Events", 30, 0, 30);
  h_nHits_minus_scint_end = new TH1F("h_nHits_minus_scint_end",
                                     "EKLM Scint: #mu^{-} hit multiplicity;N_{hits};Events", 30, 0, 30);

  // ΔT0 vs variance weight v = 1/N+ + 1/N-
  h2_deltaT0_vs_v_rpc = new TH2F("h2_deltaT0_vs_v_rpc",
                                 "RPC: #DeltaT_{0} vs variance weight;v = 1/N^{+} + 1/N^{-};#DeltaT_{0} [ns]",
                                 50, 0, 2.0, 100, -40, 40);
  h2_deltaT0_vs_v_scint = new TH2F("h2_deltaT0_vs_v_scint",
                                   "BKLM Scint: #DeltaT_{0} vs variance weight;v = 1/N^{+} + 1/N^{-};#DeltaT_{0} [ns]",
                                   50, 0, 1.0, 100, -40, 40);
  h2_deltaT0_vs_v_scint_end = new TH2F("h2_deltaT0_vs_v_scint_end",
                                       "EKLM Scint: #DeltaT_{0} vs variance weight;v = 1/N^{+} + 1/N^{-};#DeltaT_{0} [ns]",
                                       50, 0, 1.0, 100, -20, 20);

  // Profile: RMS(ΔT0) vs v (should scale as √v if model is correct)
  prof_deltaT0_rms_vs_v_rpc = new TProfile("prof_deltaT0_rms_vs_v_rpc",
                                           "RPC: RMS(#DeltaT_{0}) vs v;v = 1/N^{+} + 1/N^{-};RMS(#DeltaT_{0}) [ns]",
                                           20, 0, 2.0, "s");  // "s" option for RMS
  prof_deltaT0_rms_vs_v_scint = new TProfile("prof_deltaT0_rms_vs_v_scint",
                                             "BKLM Scint: RMS(#DeltaT_{0}) vs v;v = 1/N^{+} + 1/N^{-};RMS(#DeltaT_{0}) [ns]",
                                             20, 0, 1.0, "s");
  prof_deltaT0_rms_vs_v_scint_end = new TProfile("prof_deltaT0_rms_vs_v_scint_end",
                                                 "EKLM Scint: RMS(#DeltaT_{0}) vs v;v = 1/N^{+} + 1/N^{-};RMS(#DeltaT_{0}) [ns]",
                                                 20, 0, 1.0, "s");

  // ΔT0 vs total hits
  h2_deltaT0_vs_nhits_rpc = new TH2F("h2_deltaT0_vs_nhits_rpc",
                                     "RPC: #DeltaT_{0} vs total hits;N^{+} + N^{-};#DeltaT_{0} [ns]",
                                     50, 0, 200, 100, -40, 40);
  h2_deltaT0_vs_nhits_scint = new TH2F("h2_deltaT0_vs_nhits_scint",
                                       "BKLM Scint: #DeltaT_{0} vs total hits;N^{+} + N^{-};#DeltaT_{0} [ns]",
                                       40, 0, 40, 100, -40, 40);
  h2_deltaT0_vs_nhits_scint_end = new TH2F("h2_deltaT0_vs_nhits_scint_end",
                                           "EKLM Scint: #DeltaT_{0} vs total hits;N^{+} + N^{-};#DeltaT_{0} [ns]",
                                           40, 0, 40, 100, -20, 20);

  // ΔT0 separated by hit multiplicity bins
  hc_eventT0_rpc_lowN = new TH1F("hc_eventT0_rpc_lowN",
                                 "RPC: #DeltaT_{0} (N^{+}+N^{-} < 10);#DeltaT_{0} [ns]", nBin_t0, -40.0, 40.0);
  hc_eventT0_rpc_midN = new TH1F("hc_eventT0_rpc_midN",
                                 "RPC: #DeltaT_{0} (10 #leq N^{+}+N^{-} < 30);#DeltaT_{0} [ns]", nBin_t0, -40.0, 40.0);
  hc_eventT0_rpc_highN = new TH1F("hc_eventT0_rpc_highN",
                                  "RPC: #DeltaT_{0} (N^{+}+N^{-} #geq 30);#DeltaT_{0} [ns]", nBin_t0, -40.0, 40.0);

  hc_eventT0_scint_lowN = new TH1F("hc_eventT0_scint_lowN",
                                   "BKLM Scint: #DeltaT_{0} (N^{+}+N^{-} < 5);#DeltaT_{0} [ns]", nBin_t0, -40.0, 40.0);
  hc_eventT0_scint_midN = new TH1F("hc_eventT0_scint_midN",
                                   "BKLM Scint: #DeltaT_{0} (5 #leq N^{+}+N^{-} < 15);#DeltaT_{0} [ns]", nBin_t0, -40.0, 40.0);
  hc_eventT0_scint_highN = new TH1F("hc_eventT0_scint_highN",
                                    "BKLM Scint: #DeltaT_{0} (N^{+}+N^{-} #geq 15);#DeltaT_{0} [ns]", nBin_t0, -40.0, 40.0);

  hc_eventT0_scint_end_lowN = new TH1F("hc_eventT0_scint_end_lowN",
                                       "EKLM Scint: #DeltaT_{0} (N^{+}+N^{-} < 5);#DeltaT_{0} [ns]", nBin_t0, -20.0, 20.0);
  hc_eventT0_scint_end_midN = new TH1F("hc_eventT0_scint_end_midN",
                                       "EKLM Scint: #DeltaT_{0} (5 #leq N^{+}+N^{-} < 15);#DeltaT_{0} [ns]", nBin_t0, -20.0, 20.0);
  hc_eventT0_scint_end_highN = new TH1F("hc_eventT0_scint_end_highN",
                                        "EKLM Scint: #DeltaT_{0} (N^{+}+N^{-} #geq 15);#DeltaT_{0} [ns]", nBin_t0, -20.0, 20.0);

  if (!m_saveAllPlots) {
    B2INFO("Skipping debug histogram allocation (m_saveAllPlots = false)");
    return;  // Skip all debugging histogram allocation
  }

  for (int iF = 0; iF < 2; ++iF) {
    hn = Form("h_timeF%d_rpc", iF);
    ht = Form("Time distribution for RPC of %s; T_rec-T_0-T_fly-T_propagation[ns]", iFstring[iF].Data());
    h_timeF_rpc[iF] = new TH1F(hn.Data(), ht.Data(), nBin, m_LowerTimeBoundaryRPC, m_UpperTimeBoundaryRPC);
    hn = Form("h_timeF%d_scint", iF);
    ht = Form("Time distribution for Scintillator of %s; T_rec-T_0-T_fly-T_propagation[ns]", iFstring[iF].Data());
    h_timeF_scint[iF] = new TH1F(hn.Data(), ht.Data(), nBin_scint, m_LowerTimeBoundaryScintillatorsBKLM,
                                 m_UpperTimeBoundaryScintillatorsBKLM);
    hn = Form("h_timeF%d_scint_end", iF);
    ht = Form("Time distribution for Scintillator of %s (Endcap); T_rec-T_0-T_fly-T_propagation[ns]", iFstring[iF].Data());
    h_timeF_scint_end[iF] = new TH1F(hn.Data(), ht.Data(), nBin_scint, m_LowerTimeBoundaryScintillatorsEKLM,
                                     m_UpperTimeBoundaryScintillatorsEKLM);

    hn = Form("h2_timeF%d_rpc", iF);
    ht = Form("Time distribution for RPC of %s; Sector Index; T_rec-T_0-T_fly-T_propagation[ns]", iFstring[iF].Data());
    h2_timeF_rpc[iF] = new TH2F(hn.Data(), ht.Data(), 8, 0, 8, nBin, m_LowerTimeBoundaryRPC, m_UpperTimeBoundaryRPC);
    hn = Form("h2_timeF%d_scint", iF);
    ht = Form("Time distribution for Scintillator of %s; Sector Index; T_rec-T_0-T_fly-T_propagation[ns]", iFstring[iF].Data());
    h2_timeF_scint[iF] = new TH2F(hn.Data(), ht.Data(), 8, 0, 8, nBin_scint, m_LowerTimeBoundaryScintillatorsBKLM,
                                  m_UpperTimeBoundaryScintillatorsBKLM);
    hn = Form("h2_timeF%d_scint_end", iF);
    ht = Form("Time distribution for Scintillator of %s (Endcap); Sector Index; T_rec-T_0-T_fly-T_propagation[ns]",
              iFstring[iF].Data());
    h2_timeF_scint_end[iF] = new TH2F(hn.Data(), ht.Data(), 4, 0, 4, nBin_scint, m_LowerTimeBoundaryScintillatorsEKLM,
                                      m_UpperTimeBoundaryScintillatorsEKLM);

    hn = Form("hc_timeF%d_rpc", iF);
    ht = Form("Calibrated time distribution for RPC of %s; T_rec-T_0-T_fly-T_propagation-T_calibration[ns]", iFstring[iF].Data());
    hc_timeF_rpc[iF] = new TH1F(hn.Data(), ht.Data(), nBin, m_LowerTimeBoundaryCalibratedRPC, m_UpperTimeBoundaryCalibratedRPC);
    hn = Form("hc_timeF%d_scint", iF);
    ht = Form("Calibrated time distribution for Scintillator of %s; T_rec-T_0-T_fly-T_propagation-T_calibration[ns]",
              iFstring[iF].Data());
    hc_timeF_scint[iF] = new TH1F(hn.Data(), ht.Data(), nBin_scint, m_LowerTimeBoundaryCalibratedScintillatorsBKLM,
                                  m_UpperTimeBoundaryCalibratedScintillatorsBKLM);
    hn = Form("hc_timeF%d_scint_end", iF);
    ht = Form("Calibrated time distribution for Scintillator of %s (Endcap); T_rec-T_0-T_fly-T_propagation-T_calibration[ns]",
              iFstring[iF].Data());
    hc_timeF_scint_end[iF] = new TH1F(hn.Data(), ht.Data(), nBin_scint, m_LowerTimeBoundaryCalibratedScintillatorsEKLM,
                                      m_UpperTimeBoundaryCalibratedScintillatorsEKLM);

    hn = Form("h2c_timeF%d_rpc", iF);
    ht = Form("Calibrated time distribution for RPC of %s; Sector Index; T_rec-T_0-T_fly-T_propagation-T_calibration[ns]",
              iFstring[iF].Data());
    h2c_timeF_rpc[iF] = new TH2F(hn.Data(), ht.Data(), 8, 0, 8, nBin, m_LowerTimeBoundaryCalibratedRPC,
                                 m_UpperTimeBoundaryCalibratedRPC);
    hn = Form("h2c_timeF%d_scint", iF);
    ht = Form("Calibrated time distribution for Scintillator of %s; Sector Index; T_rec-T_0-T_fly-T_propagation-T_calibration[ns]",
              iFstring[iF].Data());
    h2c_timeF_scint[iF] = new TH2F(hn.Data(), ht.Data(), 8, 0, 8, nBin_scint, m_LowerTimeBoundaryCalibratedScintillatorsBKLM,
                                   m_UpperTimeBoundaryCalibratedScintillatorsBKLM);
    hn = Form("h2c_timeF%d_scint_end", iF);
    ht = Form("Calibrated time distribution for Scintillator of %s (Endcap) ; Sector Index; T_rec-T_0-T_fly-T_propagation-T_calibration[ns]",
              iFstring[iF].Data());
    h2c_timeF_scint_end[iF] = new TH2F(hn.Data(), ht.Data(), 4, 0, 4, nBin_scint, m_LowerTimeBoundaryCalibratedScintillatorsEKLM,
                                       m_UpperTimeBoundaryCalibratedScintillatorsEKLM);

    for (int iS = 0; iS < 8; ++iS) {
      // Barrel parts
      hn = Form("h_timeF%d_S%d_scint", iF, iS);
      ht = Form("Time distribution for Scintillator of Sector%d, %s; T_rec-T_0-T_fly-T_propagation[ns]", iS, iFstring[iF].Data());
      h_timeFS_scint[iF][iS] = new TH1F(hn.Data(), ht.Data(), nBin_scint, m_LowerTimeBoundaryScintillatorsBKLM,
                                        m_UpperTimeBoundaryScintillatorsBKLM);
      hn = Form("h_timeF%d_S%d_rpc", iF, iS);
      ht = Form("Time distribution for RPC of Sector%d, %s; T_rec-T_0-T_fly-T_propagation[ns]", iS, iFstring[iF].Data());
      h_timeFS_rpc[iF][iS] = new TH1F(hn.Data(), ht.Data(), nBin, m_LowerTimeBoundaryRPC, m_UpperTimeBoundaryRPC);
      hn = Form("h2_timeF%d_S%d", iF, iS);
      ht = Form("Time distribution of Sector%d, %s; Layer Index; T_rec-T_0-T_fly-T_propagation[ns]", iS, iFstring[iF].Data());
      h2_timeFS[iF][iS] = new TH2F(hn.Data(), ht.Data(), 15, 0, 15, nBin_scint, m_LowerTimeBoundaryRPC,
                                   m_UpperTimeBoundaryScintillatorsBKLM);

      hn = Form("hc_timeF%d_S%d_scint", iF, iS);
      ht = Form("Calibrated time distribution for Scintillator of Sector%d, %s; T_rec-T_0-T_fly-T_propagation-T_calibration[ns]", iS,
                iFstring[iF].Data());
      hc_timeFS_scint[iF][iS] = new TH1F(hn.Data(), ht.Data(), nBin_scint, m_LowerTimeBoundaryCalibratedScintillatorsBKLM,
                                         m_UpperTimeBoundaryCalibratedScintillatorsBKLM);
      hn = Form("hc_timeF%d_S%d_rpc", iF, iS);
      ht = Form("Calibrated time distribution for RPC of Sector%d, %s; T_rec-T_0-T_fly-T_propagation-T_calibration[ns]", iS,
                iFstring[iF].Data());
      hc_timeFS_rpc[iF][iS] = new TH1F(hn.Data(), ht.Data(), nBin_scint, m_LowerTimeBoundaryCalibratedRPC,
                                       m_UpperTimeBoundaryCalibratedRPC);
      hn = Form("h2c_timeF%d_S%d", iF, iS);
      ht = Form("Calibrated time distribution of Sector%d, %s; Layer Index; T_rec-T_0-T_fly-T_propagation-T_calibration[ns]", iS,
                iFstring[iF].Data());
      h2c_timeFS[iF][iS] = new TH2F(hn.Data(), ht.Data(), 15, 0, 15, nBin_scint, m_LowerTimeBoundaryCalibratedRPC,
                                    m_UpperTimeBoundaryCalibratedScintillatorsBKLM);

      // Inner 2 layers --> Scintillators
      for (int iL = 0; iL < 2; ++iL) {
        hn = Form("h_timeF%d_S%d_L%d", iF, iS, iL);
        ht = Form("Time distribution for Scintillator of Layer%d, Sector%d, %s; T_rec-T_0-T_fly-T_propagation[ns]", iL, iS,
                  iFstring[iF].Data());
        h_timeFSL[iF][iS][iL] = new TH1F(hn.Data(), ht.Data(), nBin_scint, m_LowerTimeBoundaryScintillatorsBKLM,
                                         m_UpperTimeBoundaryScintillatorsBKLM);
        hn = Form("hc_timeF%d_S%d_L%d", iF, iS, iL);
        ht = Form("Calibrated time distribution for Scintillator of Layer%d, Sector%d, %s; T_rec-T_0-T_fly-T_propagation-T_calibration[ns]",
                  iL, iS, iFstring[iF].Data());
        hc_timeFSL[iF][iS][iL] = new TH1F(hn.Data(), ht.Data(), nBin_scint, m_LowerTimeBoundaryCalibratedScintillatorsBKLM,
                                          m_UpperTimeBoundaryCalibratedScintillatorsBKLM);

        for (int iP = 0; iP < 2; ++iP) {
          hn = Form("h_timeF%d_S%d_L%d_P%d", iF, iS, iL, iP);
          ht = Form("Time distribution for Scintillator of %s, Layer%d, Sector%d, %s; T_rec-T_0-T_fly-T_propagation[ns]",
                    iPstring[iP].Data(), iL, iS, iFstring[iF].Data());
          h_timeFSLP[iF][iS][iL][iP] = new TH1F(hn.Data(), ht.Data(), nBin_scint, m_LowerTimeBoundaryScintillatorsBKLM,
                                                m_UpperTimeBoundaryScintillatorsBKLM);
          hn = Form("h2_timeF%d_S%d_L%d_P%d", iF, iS, iL, iP);
          ht = Form("Time distribution for Scintillator of %s, Layer%d, Sector%d, %s; Channel Index; T_rec-T_0-T_fly-T_propagation[ns]",
                    iPstring[iP].Data(), iL, iS, iFstring[iF].Data());
          h2_timeFSLP[iF][iS][iL][iP] = new TH2F(hn.Data(), ht.Data(), 54, 0, 54, nBin_scint, m_LowerTimeBoundaryScintillatorsBKLM,
                                                 m_UpperTimeBoundaryScintillatorsBKLM);

          hn = Form("hc_timeF%d_S%d_L%d_P%d", iF, iS, iL, iP);
          ht = Form("Calibrated time distribution for Scintillator of %s, Layer%d, Sector%d, %s; T_rec-T_0-T_fly-T_propagation-T_calibration[ns]",
                    iPstring[iP].Data(), iL, iS, iFstring[iF].Data());
          hc_timeFSLP[iF][iS][iL][iP] = new TH1F(hn.Data(), ht.Data(), nBin_scint, m_LowerTimeBoundaryCalibratedScintillatorsBKLM,
                                                 m_UpperTimeBoundaryCalibratedScintillatorsBKLM);
          hn = Form("h2c_timeF%d_S%d_L%d_P%d", iF, iS, iL, iP);
          ht = Form("Calibrated time distribution for Scintillator of %s, Layer%d, Sector%d, %s; Channel Index; T_rec-T_0-T_fly-T_propagation-T_calibration[ns]",
                    iPstring[iP].Data(), iL, iS, iFstring[iF].Data());
          h2c_timeFSLP[iF][iS][iL][iP] = new TH2F(hn.Data(), ht.Data(), 54, 0, 54, nBin_scint, m_LowerTimeBoundaryCalibratedScintillatorsBKLM,
                                                  m_UpperTimeBoundaryCalibratedScintillatorsBKLM);
        }
      }

      for (int iL = 2; iL < 15; ++iL) {
        hn = Form("h_timeF%d_S%d_L%d", iF, iS, iL);
        ht = Form("time distribution for RPC of Layer%d, Sector%d, %s; T_rec-T_0-T_fly-T_propagation[ns]", iL, iS, iFstring[iF].Data());
        h_timeFSL[iF][iS][iL] = new TH1F(hn.Data(), ht.Data(), nBin, m_LowerTimeBoundaryRPC, m_UpperTimeBoundaryRPC);

        hn = Form("hc_timeF%d_S%d_L%d", iF, iS, iL);
        ht = Form("Calibrated time distribution for RPC of Layer%d, Sector%d, %s; T_rec-T_0-T_fly-T_propagation-T_calibration[ns]", iL, iS,
                  iFstring[iF].Data());
        hc_timeFSL[iF][iS][iL] = new TH1F(hn.Data(), ht.Data(), nBin, m_LowerTimeBoundaryCalibratedRPC, m_UpperTimeBoundaryCalibratedRPC);

        for (int iP = 0; iP < 2; ++iP) {
          hn = Form("h_timeF%d_S%d_L%d_P%d", iF, iS, iL, iP);
          ht = Form("time distribution for RPC of %s, Layer%d, Sector%d, %s; T_rec-T_0-T_fly-T_propagation[ns]", iPstring[iP].Data(), iL, iS,
                    iFstring[iF].Data());
          h_timeFSLP[iF][iS][iL][iP] = new TH1F(hn.Data(), ht.Data(), nBin, m_LowerTimeBoundaryRPC, m_UpperTimeBoundaryRPC);

          hn = Form("h2_timeF%d_S%d_L%d_P%d", iF, iS, iL, iP);
          ht = Form("time distribution for RPC of %s, Layer%d, Sector%d, %s; Channel Index; T_rec-T_0-T_fly-T_propagation[ns]",
                    iPstring[iP].Data(), iL, iS, iFstring[iF].Data());
          h2_timeFSLP[iF][iS][iL][iP] = new TH2F(hn.Data(), ht.Data(), 48, 0, 48, nBin, m_LowerTimeBoundaryRPC, m_UpperTimeBoundaryRPC);

          hn = Form("hc_timeF%d_S%d_L%d_P%d", iF, iS, iL, iP);
          ht = Form("Calibrated time distribution for RPC of %s, Layer%d, Sector%d, %s; T_rec-T_0-T_fly-T_propagation-T_calibration[ns]",
                    iPstring[iP].Data(), iL, iS, iFstring[iF].Data());
          hc_timeFSLP[iF][iS][iL][iP] = new TH1F(hn.Data(), ht.Data(), nBin, m_LowerTimeBoundaryCalibratedRPC,
                                                 m_UpperTimeBoundaryCalibratedRPC);

          hn = Form("h2c_timeF%d_S%d_L%d_P%d", iF, iS, iL, iP);
          ht = Form("Calibrated time distribution for RPC of %s, Layer%d, Sector%d, %s; Channel Index; T_rec-T_0-T_fly-T_propagation-T_calibration[ns]",
                    iPstring[iP].Data(), iL, iS, iFstring[iF].Data());
          h2c_timeFSLP[iF][iS][iL][iP] = new TH2F(hn.Data(), ht.Data(), 48, 0, 48, nBin, m_LowerTimeBoundaryCalibratedRPC,
                                                  m_UpperTimeBoundaryCalibratedRPC);
        }
      }
    }
    // Endcap part
    int maxLay = 12 + 2 * iF;
    for (int iS = 0; iS < 4; ++iS) {
      hn = Form("h_timeF%d_S%d_scint_end", iF, iS);
      ht = Form("Time distribution for Scintillator of Sector%d, %s (Endcap); T_rec-T_0-T_fly-T_propagation[ns]", iS,
                iFstring[iF].Data());
      h_timeFS_scint_end[iF][iS] = new TH1F(hn.Data(), ht.Data(), nBin_scint, m_LowerTimeBoundaryScintillatorsEKLM,
                                            m_UpperTimeBoundaryScintillatorsEKLM);
      hn = Form("h2_timeF%d_S%d_end", iF, iS);
      ht = Form("Time distribution of Sector%d, %s (Endcap); Layer Index; T_rec-T_0-T_fly-T_propagation[ns]", iS, iFstring[iF].Data());
      h2_timeFS_end[iF][iS] = new TH2F(hn.Data(), ht.Data(), maxLay, 0, maxLay, nBin_scint, m_LowerTimeBoundaryScintillatorsEKLM,
                                       m_UpperTimeBoundaryScintillatorsEKLM);
      hn = Form("hc_timeF%d_S%d_scint_end", iF, iS);
      ht = Form("Calibrated time distribution for Scintillator of Sector%d (Endcap), %s; T_rec-T_0-T_fly-T_propagation-T_calibration[ns]",
                iS, iFstring[iF].Data());
      hc_timeFS_scint_end[iF][iS] = new TH1F(hn.Data(), ht.Data(), nBin_scint, m_LowerTimeBoundaryCalibratedScintillatorsEKLM,
                                             m_UpperTimeBoundaryCalibratedScintillatorsEKLM);
      hn = Form("h2c_timeF%d_S%d_end", iF, iS);
      ht = Form("Calibrated time distribution of Sector%d, %s (Endcap); Layer Index; T_rec-T_0-T_fly-T_propagation-T_calibration[ns]",
                iS, iFstring[iF].Data());
      h2c_timeFS_end[iF][iS] = new TH2F(hn.Data(), ht.Data(), maxLay, 0, maxLay, nBin_scint,
                                        m_LowerTimeBoundaryCalibratedScintillatorsEKLM,
                                        m_UpperTimeBoundaryCalibratedScintillatorsEKLM);

      for (int iL = 0; iL < maxLay; ++iL) {
        hn = Form("h_timeF%d_S%d_L%d_end", iF, iS, iL);
        ht = Form("Time distribution for Scintillator of Layer%d, Sector%d, %s (Endcap); T_rec-T_0-T_fly-T_propagation[ns]", iL, iS,
                  iFstring[iF].Data());
        h_timeFSL_end[iF][iS][iL] = new TH1F(hn.Data(), ht.Data(), nBin_scint, m_LowerTimeBoundaryScintillatorsEKLM,
                                             m_UpperTimeBoundaryScintillatorsEKLM);
        hn = Form("hc_timeF%d_S%d_L%d_end", iF, iS, iL);
        ht = Form("Calibrated time distribution for Scintillator of Layer%d, Sector%d, %s (Endcap); T_rec-T_0-T_fly-T_propagation-T_calibration[ns]",
                  iL, iS, iFstring[iF].Data());
        hc_timeFSL_end[iF][iS][iL] = new TH1F(hn.Data(), ht.Data(), nBin_scint, m_LowerTimeBoundaryCalibratedScintillatorsEKLM,
                                              m_UpperTimeBoundaryCalibratedScintillatorsEKLM);

        for (int iP = 0; iP < 2; ++iP) {
          hn = Form("h_timeF%d_S%d_L%d_P%d_end", iF, iS, iL, iP);
          ht = Form("Time distribution for Scintillator of %s, Layer%d, Sector%d, %s (Endcap); T_rec-T_0-T_fly-T_propagation[ns]",
                    iPstring[iP].Data(), iL, iS, iFstring[iF].Data());
          h_timeFSLP_end[iF][iS][iL][iP] = new TH1F(hn.Data(), ht.Data(), nBin_scint, m_LowerTimeBoundaryScintillatorsEKLM,
                                                    m_UpperTimeBoundaryScintillatorsEKLM);

          hn = Form("h2_timeF%d_S%d_L%d_P%d_end", iF, iS, iL, iP);
          ht = Form("Time distribution for Scintillator of %s, Layer%d, Sector%d, %s (Endcap); Channel Index; T_rec-T_0-T_fly-T_propagation[ns]",
                    iPstring[iP].Data(), iL, iS, iFstring[iF].Data());
          h2_timeFSLP_end[iF][iS][iL][iP] = new TH2F(hn.Data(), ht.Data(), 75, 0, 75, nBin_scint, m_LowerTimeBoundaryScintillatorsEKLM,
                                                     m_UpperTimeBoundaryScintillatorsEKLM);

          hn = Form("hc_timeF%d_S%d_L%d_P%d_end", iF, iS, iL, iP);
          ht = Form("Calibrated time distribution for Scintillator of %s, Layer%d, Sector%d, %s (Endcap); T_rec-T_0-T_fly-T_propagation-T_calibration[ns]",
                    iPstring[iP].Data(), iL, iS, iFstring[iF].Data());
          hc_timeFSLP_end[iF][iS][iL][iP] = new TH1F(hn.Data(), ht.Data(), nBin_scint, m_LowerTimeBoundaryCalibratedScintillatorsEKLM,
                                                     m_UpperTimeBoundaryCalibratedScintillatorsEKLM);

          hn = Form("h2c_timeF%d_S%d_L%d_P%d_end", iF, iS, iL, iP);
          ht = Form("Calibrated time distribution for Scintillator of %s, Layer%d, Sector%d, %s (Endcap); Channel Index; T_rec-T_0-T_fly-T_propagation-T_calibration[ns]",
                    iPstring[iP].Data(), iL, iS, iFstring[iF].Data());
          h2c_timeFSLP_end[iF][iS][iL][iP] = new TH2F(hn.Data(), ht.Data(), 75, 0, 75, nBin_scint,
                                                      m_LowerTimeBoundaryCalibratedScintillatorsEKLM,
                                                      m_UpperTimeBoundaryCalibratedScintillatorsEKLM);
        }
      }
    }
  }

  // NOTE: Directory structure for per-channel histograms is created in createHistograms()
  // because m_outFile is not yet created when setupDatabase() is called.
}

void KLMTimeAlgorithm::fillTimeDistanceProfiles(
  TProfile* profileRpcPhi, TProfile* profileRpcZ,
  TProfile* profileBKLMScintillatorPhi, TProfile* profileBKLMScintillatorZ,
  TProfile* profileEKLMScintillatorPlane1,
  TProfile* profileEKLMScintillatorPlane2, bool fill2dHistograms)
{
  B2INFO("Filling time-distance profiles" << (fill2dHistograms ? " with 2D histograms" : "") << " (batched processing)...");

  TString iFstring[2] = {"Backward", "Forward"};
  TString iPstring[2] = {"ZReadout", "PhiReadout"};

  // Define the 6 batches (same as in calibrate())
  auto isRPCBackward = [](const KLMChannelIndex & ch) {
    return ch.getSubdetector() == KLMElementNumbers::c_BKLM &&
           ch.getLayer() >= BKLMElementNumbers::c_FirstRPCLayer &&
           ch.getSection() == BKLMElementNumbers::c_BackwardSection;
  };

  auto isRPCForward = [](const KLMChannelIndex & ch) {
    return ch.getSubdetector() == KLMElementNumbers::c_BKLM &&
           ch.getLayer() >= BKLMElementNumbers::c_FirstRPCLayer &&
           ch.getSection() == BKLMElementNumbers::c_ForwardSection;
  };

  auto isBKLMScintillatorBackward = [](const KLMChannelIndex & ch) {
    return ch.getSubdetector() == KLMElementNumbers::c_BKLM &&
           ch.getLayer() < BKLMElementNumbers::c_FirstRPCLayer &&
           ch.getSection() == BKLMElementNumbers::c_BackwardSection;
  };

  auto isBKLMScintillatorForward = [](const KLMChannelIndex & ch) {
    return ch.getSubdetector() == KLMElementNumbers::c_BKLM &&
           ch.getLayer() < BKLMElementNumbers::c_FirstRPCLayer &&
           ch.getSection() == BKLMElementNumbers::c_ForwardSection;
  };

  auto isEKLMScintillatorBackward = [](const KLMChannelIndex & ch) {
    return ch.getSubdetector() == KLMElementNumbers::c_EKLM &&
           ch.getSection() == EKLMElementNumbers::c_BackwardSection;
  };

  auto isEKLMScintillatorForward = [](const KLMChannelIndex & ch) {
    return ch.getSubdetector() == KLMElementNumbers::c_EKLM &&
           ch.getSection() == EKLMElementNumbers::c_ForwardSection;
  };

  std::vector<std::pair<std::string, std::function<bool(const KLMChannelIndex&)>>> batches = {
    {"RPC Backward", isRPCBackward},
    {"RPC Forward", isRPCForward},
    {"BKLM Scintillator Backward", isBKLMScintillatorBackward},
    {"BKLM Scintillator Forward", isBKLMScintillatorForward},
    {"EKLM Scintillator Backward", isEKLMScintillatorBackward},
    {"EKLM Scintillator Forward", isEKLMScintillatorForward}
  };

  // Process each batch
  for (const auto& batch : batches) {
    B2INFO("Processing batch for profiles: " << batch.first);
    readCalibrationDataBatch(batch.second);

    // Temporary storage for per-channel 2D histograms (only if fill2dHistograms is true)
    // Store pairs of (histogram, target directory) so we can write to correct folder
    std::map<KLMChannelNumber, std::pair<TH2F*, TDirectory*>> tempHistBKLM;
    std::map<KLMChannelNumber, std::pair<TH2F*, TDirectory*>> tempHistEKLM;

    for (KLMChannelIndex klmChannel = m_klmChannels.begin(); klmChannel != m_klmChannels.end(); ++klmChannel) {
      KLMChannelNumber channel = klmChannel.getKLMChannelNumber();

      // Skip if not in current batch
      if (!batch.second(klmChannel))
        continue;

      if (m_cFlag[channel] == ChannelCalibrationStatus::c_NotEnoughData)
        continue;

      if (m_evts.find(channel) == m_evts.end())
        continue;

      std::vector<struct Event> eventsChannel = m_evts[channel];
      int iSub = klmChannel.getSubdetector();

      // Create 2D histogram for this channel if needed
      TH2F* hist2d = nullptr;
      if (fill2dHistograms) {
        if (iSub == KLMElementNumbers::c_BKLM) {
          int iF = klmChannel.getSection();
          int iS = klmChannel.getSector() - 1;
          int iL = klmChannel.getLayer() - 1;
          int iP = klmChannel.getPlane();
          int iC = klmChannel.getStrip() - 1;

          // Only create for scintillators (layers 0-1)
          if (iL < 2) {
            TString hn = Form("time_length_bklm_F%d_S%d_L%d_P%d_C%d", iF, iS, iL, iP, iC);
            double stripLength = 200;
            hist2d = new TH2F(hn.Data(),
                              "Time versus propagation length; "
                              "propagation distance[cm]; "
                              "T_rec-T_0-T_fly-'T_calibration'[ns]",
                              50, 0.0, stripLength,
                              50, m_LowerTimeBoundaryCalibratedScintillatorsBKLM,
                              m_UpperTimeBoundaryCalibratedScintillatorsBKLM);
            tempHistBKLM[channel] = std::make_pair(hist2d, m_channelHistDir_BKLM[iF][iS][iL][iP]);
          }
        } else { // EKLM
          int iF = klmChannel.getSection() - 1;
          int iS = klmChannel.getSector() - 1;
          int iL = klmChannel.getLayer() - 1;
          int iP = klmChannel.getPlane() - 1;
          int iC = klmChannel.getStrip() - 1;

          TString hn = Form("time_length_eklm_F%d_S%d_L%d_P%d_C%d", iF, iS, iL, iP, iC);
          double stripLength = m_EKLMGeometry->getStripLength(iC + 1) / CLHEP::cm * Unit::cm;
          hist2d = new TH2F(hn.Data(),
                            "Time versus propagation length; "
                            "propagation distance[cm]; "
                            "T_rec-T_0-T_fly-'T_calibration'[ns]",
                            50, 0.0, stripLength,
                            50, m_LowerTimeBoundaryCalibratedScintillatorsEKLM,
                            m_UpperTimeBoundaryCalibratedScintillatorsEKLM);
          tempHistEKLM[channel] = std::make_pair(hist2d, m_channelHistDir_EKLM[iF][iS][iL][iP]);
        }
      }

      // Fill histograms
      for (const Event& event : eventsChannel) {
        double timeHit = event.time() - m_timeShift[channel];
        if (m_useEventT0)
          timeHit = timeHit - event.t0;
        double distHit = event.dist;

        if (timeHit <= -400e3)
          continue;

        if (iSub == KLMElementNumbers::c_BKLM) {
          int iL = klmChannel.getLayer() - 1;
          int iP = klmChannel.getPlane();

          if (iL > 1) {
            // RPC
            if (iP) {
              profileRpcPhi->Fill(distHit, timeHit);
            } else {
              profileRpcZ->Fill(distHit, timeHit);
            }
          } else {
            // Scintillator
            if (m_applyChargeRestriction) {
              uint16_t Charge = event.getADCcount;
              if (Charge <= 30 || Charge >= 320) {
                continue;
              }
            }

            if (hist2d)
              hist2d->Fill(distHit, timeHit);

            if (iP) {
              profileBKLMScintillatorPhi->Fill(distHit, timeHit);
            } else {
              profileBKLMScintillatorZ->Fill(distHit, timeHit);
            }
          }
        } else {
          // EKLM
          if (m_applyChargeRestriction) {
            uint16_t Charge = event.getADCcount;
            if (Charge <= 40 || Charge >= 350) {
              continue;
            }
          }

          int iP = klmChannel.getPlane() - 1;

          if (hist2d)
            hist2d->Fill(distHit, timeHit);

          if (iP) {
            profileEKLMScintillatorPlane1->Fill(distHit, timeHit);
          } else {
            profileEKLMScintillatorPlane2->Fill(distHit, timeHit);
          }
        }
      }
    }

    // Write and delete 2D histograms for this batch
    // Use m_saveChannelHists (not m_saveAllPlots) since these are per-channel histograms
    // and the directories are created based on m_saveChannelHists
    if (fill2dHistograms) {
      for (auto& entry : tempHistBKLM) {
        writeThenDelete_(entry.second.first, m_saveChannelHists, entry.second.second);
      }
      for (auto& entry : tempHistEKLM) {
        writeThenDelete_(entry.second.first, m_saveChannelHists, entry.second.second);
      }
    }

    m_evts.clear();
    B2INFO("Batch processed and cleared: " << batch.first);
  }

  B2INFO("Time-distance profile filling complete.");
}

void KLMTimeAlgorithm::timeDistance2dFit(
  const std::vector< std::pair<KLMChannelNumber, unsigned int> >& channels,
  double& delay, double& delayError)
{
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
      s_LowerTimeBoundary = m_LowerTimeBoundaryScintillatorsBKLM;
      s_UpperTimeBoundary = m_UpperTimeBoundaryScintillatorsBKLM;
      const bklm::Module* module =
        m_BKLMGeometry->findModule(section, sector, layer);
      s_StripLength = module->getStripLength(plane, strip);
    } else {
      s_LowerTimeBoundary = m_LowerTimeBoundaryScintillatorsEKLM;
      s_UpperTimeBoundary = m_UpperTimeBoundaryScintillatorsEKLM;
      s_StripLength = m_EKLMGeometry->getStripLength(strip) /
                      CLHEP::cm * Unit::cm;
    }
    for (int j = 0; j < c_NBinsTime; ++j) {
      for (int k = 0; k < c_NBinsDistance; ++k)
        s_BinnedData[j][k] = 0;
    }
    eventsChannel = m_evts[channels[i].first];
    double averageTime = 0;
    for (const Event& event : eventsChannel) {
      double timeHit = event.time();
      if (m_useEventT0)
        timeHit = timeHit - event.t0;

      if (timeHit <= -400e3) {
        continue;
      }

      averageTime = averageTime + timeHit;
      int timeBin = std::floor((timeHit - s_LowerTimeBoundary) * c_NBinsTime /
                               (s_UpperTimeBoundary - s_LowerTimeBoundary));
      if (timeBin < 0 || timeBin >= c_NBinsTime)
        continue;
      int distanceBin = std::floor(event.dist * c_NBinsDistance / s_StripLength);
      if (distanceBin < 0 || distanceBin >= c_NBinsDistance) {
        B2ERROR("The distance to SiPM is greater than the strip length.");
        continue;
      }
      s_BinnedData[timeBin][distanceBin] += 1;
    }
    averageTime = averageTime / eventsChannel.size();
    TMinuit minuit(5);
    minuit.SetPrintLevel(-1);
    int minuitResult;
    minuit.mnparm(0, "P0", 1, 0.001, 0, 0, minuitResult);
    minuit.mnparm(1, "N", 10, 0.001, 0, 0, minuitResult);
    minuit.mnparm(2, "T0", averageTime, 0.001, 0, 0, minuitResult);
    minuit.mnparm(3, "SIGMA", 10, 0.001, 0, 0, minuitResult);
    minuit.mnparm(4, "DELAY", 0.0, 0.001, 0, 0, minuitResult);
    minuit.SetFCN(fcn);
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

void KLMTimeAlgorithm::writeThenDelete_(TH1* h, bool write, TDirectory* dir)
{
  if (h == nullptr)
    return;
  if (write && m_outFile) {
    // Follow the pattern from saveHist(): cd into directory, then SetDirectory, then Write
    if (dir) {
      dir->cd();
      h->SetDirectory(dir);
    } else {
      m_outFile->cd();
      h->SetDirectory(m_outFile);
    }
    h->Write();
    m_outFile->cd();  // Return to root directory
  }
  delete h;
}

void KLMTimeAlgorithm::writeThenDelete_(TH2* h, bool write, TDirectory* dir)
{
  if (h == nullptr)
    return;
  if (write && m_outFile) {
    // Follow the pattern from saveHist(): cd into directory, then SetDirectory, then Write
    if (dir) {
      dir->cd();
      h->SetDirectory(dir);
    } else {
      m_outFile->cd();
      h->SetDirectory(m_outFile);
    }
    h->Write();
    m_outFile->cd();  // Return to root directory
  }
  delete h;
}

bool KLMTimeAlgorithm::passesADCCut(const Event& event, int subdetector, int layer_0indexed) const
{
  // If charge restriction is disabled, accept all hits
  if (!m_applyChargeRestriction) {
    return true;
  }

  uint16_t charge = event.getADCcount;

  if (subdetector == KLMElementNumbers::c_BKLM) {
    // FIXED: Use constant for RPC check (0-indexed comparison)
    if (layer_0indexed >= (BKLMElementNumbers::c_FirstRPCLayer - 1)) {
      return true;  // RPC - no ADC cut
    } else {
      return (charge > 30 && charge < 320);  // Scintillator
    }
  } else {
    return (charge > 40 && charge < 350);  // EKLM
  }
}

CalibrationAlgorithm::EResult KLMTimeAlgorithm::calibrate()
{
  int channelId;
  gROOT->SetBatch(kTRUE);
  setupDatabase();
  m_timeCableDelay = new KLMTimeCableDelay();
  m_timeConstants = new KLMTimeConstants();
  m_timeResolution = new KLMTimeResolution();
  m_eventT0HitResolution = new KLMEventT0HitResolution();

  fcn_gaus = new TF1("fcn_gaus", "gaus");
  fcn_land = new TF1("fcn_land", "landau");
  fcn_pol1 = new TF1("fcn_pol1", "pol1");
  fcn_const = new TF1("fcn_const", "pol0");

  // Initial validation only - DON'T load all data yet
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
    if (i < 0)
      break;
  }
  m_outFile = new TFile(name.c_str(), "recreate");
  createHistograms();

  std::vector<struct Event> eventsChannel;
  eventsChannel.clear();
  m_cFlag.clear();
  m_minimizerOptions.SetDefaultStrategy(2);

  B2INFO("Counting events per channel...");
  std::map<KLMChannelNumber, unsigned int> eventCounts;
  readCalibrationDataCounts(eventCounts);

  /* Sort channels by number of events and initialize flags. */
  std::vector< std::pair<KLMChannelNumber, unsigned int> > channelsBKLM;
  std::vector< std::pair<KLMChannelNumber, unsigned int> > channelsEKLM;
  KLMChannelIndex klmChannels;

  for (KLMChannelIndex& klmChannel : klmChannels) {
    KLMChannelNumber channel = klmChannel.getKLMChannelNumber();
    m_cFlag[channel] = ChannelCalibrationStatus::c_NotEnoughData;

    if (eventCounts.find(channel) == eventCounts.end())
      continue;

    int nEvents = eventCounts[channel];
    if (nEvents < m_lower_limit_counts) {
      B2WARNING("Not enough calibration data collected."
                << LogVar("channel", channel)
                << LogVar("number of digit", nEvents));
      continue;
    }

    m_cFlag[channel] = ChannelCalibrationStatus::c_FailedFit;

    if (klmChannel.getSubdetector() == KLMElementNumbers::c_BKLM &&
        klmChannel.getLayer() < BKLMElementNumbers::c_FirstRPCLayer) {
      channelsBKLM.push_back(std::pair<KLMChannelNumber, unsigned int>(channel, nEvents));
    }
    if (klmChannel.getSubdetector() == KLMElementNumbers::c_EKLM) {
      channelsEKLM.push_back(std::pair<KLMChannelNumber, unsigned int>(channel, nEvents));
    }
  }

  std::sort(channelsBKLM.begin(), channelsBKLM.end(), compareEventNumber);
  std::sort(channelsEKLM.begin(), channelsEKLM.end(), compareEventNumber);

  /* Two-dimensional fit using top channels only. */
  double delayBKLM, delayBKLMError;
  double delayEKLM, delayEKLMError;

  /* Load data for 2D fit channels only. */
  readCalibrationDataFor2DFit(channelsBKLM, channelsEKLM);
  timeDistance2dFit(channelsBKLM, delayBKLM, delayBKLMError);
  timeDistance2dFit(channelsEKLM, delayEKLM, delayEKLMError);
  m_evts.clear();

  B2INFO("2D fits complete, data cleared.");

  /* Define processing batches for channel calibration. */
  auto isRPCBackward = [](const KLMChannelIndex & ch) {
    return ch.getSubdetector() == KLMElementNumbers::c_BKLM &&
           ch.getLayer() >= BKLMElementNumbers::c_FirstRPCLayer &&
           ch.getSection() == BKLMElementNumbers::c_BackwardSection;
  };

  auto isRPCForward = [](const KLMChannelIndex & ch) {
    return ch.getSubdetector() == KLMElementNumbers::c_BKLM &&
           ch.getLayer() >= BKLMElementNumbers::c_FirstRPCLayer &&
           ch.getSection() == BKLMElementNumbers::c_ForwardSection;
  };

  auto isBKLMScintillatorBackward = [](const KLMChannelIndex & ch) {
    return ch.getSubdetector() == KLMElementNumbers::c_BKLM &&
           ch.getLayer() < BKLMElementNumbers::c_FirstRPCLayer &&
           ch.getSection() == BKLMElementNumbers::c_BackwardSection;
  };

  auto isBKLMScintillatorForward = [](const KLMChannelIndex & ch) {
    return ch.getSubdetector() == KLMElementNumbers::c_BKLM &&
           ch.getLayer() < BKLMElementNumbers::c_FirstRPCLayer &&
           ch.getSection() == BKLMElementNumbers::c_ForwardSection;
  };

  auto isEKLMScintillatorBackward = [](const KLMChannelIndex & ch) {
    return ch.getSubdetector() == KLMElementNumbers::c_EKLM &&
           ch.getSection() == EKLMElementNumbers::c_BackwardSection;
  };

  auto isEKLMScintillatorForward = [](const KLMChannelIndex & ch) {
    return ch.getSubdetector() == KLMElementNumbers::c_EKLM &&
           ch.getSection() == EKLMElementNumbers::c_ForwardSection;
  };

  std::vector<std::pair<std::string, std::function<bool(const KLMChannelIndex&)>>> batches = {
    {"RPC Backward", isRPCBackward},
    {"RPC Forward", isRPCForward},
    {"BKLM Scintillator Backward", isBKLMScintillatorBackward},
    {"BKLM Scintillator Forward", isBKLMScintillatorForward},
    {"EKLM Scintillator Backward", isEKLMScintillatorBackward},
    {"EKLM Scintillator Forward", isEKLMScintillatorForward}
  };

  /**********************************
   * FIRST LOOP (BATCHED)
   * Fill global histograms to compute global means
   **********************************/
  B2INFO("First loop: Computing global statistics (batched processing)...");

  TString iFstring[2] = {"Backward", "Forward"};
  TString iPstring[2] = {"ZReadout", "PhiReadout"};
  int nBin = 80;
  int nBin_scint = 80;

  for (const auto& batch : batches) {
    B2INFO("Processing batch for global stats: " << batch.first);
    readCalibrationDataBatch(batch.second);

    for (KLMChannelIndex klmChannel = m_klmChannels.begin(); klmChannel != m_klmChannels.end(); ++klmChannel) {
      channelId = klmChannel.getKLMChannelNumber();

      if (!batch.second(klmChannel))
        continue;

      if (m_cFlag[channelId] == ChannelCalibrationStatus::c_NotEnoughData)
        continue;

      if (m_evts.find(channelId) == m_evts.end())
        continue;

      eventsChannel = m_evts[channelId];
      int iSub = klmChannel.getSubdetector();
      int iL = (iSub == KLMElementNumbers::c_BKLM) ? klmChannel.getLayer() - 1 : -1;

      // Fill global histograms only
      for (const Event& event : eventsChannel) {
        // Apply ADC cut early
        if (!passesADCCut(event, iSub, iL))
          continue;

        XYZVector diffD = XYZVector(event.diffDistX, event.diffDistY, event.diffDistZ);
        h_diff->Fill(diffD.R());

        double timeHit = event.time();
        if (m_useEventT0)
          timeHit = timeHit - event.t0;

        if (timeHit <= -400e3)
          continue;

        if (iSub == KLMElementNumbers::c_BKLM) {
          // FIXED: Use constant instead of hardcoded value
          if (iL >= (BKLMElementNumbers::c_FirstRPCLayer - 1)) {
            h_time_rpc_tc->Fill(timeHit);
          } else {
            h_time_scint_tc->Fill(timeHit);
          }
        } else {
          h_time_scint_tc_end->Fill(timeHit);
        }
      }
    }

    m_evts.clear();
    B2INFO("Batch processed and cleared: " << batch.first);
  }

  // Compute global means
  m_timeShift.clear();
  double tmpMean_rpc_global = h_time_rpc_tc->GetMean();
  double tmpMean_scint_global = h_time_scint_tc->GetMean();
  double tmpMean_scint_global_end = h_time_scint_tc_end->GetMean();

  B2INFO("Global Mean for Raw." << LogVar("RPC", tmpMean_rpc_global)
         << LogVar("Scint BKLM", tmpMean_scint_global)
         << LogVar("Scint EKLM", tmpMean_scint_global_end));

  /**********************************
   * SECOND PASS (BATCHED)
   * Compute per-channel time shifts
   **********************************/
  B2INFO("Second pass: Computing per-channel time shifts (batched processing)...");

  for (const auto& batch : batches) {
    B2INFO("Processing batch for time shifts: " << batch.first);
    readCalibrationDataBatch(batch.second);

    for (KLMChannelIndex klmChannel = m_klmChannels.begin(); klmChannel != m_klmChannels.end(); ++klmChannel) {
      channelId = klmChannel.getKLMChannelNumber();

      if (!batch.second(klmChannel))
        continue;

      if (m_cFlag[channelId] == ChannelCalibrationStatus::c_NotEnoughData)
        continue;

      if (m_evts.find(channelId) == m_evts.end())
        continue;

      eventsChannel = m_evts[channelId];
      int iSub = klmChannel.getSubdetector();
      int iF, iS, iL, iP, iC;

      if (iSub == KLMElementNumbers::c_BKLM) {
        iF = klmChannel.getSection();
        iS = klmChannel.getSector() - 1;
        iL = klmChannel.getLayer() - 1;
        iP = klmChannel.getPlane();
        iC = klmChannel.getStrip() - 1;
      } else {
        iF = klmChannel.getSection() - 1;
        iS = klmChannel.getSector() - 1;
        iL = klmChannel.getLayer() - 1;
        iP = klmChannel.getPlane() - 1;
        iC = klmChannel.getStrip() - 1;
      }

      // Create and fill temp histogram
      TString hn, ht;
      TH1F* h_temp_tc = nullptr;

      if (iSub == KLMElementNumbers::c_BKLM) {
        // FIXED: Use constant instead of hardcoded value
        if (iL >= (BKLMElementNumbers::c_FirstRPCLayer - 1)) {
          hn = Form("h_timeF%d_S%d_L%d_P%d_C%d_tc", iF, iS, iL, iP, iC);
          ht = Form("Time distribution for RPC of Channel%d, %s, Layer%d, Sector%d, %s",
                    iC, iPstring[iP].Data(), iL, iS, iFstring[iF].Data());
          h_temp_tc = new TH1F(hn.Data(), ht.Data(), nBin, m_LowerTimeBoundaryRPC, m_UpperTimeBoundaryRPC);
        } else {
          hn = Form("h_timeF%d_S%d_L%d_P%d_C%d_tc", iF, iS, iL, iP, iC);
          ht = Form("time distribution for Scintillator of Channel%d, %s, Layer%d, Sector%d, %s",
                    iC, iPstring[iP].Data(), iL, iS, iFstring[iF].Data());
          h_temp_tc = new TH1F(hn.Data(), ht.Data(), nBin_scint, m_LowerTimeBoundaryScintillatorsBKLM,
                               m_UpperTimeBoundaryScintillatorsBKLM);
        }
      } else {
        hn = Form("h_timeF%d_S%d_L%d_P%d_C%d_tc_end", iF, iS, iL, iP, iC);
        ht = Form("Time distribution for Scintillator of Channel%d, %s, Layer%d, Sector%d, %s (Endcap)",
                  iC, iPstring[iP].Data(), iL, iS, iFstring[iF].Data());
        h_temp_tc = new TH1F(hn.Data(), ht.Data(), nBin_scint, m_LowerTimeBoundaryScintillatorsEKLM,
                             m_UpperTimeBoundaryScintillatorsEKLM);
      }

      for (const Event& event : eventsChannel) {
        // Add ADC cut
        if (!passesADCCut(event, iSub, iL))
          continue;

        double timeHit = event.time();
        if (m_useEventT0)
          timeHit = timeHit - event.t0;
        if (timeHit <= -400e3)
          continue;
        h_temp_tc->Fill(timeHit);
      }

      h_temp_tc->Fit(fcn_gaus, "LESQ");
      double tmpMean_channel = fcn_gaus->GetParameter(1);

      if (iSub == KLMElementNumbers::c_BKLM) {
        // FIXED: Use constant instead of hardcoded value
        if (iL >= (BKLMElementNumbers::c_FirstRPCLayer - 1)) {
          m_timeShift[channelId] = tmpMean_channel - tmpMean_rpc_global;
        } else {
          m_timeShift[channelId] = tmpMean_channel - tmpMean_scint_global;
        }
      } else {
        m_timeShift[channelId] = tmpMean_channel - tmpMean_scint_global_end;
      }

      delete h_temp_tc;
    }

    m_evts.clear();
    B2INFO("Batch processed and cleared: " << batch.first);
  }

  delete h_time_scint_tc;
  delete h_time_scint_tc_end;
  delete h_time_rpc_tc;
  B2INFO("Effective Light m_timeShift obtained.");

  // NOTE: fillTimeDistanceProfiles also needs batching - user will handle separately
  fillTimeDistanceProfiles(
    m_ProfileRpcPhi, m_ProfileRpcZ,
    m_ProfileBKLMScintillatorPhi, m_ProfileBKLMScintillatorZ,
    m_ProfileEKLMScintillatorPlane1, m_ProfileEKLMScintillatorPlane2, false);

  B2INFO("Effective light speed fitting.");

  // Fit the RPC profiles (for diagnostics), but use fixed values if configured
  m_ProfileRpcPhi->Fit("fcn_pol1", "EMQ");
  double fittedDelayRPCPhi = fcn_pol1->GetParameter(1);
  double e_slope_rpc_phi = fcn_pol1->GetParError(1);

  m_ProfileRpcZ->Fit("fcn_pol1", "EMQ");
  double fittedDelayRPCZ = fcn_pol1->GetParameter(1);
  double e_slope_rpc_z = fcn_pol1->GetParError(1);

  // Use fixed RPC delay if configured (per BELLE2-NOTE-TE-2021-015: c_eff = 0.5c)
  double delayRPCPhi, delayRPCZ;
  if (m_useFixedRPCDelay) {
    delayRPCPhi = m_fixedRPCDelay;
    delayRPCZ = m_fixedRPCDelay;
    B2INFO("Using fixed RPC propagation delay: " << m_fixedRPCDelay << " ns/cm (c_eff = 0.5c)"
           << LogVar("Fitted phi (not used)", fittedDelayRPCPhi)
           << LogVar("Fitted Z (not used)", fittedDelayRPCZ));
  } else {
    delayRPCPhi = fittedDelayRPCPhi;
    delayRPCZ = fittedDelayRPCZ;
  }

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
  if (m_useFixedRPCDelay) {
    logStr_phi = Form("%.4f ns/cm (fixed)", delayRPCPhi);
    logStr_z = Form("%.4f ns/cm (fixed)", delayRPCZ);
    B2INFO("Delay in RPCs (using fixed value):"
           << LogVar("Used Value (phi readout)", logStr_phi.Data())
           << LogVar("Used Value (z readout)", logStr_z.Data()));
  } else {
    logStr_phi = Form("%.4f +/- %.4f ns/cm", delayRPCPhi, e_slope_rpc_phi);
    logStr_z = Form("%.4f +/- %.4f ns/cm", delayRPCZ, e_slope_rpc_z);
    B2INFO("Delay in RPCs:"
           << LogVar("Fitted Value (phi readout)", logStr_phi.Data())
           << LogVar("Fitted Value (z readout)", logStr_z.Data()));
  }
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

  m_timeConstants->setDelay(delayEKLM, KLMTimeConstants::c_EKLM);
  m_timeConstants->setDelay(delayBKLM, KLMTimeConstants::c_BKLM);
  m_timeConstants->setDelay(delayRPCPhi, KLMTimeConstants::c_RPCPhi);
  m_timeConstants->setDelay(delayRPCZ, KLMTimeConstants::c_RPCZ);

  /**********************************
   * THIRD LOOP (BATCHED)
   * Fill per-channel distributions and fit
   **********************************/
  B2INFO("Third loop: Time distribution filling (batched processing)...");

  for (const auto& batch : batches) {
    B2INFO("Processing batch: " << batch.first);
    readCalibrationDataBatch(batch.second);

    for (KLMChannelIndex klmChannel = m_klmChannels.begin(); klmChannel != m_klmChannels.end(); ++klmChannel) {
      channelId = klmChannel.getKLMChannelNumber();

      if (!batch.second(klmChannel))
        continue;

      if (m_cFlag[channelId] == ChannelCalibrationStatus::c_NotEnoughData)
        continue;

      if (m_evts.find(channelId) == m_evts.end())
        continue;

      eventsChannel = m_evts[channelId];
      int iSub = klmChannel.getSubdetector();
      int iF, iS, iL, iP, iC;

      if (iSub == KLMElementNumbers::c_BKLM) {
        iF = klmChannel.getSection();
        iS = klmChannel.getSector() - 1;
        iL = klmChannel.getLayer() - 1;
        iP = klmChannel.getPlane();
        iC = klmChannel.getStrip() - 1;
      } else {
        iF = klmChannel.getSection() - 1;
        iS = klmChannel.getSector() - 1;
        iL = klmChannel.getLayer() - 1;
        iP = klmChannel.getPlane() - 1;
        iC = klmChannel.getStrip() - 1;
      }

      // Create per-channel histogram
      TString hn, ht;
      TH1F* h_temp = nullptr;

      if (iSub == KLMElementNumbers::c_BKLM) {
        // FIXED: Use constant instead of hardcoded value
        if (iL >= (BKLMElementNumbers::c_FirstRPCLayer - 1)) {
          hn = Form("h_timeF%d_S%d_L%d_P%d_C%d", iF, iS, iL, iP, iC);
          ht = Form("Time distribution for RPC of Channel%d, %s, Layer%d, Sector%d, %s",
                    iC, iPstring[iP].Data(), iL, iS, iFstring[iF].Data());
          h_temp = new TH1F(hn.Data(), ht.Data(), nBin, m_LowerTimeBoundaryRPC, m_UpperTimeBoundaryRPC);
        } else {
          hn = Form("h_timeF%d_S%d_L%d_P%d_C%d", iF, iS, iL, iP, iC);
          ht = Form("time distribution for Scintillator of Channel%d, %s, Layer%d, Sector%d, %s",
                    iC, iPstring[iP].Data(), iL, iS, iFstring[iF].Data());
          h_temp = new TH1F(hn.Data(), ht.Data(), nBin_scint, m_LowerTimeBoundaryScintillatorsBKLM,
                            m_UpperTimeBoundaryScintillatorsBKLM);
        }
      } else {
        hn = Form("h_timeF%d_S%d_L%d_P%d_C%d_end", iF, iS, iL, iP, iC);
        ht = Form("Time distribution for Scintillator of Channel%d, %s, Layer%d, Sector%d, %s (Endcap)",
                  iC, iPstring[iP].Data(), iL, iS, iFstring[iF].Data());
        h_temp = new TH1F(hn.Data(), ht.Data(), nBin_scint, m_LowerTimeBoundaryScintillatorsEKLM,
                          m_UpperTimeBoundaryScintillatorsEKLM);
      }

      // Fill histogram
      for (const Event& event : eventsChannel) {
        // Add ADC cut
        if (!passesADCCut(event, iSub, iL))
          continue;

        double timeHit = event.time();
        if (m_useEventT0)
          timeHit = timeHit - event.t0;
        if (timeHit <= -400e3)
          continue;

        if (iSub == KLMElementNumbers::c_BKLM) {
          // FIXED: Use constant instead of hardcoded value
          if (iL >= (BKLMElementNumbers::c_FirstRPCLayer - 1)) {
            double propgationT;
            if (iP == BKLMElementNumbers::c_ZPlane)
              propgationT = event.dist * delayRPCZ;
            else
              propgationT = event.dist * delayRPCPhi;
            double time = timeHit - propgationT;

            h_time_rpc->Fill(time);
            h_temp->Fill(time);

            if (m_saveAllPlots) {
              h_timeF_rpc[iF]->Fill(time);
              h_timeFS_rpc[iF][iS]->Fill(time);
              h_timeFSL[iF][iS][iL]->Fill(time);
              h_timeFSLP[iF][iS][iL][iP]->Fill(time);
              h2_timeF_rpc[iF]->Fill(iS, time);
              h2_timeFS[iF][iS]->Fill(iL, time);
              h2_timeFSLP[iF][iS][iL][iP]->Fill(iC, time);
            }
          } else {
            double propgationT = event.dist * delayBKLM;
            double time = timeHit - propgationT;

            h_time_scint->Fill(time);
            h_temp->Fill(time);

            if (m_saveAllPlots) {
              h_timeF_scint[iF]->Fill(time);
              h_timeFS_scint[iF][iS]->Fill(time);
              h_timeFSL[iF][iS][iL]->Fill(time);
              h_timeFSLP[iF][iS][iL][iP]->Fill(time);
              h2_timeF_scint[iF]->Fill(iS, time);
              h2_timeFS[iF][iS]->Fill(iL, time);
              h2_timeFSLP[iF][iS][iL][iP]->Fill(iC, time);
            }
          }
        } else {
          double propgationT = event.dist * delayEKLM;
          double time = timeHit - propgationT;

          h_time_scint_end->Fill(time);
          h_temp->Fill(time);

          if (m_saveAllPlots) {
            h_timeF_scint_end[iF]->Fill(time);
            h_timeFS_scint_end[iF][iS]->Fill(time);
            h_timeFSL_end[iF][iS][iL]->Fill(time);
            h_timeFSLP_end[iF][iS][iL][iP]->Fill(time);
            h2_timeF_scint_end[iF]->Fill(iS, time);
            h2_timeFS_end[iF][iS]->Fill(iL, time);
            h2_timeFSLP_end[iF][iS][iL][iP]->Fill(iC, time);
          }
        }
      }

      TFitResultPtr r = h_temp->Fit(fcn_gaus, "LESQ");
      if (int(r) == 0) {
        m_cFlag[channelId] = ChannelCalibrationStatus::c_SuccessfulCalibration;
        m_time_channel[channelId] = fcn_gaus->GetParameter(1);
        m_etime_channel[channelId] = fcn_gaus->GetParError(1);
      }

      // Get the appropriate directory for this channel
      TDirectory* dir = (iSub == KLMElementNumbers::c_BKLM) ?
                        m_channelHistDir_BKLM[iF][iS][iL][iP] :
                        m_channelHistDir_EKLM[iF][iS][iL][iP];
      writeThenDelete_(h_temp, m_saveChannelHists, dir);
    }

    m_evts.clear();
    B2INFO("Batch processed and cleared: " << batch.first);
  }

  B2INFO("Original filling done.");

  // Fill TGraphs with extracted parameters
  int iChannel_rpc = 0;
  int iChannel = 0;
  int iChannel_end = 0;
  for (KLMChannelIndex klmChannel = m_klmChannels.begin(); klmChannel != m_klmChannels.end(); ++klmChannel) {
    channelId = klmChannel.getKLMChannelNumber();
    if (m_cFlag[channelId] != ChannelCalibrationStatus::c_SuccessfulCalibration)
      continue;

    int iSub = klmChannel.getSubdetector();
    if (iSub == KLMElementNumbers::c_BKLM) {
      int iL = klmChannel.getLayer() - 1;
      // FIXED: Use constant instead of hardcoded value
      if (iL >= (BKLMElementNumbers::c_FirstRPCLayer - 1)) {
        gre_time_channel_rpc->SetPoint(iChannel_rpc, channelId, m_time_channel[channelId]);
        gre_time_channel_rpc->SetPointError(iChannel_rpc, 0., m_etime_channel[channelId]);
        iChannel_rpc++;
      } else {
        gre_time_channel_scint->SetPoint(iChannel, channelId, m_time_channel[channelId]);
        gre_time_channel_scint->SetPointError(iChannel, 0., m_etime_channel[channelId]);
        iChannel++;
      }
    } else {
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
    m_timeShift[channelId] = timeShift;
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
      B2ERROR("!!! Not All Channels Calibration Constant Set. Error Happened on " << LogVar("Channel", channelId));
      continue;
    }
    int iSub = klmChannel.getSubdetector();
    if (iSub == KLMElementNumbers::c_BKLM) {
      // FIXED: Use 0-indexed layer and constant
      int iL = klmChannel.getLayer() - 1;
      if (iL >= (BKLMElementNumbers::c_FirstRPCLayer - 1)) {
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

  // NOTE: This also needs batching
  fillTimeDistanceProfiles(
    m_Profile2RpcPhi, m_Profile2RpcZ,
    m_Profile2BKLMScintillatorPhi, m_Profile2BKLMScintillatorZ,
    m_Profile2EKLMScintillatorPlane1,  m_Profile2EKLMScintillatorPlane2, true);

  /**********************************
   * FOURTH LOOP (BATCHED)
   * Fill calibrated per-channel histograms
   **********************************/
  B2INFO("Fourth loop: Calibrated time distribution filling (batched processing)...");

  for (const auto& batch : batches) {
    B2INFO("Processing batch: " << batch.first);
    readCalibrationDataBatch(batch.second);

    for (KLMChannelIndex klmChannel = m_klmChannels.begin(); klmChannel != m_klmChannels.end(); ++klmChannel) {
      channelId = klmChannel.getKLMChannelNumber();

      if (!batch.second(klmChannel))
        continue;

      if (m_evts.find(channelId) == m_evts.end())
        continue;

      eventsChannel = m_evts[channelId];
      int iSub = klmChannel.getSubdetector();
      int iF, iS, iL, iP, iC;

      if (iSub == KLMElementNumbers::c_BKLM) {
        iF = klmChannel.getSection();
        iS = klmChannel.getSector() - 1;
        iL = klmChannel.getLayer() - 1;
        iP = klmChannel.getPlane();
        iC = klmChannel.getStrip() - 1;
      } else {
        iF = klmChannel.getSection() - 1;
        iS = klmChannel.getSector() - 1;
        iL = klmChannel.getLayer() - 1;
        iP = klmChannel.getPlane() - 1;
        iC = klmChannel.getStrip() - 1;
      }

      TString hn, ht;
      TH1F* hc_temp = nullptr;

      if (iSub == KLMElementNumbers::c_BKLM) {
        // FIXED: Use constant instead of hardcoded value
        if (iL >= (BKLMElementNumbers::c_FirstRPCLayer - 1)) {
          hn = Form("hc_timeF%d_S%d_L%d_P%d_C%d", iF, iS, iL, iP, iC);
          ht = Form("Calibrated time distribution for RPC of Channel%d, %s, Layer%d, Sector%d, %s",
                    iC, iPstring[iP].Data(), iL, iS, iFstring[iF].Data());
          hc_temp = new TH1F(hn.Data(), ht.Data(), nBin, m_LowerTimeBoundaryCalibratedRPC,
                             m_UpperTimeBoundaryCalibratedRPC);
        } else {
          hn = Form("hc_timeF%d_S%d_L%d_P%d_C%d", iF, iS, iL, iP, iC);
          ht = Form("Calibrated time distribution for Scintillator of Channel%d, %s, Layer%d, Sector%d, %s",
                    iC, iPstring[iP].Data(), iL, iS, iFstring[iF].Data());
          hc_temp = new TH1F(hn.Data(), ht.Data(), nBin_scint, m_LowerTimeBoundaryCalibratedScintillatorsBKLM,
                             m_UpperTimeBoundaryCalibratedScintillatorsBKLM);
        }
      } else {
        hn = Form("hc_timeF%d_S%d_L%d_P%d_C%d_end", iF, iS, iL, iP, iC);
        ht = Form("Calibrated time distribution for Scintillator of Channel%d, %s, Layer%d, Sector%d, %s (Endcap)",
                  iC, iPstring[iP].Data(), iL, iS, iFstring[iF].Data());
        hc_temp = new TH1F(hn.Data(), ht.Data(), nBin_scint, m_LowerTimeBoundaryCalibratedScintillatorsEKLM,
                           m_UpperTimeBoundaryCalibratedScintillatorsEKLM);
      }

      for (const Event& event : eventsChannel) {
        // Add ADC cut
        if (!passesADCCut(event, iSub, iL))
          continue;

        double timeHit = event.time();
        if (m_useEventT0)
          timeHit = timeHit - event.t0;
        if (timeHit <= -400e3)
          continue;

        if (iSub == KLMElementNumbers::c_BKLM) {
          // FIXED: Use constant instead of hardcoded value
          if (iL >= (BKLMElementNumbers::c_FirstRPCLayer - 1)) {
            double propgationT;
            if (iP == BKLMElementNumbers::c_ZPlane)
              propgationT = event.dist * delayRPCZ;
            else
              propgationT = event.dist * delayRPCPhi;
            double time = timeHit - propgationT - m_timeShift[channelId];

            hc_time_rpc->Fill(time);
            hc_temp->Fill(time);

            if (m_saveAllPlots) {
              hc_timeF_rpc[iF]->Fill(time);
              hc_timeFS_rpc[iF][iS]->Fill(time);
              hc_timeFSL[iF][iS][iL]->Fill(time);
              hc_timeFSLP[iF][iS][iL][iP]->Fill(time);
              h2c_timeF_rpc[iF]->Fill(iS, time);
              h2c_timeFS[iF][iS]->Fill(iL, time);
              h2c_timeFSLP[iF][iS][iL][iP]->Fill(iC, time);
            }
          } else {
            double propgationT = event.dist * delayBKLM;
            double time = timeHit - propgationT - m_timeShift[channelId];

            hc_time_scint->Fill(time);
            hc_temp->Fill(time);

            if (m_saveAllPlots) {
              hc_timeF_scint[iF]->Fill(time);
              hc_timeFS_scint[iF][iS]->Fill(time);
              hc_timeFSL[iF][iS][iL]->Fill(time);
              hc_timeFSLP[iF][iS][iL][iP]->Fill(time);
              h2c_timeF_scint[iF]->Fill(iS, time);
              h2c_timeFS[iF][iS]->Fill(iL, time);
              h2c_timeFSLP[iF][iS][iL][iP]->Fill(iC, time);
            }
          }
        } else {
          double propgationT = event.dist * delayEKLM;
          double time = timeHit - propgationT - m_timeShift[channelId];

          hc_time_scint_end->Fill(time);
          hc_temp->Fill(time);

          if (m_saveAllPlots) {
            hc_timeF_scint_end[iF]->Fill(time);
            hc_timeFS_scint_end[iF][iS]->Fill(time);
            hc_timeFSL_end[iF][iS][iL]->Fill(time);
            hc_timeFSLP_end[iF][iS][iL][iP]->Fill(time);
            h2c_timeF_scint_end[iF]->Fill(iS, time);
            h2c_timeFS_end[iF][iS]->Fill(iL, time);
            h2c_timeFSLP_end[iF][iS][iL][iP]->Fill(iC, time);
          }
        }
      }

      if (m_cFlag[channelId] == ChannelCalibrationStatus::c_NotEnoughData) {
        delete hc_temp;
        continue;
      }

      TFitResultPtr rc = hc_temp->Fit(fcn_gaus, "LESQ");
      if (int(rc) == 0) {
        m_cFlag[channelId] = ChannelCalibrationStatus::c_SuccessfulCalibration;
        m_ctime_channel[channelId] = fcn_gaus->GetParameter(1);
        mc_etime_channel[channelId] = fcn_gaus->GetParError(1);
      }

      // Get the appropriate directory for this channel
      TDirectory* dir = (iSub == KLMElementNumbers::c_BKLM) ?
                        m_channelHistDir_BKLM[iF][iS][iL][iP] :
                        m_channelHistDir_EKLM[iF][iS][iL][iP];
      writeThenDelete_(hc_temp, m_saveChannelHists, dir);
    }

    m_evts.clear();
    B2INFO("Batch processed and cleared: " << batch.first);
  }

  // Fill TGraphs with calibrated parameters
  int icChannel_rpc = 0;
  int icChannel = 0;
  int icChannel_end = 0;
  for (KLMChannelIndex klmChannel = m_klmChannels.begin(); klmChannel != m_klmChannels.end(); ++klmChannel) {
    channelId = klmChannel.getKLMChannelNumber();
    if (m_cFlag[channelId] != ChannelCalibrationStatus::c_SuccessfulCalibration)
      continue;

    int iSub = klmChannel.getSubdetector();
    if (iSub == KLMElementNumbers::c_BKLM) {
      int iL = klmChannel.getLayer() - 1;
      // FIXED: Use constant instead of hardcoded value
      if (iL >= (BKLMElementNumbers::c_FirstRPCLayer - 1)) {
        gre_ctime_channel_rpc->SetPoint(icChannel_rpc, channelId, m_ctime_channel[channelId]);
        gre_ctime_channel_rpc->SetPointError(icChannel_rpc, 0., mc_etime_channel[channelId]);
        icChannel_rpc++;
      } else {
        gre_ctime_channel_scint->SetPoint(icChannel, channelId, m_ctime_channel[channelId]);
        gre_ctime_channel_scint->SetPointError(icChannel, 0., mc_etime_channel[channelId]);
        icChannel++;
      }
    } else {
      gre_ctime_channel_scint_end->SetPoint(icChannel_end, channelId, m_ctime_channel[channelId]);
      gre_ctime_channel_scint_end->SetPointError(icChannel_end, 0., mc_etime_channel[channelId]);
      icChannel_end++;
    }
  }

  gre_ctime_channel_scint->Fit("fcn_const", "EMQ");
  m_ctime_channelAvg_scint = fcn_const->GetParameter(0);
  mc_etime_channelAvg_scint = fcn_const->GetParError(0);

  gre_ctime_channel_scint_end->Fit("fcn_const", "EMQ");
  m_ctime_channelAvg_scint_end = fcn_const->GetParameter(0);
  mc_etime_channelAvg_scint_end = fcn_const->GetParError(0);

  gre_ctime_channel_rpc->Fit("fcn_const", "EMQ");
  m_ctime_channelAvg_rpc = fcn_const->GetParameter(0);
  mc_etime_channelAvg_rpc = fcn_const->GetParError(0);

  B2INFO("Channel's time distribution fitting done.");
  B2DEBUG(20, LogVar("Average calibrated time (RPC)", m_ctime_channelAvg_rpc)
          << LogVar("Average calibrated time (BKLM scintillators)", m_ctime_channelAvg_scint)
          << LogVar("Average calibrated time (EKLM scintillators)", m_ctime_channelAvg_scint_end));

  B2INFO("Calibrated channel's time distribution filling begins.");

  m_timeRes.clear();
  for (KLMChannelIndex klmChannel = m_klmChannels.begin(); klmChannel != m_klmChannels.end(); ++klmChannel) {
    channelId = klmChannel.getKLMChannelNumber();
    hc_calibrated->Fill(m_cFlag[channelId]);
    if (m_ctime_channel.find(channelId) == m_ctime_channel.end())
      continue;
    double timeRes = m_ctime_channel[channelId];
    m_timeRes[channelId] = timeRes;
    m_timeResolution->setTimeResolution(channelId, m_timeRes[channelId]);
  }

  for (KLMChannelIndex klmChannel = m_klmChannels.begin(); klmChannel != m_klmChannels.end(); ++klmChannel) {
    channelId = klmChannel.getKLMChannelNumber();
    if (m_timeRes.find(channelId) != m_timeRes.end())
      continue;
    m_timeRes[channelId] = esti_timeRes(klmChannel);
    m_timeResolution->setTimeResolution(channelId, m_timeRes[channelId]);
    B2DEBUG(20, "Calibrated Estimation " << LogVar("Channel", channelId) << LogVar("Estimated value", m_timeRes[channelId]));
  }

  icChannel_rpc = 0;
  icChannel = 0;
  icChannel_end = 0;
  for (KLMChannelIndex klmChannel = m_klmChannels.begin(); klmChannel != m_klmChannels.end(); ++klmChannel) {
    channelId = klmChannel.getKLMChannelNumber();
    if (m_timeRes.find(channelId) == m_timeRes.end()) {
      B2ERROR("!!! Not All Channels Calibration Constant Set. Error Happened on " << LogVar("Channel", channelId));
      continue;
    }
    int iSub = klmChannel.getSubdetector();
    if (iSub == KLMElementNumbers::c_BKLM) {
      // FIXED: Use 0-indexed layer and constant
      int iL = klmChannel.getLayer() - 1;
      if (iL >= (BKLMElementNumbers::c_FirstRPCLayer - 1)) {
        gr_timeRes_channel_rpc->SetPoint(icChannel_rpc, channelId, m_timeRes[channelId]);
        icChannel_rpc++;
      } else {
        gr_timeRes_channel_scint->SetPoint(icChannel, channelId, m_timeRes[channelId]);
        icChannel++;
      }
    } else {
      gr_timeRes_channel_scint_end->SetPoint(icChannel_end, channelId, m_timeRes[channelId]);
      icChannel_end++;
    }
  }

  // ===================================================================
  // FIFTH PASS: Di-muon EventT0 analysis for hit resolution calibration
  // ===================================================================
  B2INFO("Fifth pass: Computing di-muon ΔT0 for EventT0 hit resolution calibration...");

  // Data structure for per-track T0 accumulation
  struct TrackT0Info {
    int charge;
    int nHits_BKLM_Scint;
    int nHits_BKLM_RPC_Phi;  // Split RPC by readout direction
    int nHits_BKLM_RPC_Z;
    int nHits_EKLM_Scint;
    double sumT0_BKLM_Scint;
    double sumT0_BKLM_RPC_Phi;
    double sumT0_BKLM_RPC_Z;
    double sumT0_EKLM_Scint;

    TrackT0Info() : charge(0),
      nHits_BKLM_Scint(0),
      nHits_BKLM_RPC_Phi(0),
      nHits_BKLM_RPC_Z(0),
      nHits_EKLM_Scint(0),
      sumT0_BKLM_Scint(0.0),
      sumT0_BKLM_RPC_Phi(0.0),
      sumT0_BKLM_RPC_Z(0.0),
      sumT0_EKLM_Scint(0.0) {}
  };

  // Map: (Run, Event) -> (nTrack -> TrackT0Info)
  std::map<std::pair<int, int>, std::map<int, TrackT0Info>> eventTrackMap;

  // Process all data in batches to build event-track map
  for (const auto& batch : batches) {
    B2INFO("Processing batch for di-muon analysis: " << batch.first);
    readCalibrationDataBatch(batch.second);

    for (const auto& channelPair : m_evts) {
      KLMChannelNumber channelId = channelPair.first;
      const std::vector<Event>& eventsChannel = channelPair.second;

      // Get channel geometry
      int subdetector, section, sector, layer, plane, strip;
      m_ElementNumbers->channelNumberToElementNumbers(
        channelId, &subdetector, &section, &sector, &layer, &plane, &strip);

      // Convert to 0-indexed immediately (consistent with loops 1-4)
      int iSub = subdetector;
      int iL = layer - 1;

      for (const Event& event : eventsChannel) {
        // Apply ADC cut with 0-indexed layer
        if (!passesADCCut(event, iSub, iL))
          continue;

        // Event and track identification
        std::pair<int, int> eventKey(event.Run, event.Events);
        int trackIdx = event.nTrack;
        int charge = event.Track_Charge;

        // Compute calibrated time for this hit
        double timeHit = event.time() - m_timeShift[channelId];

        if (timeHit <= -400e3)
          continue;

        // Apply propagation correction (using 0-indexed layer)
        double propT = 0.0;
        if (iSub == KLMElementNumbers::c_BKLM) {
          if (iL >= (BKLMElementNumbers::c_FirstRPCLayer - 1)) {
            // RPC
            if (plane == BKLMElementNumbers::c_ZPlane)
              propT = event.dist * delayRPCZ;
            else
              propT = event.dist * delayRPCPhi;
          } else {
            // Scintillator
            propT = event.dist * delayBKLM;
          }
        } else {
          // EKLM
          propT = event.dist * delayEKLM;
        }

        double t0_estimate = timeHit - propT;

        // Accumulate per-track T0
        TrackT0Info& trackInfo = eventTrackMap[eventKey][trackIdx];
        trackInfo.charge = charge;

        if (iSub == KLMElementNumbers::c_BKLM) {
          if (iL >= (BKLMElementNumbers::c_FirstRPCLayer - 1)) {
            // RPC - split by readout direction
            if (plane == BKLMElementNumbers::c_ZPlane) {
              trackInfo.nHits_BKLM_RPC_Z++;
              trackInfo.sumT0_BKLM_RPC_Z += t0_estimate;
            } else {
              trackInfo.nHits_BKLM_RPC_Phi++;
              trackInfo.sumT0_BKLM_RPC_Phi += t0_estimate;
            }
          } else {
            // Scintillator
            trackInfo.nHits_BKLM_Scint++;
            trackInfo.sumT0_BKLM_Scint += t0_estimate;
          }
        } else {
          trackInfo.nHits_EKLM_Scint++;
          trackInfo.sumT0_EKLM_Scint += t0_estimate;
        }
      }
    }

    m_evts.clear();
  }

  B2INFO("Event-track map built. Processing events for EventT0 histograms...");

  // Accumulators for variance calculation using the Gaussian MLE
  // Model: ΔT0_i ~ N(0, σ_hit^2 * v_i),  v_i = 1/N⁺_i + 1/N⁻_i
  double sum_delta2_over_v_BKLM_Scint   = 0.0;
  double sum_delta2_over_v_BKLM_RPC_Phi = 0.0;
  double sum_delta2_over_v_BKLM_RPC_Z   = 0.0;
  double sum_delta2_over_v_EKLM_Scint   = 0.0;

  int nDimuon_BKLM_Scint   = 0;
  int nDimuon_BKLM_RPC_Phi = 0;
  int nDimuon_BKLM_RPC_Z   = 0;
  int nDimuon_EKLM_Scint   = 0;

  for (const auto& eventPair : eventTrackMap) {
    const auto& trackMap = eventPair.second;

    // Check if exactly 2 tracks (di-muon candidate)
    if (trackMap.size() != 2)
      continue;

    auto it = trackMap.begin();
    const TrackT0Info& track1 = it->second;
    ++it;
    const TrackT0Info& track2 = it->second;

    // Check opposite charges
    if (track1.charge * track2.charge >= 0)
      continue;

    // Identify mu+ and mu-
    const TrackT0Info& muPlus  = (track1.charge > 0) ? track1 : track2;
    const TrackT0Info& muMinus = (track1.charge < 0) ? track1 : track2;

    // === BKLM Scintillator ===
    if (muPlus.nHits_BKLM_Scint > 0 && muMinus.nHits_BKLM_Scint > 0) {
      double t0_plus  = muPlus.sumT0_BKLM_Scint  / muPlus.nHits_BKLM_Scint;
      double t0_minus = muMinus.sumT0_BKLM_Scint / muMinus.nHits_BKLM_Scint;
      double deltaT0  = t0_plus - t0_minus;

      // v = 1/N⁺ + 1/N⁻ for this event
      double v = 1.0 / muPlus.nHits_BKLM_Scint + 1.0 / muMinus.nHits_BKLM_Scint;
      int nTotal = muPlus.nHits_BKLM_Scint + muMinus.nHits_BKLM_Scint;

      if (v <= 0.0)
        continue;

      // Accumulate for σ_hit^2 = (1/N_evt) Σ [ ΔT0^2 / v ]
      sum_delta2_over_v_BKLM_Scint += (deltaT0 * deltaT0) / v;
      nDimuon_BKLM_Scint++;

      // Histograms for monitoring
      h_eventT0_scint->Fill(t0_plus);
      h_eventT0_scint->Fill(t0_minus);
      hc_eventT0_scint->Fill(deltaT0);

      // ==== NEW DIAGNOSTIC FILLS ====
      h_nHits_plus_scint->Fill(muPlus.nHits_BKLM_Scint);
      h_nHits_minus_scint->Fill(muMinus.nHits_BKLM_Scint);
      h2_deltaT0_vs_v_scint->Fill(v, deltaT0);
      prof_deltaT0_rms_vs_v_scint->Fill(v, deltaT0);
      h2_deltaT0_vs_nhits_scint->Fill(nTotal, deltaT0);

      // Fill multiplicity-binned histograms
      if (nTotal < 5) {
        hc_eventT0_scint_lowN->Fill(deltaT0);
      } else if (nTotal < 15) {
        hc_eventT0_scint_midN->Fill(deltaT0);
      } else {
        hc_eventT0_scint_highN->Fill(deltaT0);
      }
    }

    // === BKLM RPC Phi ===
    if (muPlus.nHits_BKLM_RPC_Phi > 0 && muMinus.nHits_BKLM_RPC_Phi > 0) {
      double t0_plus  = muPlus.sumT0_BKLM_RPC_Phi  / muPlus.nHits_BKLM_RPC_Phi;
      double t0_minus = muMinus.sumT0_BKLM_RPC_Phi / muMinus.nHits_BKLM_RPC_Phi;
      double deltaT0  = t0_plus - t0_minus;

      double v = 1.0 / muPlus.nHits_BKLM_RPC_Phi + 1.0 / muMinus.nHits_BKLM_RPC_Phi;
      int nTotal = muPlus.nHits_BKLM_RPC_Phi + muMinus.nHits_BKLM_RPC_Phi;

      if (v <= 0.0)
        continue;

      sum_delta2_over_v_BKLM_RPC_Phi += (deltaT0 * deltaT0) / v;
      nDimuon_BKLM_RPC_Phi++;

      h_eventT0_rpc->Fill(t0_plus);
      h_eventT0_rpc->Fill(t0_minus);
      hc_eventT0_rpc->Fill(deltaT0);

      // Diagnostic fills
      h_nHits_plus_rpc->Fill(muPlus.nHits_BKLM_RPC_Phi);
      h_nHits_minus_rpc->Fill(muMinus.nHits_BKLM_RPC_Phi);
      h2_deltaT0_vs_v_rpc->Fill(v, deltaT0);
      prof_deltaT0_rms_vs_v_rpc->Fill(v, deltaT0);
      h2_deltaT0_vs_nhits_rpc->Fill(nTotal, deltaT0);

      if (nTotal < 10) {
        hc_eventT0_rpc_lowN->Fill(deltaT0);
      } else if (nTotal < 30) {
        hc_eventT0_rpc_midN->Fill(deltaT0);
      } else {
        hc_eventT0_rpc_highN->Fill(deltaT0);
      }
    }

    // === BKLM RPC Z ===
    if (muPlus.nHits_BKLM_RPC_Z > 0 && muMinus.nHits_BKLM_RPC_Z > 0) {
      double t0_plus  = muPlus.sumT0_BKLM_RPC_Z  / muPlus.nHits_BKLM_RPC_Z;
      double t0_minus = muMinus.sumT0_BKLM_RPC_Z / muMinus.nHits_BKLM_RPC_Z;
      double deltaT0  = t0_plus - t0_minus;

      double v = 1.0 / muPlus.nHits_BKLM_RPC_Z + 1.0 / muMinus.nHits_BKLM_RPC_Z;
      int nTotal = muPlus.nHits_BKLM_RPC_Z + muMinus.nHits_BKLM_RPC_Z;

      if (v <= 0.0)
        continue;

      sum_delta2_over_v_BKLM_RPC_Z += (deltaT0 * deltaT0) / v;
      nDimuon_BKLM_RPC_Z++;

      h_eventT0_rpc->Fill(t0_plus);
      h_eventT0_rpc->Fill(t0_minus);
      hc_eventT0_rpc->Fill(deltaT0);

      // Diagnostic fills
      h_nHits_plus_rpc->Fill(muPlus.nHits_BKLM_RPC_Z);
      h_nHits_minus_rpc->Fill(muMinus.nHits_BKLM_RPC_Z);
      h2_deltaT0_vs_v_rpc->Fill(v, deltaT0);
      prof_deltaT0_rms_vs_v_rpc->Fill(v, deltaT0);
      h2_deltaT0_vs_nhits_rpc->Fill(nTotal, deltaT0);

      if (nTotal < 10) {
        hc_eventT0_rpc_lowN->Fill(deltaT0);
      } else if (nTotal < 30) {
        hc_eventT0_rpc_midN->Fill(deltaT0);
      } else {
        hc_eventT0_rpc_highN->Fill(deltaT0);
      }
    }

    // === EKLM Scintillator ===
    if (muPlus.nHits_EKLM_Scint > 0 && muMinus.nHits_EKLM_Scint > 0) {
      double t0_plus  = muPlus.sumT0_EKLM_Scint  / muPlus.nHits_EKLM_Scint;
      double t0_minus = muMinus.sumT0_EKLM_Scint / muMinus.nHits_EKLM_Scint;
      double deltaT0  = t0_plus - t0_minus;

      double v = 1.0 / muPlus.nHits_EKLM_Scint + 1.0 / muMinus.nHits_EKLM_Scint;
      int nTotal = muPlus.nHits_EKLM_Scint + muMinus.nHits_EKLM_Scint;

      if (v <= 0.0)
        continue;

      sum_delta2_over_v_EKLM_Scint += (deltaT0 * deltaT0) / v;
      nDimuon_EKLM_Scint++;

      h_eventT0_scint_end->Fill(t0_plus);
      h_eventT0_scint_end->Fill(t0_minus);
      hc_eventT0_scint_end->Fill(deltaT0);

      // ==== NEW DIAGNOSTIC FILLS ====
      h_nHits_plus_scint_end->Fill(muPlus.nHits_EKLM_Scint);
      h_nHits_minus_scint_end->Fill(muMinus.nHits_EKLM_Scint);
      h2_deltaT0_vs_v_scint_end->Fill(v, deltaT0);
      prof_deltaT0_rms_vs_v_scint_end->Fill(v, deltaT0);
      h2_deltaT0_vs_nhits_scint_end->Fill(nTotal, deltaT0);

      // Fill multiplicity-binned histograms
      if (nTotal < 5) {
        hc_eventT0_scint_end_lowN->Fill(deltaT0);
      } else if (nTotal < 15) {
        hc_eventT0_scint_end_midN->Fill(deltaT0);
      } else {
        hc_eventT0_scint_end_highN->Fill(deltaT0);
      }
    }
  }

  B2INFO("Di-muon ΔT0 data collected."
         << LogVar("BKLM Scint di-muon events", nDimuon_BKLM_Scint)
         << LogVar("BKLM RPC Phi di-muon events", nDimuon_BKLM_RPC_Phi)
         << LogVar("BKLM RPC Z di-muon events", nDimuon_BKLM_RPC_Z)
         << LogVar("EKLM Scint di-muon events", nDimuon_EKLM_Scint));

  // === Extract σ_hit using the Gaussian MLE ===
  // σ_hit^2 = (1 / N_evt) Σ_i [ ΔT0_i^2 / (1/N⁺_i + 1/N⁻_i) ]

  float sigma_BKLM_Scint     = 10.0f;  // Default fallback
  float sigma_BKLM_Scint_err = 1.0f;
  if (nDimuon_BKLM_Scint > 0) {
    double sigma2 = sum_delta2_over_v_BKLM_Scint / static_cast<double>(nDimuon_BKLM_Scint);
    sigma_BKLM_Scint = static_cast<float>(std::sqrt(sigma2));
    // Uncertainty approximation (Gaussian statistics)
    sigma_BKLM_Scint_err = sigma_BKLM_Scint / std::sqrt(2.0 * nDimuon_BKLM_Scint);
  }

  float sigma_RPC_Phi     = 10.0f;
  float sigma_RPC_Phi_err = 1.0f;
  if (nDimuon_BKLM_RPC_Phi > 0) {
    double sigma2 = sum_delta2_over_v_BKLM_RPC_Phi / static_cast<double>(nDimuon_BKLM_RPC_Phi);
    sigma_RPC_Phi = static_cast<float>(std::sqrt(sigma2));
    sigma_RPC_Phi_err = sigma_RPC_Phi / std::sqrt(2.0 * nDimuon_BKLM_RPC_Phi);
  }

  float sigma_RPC_Z     = 10.0f;
  float sigma_RPC_Z_err = 1.0f;
  if (nDimuon_BKLM_RPC_Z > 0) {
    double sigma2 = sum_delta2_over_v_BKLM_RPC_Z / static_cast<double>(nDimuon_BKLM_RPC_Z);
    sigma_RPC_Z = static_cast<float>(std::sqrt(sigma2));
    sigma_RPC_Z_err = sigma_RPC_Z / std::sqrt(2.0 * nDimuon_BKLM_RPC_Z);
  }

  // Compute combined RPC sigma (weighted average by number of events)
  float sigma_RPC     = 10.0f;
  float sigma_RPC_err = 1.0f;
  int nDimuon_RPC_total = nDimuon_BKLM_RPC_Phi + nDimuon_BKLM_RPC_Z;
  if (nDimuon_RPC_total > 0) {
    // Weighted average by statistics
    double w_phi = static_cast<double>(nDimuon_BKLM_RPC_Phi) / nDimuon_RPC_total;
    double w_z   = static_cast<double>(nDimuon_BKLM_RPC_Z) / nDimuon_RPC_total;
    sigma_RPC = w_phi * sigma_RPC_Phi + w_z * sigma_RPC_Z;
    // Propagate uncertainties
    sigma_RPC_err = std::sqrt(w_phi * w_phi * sigma_RPC_Phi_err * sigma_RPC_Phi_err +
                              w_z * w_z * sigma_RPC_Z_err * sigma_RPC_Z_err);
  }

  float sigma_EKLM_Scint     = 10.0f;
  float sigma_EKLM_Scint_err = 1.0f;
  if (nDimuon_EKLM_Scint > 0) {
    double sigma2 = sum_delta2_over_v_EKLM_Scint / static_cast<double>(nDimuon_EKLM_Scint);
    sigma_EKLM_Scint = static_cast<float>(std::sqrt(sigma2));
    sigma_EKLM_Scint_err = sigma_EKLM_Scint / std::sqrt(2.0 * nDimuon_EKLM_Scint);
  }

  B2INFO("Extracted per-hit resolutions using event-by-event weighting:"
         << LogVar("σ_BKLM_Scint [ns]", sigma_BKLM_Scint) << LogVar("±", sigma_BKLM_Scint_err)
         << LogVar("σ_RPC_Phi [ns]", sigma_RPC_Phi) << LogVar("±", sigma_RPC_Phi_err)
         << LogVar("σ_RPC_Z [ns]", sigma_RPC_Z) << LogVar("±", sigma_RPC_Z_err)
         << LogVar("σ_RPC_combined [ns]", sigma_RPC) << LogVar("±", sigma_RPC_err)
         << LogVar("σ_EKLM_Scint [ns]", sigma_EKLM_Scint) << LogVar("±", sigma_EKLM_Scint_err));

  // === Store in payload ===
  m_eventT0HitResolution->setSigmaBKLMScint(sigma_BKLM_Scint, sigma_BKLM_Scint_err);
  m_eventT0HitResolution->setSigmaRPC(sigma_RPC, sigma_RPC_err);              // Combined for backward compatibility
  m_eventT0HitResolution->setSigmaRPCPhi(sigma_RPC_Phi, sigma_RPC_Phi_err);  // Direction-specific
  m_eventT0HitResolution->setSigmaRPCZ(sigma_RPC_Z, sigma_RPC_Z_err);        // Direction-specific
  m_eventT0HitResolution->setSigmaEKLMScint(sigma_EKLM_Scint, sigma_EKLM_Scint_err);

  B2INFO("EventT0 hit resolution calibration complete and stored in payload.");

  // Clear temporary data
  eventTrackMap.clear();

  delete fcn_const;
  m_evts.clear();
  m_timeShift.clear();
  m_timeRes.clear();
  m_cFlag.clear();

  saveHist();

  saveCalibration(m_timeCableDelay, "KLMTimeCableDelay");
  saveCalibration(m_timeConstants, "KLMTimeConstants");
  saveCalibration(m_timeResolution, "KLMTimeResolution");
  saveCalibration(m_eventT0HitResolution, "KLMEventT0HitResolution");

  return CalibrationAlgorithm::c_OK;
}

void KLMTimeAlgorithm::saveHist()
{
  m_outFile->cd();
  B2INFO("Save Histograms into Files.");

  /* Save vital plots. */
  TDirectory* dir_monitor = m_outFile->mkdir("monitor_Hists", "", true);
  dir_monitor->cd();
  h_calibrated->SetDirectory(dir_monitor);
  hc_calibrated->SetDirectory(dir_monitor);
  h_diff->SetDirectory(dir_monitor);

  m_outFile->cd();
  TDirectory* dir_eventT0 = m_outFile->mkdir("EventT0", "", true);
  dir_eventT0->cd();

  // Original histograms
  h_eventT0_rpc->SetDirectory(dir_eventT0);
  h_eventT0_scint->SetDirectory(dir_eventT0);
  h_eventT0_scint_end->SetDirectory(dir_eventT0);

  hc_eventT0_rpc->SetDirectory(dir_eventT0);
  hc_eventT0_scint->SetDirectory(dir_eventT0);
  hc_eventT0_scint_end->SetDirectory(dir_eventT0);

  // ==== NEW DIAGNOSTIC PLOTS ====

  // Hit multiplicity
  h_nHits_plus_rpc->SetDirectory(dir_eventT0);
  h_nHits_minus_rpc->SetDirectory(dir_eventT0);
  h_nHits_plus_scint->SetDirectory(dir_eventT0);
  h_nHits_minus_scint->SetDirectory(dir_eventT0);
  h_nHits_plus_scint_end->SetDirectory(dir_eventT0);
  h_nHits_minus_scint_end->SetDirectory(dir_eventT0);

  // ΔT0 vs v
  h2_deltaT0_vs_v_rpc->SetDirectory(dir_eventT0);
  h2_deltaT0_vs_v_scint->SetDirectory(dir_eventT0);
  h2_deltaT0_vs_v_scint_end->SetDirectory(dir_eventT0);

  // Profile plots
  prof_deltaT0_rms_vs_v_rpc->SetDirectory(dir_eventT0);
  prof_deltaT0_rms_vs_v_scint->SetDirectory(dir_eventT0);
  prof_deltaT0_rms_vs_v_scint_end->SetDirectory(dir_eventT0);

  // ΔT0 vs total hits
  h2_deltaT0_vs_nhits_rpc->SetDirectory(dir_eventT0);
  h2_deltaT0_vs_nhits_scint->SetDirectory(dir_eventT0);
  h2_deltaT0_vs_nhits_scint_end->SetDirectory(dir_eventT0);

  // Multiplicity-binned ΔT0
  hc_eventT0_rpc_lowN->SetDirectory(dir_eventT0);
  hc_eventT0_rpc_midN->SetDirectory(dir_eventT0);
  hc_eventT0_rpc_highN->SetDirectory(dir_eventT0);
  hc_eventT0_scint_lowN->SetDirectory(dir_eventT0);
  hc_eventT0_scint_midN->SetDirectory(dir_eventT0);
  hc_eventT0_scint_highN->SetDirectory(dir_eventT0);
  hc_eventT0_scint_end_lowN->SetDirectory(dir_eventT0);
  hc_eventT0_scint_end_midN->SetDirectory(dir_eventT0);
  hc_eventT0_scint_end_highN->SetDirectory(dir_eventT0);

  m_outFile->cd();
  TDirectory* dir_effC = m_outFile->mkdir("effC_Hists", "", true);
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
  TDirectory* dir_time = m_outFile->mkdir("time", "", true);
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
  gre_ctime_channel_rpc->Write("gre_ctime_channel_rpc");
  gre_ctime_channel_scint->Write("gre_ctime_channel_scint");
  gre_ctime_channel_scint_end->Write("gre_ctime_channel_scint_end");
  gr_timeRes_channel_rpc->Write("gr_timeRes_channel_rpc");
  gr_timeRes_channel_scint->Write("gr_timeRes_channel_scint");
  gr_timeRes_channel_scint_end->Write("gr_timeRes_channel_scint_end");

  B2INFO("Top file setup Done.");

  /* Save debug plots (only if m_saveAllPlots is true). */
  if (!m_saveAllPlots) {
    B2INFO("Skipping debug histogram directory creation (m_saveAllPlots = false)");
    m_outFile->cd();
    m_outFile->Write();
    m_outFile->Close();
    B2INFO("File Write and Close. Done.");
    return;
  }

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
    dir_time_F[iF] = dir_time->mkdir(dirname, "", true);
    dir_time_F[iF]->cd();

    for (int iS = 0; iS < 8; ++iS) {
      h_timeFS_rpc[iF][iS]->SetDirectory(dir_time_F[iF]);
      hc_timeFS_rpc[iF][iS]->SetDirectory(dir_time_F[iF]);

      h_timeFS_scint[iF][iS]->SetDirectory(dir_time_F[iF]);
      hc_timeFS_scint[iF][iS]->SetDirectory(dir_time_F[iF]);

      h2_timeFS[iF][iS]->SetDirectory(dir_time_F[iF]);
      h2c_timeFS[iF][iS]->SetDirectory(dir_time_F[iF]);

      sprintf(dirname, "Sector_%d", iS + 1);
      dir_time_FS[iF][iS] = dir_time_F[iF]->mkdir(dirname, "", true);
      dir_time_FS[iF][iS]->cd();

      for (int iL = 0; iL < 15; ++iL) {
        h_timeFSL[iF][iS][iL]->SetDirectory(dir_time_FS[iF][iS]);
        hc_timeFSL[iF][iS][iL]->SetDirectory(dir_time_FS[iF][iS]);

        sprintf(dirname, "Layer_%d", iL + 1);
        dir_time_FSL[iF][iS][iL] = dir_time_FS[iF][iS]->mkdir(dirname, "", true);
        dir_time_FSL[iF][iS][iL]->cd();
        for (int iP = 0; iP < 2; ++iP) {
          h_timeFSLP[iF][iS][iL][iP]->SetDirectory(dir_time_FSL[iF][iS][iL]);
          hc_timeFSLP[iF][iS][iL][iP]->SetDirectory(dir_time_FSL[iF][iS][iL]);
          h2_timeFSLP[iF][iS][iL][iP]->SetDirectory(dir_time_FSL[iF][iS][iL]);
          h2c_timeFSLP[iF][iS][iL][iP]->SetDirectory(dir_time_FSL[iF][iS][iL]);

          sprintf(dirname, "Plane_%d", iP);
          dir_time_FSLP[iF][iS][iL][iP] = dir_time_FSL[iF][iS][iL]->mkdir(dirname, "", true);
          dir_time_FSLP[iF][iS][iL][iP]->cd();

        }
      }
    }

    sprintf(dirname, "isForward_%d_end", iF + 1);
    dir_time_F_end[iF] = dir_time->mkdir(dirname, "", true);
    dir_time_F_end[iF]->cd();
    int maxLayer = 12 + 2 * iF;
    for (int iS = 0; iS < 4; ++iS) {
      h_timeFS_scint_end[iF][iS]->SetDirectory(dir_time_F_end[iF]);
      hc_timeFS_scint_end[iF][iS]->SetDirectory(dir_time_F_end[iF]);

      h2_timeFS_end[iF][iS]->SetDirectory(dir_time_F_end[iF]);
      h2c_timeFS_end[iF][iS]->SetDirectory(dir_time_F_end[iF]);

      sprintf(dirname, "Sector_%d_end", iS + 1);
      dir_time_FS_end[iF][iS] = dir_time_F_end[iF]->mkdir(dirname, "", true);
      dir_time_FS_end[iF][iS]->cd();
      for (int iL = 0; iL < maxLayer; ++iL) {
        h_timeFSL_end[iF][iS][iL]->SetDirectory(dir_time_FS_end[iF][iS]);
        hc_timeFSL_end[iF][iS][iL]->SetDirectory(dir_time_FS_end[iF][iS]);

        sprintf(dirname, "Layer_%d_end", iL + 1);
        dir_time_FSL_end[iF][iS][iL] = dir_time_FS_end[iF][iS]->mkdir(dirname, "", true);
        dir_time_FSL_end[iF][iS][iL]->cd();
        for (int iP = 0; iP < 2; ++iP) {
          h_timeFSLP_end[iF][iS][iL][iP]->SetDirectory(dir_time_FSL_end[iF][iS][iL]);
          hc_timeFSLP_end[iF][iS][iL][iP]->SetDirectory(dir_time_FSL_end[iF][iS][iL]);
          h2_timeFSLP_end[iF][iS][iL][iP]->SetDirectory(dir_time_FSL_end[iF][iS][iL]);
          h2c_timeFSLP_end[iF][iS][iL][iP]->SetDirectory(dir_time_FSL_end[iF][iS][iL]);

          sprintf(dirname, "plane_%d_end", iP);
          dir_time_FSLP_end[iF][iS][iL][iP] = dir_time_FSL_end[iF][iS][iL]->mkdir(dirname, "", true);
          dir_time_FSLP_end[iF][iS][iL][iP]->cd();

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

double KLMTimeAlgorithm::esti_timeRes(const KLMChannelIndex& klmChannel)
{
  double tR = 0.0;
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
    tR = tR_upperStrip(kCIndex_upper).second;
  } else if (iC == totNStrips) {
    KLMChannelIndex kCIndex_lower(iSub, iF, iS, iL, iP, iC - 1);
    tR = tR_lowerStrip(kCIndex_lower).second;
  } else {
    KLMChannelIndex kCIndex_upper(iSub, iF, iS, iL, iP, iC + 1);
    KLMChannelIndex kCIndex_lower(iSub, iF, iS, iL, iP, iC - 1);
    std::pair<int, double> tR_upper = tR_upperStrip(kCIndex_upper);
    std::pair<int, double> tR_lower = tR_lowerStrip(kCIndex_lower);
    unsigned int tr_upper = tR_upper.first - iC;
    unsigned int tr_lower = iC - tR_lower.first;
    unsigned int tr = tR_upper.first - tR_lower.first;
    tR = (double(tr_upper) * tR_lower.second + double(tr_lower) * tR_upper.second) / double(tr);
  }
  return tR;
}

std::pair<int, double> KLMTimeAlgorithm::tR_upperStrip(const KLMChannelIndex& klmChannel)
{
  std::pair<int, double> tR;
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
  if (m_timeRes.find(cId) != m_timeRes.end()) {
    tR.first = iC;
    tR.second = m_timeRes[cId];
  } else if (iC == totNStrips) {
    tR.first = iC;
    tR.second = 0.0;
  } else {
    KLMChannelIndex kCIndex(iSub, iF, iS, iL, iP, iC + 1);
    tR = tR_upperStrip(kCIndex);
  }
  return tR;
}

std::pair<int, double> KLMTimeAlgorithm::tR_lowerStrip(const KLMChannelIndex& klmChannel)
{
  std::pair<int, double> tR;
  int cId = klmChannel.getKLMChannelNumber();
  int iSub = klmChannel.getSubdetector();
  int iF = klmChannel.getSection();
  int iS = klmChannel.getSector();
  int iL = klmChannel.getLayer();
  int iP = klmChannel.getPlane();
  int iC = klmChannel.getStrip();
  if (m_timeRes.find(cId) != m_timeRes.end()) {
    tR.first = iC;
    tR.second = m_timeRes[cId];
  } else if (iC == 1) {
    tR.first = iC;
    tR.second = 0.0;
  } else {
    KLMChannelIndex kCIndex(iSub, iF, iS, iL, iP, iC - 1);
    tR = tR_lowerStrip(kCIndex);
  }
  return tR;
}