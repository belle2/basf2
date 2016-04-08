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
#include <beast/he3tube/dataobjects/He3tubeHit.h>

#include <mdst/dataobjects/MCParticle.h>
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
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

}

He3DigitizerModule::~He3DigitizerModule()
{
}

void He3DigitizerModule::initialize()
{
  B2INFO("He3Digitizer: Initializing");
  StoreArray<He3tubeHit>::registerPersistent();

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


  //skip events with no He3SimHits, but continue the event counter
  if (He3SimHits.getEntries() == 0) {
    B2DEBUG(250, "Skipping event #" << Event << " since there are no sim hits");
    Event++;
    return;
  }

  auto peak = new double[numOfTubes]();
  auto lowTime = new double[numOfTubes]();  //earliest deposit in each tube
  std::fill_n(lowTime, numOfTubes, 9999999999999);
  auto edepDet = new double[numOfTubes]();  //total energy deposited per tube
  auto NbEle_tot = new double[numOfTubes]();

  int nentries = He3SimHits.getEntries();
  for (int i = 0; i < nentries; i++) {
    He3tubeSimHit* aHit = He3SimHits[i];

    //get various info from this simhit
    double edep = aHit->getEnergyDep();
    double niel = aHit->getEnergyNiel();
    int detNB = aHit->getdetNb();
    double time = aHit->getGlTime();
    if (time < lowTime[detNB]) lowTime[detNB] = time; //get the earliest time for this simhit

    if (detNB >= numOfTubes) {
      B2WARNING("He3Digitizer: Detector number of He3tubeSimHit is greater than number implemented! Ignoring He3tubeSimHit.");
      continue;
    }

    //ionization energy
    double ionEn = (edep - niel) * 1e3; //MeV to keV

    if ((ionEn * 1e3) < m_Workfct) continue;  //keV to eV
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

  for (int i = 0; i < numOfTubes; i++) {
    peak[i] = (double)NbEle_tot[i] * m_ConversionFactor; //convert the number of ions into a value in ADC counts
    if (peak[i] != 0) {

      //create He3tubeHit
      He3Hits.appendNew(He3tubeHit(edepDet[i], i, peak[i], lowTime[i]));

      B2DEBUG(80, "He3Digitizer: " << edepDet[i] << "MeV deposited in tube #" << i << " with waveform peak of " << peak[i]);
    }
  }

  Event++;



  //delete arrays
  delete[] peak;
  delete[] lowTime;
  delete[] edepDet;


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
