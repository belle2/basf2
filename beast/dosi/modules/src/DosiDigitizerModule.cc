/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <beast/dosi/modules/DosiDigitizerModule.h>
#include <beast/dosi/dataobjects/DosiSimHit.h>

#include <framework/logging/Logger.h>
#include <framework/gearbox/GearDir.h>
#include <framework/core/RandomNumbers.h>

//c++
#include <string>
#include <fstream>
#include <vector>

using namespace std;
using namespace Belle2;
using namespace dosi;


//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(DosiDigitizer)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

DosiDigitizerModule::DosiDigitizerModule() : Module()
{
  // Set module properties
  setDescription("Dosi digitizer module");

}

DosiDigitizerModule::~DosiDigitizerModule()
{
}

void DosiDigitizerModule::initialize()
{
  B2INFO("DosiDigitizer: Initializing");
  m_dosiHit.registerInDataStore();

  //get xml data
  getXMLData();

}

void DosiDigitizerModule::beginRun()
{
}

void DosiDigitizerModule::event()
{
  StoreArray<DosiSimHit> DosiSimHits;
  StoreArray<DosiHit> DosiHits;
  int nentries = DosiSimHits.getEntries();
  for (int i = 0; i < nentries; i++) {
    DosiSimHit* aHit = DosiSimHits[i];
    int m_cellID = aHit->getCellId();
    int m_trackID = aHit->getTrackId();
    int pdgCode = aHit->getPDGCode();
    double m_Time = aHit->getFlightTime();
    TVector3 m_Mom = aHit->getMomentum();
    TVector3 m_Pos = aHit->getPosition();
    double m_energyDeposit = aHit->getEnergyDep();
    double erecdep = m_energyDeposit;
    erecdep += gRandom->Gaus(0, GetEnergyResolutionGeV(m_energyDeposit, m_cellID));
    DosiHits.appendNew(DosiHit(m_cellID, m_trackID, pdgCode, m_Time * m_energyDeposit / erecdep, m_energyDeposit, m_Mom,
                               m_Pos * (m_energyDeposit / erecdep), erecdep));
  }

}
//read from DOSI.xml
void DosiDigitizerModule::getXMLData()
{
  //GearDir content = GearDir("/Detector/DetectorComponent[@name=\"DOSI\"]/Content/");

  B2INFO("DosiDigitizer: Aquired dosi locations and gas parameters");
  B2INFO("              from DOSI.xml. There are " << nDOSI << " DOSIs implemented");

}


Double_t DosiDigitizerModule::GetEnergyResolutionGeV(Double_t pEnergy, int CellId)
{
  // Returns energy resolution in GeV when supplied Energy in GeV
  return (m_EnergyResolutionFactor[CellId] * TMath::Sqrt(pEnergy) + m_EnergyResolutionConst[CellId] * pEnergy);

}


void DosiDigitizerModule::endRun()
{
}

void DosiDigitizerModule::terminate()
{
}


