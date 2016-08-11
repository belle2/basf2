/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Igal Jaegle                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <beast/beamabort/modules/BeamabortStudyModule.h>
#include <beast/beamabort/dataobjects/BeamabortSimHit.h>
#include <beast/beamabort/dataobjects/BeamabortHit.h>
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/RelationArray.h>
#include <framework/datastore/RelationIndex.h>
#include <framework/logging/Logger.h>
#include <framework/gearbox/Gearbox.h>
#include <framework/gearbox/GearDir.h>
#include <cmath>
#include <boost/foreach.hpp>


#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

// ROOT
#include <TVector3.h>
#include <TRandom.h>
#include <TH1.h>
#include <TH2.h>
#include <TH3.h>
#include <TFile.h>
#include <TMath.h>

#include <generators/SAD/dataobjects/SADMetaHit.h>

using namespace std;

using namespace Belle2;
using namespace beamabort;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(BeamabortStudy)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

BeamabortStudyModule::BeamabortStudyModule() : HistoModule()
{
  // Set module properties
  setDescription("Study module for Beamaborts (BEAST)");

  //Default values are set here. New values can be in PINDIODE.xml.
  addParam("WorkFunction", m_WorkFunction, "Convert eV to e [e/eV] ", 1.12);
  addParam("FanoFactor", m_FanoFactor, "e resolution ", 0.1);
}

BeamabortStudyModule::~BeamabortStudyModule()
{
}

//This module is a histomodule. Any histogram created here will be saved by the HistoManager module
void BeamabortStudyModule::defineHisto()
{
  //Default values are set here. New values can be in BEAMABORT.xml.
  for (int i = 0; i < 8; i++) {
    h_dose[i] = new TH1F(TString::Format("h_dose_%d", i), "", 1000, 0., 1000.);
    h_amp[i] = new TH1F(TString::Format("h_amp_%d", i), "", 10000, 0., 10000.);
    h_time[i] = new TH1F(TString::Format("h_time_%d", i), "", 1000, 0., 100.);
    h_vtime[i] = new TH1F(TString::Format("h_vtime_%d", i), "", 1000, 0., 100.);
    h_idose[i] = new TH1F(TString::Format("h_idose_%d", i), "", 1000, 0., 1000.);
    h_iamp[i] = new TH1F(TString::Format("h_iamp_%d", i), "", 10000, 0., 10000.);
    h_itime[i] = new TH1F(TString::Format("h_itime_%d", i), "", 1000, 0., 100.);
    h_ivtime[i] = new TH1F(TString::Format("h_ivtime_%d", i), "", 1000, 0., 100.);
    h_Amp[i] = new TH1F(TString::Format("h_Amp_%d", i), "", 100000, 0., 100000.);
    h_edep[i] = new TH1F(TString::Format("h_edep_%d", i), "", 4000, 0., 4000.);
  }
  h_s = new TH1F("h_s", "", 4000, -200., 200.);
  h_s_cut = new TH1F("h_s_cut", "", 4000, -200., 200.);
}


void BeamabortStudyModule::initialize()
{
  B2INFO("BeamabortStudyModule: Initialize");

  //read beamabort xml file
  getXMLData();

  REG_HISTOGRAM

}

void BeamabortStudyModule::beginRun()
{
}

void BeamabortStudyModule::event()
{
  //Here comes the actual event processing
  double curr[8];
  double Edep[8];
  for (int i = 0; i < 8; i++) {
    curr[i] = 0;
    Edep[i] = 0;
  }
  StoreArray<BeamabortSimHit> SimHits;
  StoreArray<BeamabortHit> Hits;
  StoreArray<SADMetaHit> SADtruth;

  //Skip events with no Hits
  if (SimHits.getEntries() == 0) {
    return;
  }

  int nSAD = SADtruth.getEntries();
  //Bool_t Reject = false;
  for (int i = 0; i < nSAD; i++) {
    SADMetaHit* aHit = SADtruth[i];
    double s = aHit->gets();
    //double rate = aHit->getrate();
    h_s->Fill(-s);
    if ((-33.0 <= -s && -s <= -30.0) || (19.0 <= -s && -s <= 23.0)) {
      h_s_cut->Fill(-s);
      //Reject = true;
    }
  }
  int nSimHits = SimHits.getEntries();
  //if(!Reject) {
  for (int i = 0; i < nSimHits; i++) {
    BeamabortSimHit* aHit = SimHits[i];
    int detNb = aHit->getCellId();
    if (detNb < 8) {
      double edep = aHit->getEnergyDep();
      double time = aHit->getFlightTime();
      double meanEl = edep / m_WorkFunction * 1e9; //GeV to eV
      double sigma = sqrt(m_FanoFactor * meanEl); //sigma in electron
      int NbEle = (int)gRandom->Gaus(meanEl, sigma); //electron number
      double Amp = NbEle / (6.25 * 1e18); // A x s
      Edep[detNb] += edep;
      curr[detNb] += Amp;
      h_dose[detNb]->Fill(edep * 1e6); //GeV to keV
      h_amp[detNb]->Fill(Amp * 1e15); //A x s -> mA x s
      h_time[detNb]->Fill(time);
      h_vtime[detNb]->Fill(time, Amp * 1e15);
    }
  }

  for (int i = 0; i < 8; i++) {
    if (curr[i] > 0 && Edep[i] > 0) {
      h_Amp[i]->Fill(curr[i] * 1e15);
      h_edep[i]->Fill(Edep[i] * 1e6);
    }
  }
  int nHits = Hits.getEntries();
  for (int i = 0; i < nHits; i++) {
    BeamabortHit* aHit = Hits[i];
    int detNb = aHit->getdetNb();
    double edep = aHit->getedep();
    double current = aHit->getI();
    double time = aHit->gettime();
    h_idose[detNb]->Fill(edep * 1e6); //GeV to keV
    h_iamp[detNb]->Fill(current * 1e6); //I to uI
    h_itime[detNb]->Fill(time);
    h_ivtime[detNb]->Fill(time, current * 1e6);
  }

  //}
}
//read tube centers, impulse response, and garfield drift data filename from BEAMABORT.xml
void BeamabortStudyModule::getXMLData()
{
  GearDir content = GearDir("/Detector/DetectorComponent[@name=\"BEAMABORT\"]/Content/");
  /*
  //get the location of the tubes
  BOOST_FOREACH(const GearDir & activeParams, content.getNodes("Active")) {

    PINCenter.push_back(TVector3(activeParams.getLength("z_beamabort"), activeParams.getLength("r_beamabort"),
                                 activeParams.getLength("Phi")));
    nPIN++;
  }
  */
  m_WorkFunction = content.getDouble("WorkFunction");
  m_FanoFactor = content.getDouble("FanoFactor");

  B2INFO("BeamabortStudy");

}
void BeamabortStudyModule::endRun()
{


}

void BeamabortStudyModule::terminate()
{
}


