/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Sam de Jong                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <beast/he3tube/modules/He3DigitizerModule.h>
#include <beast/he3tube/dataobjects/He3tubeSimHit.h>

#include <mdst/dataobjects/MCParticle.h>
#include <framework/datastore/RelationArray.h>
#include <framework/datastore/RelationIndex.h>
#include <framework/logging/Logger.h>
#include <framework/gearbox/Gearbox.h>
#include <framework/gearbox/GearDir.h>
#include <framework/gearbox/Unit.h>

//c++
#include <cmath>
#include <boost/foreach.hpp>
#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <stdlib.h>

// ROOT
#include <TVector3.h>
#include <TFile.h>
#include <TTree.h>
#include <TString.h>
#include <TSystem.h>
#include <TRandom.h>

using namespace std;
using namespace Belle2;
using namespace he3tube;


//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(He3Digitizer)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

He3DigitizerModule::He3DigitizerModule() : Module()
{
  // Set module properties
  setDescription("He3tube digitizer module");

  addParam("conversionFactor", m_ConversionFactor,
           "Conversion factor for pulse heights", 0.303132019);
  addParam("useMCParticles", m_mcpExist, "Use MCParticles to determine if neutrons are present", true);

}

He3DigitizerModule::~He3DigitizerModule()
{
}

void He3DigitizerModule::initialize()
{
  B2INFO("He3Digitizer: Initializing");
  m_he3tubeHit.registerInDataStore();

  StoreArray<MCParticle>   mcParticles;
  StoreArray<He3tubeSimHit>  simHits;
  RelationArray relMCSimHit(mcParticles, simHits);

  getXMLData();
}

void He3DigitizerModule::beginRun()
{
}

void He3DigitizerModule::event()
{
  B2DEBUG(250, "Beginning of event method");

  StoreArray<He3tubeSimHit> He3SimHits;
  StoreArray<He3tubeHit> He3Hits;

  auto peak = new double[numOfTubes]();
  auto lowTime = new double[numOfTubes]();  //earliest deposit in each tube
  std::fill_n(lowTime, numOfTubes, 9999999999999);
  auto edepDet = new double[numOfTubes]();  //total energy deposited per tube
  auto NbEle_tot = new double[numOfTubes]();

  //skip events with no He3SimHits, but continue the event counter
  if (He3SimHits.getEntries() == 0) {
    B2DEBUG(250, "Skipping event #" << Event << " since there are no sim hits");
    Event++;
    return;
  }

  auto definiteNeutron = new bool[numOfTubes]();
  //vector<int> neutronIDs;

  if (m_mcpExist) { //if the mcparticles exist, we know which simhits are from neutron events

    StoreArray<MCParticle>   mcParticles;
    RelationIndex<MCParticle, He3tubeSimHit> relMCSimHit(mcParticles, He3SimHits);

    int nMCParticles = mcParticles.getEntries();
    for (int i = 0; i < nMCParticles; ++i) {
      MCParticle& mcp = *mcParticles[i];

      //Find all He3tubeSimHits which point from that MCParticle using a typedef and BOOST_FOREACH
      //The typedef is needed as BOOST_FOREACH is a macro and cannot handle anything including a comma
      typedef RelationIndex<MCParticle, He3tubeSimHit>::Element relMCSimHit_Element;
      BOOST_FOREACH(const relMCSimHit_Element & relation, relMCSimHit.getElementsFrom(mcp)) {

        //int processNum = mcp.getSecondaryPhysicsProcess();

        MCParticle* mom = mcp.getMother();

        if (mom == NULL) continue;

        //if(processNum==121){
        if (mom->getPDG() == 2112) {
          He3tubeSimHit* aHit = He3SimHits[relation.indexTo];
          int detNB = aHit->getdetNb();
          definiteNeutron[detNB] = true;
          ProcessHit(aHit, lowTime, edepDet, NbEle_tot);
        }
      }
    }



  } else {  //if mcparticles don't exist, we don't know which simhits are from neutron events
    int nentries = He3SimHits.getEntries();
    for (int i = 0; i < nentries; i++) {
      He3tubeSimHit* aHit = He3SimHits[i];
      int pdg = aHit->gettkPDG();
      if (pdg == 2212 || pdg == 1000010030) ProcessHit(aHit, lowTime, edepDet, NbEle_tot);
    }

  }



  for (int i = 0; i < numOfTubes; i++) {
    peak[i] = (double)NbEle_tot[i] * m_ConversionFactor; //convert the number of ions into a value in ADC counts
    if (peak[i] != 0) {

      //create He3tubeHit
      He3Hits.appendNew(He3tubeHit(edepDet[i], i, peak[i], lowTime[i], definiteNeutron[i]));

      B2DEBUG(80, "He3Digitizer: " << edepDet[i] << "MeV deposited in tube #" << i << " with waveform peak of " << peak[i]);
    }
  }

  Event++;

  //delete arrays
  delete[] peak;
  delete[] lowTime;
  delete[] edepDet;
  delete[] NbEle_tot;
  delete[] definiteNeutron;

}


void He3DigitizerModule::ProcessHit(He3tubeSimHit* aHit, double* lowTime, double* edepDet, double* NbEle_tot)
{

  //get various info from this
  double edep = aHit->getEnergyDep();
  double niel = aHit->getEnergyNiel();
  int detNB = aHit->getdetNb();
  double time = aHit->getGlTime();
  if (time < lowTime[detNB]) lowTime[detNB] =
      time; //get the earliest time for this simhit

  if (detNB >= numOfTubes) {
    B2WARNING("He3Digitizer: Detector number of He3tubeSimHit is greater than number implemented! Ignoring He3tubeSimHit.");
    return;
  }

  //ionization energy
  double ionEn = (edep - niel) * 1e3; //MeV to keV

  if ((ionEn * 1e3) < m_Workfct) return;  //keV to eV
  //if there is enough energy to ionize the gas, get the number if ions produced
  if ((ionEn * 1e3) > m_Workfct) {

    double meanE1 = ionEn * 1e3 / m_Workfct;
    double sigma = sqrt(m_Fanofac * meanE1);
    const int NbEle = (int)gRandom->Gaus(meanE1,
                                         sigma); //this is the number of electrons created, and the number which reach the wire, since gain is essentially 1.

    NbEle_tot[detNB] = NbEle_tot[detNB] + NbEle;

  }

  edepDet[detNB] = edepDet[detNB] + edep;

}


void He3DigitizerModule::getXMLData()
{
  GearDir content = GearDir("/Detector/DetectorComponent[@name=\"HE3TUBE\"]/Content/");

  //get the location of the tubes
  BOOST_FOREACH(const GearDir & activeParams, content.getNodes("Active")) {
    B2DEBUG(250, "Tubes located at x=" << activeParams.getLength("x_he3tube"));
    numOfTubes++;
  }


  B2INFO("He3Digitizer: There are " << numOfTubes << " tubes implemented");

}

void He3DigitizerModule::endRun()
{
}

void He3DigitizerModule::terminate()
{
}
