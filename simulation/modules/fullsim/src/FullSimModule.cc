/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll, Guofu Cao                                  *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <simulation/modules/fullsim/FullSimModule.h>
#include <simulation/kernel/RunManager.h>
#include <simulation/kernel/DetectorConstruction.h>
#include <simulation/kernel/PhysicsList.h>

#include <QGSP_BERT.hh>

#include <TGeoManager.h>
#include <TG4RootNavMgr.h>

using namespace std;
using namespace Belle2;
using namespace Belle2::Simulation;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(FullSimModule, "FullSim")

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

FullSimModule::FullSimModule() : Module()
{
  //Set module properties
  setDescription("Performs the full Geant4 detector simulation. Requires a valid geometry in memory.");

  //Parameter definition
}


FullSimModule::~FullSimModule()
{

}


void FullSimModule::initialize()
{
  //Perform some initial checks
  if (gGeoManager == NULL) {
    B2ERROR("No geometry was found in memory (gGeoManager is NULL) !")
    return;
  }

  if (!gGeoManager->IsClosed()) {
    B2ERROR("The geometry was not closed ! Make sure gGeoManager->CloseGeometry() was called.")
    return;
  }

  //Create an instance of the G4Root Navigation manager.
  TG4RootNavMgr* g4rootNavMgr = TG4RootNavMgr::GetInstance(gGeoManager);

  if (g4rootNavMgr == NULL) {
    B2ERROR("Could not retrieve an instance of the TG4RootNavMgr !")
    return;
  }

  //Get the instance of the run manager.
  RunManager& runManager = RunManager::Instance();

  //Convert the TGeo volumes/materials to Geant4 volumes/materials and then
  //call the DetectorConstruction class to allow the user based modification
  //of the created geant4 volumes/materials.
  g4rootNavMgr->Initialize(new DetectorConstruction());
  g4rootNavMgr->ConnectToG4();

  //Create the Physics list
  runManager.SetUserInitialization(new PhysicsList<QGSP_BERT>);

  //Generator action
  //G4VUserPrimaryGeneratorAction* generatorAction = new B4PrimaryGeneratorAction();
  //runManager.SetUserAction(generatorAction);

}


void FullSimModule::beginRun()
{

}


void FullSimModule::event()
{

}


void FullSimModule::endRun()
{

}


void FullSimModule::terminate()
{

}
