/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * This module performs the correction for EM shower (mainly longitudinal *
 * leakage): corr = (Reconstructed / Truth).                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Torben Ferber (ferber@physics.ubc.ca) (TF)               *
 *               Alon Hershenhorn (hershen@physics.ubc.ca)                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

// THIS MODULE
#include <ecl/modules/eclShowerCorrection/ECLShowerCorrectorModule.h>

// FRAMEWORK
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/RelationArray.h>
#include <framework/gearbox/Unit.h>
#include <framework/logging/Logger.h>
#include <framework/utilities/FileSystem.h>
#include <framework/database/DBArray.h>

// ECL
#include <ecl/dataobjects/ECLConnectedRegion.h>
#include <ecl/dataobjects/ECLShower.h>

// OTHER
#include <vector>
#include <fstream>      // std::ifstream

using namespace Belle2;
using namespace ECL;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(ECLShowerCorrector)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

ECLShowerCorrectorModule::ECLShowerCorrectorModule() : Module(), m_leakageCorrectionArray("ecl_leakage_corrections")
{
  // Set description
  setDescription("ECLShowerCorrectorModule: Corrects for MC truth to reconstruction differences");
  setPropertyFlags(c_ParallelProcessingCertified);

}

ECLShowerCorrectorModule::~ECLShowerCorrectorModule()
{
}

void ECLShowerCorrectorModule::initialize()
{
  // Register in datastore
  StoreArray<ECLShower> eclShowers(eclShowerArrayName());
  eclShowers.registerInDataStore();
}

void ECLShowerCorrectorModule::beginRun()
{
  // TODO: callback!
  prepareLeakageCorrections();
}

void ECLShowerCorrectorModule::event()
{
  // input array
  StoreArray<ECLShower> eclShowers(eclShowerArrayName());

  // loop over all ECLShowers
  for (auto& eclShower : eclShowers) {

    // only correct N1 showers!
    if (eclShower.getHypothesisId() == ECLConnectedRegion::c_N1) {

      const double energy        = eclShower.getEnergy();
      const double energyHighest = eclShower.getEnergyHighestCrystal();
      const double theta         = eclShower.getTheta();
      const double background    = 1.0; //TODO get background from eclcount and decide if its zero or 1

      // Get the correction
      const double correctionFactor = getLeakageCorrection(theta, energy, background);
      const double correctedEnergy = energy * correctionFactor;
      const double correctedEnergyHighest = energyHighest * correctionFactor;
      // ...

      // Set the correction
      eclShower.setEnergy(correctedEnergy);
      eclShower.setEnergyHighestCrystal(correctedEnergyHighest);
    } // end correction N1 only
  } // end loop over all shower

}

void ECLShowerCorrectorModule::endRun()
{
  ;
}

void ECLShowerCorrectorModule::terminate()
{
  ;
}

void ECLShowerCorrectorModule::prepareLeakageCorrections()
{
  ;
}


double ECLShowerCorrectorModule::getLeakageCorrection(const double theta, const double energy, const double background) const
{
  return 1.0;
}

double ECLShowerCorrectorModule::getLeakageCorrectionUncertainty(const double theta, const double energy,
    const double background) const
{
  return 0.0;
}
