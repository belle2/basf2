/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2011 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Kuhr                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <geometry/modules/overlapchecker/OverlapCheckerModule.h>
#include <geometry/GeometryManager.h>
#include <framework/gearbox/Unit.h>

#include "G4VPhysicalVolume.hh"
#include "G4LogicalVolume.hh"
#include "G4UnitsTable.hh"
#include "G4VExceptionHandler.hh"
#include "G4StateManager.hh"
#include "G4AffineTransform.hh"
#include "G4VSolid.hh"

#include <regex>

using namespace Belle2;

REG_MODULE(OverlapChecker);

namespace {
  /** Class to handle G4Exceptions raised when an overlap if found during overlap check */
  class OverlapHandler: public G4VExceptionHandler {
  public:
    /** Constructor which just takes a callback function pointer to be called in case of exceptions */
    OverlapHandler(std::function<void(const std::string&)> callback): m_callback(callback) {}
    /** Called when an exception is raised. Calls the callback and does nothing else */
    virtual bool Notify(const char*, const char*, G4ExceptionSeverity, const char* description)
    {
      std::string message{description};
      m_callback(description);
      return false;
    }
  private:
    /** Pointer to the callback function */
    std::function<void(const std::string&)> m_callback;
  };
}

OverlapCheckerModule::OverlapCheckerModule()
{
  //Set module properties and the description
  setDescription("Checks the geometry for overlaps.");

  //Parameter definition
  addParam("points", m_points, "Number of test points.", m_points);
  addParam("tolerance", m_tolerance, "Tolerance of overlap check.", m_tolerance);
  addParam("maxErrors", m_maxErrors, "Number of overlap errors per volume before continuing with next volume", m_maxErrors);
  addParam("maxDepth", m_maxDepth, "Maximum depth to go into the geometry tree, 0 means no maximum", m_maxDepth);
  addParam("prefix", m_prefix, "Prefix path, only volumes starting with the given path are checked", m_prefix);
}

void OverlapCheckerModule::initialize()
{
  m_displayData.registerInDataStore();
  if (m_maxErrors <= 0) m_maxErrors = m_points + 1;
}

void OverlapCheckerModule::event()
{
  if (!m_displayData.isValid()) m_displayData.create();
  //Check the geometry tree for overlaps
  G4UnitDefinition::BuildUnitsTable();
  G4VPhysicalVolume* volume = geometry::GeometryManager::getInstance().getTopVolume();
  if (!volume) {
    B2ERROR("No geometry found. => Add the Geometry module to the path before the OverlapChecker module.");
    return;
  }
  m_seen.clear();
  // reset the navigation history to be in the top level volume
  m_nav.Reset();
  // remember the existing G4Exception handler and set our own one to find the position of the intersection
  G4VExceptionHandler* old = G4StateManager::GetStateManager()->GetExceptionHandler();
  // set our own exception handler
  OverlapHandler handler([&](const std::string & message) { handleOverlap(message); });
  G4StateManager::GetStateManager()->SetExceptionHandler(&handler);
  // now check for overlaps
  checkVolume(volume, "");
  // and reset the exception handler
  G4StateManager::GetStateManager()->SetExceptionHandler(old);

  //Print the list of found overlaps
  for (unsigned int iOverlap = 0; iOverlap < m_overlaps.size(); iOverlap++) {
    B2ERROR("Overlaps detected for " << m_overlaps[iOverlap]);
  }
}

void OverlapCheckerModule::handleOverlap(const std::string& geant4Message)
{
  // ok, let's handle the Overlap message
  G4VPhysicalVolume* volume = m_nav.GetTopVolume();
  m_nav.BackLevel();
  B2ERROR(geant4Message);
  std::regex r("(mother)?\\s*local point \\(([-+0-9eE.]+),([-+0-9eE.]+),([-+0-9eE.]+)\\)");
  std::smatch m;
  if (std::regex_search(geant4Message, m, r)) {
    G4ThreeVector posLocal(std::atof(m[2].str().c_str()), std::atof(m[3].str().c_str()), std::atof(m[4].str().c_str()));
    if (m[1].length() == 0) {
      // Damn you Geant4, giving me the coordinates in the coordinate system of
      // the sister volume which I don't know exactly is almost useless. Aaah
      // darn it, let's check all sisters, see if they have the correct name
      // and if so check if the point transformed to the mother system is
      // inside the one were checking. So let's remember the transformation
      // relative to the top volume of our current volume.
      G4AffineTransform trans_volume(volume->GetRotation(), volume->GetTranslation());
      trans_volume.Invert();
      // And the solid
      G4VSolid* solid = volume->GetLogicalVolume()->GetSolid();
      // Now look for the name of the intersecting volume in the exception message (jaaaay)
      std::regex nameRegex("with (.*) volume's");
      std::smatch nameMatch;
      // And if we can find the name we can draw the intersection
      if (std::regex_search(geant4Message, nameMatch, nameRegex)) {
        const std::string& name = nameMatch[1].str();
        // By looping over all sisters
        for (int i = 0; i < volume->GetMotherLogical()->GetNoDaughters(); ++i) {
          G4VPhysicalVolume* sister = volume->GetMotherLogical()->GetDaughter(i);
          // ignoring the ones which don't match the name
          if (name != sister->GetName()) continue;
          // now transform the point into the coordinate system of the volume we actually look at
          G4AffineTransform trans_sister(sister->GetRotation(), sister->GetTranslation());
          G4ThreeVector posMother = trans_sister.TransformPoint(posLocal);
          G4ThreeVector posSister = trans_volume.TransformPoint(posMother);
          // and check if the point is inside our volume
          if (solid->Inside(posSister) != kOutside) {
            // if so this is the right volume which is intersecting.
            B2INFO("Found intersecting volume " << sister->GetName() << "." << sister->GetCopyNo());
            // so add the point to the display data
            G4AffineTransform t = m_nav.GetTopTransform().Inverse();
            G4ThreeVector global = t.TransformPoint(posMother);
            m_displayData->addPoint(geant4Message, TVector3(global[0], global[1], global[2]) * Unit::mm);
          }
        }
      } else {
        B2ERROR("Could not find name of intersecting volume");
      }
    } else {
      // in case of overlap with mother the life is so much simpler: just convert the point to global and save it.
      G4AffineTransform t = m_nav.GetTopTransform().Inverse();
      G4ThreeVector global = t.TransformPoint(posLocal);
      m_displayData->addPoint(geant4Message, TVector3(global[0], global[1], global[2]) * Unit::mm);
    }
  }
  m_nav.NewLevel(volume);
}

bool OverlapCheckerModule::checkVolume(G4VPhysicalVolume* volume, const std::string& path, int depth)
{
  // check if we exceeded maximum recursion depth
  if (m_maxDepth > 0 && depth >= m_maxDepth) return false;
  // add the volume to the navigation history
  m_nav.NewLevel(volume);
  // remember the path to the volume
  std::string volumePath = path + "/" + volume->GetName();
  bool result{false};
  // check if we have a prefix we have to match. if not prefix or prefix matches, check for overlaps
  if (m_prefix.empty() || volumePath.substr(0, m_prefix.size()) == m_prefix) {
    result = volume->CheckOverlaps(m_points, m_tolerance, true, m_maxErrors);
    if (result) {
      m_overlaps.push_back(volumePath);
    }
  }

  //Check the daughter volumes for overlaps
  G4LogicalVolume* logicalVolume = volume->GetLogicalVolume();
  for (int iDaughter = 0; iDaughter < logicalVolume->GetNoDaughters(); iDaughter++) {
    G4VPhysicalVolume* daughter = logicalVolume->GetDaughter(iDaughter);
    // check if we already checked this particular volume, if so skip it
    auto it = m_seen.insert(daughter);
    if (!it.second) continue;
    result |= checkVolume(daughter, volumePath, depth + 1);
  }
  m_nav.BackLevel();
  return result;
}
