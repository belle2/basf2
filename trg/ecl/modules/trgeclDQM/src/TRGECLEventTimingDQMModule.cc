/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <trg/ecl/modules/trgeclDQM/TRGECLEventTimingDQMModule.h>
#include <framework/core/HistoModule.h>
#include <framework/datastore/StoreArray.h>
#include <iostream>
#include <string>
using namespace Belle2;

REG_MODULE(TRGECLEventTimingDQM);


TRGECLEventTimingDQMModule::TRGECLEventTimingDQMModule(): HistoModule()
{
  setPropertyFlags(c_ParallelProcessingCertified);
  setDescription("ECL trigger event timing histograms");
  m_trgecl_map = new TrgEclMapping();

  m_histEventT0CoarseXMin = -150;
  m_histEventT0CoarseXMax = 150;
  m_histEventT0FineXMin = -150;
  m_histEventT0FineXMax = 150;

  for (int iii = 0; iii < c_NBinEventT0; iii++) {
    if (iii <= 1) {
      m_histEventT0NBin.push_back(100);
      m_histEventT0XMin.push_back(-200);
      m_histEventT0XMax.push_back(200);
    } else if (iii <= 5) {
      m_histEventT0NBin.push_back(100);
      m_histEventT0XMin.push_back(-100);
      m_histEventT0XMax.push_back(100);
    } else {
      m_histEventT0NBin.push_back(50);
      m_histEventT0XMin.push_back(-50);
      m_histEventT0XMax.push_back(50);
    }
  }
}


TRGECLEventTimingDQMModule::~TRGECLEventTimingDQMModule()
{
  delete m_trgecl_map;
}

void TRGECLEventTimingDQMModule::defineHisto()
{

  TDirectory* oldDir = gDirectory;
  oldDir->mkdir("TRGECLEventTiming")->cd();

  m_histMaxTCE =
    new TH1F("h_MaxTCE",
             "[TRGECL] Most Energetic TC Energy ; max TC Energy (ADC) ; Entries / 10 ADC",
             140, 0, 1400);
  m_histMaxTCId =
    new TH1F("h_MaxTCId",
             "[TRGECL] Most Energetic TC ID ; max TC ID ; Entries",
             578, 0, 578);
  m_histMaxTCThetaId =
    new TH1F("h_MaxTCThetaId",
             "[TRGECL] Most Energetic TC Theta ID ; max TC #theta ID ; Entries",
             19, 0, 19);
  m_histEventTimingQuality =
    new TH1F("h_EventTimingQuality",
             "[TRGECL] Event timing quality ; Event timing quality ; Entries",
             4, 0, 4);

  m_histEventT0Coarse =
    new TH1F("h_EventT0Coarse",
             "[TRGECL] EventT0 with coarse event timing ; EventT0 (ns) ; Entries / 2 ns",
             150, m_histEventT0CoarseXMin, m_histEventT0CoarseXMax);
  m_histEventT0Fine =
    new TH1F("h_EventT0Fine",
             "[TRGECL] EventT0 with fine event timing ; EventT0 (ns) ; Entries / 2 ns",
             150, m_histEventT0FineXMin, m_histEventT0FineXMax);

  // set label for h_EventTimingQuality
  m_histEventTimingQuality->GetXaxis()->SetBinLabel(1, "None");
  m_histEventTimingQuality->GetXaxis()->SetBinLabel(2, "Coarse");
  m_histEventTimingQuality->GetXaxis()->SetBinLabel(3, "Fine");
  m_histEventTimingQuality->GetXaxis()->SetBinLabel(4, "Super fine");

  // EventT0 histograms for different TC energy region
  for (int energyBin = 0; energyBin < c_NBinEventT0; energyBin++) {

    // set TC energy(ADC) ranges for each histogram
    std::stringstream ss1, ss2;
    ss1 << std::setfill('0') << std::setw(3) << std::to_string(10 + 20 * energyBin);
    ss2 << std::setfill('0') << std::setw(3) << std::to_string(10 + 20 * (energyBin + 1));
    TString ECUT1 = ss1.str() + "to" + ss2.str();
    TString ECUT2 = ss1.str() + "-"  + ss2.str();

    // EventT0
    m_histEventT0[energyBin] =
      new TH1F("h_EventT0_MaxTCE_" + ECUT1 + "",
               "[TRGECL] EventT0 MaxTCE (" + ECUT2 + " ADC) ; EventT0 (ns); Entries / 2 ns",
               m_histEventT0NBin[energyBin],
               m_histEventT0XMin[energyBin],
               m_histEventT0XMax[energyBin]);
  }

  oldDir->cd();
}

void TRGECLEventTimingDQMModule::initialize()
{
  m_eventT0.isOptional();
  REG_HISTOGRAM
}

void TRGECLEventTimingDQMModule::beginRun()
{

  if (!m_eventT0.isValid()) {
    B2DEBUG(20, "No EventT0 is found.");
    return;
  }

  if (!m_objTrgSummary.isValid()) {
    B2DEBUG(20, "No TRGSummary is found.");
    return;
  }

  m_histMaxTCE->Reset();
  m_histMaxTCId->Reset();
  m_histMaxTCThetaId->Reset();
  m_histEventTimingQuality->Reset();
  m_histEventT0Coarse->Reset();
  m_histEventT0Fine->Reset();

  for (int energyBin = 0; energyBin < c_NBinEventT0; energyBin++) {
    m_histEventT0[energyBin]->Reset();
  }

}

void TRGECLEventTimingDQMModule::event()
{

  // check TRGSummary object
  if (!m_objTrgSummary.isValid()) {
    B2DEBUG(20, "TRGSummary object not available");
    return;
  }
  //! L1 timing source in TRGSummary
  //! 0=ecltrg, 1=toptrg, 2=self trg, 3=cdctrg, 5=delayed Bhabha,
  //! 7=random trigger from gdl, 13=poisson random trigger
  int m_L1TimingSrc = m_objTrgSummary->getTimType();
  // accept only event that L1 timing is from ECL trigger
  if (m_L1TimingSrc != 0) {
    B2DEBUG(20, "This event is not deternied by ECL trigger.");
    return ;
  }

  // Skip this event if there is no EventT0, to avoid crashing other DQM
  if (!m_eventT0.isOptional()) {
    B2DEBUG(20, "No EventT0 is found.");
    return;
  }

  // Determine if there is a valid EventT0 to use and then extract the information about it
  if (!m_eventT0.isValid()) {
    B2DEBUG(20, "No valid EventT0 is found.");
    return ;
  }

  // get event timing window of ecl trigger
  int evt_win      = -1000;
  int evt_time_win = -1000;
  for (const TRGECLUnpackerEvtStore& iii : m_trgeclUnpackerEvtStores) {
    evt_win      = iii.getEvtWin();
    evt_time_win = iii.getEvtTimeWin();
  }
  int flag_win_a = -1;
  int flag_win_b = -1;
  if (evt_win == 3) {
    if (evt_time_win == -1) {
      flag_win_a = 2;
      flag_win_b = 3;
    } else {
      flag_win_a = 3;
      flag_win_b = 4;
    }
  } else if (evt_win == 4) {
    if (evt_time_win == -1) {
      flag_win_a = 3;
      flag_win_b = 4;
    } else {
      flag_win_a = 4;
      flag_win_b = 5;
    }
  } else {
    B2DEBUG(20, "L1 timing is out of ETM hit window");
    return;
  }

  // TC information
  int MaxTCEnergy = 0;
  int MaxTCId = -1000;
  int MaxTCThetaId = 0;
  for (const TRGECLUnpackerStore& iii : m_trgeclUnpackerStores) {
    // L1 window selection
    int hit_win =  iii.getHitWin();
    if (!(hit_win == flag_win_a ||
          hit_win == flag_win_b)) { continue; }
    int TCId = iii.getTCId();
    int TCEnergy = iii.getTCEnergy();
    if (TCId < 1 || TCId > 576 || TCEnergy == 0) { continue; }
    // select most energetic TC
    if (TCEnergy > MaxTCEnergy) {
      MaxTCEnergy  = TCEnergy;
      MaxTCId      = TCId;
      MaxTCThetaId = m_trgecl_map->getTCThetaIdFromTCId(TCId);
    }
  }
  if (MaxTCId == -1000) {
    B2DEBUG(20, "No max TC is found.");
    return;
  }
  m_histMaxTCE->Fill(MaxTCEnergy);
  m_histMaxTCId->Fill(MaxTCId);
  m_histMaxTCThetaId->Fill(MaxTCThetaId);

  // Event timing quality from TRGSummary
  // 0=none, 1=coarse, 2=fine, 3=super fine
  int gdlTimQuality = m_objTrgSummary->getTimQuality();
  m_histEventTimingQuality->Fill(gdlTimQuality);

  // get EventT0
  const double eventT0 =
    m_eventT0->hasEventT0() ?
    m_eventT0->getEventT0() : -1000;

  // EventT0 with coarse or fine event timing
  if (gdlTimQuality == 1) {
    // coarse
    if (eventT0 > m_histEventT0CoarseXMin &&
        eventT0 < m_histEventT0CoarseXMax) {
      m_histEventT0Coarse->Fill(eventT0);
    }
  } else if (gdlTimQuality >= 2) {
    // fine or super fine
    if (eventT0 > m_histEventT0FineXMin &&
        eventT0 < m_histEventT0FineXMax) {
      m_histEventT0Fine->Fill(eventT0);
    }
  }

  // set TC energy region for EventT0
  bool IsMaxTCEnergy[c_NBinEventT0] = {false};
  int EnergyBinWidth = 20;
  for (int idx = 0; idx < c_NBinEventT0;  idx++) {
    if (MaxTCEnergy >= 10 + EnergyBinWidth * idx &&
        MaxTCEnergy <= 10 + EnergyBinWidth * (idx + 1)) {
      IsMaxTCEnergy[idx] = true;
    }
  }

  // fill EventT0 for different TC energy region
  for (int binADC = 0 ; binADC < c_NBinEventT0; binADC++) {
    if (IsMaxTCEnergy[binADC] == true) {
      if (eventT0 > m_histEventT0XMin[binADC] &&
          eventT0 < m_histEventT0XMax[binADC]) {
        m_histEventT0[binADC]->Fill(eventT0);
      }
    }
  }

}
