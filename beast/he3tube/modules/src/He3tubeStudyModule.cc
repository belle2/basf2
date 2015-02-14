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
  h_NeutronRate = new TH1F("NeutronRate", "Neutron Hits per second;Tube ", 8, -0.5, 7.5);

  h_Edep1H3H = new TH1F("Edep1H3H", "Energy deposited by Proton and Tritium; MeV", 100, 0.7, 0.8);
  h_Edep1H3H_detNB = new TH1F("Edep1H3H_tube", "Energy deposited by Proton and Tritium in each tube;Tube Num; MeV", 8, -0.5, 7.5);
  h_Edep1H = new TH1F("Edep1H", "Energy deposited by Protons;MeV", 100, 0, 0.7);
  h_Edep3H = new TH1F("Edep3H", "Energy deposited by Tritiums;MeV", 100, 0, 0.4);
  h_TotEdep = new TH1F("TotEdep", "Total energy deposited;MeV", 100, 0, 1.5);
  h_DetN_Edep = new TH1F("DetN_Edep", "Energy deposited vs detector number;Tube Num;MeV", 8, -0.5, 7.5);

  h_PulseHeights = new TH1F("PulseHeights", "Pulse height of waveforms", 100, 0, 1);

}


void He3tubeStudyModule::initialize()
{
  B2INFO("He3tubeStudyModule: Initialize");

  REG_HISTOGRAM


  rateCorrection = m_sampletime / 1e6;
}

void He3tubeStudyModule::beginRun()
{
  B2INFO("He3tubeStudyModule: Begin of new run");

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

  int detectorNumber = -1;
  bool NeutronProcess = false;

  for (int i = 0; i < simHits.getEntries(); i++) {
    He3tubeSimHit* aHit = simHits[i];
    double edep = aHit->getEnergyDep();
    int detNB = aHit->getdetNb();
    int PID = aHit->gettkPDG();

    h_DetN_Edep->Fill(detNB, edep);
    totedepSum = totedepSum + edep;

    //was the process that created the particle a neutron process?
    if (aHit->getNeuProcess()) {
      B2INFO("Neutron process created a " << PID);

      NeutronProcess = true;
      detectorNumber = detNB;

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
    h_PulseHeights->Fill(aHit->getPeakV());
  }

  if (NeutronProcess) {
    nNeutronHits++;
    h_NeutronHits->Fill(detectorNumber);                      //number of neutrons in each tube
    h_NeutronRate->Fill(detectorNumber, 1 / rateCorrection);  //rate in Hz for each tube
  }

  eventNum++;



}

void He3tubeStudyModule::endRun()
{

  B2INFO("He3tubeStudyModule: End of run");

  B2INFO("# of neutrons: " << nNeutronHits);
  B2INFO("# of 3H hits:  " << n3Hhits);
  B2INFO("# of H hits:   " << nPhits);



}

void He3tubeStudyModule::terminate()
{
  B2INFO("He3tubeStudyModule: Terminate");

}


