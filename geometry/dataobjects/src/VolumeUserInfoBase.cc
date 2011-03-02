/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <geometry/dataobjects/VolumeUserInfoBase.h>

#include <globals.hh>
#include <G4UserLimits.hh>

using namespace std;
using namespace Belle2;

ClassImp(VolumeUserInfoBase)


void VolumeUserInfoBase::updateG4Volume(G4VPhysicalVolume* g4Volume, TG4RootDetectorConstruction *detConstruct)
{
  if (m_stepSize > 0.0) {
    g4Volume->GetLogicalVolume()->SetUserLimits(new G4UserLimits(m_stepSize * cm)); //convert to Geant4 units
  }
}
