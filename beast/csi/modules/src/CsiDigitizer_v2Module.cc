/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Igal Jaegle                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <beast/csi/modules/CsiDigitizer_v2Module.h>
#include <beast/csi/dataobjects/CsiSimHit.h>

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
using namespace csi;


//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(CsiDigitizer_v2)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

CsiDigitizer_v2Module::CsiDigitizer_v2Module() : Module()
{
  // Set module properties
  setDescription("Csi digitizer_v2 module");

  //Default values are set here. New values can be in CSI.xml.
  //addParam("EnergyResolutionPureCsIFactor", m_EnergyResolutionPureCsIFactor, "Energy resolution factor ", 1.4);
  //addParam("EnergyResolutionPureCsIConst", m_EnergyResolutionPureCsIConst, "Energy resolution constant ", 1.4);
}

CsiDigitizer_v2Module::~CsiDigitizer_v2Module()
{
}

void CsiDigitizer_v2Module::initialize()
{
  B2INFO("CsiDigitizer_v2: Initializing");
  m_csiHit_v2.registerInDataStore();

  //get xml data
  getXMLData();

}

void CsiDigitizer_v2Module::beginRun()
{
}

void CsiDigitizer_v2Module::event()
{
  StoreArray<CsiSimHit> CsiSimHits;
  StoreArray<CsiHit_v2> CsiHits_v2;
  int nentries = CsiSimHits.getEntries();
  for (int i = 0; i < nentries; i++) {
    CsiSimHit* aHit = CsiSimHits[i];
    int m_cellID = aHit->getCellId();
    int m_box = (int)m_cellID / 6;
    int m_cry = m_cellID - m_box * 3;
    int m_trackID = aHit->getTrackId();
    int pdgCode = aHit->getPDGCode();
    double m_Time = aHit->getFlightTime();
    TVector3 m_Mom = aHit->getMomentum();
    TVector3 m_Pos = aHit->getPosition();
    double m_energyDeposit = aHit->getEnergyDep();
    double erecdep = m_energyDeposit;
    erecdep += gRandom->Gaus(0, GetEnergyResolutionGeV(m_energyDeposit, m_cry));
    //if (m_Threshold[m_cry] <= erecdep && erecdep <= m_Range[m_cry]) {
    CsiHits_v2.appendNew(CsiHit_v2(m_cellID, m_trackID, pdgCode, m_Time * m_energyDeposit / erecdep, m_energyDeposit, m_Mom,
                                   m_Pos * (m_energyDeposit / erecdep), erecdep));
    //}
  }

}
//read from CSI.xml
void CsiDigitizer_v2Module::getXMLData()
{
  GearDir content = GearDir("/Detector/DetectorComponent[@name=\"CSI\"]/Content/");

  int iEnergyResolutionConst = 0;
  for (double EnergyResolutionConst : content.getArray("EnergyResolutionConst", {0})) {
    m_EnergyResolutionConst[iEnergyResolutionConst] = EnergyResolutionConst;
    iEnergyResolutionConst++;
  }
  int iEnergyResolutionFactor = 0;
  for (double EnergyResolutionFactor : content.getArray("EnergyResolutionFactor", {0})) {
    m_EnergyResolutionFactor[iEnergyResolutionFactor] = EnergyResolutionFactor;
    iEnergyResolutionFactor++;
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

  B2INFO("CsiDigitizer_V2: Aquired csi locations and gas parameters");
  B2INFO("              from CSI.xml. There are " << nCSI << " CSIs implemented");

}


Double_t CsiDigitizer_v2Module::GetEnergyResolutionGeV(Double_t pEnergy, int pcry)
{
  // Returns energy resolution in GeV when supplied Energy in GeV
  return (m_EnergyResolutionFactor[pcry] * TMath::Sqrt(pEnergy) + m_EnergyResolutionConst[pcry] * pEnergy);

}


void CsiDigitizer_v2Module::endRun()
{
}

void CsiDigitizer_v2Module::terminate()
{
}


