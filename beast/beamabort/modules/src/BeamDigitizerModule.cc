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
#include <beast/beamabort/dataobjects/BeamabortHit.h>

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
  addParam("PreampGain", m_PreampGain, "Charge sensitive preamplifier gain [volts/C] ", 1.4);

}

BeamDigitizerModule::~BeamDigitizerModule()
{
}

void BeamDigitizerModule::initialize()
{
  B2INFO("BeamDigitizer: Initializing");
  StoreArray<BeamabortHit>::registerPersistent();

  //get the garfield drift data, gas, and BEAM paramters
  getXMLData();

}

void BeamDigitizerModule::beginRun()
{
}

void BeamDigitizerModule::event()
{
  /*
  StoreArray<MCParticle> particles;
  StoreArray<BeamabortSimHit> BeamSimHits;
  StoreArray<BeamabortHit> BeamHits;
  //Skip events with no BeamSimHits, but continue the event counter
  if (BeamSimHits.getEntries() == 0) {
    Event++;
    return;
  }

  //Declare and initialze energy and time
  double edep[nBEAM];
  double time[nBEAM];
  double itime[nBEAM];
  double volt[nBEAM];
  int pdg[nBEAM];
  for (int i = 0; i < nBEAM; i++) {
    edep[i] = 0;
    time[i] = 0;
    itime[i] = 0;
    volt[i] = 0;
    pdg[i] = 0;
  }

  int nentries = BeamSimHits.getEntries();
  for (int i = 0; i < nentries; i++) {
    BeamabortSimHit* aHit = BeamSimHits[i];
    int detNb = aHit->getCellId();
    edep[detNb] += aHit->getEnergyDep();
    time[detNb] += aHit->getFlightTime();
    itime[detNb] ++;
    int PDG = aHit->getPDGCode();
    if (PDG == 22) pdg[detNb] = 1;
  }

  for (int i = 0; i < nBEAM; i++) {
    if (itime[i] > 0) {
      time[i] /= itime[i];
      volt[i] = edep[i] * 1e6 * 1.602176565e-19 * m_PreampGain * 1e12;
      BeamHits.appendNew(BeamabortHit(i, edep[i], volt[i], time[i], pdg[i]));
    }
  }
  */
  Event++;

}
//read from BEAMABORT.xml
void BeamDigitizerModule::getXMLData()
{
  GearDir content = GearDir("/Detector/DetectorComponent[@name=\"BEAMABORT\"]/Content/");

  //get the location of the tubes
  BOOST_FOREACH(const GearDir & activeParams, content.getNodes("Active")) {
    BEAMCenter.push_back(TVector3(activeParams.getLength("z_pindiode"), activeParams.getLength("r_pindiode"),
                                  activeParams.getLength("Phi")));
    nBEAM++;
  }
  m_PreampGain = content.getDouble("PreampGain");

  B2INFO("BeamDigitizer: Aquired pin locations and gas parameters");
  B2INFO("              from BEAMABORT.xml. There are " << nBEAM << " BEAMs implemented");

}

void BeamDigitizerModule::endRun()
{
}

void BeamDigitizerModule::terminate()
{
}


