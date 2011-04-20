/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll, Guofu Cao                                  *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <simulation/kernel/VGMDetectorConstruction.h>

#include <G4VPhysicalVolume.hh>

#include <Geant4GM/volumes/Factory.h>
#include <RootGM/volumes/Factory.h>
#include <TGeoManager.h>

using namespace std;
using namespace Belle2;
using namespace Belle2::Simulation;


VGMDetectorConstruction::VGMDetectorConstruction()
{

}


VGMDetectorConstruction::~VGMDetectorConstruction()
{

}


G4VPhysicalVolume* VGMDetectorConstruction::Construct()
{
  //Import geometry from Root to VGM
  RootGM::Factory rootFactory;
  //rootFactory.SetDebug(1);
  rootFactory.Import(gGeoManager->GetTopNode());

  //Export VGM geometry to Geant4
  Geant4GM::Factory geant4Factory;
  //geant4Factory.SetDebug(1);
  rootFactory.Export(&geant4Factory);
  G4VPhysicalVolume* world = geant4Factory.World();

  return world;
}
