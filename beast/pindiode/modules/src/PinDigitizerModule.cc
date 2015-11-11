/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Igal Jaegle                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <beast/pindiode/modules/PinDigitizerModule.h>
#include <beast/pindiode/dataobjects/PindiodeSimHit.h>
#include <beast/pindiode/dataobjects/PindiodeHit.h>

#include <mdst/dataobjects/MCParticle.h>
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
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
using namespace pindiode;


//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(PinDigitizer)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

PinDigitizerModule::PinDigitizerModule() : Module()
{
  // Set module properties
  setDescription("Pindiode digitizer module");

  //Default values are set here. New values can be in PINDIODE.xml.
  addParam("CrematGain", m_CrematGain, "Charge sensitive preamplifier gain [volts/C] ", 1.4);
  addParam("WorkFunction", m_WorkFunction, "Convert eV to e [e/eV] ", 1.12);

}

PinDigitizerModule::~PinDigitizerModule()
{
}

void PinDigitizerModule::initialize()
{
  B2INFO("PinDigitizer: Initializing");
  StoreArray<PindiodeHit>::registerPersistent();

  //get xml data
  getXMLData();

}

void PinDigitizerModule::beginRun()
{
}

void PinDigitizerModule::event()
{

  StoreArray<MCParticle> particles;
  StoreArray<PindiodeSimHit> PinSimHits;
  StoreArray<PindiodeHit> PinHits;
  //Skip events with no PinSimHits, but continue the event counter
  if (PinSimHits.getEntries() == 0) {
    //Event++;
    return;
  }

  //Declare and initialze energy and time
  double edep[2 * nPIN];
  double time[2 * nPIN];
  double itime[2 * nPIN];
  double volt[2 * nPIN];
  int pdg[nPIN];
  for (int i = 0; i < 2 * nPIN; i++) {
    edep[i] = 0;
    time[i] = 0;
    itime[i] = 0;
    volt[i] = 0;
    pdg[i] = 0;
  }

  int nentries = PinSimHits.getEntries();
  for (int i = 0; i < nentries; i++) {
    PindiodeSimHit* aHit = PinSimHits[i];
    int detNb = aHit->getCellId();
    if (detNb > 2 * nPIN)continue;
    edep[detNb] += aHit->getEnergyDep();
    time[detNb] += aHit->getFlightTime();
    itime[detNb] ++;
    int PDG = aHit->getPDGCode();
    if (PDG == 22) pdg[detNb] = 1;
  }

  for (int i = 0; i < 2 * nPIN; i++) {
    if (itime[i] > 0) {
      time[i] /= itime[i];
      volt[i] = edep[i] / m_WorkFunction * 1e6 * 1.602176565e-19 * m_CrematGain * 1e12;
      PinHits.appendNew(PindiodeHit(i, edep[i], volt[i], time[i], pdg[i]));
    }
  }

  //Event++;

}
//read tube centers, impulse response, and garfield drift data filename from PINDIODE.xml
void PinDigitizerModule::getXMLData()
{
  GearDir content = GearDir("/Detector/DetectorComponent[@name=\"PINDIODE\"]/Content/");

  //get the location of the tubes
  BOOST_FOREACH(const GearDir & activeParams, content.getNodes("Active")) {
    PINCenter.push_back(TVector3(activeParams.getLength("z_pindiode"), activeParams.getLength("r_pindiode"),
                                 activeParams.getLength("Phi")));
    nPIN++;
  }
  m_CrematGain = content.getDouble("CrematGain");

  B2INFO("PinDigitizer: Aquired pin locations and gas parameters");
  B2INFO("              from PINDIODE.xml. There are " << nPIN << " PINs implemented");

}

void PinDigitizerModule::endRun()
{
}

void PinDigitizerModule::terminate()
{
}


