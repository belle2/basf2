/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Sam de Jong                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <beast/he3tube/modules/He3tubeStudyModule.h>
#include <beast/he3tube/dataobjects/He3tubeSimHit.h>
#include <beast/he3tube/dataobjects/He3tubeHit.h>
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/RelationArray.h>
#include <framework/datastore/RelationIndex.h>
#include <framework/logging/Logger.h>
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

int eventNum = 0;

using namespace std;

using namespace Belle2;
using namespace he3tube;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(He3tubeStudy)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

He3tubeStudyModule::He3tubeStudyModule() : HistoModule()
{
  // Set module properties
  setDescription("Study module for He3tubes (BEAST)");

  // Parameter definitions
  addParam("sampleTime", m_sampletime, "Length of sample, in us", 10000);

}

He3tubeStudyModule::~He3tubeStudyModule()
{
}

//This module is a histomodule. Any histogram created here will be saved by the HistoManager module
void He3tubeStudyModule::defineHisto()
{


  h_NeutronHits = new TH1F("NeutronHits", "Neutron Hits;Tube ", 8, -0.5, 7.5);
  h_NeutronRate = new TH1F("NeutronRate", "Neutron Hits per second;Tube; Rate (Hz)", 8, -0.5, 7.5);

  h_Edep1H3H =       new TH1F("Edep1H3H"     , "Energy deposited by Proton and Tritium; MeV", 100, 0.7, 0.8);
  h_Edep1H3H_detNB = new TH1F("Edep1H3H_tube", "Energy deposited by Proton and Tritium in each tube;Tube Num; MeV", 8, -0.5, 7.5);
  h_Edep1H =         new TH1F("Edep1H"       , "Energy deposited by Protons;MeV", 100, 0, 0.7);
  h_Edep3H =         new TH1F("Edep3H"       , "Energy deposited by Tritiums;MeV", 100, 0, 0.4);
  h_TotEdep =        new TH1F("TotEdep"      , "Total energy deposited;MeV", 100, 0, 1.5);
  h_DetN_Edep =      new TH1F("DetN_Edep"    , "Energy deposited vs detector number;Tube Num;MeV", 8, -0.5, 7.5);

  h_PulseHeights_NotNeutron = new TH1F("PulseHeights_NotNeutron", "Pulse height of waveforms from non-neutron events", 100, 0, 1);
  h_PulseHeights_Neutron =    new TH1F("PulseHeights_Neutron"   , "Pulse height of waveforms from neutron events", 100, 0, 1);

}


void He3tubeStudyModule::initialize()
{
  B2INFO("He3tubeStudyModule: Initialize");

  REG_HISTOGRAM

  //convert sample time into rate in Hz
  rateCorrection = m_sampletime / 1e6;
}

void He3tubeStudyModule::beginRun()
{
}

void He3tubeStudyModule::event()
{
  //Here comes the actual event processing

  StoreArray<He3tubeSimHit>  simHits;
  StoreArray<He3tubeHit> Hits;

  //initalize various counters
  double edepSum = 0;
  double edepSum_1H = 0;
  double edepSum_3H = 0;
  double totedepSum = 0;

  //int detectorNumber = -1;
  bool NeutronProcess[8] = {false};

  for (int i = 0; i < simHits.getEntries(); i++) {
    He3tubeSimHit* aHit = simHits[i];
    double edep = aHit->getEnergyDep();
    int detNB = aHit->getdetNb();
    int PID = aHit->gettkPDG();

    h_DetN_Edep->Fill(detNB, edep);
    totedepSum = totedepSum + edep;

    //was the process that created the particle a neutron process?
    if (aHit->getNeuProcess()) {
      NeutronProcess[detNB] = true;
      h_Edep1H3H_detNB->Fill(detNB, edep);
      edepSum = edepSum + edep;

    }

    //energy deposited by protons
    if (PID == 2212) {
      nPhits++;
      edepSum_1H = edepSum_1H + edep;
    }
    //energy deposited by tritiums
    if (PID == 1000010030) {
      n3Hhits++;
      edepSum_3H = edepSum_3H + edep;
    }


  }

  if (totedepSum != 0)  h_TotEdep->Fill(totedepSum);
  if (edepSum != 0)     h_Edep1H3H->Fill(edepSum);
  if (edepSum_1H != 0)  h_Edep1H->Fill(edepSum_1H);
  if (edepSum_3H != 0)  h_Edep3H->Fill(edepSum_3H);

  //pulse heights of digitized waveforms
  for (int i = 0; i < Hits.getEntries(); i++) {
    He3tubeHit* aHit = Hits[i];
    if (NeutronProcess[aHit->getdetNb()]) {
      h_PulseHeights_Neutron->Fill(aHit->getPeakV());
    } else h_PulseHeights_NotNeutron->Fill(aHit->getPeakV());
  }

  for (int detNB = 0; detNB < 8; detNB++) {
    if (NeutronProcess[detNB]) {
      B2DEBUG(80, "He3tubeStudyModule: Neutron in tube #" << detNB);
      nNeutronHits++;
      h_NeutronHits->Fill(detNB);                      //number of neutrons in each tube
      h_NeutronRate->Fill(detNB, 1 / rateCorrection);  //rate in Hz for each tube
    }
  }

  eventNum++;



}

void He3tubeStudyModule::endRun()
{

  B2RESULT("He3tubeStudyModule: # of neutrons: " << nNeutronHits);
  B2RESULT("                    # of 3H hits:  " << n3Hhits);
  B2RESULT("                    # of H hits:   " << nPhits);



}

void He3tubeStudyModule::terminate()
{
}


