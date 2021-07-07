/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <vtx/modules/vtxReconstruction/ActivateVTXClusterPositionEstimatorModule.h>
#include <vtx/reconstruction/VTXClusterPositionEstimator.h>

using namespace Belle2;
using namespace Belle2::VTX;
using namespace std;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(ActivateVTXClusterPositionEstimator)

ActivateVTXClusterPositionEstimatorModule::ActivateVTXClusterPositionEstimatorModule() : Module()
{
  //Set module properties
  setDescription("Initialization of VTXClusterPositionEstimator");
  setPropertyFlags(c_ParallelProcessingCertified);
}


void ActivateVTXClusterPositionEstimatorModule::initialize()
{
  VTXClusterPositionEstimator::getInstance().initialize();
}


