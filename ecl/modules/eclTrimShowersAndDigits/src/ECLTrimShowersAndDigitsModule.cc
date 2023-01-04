/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <ecl/modules/eclTrimShowersAndDigits/ECLTrimShowersAndDigitsModule.h>
#include <mdst/dataobjects/ECLCluster.h>
#include <ecl/dataobjects/ECLCalDigit.h>
#include <ecl/dataobjects/ECLShower.h>
#include <mdst/dataobjects/MCParticle.h>
#include <framework/datastore/DataStore.h>

#include <iostream>

using namespace std;
using namespace Belle2;


//-----------------------------------------------------------------
//..Register the Module
REG_MODULE(ECLTrimShowersAndDigits);


//-----------------------------------------------------------------
//..Implementation
ECLTrimShowersAndDigitsModule::ECLTrimShowersAndDigitsModule() : Module()
{
  // Set module properties
  setDescription("Create trimmed ECLCalDigit and ECLShower dataobjects");

}


//-----------------------------------------------------------------
//..Initialize
void ECLTrimShowersAndDigitsModule::initialize()
{

  //..Required data objects
  m_eclClusterArray.isRequired();
  m_eclShowerArray.isRequired();
  m_eclCalDigitArray.isRequired();

  //..Define the reduced data objects
  m_selectedShowers.registerSubset(m_eclShowerArray, m_ShowerArrayName);
  m_selectedShowers.inheritAllRelations();

  m_selectedDigits.registerSubset(m_eclCalDigitArray, m_DigitArrayName, DataStore::c_WriteOut);
  m_selectedDigits.inheritAllRelations();
}


//-----------------------------------------------------------------
//..Event
void ECLTrimShowersAndDigitsModule::event()
{

  //..ECLCluster containing the maximum MC true energy. Based on clusterTotalMCMatchWeight.
  const int nCluster = m_eclClusterArray.getEntries();
  double clusterMaxMCE = 0.;
  maxCellID = 9999; // valid cellIDs are [0, 8735]
  for (int i = 0; i < nCluster; i++) {
    if (m_eclClusterArray[i]->hasHypothesis(ECLCluster::EHypothesisBit::c_nPhotons)) {

      const auto clusterMCRelations = m_eclClusterArray[i]->getRelationsWith<MCParticle>();
      double mcE = 0.;
      for (unsigned int ir = 0; ir < clusterMCRelations.size(); ++ir) {
        mcE += clusterMCRelations.weight(ir);
      }
      if (mcE > clusterMaxMCE) {
        clusterMaxMCE = mcE;
        maxCellID = m_eclClusterArray[i]->getMaxECellId();
      }
    }
  }

  //..Only interested in events with a single MCParticle; more than that means
  //  a conversion happened.
  const int nMC = m_mcParticleArray.getEntries();
  if (nMC != 1) {maxCellID = 9999;}

  //..Select the matching ECLShower
  m_selectedShowers.select([this](const ECLShower * shower) {
    return this->selectShower(shower);
  });

  //..Select matching ECLCalDigits
  m_selectedDigits.select([this](const ECLCalDigit * digit) {
    return this->selectDigit(digit);
  });
}

//-----------------------------------------------------------------
//..Terminate
void ECLTrimShowersAndDigitsModule::terminate()
{
}

//-----------------------------------------------------------------
//..Select the ECLShowers to keep
bool ECLTrimShowersAndDigitsModule::selectShower(const ECLShower* shower)
{
  //..ECLShower must be a photon hypothesis and be related to correct ECLCluster
  bool returnValue = false;
  const auto clusterShowerRelations = shower->getRelationsWith<ECLCluster>();
  if (clusterShowerRelations.size() > 0 and shower->getHypothesisId() == ECLShower::c_nPhotons) {
    const auto cluster = clusterShowerRelations.object(0);
    const unsigned short cellID = cluster->getMaxECellId();
    if (cellID == maxCellID) {returnValue = true;}
  }
  return returnValue;
}

//-----------------------------------------------------------------
//..Select the ECLCalDigits to keep
bool ECLTrimShowersAndDigitsModule::selectDigit(const ECLCalDigit* digit)
{
  //..ECLCalDigit must be related to the correct ECLCluster (identified by cellID)
  bool returnValue = false;
  const auto clusterDigitRelations = digit->getRelationsWith<ECLCluster>();
  for (unsigned int ir = 0; ir < clusterDigitRelations.size(); ++ir) {
    const auto cluster = clusterDigitRelations.object(ir);
    const unsigned short cellID = cluster->getMaxECellId();
    if (cellID == maxCellID) {returnValue = true;}
  }
  return returnValue;
}
