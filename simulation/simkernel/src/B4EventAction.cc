/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Guofu Cao                                                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <simulation/simkernel/B4EventAction.h>
#include <simulation/simkernel/B4VHit.h>

#include "G4UImanager.hh"
#include "G4SDManager.hh"
#include "G4DigiManager.hh"

using namespace Belle2;

B4EventAction::B4EventAction(void): G4UserEventAction()
{
}


B4EventAction::~B4EventAction(void)
{
}


void B4EventAction::BeginOfEventAction(const G4Event* event)
{
  // Get event id
  G4int eventId = event->GetEventID();
  if (eventId % 100 == 0)  G4cout << "---> Begin of event: " << eventId << G4endl;
}


void B4EventAction::EndOfEventAction(const G4Event* event)
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
