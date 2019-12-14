/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 *  ECL Data Quality Monitor (First Module)                               *
 *                                                                        *
 *  This module provides histograms for ECL Data Quality Monitoring       *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Dmitry Matvienko (d.v.matvienko@inp.nsk.su)              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

//THIS MODULE
#include <ecl/modules/eclDQM/eclDQM.h>

//Boost
#include <boost/format.hpp>

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
  addParam("HitMapThresholds", m_HitMapThresholds, "Thresholds to display hit map, MeV", std::vector<double> {0, 5, 10, 50});
  addParam("TotalEnergyThresholds", m_TotalEnergyThresholds, "Thresholds to display total energy, MeV", std::vector<double> {0, 5, 7});
  addParam("TimingThresholds", m_TimingThresholds, "Thresholds (MeV) to display ECL timing", std::vector<double> {5, 10, 50});
  addParam("HitNumberThresholdsUpperlimits", m_HitNumberThresholdsUpperLimits,
           "Thresholds (MeV) and upper limit (# of hits) to display hit multiplicity",
  std::vector<std::pair<double, double>> {{0, 10000}, {10, 1000}});
  addParam("WaveformOption", m_WaveformOption, "Option (all,psd,logic,rand,dphy,other) to display waveform flow",
           m_WaveformOption);
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
  h_evtot = new TH1F("event", "Event bank", 1, 0, 1);
  h_evtot->SetOption("LIVE");

  h_quality = new TH1F("quality", "Fit quality flag (0-good, 1- large amplitude, 2 - w/o time,  3 - bad chi2)", 4, 0, 4);
  h_quality->GetXaxis()->SetTitle("Flag number");
  h_quality->SetFillColor(kPink - 4);
  h_quality->SetOption("LIVE");

  h_trigtag1 = new TH1F("trigtag1", "Trigger tag flag # 1", 2, 0, 2);
  h_trigtag1->GetXaxis()->SetTitle("Trigger tag flag #1");
  h_trigtag1->SetOption("LIVE");
  h_trigtag1->SetFillColor(kPink - 4);

  h_adc_hits = new TH1F("adc_hits", "Fraction of psd waveforms vs digits", 1000, 0, 2);
  h_adc_hits->GetXaxis()->SetTitle("psd waveforms / digits");
  h_adc_hits->SetOption("LIVE");

  for (const auto& id : m_HitMapThresholds) {
    std::string h_name, h_title;
    h_name = str(boost::format("cid_Thr%1%MeV") % id);
    h_title = str(boost::format("Crystal ID (SW Thr  = %1% MeV)") % id);
    TH1F* h = new TH1F(h_name.c_str(), h_title.c_str(), 8736, 1, 8737);
    h->GetXaxis()->SetTitle("Cell ID");
    h->SetOption("LIVE");
    h_cids.push_back(h);
  }

  for (const auto& id : m_TotalEnergyThresholds) {
    std::string h_name, h_title;
    h_name = str(boost::format("edep_Thr%1%MeV") % id);
    h_title = str(boost::format("Total energy (SW Thr = %1% MeV)") % id);
    TH1F* h = new TH1F(h_name.c_str(), h_title.c_str(), (int)(100 * m_EnergyUpperThr), 0, m_EnergyUpperThr);
    h->GetXaxis()->SetTitle("energy, [GeV]");
    h->SetOption("LIVE");
    h_edeps.push_back(h);
  }

  for (const auto& id : m_TimingThresholds) {
    std::string h_bar_name, h_bar_title;
    std::string h_end_name, h_end_title;
    h_bar_name = str(boost::format("time_barrel_Thr%1%MeV") % id);
    h_bar_title = str(boost::format("Reconstructed time for ECL barrel (sw Thr = %1% MeV)") % id);
    h_end_name = str(boost::format("time_endcaps_Thr%1%MeV") % id);
    h_end_title = str(boost::format("Reconstructed time for ECL endcaps (sw Thr = %1% MeV)") % id);
    TH1F* h_time_barrel = new TH1F(h_bar_name.c_str(), h_bar_title.c_str(), 206, -1030, 1030);
    TH1F* h_time_endcap = new TH1F(h_end_name.c_str(), h_end_title.c_str(), 206, -1030, 1030);
    h_time_barrel->GetXaxis()->SetTitle("time, [nsec]");
    h_time_endcap->GetXaxis()->SetTitle("time, [nsec]");
    h_time_barrel->SetOption("LIVE");
    h_time_endcap->SetOption("LIVE");
    h_time_barrels.push_back(h_time_barrel);
    h_time_endcaps.push_back(h_time_endcap);
  }

  for (const auto& id : m_HitNumberThresholdsUpperLimits) {
    std::string h_name, h_title;
    h_name = str(boost::format("ncev_Thr%1%MeV") % (id.first));
    h_title = str(boost::format("Number of hits in event (sw Thr = %1% MeV)") % (id.first));
    TH1F* h = new TH1F(h_name.c_str(), h_title.c_str(), id.second, 0, id.second);
    h->GetXaxis()->SetTitle("Number of hits");
    h->SetOption("LIVE");
    h_ncevs.push_back(h);
  }

  for (int i = 0; i < ECL_CRATES; i++) {
    int crate = i + 1;
    std::string h_name, h_title;
    h_name = str(boost::format("time_crate_%1%_Thr1GeV") % (crate));
    h_title = str(boost::format("Reconstructed time for ECL crate #%1% with Thr = 1 GeV") % (crate));
    TH1F* h = new TH1F(h_name.c_str(), h_title.c_str(), 400, -100, 100);
    h->GetXaxis()->SetTitle("time [ns]");
    h->SetOption("LIVE");
    h_time_crate_Thr1GeV.push_back(h);
  }

  for (const auto& id : m_WaveformOption) {
    if (id != "all" && id != "psd" && id != "logic" && id != "rand" && id != "dphy" && id != "other")
      B2WARNING("Waveform Options are not correctly assigned. They must be 'all', 'psd', 'logic', 'rand', 'dphy', 'other'!");
    std::string h_title;
    std::string h_cell_name, h_sh_name, h_cr_name;
    h_title = str(boost::format("Fraction of waveforms for %1%") % (id));
    h_cell_name = str(boost::format("wf_cid_%1%") % (id));
    h_sh_name = str(boost::format("wf_sh_%1%") % (id));
    h_cr_name = str(boost::format("wf_cr_%1%") % (id));
    TH1F* h_cell = new TH1F(h_cell_name.c_str(), h_title.c_str(), 8736, 1, 8737);
    //To be consistent w/ logic histograms where Shaper ID are in range (1,...,624) according to DB objects.
    TH1F* h_sh = new TH1F(h_sh_name.c_str(), h_title.c_str(), 624, 1, 625);
    TH1F* h_cr = new TH1F(h_cr_name.c_str(), h_title.c_str(), 52, 1, 53);
    h_cell->GetXaxis()->SetTitle("Cell ID");
    h_sh->GetXaxis()->SetTitle("Shaper ID");
    h_cr->GetXaxis()->SetTitle("Crate ID");
    h_cell->SetOption("LIVE");
    h_sh->SetOption("LIVE");
    h_cr->SetOption("LIVE");
    h_cells.push_back(h_cell);
    h_shs.push_back(h_sh);
    h_crs.push_back(h_cr);
  }

  //2D histograms creation.

  h_trigtag2_trigid = new TH2F("trigtag2_trigid", "Trigger tag flag # 2 vs. Crate ID", 52, 1, 53, 11, -1, 10);
  h_trigtag2_trigid->GetXaxis()->SetTitle("Crate ID");
  h_trigtag2_trigid->GetYaxis()->SetTitle("Trigger tag flag #2");
  h_trigtag2_trigid->SetOption("LIVE");

  h_pedmean_cellid = new TProfile("pedmean_cellid", "Pedestal vs. Cell ID", 8736, 1, 8737);
  h_pedmean_cellid->GetXaxis()->SetTitle("Cell ID");
  h_pedmean_cellid->GetYaxis()->SetTitle("Pedestal Average");
  h_pedmean_cellid->SetOption("LIVE");

  h_pedrms_cellid = new TProfile("pedrms_cellid", "Pedestal rms error vs. Cell ID",
                                 8736, 1, 8737);
  h_pedrms_cellid->GetXaxis()->SetTitle("Cell ID");
  h_pedrms_cellid->GetYaxis()->SetTitle("Pedestal rms error");
  h_pedrms_cellid->SetOption("LIVE");

  h_pedrms_thetaid = new TProfile("pedrms_thetaid", "Pedestal rms error vs. Theta ID",
                                  68, 0, 68);
  h_pedrms_thetaid->GetXaxis()->SetTitle("Theta ID");
  h_pedrms_thetaid->GetYaxis()->SetTitle("Pedestal rms error");
  h_pedrms_thetaid->SetOption("LIVE");

  h_trigtime_trigid = new TH2F("trigtime_trigid", "Trigger time vs. Crate ID", 52, 1, 53, 145, 0, 145);
  h_trigtime_trigid->GetXaxis()->SetTitle("Crate ID");
  h_trigtime_trigid->GetYaxis()->SetTitle("Trigger time");
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

  if (!mapper.initFromDB()) B2FATAL("ECL Display:: Can't initialize eclChannelMapper");

  ecltot.resize(m_TotalEnergyThresholds.size());
  nhits.resize(m_HitNumberThresholdsUpperLimits.size());

  m_geom = ECLGeometryPar::Instance();

  v_totalthrApsd.resize((m_calibrationThrApsd->getCalibVector()).size());
  for (size_t i = 0; i < v_totalthrApsd.size(); i++) v_totalthrApsd[i] = (int)(m_calibrationThrApsd->getCalibVector())[i];
}

void ECLDQMModule::beginRun()
{
  h_quality->Reset();
  h_trigtag1->Reset();
  h_adc_hits->Reset();
  std::for_each(h_cids.begin(), h_cids.end(), [](auto & it) {it->Reset();});
  std::for_each(h_edeps.begin(), h_edeps.end(), [](auto & it) {it->Reset();});
  std::for_each(h_time_barrels.begin(), h_time_barrels.end(), [](auto & it) {it->Reset();});
  std::for_each(h_time_endcaps.begin(), h_time_endcaps.end(), [](auto & it) {it->Reset();});
  std::for_each(h_ncevs.begin(), h_ncevs.end(), [](auto & it) {it->Reset();});
  std::for_each(h_cells.begin(), h_cells.end(), [](auto & it) {it->Reset();});
  std::for_each(h_shs.begin(), h_shs.end(), [](auto & it) {it->Reset();});
  std::for_each(h_crs.begin(), h_crs.end(), [](auto & it) {it->Reset();});
  for (int i = 0; i < ECL_CRATES; i++) h_time_crate_Thr1GeV[i]->Reset();
  h_trigtag2_trigid->Reset();
  h_pedmean_cellid->Reset();
  h_pedrms_cellid->Reset();
  h_pedrms_thetaid->Reset();
  h_trigtime_trigid->Reset();
}

unsigned short ECLDQMModule::conversion(int cellID)
{
  unsigned short iCrate = mapper.getCrateID(cellID);
  unsigned short iShaperPosition = mapper.getShaperPosition(cellID);
  return (iCrate - 1) * 12 + iShaperPosition;
}

void ECLDQMModule::event()
{
  int trigtag1 = 0;
  int flagtag = 1;
  for (auto& value : ecltot) value = 0;
  for (auto& value : nhits) value = 0;

  if (m_eventmetadata.isValid()) {
    m_iEvent = m_eventmetadata->getEvent();
    h_evtot->Fill(0);
  } else m_iEvent = -1;

  for (auto& aECLDigit : m_ECLDigits) h_quality->Fill(aECLDigit.getQuality());  //Fit quality histogram filling.


  for (auto& aECLTrig : m_ECLTrigs) {
    double itrg = aECLTrig.getTimeTrig();
    //trigger time conversion to acceptable units in range (0, ..., 142).
    //one trigger time clock corresponds to 0.567/144*1000 = 3.93 ns
    int tg = (int)itrg - 2 * ((int)itrg / 8);
    h_trigtime_trigid->Fill(aECLTrig.getTrigId(), tg); //Trigger time histogram filling.
    trigtag1 += aECLTrig.getTrigTag();
    h_trigtag2_trigid->Fill(aECLTrig.getTrigId(), aECLTrig.getTrigTagQualityFlag()); //Trigger tag flag #2 histogram filling.
  }

  if (m_ECLTrigs.getEntries() > 0) trigtag1 /= m_ECLTrigs.getEntries();

  int compar = (65535 & m_iEvent);
  if (compar == trigtag1) flagtag = 0;
  h_trigtag1->Fill(flagtag);  //Trigger tag flag #1 histogram filling.

  for (auto& aECLCalDigit : m_ECLCalDigits) {
    int cid        = aECLCalDigit.getCellId();
    double energy  = aECLCalDigit.getEnergy(); //get calibrated energy.
    double timing  = aECLCalDigit.getTime();   //get calibrated time.

    for (const auto& id : m_HitMapThresholds) {
      auto scale = id / 1000.;
      auto index = std::distance(m_HitMapThresholds.begin(), std::find(m_HitMapThresholds.begin(), m_HitMapThresholds.end(), id));
      if (energy > scale) h_cids[index]->Fill(cid);
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

    for (const auto& id : m_HitNumberThresholdsUpperLimits) {
      auto scale = id.first / 1000.;
      auto index = std::distance(m_HitNumberThresholdsUpperLimits.begin(), std::find(m_HitNumberThresholdsUpperLimits.begin(),
                                 m_HitNumberThresholdsUpperLimits.end(), id));
      if (energy > scale) nhits[index] += 1;
    }

    if (energy > 1.000) h_time_crate_Thr1GeV[mapper.getCrateID(cid) - 1]->Fill(timing);
  }

  for (auto& h : h_edeps) {
    auto index = std::distance(h_edeps.begin(), std::find(h_edeps.begin(), h_edeps.end(), h));
    h->Fill(ecltot[index]);
  }

  for (auto& h : h_ncevs) {
    auto index = std::distance(h_ncevs.begin(), std::find(h_ncevs.begin(), h_ncevs.end(), h));
    h->Fill(nhits[index]);
  }

  int NDsps = 0;
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
    h_pedrms_cellid->Fill(aECLDsp.getCellId(), m_PedestalRms[i]); //Pedestal rms histogram filling.
    m_geom->Mapping(i);
    h_pedrms_thetaid->Fill(m_geom->GetThetaID(), m_PedestalRms[i]);

    ECLDigit* aECLDigit = aECLDsp.getRelated<ECLDigit>();

    bool psd = false;
    for (const auto& id : m_WaveformOption) {
      auto index = std::distance(m_WaveformOption.begin(), std::find(m_WaveformOption.begin(), m_WaveformOption.end(), id));
      if (id != "all" && id != "psd" && id != "logic" && id != "rand" && id != "dphy" && id != "other") continue;
      else if (id == "psd" && (m_iEvent % 1000 == 999 ||
                               m_l1Trigger->getTimType() == TRGSummary::ETimingType::TTYP_RAND ||
                               m_l1Trigger->getTimType() == TRGSummary::ETimingType::TTYP_DPHY ||
                               !aECLDigit || aECLDigit->getAmp() < (v_totalthrApsd[i] / 4 * 4))) continue;
      else if (id == "logic" && m_iEvent % 1000 != 999) continue;
      else if (id == "rand" && (m_iEvent % 1000 == 999 ||
                                m_l1Trigger->getTimType() != TRGSummary::ETimingType::TTYP_RAND)) continue;
      else if (id == "dphy" && (m_iEvent % 1000 == 999 ||
                                m_l1Trigger->getTimType() != TRGSummary::ETimingType::TTYP_DPHY)) continue;
      else if (id == "other" && (m_iEvent % 1000 == 999 ||
                                 m_l1Trigger->getTimType() == TRGSummary::ETimingType::TTYP_RAND ||
                                 m_l1Trigger->getTimType() == TRGSummary::ETimingType::TTYP_DPHY ||
                                 (aECLDigit && aECLDigit->getAmp() >= (v_totalthrApsd[i] / 4 * 4)))) continue;
      h_cells[index]->Fill(aECLDsp.getCellId());
      h_shs[index]->Fill(conversion(aECLDsp.getCellId()));
      h_crs[index]->Fill(mapper.getCrateID(aECLDsp.getCellId()));
      if (id == "psd" && !psd) {
        NDsps ++;
        psd = true;
      }
    }
  }
  if (m_ECLDigits.getEntries() > 0 && NDsps > 0)
    h_adc_hits->Fill((double)NDsps / (double)m_ECLDigits.getEntries()); //ADC hits histogram filling.
}

void ECLDQMModule::endRun()
{
}


void ECLDQMModule::terminate()
{
}
