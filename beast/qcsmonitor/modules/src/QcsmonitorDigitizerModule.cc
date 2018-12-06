/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Igal Jaegle                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <beast/qcsmonitor/modules/QcsmonitorDigitizerModule.h>
#include <beast/qcsmonitor/dataobjects/QcsmonitorSimHit.h>

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
using namespace qcsmonitor;


//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(QcsmonitorDigitizer)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

QcsmonitorDigitizerModule::QcsmonitorDigitizerModule() : Module()
{
  // Set module properties
  setDescription("Qcsmonitor digitizer module");

  //Default values are set here. New values can be in QCSMONITOR.xml.
  addParam("ScintCell", m_ScintCell, "Number of scintillator cell", 2);
  addParam("TimeStep", m_TimeStep, "Time step", 0.8);
  addParam("C_keV_to_MIP", m_C_keV_to_MIP, "C_keV_to_MIP", 241.65);
  addParam("C_MIP_to_PE", m_C_MIP_to_PE, "C_MIP_to_PE", 15.0);
  addParam("MinTime", m_MinTime, "Min. time", 0.0);
  addParam("MaxTime", m_MaxTime, "Max. time", 750.0);
  addParam("MIPthres", m_MIPthres, "Energy threshold in keV", 0.5);
}

QcsmonitorDigitizerModule::~QcsmonitorDigitizerModule()
{
}

void QcsmonitorDigitizerModule::initialize()
{
  B2INFO("QcsmonitorDigitizer: Initializing");
  m_qcsmonitorHit.registerInDataStore();

  //get the garfield drift data, gas, and QCSMONITOR paramters
  getXMLData();

}

void QcsmonitorDigitizerModule::beginRun()
{
}

void QcsmonitorDigitizerModule::event()
{

  StoreArray<MCParticle> particles;
  StoreArray<QcsmonitorSimHit> QcsmonitorSimHits;
  StoreArray<QcsmonitorHit> QcsmonitorHits;

  //Skip events with no QcsmonitorSimHits, but continue the event counter
  if (QcsmonitorSimHits.getEntries() == 0) {
    return;
  }

  StoreArray<QcsmonitorSimHit> SimHits;
  StoreArray<QcsmonitorHit> Hits;

  int number_of_timebins = (int)((m_MaxTime - m_MinTime) / m_TimeStep);

  for (int i = 0; i < 1000; i ++)
    for (int j = 0; j < 100; j ++)
      hitsarrayinMIP[i][j] = 0;

  for (const auto& SimHit : SimHits) {
    const int detNb = SimHit.getCellId();
    const double Edep = SimHit.getEnergyDep() * 1e6; //GeV -> keV
    const double tof = SimHit.getFlightTime(); //ns
    int TimeBin = tof / m_TimeStep;
    double MIP = Edep / m_C_keV_to_MIP;
    //double PE = MIP * m_C_MIP_to_PE;
    if (m_MinTime < tof && tof < m_MaxTime && TimeBin < 1000 && detNb < 100)
      hitsarrayinMIP[TimeBin][detNb] += MIP;
  }

  /*
    for (const auto& SimHit : SimHits) {
    const int detNb = SimHit.getCellId();
    //int pdg = SimHit.getPDGCode();
    const double Edep = SimHit.getEnergyDep() * 1e6; //GeV -> keV
    const double tof = SimHit.getFlightTime(); //ns
    int TimeBin = tof / m_TimeStep;
    double MIP = Edep / m_C_keV_to_MIP;
    double PE = MIP * m_C_MIP_to_PE;
    if ((m_MinTime < tof && tof < m_MaxTime) &&  MIP > m_MIPthres)
    //if (hitsarrayinMIP[TimeBin][detNb] > m_MIPthres)
    Hits.appendNew(QcsmonitorHit(detNb,  TimeBin, Edep, MIP, PE));
    }
  */

  for (int i = 0; i < number_of_timebins; i ++) {
    for (int j = 0; j < m_ScintCell; j ++) {
      if (hitsarrayinMIP[i][j] > m_MIPthres) {
        double MIP = hitsarrayinMIP[i][j];
        double Edep = MIP * m_C_keV_to_MIP * 1e-6; //keV -> GeV.
        double PE = MIP * m_C_MIP_to_PE;
        Hits.appendNew(QcsmonitorHit(j, i, Edep, MIP, PE));
      }
    }
  }

}

//read tube centers, impulse response, and garfield drift data filename from QCSMONITOR.xml
void QcsmonitorDigitizerModule::getXMLData()
{
  GearDir content = GearDir("/Detector/DetectorComponent[@name=\"QCSMONITOR\"]/Content/");

  m_ScintCell = content.getInt("ScintCell");
  m_TimeStep = content.getDouble("TimeStep");
  m_MinTime = content.getDouble("MinTime");
  m_MaxTime = content.getDouble("MaxTime");
  m_MIPthres = content.getDouble("MIPthres");
  m_C_keV_to_MIP = content.getDouble("C_keV_to_MIP");
  m_C_MIP_to_PE = content.getDouble("C_MIP_to_PE");

  B2INFO("QcsmonitorDigitizer: Aquired qcsmonitor locations and gas parameters");

}

void QcsmonitorDigitizerModule::endRun()
{
}

void QcsmonitorDigitizerModule::terminate()
{
}


