/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Igal Jaegle                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <beast/claw/modules/ClawDigitizerModule.h>
#include <beast/claw/dataobjects/ClawSimHit.h>
#include <beast/claw/dataobjects/ClawHit.h>

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
using namespace claw;


//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(ClawDigitizer)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

ClawDigitizerModule::ClawDigitizerModule() : Module()
{
  // Set module properties
  setDescription("Claw digitizer module");

  //Default values are set here. New values can be in CLAW.xml.
  addParam("ScintCell", m_ScintCell, "Number of scintillator cell", 16);
  addParam("TimeStep", m_TimeStep, "Time step", 0.8);
  addParam("MinTime", m_MinTime, "Min. time", 0.0);
  addParam("MaxTime", m_MaxTime, "Max. time", 750.0);
  addParam("Ethres", m_Ethres, "Energy threshold in MeV", 0.0);

}

ClawDigitizerModule::~ClawDigitizerModule()
{
}

void ClawDigitizerModule::initialize()
{
  B2INFO("ClawDigitizer: Initializing");
  StoreArray<ClawHit>::registerPersistent();

  //get the garfield drift data, gas, and CLAW paramters
  getXMLData();

}

void ClawDigitizerModule::beginRun()
{
}

void ClawDigitizerModule::event()
{

  StoreArray<MCParticle> particles;
  StoreArray<ClawSimHit> ClawSimHits;
  StoreArray<ClawHit> ClawHits;

  //Skip events with no ClawSimHits, but continue the event counter
  if (ClawSimHits.getEntries() == 0) {
    return;
  }
  /*
  //auto edepArray = new vector<double>[m_ScintCell](); //energy deposits per scintillator cell
  auto adepArray = new vector<double>[m_ScintCell](); //energy deposits corrected per scintillator cell
  auto timeArray = new vector<double>[m_ScintCell](); //time cell bin nmber
  //auto pdgArray = new vector<double>[numOfCells](); //particle pdg that deposits energy

  int nentries = ClawSimHits.getEntries();
  //loop on all entries to store for each CLAW scintillator cell
  for (int i = 0; i < nentries; i++) {
    ClawSimHit* aHit = ClawSimHits[i];
    int detNb = aHit->getdetNb();
    //int pdgid = aHit->gettkPDG();
    //float edep = aHit->getEnergyDep();
    float adep = aHit->getEnergyNiel();
    float G4time = aHit->getGlTime();

    //edepArray[detNb].push_back(edep);
    adepArray[detNb].push_back(adep);
    timeArray[detNb].push_back(G4time);
    //pdgArray[detNb].push_back(pdgid);

  }
  //determine the number of time bin
  int TimeNbins = (int)((m_MaxTime - m_MinTime) / m_TimeStep);

  //loop over CLAW scintillator cell
  for (int i = 0; i < m_ScintCell; i ++) {

    //define and initialize the energy sum per time cell for each scintillator
    double esum[TimeNbins];
    for (int j = 0; j < TimeNbins; j ++) esum[j] = 0;

    //loop over array entries
    for (int j = 0; j < (int) adepArray[i].size(); j ++) {

      //calculate time bin value
      int i_bin = timeArray[i][j] / m_TimeStep;

      //check if hit within time range
      if (m_MinTime <= timeArray[i][j] && timeArray[i][j] <= m_MaxTime)
        esum[i_bin] += adepArray[i][j];
    }

    //loop over number of time bin
    for (int j = 0; j < TimeNbins; j ++) {

      //if esum > 0 then store in ClawHit detNb, time bin, energy sum, and particle counter
      if (esum[j] > m_Ethres) {
        int ncount = esum[j] / m_ConvFac;
        ClawHits.appendNew(i, j , esum[j], ncount);
      }
    }
  }

  //delete array
  delete [] adepArray;
  delete [] timeArray;
  */
}

//read tube centers, impulse response, and garfield drift data filename from CLAW.xml
void ClawDigitizerModule::getXMLData()
{
  GearDir content = GearDir("/Detector/DetectorComponent[@name=\"CLAW\"]/Content/");

  m_ScintCell = content.getInt("ScintCell");
  m_TimeStep = content.getDouble("TimeStep");
  m_MinTime = content.getDouble("MinTime");
  m_MaxTime = content.getDouble("MaxTime");
  m_Ethres = content.getDouble("Ethres");
  m_ConvFac = content.getDouble("ConvFac");

  B2INFO("ClawDigitizer: Aquired claw locations and gas parameters");
  B2INFO("              from CLAW.xml. There are " << m_ScintCell << " CLAWs implemented");

}

void ClawDigitizerModule::endRun()
{
}

void ClawDigitizerModule::terminate()
{
}


