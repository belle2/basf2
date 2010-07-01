/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Guofu Cao                                                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <simulation/simkernel/B4SensitiveDetectorBase.h>

#include "G4SDManager.hh"

using namespace Belle2;

B4SensitiveDetectorBase::B4SensitiveDetectorBase(const G4String name) : G4VSensitiveDetector(name)
{
  //------------------------
  // Default HitCollection.
  //------------------------
  collectionName.insert(name);
  G4SDManager *SDman = G4SDManager::GetSDMpointer();
  SDman->AddNewDetector(this);
  m_hcid.push_back(GetCollectionID(0));
}

G4int B4SensitiveDetectorBase::RegisterNewCollection(const G4String &name)
{
  //---------------------------------------------------------------
  // Add a new collection into Geant4 sensitive detector manager.
  //---------------------------------------------------------------
  G4SDManager *fSDM = G4SDManager::GetSDMpointer();
  fSDM->AddNewCollection(SensitiveDetectorName, name);
  G4int clid = GetNumberOfCollections();
  collectionName.insert(name);
  m_hcid.push_back(GetCollectionID(clid));
  return clid;
}

void B4SensitiveDetectorBase::Initialize(G4HCofThisEvent *HCTE)
{
  //------------------------------------------------------------------------------
  // Create hits collections and add them into G4HCofThisEvent which is a class
  // which stores hits collections generated at one event.
  //------------------------------------------------------------------------------
  m_collection.clear();
  CreateCollection();
  for (int i = 0; i < GetNumberOfCollections(); ++i) {
    HCTE->AddHitsCollection(m_hcid[i], m_collection[i]);
  }
}

void B4SensitiveDetectorBase::AddbgOne(bool)
{
}
