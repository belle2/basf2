/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll, Peter Kvasnicka                                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <pxd/dataobjects/PXDVolumeUserInfo.h>

#include <G4LogicalVolume.hh>
#include <G4LogicalVolumeStore.hh>
#include <G4VPhysicalVolume.hh>
#include <G4PhysicalVolumeStore.hh>
#include <G4Region.hh>
#include <G4RegionStore.hh>
#include <G4UserLimits.hh>
#include <G4ProductionCuts.hh>
#include <TG4RootDetectorConstruction.h>

#include <framework/logging/Logger.h>

using namespace std;
using namespace Belle2;

ClassImp(PXDVolumeUserInfo)

PXDVolumeUserInfo& PXDVolumeUserInfo::operator=(const PXDVolumeUserInfo & other)
{
  VolumeUserInfoBase::operator=(other);
  this->m_layerID = other.getLayerID();
  this->m_ladderID = other.getLadderID();
  this->m_sensorID = other.getSensorID();
  this->m_uPitch = other.getUPitch();
  this->m_uCells = other.getUCells();
  this->m_vPitch = other.getVPitch();
  this->m_vCells = other.getVCells();
  return *this;
}

void PXDVolumeUserInfo::updateG4Volume(G4VPhysicalVolume* g4Volume, TG4RootDetectorConstruction *detConstruct)
{
  // No call of the base class method, we now also set the step length region-wise.

  // Get the PXDActiveRegion from Geant; no complaints if not found.
  G4String regionName(PXDACTIVE_G4REGION_NAME);
  G4Region* region = G4RegionStore::GetInstance()->GetRegion(regionName, false);

  // If there is no such region, create it and set stepsize and rangecut.
  if (region == NULL) {

    // Get "world" volume = the volume of the detector.
    G4VPhysicalVolume* pxdVol = G4PhysicalVolumeStore::GetInstance()->GetVolume("PXD");

    region = new G4Region(regionName);
    region->SetWorld(pxdVol);

    // Set step length
    if (m_stepSize > 0.0) {
      region->SetUserLimits(new G4UserLimits(m_stepSize * cm)); //convert to Geant4 units
    } // if stepSize

    // Set range cut
    if (m_rangeCut > 0) {
      G4ProductionCuts *cuts = new G4ProductionCuts;
      cuts->SetProductionCut(m_rangeCut * cm, G4ProductionCuts::GetIndex("e-")); // only set for e-
      // TODO: check cut settings and distributions of other particles.
      region->SetProductionCuts(cuts);
    } // if rangeCut
  } // if new region

  // Add the current volume to region
  region->AddRootLogicalVolume(g4Volume->GetLogicalVolume());
  region->RegionModified(true);

} // updateG4Volume

