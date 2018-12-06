/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Igal Jaegle                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <beast/beamabort/modules/BeamDigitizerModule.h>
#include <beast/beamabort/dataobjects/BeamabortSimHit.h>

#include <mdst/dataobjects/MCParticle.h>
#include <framework/datastore/RelationArray.h>
#include <framework/datastore/RelationIndex.h>
#include <framework/logging/Logger.h>
#include <framework/gearbox/Gearbox.h>
#include <framework/gearbox/GearDir.h>
#include <framework/gearbox/Unit.h>
#include <framework/core/RandomNumbers.h>

//c++
#include <cmath>
#include <boost/foreach.hpp>
#include <string>
#include <sstream>
#include <iostream>
#include <fstream>
#include <vector>

// ROOT
#include <TH1.h>
#include <TH2.h>
#include <TH3.h>
#include <TFile.h>


using namespace std;
using namespace Belle2;
using namespace beamabort;


//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(BeamDigitizer)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

BeamDigitizerModule::BeamDigitizerModule() : Module()
{
  // Set module properties
  setDescription("Beamabort digitizer module");

  //Default values are set here. New values can be in BEAMABORT.xml.
  addParam("WorkFunction", m_WorkFunction, "Convert eV to e [e/eV] ", 1.12);
  addParam("FanoFactor", m_FanoFactor, "e resolution ", 0.1);

}

BeamDigitizerModule::~BeamDigitizerModule()
{
}

void BeamDigitizerModule::initialize()
{
  B2INFO("BeamDigitizer: Initializing");
  m_beamabortHit.registerInDataStore();

}

void BeamDigitizerModule::beginRun()
{
}

void BeamDigitizerModule::event()
{

  StoreArray<BeamabortSimHit> BeamSimHits;
  StoreArray<BeamabortHit> BeamHits;
  //Skip events with no BeamSimHits, but continue the event counter
  if (BeamSimHits.getEntries() == 0)
    return;


  int nentries = BeamSimHits.getEntries();
  for (int i = 0; i < nentries; i++) {
    BeamabortSimHit* aHit = BeamSimHits[i];
    int detNb = aHit->getCellId();
    double edep = aHit->getEnergyDep();
    double time = aHit->getFlightTime();
    int pdg = aHit->getPDGCode();
    double meanEl = edep / m_WorkFunction * 1e9; //GeV to eV
    double sigma = sqrt(m_FanoFactor * meanEl); //sigma in electron
    int NbEle = (int)gRandom->Gaus(meanEl, sigma); //electron number
    double fedep = ((double) NbEle) * m_WorkFunction * 1e-3; //eV to keV
    double Amp = NbEle / (6.25 * 1e18); // A x s
    //if ((fedep * 1e3) > m_WorkFunction)
    BeamHits.appendNew(BeamabortHit(fedep, Amp, time, detNb, pdg));
  }

}

void BeamDigitizerModule::endRun()
{
}

void BeamDigitizerModule::terminate()
{
}


