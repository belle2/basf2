/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * This module calculates shower shape variables.                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Torben Ferber (ferber@physics.ubc.ca)                    *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

// THIS MODULE
#include <ecl/modules/eclShowerShape/ECLShowerShapeModule.h>

// FRAMEWORK
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/RelationArray.h>
#include <framework/datastore/RelationVector.h>
#include <framework/gearbox/Unit.h>
#include <framework/logging/Logger.h>

// ECL
#include <ecl/dataobjects/ECLCalDigit.h>
#include <ecl/dataobjects/ECLConnectedRegion.h>
#include <ecl/dataobjects/ECLShower.h>

using namespace Belle2;
using namespace ECL;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(ECLShowerShape)
REG_MODULE(ECLShowerShapePureCsI)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

ECLShowerShapeModule::ECLShowerShapeModule() : Module()
{
  // Set description
  setDescription("ECLShowerShapeModule: Calculate ECL shower shape variable (e.g. E9E25)");
  setPropertyFlags(c_ParallelProcessingCertified);

}

ECLShowerShapeModule::~ECLShowerShapeModule()
{
}

void ECLShowerShapeModule::initialize()
{
  // Register in datastore
  StoreArray<ECLCalDigit> eclCalDigits(eclCalDigitArrayName());
  StoreArray<ECLShower> eclShowers(eclShowerArrayName());
  StoreArray<ECLConnectedRegion> eclCRs(eclConnectedRegionArrayName());

  eclCalDigits.registerInDataStore();
  eclShowers.registerInDataStore();
  eclCRs.registerInDataStore();

}

void ECLShowerShapeModule::beginRun()
{
  // Do not use this for Database updates, they will not follow the concept of a "run"
  ;
}

void ECLShowerShapeModule::event()
{
  // input array
  StoreArray<ECLCalDigit> eclCalDigits(eclCalDigitArrayName());
  StoreArray<ECLShower> eclShowers(eclShowerArrayName());

  // loop over all ECLShowers
  for (const auto& eclShower : eclShowers) {

    // get the relations vector <digit, weight>
    auto relatedDigitsPairs = eclShower.getRelationsTo<ECLCalDigit>();

    // loop over all <digit, weight> pairs that are related to this shower
    for (unsigned int iRel = 0; iRel < relatedDigitsPairs.size(); iRel++) {
      const auto aECLCalDigit = relatedDigitsPairs.object(iRel);
      const auto weight = relatedDigitsPairs.weight(iRel);
      B2INFO("ECLShowerShapeModule: ECLCalDigit pointer: " << aECLCalDigit << " weight: " << weight);
    }

  }

}

void ECLShowerShapeModule::endRun()
{
  ;
}

void ECLShowerShapeModule::terminate()
{
  ;
}
