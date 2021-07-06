/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <pxd/modules/pxdReconstruction/ActivatePXDPixelMaskerModule.h>
#include <pxd/reconstruction/PXDPixelMasker.h>

using namespace Belle2;
using namespace Belle2::PXD;
using namespace std;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(ActivatePXDPixelMasker)

ActivatePXDPixelMaskerModule::ActivatePXDPixelMaskerModule() : Module()
{
  //Set module properties
  setDescription("Initialization of PXDPixelMasker");
  setPropertyFlags(c_ParallelProcessingCertified);
}


void ActivatePXDPixelMaskerModule::initialize()
{
  PXDPixelMasker::getInstance().initialize();
}


