/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * This module performs the calibration for EM shower. This calibration   *
 * corrects for differences between data and MC: corr = data / MC         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Torben Ferber (ferber@physics.ubc.ca)                    *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

// THIS MODULE
#include <ecl/modules/eclShowerCalibration/ECLShowerCalibratorModule.h>

// FRAMEWORK
#include <framework/logging/Logger.h>

// ECL
#include <ecl/dataobjects/ECLConnectedRegion.h>
#include <ecl/dataobjects/ECLShower.h>

using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(ECLShowerCalibrator)
REG_MODULE(ECLShowerCalibratorPureCsI)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

ECLShowerCalibratorModule::ECLShowerCalibratorModule() : Module()
{
  // Set description
  setDescription("ECLShowerCalibratorModule: Corrects for data to MC differences");
  setPropertyFlags(c_ParallelProcessingCertified);

}

ECLShowerCalibratorModule::~ECLShowerCalibratorModule()
{
}


void ECLShowerCalibratorModule::initialize()
{
  // Register in datastore
  m_eclShowers.registerInDataStore(eclShowerArrayName());
  m_eclCRs.registerInDataStore(eclConnectedRegionArrayName());
}

void ECLShowerCalibratorModule::beginRun()
{
  // Do not use this for Database updates
  ;
}

void ECLShowerCalibratorModule::event()
{

  // loop over all ECLShowers
//  for (const auto& eclShower : eclShowers) {
  // check hypothesis, do other things...

//  }

}

void ECLShowerCalibratorModule::endRun()
{
  ;
}

void ECLShowerCalibratorModule::terminate()
{
  ;
}
