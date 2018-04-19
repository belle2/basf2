/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 *   This module computes shower variables using pulse shape              *
 *   information from offline two component fits.  Using pulse            *
 *   shape discrimination, these shower variables can be used             *
 *   for particle id.                                                     *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors:  Savino Longo (longos@uvic.ca)                           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
//This module
#include <ecl/modules/eclClusterPSD/ECLClusterPSD.h>

// ECL
#include <ecl/dataobjects/ECLShower.h>
#include <ecl/dataobjects/ECLCalDigit.h>

// FRAMEWORK
#include <framework/logging/Logger.h>

using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Modules
//-----------------------------------------------------------------
REG_MODULE(ECLClusterPSD)
//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

// constructor
ECLClusterPSDModule::ECLClusterPSDModule()
{
  // Set module properties
  setDescription("Module uses offline two component fit results to compute pulse shape discrimation variables for particle identification.");
  setPropertyFlags(c_ParallelProcessingCertified);
  addParam("Chi2Threshold", m_Chi2Threshold, "Chi2 Threshold", 20.);
  addParam("CrystalHadronEnergyThreshold", m_CrystalHadronEnergyThreshold,
           "Hadron component energy threshold to identify as hadron digit.(GeV)", 0.003);
}

// destructor
ECLClusterPSDModule::~ECLClusterPSDModule()
{
}

// initialize
void ECLClusterPSDModule::initialize()
{
  // ECL dataobjects
  m_eclShowers.registerInDataStore(eclShowerArrayName());
  m_eclCalDigits.registerInDataStore(eclCalDigitArrayName());
}

// begin run
void ECLClusterPSDModule::beginRun()
{
}

void ECLClusterPSDModule::event()
{

  for (auto& shower : m_eclShowers) {

    auto relatedDigits = shower.getRelationsTo<ECLCalDigit>();

    double cluster2CTotalEnergy = 0;
    double cluster2CHadronEnergy = 0;
    double numberofHadronDigits = 0;
    double nWaveforminCluster = 0;

    for (unsigned int iRel = 0; iRel < relatedDigits.size(); iRel++) {

      const auto weight = relatedDigits.weight(iRel);

      const auto caldigit = relatedDigits.object(iRel);
      const double digit2CChi2 = caldigit->getTwoComponentChi2();

      if (digit2CChi2 < 0)  continue; //only digits with waveforms

      if (digit2CChi2 < m_Chi2Threshold) { //must be a good fit

        const double digit2CTotalEnergy = caldigit->getTwoComponentTotalEnergy();
        const double digit2CHadronComponentEnergy = caldigit->getTwoComponentHadronEnergy();
        cluster2CTotalEnergy += digit2CTotalEnergy;
        cluster2CHadronEnergy += digit2CHadronComponentEnergy;
        if (digit2CHadronComponentEnergy > m_CrystalHadronEnergyThreshold)  numberofHadronDigits += weight;
        nWaveforminCluster += weight;
      }
    }

    if (nWaveforminCluster > 0) {
      if (cluster2CTotalEnergy != 0) shower.setShowerHadronIntensity(cluster2CHadronEnergy / cluster2CTotalEnergy);
      shower.setNumberOfHadronDigits(numberofHadronDigits);
      shower.addStatus(ECLShower::c_hasPulseShapeDiscrimination);
    } else {
      shower.setShowerHadronIntensity(0);
      shower.setNumberOfHadronDigits(0);
    }
  }
}

// end run
void ECLClusterPSDModule::endRun()
{
}

// terminate
void ECLClusterPSDModule::terminate()
{
}
