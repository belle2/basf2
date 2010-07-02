/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Guofu Cao                                                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <cdc/simcdc/CDCConstruction.h>
#include <cdc/simcdc/CDCSD.h>

using namespace Belle2;

//-----------------------------------------------------------------------
// Create an instance of this class - necessary to register this driver.
//-----------------------------------------------------------------------
INSTANTIATE(CDCConstruction)

void CDCConstruction::initialize(TG4RootDetectorConstruction* dc)
{
  //Add CDC sensitive detector to sensitive detector manager (using auto-naming mechanism)
  setSensitiveDetectorAuto(new CDCSD("CDCSD", (2*24)*eV, 10*MeV), dc);
}
