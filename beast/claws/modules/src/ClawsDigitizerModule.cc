/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Igal Jaegle                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <beast/claws/modules/ClawsDigitizerModule.h>
#include <beast/claws/dataobjects/CLAWSSimHit.h>

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
using namespace claws;


//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(ClawsDigitizer)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

ClawsDigitizerModule::ClawsDigitizerModule() : Module()
{
  // Set module properties
  setDescription("Claws digitizer module");

  //Default values are set here. New values can be in CLAWS.xml.
  addParam("ScintCell", m_ScintCell, "Number of scintillator cell", 16);
  addParam("TimeStep", m_TimeStep, "Time step", 0.8);
  addParam("C_keV_to_MIP", m_C_keV_to_MIP, "C_keV_to_MIP", 805.5);
  addParam("C_MIP_to_PE", m_C_MIP_to_PE, "C_MIP_to_PE");
  addParam("MinTime", m_MinTime, "Min. time", 0.0);
  addParam("MaxTime", m_MaxTime, "Max. time", 750.0);
  addParam("PEthres", m_PEthres, "Energy threshold in keV", 1.0);
}

ClawsDigitizerModule::~ClawsDigitizerModule()
{
}

void ClawsDigitizerModule::initialize()
{
  B2INFO("ClawsDigitizer: Initializing");
  m_clawsHit.registerInDataStore();

  //get the garfield drift data, gas, and CLAWS paramters
  getXMLData();

}

void ClawsDigitizerModule::beginRun()
{
}

void ClawsDigitizerModule::event()
{

  StoreArray<MCParticle> particles;
  StoreArray<CLAWSSimHit> CLAWSSimHits;
  StoreArray<ClawsHit> ClawsHits;

  //Skip events with no CLAWSSimHits, but continue the event counter
  if (CLAWSSimHits.getEntries() == 0) {
    return;
  }

  StoreArray<CLAWSSimHit> SimHits;
  StoreArray<ClawsHit> Hits;
  /*
  int number_of_timebins = (int)((m_MaxTime - m_MinTime) / m_TimeStep);

  for (int i = 0; i < 1000; i ++)
    for (int j = 0; j < 100; j ++)
      hitsarrayinPE[i][j] = 0;

  for (const auto& SimHit : SimHits) {
    const int detNb = SimHit.getCellId();
    const double Edep = SimHit.getEnergyDep() * 1e6; //GeV -> keV
    const double tof = SimHit.getFlightTime(); //ns
    int TimeBin = tof / m_TimeStep;
    double MIP = Edep / m_C_keV_to_MIP;
    double PE = MIP * m_C_MIP_to_PE;
    if (m_MinTime < tof && tof < m_MaxTime && TimeBin < 1000 && detNb < 100)
      hitsarrayinPE[TimeBin][detNb] += PE;
  }
  */
  for (const auto& SimHit : SimHits) {
    int lad = SimHit.getLadder();
    int sen = SimHit.getSensor();
    //const int detNb = SimHit.getCellId();
    //int pdg = SimHit.getPDGCode();
    int detNb = (lad - 1) * 8 + sen - 1;
    const double Edep = SimHit.getEnergyVisible() * 1e6; //GeV -> keV
    const double tof = SimHit.getTime(); //ns
    int TimeBin = tof / m_TimeStep;
    double MIP = Edep / m_C_keV_to_MIP;
    double PE = MIP * m_C_MIP_to_PE[detNb];
    if ((m_MinTime < tof && tof < m_MaxTime) &&  PE > m_PEthres)
      Hits.appendNew(ClawsHit(detNb,  TimeBin, Edep, MIP, PE));
  }
  /*
  for (int i = 0; i < number_of_timebins; i ++) {
    for (int j = 0; j < m_ScintCell; j ++) {
      if (hitsarrayinPE[i][j] > m_PEthres) {
        double PE = hitsarrayinPE[i][j];
        double MIP = PE / m_C_MIP_to_PE;
        double Edep = MIP * m_C_keV_to_MIP * 1e-6; //keV -> GeV.
        Hits.appendNew(ClawsHit(j, i, Edep, MIP, PE));
      }
    }
  }
  */
}

//read tube centers, impulse response, and garfield drift data filename from CLAWS.xml
void ClawsDigitizerModule::getXMLData()
{
  GearDir content = GearDir("/Detector/DetectorComponent[@name=\"CLAWS\"]/Content/");

  m_ScintCell = content.getInt("ScintCell");
  m_TimeStep = content.getDouble("TimeStep");
  m_MinTime = content.getDouble("MinTime");
  m_MaxTime = content.getDouble("MaxTime");
  m_PEthres = content.getDouble("PEthres");
  m_C_keV_to_MIP = content.getDouble("C_keV_to_MIP");
  //m_C_MIP_to_PE = content.getDouble("C_MIP_to_PE");

  B2INFO("ClawsDigitizer: Aquired claws locations and gas parameters");

}

void ClawsDigitizerModule::endRun()
{
}

void ClawsDigitizerModule::terminate()
{
}


