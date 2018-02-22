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

// ECL
#include <ecl/modules/eclClusterPSD/ECLClusterPSD.h>
#include <ecl/dataobjects/ECLShower.h>
#include <ecl/dataobjects/ECLCalDigit.h>
#include <ecl/dataobjects/ECLEventInformation.h>
#include <ecl/digitization/EclConfiguration.h>
#include <mdst/dataobjects/ECLCluster.h>
// FRAMEWORK
#include <framework/datastore/RelationArray.h>
#include <framework/datastore/RelationIndex.h>
#include <framework/datastore/RelationsObject.h>
#include <framework/datastore/StoreArray.h>
#include <framework/gearbox/Unit.h>
#include <framework/logging/Logger.h>

using namespace Belle2;
using namespace ECL;

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
  StoreArray<ECLCalDigit> eclCalDigits(eclCalDigitArrayName());
  StoreArray<ECLShower> eclshowers("ECLShowers");
  eclshowers.registerInDataStore("ECLShowers");
}

// begin run
void ECLClusterPSDModule::beginRun()
{
}

void ECLClusterPSDModule::event()
{
  StoreArray<ECLShower> eclshowers;
  StoreArray<ECLCalDigit> eclCalDigits(eclCalDigitArrayName());

  for (auto& shower : eclshowers) {

    auto relatedDigits = shower.getRelationsTo<ECLCalDigit>();

    double cluster2CTotalEnergy = 0;
    double cluster2CHadronEnergy = 0;
    int numberofHadronDigits = 0;
    int nWaveforminCluster = 0;

    for (unsigned int iRel = 0; iRel < relatedDigits.size(); iRel++) {
      const auto caldigit = relatedDigits.object(iRel);
      const double digit2CChi2 = caldigit->getTwoComponentChi2();

      if (digit2CChi2 < 0)  continue; //only digits with waveforms

      if (digit2CChi2 < m_Chi2Threshold) { //must be a good fit

        const double digit2CTotalEnergy = caldigit->getTwoComponentTotalEnergy();
        const double digit2CHadronComponentEnergy = caldigit->getTwoComponentHadronEnergy();
        cluster2CTotalEnergy += digit2CTotalEnergy;
        cluster2CHadronEnergy += digit2CHadronComponentEnergy;
        if (digit2CHadronComponentEnergy > m_CrystalHadronEnergyThreshold)  numberofHadronDigits++;
        nWaveforminCluster++;
      }
    }

    if (nWaveforminCluster > 0) {
      if (cluster2CTotalEnergy != 0) shower.setShowerHadronIntensity(cluster2CHadronEnergy / cluster2CTotalEnergy);
      shower.setNumberofHadronDigits(numberofHadronDigits);
      shower.addStatus(ECLShower::c_hasPulseShapeDiscrimination);
    } else {
      shower.setShowerHadronIntensity(0);
      shower.setNumberofHadronDigits(0);
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

