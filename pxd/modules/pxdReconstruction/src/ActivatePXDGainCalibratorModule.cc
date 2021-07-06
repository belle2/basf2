/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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


