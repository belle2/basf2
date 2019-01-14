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

//ROOT
#include <TH1F.h>
#include <TH2F.h>
#include <TDirectory.h>

//STL
#include <iostream>

//NAMESPACE(S)
using namespace Belle2;
using namespace ECL;

REG_MODULE(ECLDQM)

ECLDQMModule::ECLDQMModule() : HistoModule()
{
  //Set module properties.
  setDescription("ECL Data Quality Monitor");
  setPropertyFlags(c_ParallelProcessingCertified);  // specify parallel processing.
  addParam("histogramDirectoryName", m_histogramDirectoryName,
           "histogram directory in ROOT file", std::string("ECL"));
  addParam("NHitsUpperThr1", m_NHitsUpperThr1, "Upper threshold of number of hits in event", 10000);
  addParam("NHitsUpperThr2", m_NHitsUpperThr2, "Upper threshold of number of hits in event (w/ Thr=10 MeV)", 1000);
  addParam("EnergyUpperThr", m_EnergyUpperThr, "Upper threshold of energy deposition in event, [GeV]", 10.0 * Belle2::Unit::GeV);
  addParam("PedestalMeanUpperThr", m_PedestalMeanUpperThr, "Upper threshold of pedestal distribution", 15000);
  addParam("PedestalMeanLowerThr", m_PedestalMeanLowerThr, "Lower threshold of pedestal distribution", -15000);
  addParam("PedestalRmsUpperThr", m_PedestalRmsUpperThr, "Upper threshold of pedestal rms error distribution", 1000.);
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

  h_cid = new TH1F("cid", "Crystal ID", 8736, 1, 8737);
  h_cid->GetXaxis()->SetTitle("Cell ID");
  h_cid->SetOption("LIVE");

  h_cid_Thr5MeV = new TH1F("cid_Thr5MeV", "Crystal ID (Thr = 5 MeV)", 8736, 1, 8737);
  h_cid_Thr5MeV->GetXaxis()->SetTitle("Cell ID");
  h_cid_Thr5MeV->SetOption("LIVE");

  h_cid_Thr10MeV = new TH1F("cid_Thr10MeV", "Crystal ID (Thr  = 10 MeV)", 8736, 1, 8737);
  h_cid_Thr10MeV->GetXaxis()->SetTitle("Cell ID");
  h_cid_Thr10MeV->SetOption("LIVE");

  h_cid_Thr50MeV = new TH1F("cid_Thr50MeV", "Crystal ID (Thr = 50 MeV)", 8736, 1, 8737);
  h_cid_Thr50MeV->GetXaxis()->SetTitle("Cell ID");
  h_cid_Thr50MeV->SetOption("LIVE");

  h_ncev = new TH1F("ncev", "Number of hits in event", (int)(m_NHitsUpperThr1 / 10), 0, m_NHitsUpperThr1);
  h_ncev->GetXaxis()->SetTitle("Number of hits");
  h_ncev->SetOption("LIVE");

  h_ncev_Thr10MeV = new TH1F("ncev_Thr10MeV", "Number of hits in event (Thr = 10 MeV)", m_NHitsUpperThr2, 0, m_NHitsUpperThr2);
  h_ncev_Thr10MeV->GetXaxis()->SetTitle("Number of hits");
  h_ncev_Thr10MeV->SetOption("LIVE");

  h_edep = new TH1F("edep", "Energy deposition in event", (int)(100 * m_EnergyUpperThr), 0, m_EnergyUpperThr);
  h_edep->GetXaxis()->SetTitle("energy, [GeV]");
  h_edep->SetOption("LIVE");

  h_time_barrel_Thr5MeV = new TH1F("time_barrel_Thr5MeV", "Reconstructed time for ECL barrel with Thr = 5 MeV", 206, -1030,
                                   1030);
  h_time_barrel_Thr5MeV->GetXaxis()->SetTitle("time [ns]");
  h_time_barrel_Thr5MeV->SetOption("LIVE");

  h_time_endcaps_Thr5MeV = new TH1F("time_endcaps_Thr5MeV", "Reconstructed time for ECL endcaps with Thr = 5 MeV", 206, -1030,
                                    1030);
  h_time_endcaps_Thr5MeV->GetXaxis()->SetTitle("time [ns]");
  h_time_endcaps_Thr5MeV->SetOption("LIVE");

  h_time_barrel_Thr10MeV = new TH1F("time_barrel_Thr10MeV", "Reconstructed time for ECL barrel with Thr = 10 MeV", 206, -1030,
                                    1030);
  h_time_barrel_Thr10MeV->GetXaxis()->SetTitle("time [ns]");
  h_time_barrel_Thr10MeV->SetOption("LIVE");

  h_time_endcaps_Thr10MeV = new TH1F("time_endcaps_Thr10MeV", "Reconstructed time for ECL endcaps with Thr = 10 MeV", 206, -1030,
                                     1030);
  h_time_endcaps_Thr10MeV->GetXaxis()->SetTitle("time [ns]");
  h_time_endcaps_Thr10MeV->SetOption("LIVE");

  h_time_barrel_Thr50MeV = new TH1F("time_barrel_Thr50MeV", "Reconstructed time for ECL barrel with Thr = 50 MeV", 206, -1030,
                                    1030);
  h_time_barrel_Thr50MeV->GetXaxis()->SetTitle("time [ns]");
  h_time_barrel_Thr50MeV->SetOption("LIVE");

  h_time_endcaps_Thr50MeV = new TH1F("time_endcaps_Thr50MeV", "Reconstructed time for ECL endcaps with Thr = 50 MeV", 206, -1030,
                                     1030);
  h_time_endcaps_Thr50MeV->GetXaxis()->SetTitle("time [ns]");
  h_time_endcaps_Thr50MeV->SetOption("LIVE");

  h_quality = new TH1F("quality", "Fit quality flag (0-good, 1- large amplitude, 2 - w/o time,  3 - bad chi2)", 4, 0, 4);
  h_quality->GetXaxis()->SetTitle("Flag number");
  h_quality->SetFillColor(kPink - 4);
  h_quality->SetOption("LIVE");

  h_trigtag1 = new TH1F("trigtag1", "Trigger tag flag # 1", 2, 0, 2);
  h_trigtag1->GetXaxis()->SetTitle("Trigger tag flag #1");
  h_trigtag1->SetOption("LIVE");
  h_trigtag1->SetFillColor(kPink - 4);

  h_adc_flag = new TH1F("adc_flag", "Flag of ADC samples", 3, 0, 3);
  h_adc_flag->GetXaxis()->SetTitle("Flag of ADC samples");
  h_adc_flag->SetOption("LIVE");
  h_adc_flag->SetFillColor(kPink - 4);

  h_adc_hits = new TH1F("adc_hits", "Fraction of ADC samples in event (w/o 8736 ADC samples)", 250, 0, 5);
  h_adc_hits->GetXaxis()->SetTitle("Fraction of ADC samples");
  h_adc_hits->SetOption("LIVE");

  for (int i = 0; i < ECL_CRATES; i++) {
    int crate = i + 1;
    std::string h_name, h_title;
    h_name = str(boost::format("time_crate_%1%_Thr1GeV") % (crate));
    h_title = str(boost::format("Reconstructed time for ECL crate #%1% with Thr = 1 GeV") % (crate));
    TH1F* h = new TH1F(h_name.c_str(), h_title.c_str(), 8240, -1030, 1030);
    h->GetXaxis()->SetTitle("time [ns]");
    h->SetOption("LIVE");
    h_time_crate_Thr1GeV.push_back(h);
  }

  //2D histograms creation.

  h_trigtag2_trigid = new TH2F("trigtag2_trigid", "Trigger tag flag # 2 vs. Crate ID", 52, 1, 53, 11, -1, 10);
  h_trigtag2_trigid->GetXaxis()->SetTitle("Crate ID");
  h_trigtag2_trigid->GetYaxis()->SetTitle("Trigger tag flag #2");
  h_trigtag2_trigid->SetOption("LIVE");

  h_pedmean_cellid = new TH2F("pedmean_cellid", "Pedestal Average vs. Cell ID", 8736, 1, 8737, 200, m_PedestalMeanLowerThr,
                              m_PedestalMeanUpperThr);
  h_pedmean_cellid->GetXaxis()->SetTitle("Cell ID");
  h_pedmean_cellid->GetYaxis()->SetTitle("Pedestal Average");
  h_pedmean_cellid->SetOption("LIVE");

  h_pedrms_cellid = new TH2F("pedrms_cellid", "Pedestal rms error vs. Cell ID", 8736, 1, 8737, (int)m_PedestalRmsUpperThr, 0,
                             m_PedestalRmsUpperThr);
  h_pedrms_cellid->GetXaxis()->SetTitle("Cell ID");
  h_pedrms_cellid->GetYaxis()->SetTitle("Pedestal rms error");
  h_pedrms_cellid->SetOption("LIVE");

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
  if (!mapper.initFromDB()) B2FATAL("ECL Display:: Can't initialize eclChannelMapper");
}

void ECLDQMModule::beginRun()
{
  for (int i = 0; i < ECL_CRATES; i++) h_time_crate_Thr1GeV[i]->Reset();
  h_cid->Reset();
  h_cid_Thr5MeV->Reset();
  h_cid_Thr10MeV->Reset();
  h_cid_Thr50MeV->Reset();
  h_ncev->Reset();
  h_ncev_Thr10MeV->Reset();
  h_edep->Reset();
  h_time_barrel_Thr5MeV->Reset();
  h_time_endcaps_Thr5MeV->Reset();
  h_time_barrel_Thr10MeV->Reset();
  h_time_endcaps_Thr10MeV->Reset();
  h_time_barrel_Thr50MeV->Reset();
  h_time_endcaps_Thr50MeV->Reset();
  h_quality->Reset();
  h_trigtag1->Reset();
  h_adc_flag->Reset();
  h_adc_hits->Reset();
  h_trigtag2_trigid->Reset();
  h_pedmean_cellid->Reset();
  h_pedrms_cellid->Reset();
  h_trigtime_trigid->Reset();
}

void ECLDQMModule::event()
{
  int NHitsEvent = m_ECLCalDigits.getEntries();
  int trigtag1 = 0;
  int flagtag = 1;
  double ecletot = 0.;
  int NHitsEventThr10MeV = 0;
  double adc_flag_bin[3] = {0., 0., 0.};


  if (m_eventmetadata) m_iEvent = m_eventmetadata->getEvent();
  else m_iEvent = -1;

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

    h_cid->Fill(cid); //Cell ID histogram filling.

    if (energy > 0.005) {
      h_cid_Thr5MeV->Fill(cid); //Cell ID histogram filling.
      if (cid > ECL_FWD_CHANNELS
          && cid < ECL_FWD_CHANNELS + ECL_BARREL_CHANNELS) h_time_barrel_Thr5MeV->Fill(timing);
      else h_time_endcaps_Thr5MeV->Fill(timing); //Time histogram filling.
    }

    if (energy > 0.010)  {
      NHitsEventThr10MeV++;
      h_cid_Thr10MeV->Fill(cid); //Cell ID histogram filling.
      if (cid > ECL_FWD_CHANNELS
          && cid < ECL_FWD_CHANNELS + ECL_BARREL_CHANNELS) h_time_barrel_Thr10MeV->Fill(timing);
      else h_time_endcaps_Thr10MeV->Fill(timing); //Time histogram filling.
    }

    if (energy > 0.050) {
      h_cid_Thr50MeV->Fill(cid); //Cell ID histogram filling.
      if (cid > ECL_FWD_CHANNELS
          && cid < ECL_FWD_CHANNELS + ECL_BARREL_CHANNELS) h_time_barrel_Thr50MeV->Fill(timing);
      else h_time_endcaps_Thr50MeV->Fill(timing); //Time histogram filling.
    }

    if (energy > 1.000) h_time_crate_Thr1GeV[mapper.getCrateID(cid) - 1]->Fill(timing);

    ecletot += energy;
  }

  h_edep->Fill(ecletot); //Energy histogram filling.
  h_ncev->Fill(NHitsEvent); //Multiplicity histogram filling.
  h_ncev_Thr10MeV->Fill(NHitsEventThr10MeV); //Multiplicity histogram filling.

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
    h_pedrms_cellid->Fill(aECLDsp.getCellId(), m_PedestalRms[i]); //Pedestal Rms error histogram filling.
  }
  h_adc_flag->Fill(0); //ADC flag histogram filling.
  if (m_ECLDsps.getEntries() == ECL_TOTAL_CHANNELS) h_adc_flag->Fill(1); //ADC flag histogram filling.
  if (m_ECLDsps.getEntries() > 0 && m_ECLDsps.getEntries() < ECL_TOTAL_CHANNELS) h_adc_flag->Fill(2); //ADC flag histogram filling.
  for (int i = 0; i < 3; i++) adc_flag_bin[i] = h_adc_flag->GetBinContent(i + 1);
  std::string adc_flag_title = str(boost::format("Flag of ADC samples (%1%, %2%)") % (adc_flag_bin[1] / adc_flag_bin[0]) %
                                   (adc_flag_bin[2] / adc_flag_bin[0]));
  h_adc_flag->SetTitle(adc_flag_title.c_str());
  if (m_ECLDsps.getEntries() > 0 && m_ECLDsps.getEntries() < ECL_TOTAL_CHANNELS && m_ECLDigits.getEntries() > 0)
    h_adc_hits->Fill((double)m_ECLDsps.getEntries() / (double)m_ECLDigits.getEntries()); //ADC hits histogram filling.
}

void ECLDQMModule::endRun()
{
}


void ECLDQMModule::terminate()
{
}
