/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <beast/beamabort/modules/BeamabortStudyModule.h>
#include <beast/beamabort/dataobjects/BeamabortSimHit.h>
#include <beast/beamabort/dataobjects/BeamabortHit.h>
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

void BeamabortStudyModule::defineHisto()
{
  B2INFO("BeamabortStudyModule: Initialize");

  for (int i = 0; i < 8; i++) {
    h_dia_dose[i] = new TH1F(TString::Format("dia_dose_%d", i), "", 10000, 0., 10000.);
    h_dia_dose[i]->Sumw2();
  }

  //read beamabort xml file

}

//This module is a histomodule. Any histogram created here will be saved by the HistoManager module
void BeamabortStudyModule::initialize()
{
  getXMLData();



  //Default values are set here. New values can be in BEAMABORT.xml.
  for (int i = 0; i < 4; i++) {
    h_dia_rate[i] = new TH1F(TString::Format("dia_rate_%d", i), "Count", 8, 0., 8.);
    h_dia_rate[i]->Sumw2();
  }
  for (int i = 0; i < 2; i++) {
    h_dia_rs_rate[i] = new TH2F(TString::Format("dia_rs_rate_%d", i), "Count vs ring section", 8, 0., 8., 12, 0., 12.);
    h_dia_rs_rate[i]->Sumw2();
  }

  for (int i = 0; i < 8; i++) {
    h_dia_doseWeight[i] = new TH1F(TString::Format("dia_doseWeight_%d", i), "", 10000, 0., 10000.);
    h_dia_amp[i] = new TH1F(TString::Format("dia_amp_%d", i), "", 10000, 0., 10000.);
    h_dia_time[i] = new TH1F(TString::Format("dia_time_%d", i), "", 1000, 0., 100.);
    h_dia_vtime[i] = new TH1F(TString::Format("dia_vtime_%d", i), "", 1000, 0., 100.);
    h_dia_idose[i] = new TH1F(TString::Format("dia_idose_%d", i), "", 10000, 0., 10000.);
    h_dia_idoseWeight[i] = new TH1F(TString::Format("dia_idoseWeight_%d", i), "", 10000, 0., 10000.);
    h_dia_rs_idose[i] = new TH2F(TString::Format("dia_rs_idose_%d", i), "", 10000, 0., 10000., 12, 0., 12.);
    h_dia_rs_idoseWeight[i] = new TH2F(TString::Format("dia_rs_idoseWeight_%d", i), "", 10000, 0., 10000., 12, 0., 12.);
    h_dia_iamp[i] = new TH1F(TString::Format("dia_iamp_%d", i), "", 10000, 0., 10000.);
    h_dia_itime[i] = new TH1F(TString::Format("dia_itime_%d", i), "", 1000, 0., 100.);
    h_dia_ivtime[i] = new TH1F(TString::Format("dia_ivtime_%d", i), "", 1000, 0., 100.);
    h_dia_Amp[i] = new TH1F(TString::Format("dia_Amp_%d", i), "", 100000, 0., 100000.);
    h_dia_edep[i] = new TH1F(TString::Format("dia_edep_%d", i), "", 4000, 0., 4000.);

    h_dia_doseWeight[i]->Sumw2();
    h_dia_idose[i]->Sumw2();
    h_dia_idoseWeight[i]->Sumw2();
    h_dia_rs_idose[i]->Sumw2();
    h_dia_rs_idoseWeight[i]->Sumw2();
  }

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
  StoreArray<SADMetaHit> MetaHits;

  //Skip events with no Hits
  if (SimHits.getEntries() == 0) {
    return;
  }

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

  for (const auto& SimHit : SimHits) {
    int detNb = SimHit.getCellId();
    if (detNb < 8) {
      double edep = SimHit.getEnergyDep();
      double time = SimHit.getFlightTime();
      double meanEl = edep / m_WorkFunction * 1e9; //GeV to eV
      double sigma = sqrt(m_FanoFactor * meanEl); //sigma in electron
      int NbEle = (int)gRandom->Gaus(meanEl, sigma); //electron number
      double Amp = NbEle / (6.25 * 1e18); // A x s
      Edep[detNb] += edep;
      curr[detNb] += Amp;
      h_dia_dose[detNb]->Fill(edep * 1e6); //GeV to keV
      h_dia_doseWeight[detNb]->Fill(edep * 1e6, rate); //GeV to keV
      h_dia_amp[detNb]->Fill(Amp * 1e15); //A x s -> mA x s
      h_dia_time[detNb]->Fill(time);
      h_dia_vtime[detNb]->Fill(time, Amp * 1e15);
      h_dia_rate[0]->Fill(detNb);
      h_dia_rate[1]->Fill(detNb, rate);
    }
  }

  for (int i = 0; i < 4; i++) {
    if (curr[i] > 0 && Edep[i] > 0) {
      h_dia_Amp[i]->Fill(curr[i] * 1e15);
      h_dia_edep[i]->Fill(Edep[i] * 1e6);
    }
  }

  for (const auto& Hit : Hits) {
    int detNb = Hit.getdetNb();
    double edep = Hit.getedep();
    double current = Hit.getI();
    double time = Hit.gettime();
    h_dia_idose[detNb]->Fill(edep); //keV
    h_dia_idoseWeight[detNb]->Fill(edep, rate); //keV
    h_dia_rs_idose[detNb]->Fill(edep, ring_section); //keV
    h_dia_rs_idoseWeight[detNb]->Fill(edep, ring_section, rate); //keV
    h_dia_iamp[detNb]->Fill(current * 1e6); //I to uI
    h_dia_itime[detNb]->Fill(time);
    h_dia_ivtime[detNb]->Fill(time, current * 1e6);
    h_dia_rate[2]->Fill(detNb);
    h_dia_rate[3]->Fill(detNb, rate);

    h_dia_rs_rate[0]->Fill(detNb, ring_section);
    h_dia_rs_rate[1]->Fill(detNb, ring_section, rate);
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


