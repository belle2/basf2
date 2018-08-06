/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Mikhail Remnev                                           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

/**********************************************/
// Uncomment this line if you want to use ECLBhabhaTCollector for release-01-02-09
// #define ECL_BHABHA_TCOLLECTOR_RELEASE010209
/**********************************************/

#include <ecl/modules/eclBhabhaTCollector/ECLBhabhaTCollectorModule.h>
#include <framework/dataobjects/EventMetaData.h>
#include <framework/gearbox/Const.h>
#include <ecl/dbobjects/ECLCrystalCalib.h>
#include <ecl/dataobjects/ECLDigit.h>
#include <ecl/dataobjects/ECLCalDigit.h>
#include <ecl/dataobjects/ECLTrig.h>
#include <mdst/dataobjects/ECLCluster.h>
#include <mdst/dataobjects/Track.h>
#include <tracking/dataobjects/RecoTrack.h>

#include <TH2F.h>
#include <TTree.h>
#include <TFile.h>

using namespace Belle2;
using namespace ECL;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(ECLBhabhaTCollector)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

ECLBhabhaTCollectorModule::ECLBhabhaTCollectorModule() : CalibrationCollectorModule(),
  m_ElectronicsTime("ECLCrystalElectronicsTime"),
  m_FlightTime("ECLCrystalFlightTime"),
  m_dbg_tree(0),
  m_tree_evtn(0)
{
  setDescription("This module generates sum of all event times per crystal");

  addParam("minEn", m_minEn,
           "Events with energy lower than this value are excluded", 0.01);
  addParam("maxEn", m_maxEn,
           "Events with energy higher than this value are excluded", 10.0);

  addParam("maxTotalEn", m_maxTotalEn,
           "Events with energy sum higher than this "
           "value are excluded", 1000.0);
  addParam("timeAbsMax", m_timeAbsMax, // (Time in ADC units)
           "Events with abs(getTimeFit) > m_timeAbsMax "
           "are excluded", (short)80);
  addParam("nentriesMax", m_nentriesMax, // (ECLDigit entries per event)
           "Events with ECLDigits.getEntries() > m_nentriesMax "
           "are excluded", 950);

  addParam("minCrystal", m_minCrystal,
           "First CellId to handle.", 1); // 1153 -- first crystal in barrel.
  addParam("maxCrystal", m_maxCrystal,
           "Last CellId to handle.", 8736);

  addParam("saveTree", m_saveTree,
           "If true, TTree 'tree' with more detailed event info is saved in "
           "the output file specified by HistoManager",
           false);

  addParam("weightedHist", m_weightedHist,
           "If true, time difference histogram is filled with weight "
           "min(energy*energy, 1 GeV)",
           true);

  // specify this flag if you need parallel processing
  setPropertyFlags(c_ParallelProcessingCertified);
}

ECLBhabhaTCollectorModule::~ECLBhabhaTCollectorModule()
{
}

void ECLBhabhaTCollectorModule::inDefineHisto()
{
  //=== Prepare TTree for debug output
  if (m_saveTree) {
    m_dbg_tree = new TTree("tree", "Debug data for bhabha time calibration");
    m_dbg_tree->Branch("evtn"   , &m_tree_evtn)   ->SetTitle("Event number");
    m_dbg_tree->Branch("cid"    , &m_tree_cid)    ->SetTitle("Cell ID, 1..8736");
    m_dbg_tree->Branch("amp"    , &m_tree_amp)    ->SetTitle("Amplitude, ADC units");
    m_dbg_tree->Branch("energy" , &m_tree_en)     ->SetTitle("Energy, GeV");
    m_dbg_tree->Branch("en_sum" , &en_sum)        ->SetTitle("Total energy, GeV");
    m_dbg_tree->Branch("time"   , &m_tree_time)   ->SetTitle("Time, ns");
    m_dbg_tree->Branch("quality", &m_tree_quality)->SetTitle("ECL FPGA fit quality, see Confluence article");
    m_dbg_tree->Branch("t0"     , &m_tree_t0)     ->SetTitle("T0, ns");
    m_dbg_tree->Branch("track"  , &m_tracks)      ->SetTitle("Number of good tracks");
    m_dbg_tree->Branch("track_chi2", m_track_chi2, "track_chi2[track]/D")->SetTitle("chi2 for tracks");
    m_dbg_tree->Branch("track_ndf" , m_track_ndf , "track_ndf[track]/D")->SetTitle("ndf for tracks");

    m_dbg_tree->SetAutoSave(10);
  }
}

void ECLBhabhaTCollectorModule::prepare()
{
  //=== MetaData
  StoreObjPtr<EventMetaData> evtMetaData;
  B2INFO("ECLBhabhaTCollector: Experiment = " << evtMetaData->getExperiment() <<
         "  run = " << evtMetaData->getRun());

  //=== Load preceding calibrations from the database

  //== Get expected energies and calibration constants from DB. Need to call
  //   hasChanged() for later comparison

  if (m_ElectronicsTime.hasChanged()) {
    ElectronicsTime = m_ElectronicsTime->getCalibVector();
  }
  if (m_FlightTime.hasChanged()) {
    FlightTime = m_FlightTime->getCalibVector();
  }

  //=== Create histograms and register them in the data store

  int nbins = m_timeAbsMax * 2;
  int max_t = m_timeAbsMax;
  int min_t = -m_timeAbsMax;

  auto TimevsCrys = new TH2F("TimevsCrys", "Time shifted by T0 vs crystal ID;crystal ID;Time (ns)",
                             8736, 0, 8736, nbins, min_t, max_t);
  registerObject<TH2F>("TimevsCrys", TimevsCrys);

  //=== Required data objects

  event_t0.isRequired();
  tracks.isRequired();
  ecl_digits.isRequired();
  ecl_cal_digits.isRequired();
}

void ECLBhabhaTCollectorModule::collect()
{
  //=== Check if DB objects have changed
  if (m_ElectronicsTime.hasChanged()) {
    B2FATAL("eclBhabhaTCollector: ElectronicsTime has changed");
  }
  if (m_FlightTime.hasChanged()) {
    B2FATAL("eclBhabhaTCollector: FlightTime has changed");
  }

  // TODO: Select by beam angular size?

  //=== Check if entire event should be excluded.

  // Check if number of entries satisfies module conditions.
  if (ecl_digits.getEntries() > m_nentriesMax) return;

  // Check if event t0 was determined for this event.
  if (!event_t0.isValid()) return;

#ifdef ECL_BHABHA_TCOLLECTOR_RELEASE010209
  // For release 01-02-09 use the code below to check T0 quality.

  // Skip event if we have no value of T0 from CDC.
  if (!event_t0->hasEventT0(Const::EDetector::CDC)) return;

  double t0 = event_t0->getEventT0(Const::EDetector::CDC);
#else
  // For other branches (master, specifically) this code is used to check T0 quality.

  auto t0_list = event_t0->getTemporaryEventT0s(Const::EDetector::CDC);

  // Skip event if we have no value of T0 from CDC.
  if (t0_list.size() == 0) return;

  double t0     = NAN;
  double t0_unc = NAN;

  for (auto& t0_comp : t0_list) {
    // Skip if EventT0 was based on ECL data.
    if (t0_comp.detectorSet.getIndex(Const::EDetector::ECL) != -1) {
      return;
    }

    // Skip if we already got lower uncertainty.
    // In case of (NAN < t0_comp.eventT0Uncertainty), this is always false.
    if (t0_unc < t0_comp.eventT0Uncertainty) continue;

    t0     = t0_comp.eventT0;
    t0_unc = t0_comp.eventT0Uncertainty;
  }

  if (std::isnan(t0)) return;
#endif

  //=== Get number of good tracks

  m_tracks = 0;
  for (int i = 0; i < tracks.getEntries(); i++) {
    auto reco_track = tracks[i]->getRelated<RecoTrack>();

    // Skip tracks that don't have related RecoTrack
    if (!reco_track) continue;
    // Skip tracks that don't have related ECLCluster
    if (!tracks[i]->getRelated<ECLCluster>()) continue;

    // Skip tracks where fit hasn't converged.
    if (!reco_track->getTrackFitStatus()->isFitConvergedFully()) continue;

    double chi2, ndf;
    chi2 = reco_track->getTrackFitStatus()->getChi2();
    ndf = reco_track->getTrackFitStatus()->getNdf();

    // Skip tracks with high chi2
    if (chi2 / ndf > 3) continue;

    if (m_tracks < 30) {
      if (m_saveTree) {
        m_track_chi2[m_tracks] = chi2;
        m_track_ndf [m_tracks] = ndf;
      }
      m_tracks++;
    }
  }

  // Skip events with less than 1 good track.
  if (m_tracks < 1) return;

  //=== Get total energy deposition in ECL for event

  en_sum = 0;
  for (ECLCalDigit& ecl_cal : ecl_cal_digits)
    en_sum += ecl_cal.getEnergy();

  // Skip events with total energy higher than maximum
  if (en_sum > m_maxTotalEn) return;

  //=== Check each crystal in the processed event and fill histogram.

  int cid;
  double time;

  // TODO: Save in ADC ticks -- that way we will have slightly better fitting.
  //
  // Conversion coefficient from ADC ticks to nanoseconds
  const double TICKS_TO_NS = 0.4931; // ns/clock

  for (int i = 0; i < ecl_digits.getEntries(); i++) {
    ECLDigit*    ecl_dig = ecl_digits[i];
    ECLCalDigit* ecl_cal = ecl_dig->getRelated<ECLCalDigit>();

    //== Check whether specific ECLDigits should be excluded.

    // Events should be calibrated.
    if (ecl_cal == 0) continue;

    auto en = ecl_cal->getEnergy();
    cid   = ecl_dig->getCellId();
    time  = ecl_dig->getTimeFit() * TICKS_TO_NS - t0;

    // Offset time by electronics calibration and flight time calibration.
    time += ElectronicsTime[cid - 1] * TICKS_TO_NS;
    time -= FlightTime[cid - 1];

    // Cell ID should be within specified range.
    if (cid < m_minCrystal || cid > m_maxCrystal) continue;
    // Absolute time should be in specified range condition.
    if (fabs(time) > m_timeAbsMax) continue;
    // Amplitude should be in specified range condition.
    if (en < m_minEn || en > m_maxEn) continue;

    // Fit quality flag -- choose only events with best fit quality
    if (ecl_dig->getQuality() != 0) continue;

    // TODO: Move these values to parameters.
    // Energy should either
    //  a. Total energy for event should be within specified interval.
    //  b. Be higher than 150 MeV for specific event.
    if (en_sum < 10.5 || en_sum > 11.5) {
      if (en < 0.15) continue;
    }

    //== Fill output histogram.

    float weight = 1.0;
    if (m_weightedHist) {
      weight = en < 1 ? en * en : 1;
    }
    getObjectPtr<TH2F>("TimevsCrys")->Fill((cid - 1) + 0.001, time, weight);

    //== Save debug TTree with detailed information if necessary.

    if (m_saveTree) {
      StoreObjPtr<EventMetaData> evtMetaData;

      m_tree_cid     = ecl_dig->getCellId();
      m_tree_amp     = ecl_dig->getAmp();
      m_tree_en      = en;
      m_tree_time    = ecl_dig->getTimeFit() * TICKS_TO_NS;
      m_tree_quality = ecl_dig->getQuality();
      m_tree_t0      = t0;
      m_tree_evtn    = evtMetaData->getEvent();

      m_dbg_tree->Fill();
    }
  }
}

