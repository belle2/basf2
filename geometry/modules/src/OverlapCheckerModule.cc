/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2011 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Kuhr                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <geometry/modules/OverlapCheckerModule.h>
#include <geometry/GeometryManager.h>

#include "G4VPhysicalVolume.hh"
#include "G4LogicalVolume.hh"
#include "G4UnitsTable.hh"

using namespace Belle2;

REG_MODULE(OverlapChecker);

OverlapCheckerModule::OverlapCheckerModule()
{
  //Set module properties and the description
  setDescription("Checks the geometry for overlaps.");

  //Parameter definition
  addParam("Points", m_points, "Number of test points.", 1000);
  addParam("Tolerance", m_tolerance, "Tolerance of overlap check.", 0.);
}

void OverlapCheckerModule::initialize()
{
  //Check the geometry tree for overlaps
  G4UnitDefinition::BuildUnitsTable();
  G4VPhysicalVolume* volume = geometry::GeometryManager::getInstance().getTopVolume();
  if (!volume) {
    B2ERROR("No geometry found. => Add the Geometry module to the path before the OverlapChecker module.");
    return;
  }
  checkVolume(volume, "");

  //Print the list of found overlaps
  for (unsigned int iOverlap = 0; iOverlap < m_overlaps.size(); iOverlap++) {
    B2ERROR("Overlaps detected for " << m_overlaps[iOverlap]);
  }
}

bool OverlapCheckerModule::checkVolume(G4VPhysicalVolume* volume, const std::string& path)
{
  //Check the given volume for overlaps
  std::string volumePath = path + "/" + volume->GetName();
  bool result = volume->CheckOverlaps(m_points, m_tolerance);
  if (result) {
    m_overlaps.push_back(volumePath);
  }

  //Check the daughter volumes for overlaps
  G4LogicalVolume* logicalVolume = volume->GetLogicalVolume();
  for (int iDaughter = 0; iDaughter < logicalVolume->GetNoDaughters(); iDaughter++) {
    G4VPhysicalVolume* daughter = logicalVolume->GetDaughter(iDaughter);
    result = checkVolume(daughter, volumePath) || result;
  }
  return result;
}
