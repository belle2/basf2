/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

//THIS MODULE
#include <ecl/modules/eclDQM/eclDQM.h>

//Boost
#include <boost/format.hpp>
#include <boost/range/combine.hpp>

//FRAMEWORK
#include <framework/core/HistoModule.h>
#include <framework/dataobjects/EventMetaData.h>
#include <framework/gearbox/Unit.h>
#include <framework/logging/Logger.h>

//ECL
#include <ecl/dataobjects/ECLDigit.h>
#include <ecl/dataobjects/ECLCalDigit.h>
#include <ecl/dataobjects/ECLTrig.h>
#include <ecl/dataobjects/ECLDsp.h>
#include <ecl/utility/ECLChannelMapper.h>
#include <ecl/geometry/ECLGeometryPar.h>

//TRG
#include <mdst/dataobjects/TRGSummary.h>

//ROOT
#include <TProfile.h>
#include <TH1F.h>
#include <TH2F.h>
#include <TDirectory.h>

//STL
#include <iostream>
#include <iterator>
#include <cmath>
#include <stdexcept>

//NAMESPACE(S)
using namespace Belle2;
using namespace ECL;

REG_MODULE(ECLDQM)

ECLDQMModule::ECLDQMModule()
  : HistoModule(),
    m_calibrationThrApsd("ECL_FPGA_StoreWaveform")
{
  //Set module properties.
  setDescription("ECL Data Quality Monitor");
  setPropertyFlags(c_ParallelProcessingCertified);  // specify parallel processing.

  m_WaveformOption = {"psd", "logic", "rand", "dphy", "other"};

  addParam("histogramDirectoryName", m_histogramDirectoryName,
           "histogram directory in ROOT file", std::string("ECL"));
  addParam("EnergyUpperThr", m_EnergyUpperThr, "Upper threshold of energy deposition in event, [GeV]", 20.0 * Belle2::Unit::GeV);
  addParam("HitThresholds", m_HitThresholds, "Thresholds to display hit occupancy, MeV", std::vector<double> {0, 5, 10, 50});
  addParam("TotalEnergyThresholds", m_TotalEnergyThresholds, "Thresholds to display total energy, MeV", std::vector<double> {0, 5, 7});
  addParam("TimingThresholds", m_TimingThresholds, "Thresholds (MeV) to display ECL timing", std::vector<double> {5, 10, 50});
  addParam("HitNumberUpperlimits", m_HitNumberUpperLimits,
           "Upper limit (# of hits) to display hit multiplicity", std::vector<double> {10000, 1000, 700, 200});
  addParam("WaveformOption", m_WaveformOption, "Option (all,psd,logic,rand,dphy,other) to display waveform flow",
           m_WaveformOption);
  addParam("DPHYTTYP", m_DPHYTTYP,
           "Flag to control trigger of delayed bhabha events; 0 - select events by 'bha_delay' trigger bit, 1 - select by TTYP_DPHY", false);
}

ECLDQMModule::~ECLDQMModule()
{
}


void ECLDQMModule::defineHisto()
{
  TDirectory* oldDir = gDirectory;

  // Create a separate histogram directory and cd into it.

  TDirectory* dirDAQ = dynamic_cast<TDirectory*>(oldDir->Get(m_histogramDirectoryName.c_str()));
  if (!dirDAQ) dirDAQ = oldDir->mkdir(m_histogramDirectoryName.c_str());
  dirDAQ->cd();

  //1D histograms creation.
  h_evtot = new TH1F("event", "Total event bank", 1, 0, 1);
  h_evtot->SetOption("LIVE");

  h_quality = new TH1F("quality", "Fit quality flag. 0-good, 1-integer overflow, 2-low amplitude, 3-bad chi2", 4, 0, 4);
  h_quality->GetXaxis()->SetTitle("Flag number");
  h_quality->GetYaxis()->SetTitle("ECL hits count");
  h_quality->SetFillColor(kPink - 4);
  h_quality->SetOption("LIVE");

  h_quality_other = new TH1F("quality_other", "Fit quality flag for unexpectedly saved waveforms", 4, 0, 4);
  h_quality_other->GetXaxis()->SetTitle("Flag number. 0-good,1-int overflow,2-low amplitude,3-bad chi2");
  h_quality_other->SetFillColor(kPink - 4);
  h_quality_other->SetOption("LIVE");

  h_bad_quality = new TH1F("bad_quality", "Fraction of hits with bad chi2 (qual=3) and E > 1 GeV vs Cell ID", 8736, 1, 8737);
  h_bad_quality->GetXaxis()->SetTitle("Cell ID");
  h_bad_quality->GetYaxis()->SetTitle("ECL hits count");
  h_bad_quality->SetOption("LIVE");

  h_trigtag1 = new TH1F("trigtag1", "Consistency b/w global event number and trigger tag. 0-good, 1-DQM error", 2, 0, 2);
  h_trigtag1->GetXaxis()->SetTitle("Flag number");
  h_trigtag1->GetYaxis()->SetTitle("Events count");
  h_trigtag1->SetDrawOption("hist");
  h_trigtag1->SetOption("LIVE");
  h_trigtag1->SetFillColor(kPink - 4);

  h_adc_hits = new TH1F("adc_hits", "Fraction of high-energy hits (E > 50 MeV)", 1001, 0, 1.001);
  h_adc_hits->GetXaxis()->SetTitle("Fraction");
  h_adc_hits->GetYaxis()->SetTitle("Events count");
  h_adc_hits->SetOption("LIVE");

  h_time_crate_Thr1GeV_large = new TH1F("time_crate_Thr1GeV_large",
                                        "Number of hits with timing outside #pm 100 ns per Crate ID (E > 1 GeV)",
                                        52, 1, 53);
  h_time_crate_Thr1GeV_large->GetXaxis()->SetTitle("Crate ID (same as ECLCollector ID)");
  h_time_crate_Thr1GeV_large->GetYaxis()->SetTitle("ECL hits count");
  h_time_crate_Thr1GeV_large->SetOption("LIVE");

  for (const auto& id : m_HitThresholds) {
    std::string h_name, h_title;
    h_name = str(boost::format("cid_Thr%1%MeV") % id);
    h_title = str(boost::format("Occupancy per Cell ID (E > %1% MeV)") % id);
    TH1F* h = new TH1F(h_name.c_str(), h_title.c_str(), 8736, 1, 8737);
    h->GetXaxis()->SetTitle("Cell ID");
    h->GetYaxis()->SetTitle("Occupancy (hits / events_count)");
    h->SetOption("LIVE");
    h_cids.push_back(h);
  }

  for (const auto& id : m_TotalEnergyThresholds) {
    std::string h_name, h_title;
    h_name = str(boost::format("edep_Thr%1%MeV") % id);
    h_title = str(boost::format("Total energy (thr = %1% MeV)") % id);
    TH1F* h = new TH1F(h_name.c_str(), h_title.c_str(), (int)(100 * m_EnergyUpperThr), 0, m_EnergyUpperThr);
    h->GetXaxis()->SetTitle("Energy, [GeV]");
    h->SetOption("LIVE");
    h_edeps.push_back(h);
  }

  for (const auto& id : m_TimingThresholds) {
    std::string h_bar_name, h_bar_title;
    std::string h_end_name, h_end_title;
    h_bar_name = str(boost::format("time_barrel_Thr%1%MeV") % id);
    h_bar_title = str(boost::format("Reconstructed time for ECL barrel (E > %1% MeV)") % id);
    h_end_name = str(boost::format("time_endcaps_Thr%1%MeV") % id);
    h_end_title = str(boost::format("Reconstructed time for ECL endcaps (E > %1% MeV)") % id);
    TH1F* h_time_barrel = new TH1F(h_bar_name.c_str(), h_bar_title.c_str(), 206, -1030, 1030);
    TH1F* h_time_endcap = new TH1F(h_end_name.c_str(), h_end_title.c_str(), 206, -1030, 1030);
    h_time_barrel->GetXaxis()->SetTitle("Time, [ns]");
    h_time_endcap->GetXaxis()->SetTitle("Time, [ns]");
    h_time_barrel->SetOption("LIVE");
    h_time_endcap->SetOption("LIVE");
    h_time_barrels.push_back(h_time_barrel);
    h_time_endcaps.push_back(h_time_endcap);
  }

  for (const auto& id : boost::combine(m_HitThresholds, m_HitNumberUpperLimits)) {
    double id1 = 0, id2 = 0;
    boost::tie(id1, id2) = id;
    std::string h_name, h_title;
    h_name = str(boost::format("ncev_Thr%1%MeV") % id1);
    h_title = str(boost::format("Number of hits in event (E > %1% MeV)") % id1);
    TH1F* h = new TH1F(h_name.c_str(), h_title.c_str(), id2, 0, id2);
    h->GetXaxis()->SetTitle("Number of hits");
    h->SetOption("LIVE");
    h_ncevs.push_back(h);
  }

  for (int i = 0; i < ECL_CRATES; i++) {
    int crate = i + 1;
    std::string h_name, h_title;
    h_name = str(boost::format("time_crate_%1%_Thr1GeV") % (crate));
    h_title = str(boost::format("Reconstructed time for ECL crate #%1% with E > 1 GeV") % (crate));
    TH1F* h = new TH1F(h_name.c_str(), h_title.c_str(), 400, -100, 100);
    h->GetXaxis()->SetTitle("Time [ns]");
    h->SetOption("LIVE");
    h_time_crate_Thr1GeV.push_back(h);
  }

  for (const auto& id : m_WaveformOption) {
    if (id != "all" && id != "psd" && id != "logic" && id != "rand" && id != "dphy" && id != "other")
      B2WARNING("Waveform Options are not correctly assigned. They must be 'all', 'psd', 'logic', 'rand', 'dphy', 'other'!");
    std::string h_title;
    std::string h_cell_name;
    if (id == "other") h_title = "Unexpectedly saved waveforms";
    if (id == "psd") h_title = "#frac{Saved}{Expected} waveforms for high-energy hits (E > 50 MeV)";
    if (id == "logic") h_title = "#frac{Saved}{Expected} waveforms for every 1000th event";
    if (id == "rand") h_title = "#frac{Saved}{Expected} waveforms for random trigger events";
    if (id == "dphy") h_title = "#frac{Saved}{Expected} waveforms for delayed bhabha (DPHY) events";
    if (id == "all") h_title = "#frac{Saved}{Expected} waveforms for all events";
    h_cell_name = str(boost::format("wf_cid_%1%") % (id));
    TH1F* h_cell = new TH1F(h_cell_name.c_str(), h_title.c_str(), 8736, 1, 8737);
    h_cell->GetXaxis()->SetTitle("Cell ID");
    h_cell->SetOption("LIVE");
    if (id == "psd") {
      h_cell_psd_norm = new TH1F("psd_cid", "Normalization to psd hits for cid", 8736, 1, 8737);
      h_cell_psd_norm->SetOption("LIVE");
    }
    if (id == "logic") {
      h_evtot_logic = new TH1F("event_logic", "Event bank for logic", 1, 0, 1);
      h_evtot_logic->SetOption("LIVE");
    }
    if (id == "rand") {
      h_evtot_rand = new TH1F("event_rand", "Event bank for rand", 1, 0, 1);
      h_evtot_rand->SetOption("LIVE");
    }
    if (id == "dphy") {
      h_evtot_dphy = new TH1F("event_dphy", "Event bank for dphy", 1, 0, 1);
      h_evtot_dphy->SetOption("LIVE");
    }
    h_cells.push_back(h_cell);
  }

  //2D histograms creation.

  h_trigtag2_trigid = new TH2F("trigtag2_trigid", "Internal data consistency vs crate. 0-good, 1-data corruption",
                               52, 1, 53, 11, -1, 10);
  h_trigtag2_trigid->GetXaxis()->SetTitle("Crate ID (same as ECLCollector ID)");
  h_trigtag2_trigid->GetYaxis()->SetTitle("Data consistency flag");
  h_trigtag2_trigid->SetOption("LIVE");

  h_pedmean_cellid = new TProfile("pedmean_cellid", "Pedestal vs Cell ID", 8736, 1, 8737);
  h_pedmean_cellid->GetXaxis()->SetTitle("Cell ID");
  h_pedmean_cellid->GetYaxis()->SetTitle("Ped. average (ADC units, #approx 0.05 MeV)");
  h_pedmean_cellid->SetOption("LIVE");

  h_pedrms_cellid = new TProfile("pedrms_cellid", "Pedestal stddev vs Cell ID",
                                 8736, 1, 8737);
  h_pedrms_cellid->GetXaxis()->SetTitle("Cell ID");
  h_pedrms_cellid->GetYaxis()->SetTitle("Ped. stddev (ADC units, #approx 0.05 MeV)");
  h_pedrms_cellid->SetOption("LIVE");

  h_pedrms_thetaid = new TProfile("pedrms_thetaid", "Pedestal stddev vs #theta ID",
                                  68, 0, 68);
  h_pedrms_thetaid->GetXaxis()->SetTitle("#theta ID (0-12=FWD, 59-67=BWD endcap)");
  h_pedrms_thetaid->GetYaxis()->SetTitle("Ped. stddev (ADC units, #approx 0.05 MeV)");
  h_pedrms_thetaid->SetOption("LIVE");

  h_trigtime_trigid = new TH2F("trigtime_trigid", "Trigger time vs Crate ID", 52, 1, 53, 145, 0, 145);
  h_trigtime_trigid->GetXaxis()->SetTitle("Crate ID (same as ECLCollector ID)");
  h_trigtime_trigid->GetYaxis()->SetTitle("Trigger time (only even, 0-142)");
  h_trigtime_trigid->SetOption("LIVE");

  //cd into parent directory.

  oldDir->cd();
}

void ECLDQMModule::initialize()
{
  REG_HISTOGRAM;   // required to register histograms to HistoManager.
  m_ECLDigits.isRequired();
  m_ECLCalDigits.isOptional();
  m_ECLTrigs.isOptional();
  m_ECLDsps.isOptional();
  m_l1Trigger.isOptional();

  if (!mapper.initFromDB()) B2FATAL("ECL DQM: Can't initialize eclChannelMapper");

  ecltot.resize(m_TotalEnergyThresholds.size());
  nhits.resize(m_HitNumberUpperLimits.size());

  m_geom = ECLGeometryPar::Instance();

  v_totalthrApsd.resize((m_calibrationThrApsd->getCalibVector()).size());
  for (size_t i = 0; i < v_totalthrApsd.size(); i++) v_totalthrApsd[i] = (int)(m_calibrationThrApsd->getCalibVector())[i];
}

void ECLDQMModule::beginRun()
{
  h_evtot->Reset();
  h_evtot_logic->Reset();
  h_evtot_rand->Reset();
  h_evtot_dphy->Reset();
  h_quality->Reset();
  h_quality_other->Reset();
  h_bad_quality->Reset();
  h_trigtag1->Reset();
  h_adc_hits->Reset();
  h_time_crate_Thr1GeV_large->Reset();
  h_cell_psd_norm->Reset();
  std::for_each(h_cids.begin(), h_cids.end(), [](auto & it) {it->Reset();});
  std::for_each(h_edeps.begin(), h_edeps.end(), [](auto & it) {it->Reset();});
  std::for_each(h_time_barrels.begin(), h_time_barrels.end(), [](auto & it) {it->Reset();});
  std::for_each(h_time_endcaps.begin(), h_time_endcaps.end(), [](auto & it) {it->Reset();});
  std::for_each(h_ncevs.begin(), h_ncevs.end(), [](auto & it) {it->Reset();});
  std::for_each(h_cells.begin(), h_cells.end(), [](auto & it) {it->Reset();});
  for (int i = 0; i < ECL_CRATES; i++) h_time_crate_Thr1GeV[i]->Reset();
  h_trigtag2_trigid->Reset();
  h_pedmean_cellid->Reset();
  h_pedrms_cellid->Reset();
  h_pedrms_thetaid->Reset();
  h_trigtime_trigid->Reset();
}

void ECLDQMModule::event()
{
  int trigtag1 = 0;
  int NDigits = 0;
  for (auto& value : ecltot) value = 0;
  for (auto& value : nhits) value = 0;
  bool bhatrig = false;

  if (m_l1Trigger.isValid() && m_DPHYTTYP) bhatrig = m_l1Trigger->getTimType() == TRGSummary::ETimingType::TTYP_DPHY;
  else if (m_l1Trigger.isValid() && !m_DPHYTTYP) {
    try { bhatrig = m_l1Trigger->testInput("bha_delay"); }
    catch (const std::exception&) { bhatrig = false; }
  }

  m_iEvent = -1;
  if (m_eventmetadata.isValid()) {
    if (m_eventmetadata->getErrorFlag() != 0x10) {
      m_iEvent = m_eventmetadata->getEvent();
      h_evtot->Fill(0);
      for (const auto& id : m_WaveformOption) {
        if (id == "logic" && m_iEvent % 1000 == 999) h_evtot_logic->Fill(0);
        if (id == "rand" && isRandomTrigger()) h_evtot_rand->Fill(0);
        if (id == "dphy" && bhatrig) h_evtot_dphy->Fill(0);
      }
    }
  }

  for (auto& aECLDigit : m_ECLDigits) {
    int i = aECLDigit.getCellId() - 1;
    h_quality->Fill(aECLDigit.getQuality());  //Fit quality histogram filling.
    if (aECLDigit.getAmp() > 2.e04 && aECLDigit.getQuality() == 3) h_bad_quality->Fill(aECLDigit.getCellId());
    if (aECLDigit.getAmp() >= (v_totalthrApsd[i] / 4 * 4)) NDigits ++;
    for (const auto& id : m_WaveformOption) {
      if (id != "psd") continue;
      else if (id == "psd" && (m_iEvent % 1000 == 999 ||
                               isRandomTrigger() ||
                               bhatrig ||
                               aECLDigit.getAmp() < (v_totalthrApsd[i] / 4 * 4))) continue;
      h_cell_psd_norm->Fill(aECLDigit.getCellId());
    }
  }

  for (auto& aECLTrig : m_ECLTrigs) {
    double itrg = aECLTrig.getTimeTrig();
    //trigger time conversion to acceptable units in range (0, ..., 142).
    //one trigger time clock corresponds to 0.567/144*1000 = 3.93 ns
    int tg = (int)itrg - 2 * ((int)itrg / 8);
    h_trigtime_trigid->Fill(aECLTrig.getTrigId(), tg); //Trigger time histogram filling.
    trigtag1 += aECLTrig.getTrigTag();
    h_trigtag2_trigid->Fill(aECLTrig.getTrigId(), aECLTrig.getTrigTagQualityFlag()); //Data consistency histogram filling.
  }

  if (m_ECLTrigs.getEntries() > 0) {
    int flagtag = 1;
    trigtag1 /= m_ECLTrigs.getEntries();
    int compar = (65535 & m_iEvent);
    if (compar == trigtag1) flagtag = 0;
    h_trigtag1->Fill(flagtag);  //Trigger tag flag #1 histogram filling.
  }

  for (auto& aECLCalDigit : m_ECLCalDigits) {
    int cid        = aECLCalDigit.getCellId();
    double energy  = aECLCalDigit.getEnergy(); //get calibrated energy.
    double timing  = aECLCalDigit.getTime();   //get calibrated time.

    for (const auto& id : m_HitThresholds) {
      auto scale = id / 1000.;
      auto index = std::distance(m_HitThresholds.begin(), std::find(m_HitThresholds.begin(), m_HitThresholds.end(), id));
      if (energy > scale)  {
        h_cids[index]->Fill(cid);
        nhits[index] += 1;
      }
    }

    for (const auto& id : m_TotalEnergyThresholds) {
      auto scale = id / 1000.;
      auto index = std::distance(m_TotalEnergyThresholds.begin(), std::find(m_TotalEnergyThresholds.begin(),
                                 m_TotalEnergyThresholds.end(), id));
      if (energy > scale) ecltot[index] += energy;
    }

    for (const auto& id : m_TimingThresholds) {
      auto scale = id / 1000.;
      auto index = std::distance(m_TimingThresholds.begin(), std::find(m_TimingThresholds.begin(), m_TimingThresholds.end(), id));
      if (energy > scale) {
        if (cid > ECL_FWD_CHANNELS && cid <= ECL_FWD_CHANNELS + ECL_BARREL_CHANNELS) h_time_barrels[index]->Fill(timing);
        else h_time_endcaps[index]->Fill(timing);
      }
    }

    if (energy > 1.000 && std::abs(timing) < 100.)  h_time_crate_Thr1GeV[mapper.getCrateID(cid) - 1]->Fill(timing);
    if (energy > 1.000 && std::abs(timing) > 100.) h_time_crate_Thr1GeV_large->Fill(mapper.getCrateID(cid));
  }

  for (auto& h : h_edeps) {
    auto index = std::distance(h_edeps.begin(), std::find(h_edeps.begin(), h_edeps.end(), h));
    h->Fill(ecltot[index]);
  }

  for (auto& h : h_ncevs) {
    auto index = std::distance(h_ncevs.begin(), std::find(h_ncevs.begin(), h_ncevs.end(), h));
    h->Fill(nhits[index]);
  }

  for (auto& aECLDsp : m_ECLDsps)  {
    int i = aECLDsp.getCellId() - 1; //get number of Cell ID in m_DspArray.
    aECLDsp.getDspA(m_DspArray[i]);
    m_PedestalMean[i] = 0;
    m_PedestalRms[i] = 0;

    for (int j = 0; j < 16; j++) m_PedestalMean[i] += m_DspArray[i][j];
    m_PedestalMean[i] /= 16;
    h_pedmean_cellid->Fill(aECLDsp.getCellId(), m_PedestalMean[i]); //Pedestal Avg histogram filling.

    for (int j = 0; j < 16; j++) m_PedestalRms[i] += pow(m_DspArray[i][j] - m_PedestalMean[i], 2);
    m_PedestalRms[i] = sqrt(m_PedestalRms[i] / 15.);
    h_pedrms_cellid->Fill(aECLDsp.getCellId(), m_PedestalRms[i]); //Pedestal stddev histogram filling.
    m_geom->Mapping(i);
    h_pedrms_thetaid->Fill(m_geom->GetThetaID(), m_PedestalRms[i]);

    ECLDigit* aECLDigit = ECLDigit::getByCellID(aECLDsp.getCellId());

    for (const auto& id : m_WaveformOption) {
      auto index = std::distance(m_WaveformOption.begin(), std::find(m_WaveformOption.begin(), m_WaveformOption.end(), id));
      if (id != "all" && id != "psd" && id != "logic" && id != "rand" && id != "dphy" && id != "other") continue;
      else if (id == "psd" && (m_iEvent % 1000 == 999 || isRandomTrigger() || bhatrig ||
                               !aECLDigit || aECLDigit->getAmp() < (v_totalthrApsd[i] / 4 * 4))) continue;
      else if (id == "logic" && m_iEvent % 1000 != 999) continue;
      else if (id == "rand" && (m_iEvent % 1000 == 999 || !isRandomTrigger())) continue;
      else if (id == "dphy" && (m_iEvent % 1000 == 999 || !bhatrig)) continue;
      else if (id == "other" && (m_iEvent % 1000 == 999 || isRandomTrigger() || bhatrig ||
                                 (aECLDigit && aECLDigit->getAmp() >= (v_totalthrApsd[i] / 4 * 4)))) continue;
      h_cells[index]->Fill(aECLDsp.getCellId());
      if (id == "other" && aECLDigit) h_quality_other->Fill(aECLDigit->getQuality());
    }
  }
  if (m_ECLDigits.getEntries() > 0)
    h_adc_hits->Fill((double)NDigits / (double)m_ECLDigits.getEntries()); //Fraction of high-energy hits
}

void ECLDQMModule::endRun()
{
}


void ECLDQMModule::terminate()
{
}

bool ECLDQMModule::isRandomTrigger()
{
  if (!m_l1Trigger.isValid()) return false;
  return m_l1Trigger->getTimType() == TRGSummary::ETimingType::TTYP_RAND ||
         m_l1Trigger->getTimType() == TRGSummary::ETimingType::TTYP_POIS;
}

