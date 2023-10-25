/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <beast/bgo/modules/BgoDigitizerModule.h>
#include <beast/bgo/dataobjects/BgoSimHit.h>

#include <framework/logging/Logger.h>
#include <framework/gearbox/GearDir.h>
#include <framework/core/RandomNumbers.h>

// ROOT
#include <Math/Vector3D.h>
#include <TMath.h>

//c++
#include <string>
#include <fstream>
#include <vector>

using namespace std;
using namespace Belle2;
using namespace bgo;


//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(BgoDigitizer);

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

BgoDigitizerModule::BgoDigitizerModule() : Module()
{
  // Set module properties
  setDescription("Bgo digitizer module");

  //Default values are set here. New values can be in BGO.xml.
  //addParam("EnergyResolutionFactor", m_EnergyResolutionFactor, "Energy resolution factor ");
  //addParam("EnergyResolutionConst", m_EnergyResolutionConst, "Energy resolution constant ");
  //addParam("Threshold", m_Threshold, "Energy threshold");
  //addParam("Range", m_Range, "Energy range")
}

BgoDigitizerModule::~BgoDigitizerModule()
{
}

void BgoDigitizerModule::initialize()
{
  B2INFO("BgoDigitizer: Initializing");
  m_bgoHit.registerInDataStore();

  //get xml data
  getXMLData();

}

void BgoDigitizerModule::beginRun()
{
}

void BgoDigitizerModule::event()
{
  StoreArray<BgoSimHit> BgoSimHits;
  StoreArray<BgoHit> BgoHits;
  int nentries = BgoSimHits.getEntries();
  for (int i = 0; i < nentries; i++) {
    BgoSimHit* aHit = BgoSimHits[i];
    int m_cellID = aHit->getCellId();
    int m_trackID = aHit->getTrackId();
    int pdgCode = aHit->getPDGCode();
    double m_Time = aHit->getFlightTime();
    ROOT::Math::XYZVector m_Mom = aHit->getMomentum();
    ROOT::Math::XYZVector m_Pos = aHit->getPosition();
    double m_energyDeposit = aHit->getEnergyDep();
    double erecdep = m_energyDeposit;
    erecdep += gRandom->Gaus(0, GetEnergyResolutionGeV(m_energyDeposit, m_cellID));
    //if (m_Threshold[m_cellID] <= erecdep && erecdep <= m_Range[m_cellID]) {
    BgoHits.appendNew(BgoHit(m_cellID, m_trackID, pdgCode, m_Time * m_energyDeposit / erecdep, m_energyDeposit, m_Mom,
                             m_Pos * (m_energyDeposit / erecdep), erecdep));
    //}
  }

}
//read from BGO.xml
void BgoDigitizerModule::getXMLData()
{
  GearDir content = GearDir("/Detector/DetectorComponent[@name=\"BGO\"]/Content/");

  //m_EnergyResolutionConst = content.getDouble("EnergyResolutionConst");
  //m_EnergyResolutionFactor = content.getDouble("EnergyResolutionFactor");
  int iEnResConst = 0;
  for (double  EnResConst : content.getArray("EnergyResolutionConst", {0})) {
    m_EnergyResolutionConst[iEnResConst] = EnResConst;
    iEnResConst++;
  }
  int iEnResFac = 0;
  for (double  EnResFac : content.getArray("EnergyResolutionFactor", {0})) {
    m_EnergyResolutionFactor[iEnResFac] = EnResFac / 100.;
    iEnResFac++;
  }
  int iThres = 0;
  for (double Threshold : content.getArray("Threshold", {0})) {
    //Threshold *= CLHEP::GeV;
    m_Threshold[iThres] = Threshold;
    iThres++;
  }
  int iRange = 0;
  for (double Range : content.getArray("Range", {0})) {
    //Range *= CLHEP::GeV;
    m_Range[iRange] = Range;
    iRange++;
  }

  B2INFO("BgoDigitizer: Aquired bgo locations and gas parameters");
  B2INFO("              from BGO.xml. There are " << nBGO << " BGOs implemented");

}


Double_t BgoDigitizerModule::GetEnergyResolutionGeV(Double_t pEnergy, int CellId)
{
  // Returns energy resolution in GeV when supplied Energy in GeV
  return (m_EnergyResolutionFactor[CellId] * TMath::Sqrt(pEnergy) + m_EnergyResolutionConst[CellId] * pEnergy);

}


void BgoDigitizerModule::endRun()
{
}

void BgoDigitizerModule::terminate()
{
}


