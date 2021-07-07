/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <beast/pindiode/modules/PinDigitizerModule.h>
#include <beast/pindiode/dataobjects/PindiodeSimHit.h>

#include <framework/logging/Logger.h>
#include <framework/core/RandomNumbers.h>

//c++
#include <cmath>
#include <string>
#include <fstream>

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
  addParam("FanoFactor", m_FanoFactor, "e resolution ", 0.1);

}

PinDigitizerModule::~PinDigitizerModule()
{
}

void PinDigitizerModule::initialize()
{
  B2INFO("PinDigitizer: Initializing");
  m_pindiodeHit.registerInDataStore();

}

void PinDigitizerModule::beginRun()
{
}

void PinDigitizerModule::event()
{


  StoreArray<PindiodeSimHit> PinSimHits;
  StoreArray<PindiodeHit> PinHits;
  //Skip events with no PinSimHits, but continue the event counter
  if (PinSimHits.getEntries() == 0)
    return;

  int nentries = PinSimHits.getEntries();
  for (int i = 0; i < nentries; i++) {
    PindiodeSimHit* aHit = PinSimHits[i];
    int detNb = aHit->getCellId();
    double edep = aHit->getEnergyDep();
    double time = aHit->getFlightTime();
    int pdg = aHit->getPDGCode();
    double meanEl = edep / m_WorkFunction * 1e9; //GeV to eV
    double sigma = sqrt(m_FanoFactor * meanEl); //sigma in electron
    int NbEle = (int)gRandom->Gaus(meanEl, sigma); //electron number
    double fedep = ((double) NbEle) * m_WorkFunction * 1e-3; //eV to keV
    double volt = NbEle * 1.602176565e-19 * m_CrematGain * 1e12; // volt
    //if ((fedep * 1e3) > m_WorkFunction)
    PinHits.appendNew(PindiodeHit(fedep, volt, time, detNb, pdg));
  }

}

void PinDigitizerModule::endRun()
{
}

void PinDigitizerModule::terminate()
{
}


