/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Guofu Cao, Andreas Moll                                  *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <simulation/kernel/EventAction.h>
//#include <simulation/kernel/SimHit.h>
#include <simulation/simkernel/B4VHit.h>

#include <G4UImanager.hh>
#include <G4SDManager.hh>
#include <G4DigiManager.hh>

using namespace Belle2;
using namespace Belle2::Simulation;


EventAction::EventAction(): G4UserEventAction()
{

}


EventAction::~EventAction()
{

}


void EventAction::BeginOfEventAction(const G4Event* event)
{

}


void EventAction::EndOfEventAction(const G4Event* event)
/*{
  //Get the pointer of the sensitive detector manager.
  G4SDManager* SDman = G4SDManager::GetSDMpointer();

  //Get the pointer of the hits collection table.
  G4HCtable* HCtable = SDman->GetHCtable();

  //Get the pointer of the hits collection in this event.
  G4HCofThisEvent* HCE = event->GetHCofThisEvent();

  //Get the number of the hits collections.
  G4int nCols = HCtable->entries();

  //Loop over each hits collection and check if it is registered
  //by a sensitive detector. If true, then save the hits.
  for (G4int iCols = 0; iCols < nCols; ++iCols) {

    //Get the hits collection name.
    G4String colName = HCtable->GetHCname(iCols);

    //Get the sensitive detector name.
    G4String SDName = HCtable->GetSDname(iCols);

    //Check if this sensitive detector is existing.
    G4VSensitiveDetector* SD = SDman->FindSensitiveDetector(SDName, false);

    if (SD != NULL) {
      //Get the hits collection id.
      G4int colID = SDman->GetCollectionID(colName);

      //Get the pointer of the hits collection.
      HitsCollection* hitCol = (HitsCollection*) HCE->GetHC(colID);

      //Get the number of the hits in this collection.
      G4int nHits = hitCol->entries();

      //Loop over each hit and save the hit information into the DataStore.
      for (G4int iHits = 0; iHits < nHits; ++iHits) {
        (*hitCol)[iHits]->Save(iHits);
      }
    }
  }
}*/

{
  //------------------------------------------------
  // Get the pointer of sensitive detector manager.
  //------------------------------------------------
  G4SDManager* SDman = G4SDManager::GetSDMpointer();

  //------------------------------------------
  // Get the pointer of hits collection table.
  //------------------------------------------
  G4HCtable* HCtable = SDman->GetHCtable();

  //-------------------------------------------------
  // Get the pointer of hits collection in this event.
  //-------------------------------------------------
  G4HCofThisEvent* HCE = event->GetHCofThisEvent();

  //-------------------------------------
  // Get the number of hits collections.
  //-------------------------------------
  G4int ncols = HCtable->entries();

  //----------------------------------------------------------------
  // Loop over each hits collection and check if it is registered
  // by a sensitive detector. If true, then save hits.
  //----------------------------------------------------------------
  for (G4int icols = 0; icols < ncols; icols++) {
    // Get hits collection name.
    G4String colName = HCtable->GetHCname(icols);

    // Get sensitive detector name.
    G4String SDName = HCtable->GetSDname(icols);

    // Check if this sensitive detector is existing.
    G4VSensitiveDetector* SD = SDman->FindSensitiveDetector(SDName, false);

    if (SD != 0) {
      // Get hits collection id.
      G4int colID = SDman->GetCollectionID(colName);

      // Get the pointer of hits collection.
      B4VHitsCollection* B4HC = (B4VHitsCollection*) HCE->GetHC(colID);

      // Get the number of hits in this collection.
      G4int nhits = B4HC->entries();

      // Loop over each hit and save hit information into data store.
      for (G4int ihits = 0; ihits < nhits; ihits++) {
        (*B4HC)[ihits]->Save(ihits);
      }
    }
  }
}
