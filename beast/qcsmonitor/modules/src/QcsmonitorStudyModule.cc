/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <beast/qcsmonitor/modules/QcsmonitorStudyModule.h>
#include <beast/qcsmonitor/dataobjects/QcsmonitorSimHit.h>
#include <beast/qcsmonitor/dataobjects/QcsmonitorHit.h>
#include <generators/SAD/dataobjects/SADMetaHit.h>
#include <framework/datastore/StoreArray.h>
#include <framework/gearbox/GearDir.h>
#include <framework/logging/Logger.h>
#include <framework/gearbox/Const.h>
#include <cmath>

#include <fstream>
#include <string>

// ROOT
#include <TH1.h>
#include <TH2.h>

int eventNum = 0;

using namespace std;

using namespace Belle2;
using namespace qcsmonitor;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(QcsmonitorStudy)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

QcsmonitorStudyModule::QcsmonitorStudyModule() : HistoModule()
{
  // Set module properties
  setDescription("Study module for Qcsmonitors (BEAST)");

  addParam("Ethres", m_Ethres, "Energy threshold in MeV", 0.0);
}

QcsmonitorStudyModule::~QcsmonitorStudyModule()
{
}

//This module is a histomodule. Any histogram created here will be saved by the HistoManager module
void QcsmonitorStudyModule::defineHisto()
{
  for (int i = 0; i < 40; i++) {
    h_qcss_Evtof1[i] = new TH2F(TString::Format("qcss_Evtof1_%d", i), "Energy deposited [MeV] vs TOF [ns] - all", 500, 0., 1000.,
                                100, 0., 10.);
    h_qcss_Evtof2[i] = new TH2F(TString::Format("qcss_Evtof2_%d", i), "Energy deposited [MeV] vs TOF [ns] - only photons", 500, 0.,
                                100., 1000, 0., 10.);
    h_qcss_Evtof3[i] = new TH2F(TString::Format("qcss_Evtof3_%d", i), "Energy deposited [MeV] vs TOF [ns] - only e+/e-", 500, 0.,
                                100., 1000, 0., 10.);
    h_qcss_Evtof4[i] = new TH2F(TString::Format("qcss_Evtof4_%d", i), "Energy deposited [MeV] vs TOF [ns] - only e+/e-", 500, 0.,
                                100., 1000, 0., 10.);
    h_qcss_edep[i] = new TH1F(TString::Format("qcss_edep_%d", i), "Energy deposited [MeV]", 5000, 0., 10.);
    h_Wqcss_edep[i] = new TH1F(TString::Format("Wqcss_edep_%d", i), "Energy deposited [MeV]", 5000, 0., 10.);
  }

  h_qcss_hitrate0 = new TH1F("qcss_hitrate0", "Hit distributions", 100, 0., 100.);
  h_qcss_hitrate1 = new TH1F("qcss_hitrate1", "Hit distributions", 100, 0., 100.);
  h_qcss_hitrate2 = new TH1F("qcss_hitrate2", "Hit distributions", 100, 0., 100.);
  h_qcss_hitrate1W = new TH1F("qcss_hitrate1W", "Hit distributions", 100, 0., 100.);
  h_qcss_hitrate2W = new TH1F("qcss_hitrate2W", "Hit distributions", 100, 0., 100.);

  h_qcss_hitrate0->Sumw2();
  h_qcss_hitrate1->Sumw2();
  h_qcss_hitrate1W->Sumw2();
  h_qcss_hitrate2->Sumw2();
  h_qcss_hitrate2W->Sumw2();

  h_qcss_rs_hitrate1 = new TH2F("qcss_rs_hitrate1", "Hit distributions vs rs", 100, 0., 100., 12, 0., 12.);
  h_qcss_rs_hitrate2 = new TH2F("qcss_rs_hitrate2", "Hit distributions vs rs", 100, 0., 100., 12, 0., 12.);
  h_qcss_rs_hitrate1W = new TH2F("qcss_rs_hitrate1W", "Hit distributions vs rs", 100, 0., 100., 12, 0., 12.);
  h_qcss_rs_hitrate2W = new TH2F("qcss_rs_hitrate2W", "Hit distributions vs rs", 100, 0., 100., 12, 0., 12.);

  h_qcss_rs_hitrate1->Sumw2();
  h_qcss_rs_hitrate1W->Sumw2();
  h_qcss_rs_hitrate2->Sumw2();
  h_qcss_rs_hitrate2W->Sumw2();

  for (int i = 0; i < 40; i++) {
    h_qcss_rate1[i] = new TH1F(TString::Format("qcss_rate1_%d", i), "PE distributions", 500, 0., 500.);
    h_qcss_rate2[i] = new TH1F(TString::Format("qcss_rate2_%d", i), "PE distributions", 500, 0., 500.);
    h_qcss_rate1W[i] = new TH1F(TString::Format("qcss_rate1W_%d", i), "PE distributions", 500, 0., 500.);
    h_qcss_rate2W[i] = new TH1F(TString::Format("qcss_rate2W_%d", i), "PE distributions", 500, 0., 500.);
    h_qcss_pe1[i] = new TH2F(TString::Format("qcss_pe1_%d", i), "PE distributions", 500, 0., 500., 100, 0., 1000.);
    h_qcss_pe2[i] = new TH2F(TString::Format("qcss_pe2_%d", i), "PE distributions", 500, 0., 500., 100, 0., 1000.);
    h_qcss_pe1W[i] = new TH2F(TString::Format("qcss_pe1W_%d", i), "PE distributions", 500, 0., 500., 100, 0., 1000.);
    h_qcss_pe2W[i] = new TH2F(TString::Format("qcss_pe2W_%d", i), "PE distributions", 500, 0., 500., 100, 0., 1000.);

    h_qcss_rs_rate1[i] = new TH2F(TString::Format("qcss_rs_rate1_%d", i), "PE distributions", 500, 0., 500., 12, 0., 12.);
    h_qcss_rs_rate2[i] = new TH2F(TString::Format("qcss_rs_rate2_%d", i), "PE distributions", 500, 0., 500., 12, 0., 12.);
    h_qcss_rs_rate1W[i] = new TH2F(TString::Format("qcss_rs_rate1W_%d", i), "PE distributions", 500, 0., 500., 12, 0., 12.);
    h_qcss_rs_rate2W[i] = new TH2F(TString::Format("qcss_rs_rate2W_%d", i), "PE distributions", 500, 0., 500., 12, 0., 12.);

    h_qcss_rate1[i]->Sumw2();
    h_qcss_rate2[i]->Sumw2();
    h_qcss_rate1W[i]->Sumw2();
    h_qcss_rate2W[i]->Sumw2();
    h_qcss_rs_rate1[i]->Sumw2();
    h_qcss_rs_rate2[i]->Sumw2();
    h_qcss_rs_rate1W[i]->Sumw2();
    h_qcss_rs_rate2W[i]->Sumw2();
    h_qcss_pe1[i]->Sumw2();
    h_qcss_pe2[i]->Sumw2();
    h_qcss_pe1W[i]->Sumw2();
    h_qcss_pe2W[i]->Sumw2();
  }
}


void QcsmonitorStudyModule::initialize()
{
  B2INFO("QcsmonitorStudyModule: Initialize");

  REG_HISTOGRAM

  //get QCSMONITORS paramters ie energy threshold
  getXMLData();
}

void QcsmonitorStudyModule::beginRun()
{
}

void QcsmonitorStudyModule::event()
{
  //Here comes the actual event processing
  StoreArray<QcsmonitorSimHit>  SimHits;
  StoreArray<QcsmonitorHit> Hits;
  StoreArray<SADMetaHit> MetaHits;

  double rate = 0;
  int ring_section = -1;
  for (const auto& MetaHit : MetaHits) {
    rate = MetaHit.getrate();
    ring_section = MetaHit.getring_section() - 1;
  }

  //number of entries in SimHits
  int nSimHits = SimHits.getEntries();

  //loop over all SimHit entries
  for (int i = 0; i < nSimHits; i++) {
    QcsmonitorSimHit* aHit = SimHits[i];
    int detNB = aHit->getCellId();
    if (detNB < 40) {
      //int trkID = aHit->getTrackId();
      int pdg = aHit->getPDGCode();
      double Edep = aHit->getEnergyDep() * 1e3; //GeV -> MeV
      double tof = aHit->getFlightTime(); //ns

      h_qcss_hitrate0->Fill(detNB);
      h_qcss_Evtof1[detNB]->Fill(tof, Edep);
      if (pdg == Const::photon.getPDGCode()) h_qcss_Evtof2[detNB]->Fill(tof, Edep);
      else if (fabs(pdg) == Const::electron.getPDGCode()) h_qcss_Evtof3[detNB]->Fill(tof, Edep);
      else h_qcss_Evtof4[detNB]->Fill(tof, Edep);
      if (Edep > m_Ethres) {
        h_qcss_edep[detNB]->Fill(Edep);
        h_Wqcss_edep[detNB]->Fill(Edep, rate);
      }
    }
  }

  for (const auto& Hit : Hits) {
    const int detNb = Hit.getdetNb();
    if (detNb < 40) {
      const int timebin = Hit.gettime();
      const float edep = Hit.getedep();
      const float pe = Hit.getPE();
      h_qcss_hitrate1->Fill(detNb);
      h_qcss_hitrate1W->Fill(detNb, rate);
      h_qcss_rate1[detNb]->Fill(pe);
      h_qcss_rate1W[detNb]->Fill(pe, rate);
      h_qcss_rs_rate1[detNb]->Fill(pe, ring_section);
      h_qcss_rs_rate1W[detNb]->Fill(pe, ring_section, rate);
      h_qcss_rs_hitrate1->Fill(detNb, ring_section);
      h_qcss_rs_hitrate1W->Fill(detNb, ring_section, rate);
      h_qcss_pe1[detNb]->Fill(timebin, pe);
      h_qcss_pe1W[detNb]->Fill(timebin, pe, rate);
      if (edep > m_Ethres) {
        h_qcss_hitrate2->Fill(detNb);
        h_qcss_hitrate2W->Fill(detNb, rate);
        h_qcss_rate2[detNb]->Fill(pe);
        h_qcss_rate2W[detNb]->Fill(pe, rate);
        h_qcss_rs_rate2[detNb]->Fill(pe, ring_section);
        h_qcss_rs_rate2W[detNb]->Fill(pe, ring_section, rate);
        h_qcss_rs_hitrate2->Fill(detNb, ring_section);
        h_qcss_rs_hitrate2W->Fill(detNb, ring_section, rate);
        h_qcss_pe2[detNb]->Fill(timebin, pe);
        h_qcss_pe2W[detNb]->Fill(timebin, pe, rate);
      }
    }
  }

}

//read energy threshold from QCSMONITOR.xml
void QcsmonitorStudyModule::getXMLData()
{
  GearDir content = GearDir("/Detector/DetectorComponent[@name=\"QCSMONITOR\"]/Content/");
  m_Ethres = content.getDouble("Ethres");

  B2INFO("QcsmonitorStudy");
}


void QcsmonitorStudyModule::endRun()
{



}

void QcsmonitorStudyModule::terminate()
{
}


