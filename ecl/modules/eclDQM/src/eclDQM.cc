/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 *  ECL Data Quality Monitor                                              *
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

//FRAMEWORK
#include <framework/core/HistoModule.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
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
#include "TH1F.h"
#include "TH2F.h"
#include <TDirectory.h>

#include <iostream>

//NAMESPACE(S)
using namespace Belle2;
using namespace ECL;
using namespace std;

REG_MODULE(ECLDQM)

ECLDQMModule::ECLDQMModule() : HistoModule()
{
  //Set module properties.
  setDescription("ECL Data Quality Monitor");
  setPropertyFlags(c_ParallelProcessingCertified);  // specify parallel processing.
  addParam("histogramDirectoryName", m_histogramDirectoryName,
           "histogram directory in ROOT file", string("ECL"));
  addParam("EnergyUpperThr", m_EnergyUpperThr, "Upper threshold of energy deposition in event, [GeV]", 1.5 * Belle2::Unit::GeV);
  addParam("PedestalMeanUpperThr", m_PedestalMeanUpperThr, "Upper threshold of pedestal distribution", 7000);
  addParam("PedestalMeanLowerThr", m_PedestalMeanLowerThr, "Lower threshold of pedestal distribution", -1000);
  addParam("PedestalRmsUpperThr", m_PedestalRmsUpperThr, "Upper threshold of pedestal rms error distribution", 100.);

}

ECLDQMModule::~ECLDQMModule()
{
}


void ECLDQMModule::defineHisto()
{
  TDirectory* oldDir = gDirectory;

  // Create a separate histogram directory and cd into it.

  TDirectory* dirDAQ = NULL;
  dirDAQ = dynamic_cast<TDirectory*>(oldDir->Get(m_histogramDirectoryName.c_str()));
  if (!dirDAQ) dirDAQ = oldDir->mkdir(m_histogramDirectoryName.c_str());
  dirDAQ->cd();

  //1D histograms creation.

  //Readout Cell_IDs.

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

  //Channel multiplicity.

  h_ncev = new TH1F("ncev", "Number of hits per event", 70, 0, 70);
  h_ncev->GetXaxis()->SetTitle("Number of hits");
  h_ncev->SetOption("LIVE");

  h_ncev_Thr10MeV = new TH1F("ncev_Thr10MeV", "Number of hits per event (Thr = 10 MeV)", 70, 0, 70);
  h_ncev_Thr10MeV->GetXaxis()->SetTitle("Number of hits");
  h_ncev_Thr10MeV->SetOption("LIVE");

  //Energy deposition.

  h_edep = new TH1F("edep", "Energy deposition in event", int(100 * m_EnergyUpperThr), 0, m_EnergyUpperThr);
  h_edep->GetXaxis()->SetTitle("energy, [GeV]");
  h_edep->SetOption("LIVE");

  //Timing distributions.

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

  //Fit quality flag.

  h_quality = new TH1F("quality", "Fit quality flag (0-good, 1- large amplitude, 3 - bad chi2)", 5, -1, 4);
  h_quality->GetXaxis()->SetTitle("Flag number");
  h_quality->SetOption("LIVE");

  //Trigger tag flag #1.

  h_trigtag1 = new TH1F("trigtag1", "Trigger tag flag # 1", 4, -1, 3);
  h_trigtag1->GetXaxis()->SetTitle("Trigger tag flag #1");
  h_trigtag1->SetOption("LIVE");


  //2D histograms creation.

  //Trigger tag flag #2.

  h_trigtag2_trigid = new TH2F("trigtag2_trigid", "Trigger tag flag # 2 vs. Trig. Cell ID", 52, 1, 53, 11, -1, 10);
  h_trigtag2_trigid->GetXaxis()->SetTitle("Trigger Cell ID");
  h_trigtag2_trigid->GetYaxis()->SetTitle("Trigger tag flag #2");
  h_trigtag2_trigid->SetOption("LIVE");


  //Pedestal.

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

  //Trigger time.

  h_trigtime_trigid = new TH2F("trigtime_trigid", "Trigger time vs. Trig. Cell ID", 52, 1, 53, 145, 0, 145);
  h_trigtime_trigid->GetXaxis()->SetTitle("Trigger Cell ID");
  h_trigtime_trigid->GetYaxis()->SetTitle("Trigger time");
  h_trigtime_trigid->SetOption("LIVE");

  //cd into parent directory.

  oldDir->cd();
}

void ECLDQMModule::initialize()
{
  REG_HISTOGRAM;   // required to register histograms to HistoManager.

  StoreArray<ECLDigit> ECLDigits;
  ECLDigits.isRequired();

  StoreArray<ECLCalDigit> ECLCalDigits;
  ECLCalDigits.isOptional();

  StoreArray<ECLTrig> ECLTrigs;
  ECLTrigs.isOptional();

  StoreArray<ECLDsp> ECLDsps;
  ECLDsps.isOptional();


}

void ECLDQMModule::beginRun()
{
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
  h_trigtag2_trigid->Reset();
  h_pedmean_cellid->Reset();
  h_pedrms_cellid->Reset();
  h_trigtime_trigid->Reset();
}

void ECLDQMModule::event()
{
  int m_iEvent;

  StoreObjPtr<EventMetaData> eventmetadata;
  if (eventmetadata) m_iEvent = eventmetadata->getEvent();
  else m_iEvent = -1;

  StoreArray<ECLDigit> ECLDigits;
  StoreArray<ECLCalDigit> ECLCalDigits;
  StoreArray<ECLTrig> ECLTrigs;
  StoreArray<ECLDsp> ECLDsps;

  int NHitsEvent = ECLCalDigits.getEntries();
  int trigtag1 = 0;
  int flagtag = 1;

  if (ECLDigits.getEntries() != NHitsEvent) B2ERROR("ECLDigit and ECLCalDigit objects have different number of entries!!!");

  for (auto& aECLDigit : ECLDigits) {

    h_quality->Fill(aECLDigit.getQuality());  //Fit quality histogram filling.

  }

  for (auto& aECLTrig : ECLTrigs) {

    double itrg = aECLTrig.getTimeTrig();

    //trigger time conversion to acceptable units in range (0, ..., 142).
    //one trigger time clock corresponds to 0.567/144*1000 = 3.93 ns

    int tg = (int)itrg - 2 * ((int)itrg / 8);

    h_trigtime_trigid->Fill(aECLTrig.getTrigId(), tg); //Trigger time histogram filling.

    trigtag1 += aECLTrig.getTrigTag();

    h_trigtag2_trigid->Fill(aECLTrig.getTrigId(), aECLTrig.getTrigTagQualityFlag()); //Trigger tag flag #2 histogram filling.

  }

  if (ECLTrigs.getEntries() > 0) {
    trigtag1 /= ECLTrigs.getEntries();
  }

  int compar = (65535 & m_iEvent);
  if (compar == trigtag1) flagtag = 0;

  h_trigtag1->Fill(flagtag);  //Trigger tag flag #1 histogram filling.

  double ecletot = 0;
  int NHitsEventThr10MeV = 0;

  for (auto& aECLCalDigit : ECLCalDigits) {

    int cid        = aECLCalDigit.getCellId();
    double energy  = aECLCalDigit.getEnergy(); //get calibrated energy.
    double timing  = aECLCalDigit.getTime();   //get calibrated time.

    h_cid->Fill(cid); //Cell ID histogram filling.

    if (energy > 0.005) {

      h_cid_Thr5MeV->Fill(cid); //Cell ID histogram filling.

      //Time histogram filling.

      if (cid > ECL_FWD_CHANNELS
          && cid < ECL_FWD_CHANNELS + ECL_BARREL_CHANNELS) h_time_barrel_Thr5MeV->Fill(timing);
      else h_time_endcaps_Thr5MeV->Fill(timing);

    }

    if (energy > 0.010)  {

      NHitsEventThr10MeV ++;

      h_cid_Thr10MeV->Fill(cid); //Cell ID histogram filling.

      //Time histogram filling.

      if (cid > ECL_FWD_CHANNELS
          && cid < ECL_FWD_CHANNELS + ECL_BARREL_CHANNELS) h_time_barrel_Thr10MeV->Fill(timing);
      else h_time_endcaps_Thr10MeV->Fill(timing);

    }

    if (energy > 0.050) {

      h_cid_Thr50MeV->Fill(cid); //Cell ID histogram filling.

      //Time histogram filling.

      if (cid > ECL_FWD_CHANNELS
          && cid < ECL_FWD_CHANNELS + ECL_BARREL_CHANNELS) h_time_barrel_Thr50MeV->Fill(timing);
      else h_time_endcaps_Thr50MeV->Fill(timing);

    }

    ecletot += energy;

  }

  h_edep->Fill(ecletot); //Energy histogram filling.

  //Multiplicity histograms filling.

  h_ncev->Fill(NHitsEvent);
  h_ncev_Thr10MeV->Fill(NHitsEventThr10MeV);

  for (auto& aECLDsp : ECLDsps)  {
    int i = aECLDsp.getCellId() - 1; //get number of Cell ID in m_DspArray.

    if (i >= 0) {
      aECLDsp.getDspA(m_DspArray[i]);
      m_PedestalMean[i] = 0;
      m_PedestalRms[i] = 0;

      for (int j = 0; j < 16; j++) {
        m_PedestalMean[i] += m_DspArray[i][j];
      }

      m_PedestalMean[i] /= 16;

      h_pedmean_cellid->Fill(aECLDsp.getCellId(), m_PedestalMean[i]); //Pedestal Avg histogram filling.

      for (int j = 0; j < 16; j++) {
        m_PedestalRms[i] += pow(m_DspArray[i][j] - m_PedestalMean[i], 2);
      }

      m_PedestalRms[i] = sqrt(m_PedestalRms[i] / 15.);

      h_pedrms_cellid->Fill(aECLDsp.getCellId(), m_PedestalRms[i]); //Pedestal Rms error histogram filling.
    }
  }
}

void ECLDQMModule::endRun()
{
}


void ECLDQMModule::terminate()
{
}
