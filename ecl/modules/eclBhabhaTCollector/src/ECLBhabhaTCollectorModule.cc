/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <ecl/modules/eclBhabhaTCollector/ECLBhabhaTCollectorModule.h>
#include <framework/dataobjects/EventMetaData.h>
#include <framework/gearbox/Const.h>
#include <ecl/dbobjects/ECLCrystalCalib.h>
#include <ecl/dbobjects/ECLReferenceCrystalPerCrateCalib.h>
#include <ecl/dataobjects/ECLDigit.h>
#include <ecl/dataobjects/ECLCalDigit.h>
#include <mdst/dataobjects/ECLCluster.h>
#include <mdst/dataobjects/Track.h>
#include <mdst/dataobjects/HitPatternCDC.h>
#include <tracking/dataobjects/RecoTrack.h>
#include <ecl/digitization/EclConfiguration.h>
#include <analysis/utility/PCmsLabTransform.h>
#include <analysis/ClusterUtility/ClusterUtils.h>
#include <boost/optional.hpp>
#include <ecl/geometry/ECLGeometryPar.h>

#include <TH2F.h>
#include <TTree.h>
#include <TFile.h>

using namespace Belle2;
using namespace ECL;
using namespace std;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(ECLBhabhaTCollector)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

ECLBhabhaTCollectorModule::ECLBhabhaTCollectorModule() : CalibrationCollectorModule(),
  m_ElectronicsDB("ECLCrystalElectronics"),
  m_ElectronicsTimeDB("ECLCrystalElectronicsTime"),
  m_FlightTimeDB("ECLCrystalFlightTime"),
  m_PreviousCrystalTimeDB("ECLCrystalTimeOffset"),
  m_CrateTimeDB("ECLCrateTimeOffset"),
  m_RefCrystalsCalibDB("ECLReferenceCrystalPerCrateCalib")//,
  //m_dbgTree_electrons(0),
  //m_tree_evtNum(0)//,
{
  setDescription("This module generates sum of all event times per crystal");

  addParam("timeAbsMax", m_timeAbsMax, // (Time in ns)
           "Events with fabs(getTimeFit) > m_timeAbsMax "
           "are excluded", (short)80);

  addParam("minCrystal", m_minCrystal,
           "First CellId to handle.", 1);
  addParam("maxCrystal", m_maxCrystal,
           "Last CellId to handle.", 8736);

  addParam("saveTree", m_saveTree,
           "If true, TTree 'tree' with more detailed event info is saved in "
           "the output file specified by HistoManager",
           false);

  addParam("looseTrkZ0", m_looseTrkZ0, "max Z0 for loose tracks (cm)", 10.);
  addParam("tightTrkZ0", m_tightTrkZ0, "max Z0 for tight tracks (cm)", 2.);
  addParam("looseTrkD0", m_looseTrkD0, "max D0 for loose tracks (cm)", 2.);
  addParam("tightTrkD0", m_tightTrkD0, "max D0 for tight tracks (cm)", 0.5);  // beam pipe radius = 1cm in 2019

  addParam("hadronEventT0_TO_bhabhaEventT0_correction", m_hadronEventT0_TO_bhabhaEventT0_correction,
           "CDC bhabha t0 bias correction (ns)", 0.);

  // specify this flag if you need parallel processing
  setPropertyFlags(c_ParallelProcessingCertified);
}

ECLBhabhaTCollectorModule::~ECLBhabhaTCollectorModule()
{
}

void ECLBhabhaTCollectorModule::inDefineHisto()
{
  //=== Prepare TTree for debug output
  if (m_saveTree) {    //  /* >>>>>>>>>>>>>>>>>>>>>>>>>>>> if boolean true for saving debug trees  >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
    // Per electron
    m_dbgTree_electrons = new TTree("tree_electrons", "Debug data for bhabha time calibration - one entry per electron");
    m_dbgTree_electrons->Branch("EventNum", &m_tree_evtNum)->SetTitle("Event number");
    m_dbgTree_electrons->Branch("CrystalCellID", &m_tree_cid)->SetTitle("Cell ID, 1..8736");
    m_dbgTree_electrons->Branch("ADCamplitude", &m_tree_amp)->SetTitle("Amplitude, ADC units");
    m_dbgTree_electrons->Branch("maxEcrystalEnergy", &m_tree_en)->SetTitle("Max Energy Crystal Energy, GeV");
    m_dbgTree_electrons->Branch("maxEcrystalEnergyDIVclusterE",
                                &m_tree_E1Etot)->SetTitle("Max Energy Crystal Fraction Energy of Cluster");
    m_dbgTree_electrons->Branch("E1divE2crystal",
                                &m_tree_E1E2)->SetTitle("Max Energy Crystal DIV second max energy crystal in cluster");
    m_dbgTree_electrons->Branch("E1crystal_DIV_p", &m_tree_E1p)->SetTitle("Max Energy Crystal in cluster DIV track p");
    m_dbgTree_electrons->Branch("timetsPreviousTimeCalibs",
                                &m_tree_timetsPreviousTimeCalibs)->SetTitle("Time t_psi after application of previous Ts, ns");
    m_dbgTree_electrons->Branch("E_DIV_p", &m_E_DIV_p)->SetTitle("E DIV p");
    m_dbgTree_electrons->Branch("timeF", &m_tree_timeF)->SetTitle("Time F, ns");
    m_dbgTree_electrons->Branch("time_t_psi", &m_tree_time)->SetTitle("Time t_psi for Ts, ns");
    m_dbgTree_electrons->Branch("quality", &m_tree_quality)->SetTitle("ECL FPGA fit quality, see Confluence article");
    m_dbgTree_electrons->Branch("t0", &m_tree_t0)->SetTitle("T0, ns");
    m_dbgTree_electrons->Branch("t0_unc", &m_tree_t0_unc)->SetTitle("T0 uncertainty, ns");
    m_dbgTree_electrons->Branch("CrateID", &m_crystalCrate)->SetTitle("Crate id for crystal");
    m_dbgTree_electrons->Branch("runNum", &m_runNum)->SetTitle("Run number");

    m_dbgTree_electrons->SetAutoSave(10);


    // Per track
    m_dbgTree_tracks = new TTree("tree_tracks", "Debug data for bhabha time calibration - one entry per track");
    m_dbgTree_tracks->Branch("d0", &m_tree_d0)->SetTitle("d0, cm");
    m_dbgTree_tracks->Branch("z0", &m_tree_z0)->SetTitle("z0, cm");
    m_dbgTree_tracks->Branch("p", &m_tree_p)->SetTitle("track momentum, GeV");
    m_dbgTree_tracks->Branch("charge", &m_charge)->SetTitle("track electric charge");
    m_dbgTree_tracks->Branch("Num_CDC_hits", &m_tree_nCDChits)->SetTitle("Num CDC hits");

    m_dbgTree_tracks->SetAutoSave(10);

    // Per crystal
    m_dbgTree_crystals = new TTree("tree_crystals",
                                   "Debug data for bhabha time calibration - one entry per electron - one entry per crystal");
    m_dbgTree_crystals->Branch("clustCrysE_DIV_maxEcrys",
                               &m_tree_clustCrysE_DIV_maxEcrys)->SetTitle("E of crystal i from cluster / E of max E crystal");
    m_dbgTree_crystals->Branch("Crystal_E", &m_tree_clustCrysE) ->SetTitle("E of crystal i from cluster");
    m_dbgTree_crystals->Branch("time_t_psi", &m_tree_time)->SetTitle("Time for Ts, ns");
    m_dbgTree_crystals->Branch("Crystal_cell_ID", &m_tree_cid)->SetTitle("Cell ID, 1..8736");
    m_dbgTree_crystals->Branch("quality", &m_tree_quality)->SetTitle("ECL FPGA fit quality, see Confluence article");

    m_dbgTree_crystals->SetAutoSave(10);


    // Per event
    m_dbgTree_event = new TTree("tree_event", "Debug data for bhabha time calibration - one entry per event");
    m_dbgTree_event->Branch("massInvTracks", &m_massInvTracks)->SetTitle("Invariant mass of the two tracks");

    m_dbgTree_event->SetAutoSave(10);


    m_dbgTree_evt_allCuts = new TTree("tree_evt_allCuts",
                                      "Debug data for bhabha time calibration - one entry per event after all the cuts");
    m_dbgTree_evt_allCuts->Branch("EclustPlus", &m_tree_enPlus)->SetTitle("Energy of cluster with +ve charge, GeV");
    m_dbgTree_evt_allCuts->Branch("EclustNeg", &m_tree_enNeg)->SetTitle("Energy of cluster with -ve charge, GeV");
    m_dbgTree_evt_allCuts->Branch("clustTimePos", &m_tree_tClustPos)->SetTitle("Cluster time of cluster with +ve charge, GeV");
    m_dbgTree_evt_allCuts->Branch("clustTimeNeg", &m_tree_tClustNeg)->SetTitle("Cluster time of cluster with -ve charge, GeV");
    m_dbgTree_evt_allCuts->Branch("maxEcrysTimePosClust",
                                  &m_tree_maxEcrystPosClust)->SetTitle("Time of maximum energy crystal in cluster with +ve charge, GeV");
    m_dbgTree_evt_allCuts->Branch("maxEcrysTimeNegClust",
                                  &m_tree_maxEcrystNegClust)->SetTitle("Time of maximum energy crystal in cluster with -ve charge, GeV");
    m_dbgTree_evt_allCuts->Branch("t0", &m_tree_t0)->SetTitle("T0, ns");
    m_dbgTree_evt_allCuts->Branch("t0_ECL_closestCDC", &m_tree_t0_ECLclosestCDC)->SetTitle("T0 ECL closest to CDC t0, ns");
    m_dbgTree_evt_allCuts->Branch("t0_ECL_minChi2", &m_tree_t0_ECL_minChi2)->SetTitle("T0 ECL with smallest chi squared, ns");

    m_dbgTree_evt_allCuts->SetAutoSave(10);


    // Per crystal within each cluster, entry after all the cuts
    m_dbgTree_crys_allCuts = new TTree("m_dbgTree_crys_allCuts",
                                       "Debug data for bhabha time calibration - one entry per crystal per cluster entry after all cuts");

    m_dbgTree_crys_allCuts->Branch("runNum", &m_runNum)->SetTitle("Run number");
    m_dbgTree_crys_allCuts->Branch("EventNum", &m_tree_evtNum)->SetTitle("Event number");
    m_dbgTree_crys_allCuts->Branch("m_tree_ECLCalDigitTime",
                                   &m_tree_ECLCalDigitTime)
    ->SetTitle("Time of a crystal within the cluster after application of previous calibrations except t0, ns");
    m_dbgTree_crys_allCuts->Branch("m_tree_ECLCalDigitE", &m_tree_ECLCalDigitE)->SetTitle("Energy of crystal, GeV");
    m_dbgTree_crys_allCuts->Branch("m_tree_ECLDigitAmplitude",
                                   &m_tree_ECLDigitAmplitude)->SetTitle("Amplitude of crystal signal pulse");
    m_dbgTree_crys_allCuts->Branch("timetsPreviousTimeCalibs",
                                   &m_tree_timetsPreviousTimeCalibs)->SetTitle("Time t_psi after application of previous Ts, ns");
    m_dbgTree_crys_allCuts->Branch("t0", &m_tree_t0)->SetTitle("T0, ns");
    m_dbgTree_crys_allCuts->Branch("t0_ECL_closestCDC", &m_tree_t0_ECLclosestCDC)->SetTitle("T0 ECL closest to CDC t0, ns");
    m_dbgTree_crys_allCuts->Branch("t0_ECL_minChi2", &m_tree_t0_ECL_minChi2)->SetTitle("T0 ECL with smallest chi squared, ns");
    m_dbgTree_crys_allCuts->Branch("CrystalCellID", &m_tree_cid)->SetTitle("Cell ID, 1..8736");

    m_dbgTree_crys_allCuts->SetAutoSave(10);


  }   // <<<<<<<<<<<<<<<<<<<<<  if boolean true for saving debug trees <<<<<<<<<<<<<<<<<<<<<<<<<< */


  // Per max E crystal entry after all the cuts
  //    this tree is always saved
  m_dbgTree_allCuts = new TTree("tree_allCuts",
                                "Debug data for bhabha time calibration - one entry per max E crystal entry after cuts");

  m_dbgTree_allCuts->Branch("time_t_psi", &m_tree_time)->SetTitle("Time t_psi for Ts, ns");
  m_dbgTree_allCuts->Branch("crateID", &m_crystalCrate)->SetTitle("Crate id for crystal");
  m_dbgTree_allCuts->Branch("EventNum", &m_tree_evtNum)->SetTitle("Event number");
  m_dbgTree_allCuts->Branch("runNum", &m_runNum)->SetTitle("Run number");
  m_dbgTree_allCuts->Branch("CrystalCellID", &m_tree_cid)->SetTitle("Cell ID, 1..8736");
  m_dbgTree_allCuts->Branch("maxEcrystalEnergy", &m_tree_en)->SetTitle("Max Energy Crystal Energy, GeV");
  m_dbgTree_allCuts->Branch("maxEcrystalEnergyDIVclusterE",
                            &m_tree_E1Etot)->SetTitle("Max Energy Crystal Fraction Energy of Cluster");
  m_dbgTree_allCuts->Branch("E1divE2crystal",
                            &m_tree_E1E2)->SetTitle("Max Energy Crystal DIV second max energy crystal in cluster");
  m_dbgTree_allCuts->Branch("E1crystalDIVp", &m_tree_E1p)->SetTitle("Max Energy Crystal in cluster DIV track p");
  m_dbgTree_allCuts->Branch("timetsPreviousTimeCalibs",
                            &m_tree_timetsPreviousTimeCalibs)->SetTitle("Time t_psi after application of previous Ts, ns");
  m_dbgTree_allCuts->Branch("massInvTracks", &m_massInvTracks)->SetTitle("Invariant mass of the two tracks");
  m_dbgTree_allCuts->Branch("t0", &m_tree_t0)->SetTitle("T0, ns");
  m_dbgTree_allCuts->Branch("t0_ECL_closestCDC", &m_tree_t0_ECLclosestCDC)->SetTitle("T0 ECL closest to CDC t0, ns");
  m_dbgTree_allCuts->Branch("t0_ECL_minChi2", &m_tree_t0_ECL_minChi2)->SetTitle("T0 ECL with smallest chi squared, ns");

  m_dbgTree_allCuts->Branch("clusterTime", &m_tree_tClust)->SetTitle("Cluster time of cluster with +ve charge, GeV");

  m_dbgTree_allCuts->SetAutoSave(10);

}

void ECLBhabhaTCollectorModule::prepare()
{
  //=== MetaData
  B2INFO("ECLBhabhaTCollector: Experiment = " << m_EventMetaData->getExperiment() <<
         "  run = " << m_EventMetaData->getRun());


  //=== Create histograms and register them in the data store
  int nbins = m_timeAbsMax * 8;
  int max_t = m_timeAbsMax;
  int min_t = -m_timeAbsMax;


  auto TimevsCrysPrevCrateCalibPrevCrystCalib = new TH2F("TimevsCrysPrevCrateCalibPrevCrystCalib",
                                                         "Time t psi - ts - tcrate (previous calibs) vs crystal cell ID;crystal cell ID;Time t_psi with previous calib (ns)",
                                                         8736, 1, 8736 + 1, nbins, min_t, max_t);
  registerObject<TH2F>("TimevsCrysPrevCrateCalibPrevCrystCalib", TimevsCrysPrevCrateCalibPrevCrystCalib);

  auto TimevsCratePrevCrateCalibPrevCrystCalib = new TH2F("TimevsCratePrevCrateCalibPrevCrystCalib",
                                                          "Time t psi - ts - tcrate (previous calibs) vs crate ID;crate ID;Time t_psi previous calib (ns)",
                                                          52, 1, 52 + 1, nbins, min_t, max_t);
  registerObject<TH2F>("TimevsCratePrevCrateCalibPrevCrystCalib", TimevsCratePrevCrateCalibPrevCrystCalib);

  auto TimevsCrysNoCalibrations = new TH2F("TimevsCrysNoCalibrations",
                                           "Time tpsi vs crystal cell ID;crystal cell ID;Time t_psi (ns)", 8736, 1, 8736 + 1, nbins, min_t, max_t);
  registerObject<TH2F>("TimevsCrysNoCalibrations", TimevsCrysNoCalibrations);

  auto TimevsCrateNoCalibrations = new TH2F("TimevsCrateNoCalibrations",
                                            "Time tpsi vs crate ID;crate ID;Time t_psi (ns)", 52, 1, 52 + 1, nbins, min_t, max_t);
  registerObject<TH2F>("TimevsCrateNoCalibrations", TimevsCrateNoCalibrations);

  auto TimevsCrysPrevCrateCalibNoCrystCalib = new TH2F("TimevsCrysPrevCrateCalibNoCrystCalib",
                                                       "Time tpsi - tcrate (previous calib) vs crystal cell ID;crystal cell ID;Time t_psi including previous crate calib (ns)",
                                                       8736, 1, 8736 + 1, nbins, min_t, max_t);
  registerObject<TH2F>("TimevsCrysPrevCrateCalibNoCrystCalib", TimevsCrysPrevCrateCalibNoCrystCalib);

  auto TimevsCrateNoCrateCalibPrevCrystCalib = new TH2F("TimevsCrateNoCrateCalibPrevCrystCalib",
                                                        "Time tpsi - ts (previous calib) vs crate ID;crate ID;Time t_psi including previous crystal calib (ns)",
                                                        52, 1, 52 + 1, nbins, min_t, max_t);
  registerObject<TH2F>("TimevsCrateNoCrateCalibPrevCrystCalib", TimevsCrateNoCrateCalibPrevCrystCalib);


  auto TsDatabase = new TH1F("TsDatabase", ";cell id;Ts from database", 8736, 1, 8736 + 1);
  registerObject<TH1F>("TsDatabase", TsDatabase);

  auto TsDatabaseUnc = new TH1F("TsDatabaseUnc", ";cell id;Ts uncertainty from database", 8736, 1, 8736 + 1);
  registerObject<TH1F>("TsDatabaseUnc", TsDatabaseUnc);

  auto TcrateDatabase = new TH1F("TcrateDatabase", ";cell id;Tcrate from database", 8736, 1, 8736 + 1);
  registerObject<TH1F>("TcrateDatabase", TcrateDatabase);

  auto TcrateUncDatabase = new TH1F("TcrateUncDatabase", ";cell id;Tcrate uncertainty from database", 8736, 1, 8736 + 1);
  registerObject<TH1F>("TcrateUncDatabase", TcrateUncDatabase);


  auto tcrateDatabase_ns = new TH1F("tcrateDatabase_ns", ";crate id;tcrate derived from database", 52, 1, 52 + 1);
  registerObject<TH1F>("tcrateDatabase_ns", tcrateDatabase_ns);


  auto databaseCounter = new TH1I("databaseCounter",
                                  ";A database was read in;Number of times database was saved to histogram", 1, 1, 2);
  registerObject<TH1I>("databaseCounter", databaseCounter);


  auto numCrystalEntriesPerEvent = new TH1F("numCrystalEntriesPerEvent",
                                            ";Number crystal entries;Number of events", 15, 0, 15);
  registerObject<TH1F>("numCrystalEntriesPerEvent", numCrystalEntriesPerEvent);

  auto cutflow = new TH1F("cutflow", ";Cut label number;Number of events passing cut", 20, 0, 20);
  registerObject<TH1F>("cutflow", cutflow);

  auto maxEcrsytalEnergyFraction = new TH1F("maxEcrsytalEnergyFraction",
                                            ";Maximum energy crystal energy / (sum) cluster energy;Number", 22, 0, 1.1);
  registerObject<TH1F>("maxEcrsytalEnergyFraction", maxEcrsytalEnergyFraction);

  auto refCrysIDzeroingCrate = new TH1F("refCrysIDzeroingCrate", ";cell id;Boolean - is reference crystal", 8736, 1, 8736 + 1);
  registerObject<TH1F>("refCrysIDzeroingCrate", refCrysIDzeroingCrate);

  auto CDCEventT0Correction = new TH1F("CDCEventT0Correction", ";;CDC event t0 offset correction  [ns]", 1, 1, 2);
  registerObject<TH1F>("CDCEventT0Correction", CDCEventT0Correction);


  //=== Required data objects
  m_eventT0.isRequired();
  tracks.isRequired();
  m_eclClusterArray.isRequired();
  m_eclCalDigitArray.isRequired();
  m_eclDigitArray.isRequired();

  B2INFO("hadronEventT0_TO_bhabhaEventT0_correction = " <<  m_hadronEventT0_TO_bhabhaEventT0_correction <<
         " ns correction to CDC event t0 will be applied");

}

void ECLBhabhaTCollectorModule::collect()
{
  int cutIndexPassed = 0;
  getObjectPtr<TH1F>("cutflow")->Fill(cutIndexPassed);
  B2DEBUG(22, "Cutflow: no cuts: index = " << cutIndexPassed);
  B2DEBUG(22, "Event number = " << m_EventMetaData->getEvent());


  /* Use ECLChannelMapper to get other detector indices for the crystals */
  /* For conversion from CellID to crate, shaper, and channel ids. */

  // Use smart pointer to avoid memory leak when the ECLChannelMapper object needs destroying at the end of the event.
  shared_ptr< ECL::ECLChannelMapper > crystalMapper(new ECL::ECLChannelMapper());
  crystalMapper->initFromDB();

  //== Get expected energies and calibration constants from DB. Need to call
  //   hasChanged() for later comparison
  if (m_ElectronicsDB.hasChanged()) {
    m_Electronics = m_ElectronicsDB->getCalibVector();
  }
  if (m_ElectronicsTimeDB.hasChanged()) {
    m_ElectronicsTime = m_ElectronicsTimeDB->getCalibVector();
  }
  if (m_FlightTimeDB.hasChanged()) {
    m_FlightTime = m_FlightTimeDB->getCalibVector();
  }

  // Get the previous crystal time offset (the same thing that this calibration is meant to calculate).
  // This can be used for testing purposes, and for the crate time offset.
  if (m_PreviousCrystalTimeDB.hasChanged()) {
    m_PreviousCrystalTime = m_PreviousCrystalTimeDB->getCalibVector();
    m_PreviousCrystalTimeUnc = m_PreviousCrystalTimeDB->getCalibUncVector();
  }

  B2DEBUG(29, "Finished checking if previous crystal time payload has changed");
  if (m_CrateTimeDB.hasChanged()) {
    m_CrateTime = m_CrateTimeDB->getCalibVector();
    m_CrateTimeUnc = m_CrateTimeDB->getCalibUncVector();
  }
  B2DEBUG(29, "Finished checking if previous crate time payload has changed");
  B2DEBUG(29, "m_CrateTime size = " << m_CrateTime.size());
  B2DEBUG(25, "Crate time +- uncertainty [0]= " << m_CrateTime[0] << " +- " << m_CrateTimeUnc[0]);
  B2DEBUG(25, "Crate time +- uncertainty [8735]= " << m_CrateTime[8735] << " +- " << m_CrateTimeUnc[8735]);

  B2DEBUG(29, "Finished checking if previous crate time payload has changed");
  if (m_RefCrystalsCalibDB.hasChanged()) {
    m_RefCrystalsCalib = m_RefCrystalsCalibDB->getReferenceCrystals();
  }
  B2DEBUG(29, "Finished checking if reference crystal cell ids payload has changed");


  B2DEBUG(25, "ECLBhabhaTCollector:: loaded ECLCrystalTimeOffset from the database"
          << LogVar("IoV", m_PreviousCrystalTimeDB.getIoV())
          << LogVar("Checksum", m_PreviousCrystalTimeDB.getChecksum()));
  B2DEBUG(25, "ECLBhabhaTCollector:: loaded ECLCrateTimeOffset from the database"
          << LogVar("IoV", m_CrateTimeDB.getIoV())
          << LogVar("Checksum", m_CrateTimeDB.getChecksum()));
  B2DEBUG(25, "ECLBhabhaTCollector:: loaded ECLCrystalElectronics from the database"
          << LogVar("IoV", m_ElectronicsDB.getIoV())
          << LogVar("Checksum", m_ElectronicsDB.getChecksum()));
  B2DEBUG(25, "ECLBhabhaTCollector:: loaded ECLCrystalElectronicsTime from the database"
          << LogVar("IoV", m_ElectronicsTimeDB.getIoV())
          << LogVar("Checksum", m_ElectronicsTimeDB.getChecksum()));
  B2DEBUG(25, "ECLBhabhaTCollector:: loaded ECLCrystalFlightTime from the database"
          << LogVar("IoV", m_FlightTimeDB.getIoV())
          << LogVar("Checksum", m_FlightTimeDB.getChecksum()));
  B2DEBUG(25, "ECLBhabhaTCollector:: loaded ECLReferenceCrystalPerCrateCalib from the database"
          << LogVar("IoV", m_RefCrystalsCalibDB.getIoV())
          << LogVar("Checksum", m_RefCrystalsCalibDB.getChecksum()));



  // Conversion coefficient from ADC ticks to nanoseconds
  // TICKS_TO_NS ~ 0.4913 ns/clock tick
  // 1/(4fRF) = 0.4913 ns/clock tick, where fRF is the accelerator RF frequency, fRF=508.889 MHz.
  const double TICKS_TO_NS = 1.0 / (4.0 * EclConfiguration::m_rf) * 1e3;


  vector<float> Crate_time_ns(52, 0.0); /**< vector derived from DB object */

  // Make a crate time offset vector with an entry per crate (instead of per crystal) and convert from ADC counts to ns.
  for (int crysID = 1; crysID <= 8736; crysID++) {
    int crateID_temp = crystalMapper->getCrateID(crysID);
    Crate_time_ns[crateID_temp - 1] = m_CrateTime[crysID] * TICKS_TO_NS;
  }



  /** Store the crystal cell id of those being used as the reference crystals for ts.
      One crystal per crate is defined as having ts=0.  This histogram only keeps
      track of the crystal id, not the crate id.  The talg can figure out to which
      crate to associate the crystal.
  */
  for (int crateID_temp = 1; crateID_temp <= 52; crateID_temp++) {
    getObjectPtr<TH1F>("refCrysIDzeroingCrate")->Fill(m_RefCrystalsCalib[crateID_temp - 1] + 0.001);
  }


  /** Record the input database constants */
  for (int crysID = 1; crysID <= 8736; crysID++) {
    getObjectPtr<TH1F>("TsDatabase")->SetBinContent(crysID + 0.001, m_PreviousCrystalTime[crysID - 1]);
    getObjectPtr<TH1F>("TsDatabaseUnc")->SetBinContent(crysID + 0.001, m_PreviousCrystalTimeUnc[crysID - 1]);
    getObjectPtr<TH1F>("TcrateDatabase")->SetBinContent(crysID + 0.001, m_CrateTime[crysID - 1]);
    getObjectPtr<TH1F>("TcrateUncDatabase")->SetBinContent(crysID + 0.001, m_CrateTimeUnc[crysID - 1]);
  }
  if (m_storeCalib) {
    B2INFO("ECLBhabhaTCollector:: ECLCrystalTimeOffset from the database information:"
           << LogVar("IoV", m_PreviousCrystalTimeDB.getIoV())
           << LogVar("Checksum", m_PreviousCrystalTimeDB.getChecksum()));
    B2INFO("First event so print out previous ts values");
    for (int crysID = 1; crysID <= 8736; crysID++) {
      B2INFO("cid = " << crysID << ", Ts previous = " << m_PreviousCrystalTime[crysID - 1]);
    }
    m_storeCalib = false;
  }




  for (int crateID_temp = 1; crateID_temp <= 52; crateID_temp++) {
    getObjectPtr<TH1F>("tcrateDatabase_ns")->SetBinContent(crateID_temp + 0.001, Crate_time_ns[crateID_temp - 1]);
  }

  // Use a histogram with only one bin as a counter to know the number of times the database histograms were filled.
  //    This is mostly useful for the talg when running over multiple runs and trying to read ts values.
  getObjectPtr<TH1I>("databaseCounter")->SetBinContent(1, 1);



  // Save what CDC event t0 correction was applied
  getObjectPtr<TH1F>("CDCEventT0Correction")->SetBinContent(1, m_hadronEventT0_TO_bhabhaEventT0_correction);




  /* Getting the event t0 using the full event t0 rather than from the CDC specifically */
  double evt_t0 = -1;
  double evt_t0_unc = -1;
  double evt_t0_ECL_closestCDC = -1;
  double evt_t0_ECL_minChi2 = -1;

  // Determine if there is an event t0 to use and then extract the information about it
  if (!m_eventT0.isValid()) {
    //cout << "event t0 not valid\n";
    return;
  } else if (!m_eventT0->hasTemporaryEventT0(Const::EDetector::CDC)) {
    //cout << "no event t0\n";
    return;
  } else {
    // Event has a t0 from CDC
    cutIndexPassed++;
    getObjectPtr<TH1F>("cutflow")->Fill(cutIndexPassed);
    B2DEBUG(22, "Cutflow: Event t0 exists: index = " << cutIndexPassed);


    // Get event t0 from CDC.  We don't want event t0 from ECL as we are calibrating the ECL wrt the more accurately measured time measurements of the time.  Start with the CDC since it has an event t0 but in the future we may switch to the TOP detector.
    // Based on the information from Thomas Hauth <Thomas.Hauth@kit.edu> (leaving physics) we should take the last event t0 in the list of event t0's from the CDC as the later event t0 measurements are calculated in slower but more accurate ways.
    vector<EventT0::EventT0Component> evt_t0_list = m_eventT0->getTemporaryEventT0s(Const::EDetector::CDC);
    evt_t0 = evt_t0_list.back().eventT0;   // time value
    evt_t0_unc = evt_t0_list.back().eventT0Uncertainty;   // uncertainty on event t0


    // Correct the CDC event t0 value for the bhabha bias
    evt_t0 = evt_t0 + m_hadronEventT0_TO_bhabhaEventT0_correction;   // Bias not yet fixed in CDC t0 reco.


    // Get the ECL event t0 for comparison - validations
    if (m_eventT0->hasTemporaryEventT0(Const::EDetector::ECL)) {
      vector<EventT0::EventT0Component> evt_t0_list_ECL = m_eventT0->getTemporaryEventT0s(Const::EDetector::ECL);


      double smallest_CDC_ECL_t0_diff = fabs(evt_t0_list_ECL[0].eventT0 - evt_t0);
      int smallest_CDC_ECL_t0_diff_idx = 0;
      for (long unsigned int ECLi = 0; ECLi < evt_t0_list_ECL.size(); ECLi++) {
        double tempt_ECL_t0 = evt_t0_list_ECL[ECLi].eventT0;
        if (fabs(tempt_ECL_t0 - evt_t0) < smallest_CDC_ECL_t0_diff) {
          smallest_CDC_ECL_t0_diff = fabs(tempt_ECL_t0 - evt_t0);
          smallest_CDC_ECL_t0_diff_idx = ECLi;
        }
      }

      evt_t0_ECL_closestCDC = evt_t0_list_ECL[smallest_CDC_ECL_t0_diff_idx].eventT0;   // time value
      B2DEBUG(26, "evt_t0_ECL_closestCDC = " << evt_t0_ECL_closestCDC);



      double smallest_ECL_t0_minChi2 = evt_t0_list_ECL[0].quality;
      int smallest_ECL_t0_minChi2_idx = 0;

      B2DEBUG(26, "evt_t0_list_ECL[0].quality = " << evt_t0_list_ECL[0].quality
              << ", with ECL event t0 = " << evt_t0_list_ECL[0].eventT0);

      for (long unsigned int ECLi = 0; ECLi < evt_t0_list_ECL.size(); ECLi++) {
        B2DEBUG(26, "evt_t0_list_ECL[" << ECLi << "].quality = " << evt_t0_list_ECL[ECLi].quality
                << ", with ECL event t0 = " <<
                evt_t0_list_ECL[ECLi].eventT0);
        if (evt_t0_list_ECL[ECLi].quality < smallest_ECL_t0_minChi2) {
          smallest_ECL_t0_minChi2 = evt_t0_list_ECL[ECLi].quality;
          smallest_ECL_t0_minChi2_idx = ECLi;
        }
      }

      evt_t0_ECL_minChi2 = evt_t0_list_ECL[smallest_ECL_t0_minChi2_idx].eventT0;   // time value

      B2DEBUG(26, "evt_t0_ECL_minChi2 = " << evt_t0_ECL_minChi2);
      B2DEBUG(26, "smallest_ECL_t0_minChi2_idx = " << smallest_ECL_t0_minChi2_idx);
    }
  }



  /* Determine the energies for each of the crystals since this isn't naturally connected to the cluster.
     Also determine the indexing of the ecl cal digits and the ecl digits
     Taken from Chris's ec/modules/eclGammaGammaECollector   */

  // Resize vectors
  m_EperCrys.resize(8736);
  m_eclCalDigitID.resize(8736);
  m_eclDigitID.resize(8736);


  int idx = 0;
  for (auto& eclCalDigit : m_eclCalDigitArray) {
    int tempCrysID = eclCalDigit.getCellId() - 1;
    m_EperCrys[tempCrysID] = eclCalDigit.getEnergy();
    m_eclCalDigitID[tempCrysID] = idx;
    idx++;
  }

  idx = 0;
  for (auto& eclDigit : m_eclDigitArray) {
    int tempCrysID = eclDigit.getCellId() - 1;
    m_eclDigitID[tempCrysID] = idx;
    idx++;
  }




  //---------------------------------------------------------------------
  //..Some utilities
  ClusterUtils cUtil;
  const TVector3 clustervertex = cUtil.GetIPPosition();
  PCmsLabTransform boostrotate;

  //---------------------------------------------------------------------
  //..Track properties, including 2 maxp tracks. Use pion (211) mass hypothesis,
  //     which is the only particle hypothesis currently available???
  double maxp[2] = {0., 0.};
  int maxiTrk[2] = { -1, -1};
  int nTrkAll = tracks.getEntries();

  int nTrkLoose = 0; /**< number of loose tracks */
  int nTrkTight = 0; /**< number of tight tracks */

  /* Loop over all the tracks to define the tight and loose selection tracks.
     We will select events with only 2 tight tracks and no additional loose tracks.
     Tight tracks are a subset of looses tracks. */
  for (int iTrk = 0; iTrk < nTrkAll; iTrk++) {
    // Get track biasing towards the particle being a pion based on what particle types
    // are used for reconstruction at this stage.
    const TrackFitResult* tempTrackFit = tracks[iTrk]->getTrackFitResultWithClosestMass(Const::pion);
    if (not tempTrackFit) {continue;}

    // Collect track info to be used for categorizing
    short charge = tempTrackFit->getChargeSign();
    double z0 = tempTrackFit->getZ0();
    double d0 = tempTrackFit->getD0();
    int nCDChits = tempTrackFit->getHitPatternCDC().getNHits();
    double p = tempTrackFit->getMomentum().Mag();

    // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
    //== Save debug TTree with detailed information if necessary.
    m_tree_d0 = d0;
    m_tree_z0 = z0;
    m_tree_p  = p;
    m_charge = charge;
    m_tree_nCDChits = nCDChits;

    if (m_saveTree) {
      m_dbgTree_tracks->Fill();
    }
    //<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


    /* Test if loose track  */

    // d0 and z0 cuts
    if (fabs(d0) > m_looseTrkD0) {
      continue;
    }
    if (fabs(z0) > m_looseTrkZ0) {
      continue;
    }
    // Number of hits in the CDC
    if (nCDChits < 1) {
      continue;
    }
    nTrkLoose++;



    /* Test if the loose track is also a tight track */

    // Number of hits in the CDC
    if (nCDChits < 20) {
      continue;
    }


    // d0 and z0 cuts
    if (fabs(d0) > m_tightTrkD0) {
      continue;
    }
    if (fabs(z0) > m_tightTrkZ0) {
      continue;
    }
    nTrkTight++;

    // Sorting of tight tracks.  Not really required as we only want two tight tracks (at the moment) but okay.
    //..Find the maximum p negative [0] and positive [1] tracks
    int icharge = 0;
    if (charge > 0) {icharge = 1;}
    if (p > maxp[icharge]) {
      maxp[icharge] = p;
      maxiTrk[icharge] = iTrk;
    }

  }
  /* After that last section the numbers of loose and tight tracks are known as well as the
     index of the loose tracks that have the highest p negatively charged and highest p positively
     charged tracks as measured in the centre of mass frame */


  if (nTrkTight != 2) {
    return;
  }
  // There are exactly two tight tracks
  cutIndexPassed++;
  getObjectPtr<TH1F>("cutflow")->Fill(cutIndexPassed);
  B2DEBUG(22, "Cutflow: Two tight tracks: index = " << cutIndexPassed);


  if (nTrkLoose != 2) {
    return;
  }
  // There are exactly two loose tracks as well, i.e. no additional loose tracks
  cutIndexPassed++;
  getObjectPtr<TH1F>("cutflow")->Fill(cutIndexPassed);
  B2DEBUG(22, "Cutflow: No additional loose tracks: index = " << cutIndexPassed);

  /* Determine if the two tracks have the opposite electric charge.
     We know this because the track indices stores the max pt track in [0] for negatively charged track
     and [1] fo the positively charged track.  If both are filled then both a negatively charged
     and positively charged track were found.   */
  bool oppositelyChargedTracksPassed = maxiTrk[0] != -1  &&  maxiTrk[1] != -1;
  if (!oppositelyChargedTracksPassed) {
    return;
  }
  // The two tracks have the opposite electric charges.
  cutIndexPassed++;
  getObjectPtr<TH1F>("cutflow")->Fill(cutIndexPassed);
  B2DEBUG(22, "Cutflow: Oppositely charged tracks: index = " << cutIndexPassed);




  //---------------------------------------------------------------------
  /* Determine associated energy clusters to each of the two tracks.  Sum the energies of the
     multiple clusters to each track and find the crystal with the maximum energy within all
     the sets of clusters associated to the tracks*/
  double trkEClustLab[2] = {0., 0.};
  double trkEClustCOM[2] = {0., 0.};
  double trkpLab[2];
  double trkpCOM[2];
  TLorentzVector trkp4Lab[2];
  TLorentzVector trkp4COM[2];

  // Index of the cluster and the crystal that has the highest energy crystal for the two tracks
  int crysIDMax[2] = { -1, -1 };
  double crysEMax[2] = { -1, -1 };
  double crysE2Max[2] = { -1, -1 };
  int numClustersPerTrack[2] = { 0, 0 };

  double clusterTime[2] = {0, 0};

  double E_DIV_p[2];

  vector<double> time_ECLCaldigits_bothClusters;
  vector<int> cid_ECLCaldigits_bothClusters;
  vector<double> E_ECLCaldigits_bothClusters;
  vector<double> amp_ECLDigits_bothClusters;
  vector<int> chargeID_ECLCaldigits_bothClusters;

  for (int icharge = 0; icharge < 2; icharge++) {
    if (maxiTrk[icharge] > -1) {
      B2DEBUG(22, "looping over the 2 max pt tracks");

      const TrackFitResult* tempTrackFit = tracks[maxiTrk[icharge]]->getTrackFitResultWithClosestMass(Const::pion);
      if (not tempTrackFit) {continue;}
      trkp4Lab[icharge] = tempTrackFit->get4Momentum();
      trkp4COM[icharge] = boostrotate.rotateLabToCms() * trkp4Lab[icharge];
      trkpLab[icharge] = trkp4Lab[icharge].Rho();
      trkpCOM[icharge] = trkp4COM[icharge].Rho();


      /* For each cluster associated to the current track, sum up the energies to get the total
         energy of all clusters associated to the track and find which crystal has the highest
         energy from all those clusters*/
      auto eclClusterRelationsFromTracks = tracks[maxiTrk[icharge]]->getRelationsTo<ECLCluster>();
      for (unsigned int clusterIdx = 0; clusterIdx < eclClusterRelationsFromTracks.size(); clusterIdx++) {

        B2DEBUG(22, "Looking at clusters.  index = " << clusterIdx);
        auto cluster = eclClusterRelationsFromTracks[clusterIdx];
        bool goodClusterType = false;

        if (cluster->hasHypothesis(Belle2::ECLCluster::EHypothesisBit::c_nPhotons)) {
          trkEClustLab[icharge] += cluster->getEnergy(Belle2::ECLCluster::EHypothesisBit::c_nPhotons);
          goodClusterType = true;
          numClustersPerTrack[icharge]++;
        }

        if (goodClusterType) {

          clusterTime[icharge] = cluster->getTime();

          auto eclClusterRelations = cluster->getRelationsTo<ECLCalDigit>("ECLCalDigits");

          // Find the crystal that has the largest energy
          for (unsigned int ir = 0; ir < eclClusterRelations.size(); ir++) {
            const auto calDigit = eclClusterRelations.object(ir);
            int tempCrysID = calDigit->getCellId() - 1;
            double tempE = m_EperCrys[tempCrysID];

            int eclDigitIndex = m_eclDigitID[tempCrysID];
            ECLDigit*    ecl_dig = m_eclDigitArray[eclDigitIndex];

            // for the max E crystal
            if (tempE > crysEMax[icharge]) {
              // Set 2nd highest E crystal to the info from the highest E crystal
              crysE2Max[icharge] = crysEMax[icharge];
              // Set the highest E crystal to the current crystal
              crysEMax[icharge] = tempE;
              crysIDMax[icharge] = tempCrysID;
            }
            // for the 2nd highest E crystal
            if (tempE > crysE2Max[icharge] && tempCrysID != crysIDMax[icharge]) {
              crysE2Max[icharge] = tempE;
            }

            // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
            // If we drop the information about the second highest energy crystal we could use
            //    m_eclClusterArray[ic]->getMaxECellId()

            B2DEBUG(26,  "calDigit(ir" << ir << ") time = " << calDigit->getTime() << "ns , with E = " << tempE << " GeV");
            time_ECLCaldigits_bothClusters.push_back(calDigit->getTime());
            cid_ECLCaldigits_bothClusters.push_back(tempCrysID);
            E_ECLCaldigits_bothClusters.push_back(tempE);
            amp_ECLDigits_bothClusters.push_back(ecl_dig->getAmp());
            chargeID_ECLCaldigits_bothClusters.push_back(icharge);

          }
        }
      }
      trkEClustCOM[icharge] = trkEClustLab[icharge] * trkpCOM[icharge] / trkpLab[icharge];

      // Check both electrons to see if their cluster energy / track momentum is good.
      // The Belle II physics book shows that this is the main way of separating electrons from other particles
      // Done in the centre of mass reference frame although I believe E/p is invariant under a boost.
      E_DIV_p[icharge] = trkEClustCOM[icharge] / trkpCOM[icharge];

    }
  }
  /* At the end of this section the 3-momenta magnitudes and the cluster energies are known
     for the two saved track indices for both the lab and COM frames.
     The crystal with the maximum energy, one associated to each track, is recorded*/



  //=== Check each crystal in the processed event and fill histogram.

  int numCrystalsPassingCuts = 0;

  int crystalIDs[2] = { -1, -1};
  int crateIDs[2] = { -1, -1};
  double ts_prevCalib[2] = { -1, -1};
  double tcrate_prevCalib[2] = { -1, -1};
  double times_noPreviousCalibrations[2] = { -1, -1};
  bool crystalCutsPassed[2] = {false, false};
  double crystalEnergies[2] = { -1, -1};
  double crystalEnergies2[2] = { -1, -1};

  for (int iCharge = 0; iCharge < 2; iCharge++) {
    int crystal_idx = crysIDMax[iCharge];
    int eclCalDigitIndex = m_eclCalDigitID[crystal_idx];
    int eclDigitIndex = m_eclDigitID[crystal_idx];

    ECLDigit*    ecl_dig = m_eclDigitArray[eclDigitIndex];
    ECLCalDigit* ecl_cal = m_eclCalDigitArray[eclCalDigitIndex];

    //== Check whether specific ECLDigits should be excluded.

    auto en = ecl_cal->getEnergy();
    auto amplitude = ecl_dig->getAmp();
    crystalEnergies[iCharge] = en;

    int cid   = ecl_dig->getCellId();
    double time  = ecl_dig->getTimeFit() * TICKS_TO_NS - evt_t0;

    // Offset time by electronics calibration and flight time calibration.
    time -= m_ElectronicsTime[cid - 1] * TICKS_TO_NS;
    time -= m_FlightTime[cid - 1];


    // Apply the time walk correction: time shift as a function of the amplitude corrected by the electronics calibration.
    //    The electronics calibration also accounts for crystals that have a dead pre-amp and thus half the normal amplitude.
    double energyTimeShift = m_ECLTimeUtil->energyDependentTimeOffsetElectronic(amplitude * m_Electronics[cid - 1]) * TICKS_TO_NS;

    B2DEBUG(29, "cellid = " << cid << ", amplitude = " << amplitude << ", time before t(E) shift = " << time <<
            ", t(E) shift = " << energyTimeShift << " ns");
    time -= energyTimeShift;


    // Cell ID should be within specified range.
    if (cid < m_minCrystal || cid > m_maxCrystal) continue;

    // Absolute time should be in specified range condition.
    if (fabs(time) > m_timeAbsMax) continue;

    // Fit quality flag -- choose only events with best fit quality
    if (ecl_dig->getQuality() != 0) continue;

    //== Save time and crystal information.  Fill plot after both electrons are tested
    crystalIDs[iCharge] = cid;
    crateIDs[iCharge] = crystalMapper->getCrateID(ecl_cal->getCellId());


    ts_prevCalib[iCharge] = m_PreviousCrystalTime[cid - 1] * TICKS_TO_NS;
    tcrate_prevCalib[iCharge] = m_CrateTime[cid - 1] * TICKS_TO_NS;
    times_noPreviousCalibrations[iCharge] = time;


    B2DEBUG(26, "iCharge = " << iCharge);
    B2DEBUG(26, "crateIDs[iCharge] = " << crateIDs[iCharge]);
    B2DEBUG(26, "times_noPreviousCalibrations[iCharge] = " << times_noPreviousCalibrations[iCharge]);
    B2DEBUG(26, "tcrate_prevCalib[iCharge] = " << tcrate_prevCalib[iCharge]);
    B2DEBUG(26, "ts_prevCalib[iCharge] = " << ts_prevCalib[iCharge]);


    crystalCutsPassed[iCharge] = true;


    // For second most energetic energy crystal
    crystalEnergies2[iCharge] = crysE2Max[iCharge];


//  >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> Tree saving
    //== Save debug TTree with detailed information if necessary.
    m_tree_cid      = ecl_dig->getCellId();
    m_tree_amp      = ecl_dig->getAmp();
    m_tree_en       = en;
    m_tree_E1Etot   = en / trkEClustLab[iCharge];
    m_tree_E1E2     = en / crystalEnergies2[iCharge];
    m_tree_E1p      = en / trkpLab[iCharge];
    m_tree_timetsPreviousTimeCalibs = time - ts_prevCalib[iCharge] - tcrate_prevCalib[iCharge];
    m_tree_timeF    = ecl_dig->getTimeFit() * TICKS_TO_NS;
    m_tree_time     = time;
    m_tree_quality  = ecl_dig->getQuality();
    m_tree_t0       = evt_t0;
    m_tree_t0_unc   = evt_t0_unc;
    m_E_DIV_p       = E_DIV_p[iCharge];
    m_tree_evtNum  = m_EventMetaData->getEvent();
    m_crystalCrate  = crystalMapper->getCrateID(ecl_cal->getCellId());
    m_runNum        = m_EventMetaData->getRun();

    if (m_saveTree) {
      m_dbgTree_electrons->Fill();
    }
//  <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< Tree saving

    // Fill histogram with information about maximum energy crystal energy fraction
    getObjectPtr<TH1F>("maxEcrsytalEnergyFraction")->Fill(en / trkEClustLab[iCharge]);


  }



  // Check both electrons to see if their cluster energy / track momentum is good.
  // The Belle II physics book shows that this is the main way of separating electrons from other particles
  // Done in the centre of mass reference frame although I believe E/p is invariant under a boost.
  bool E_DIV_p_instance_passed[2] = {false, false};
  double E_DIV_p_CUT = 0.7;
  for (int icharge = 0; icharge < 2; icharge++) {
    E_DIV_p_instance_passed[icharge] = E_DIV_p[icharge] > E_DIV_p_CUT;
  }
  if (!E_DIV_p_instance_passed[0] || !E_DIV_p_instance_passed[1]) {
    return;
  }
  // E/p sufficiently large
  cutIndexPassed++;
  getObjectPtr<TH1F>("cutflow")->Fill(cutIndexPassed);
  B2DEBUG(22, "Cutflow: E_i/p_i > " << E_DIV_p_CUT  << ": index = " << cutIndexPassed);



  // Start of cuts on both the combined system of tracks and energy clusters

  double invMassTrk = (trkp4Lab[0] + trkp4Lab[1]).M();
  double invMass_CUT = 0.9;
  m_massInvTracks = invMassTrk;   // invariant mass of the two tracks

//  >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> Tree saving
  if (m_saveTree) {
    m_dbgTree_event->Fill();
  }
//  <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< Tree saving

  bool invMassCutsPassed = invMassTrk > (invMass_CUT * boostrotate.getCMSEnergy());
  if (!invMassCutsPassed) {
    return;
  }
  // Invariable mass of the two tracks are above the minimum
  cutIndexPassed++;
  getObjectPtr<TH1F>("cutflow")->Fill(cutIndexPassed);
  B2DEBUG(22, "Cutflow: m(track 1+2) > " << invMass_CUT << "*E_COM = " << invMass_CUT << " * " << boostrotate.getCMSEnergy() <<
          " : index = " << cutIndexPassed);



  //== Fill output histogram.
  for (int iCharge = 0; iCharge < 2; iCharge++) {
    if (crystalCutsPassed[iCharge]) {
      getObjectPtr<TH2F>("TimevsCrysPrevCrateCalibPrevCrystCalib")->Fill((crystalIDs[iCharge]) + 0.001,
          times_noPreviousCalibrations[iCharge] - ts_prevCalib[iCharge] - tcrate_prevCalib[iCharge] , 1);
      getObjectPtr<TH2F>("TimevsCratePrevCrateCalibPrevCrystCalib")->Fill((crateIDs[iCharge]) + 0.001,
          times_noPreviousCalibrations[iCharge]  - ts_prevCalib[iCharge] - tcrate_prevCalib[iCharge], 1);
      getObjectPtr<TH2F>("TimevsCrysNoCalibrations")->Fill((crystalIDs[iCharge]) + 0.001, times_noPreviousCalibrations[iCharge], 1);
      getObjectPtr<TH2F>("TimevsCrateNoCalibrations")->Fill((crateIDs[iCharge]) + 0.001, times_noPreviousCalibrations[iCharge], 1);
      getObjectPtr<TH2F>("TimevsCrysPrevCrateCalibNoCrystCalib")->Fill((crystalIDs[iCharge]) + 0.001,
          times_noPreviousCalibrations[iCharge] - tcrate_prevCalib[iCharge], 1);
      getObjectPtr<TH2F>("TimevsCrateNoCrateCalibPrevCrystCalib")->Fill((crateIDs[iCharge]) + 0.001,
          times_noPreviousCalibrations[iCharge]  - ts_prevCalib[iCharge] , 1);

      // Record number of crystals used from the event.  Should be exactly two.
      numCrystalsPassingCuts++;

    }
  }


  // Change cutflow method for this bit ... don't call return because we used to call the hadron cluster stuff afterwards
  //
  if (crystalCutsPassed[0] || crystalCutsPassed[1]) {
    // At least one ECL crystal time and quality cuts passed
    cutIndexPassed++;
    getObjectPtr<TH1F>("cutflow")->Fill(cutIndexPassed);
    B2DEBUG(22, "Cutflow: At least one crystal time and quality cuts passed: index = " << cutIndexPassed);

    getObjectPtr<TH1F>("numCrystalEntriesPerEvent")->Fill(numCrystalsPassingCuts);
  }


  // Save final information to the tree after all cuts are applied
  for (int iCharge = 0; iCharge < 2; iCharge++) {
    if (crystalCutsPassed[iCharge]) {
      //  >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> Tree saving
      m_tree_evtNum  = m_EventMetaData->getEvent();
      m_tree_cid      = crystalIDs[iCharge];
      //m_tree_time     = times[iCharge];
      m_tree_time     = times_noPreviousCalibrations[iCharge];
      m_crystalCrate  = crateIDs[iCharge];
      m_runNum        = m_EventMetaData->getRun();
      m_tree_en       = crystalEnergies[iCharge];  // for studies of ts as a function of energy
      m_tree_E1Etot   = crystalEnergies[iCharge] / trkEClustLab[iCharge];
      m_tree_E1E2     = crystalEnergies[iCharge] / crystalEnergies2[iCharge];
      m_tree_E1p      = crystalEnergies[iCharge] / trkpLab[iCharge];
      m_tree_timetsPreviousTimeCalibs = times_noPreviousCalibrations[iCharge] - ts_prevCalib[iCharge] - tcrate_prevCalib[iCharge];
      m_tree_t0       = evt_t0;
      m_tree_t0_ECLclosestCDC   = evt_t0_ECL_closestCDC;
      m_tree_t0_ECL_minChi2   = evt_t0_ECL_minChi2;
      m_tree_tClust = clusterTime[iCharge];

      m_massInvTracks = invMassTrk;   // This is probably already set but I'll set it again anyways just so that it is clear

      if (m_saveTree) {
        m_dbgTree_allCuts->Fill();
      }
      //  <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< Tree saving
    }
  }




  if (crystalCutsPassed[0]       &&  crystalCutsPassed[1]        &&
      numClustersPerTrack[0] == 1  &&  numClustersPerTrack[1] == 1) {
    m_tree_enNeg = trkEClustLab[0];
    m_tree_enPlus = trkEClustLab[1];
    m_tree_tClustNeg = clusterTime[0];
    m_tree_tClustPos = clusterTime[1];
    m_tree_maxEcrystPosClust = times_noPreviousCalibrations[0] - ts_prevCalib[0] - tcrate_prevCalib[0];
    m_tree_maxEcrystNegClust = times_noPreviousCalibrations[1] - ts_prevCalib[1] - tcrate_prevCalib[1];
    m_tree_t0       = evt_t0;
    m_tree_t0_ECLclosestCDC   = evt_t0_ECL_closestCDC;
    m_tree_t0_ECL_minChi2   = evt_t0_ECL_minChi2;

    //  >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> Tree saving
    if (m_saveTree) {
      m_dbgTree_evt_allCuts->Fill();
    }
    //  <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< Tree saving
  }


  B2DEBUG(26, "m_tree_maxEcrystPosClust + evt_t0 = " << m_tree_maxEcrystPosClust + evt_t0);
  B2DEBUG(26, "m_tree_maxEcrystNegClust + evt_t0 = " << m_tree_maxEcrystNegClust + evt_t0);
  B2DEBUG(26, "CDC evt_t0 = " << evt_t0);
  B2DEBUG(26, "ECL min chi2 even t0, m_tree_t0_ECL_minChi2 = " << m_tree_t0_ECL_minChi2);



  for (long unsigned int digit_i = 0; digit_i < time_ECLCaldigits_bothClusters.size(); digit_i++) {
    m_runNum        = m_EventMetaData->getRun();
    m_tree_evtNum  = m_EventMetaData->getEvent();
    m_tree_ECLCalDigitTime  = time_ECLCaldigits_bothClusters[digit_i];
    m_tree_ECLCalDigitE = E_ECLCaldigits_bothClusters[digit_i];
    m_tree_ECLDigitAmplitude = amp_ECLDigits_bothClusters[digit_i];
    m_tree_t0       = evt_t0;
    m_tree_t0_ECLclosestCDC   = evt_t0_ECL_closestCDC;
    m_tree_t0_ECL_minChi2   = evt_t0_ECL_minChi2;
    m_tree_timetsPreviousTimeCalibs = times_noPreviousCalibrations[chargeID_ECLCaldigits_bothClusters[digit_i]] -
                                      ts_prevCalib[chargeID_ECLCaldigits_bothClusters[digit_i]] -
                                      tcrate_prevCalib[chargeID_ECLCaldigits_bothClusters[digit_i]];
    m_tree_cid = cid_ECLCaldigits_bothClusters[digit_i];

    //  >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> Tree saving
    if (m_saveTree) {
      m_dbgTree_crys_allCuts->Fill();
    }
    //  <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< Tree saving

  }


  B2DEBUG(26, "This was for event number = " << m_tree_evtNum);

}


