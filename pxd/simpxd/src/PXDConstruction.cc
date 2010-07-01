/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <pxd/simpxd/PXDConstruction.h>
#include <pxd/simpxd/PXDSensitiveDetector.h>

#include <string>

using namespace Belle2;
using namespace std;

//-----------------------------------------------------------------------
// Create an instance of this class - necessary to register this driver.
//-----------------------------------------------------------------------
INSTANTIATE(PXDConstruction)


PXDConstruction::PXDConstruction() : B4VSubDetectorDriver(string("PXDBelleII"))
{

}


void PXDConstruction::initialize(TG4RootDetectorConstruction* dc)
{
  //Add PXD sensitive detector to sensitive detector manager (using auto-naming mechanism)
  setSensitiveDetAndVolumes(new PXDSensitiveDetector("PXDSensitiveDetector"), dc);
}
