/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Laura Zani                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <SVDCalibrationsMonitorModule.h>


using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(SVDCalibrationsMonitor)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

SVDCalibrationsMonitorModule::SVDCalibrationsMonitorModule() : Module()
{
  // Set module properties
  setDescription("Module to check the updated calibration constants");

  // Parameter definitions

}

void SVDCalibrationsMonitorModule::initialize()
{
}

void SVDCalibrationsMonitorModule::beginRun()
{
}

void SVDCalibrationsMonitorModule::event()
{
}

void SVDCalibrationsMonitorModule::endRun()
{
}

void SVDCalibrationsMonitorModule::terminate()
{
}


