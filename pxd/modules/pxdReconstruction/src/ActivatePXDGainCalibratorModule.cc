/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Benjamin Schwenker                                       *
 *                                                                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <pxd/modules/pxdReconstruction/ActivatePXDGainCalibratorModule.h>
#include <pxd/reconstruction/PXDGainCalibrator.h>

using namespace Belle2;
using namespace Belle2::PXD;
using namespace std;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(ActivatePXDGainCalibrator)

ActivatePXDGainCalibratorModule::ActivatePXDGainCalibratorModule() : Module()
{
  //Set module properties
  setDescription("Initialization of PXDGainCalibrator");
  setPropertyFlags(c_ParallelProcessingCertified);
}


void ActivatePXDGainCalibratorModule::initialize()
{
  PXDGainCalibrator::getInstance().initialize();
}


