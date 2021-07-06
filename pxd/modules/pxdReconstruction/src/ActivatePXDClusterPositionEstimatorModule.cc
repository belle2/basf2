/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <pxd/modules/pxdReconstruction/ActivatePXDClusterPositionEstimatorModule.h>
#include <pxd/reconstruction/PXDClusterPositionEstimator.h>

using namespace Belle2;
using namespace Belle2::PXD;
using namespace std;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(ActivatePXDClusterPositionEstimator)

ActivatePXDClusterPositionEstimatorModule::ActivatePXDClusterPositionEstimatorModule() : Module()
{
  //Set module properties
  setDescription("Initialization of PXDClusterPositionEstimator");
  setPropertyFlags(c_ParallelProcessingCertified);
}


void ActivatePXDClusterPositionEstimatorModule::initialize()
{
  PXDClusterPositionEstimator::getInstance().initialize();
}


