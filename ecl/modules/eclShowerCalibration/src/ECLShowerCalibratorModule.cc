/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

// THIS MODULE
#include <ecl/modules/eclShowerCalibration/ECLShowerCalibratorModule.h>

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
