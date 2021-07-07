/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <beast/pindiode/modules/PindiodeStudyModule.h>
#include <beast/pindiode/dataobjects/PindiodeSimHit.h>
#include <beast/pindiode/dataobjects/PindiodeHit.h>
#include <generators/SAD/dataobjects/SADMetaHit.h>
#include <framework/datastore/StoreArray.h>
#include <framework/logging/Logger.h>
#include <framework/gearbox/GearDir.h>
#include <cmath>

#include <fstream>
#include <string>

// ROOT
#include <TRandom.h>
#include <TH1.h>
#include <TH2.h>

using namespace std;

using namespace Belle2;
using namespace pindiode;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(PindiodeStudy)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

PindiodeStudyModule::PindiodeStudyModule() : HistoModule()
{
  // Set module properties
  setDescription("Study module for Pindiodes (BEAST)");

  //Default values are set here. New values can be in PINDIODE.xml.
  addParam("CrematGain", m_CrematGain, "Charge sensitive preamplifier gain [volts/C] ", 1.4);
  addParam("WorkFunction", m_WorkFunction, "Convert eV to e [e/eV] ", 1.12);
  addParam("FanoFactor", m_FanoFactor, "e resolution ", 0.1);
}

PindiodeStudyModule::~PindiodeStudyModule()
{
}

//This module is a histomodule. Any histogram created here will be saved by the HistoManager module
void PindiodeStudyModule::defineHisto()
{
  //Default values are set here. New values can be in PINDIODE.xml.
  for (int i = 0; i < 4; i++) {
    h_pin_rate[i] = new TH1F(TString::Format("pin_rate_%d", i), "Count", 64, 0., 64.);
    h_pin_rate[i]->Sumw2();
  }
  for (int i = 0; i < 2; i++) {
    h_pin_rs_rate[i] = new TH2F(TString::Format("pin_rs_rate_%d", i), "Count vs. ring section", 64, 0., 64., 12, 0., 12.);
    h_pin_rs_rate[i]->Sumw2();
  }
  for (int i = 0; i < 64; i++) {
    h_pin_dose1[i] = new TH1F(TString::Format("pin_dose1_%d", i), "", 10000, 0., 10000.);
    h_pin_dose2[i] = new TH1F(TString::Format("pin_dose2_%d", i), "", 10000, 0., 10000.);
    h_pin_dose1Weight[i] = new TH1F(TString::Format("pin_dose1Weight_%d", i), "", 10000, 0., 10000.);
    h_pin_dose2Weight[i] = new TH1F(TString::Format("pin_dose2Weight_%d", i), "", 10000, 0., 10000.);
    h_pin_volt[i] = new TH1F(TString::Format("pin_volt_%d", i), "", 10000, 0., 100.);
    h_pin_time[i] = new TH1F(TString::Format("pin_time_%d", i), "", 1000, 0., 100.);
    h_pin_vtime[i] = new TH1F(TString::Format("pin_vtime_%d", i), "", 1000, 0., 100.);

    h_pin_idose[i] = new TH1F(TString::Format("pin_idose_%d", i), "", 10000, 0., 10000.);
    h_pin_idoseWeight[i] = new TH1F(TString::Format("pin_idoseWeight_%d", i), "", 10000, 0., 10000.);

    h_pin_rs_idose[i] = new TH2F(TString::Format("pin_rs_idose_%d", i), "", 10000, 0., 10000., 12, 0., 12.);
    h_pin_rs_idoseWeight[i] = new TH2F(TString::Format("pin_rs_idoseWeight_%d", i), "", 10000, 0., 10000., 12, 0., 12.);

    h_pin_ivolt[i] = new TH1F(TString::Format("pin_ivolt_%d", i), "", 10000, 0., 100.);
    h_pin_itime[i] = new TH1F(TString::Format("pin_itime_%d", i), "", 1000, 0., 100.);
    h_pin_ivtime[i] = new TH1F(TString::Format("pin_ivtime_%d", i), "", 1000, 0., 100.);

    h_pin_dose1[i]->Sumw2();
    h_pin_dose2[i]->Sumw2();
    h_pin_dose1Weight[i]->Sumw2();
    h_pin_dose2Weight[i]->Sumw2();
    h_pin_idose[i]->Sumw2();
    h_pin_idoseWeight[i]->Sumw2();
    h_pin_rs_idose[i]->Sumw2();
    h_pin_rs_idoseWeight[i]->Sumw2();
  }

}


void PindiodeStudyModule::initialize()
{
  B2INFO("PindiodeStudyModule: Initialize");

  //read pindiode xml file
  getXMLData();

  REG_HISTOGRAM

}

void PindiodeStudyModule::beginRun()
{
}

void PindiodeStudyModule::event()
{
  //Here comes the actual event processing

  StoreArray<PindiodeSimHit>  SimHits;
  StoreArray<PindiodeHit> Hits;
  StoreArray<SADMetaHit> MetaHits;

  //Look at the meta data to extract IR rate and scattering ring section
  double rate = 0;
  int ring_section = -1;
  int section_ordering[12] = {1, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2};
  for (const auto& MetaHit : MetaHits) {
    rate = MetaHit.getrate();
    double sad_ssraw = MetaHit.getssraw();
    double ssraw = 0;
    if (sad_ssraw >= 0) ssraw = sad_ssraw / 100.;
    else ssraw = 3000. + sad_ssraw / 100.;
    //else if (sad_ssraw < 0) ssraw = 3000. + sad_ssraw / 100.;
    ring_section = section_ordering[(int)((ssraw) / 250.)] - 1;
    //ring_section = MetaHit.getring_section() - 1;
  }

  //Skip events with no Hits
  if (SimHits.getEntries() == 0) {
    return;
  }

  for (const auto& SimHit : SimHits) {
    int detNb = SimHit.getCellId();
    if (detNb < 64) {
      double edep = SimHit.getEnergyDep();
      double time = SimHit.getFlightTime();
      //int PDG = aHit->getPDGCode();
      const double meanEl = edep / m_WorkFunction * 1e9; //GeV to eV
      const double sigma = sqrt(m_FanoFactor * meanEl); //sigma in electron
      const int NbEle = (int)gRandom->Gaus(meanEl, sigma); //electron number
      double volt = NbEle * 1.602176565e-19 * m_CrematGain * 1e12; // volt
      h_pin_dose1[detNb]->Fill(edep * 1e6); //GeV to keV
      h_pin_dose1Weight[detNb]->Fill(edep * 1e6, rate); //GeV to keV
      if ((edep * 1e9) > m_WorkFunction) {
        h_pin_dose2[detNb]->Fill(edep * 1e6); //GeV to keV
        h_pin_dose2Weight[detNb]->Fill(edep * 1e6, rate); //GeV to keV
        h_pin_volt[detNb]->Fill(volt * 1e3); //V to mV
        h_pin_time[detNb]->Fill(time);
        h_pin_vtime[detNb]->Fill(time, volt);
        h_pin_rate[0]->Fill(detNb);
        h_pin_rate[1]->Fill(detNb, rate);
      }
    }
  }

  for (const auto& Hit : Hits) {
    int detNb = Hit.getdetNb();
    if (detNb < 64) {
      double edep = Hit.getedep();
      double volt = Hit.getV();
      double time = Hit.gettime();
      h_pin_idose[detNb]->Fill(edep); //keV
      h_pin_idoseWeight[detNb]->Fill(edep, rate); //keV
      h_pin_ivolt[detNb]->Fill(volt * 1e3); //V to mV
      h_pin_itime[detNb]->Fill(time);
      h_pin_ivtime[detNb]->Fill(time, volt);
      h_pin_rate[2]->Fill(detNb);
      h_pin_rate[3]->Fill(detNb, rate);

      h_pin_rs_rate[0]->Fill(detNb, ring_section);
      h_pin_rs_rate[1]->Fill(detNb, ring_section, rate);
      h_pin_rs_idose[detNb]->Fill(edep, ring_section); //keV
      h_pin_rs_idoseWeight[detNb]->Fill(edep, ring_section, rate); //keV
    }
  }


}
//read tube centers, impulse response, and garfield drift data filename from PINDIODE.xml
void PindiodeStudyModule::getXMLData()
{
  GearDir content = GearDir("/Detector/DetectorComponent[@name=\"PINDIODE\"]/Content/");
  /*
  //get the location of the tubes
  BOOST_FOREACH(const GearDir & activeParams, content.getNodes("Active")) {

    PINCenter.push_back(TVector3(activeParams.getLength("z_pindiode"), activeParams.getLength("r_pindiode"),
                                 activeParams.getLength("Phi")));
    nPIN++;
  }
  */
  m_CrematGain = content.getDouble("CrematGain");
  m_WorkFunction = content.getDouble("WorkFunction");
  m_FanoFactor = content.getDouble("FanoFactor");

  B2INFO("PinDigitizer");

}
void PindiodeStudyModule::endRun()
{


}

void PindiodeStudyModule::terminate()
{
}


