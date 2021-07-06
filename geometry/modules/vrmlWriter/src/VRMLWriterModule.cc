/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <geometry/modules/vrmlWriter/VRMLWriterModule.h>
#include <geometry/GeometryManager.h>

#include "G4PhysicalVolumeStore.hh"
#include "G4LogicalVolumeStore.hh"
#include "G4SolidStore.hh"
#include "G4VPhysicalVolume.hh"
#include "G4LogicalVolume.hh"
#include "G4VSolid.hh"
#include "G4VisAttributes.hh"
#include "G4VisExtent.hh"
#include "G4Material.hh"
#include "G4ThreeVector.hh"
#include "G4RotationMatrix.hh"
#include "G4Transform3D.hh"
#include "G4VPVParameterisation.hh"
#include <G4Tubs.hh>
#include <G4Box.hh>
#include <G4Polyhedron.hh>
#include <G4DisplacedSolid.hh>

#include <iomanip>

using namespace Belle2;

REG_MODULE(VRMLWriter);

VRMLWriterModule::VRMLWriterModule()
{
  //Set module properties and the description
  setDescription("Write the detector geometry in a hierarchical VRML format.");

  //Parameter definition
  addParam("outputFile", m_Filename, "Output filename", std::string("belle2.wrl"));
}

void VRMLWriterModule::initialize()
{
  m_First = true;
}

void VRMLWriterModule::event()
{
  if (!m_First) return;
  m_First = false;
  G4VPhysicalVolume* topVol = geometry::GeometryManager::getInstance().getTopVolume();
  if (!topVol) {
    B2ERROR("No geometry found: add the Geometry module to the path before the VRMLWriter module.");
    return;
  }
  m_File.open(m_Filename, std::ios_base::trunc);
  writePreamble();

  G4PhysicalVolumeStore* pvStore = G4PhysicalVolumeStore::GetInstance();
  G4LogicalVolumeStore* lvStore = G4LogicalVolumeStore::GetInstance();
  G4SolidStore* solidStore = G4SolidStore::GetInstance();

  m_PVName = new std::vector<std::string>(pvStore->size(), "");
  m_LVName = new std::vector<std::string>(lvStore->size(), "");
  m_SolidName = new std::vector<std::string>(solidStore->size(), "");

  // Assign legal and unique names to each used physical volume, logical volume and solid
  for (G4VPhysicalVolume* physVol : *pvStore) {
    int pvIndex = std::find(pvStore->begin(), pvStore->end(), physVol) - pvStore->begin();
    if ((*m_PVName)[pvIndex].length() == 0)
      assignName(m_PVName, pvIndex, (*pvStore)[pvIndex]->GetName(), 0);
    G4LogicalVolume* logVol = physVol->GetLogicalVolume();
    int lvIndex = std::find(lvStore->begin(), lvStore->end(), logVol) - lvStore->begin();
    if ((*m_LVName)[lvIndex].length() == 0)
      assignName(m_LVName, lvIndex, (*lvStore)[lvIndex]->GetName(), 1);
    G4VSolid* solid = logVol->GetSolid();
    int solidIndex = std::find(solidStore->begin(), solidStore->end(), solid) - solidStore->begin();
    if ((*m_SolidName)[solidIndex].length() == 0)
      assignName(m_SolidName, solidIndex, (*solidStore)[solidIndex]->GetName(), 2);
  }

  // Write all explicitly-referenced solids as PROTOs (replicas are written later)
  // Use implicit prefix "solid_" to avoid name clashes with logical- and physical-volume names
  m_IsCylinder = new std::vector<bool>(solidStore->size(), false);
  for (unsigned int solidIndex = 0; solidIndex < solidStore->size(); ++solidIndex) {
    if ((*m_SolidName)[solidIndex].length() != 0) {
      if ((*solidStore)[solidIndex]->GetEntityType() == "G4Tubs") {
        auto* tube = (G4Tubs*)((*solidStore)[solidIndex]);
        (*m_IsCylinder)[solidIndex] = ((tube->GetInnerRadius() == 0.0) && (tube->GetDeltaPhiAngle() == 2.0 * M_PI));
      }
      describeSolid((*solidStore)[solidIndex], (*m_SolidName)[solidIndex], (*m_IsCylinder)[solidIndex]);
    }
  }

  // Recursively write all physical volumes (as DEFs) and logical volumes (as PROTOs).
  // Deepest volumes are written first; top volume is written last. Use implicit prefix
  // "lv_" for logical-volume names to avoid name clashes with solid and physical-volume names.
  m_PVIndex = new std::vector<std::vector<int> >(lvStore->size(), std::vector<int>());
  m_LVWritten = new std::vector<bool>(lvStore->size(), false);
  m_PVWritten = new std::vector<bool>(pvStore->size(), false);
  describePhysicalVolume(topVol);

  // Now tell VRML to draw the top physical volume (and, recursively, all daughters)
  int pvIndex = std::find(pvStore->begin(), pvStore->end(), topVol) - pvStore->begin();
  int lvIndex = std::find(lvStore->begin(), lvStore->end(), topVol->GetLogicalVolume()) - lvStore->begin();
  m_File << "# PhysicalVolume " << topVol->GetName() << std::endl <<
         "DEF " << (*m_PVName)[pvIndex] << " Transform {" << std::endl <<
         " children lv_" << (*m_LVName)[lvIndex] << "{}" << std::endl <<
         "}" << std::endl;
  m_File.close();
  B2INFO("VRML written to " << m_Filename);

  delete m_PVName;
  delete m_LVName;
  delete m_SolidName;
  delete m_IsCylinder;
  delete m_PVIndex;
  delete m_LVWritten;
  delete m_PVWritten;

}

void VRMLWriterModule::assignName(std::vector<std::string>* names, unsigned int index, const G4String& originalName, int select)
{
  G4SolidStore* solidStore = G4SolidStore::GetInstance();
  G4LogicalVolumeStore* lvStore = G4LogicalVolumeStore::GetInstance();
  G4PhysicalVolumeStore* pvStore = G4PhysicalVolumeStore::GetInstance();

  G4String name = originalName;
  if (name.length() == 0) { name = "anonymous"; }
  // Replace problematic characters with underscore (there may be more!)
  for (char c : " .,:;?'\"*+-=|^!/@#$\\%{}[]()<>") std::replace(name.begin(), name.end(), c, '_');
  // Avoid duplicate names for solids that will be written to VRML file
  for (int j = (int)index - 1; j >= 0; --j) {
    if ((*names)[j].length() == 0) continue;
    int match = 0;
    switch (select) {
      case 0:
        match = (*pvStore)[j]->GetName().compare((*pvStore)[index]->GetName()); break;
      case 1:
        match = (*lvStore)[j]->GetName().compare((*lvStore)[index]->GetName()); break;
      case 2:
        match = (*solidStore)[j]->GetName().compare((*solidStore)[index]->GetName()); break;
    }
    if (match == 0) {
      if (name.length() == (*names)[j].length()) {
        (*names)[j].append("_1");
      }
      int n = std::stoi((*names)[j].substr(name.length() + 1), nullptr);
      name.append("_");
      name.append(std::to_string(n + 1));
      break;
    }
  }
  (*names)[index] = name;
}

void VRMLWriterModule::describeSolid(G4VSolid* solid, const std::string& solidName, bool isCylinder)
{
  m_File << "# Solid " << solid->GetName() << " of type " << solid->GetEntityType() << std::endl;
  if (isCylinder) {
    auto* tube = (G4Tubs*)solid;
    // VRML cylinder is along y axis but G4Tubs is along z axis => rotate in logical volume
    m_File << "PROTO solid_" << solidName << " [ ] {" << std::endl <<
           " Cylinder {" << std::endl << std::setprecision(10) <<
           "  radius " << tube->GetOuterRadius() << std::endl <<
           "  height " << tube->GetZHalfLength() * 2.0 << std::endl <<
           " }" << std::endl <<
           "}" << std::endl;
  } else if (solid->GetEntityType() == "G4Box") {
    auto* box = (G4Box*)solid;
    m_File << "PROTO solid_" << solidName << " [ ] {" << std::endl <<
           " Box {" << std::endl << std::setprecision(10) <<
           "  size " << box->GetXHalfLength() * 2.0 << " " <<
           box->GetYHalfLength() * 2.0 << " " <<
           box->GetZHalfLength() * 2.0 << std::endl <<
           " }" << std::endl <<
           "}" << std::endl;
  } else if ((solid->GetEntityType() == "G4IntersectionSolid") ||
             (solid->GetEntityType() == "G4UnionSolid") ||
             (solid->GetEntityType() == "G4SubtractionSolid") ||
             (solid->GetEntityType() == "G4BooleanSolid")) {
    HepPolyhedron* polyhedron = getBooleanSolidPolyhedron(solid);
    auto* g4polyhedron = new G4Polyhedron(*polyhedron);
    writePolyhedron(g4polyhedron, solidName);
    delete polyhedron;
    delete g4polyhedron;
  } else {
    writePolyhedron(solid->GetPolyhedron(), solidName);
  }
}

void VRMLWriterModule::describeLogicalVolume(G4LogicalVolume* logVol, const std::string& lvName, const std::string& solidName,
                                             bool isCylinder)
{
  G4LogicalVolumeStore* lvStore = G4LogicalVolumeStore::GetInstance();
  int lvIndex = std::find(lvStore->begin(), lvStore->end(), logVol) - lvStore->begin();
  if ((*m_LVWritten)[lvIndex]) return; // at most one PROTO in VRML for each logical volume
  (*m_LVWritten)[lvIndex] = true;
  G4Color color(0.0, 1.0, 0.0, 0.5); // default is semi-transparent green
  if ((lvName.compare(0, 23, "eclBarrelCrystalLogical") == 0) ||
      (lvName.compare(0, 20, "eclFwdCrystalLogical") == 0) ||
      (lvName.compare(0, 20, "eclBwdCrystalLogical") == 0)) {
    color = G4Color(1.0, 0.25, 0.0, 0.7); // orange since ECL crystals have no G4VisAttribute :(
  }
  std::string visible = "";
  G4String materialName = logVol->GetMaterial()->GetName();
  // Hide containers that have vacuum, air or gas
  if (materialName == "Vacuum") visible = "#";
  if (materialName == "G4_AIR") visible = "#";
  if (materialName == "CDCGas") visible = "#";
  if (materialName == "ColdAir") visible = "#";
  if (materialName == "STR-DryAir") visible = "#";
  if (materialName == "TOPAir") visible = "#";
  if (materialName == "TOPVacuum") visible = "#";
  const G4VisAttributes* visAttr = logVol->GetVisAttributes();
  if (visAttr) {
    color = const_cast<G4Color&>(logVol->GetVisAttributes()->GetColor());
    if (!(visAttr->IsVisible())) visible = "#";
  } else {
    visible = "#";
  }
  bool hasDaughters = (visible.length() == 0) || ((*m_PVIndex)[lvIndex].size() > 0);
  if (logVol->GetSensitiveDetector() != nullptr) visible = "";
  m_File << "# LogicalVolume " << logVol->GetName() << " containing " << materialName << std::endl <<
         "PROTO lv_" << lvName << " [ ] {" << std::endl <<
         " Group {" << std::endl <<
         "  " << (hasDaughters ? "" : "#") << "children [" << std::endl;
  if (isCylinder) {
    // VRML cylinder is along y axis but G4Tubs is along z axis => rotate here
    m_File << "   " << visible << "Transform {" << std::endl <<
           "   " << visible << " rotation 1 0 0 " << M_PI_2 << std::endl <<
           "   " << visible << " children [" << std::endl;
    visible.append("  ");
  }
  m_File << "   " << visible << "Shape {" << std::endl <<
         "   " << visible << " appearance Appearance {" << std::endl <<
         "   " << visible << "  material Material {" << std::endl <<
         "   " << visible << "   diffuseColor " << color.GetRed() << " " <<
         color.GetGreen() << " " <<
         color.GetBlue() << std::endl <<
         "   " << visible << "   " << (color.GetAlpha() == 1.0 ? "#" : "") <<
         "transparency " << 1.0 - color.GetAlpha() << std::endl <<
         "   " << visible << "  }" << std::endl <<
         "   " << visible << " }" << std::endl <<
         "   " << visible << " geometry solid_" << solidName << "{}" << std::endl <<
         "   " << visible << "}" << std::endl;
  if (isCylinder) {
    visible.resize(visible.length() - 2);
    m_File << "   " << visible << " ]" << std::endl <<
           "   " << visible << "}" << std::endl;
  }
  G4PhysicalVolumeStore* pvStore = G4PhysicalVolumeStore::GetInstance();
  for (int daughter : (*m_PVIndex)[lvIndex]) {
    G4VPhysicalVolume* physVol = (*pvStore)[daughter];
    std::string pvNameDaughter = (*m_PVName)[daughter];
    int lvDaughter = std::find(lvStore->begin(), lvStore->end(), physVol->GetLogicalVolume()) - lvStore->begin();
    std::string lvNameDaughter = (*m_LVName)[lvDaughter];
    if (physVol->IsReplicated()) {
      G4VSolid* solid = logVol->GetSolid();
      EAxis axis;
      G4int nReplicas;
      G4double width;
      G4double offset;
      G4bool consuming;
      physVol->GetReplicationData(axis, nReplicas, width, offset, consuming);
      G4VPVParameterisation* physParameterisation = physVol->GetParameterisation();
      if (physParameterisation) { // parameterised volume
        for (int replica = 0; replica < nReplicas; ++replica) {
          std::string pvNameReplica = pvNameDaughter;
          std::string lvNameReplica = lvNameDaughter;
          pvNameReplica.append("_");
          pvNameReplica.append(std::to_string(replica));
          physVol->SetCopyNo(replica);
          physParameterisation->ComputeTransformation(replica, physVol);
          G4VSolid* solidReplica = physParameterisation->ComputeSolid(replica, physVol);
          if (solidReplica != solid) { // not sure if this works ...
            lvNameReplica.append("_");
            lvNameReplica.append(std::to_string(replica));
          }
          writePhysicalVolume(physVol, pvNameReplica, lvNameReplica, false);
        }
      } else { // plain replicated volume
        G4ThreeVector originalTranslation = physVol->GetTranslation();
        G4RotationMatrix* originalRotation = physVol->GetRotation();
        for (int replica = 0; replica < nReplicas; ++replica) {
          G4ThreeVector translation; // No translation
          G4RotationMatrix rotation; // No rotation
          std::string pvNameReplica = pvNameDaughter;
          std::string lvNameReplica = lvNameDaughter;
          pvNameReplica.append("_");
          pvNameReplica.append(std::to_string(replica));
          physVol->SetCopyNo(replica);
          switch (axis) {
            default:
            case kXAxis:
              translation.setX(width * (replica - 0.5 * (nReplicas - 1)));
              physVol->SetTranslation(translation);
              break;
            case kYAxis:
              translation.setY(width * (replica - 0.5 * (nReplicas - 1)));
              physVol->SetTranslation(translation);
              break;
            case kZAxis:
              translation.setZ(width * (replica - 0.5 * (nReplicas - 1)));
              physVol->SetTranslation(translation);
              break;
            case kRho:
              if (solid->GetEntityType() == "G4Tubs") {
                lvNameReplica.append("_");
                lvNameReplica.append(std::to_string(replica));
              } else {
                B2WARNING("Built-in volumes replicated along radius for " << solid->GetEntityType() <<
                          " (solid " << solid->GetName() << ") are not visualisable.");
              }
              break;
            case kPhi:
              physVol->SetRotation(&(rotation.rotateZ(-(offset + (replica + 0.5) * width))));
              break;
          }
          writePhysicalVolume(physVol, pvNameReplica, lvNameReplica, false);
        }
        // Restore originals...
        physVol->SetTranslation(originalTranslation);
        physVol->SetRotation(originalRotation);
      }
    } else {
      writePhysicalVolume(physVol, pvNameDaughter, lvNameDaughter, (*m_PVWritten)[daughter]);
    }
    (*m_PVWritten)[daughter] = true;
  }
  G4VisExtent extent = logVol->GetSolid()->GetExtent();
  double xMin = extent.GetXmin();
  double xMax = extent.GetXmax();
  double yMin = extent.GetYmin();
  double yMax = extent.GetYmax();
  double zMin = extent.GetZmin();
  double zMax = extent.GetZmax();
  double xCenter = 0.5 * (xMin + xMax);
  double yCenter = 0.5 * (yMin + yMax);
  double zCenter = 0.5 * (zMin + zMax);
  bool atOrigin = (std::fabs(xCenter) + std::fabs(yCenter) + std::fabs(zCenter) > 1.0e-12);
  m_File << "  " << (hasDaughters ? "" : "#") << "]" << std::endl <<
         "  " << (atOrigin ? "" : "#") <<
         "bboxCenter " << xCenter << " " << yCenter << " " << zCenter << std::endl <<
         "  bboxSize " << xMax - xMin << " " <<
         (isCylinder ? zMax - zMin : yMax - yMin) << " " <<
         (isCylinder ? yMax - yMin : zMax - zMin) << std::endl <<
         " }" << std::endl <<
         "} # end of " << lvName << std::endl;
}

void VRMLWriterModule::describePhysicalVolume(G4VPhysicalVolume* physVol)
{
  G4LogicalVolume* logVol = physVol->GetLogicalVolume();
  G4VSolid* solid = logVol->GetSolid();
  G4LogicalVolumeStore* lvStore = G4LogicalVolumeStore::GetInstance();
  int lvIndex = std::find(lvStore->begin(), lvStore->end(), logVol) - lvStore->begin();
  std::string lvName = (*m_LVName)[lvIndex];
  if (physVol->IsReplicated()) {
    G4SolidStore* solidStore = G4SolidStore::GetInstance();
    int solidIndex = std::find(solidStore->begin(), solidStore->end(), solid) - solidStore->begin();
    std::string solidName = (*m_SolidName)[solidIndex];
    EAxis axis;
    G4int nReplicas;
    G4double width;
    G4double offset;
    G4bool consuming;
    physVol->GetReplicationData(axis, nReplicas, width, offset, consuming);
    G4VPVParameterisation* physParameterisation = physVol->GetParameterisation();
    if (physParameterisation) { // user-parameterised volume
      for (int replica = 0; replica < nReplicas; ++replica) {
        std::string lvNameReplica = lvName;
        G4VSolid* solidReplica = physParameterisation->ComputeSolid(replica, physVol);
        if (solidReplica != solid) { // not sure if this works ...
          solidReplica->ComputeDimensions(physParameterisation, replica, physVol);
          std::string solidNameReplica = solidName;
          solidNameReplica.append("_");
          solidNameReplica.append(std::to_string(replica));
          lvNameReplica.append("_");
          lvNameReplica.append(std::to_string(replica));
          describeSolid(solidReplica, solidNameReplica, (*m_IsCylinder)[solidIndex]);
          describeLogicalVolume(logVol, lvNameReplica, solidNameReplica, (*m_IsCylinder)[solidIndex]);
        }
        descendAndDescribe(physVol, lvNameReplica, replica);
      }
    } else { // plain replicated volume - "divisions"
      for (int replica = 0; replica < nReplicas; ++replica) {
        if (axis == kRho) {
          if (solid->GetEntityType() == "G4Tubs") {
            double originalRMin = ((G4Tubs*)solid)->GetInnerRadius();
            double originalRMax = ((G4Tubs*)solid)->GetOuterRadius();
            ((G4Tubs*)solid)->SetInnerRadius(offset + width * replica);
            ((G4Tubs*)solid)->SetOuterRadius(offset + width * (replica + 1));
            std::string solidNameReplica = solidName;
            solidNameReplica.append("_");
            solidNameReplica.append(std::to_string(replica));
            std::string lvNameReplica = lvName;
            lvNameReplica.append("_");
            lvNameReplica.append(std::to_string(replica));
            describeSolid(solid, solidNameReplica, (*m_IsCylinder)[solidIndex]);
            ((G4Tubs*)solid)->SetInnerRadius(originalRMin);
            ((G4Tubs*)solid)->SetOuterRadius(originalRMax);
            describeLogicalVolume(logVol, lvNameReplica, solidNameReplica, (*m_IsCylinder)[solidIndex]);
            descendAndDescribe(physVol, lvNameReplica, replica);
          } else {
            B2WARNING("Built-in volumes replicated along radius for " << solid->GetEntityType() <<
                      " (solid " << solid->GetName() << ") are not visualisable.");
          }
        } else {
          descendAndDescribe(physVol, lvName, replica);
        }
      }
    }
  } else { // non-replicated volume
    descendAndDescribe(physVol, lvName, -1);
  }
}

void VRMLWriterModule::descendAndDescribe(G4VPhysicalVolume* physVol, const std::string& lvName, int replica)
{
  G4PhysicalVolumeStore* pvStore = G4PhysicalVolumeStore::GetInstance();
  G4LogicalVolumeStore* lvStore = G4LogicalVolumeStore::GetInstance();
  G4SolidStore* solidStore = G4SolidStore::GetInstance();

  int pvIndex = std::find(pvStore->begin(), pvStore->end(), physVol) - pvStore->begin();
  if ((*m_PVWritten)[pvIndex]) return;

  // Descend to the leaves of the tree
  G4LogicalVolume* logVol = physVol->GetLogicalVolume();
  int lvIndex = std::find(lvStore->begin(), lvStore->end(), logVol) - lvStore->begin();
  for (size_t daughter = 0; daughter < logVol->GetNoDaughters(); ++daughter) {
    G4VPhysicalVolume* dPhysVol = logVol->GetDaughter(daughter);
    (*m_PVIndex)[lvIndex].push_back(std::find(pvStore->begin(), pvStore->end(), dPhysVol) - pvStore->begin());
    describePhysicalVolume(dPhysVol);
  }
  // Write out the physical volume and its corresponding logical volume as we ascend the recursive tree
  int solidIndex = std::find(solidStore->begin(), solidStore->end(), logVol->GetSolid()) - solidStore->begin();
  if (replica <= 0) describeLogicalVolume(logVol, lvName, (*m_SolidName)[solidIndex], (*m_IsCylinder)[solidIndex]);
}

void VRMLWriterModule::writePreamble()
{
  m_File << "#VRML V2.0 utf8" << std::endl << std::endl <<
         "WorldInfo {" << std::endl <<
         "  info [ \"(c) The Belle II Collaboration\" ]" << std::endl <<
         "  title \"The Belle II Detector\"" << std::endl <<
         "}" << std::endl <<
         "Viewpoint {" << std::endl <<
         " position 0 0 15000" << std::endl <<
         " description \"front\"" << std::endl <<
         "}" << std::endl <<
         "Viewpoint {" << std::endl <<
         " position 0 0 -15000" << std::endl <<
         " orientation 0 1 0 3.141593" << std::endl <<
         " description \"back\"" << std::endl <<
         "}" << std::endl <<
         "Viewpoint {" << std::endl <<
         " position 0 15000 0" << std::endl <<
         " orientation 1 0 0 -1.570796" << std::endl <<
         " description \"top\"" << std::endl <<
         "}" << std::endl <<
         "Viewpoint {" << std::endl <<
         " position 0 -15000 0" << std::endl <<
         " orientation 1 0 0 1.570796" << std::endl <<
         " description \"bottom\"" << std::endl <<
         "}" << std::endl <<
         "Viewpoint {" << std::endl <<
         " position 15000 0 0" << std::endl <<
         " orientation 0 1 0 1.570796" << std::endl <<
         " description \"right\"" << std::endl <<
         "}" << std::endl <<
         "Viewpoint {" << std::endl <<
         " position -15000 0 0" << std::endl <<
         " orientation 0 1 0 -1.570796" << std::endl <<
         " description \"left\"" << std::endl <<
         "}" << std::endl;
}

void VRMLWriterModule::writePolyhedron(const G4Polyhedron* polyhedron, const std::string& name)
{
  if (polyhedron) {
    m_File << "PROTO solid_" << name << " [ ] {" << std::endl <<
           " IndexedFaceSet {" << std::endl <<
           "  coord Coordinate {" << std::endl <<
           "   point [" << std::endl;
    for (int j = 1; j <= polyhedron->GetNoVertices(); ++j) {
      m_File << "    " << polyhedron->GetVertex(j).x() << " " <<
             polyhedron->GetVertex(j).y() << " " <<
             polyhedron->GetVertex(j).z() << std::endl;
    }
    m_File << "   ]" << std::endl <<
           "  }" << std::endl <<
           "  coordIndex [" << std::endl;
    for (int j = 0; j < polyhedron->GetNoFacets(); ++j) {
      G4bool notLastEdge = true;
      G4int ndx = -1, edgeFlag = 1;
      m_File << "   ";
      do {
        notLastEdge = polyhedron->GetNextVertexIndex(ndx, edgeFlag);
        m_File << " " << ndx - 1;
      } while (notLastEdge);
      m_File << " -1" << std::endl;
    }
    m_File << "  ]" << std::endl <<
           " }" << std::endl <<
           "}" << std::endl;
  } else {
    B2INFO("Polyhedron representation of solid " << name << " cannot be created");
  }
}

void VRMLWriterModule::writePhysicalVolume(const G4VPhysicalVolume* physVol, const std::string& pvName, const std::string& lvName,
                                           bool written)
{
  if (written) {
    m_File << "   USE " << pvName << std::endl; // this never happens, as it turns out
  } else {
    m_File << "   # PhysicalVolume " << physVol->GetName() << " copy " << physVol->GetCopyNo() << std::endl <<
           "   DEF " << pvName << " Transform {" << std::endl; // DEF provides object name when importing VRML into Unity
    G4RotationMatrix* rot = physVol->GetObjectRotation();
    G4ThreeVector move = physVol->GetObjectTranslation();
    double yaw = std::atan2(rot->yx(), rot->xx());
    double pitch = -std::asin(rot->zx());
    double roll = std::atan2(rot->zy(), rot->zz());
    std::cout << physVol->GetName() << " translation: " << move.x() << "," << move.y() << "," << move.z() << std::endl;
    std::cout << physVol->GetName() << " rotation: " << yaw * 180.0 / M_PI << "," << pitch * 180.0 / M_PI << "," << roll * 180.0 / M_PI
              << std::endl;
    if (move.mag2() != 0.0) {
      m_File << "    translation " << std::setprecision(10) <<
             move.x() << " " << move.y() << " " << move.z() << std::endl;
    }
    if (!(rot->isIdentity())) {
      double trace = std::max(-1.0, std::min(3.0, rot->xx() + rot->yy() + rot->zz()));
      double angle = std::acos(0.5 * (trace - 1.0));
      G4ThreeVector rotA(rot->zy() - rot->yz(), rot->xz() - rot->zx(), rot->yx() - rot->xy());
      if ((rotA.x() == 0.0) && (rotA.y() == 0.0) && (rotA.z() == 0.0)) {
        // The assignment along x, y or z axis in Hep3Vector::axis() is wrong. The proper axis of
        // rotation is given by kernel(r - I), which is the cross product of two non-proportional rows
        if (rot->xx() > 0.0) {
          rotA.setX((rot->yy() - 1.0) * (rot->zz() - 1.0) -  rot->yz()        *  rot->zy());
          rotA.setY(rot->yz()        *  rot->zx()        -  rot->yx()        * (rot->zz() - 1.0));
          rotA.setZ(rot->yx()        *  rot->zy()        -  rot->zx()        * (rot->yy() - 1.0));
        } else if (rot->yy() > 0.0) {
          rotA.setX(rot->xy()        * (rot->zz() - 1.0) -  rot->xz()        *  rot->zy());
          rotA.setY(rot->xz()        *  rot->zx()        - (rot->xx() - 1.0) * (rot->zz() - 1.0));
          rotA.setZ((rot->xx() - 1.0) *  rot->zy()        -  rot->xy()        *  rot->zx());
        } else {
          rotA.setX(rot->xy()        *  rot->yz()        -  rot->xz()        * (rot->yy() - 1.0));
          rotA.setY(rot->xz()        *  rot->yx()        -  rot->yz()        * (rot->xx() - 1.0));
          rotA.setZ((rot->xx() - 1.0) * (rot->yy() - 1.0) -  rot->xy()        *  rot->yx());
        }
      }
      rotA.setMag(1.0);
      m_File << "    rotation " << std::setprecision(10) <<
             rotA.x() << " " << rotA.y() << " " << rotA.z() << " " << angle << std::endl;
    }
    m_File << "    children lv_" << lvName << "{}" << std::endl <<
           "   }" << std::endl;
  }
}

// The code in GEANT4 geometry/solids/Boolean/src/G4BooleanSolid.cc is buggy so avoid it.
// Recursively combine the polyhedrons of two solids to make a resulting polyhedron.
HepPolyhedron* VRMLWriterModule::getBooleanSolidPolyhedron(G4VSolid* solid)
{
  G4VSolid* solidA = solid->GetConstituentSolid(0);
  G4VSolid* solidB = solid->GetConstituentSolid(1);
  HepPolyhedron* polyhedronA = nullptr;
  if ((solidA->GetEntityType() == "G4IntersectionSolid") ||
      (solidA->GetEntityType() == "G4UnionSolid") ||
      (solidA->GetEntityType() == "G4SubtractionSolid") ||
      (solidA->GetEntityType() == "G4BooleanSolid")) {
    polyhedronA = getBooleanSolidPolyhedron(solidA);
  } else {
    polyhedronA = new HepPolyhedron(*(solidA->GetPolyhedron()));
  }
  HepPolyhedron* polyhedronB = nullptr;
  G4VSolid* solidB2 = solidB;
  if (solidB->GetEntityType() == "G4DisplacedSolid") {
    solidB2 = ((G4DisplacedSolid*)solidB)->GetConstituentMovedSolid();
  }
  if ((solidB2->GetEntityType() == "G4IntersectionSolid") ||
      (solidB2->GetEntityType() == "G4UnionSolid") ||
      (solidB2->GetEntityType() == "G4SubtractionSolid") ||
      (solidB2->GetEntityType() == "G4BooleanSolid")) {
    polyhedronB = getBooleanSolidPolyhedron(solidB2);
    if (solidB != solidB2) { // was solidB a G4DisplacedSolid?
      polyhedronB->Transform(G4Transform3D(
                               ((G4DisplacedSolid*)solidB)->GetObjectRotation(),
                               ((G4DisplacedSolid*)solidB)->GetObjectTranslation()));
    }
  } else {
    polyhedronB = new HepPolyhedron(*(solidB->GetPolyhedron()));
  }
  auto* result = new HepPolyhedron();
  if (solid->GetEntityType() == "G4UnionSolid") {
    *result = polyhedronA->add(*polyhedronB);
  } else if (solid->GetEntityType() == "G4SubtractionSolid") {
    *result = polyhedronA->subtract(*polyhedronB);
  } else if (solid->GetEntityType() == "G4IntersectionSolid") {
    *result = polyhedronA->intersect(*polyhedronB);
  } else {
    B2WARNING("getBooleanSolidPolyhedron(): Unrecognized boolean solid " << solid->GetName() <<
              " of type " << solid->GetEntityType());
  }
  delete polyhedronA;
  delete polyhedronB;
  return result;
}
