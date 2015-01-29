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
#include <mdst/dataobjects/MCParticle.h>
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
  //addParam("sampletime", m_sampletime, "The sample time of the input, in us", 10000);
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

  h_NeutronEnergy = new TH1F("Neutron_Energy", "Energy of Neutrons;GeV", 100, 0, 1.5);
  h_NeutronKE = new TH1F("NeutronKE", "Kinetic Energy of Neutrons;MeV", 100, 0, 10);
  h_Edep1H3H = new TH1F("Edep1H3H", "Energy deposited by Proton and Tritium; MeV", 100, 0.7, 0.8);
  h_Edep1H3H_detNB = new TH1F("Edep1H3H_tube", "Energy deposited by Proton and Tritium in each tube;Tube Num; MeV", 8, -0.5, 7.5);
  h_Edep1H = new TH1F("Edep1H", "Energy deposited by Protons;MeV", 100, 0, 0.7);
  h_Edep3H = new TH1F("Edep3H", "Energy deposited by Tritiums;MeV", 100, 0, 0.4);

  h_NeutronMomentum = new TH1F("NeutronMomentum", "Momentum of neutrons;GeV", 100, 0, 0.5);
  h_NeutronPhi = new TH1F("NeutronPhi", "Phi of neutrons", 100, 0, 360);
  h_NeutronTheta = new TH1F("NeutronTheta", "Theta of Neutrons", 100, 0, 180);
  h_NeutronProdX = new TH1F("NeutronProdX", "X coordinate of neutron production vertex", 200, -100, 100);
  h_NeutronProdY = new TH1F("NeutronProdY", "Y coordinate of neutron production vertex", 200, -100, 100);
  h_NeutronProdZ = new TH1F("NeutronProdZ", "Z coordinate of neutron production vertex", 200, -100, 100);


  h_TotEdep = new TH1F("TotEdep", "Total energy deposited;MeV", 100, 0, 1.5);
  h_DetN_Edep = new TH1F("DetN_Edep", "Energy deposited vs detector number;Tube Num;MeV", 8, -0.5, 7.5);
  h_TritiumPositions = new TH3F("NeutronPositions", "Position of Tritium origin", 100, -80, 80, 100, -80, 80, 100, -80, 80);

  h_PulseHeights = new TH1F("PulseHeights", "Pulse height of waveforms", 100, 0, 1);

}


void He3tubeStudyModule::initialize()
{
  B2INFO("He3tubeStudyModule: Initialize");

  REG_HISTOGRAM


  rateCorrection = m_sampletime / 1e6;

  StoreArray<MCParticle>   mcParticles;
  StoreArray<He3tubeSimHit>  simHits;
  RelationArray relMCSimHit(mcParticles, simHits);
  if (!(mcParticles.isRequired() && simHits.isRequired() && relMCSimHit.isRequired())) {
    //Fatal is not neccessary here as the storeArrays should just look
    //empty if not registered but let's make sure everything is present
    B2FATAL("Not all collections found, exiting processing");
  }
}

void He3tubeStudyModule::beginRun()
{
  B2INFO("He3tubeStudyModule: Begin of new run");

}

void He3tubeStudyModule::event()
{
  //Here comes the actual event processing

  StoreArray<MCParticle>   mcParticles;
  StoreArray<He3tubeSimHit>  simHits;
  StoreArray<He3tubeHit> Hits;

  RelationIndex<MCParticle, He3tubeSimHit> relMCSimHit(mcParticles, simHits);

  //initalize various counters
  int lastMCparticle = -1;
  double edepSum = 0;
  double edepSum_1H = 0;
  double edepSum_3H = 0;
  double totedepSum = 0;

  for (int i = 0; i < simHits.getEntries(); i++) {
    He3tubeSimHit* aHit = simHits[i];
    double edep = aHit->getEnergyDep();
    int detNB = aHit->getdetNb();
    int PID = aHit->gettkPDG();

    h_DetN_Edep->Fill(detNB, edep);
    totedepSum = totedepSum + edep;

    //relationship between He3tubeSimHits and MCParticles
    typedef RelationIndex<MCParticle, He3tubeSimHit>::Element relMCSimHit_Element;

    //if particle that deposited energy is a proton or a tritium
    if (PID == 2212 || PID == 1000010030) {
      BOOST_FOREACH(const relMCSimHit_Element & relation, relMCSimHit.getElementsTo(aHit)) {
        int MCParticleIndex = relation.from->getMother()->getIndex();                                        //get mother particle index
        int MotherPID = relation.from->getMother()->getPDG();                                                //get mother's PID
        double MotherE = relation.from->getMother()->getEnergy();                                            //mother's Total Energy
        double MotherMomentum = relation.from->getMother()->getMomentum().Mag();                             //mother's Momentum
        double MotherKE = (MotherMomentum * MotherMomentum / (2 * relation.from->getMother()->getMass())) * 1000; //non-relatavistic kinetic energy
        if (MotherPID == 2112) {                     // if the mother particle is a neutron
          h_Edep1H3H_detNB->Fill(detNB, edep);
          edepSum = edepSum + edep;
        }
        //fill the plots based on neutrons. since we only want one entry per neutron, make sure the particle Index is unique.
        if (MCParticleIndex != lastMCparticle && MotherPID == 2112) {
          h_TritiumPositions->Fill(relation.from->getVertex().x(), relation.from->getVertex().y(), relation.from->getVertex().z());
          B2INFO("MCParticle #" << MCParticleIndex << " with PDG code " << MotherPID << " Created a " << PID);
          nNeutronHits++;
          //fill some histograms
          h_NeutronKE->Fill(MotherKE);
          h_NeutronHits->Fill(detNB);                      //number of neutrons in each tube
          h_NeutronRate->Fill(detNB, 1 / rateCorrection);  //rate in Hz for each tube
          h_NeutronEnergy->Fill(MotherE);
          h_NeutronMomentum->Fill(MotherMomentum);
          h_NeutronPhi->Fill(relation.from->getMother()->getProductionVertex().Phi() * 180 / 3.14159);
          h_NeutronTheta->Fill(relation.from->getMother()->getProductionVertex().Theta() * 180 / 3.14159);
          h_NeutronProdX->Fill(relation.from->getMother()->getProductionVertex().X());
          h_NeutronProdY->Fill(relation.from->getMother()->getProductionVertex().Y());
          h_NeutronProdZ->Fill(relation.from->getMother()->getProductionVertex().Z());

          B2INFO("Mom: " << MotherMomentum);
          B2INFO("X: " << relation.from->getMother()->getProductionVertex().X());
          B2INFO("Y: " << relation.from->getMother()->getProductionVertex().Y());
          B2INFO("Z: " << relation.from->getMother()->getProductionVertex().Z());
          B2INFO("Theta: " << relation.from->getMother()->getProductionVertex().Theta() * 180 / 3.14159);
          B2INFO("Phi: " << relation.from->getMother()->getProductionVertex().Phi() * 180 / 3.14159);



        } else if (MCParticleIndex != lastMCparticle && MotherPID != 2112) {
          B2INFO("MCParticle #" << MCParticleIndex << " with PDG code " << MotherPID << " Created a " << PID);
        }

        lastMCparticle = MCParticleIndex;
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

  }

  if (totedepSum != 0) h_TotEdep->Fill(totedepSum);

  if (edepSum != 0) {
    h_Edep1H3H->Fill(edepSum);
  }

  if (edepSum_1H != 0) {
    h_Edep1H->Fill(edepSum_1H);
  }

  if (edepSum_3H != 0) {
    h_Edep3H->Fill(edepSum_3H);
  }

  //pulse heights of digitized waveforms
  for (int i = 0; i < Hits.getEntries(); i++) {
    He3tubeHit* aHit = Hits[i];
    h_PulseHeights->Fill(aHit->getPeakV());
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


