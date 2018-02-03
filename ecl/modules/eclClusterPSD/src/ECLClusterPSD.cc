/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
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
//
void ECLClusterPSDModule::event()
{
  StoreArray<ECLShower> eclshowers;
  StoreArray<ECLCalDigit> eclCalDigits(eclCalDigitArrayName());
  //
  for (unsigned int ishowers = 0; ishowers < (unsigned int)eclshowers.getEntries() ; ishowers++) {
    //
    auto relatedDigits = eclshowers[ishowers]->getRelationsTo<ECLCalDigit>();
    //
    double Cluster2CTotalEnergy = 0;
    double Clust2CHadronEnergy = 0;
    int NumberofHadronDigits = 0;
    int nWaveforminCluster = 0;
    //
    for (unsigned int iRel = 0; iRel < relatedDigits.size(); iRel++) {
      const auto caldigit = relatedDigits.object(iRel);
      double Digit2CChi2 = caldigit->getTwoCompChi2();
      //
      if (Digit2CChi2 < 0)  continue; //onl digits with waveforms
      //
      if (Digit2CChi2 < m_Chi2Threshold) { //must be a good fit
        //
        double Digit2CTotalEnergy = caldigit->getTwoCompTotalEnergy();
        double Digit2CHadronComponentEnergy = caldigit->getTwoCompHadronEnergy();
        Cluster2CTotalEnergy += Digit2CTotalEnergy;
        Clust2CHadronEnergy += Digit2CHadronComponentEnergy;
        if (Digit2CHadronComponentEnergy > m_CrystalHadronEnergyThreshold)  NumberofHadronDigits++;
        nWaveforminCluster++;
      }
    }
    //
    if (nWaveforminCluster > 0) {
      if (Cluster2CTotalEnergy != 0) eclshowers[ishowers]->setShowerHadronIntensity(Clust2CHadronEnergy / Cluster2CTotalEnergy);
      eclshowers[ishowers]->setNumberofHadronDigits(NumberofHadronDigits);
    } else {
      eclshowers[ishowers]->setShowerHadronIntensity(-999);
      eclshowers[ishowers]->setNumberofHadronDigits(-1);
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

