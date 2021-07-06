/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <geometry/modules/fbxWriter/FBXWriterModule.h>
#include <geometry/GeometryManager.h>

#include "G4PhysicalVolumeStore.hh"
#include "G4LogicalVolumeStore.hh"
#include "G4SolidStore.hh"
#include "G4VPhysicalVolume.hh"
#include "G4LogicalVolume.hh"
#include "G4VSolid.hh"
#include "G4DisplacedSolid.hh"
#include "G4Material.hh"
#include "G4VisAttributes.hh"
#include "G4ThreeVector.hh"
#include "G4RotationMatrix.hh"
#include "G4Transform3D.hh"
#include "G4VPVParameterisation.hh"
#include <G4Tubs.hh>
#include <G4Polyhedron.hh>

#include <iomanip>

using namespace Belle2;

REG_MODULE(FBXWriter);

FBXWriterModule::FBXWriterModule()
{
  //Set module properties and the description
  setDescription("Write the detector geometry in a (semi-)hierarchical FBX format.");

  //Parameter definition
  addParam("usePrototypes", m_UsePrototypes, "Use LogVol and PhysVol prototypes", false);

  //Parameter definition
  addParam("outputFile", m_Filename, "Output filename", std::string("belle2.fbx"));
}

void FBXWriterModule::initialize()
{
  m_First = true;
}

void FBXWriterModule::event()
{
  if (!m_First) return;
  m_First = false;
  G4VPhysicalVolume* topVol = geometry::GeometryManager::getInstance().getTopVolume();
  if (!topVol) {
    B2ERROR("No geometry found: add the Geometry module to the path before the FBXWriter module.");
    return;
  }

  G4PhysicalVolumeStore* pvStore = G4PhysicalVolumeStore::GetInstance();
  G4LogicalVolumeStore* lvStore = G4LogicalVolumeStore::GetInstance();
  G4SolidStore* solidStore = G4SolidStore::GetInstance();

  // Assign legal and unique names to each used physical volume, logical volume and solid
  m_PVName = new std::vector<std::string>(pvStore->size(), "");
  m_LVName = new std::vector<std::string>(lvStore->size(), "");
  m_SolidName = new std::vector<std::string>(solidStore->size(), "");
  for (G4VPhysicalVolume* physVol : *pvStore) {
    int pvIndex = std::find(pvStore->begin(), pvStore->end(), physVol) - pvStore->begin();
    if ((*m_PVName)[pvIndex].length() == 0) {
      assignName(m_PVName, pvIndex, physVol->GetName(), 0);
      G4LogicalVolume* logVol = physVol->GetLogicalVolume();
      int lvIndex = std::find(lvStore->begin(), lvStore->end(), logVol) - lvStore->begin();
      if ((*m_LVName)[lvIndex].length() == 0) {
        assignName(m_LVName, lvIndex, logVol->GetName(), 1);
        G4VSolid* solid = logVol->GetSolid();
        int solidIndex = std::find(solidStore->begin(), solidStore->end(), solid) - solidStore->begin();
        if ((*m_SolidName)[solidIndex].length() == 0) {
          assignName(m_SolidName, solidIndex, solid->GetName(), 2);
        }
      }
    }
  }

  // Count the number of references to each physical volume and logical volume and solid
  // so that these values can be placed in the FBX file's Definitions{} section.
  m_PVCount = new std::vector<unsigned int>(pvStore->size(), 0);
  m_LVCount = new std::vector<unsigned int>(lvStore->size(), 0);
  m_SolidCount = new std::vector<unsigned int>(solidStore->size(), 0);
  m_PVReplicas = new std::vector<unsigned int>(pvStore->size(), 0);
  m_LVReplicas = new std::vector<unsigned int>(lvStore->size(), 0);
  m_SolidReplicas = new std::vector<unsigned int>(solidStore->size(), 0);
  m_LVUnique = new std::vector<bool>(lvStore->size(), true);
  countEntities(topVol);
  unsigned int geometryCount = 0;
  unsigned int materialCount = 0;
  unsigned int modelCount = 0;
  for (unsigned int pvIndex = 0; pvIndex < pvStore->size(); ++pvIndex) {
    if ((*m_PVName)[pvIndex].length() > 0) {
      modelCount += (*m_PVCount)[pvIndex] + (*m_PVReplicas)[pvIndex];
    }
  }
  for (unsigned int lvIndex = 0; lvIndex < lvStore->size(); ++lvIndex) {
    if ((*m_LVName)[lvIndex].length() > 0) {
      if (!(*m_LVUnique)[lvIndex]) {
        modelCount += (*m_LVCount)[lvIndex] + (*m_LVReplicas)[lvIndex];
      }
      materialCount++;
    }
  }
  for (unsigned int solidIndex = 0; solidIndex < solidStore->size(); ++solidIndex) {
    if ((*m_SolidName)[solidIndex].length() > 0) {
      geometryCount += (*m_SolidCount)[solidIndex] + (*m_SolidReplicas)[solidIndex] + 1;
    }
  }

  m_File.open(m_Filename, std::ios_base::trunc);
  writePreamble(modelCount, materialCount, geometryCount);

  // Write all solids as Geometry nodes (replicas are written later).
  // Write all logical volumes as Material nodes (color information).
  // Write all physical and logical volumes as Model nodes (with replica-solids treated here).
  m_PVID = new std::vector<unsigned long long>(pvStore->size(), 0x0000010000000000LL);
  m_LVID = new std::vector<unsigned long long>(lvStore->size(), 0x000000C000000000LL);
  m_SolidID = new std::vector<unsigned long long>(solidStore->size(), 0x0000008000000000LL);
  m_MatID = new std::vector<unsigned long long>(lvStore->size(), 0x0000004000000000LL);
  m_Visible = new std::vector<bool>(lvStore->size(), false);
  m_File << "Objects:  {" << std::endl;
  for (unsigned int solidIndex = 0; solidIndex < solidStore->size(); ++solidIndex) {
    (*m_SolidID)[solidIndex] += 0x0000000001000000LL * solidIndex;
    if ((*m_SolidName)[solidIndex].length() > 0) {
      for (unsigned int solidCount = 0; solidCount <= (*m_SolidCount)[solidIndex]; ++solidCount) { // note lower and upper limits!
        writeGeometryNode((*solidStore)[solidIndex], (*m_SolidName)[solidIndex], (*m_SolidID)[solidIndex] + solidCount);
      }
    }
  }
  for (unsigned int lvIndex = 0; lvIndex < lvStore->size(); ++lvIndex) {
    (*m_MatID)[lvIndex] += 0x0000000001000000LL * lvIndex;
    (*m_LVID)[lvIndex] += 0x0000000001000000LL * lvIndex;
    if ((*m_LVName)[lvIndex].length() > 0) {
      if (!(*m_LVUnique)[lvIndex]) writeMaterialNode(lvIndex, (*m_LVName)[lvIndex]);
    }
  }
  for (unsigned int pvIndex = 0; pvIndex < pvStore->size(); ++pvIndex) {
    (*m_PVID)[pvIndex] += 0x0000000001000000LL * pvIndex;
  }
  m_PVCount->assign(pvStore->size(), 0);
  m_LVCount->assign(lvStore->size(), 0);
  m_SolidCount->assign(solidStore->size(), 0);
  addModels(topVol, 0);
  m_File << "}" << std::endl << std::endl;

  // Recursively write the connections among the solid and logical/physical volume elements
  m_PVCount->assign(pvStore->size(), 0);
  m_LVCount->assign(lvStore->size(), 0);
  m_SolidCount->assign(solidStore->size(), 0);
  m_File << "Connections:  {" << std::endl;
  addConnections(topVol, 0);
  int pvIndex = std::find(pvStore->begin(), pvStore->end(), topVol) - pvStore->begin();
  m_File << "\t; Physical volume Model::" << (*m_PVName)[pvIndex] << " to Model::RootNode" << std::endl <<
         "\tC: \"OO\"," << (*m_PVID)[pvIndex] << ",0" << std::endl << std::endl <<
         "}" << std::endl << std::endl;

  m_File << "Takes:  {" << std::endl <<
         "\tCurrent: \"\"" << std::endl <<
         "}" << std::endl;

  m_File.close();
  B2INFO("FBX written to " << m_Filename);

  delete m_PVName;
  delete m_LVName;
  delete m_SolidName;
  delete m_PVID;
  delete m_LVID;
  delete m_MatID;
  delete m_SolidID;
  delete m_PVCount;
  delete m_LVCount;
  delete m_SolidCount;
  delete m_Visible;

}

void FBXWriterModule::assignName(std::vector<std::string>* names, unsigned int index, const G4String& originalName, int select)
{
  G4PhysicalVolumeStore* pvStore = G4PhysicalVolumeStore::GetInstance();
  G4LogicalVolumeStore* lvStore = G4LogicalVolumeStore::GetInstance();
  G4SolidStore* solidStore = G4SolidStore::GetInstance();

  G4String name = originalName;
  if (name.length() == 0) { name = "anonymous"; }
  // Replace problematic characters with underscore
  for (char c : " .,:;?'\"*+-=|^!/@#$\\%{}[]()<>") std::replace(name.begin(), name.end(), c, '_');
  // Avoid duplicate names for entities that will be written to FBX file
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

void FBXWriterModule::writeGeometryNode(G4VSolid* solid, const std::string& solidName, unsigned long long solidID)
{
  if ((solid->GetEntityType() == "G4IntersectionSolid") ||
      (solid->GetEntityType() == "G4UnionSolid") ||
      (solid->GetEntityType() == "G4SubtractionSolid") ||
      (solid->GetEntityType() == "G4BooleanSolid")) {
    HepPolyhedron* polyhedron = getBooleanSolidPolyhedron(solid);
    auto* g4polyhedron = new G4Polyhedron(*polyhedron);
    writePolyhedron(solid, g4polyhedron, solidName, solidID);
    delete polyhedron;
    delete g4polyhedron;
  } else {
    writePolyhedron(solid, solid->GetPolyhedron(), solidName, solidID);
  }
}

void FBXWriterModule::writeMaterialNode(int lvIndex, const std::string& matName)
{
  G4LogicalVolumeStore* lvStore = G4LogicalVolumeStore::GetInstance();
  G4LogicalVolume* logVol = (*lvStore)[lvIndex];
  unsigned long long matID = (*m_MatID)[lvIndex];
  G4Color color(0.0, 1.0, 0.0, 0.5); // default is semi-transparent green
  if ((matName.compare(0, 23, "eclBarrelCrystalLogical") == 0) ||
      (matName.compare(0, 20, "eclFwdCrystalLogical") == 0) ||
      (matName.compare(0, 20, "eclBwdCrystalLogical") == 0) ||
      (matName.compare(0, 24, "eclBarrelCrystalPhysical") == 0) ||
      (matName.compare(0, 21, "eclFwdCrystalPhysical") == 0) ||
      (matName.compare(0, 21, "eclBwdCrystalPhysical") == 0)) {
    color = G4Color(1.0, 0.25, 0.0, 0.7); // orange since ECL crystals have no G4VisAttribute :(
  }
  bool visible = true;
  G4String materialName = logVol->GetMaterial()->GetName();
  // Hide containers that have vacuum, air or gas
  if (materialName == "Vacuum") visible = false;
  if (materialName == "G4_AIR") visible = false;
  if (materialName == "CDCGas") visible = false;
  if (materialName == "ColdAir") visible = false;
  if (materialName == "STR-DryAir") visible = false;
  if (materialName == "TOPAir") visible = false;
  if (materialName == "TOPVacuum") visible = false;
  const G4VisAttributes* visAttr = logVol->GetVisAttributes();
  if (visAttr) {
    color = const_cast<G4Color&>(logVol->GetVisAttributes()->GetColor());
    if (!(visAttr->IsVisible())) visible = false;
  } else {
    visible = false;
  }
  if (logVol->GetSensitiveDetector() != nullptr) visible = "";
  (*m_Visible)[lvIndex] = visible;
  m_File << "\t; Color for LogVol " << logVol->GetName() << std::endl <<
         "\tMaterial: " << matID << ", \"Material::" << matName << R"(", "" {)" << std::endl <<
         "\t\tVersion: 102" << std::endl <<
         "\t\tProperties70:  {" << std::endl <<
         "\t\t\tP: \"ShadingModel\", \"KString\", \"\", \"\", \"phong\"" << std::endl <<
         "\t\t\tP: \"DiffuseColor\", \"RGBColor\", \"Color\", \"A\"," <<
         color.GetRed() << "," << color.GetGreen() << "," << color.GetBlue() << std::endl <<
         "\t\t\tP: \"TransparentColor\", \"RGBColor\", \"Color\", \"A\",1,1,1" << std::endl <<
         "\t\t\tP: \"TransparencyFactor\", \"double\", \"Number\", \"\"," << (visible ? 1.0 - color.GetAlpha() : 1) << std::endl <<
         "\t\t}" << std::endl <<
         "\t}" << std::endl;
}

void FBXWriterModule::writeLVModelNode(G4LogicalVolume* logVol, const std::string& lvName, unsigned long long lvID)
{
  m_File << "\t; LogVol " << logVol->GetName() << " with solid " << logVol->GetSolid()->GetName() << std::endl <<
         "\tModel: " << lvID << ", \"Model::lv_" << lvName << R"(", "Null" {)" << std::endl <<
         "\t\tVersion: 232" << std::endl <<
         "\t\tProperties70:  {" << std::endl <<
         "\t\t}" << std::endl <<
         "\t\tShading: T" << std::endl <<
         "\t\tCulling: \"CullingOff\"" << std::endl <<
         "\t}" << std::endl;
}

void FBXWriterModule::addModels(G4VPhysicalVolume* physVol, int replica)
{
  // Descend to the leaves of the tree
  G4LogicalVolume* logVol = physVol->GetLogicalVolume();
  for (size_t daughter = 0; daughter < logVol->GetNoDaughters(); ++daughter) {
    G4VPhysicalVolume* physVolDaughter = logVol->GetDaughter(daughter);
    for (int j = 0; j < physVolDaughter->GetMultiplicity(); ++j) {
      addModels(physVolDaughter, j);
    }
  }

  // Write the physical- and logical-volume models as we ascend the recursive tree
  G4PhysicalVolumeStore* pvStore = G4PhysicalVolumeStore::GetInstance();
  G4LogicalVolumeStore* lvStore = G4LogicalVolumeStore::GetInstance();
  int pvIndex = std::find(pvStore->begin(), pvStore->end(), physVol) - pvStore->begin();
  unsigned long long pvID = (*m_PVID)[pvIndex];
  unsigned int pvCount = (*m_PVCount)[pvIndex];
  std::string pvName = (*m_PVName)[pvIndex];
  int lvIndex = std::find(lvStore->begin(), lvStore->end(), logVol) - lvStore->begin();
  unsigned long long lvID = (*m_LVID)[lvIndex];
  unsigned int lvCount = (*m_LVCount)[lvIndex];
  std::string lvName = (*m_LVName)[lvIndex];
  if ((*m_LVUnique)[lvIndex]) writeMaterialNode(lvIndex, (*m_PVName)[pvIndex]);
  if (physVol->IsReplicated()) {
    G4VSolid* solid = logVol->GetSolid();
    G4SolidStore* solidStore = G4SolidStore::GetInstance();
    int solidIndex = std::find(solidStore->begin(), solidStore->end(), solid) - solidStore->begin();
    unsigned long long solidID = (*m_SolidID)[solidIndex];
    EAxis axis;
    G4int nReplicas;
    G4double width;
    G4double offset;
    G4bool consuming;
    physVol->GetReplicationData(axis, nReplicas, width, offset, consuming);
    physVol->SetCopyNo(replica);
    G4VPVParameterisation* physParameterisation = physVol->GetParameterisation();
    if (physParameterisation) { // parameterised volume
      G4VSolid* solidReplica = physParameterisation->ComputeSolid(replica, physVol);
      physParameterisation->ComputeTransformation(replica, physVol);
      solidReplica->ComputeDimensions(physParameterisation, replica, physVol);
      if (!(*solidReplica == *solid)) {
        std::string solidName = (*m_SolidName)[solidIndex];
        solidName.append("_R");
        solidName.append(std::to_string(replica));
        writeGeometryNode(solidReplica, solidName, solidID + 0x00010000 * replica);
      }
      if (m_UsePrototypes && (*solidReplica == *solid)) {
        if ((replica == 0) && (lvCount == 0)) {
          if (!(*m_LVUnique)[lvIndex]) writeLVModelNode((*lvStore)[lvIndex], lvName, lvID);
        }
      } else {
        // DIVOT lvName.append("_R");
        // DIVOT lvName.append(std::to_string(replica));
        // DIVOT writeLVModelNode((*lvStore)[lvIndex], lvName, lvID+0x00010000*replica+lvCount);
      }
      pvName.append("_R");
      pvName.append(std::to_string(replica));
      writePVModelNode(physVol, pvName, pvID + 0x00010000 * replica + pvCount);
    } else { // plain replicated volume
      G4RotationMatrix* originalRotation = physVol->GetRotation();
      G4ThreeVector translation; // No translation
      G4RotationMatrix rotation; // No rotation
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
            double originalRMin = ((G4Tubs*)solid)->GetInnerRadius();
            double originalRMax = ((G4Tubs*)solid)->GetOuterRadius();
            ((G4Tubs*)solid)->SetInnerRadius(offset + width * replica);
            ((G4Tubs*)solid)->SetOuterRadius(offset + width * (replica + 1));
            std::string solidName = (*m_SolidName)[solidIndex];
            solidName.append("_R");
            solidName.append(std::to_string(replica));
            writeGeometryNode(solid, solidName, (*m_SolidID)[solidIndex] + 0x00010000 * replica);
            ((G4Tubs*)solid)->SetInnerRadius(originalRMin);
            ((G4Tubs*)solid)->SetOuterRadius(originalRMax);
          } else if (replica == 0) {
            B2WARNING("Built-in volumes replicated along radius for " << solid->GetEntityType() <<
                      " (solid " << solid->GetName() << ") are not visualisable.");
          }
          break;
        case kPhi:
          physVol->SetRotation(&(rotation.rotateZ(-(offset + (replica + 0.5) * width))));
          break;
      }
      if (m_UsePrototypes && !((axis == kRho) && (solid->GetEntityType() == "G4Tubs"))) {
        if ((replica == 0) && (lvCount == 0)) {
          if (!(*m_LVUnique)[lvIndex]) writeLVModelNode((*lvStore)[lvIndex], lvName, lvID);
        }
      } else {
        // DIVOT lvName.append("_R");
        // DIVOT lvName.append(std::to_string(replica));
        // DIVOT writeLVModelNode((*lvStore)[lvIndex], lvName, lvID+0x00010000*replica+lvCount);
      }
      pvName.append("_R");
      pvName.append(std::to_string(replica));
      writePVModelNode(physVol, pvName, pvID + 0x00010000 * replica + pvCount);
      if (axis == kPhi) physVol->SetRotation(originalRotation);
    }
  } else {
    if (m_UsePrototypes) {
      if (lvCount == 0) {
        if (!(*m_LVUnique)[lvIndex]) writeLVModelNode((*lvStore)[lvIndex], lvName, lvID);
      }
      if (pvCount == 0) writePVModelNode(physVol, pvName, pvID);
    } else {
      // DIVOT writeLVModelNode((*lvStore)[lvIndex], lvName, lvID+lvCount);
      writePVModelNode(physVol, pvName, pvID + pvCount);
    }
    (*m_LVCount)[lvIndex]++;
    (*m_PVCount)[pvIndex]++;
  }
}

void FBXWriterModule::countEntities(G4VPhysicalVolume* physVol)
{
  // Descend to the leaves of the tree
  G4LogicalVolume* logVol = physVol->GetLogicalVolume();
  for (size_t daughter = 0; daughter < logVol->GetNoDaughters(); ++daughter) {
    G4VPhysicalVolume* physVolDaughter = logVol->GetDaughter(daughter);
    for (int j = 0; j < physVolDaughter->GetMultiplicity(); ++j) {
      countEntities(physVolDaughter);
    }
  }
  // Count replicas and duplicates of each physical and logical volume as well as the unique
  // versions of replicated solids as we ascend the recursive tree
  G4PhysicalVolumeStore* pvStore = G4PhysicalVolumeStore::GetInstance();
  G4LogicalVolumeStore* lvStore = G4LogicalVolumeStore::GetInstance();
  G4SolidStore* solidStore = G4SolidStore::GetInstance();
  G4VSolid* solid = logVol->GetSolid();
  int pvIndex = std::find(pvStore->begin(), pvStore->end(), physVol) - pvStore->begin();
  int lvIndex = std::find(lvStore->begin(), lvStore->end(), logVol) - lvStore->begin();
  int solidIndex = std::find(solidStore->begin(), solidStore->end(), solid) - solidStore->begin();
  if (physVol->IsReplicated()) {
    EAxis axis;
    G4int nReplicas;
    G4double width;
    G4double offset;
    G4bool consuming;
    physVol->GetReplicationData(axis, nReplicas, width, offset, consuming);
    G4VPVParameterisation* physParameterisation = physVol->GetParameterisation();
    if (physParameterisation) { // parameterised volume
      G4VSolid* solidReplica = physParameterisation->ComputeSolid(0, physVol);
      physParameterisation->ComputeTransformation(0, physVol);
      solidReplica->ComputeDimensions(physParameterisation, 0, physVol);
      if (!(*solidReplica == *solid))(*m_SolidReplicas)[solidIndex]++;
      if (m_UsePrototypes && (*solidReplica == *solid)) {
        if ((*m_LVReplicas)[lvIndex] > 0)(*m_LVUnique)[lvIndex] = false;
        (*m_LVReplicas)[lvIndex] = 1;
      } else {
        (*m_LVReplicas)[lvIndex]++;
      }
      (*m_PVReplicas)[pvIndex]++;
    } else { // plain replicated volume
      if ((axis == kRho) && (solid->GetEntityType() == "G4Tubs"))(*m_SolidReplicas)[solidIndex]++;
      if (m_UsePrototypes && !((axis == kRho) && (solid->GetEntityType() == "G4Tubs"))) {
        (*m_LVReplicas)[lvIndex] = 1;
      } else {
        (*m_LVReplicas)[lvIndex]++;
      }
      (*m_PVReplicas)[pvIndex]++;
    }
  } else {
    if ((*m_LVCount)[lvIndex] > 0)(*m_LVUnique)[lvIndex] = false;
    if (m_UsePrototypes) {
      (*m_PVCount)[pvIndex] = 1;
      (*m_LVCount)[lvIndex] = 1;
    } else {
      (*m_PVCount)[pvIndex]++;
      (*m_LVCount)[lvIndex]++;
    }
  }
}

void FBXWriterModule::addConnections(G4VPhysicalVolume* physVol, int replica)
{
  // Write the PhysVolModel-parentLogVolModel connections as we descend the recursive tree.
  // If the parentLogVol is referenced at most once, use its referencing PhysVol instead.
  G4PhysicalVolumeStore* pvStore = G4PhysicalVolumeStore::GetInstance();
  G4LogicalVolumeStore* lvStore = G4LogicalVolumeStore::GetInstance();
  G4SolidStore* solidStore = G4SolidStore::GetInstance();
  G4LogicalVolume* logVol = physVol->GetLogicalVolume();
  int pvIndex = std::find(pvStore->begin(), pvStore->end(), physVol) - pvStore->begin();
  unsigned long long pvID = (*m_PVID)[pvIndex];
  unsigned int pvCount = (*m_PVCount)[pvIndex];
  std::string pvName = (*m_PVName)[pvIndex];
  int lvIndex = std::find(lvStore->begin(), lvStore->end(), logVol) - lvStore->begin();
  unsigned long long lvID = (*m_LVID)[lvIndex];
  unsigned int lvCount = (*m_LVCount)[lvIndex];
  std::string lvName = (*m_LVName)[lvIndex];
  for (size_t daughter = 0; daughter < logVol->GetNoDaughters(); ++daughter) {
    G4VPhysicalVolume* physVolDaughter = logVol->GetDaughter(daughter);
    int pvIndexDaughter = std::find(pvStore->begin(), pvStore->end(), physVolDaughter) - pvStore->begin();
    unsigned long long pvIDDaughter = (*m_PVID)[pvIndexDaughter];
    unsigned int pvCountDaughter = (*m_PVCount)[pvIndexDaughter];
    for (int j = 0; j < physVolDaughter->GetMultiplicity(); ++j) {
      if (m_UsePrototypes) {
        if ((replica == 0) && (j == 0) && (lvCount == 0) && (pvCountDaughter == 0)) {
          if ((*m_LVUnique)[lvIndex]) {
            writePVToParentPV((*m_PVName)[pvIndexDaughter], pvName, pvIDDaughter, pvID);
          } else {
            writePVToParentLV((*m_PVName)[pvIndexDaughter], lvName, pvIDDaughter, lvID);
          }
        }
      } else {
        //writePVToParentLV((*m_PVName)[pvIndexDaughter], lvName, pvIDDaughter+0x00010000*j+pvCountDaughter, lvID+0x00010000*replica+lvCount);
        writePVToParentPV((*m_PVName)[pvIndexDaughter], pvName, pvIDDaughter + 0x00010000 * j + pvCountDaughter,
                          pvID + 0x00010000 * replica + pvCount);
      }
      addConnections(physVolDaughter, j);
    }
  }

  // Write the Geometry-LogVolModel, Material-LogVolModel and PhysVolModel-LogVolModel
  // connections as we ascend the recursive tree
  G4VSolid* solid = logVol->GetSolid();
  int solidIndex = std::find(solidStore->begin(), solidStore->end(), solid) - solidStore->begin();
  unsigned long long solidID = (*m_SolidID)[solidIndex];
  unsigned long long matID = (*m_MatID)[lvIndex];
  std::string solidName = (*m_SolidName)[solidIndex];
  if (physVol->IsReplicated()) {
    pvName.append("_R");
    pvName.append(std::to_string(replica));
    EAxis axis;
    G4int nReplicas;
    G4double width;
    G4double offset;
    G4bool consuming;
    physVol->GetReplicationData(axis, nReplicas, width, offset, consuming);
    physVol->SetCopyNo(replica);
    G4VPVParameterisation* physParameterisation = physVol->GetParameterisation();
    if (physParameterisation) { // parameterised volume
      G4VSolid* solidReplica = physParameterisation->ComputeSolid(replica, physVol);
      physParameterisation->ComputeTransformation(replica, physVol);
      solidReplica->ComputeDimensions(physParameterisation, replica, physVol);
      if (!(*solidReplica == *solid)) {
        solidName.append("_R");
        solidName.append(std::to_string(replica));
        solidID += 0x00010000 * replica;
      }
      if (m_UsePrototypes && (*solidReplica == *solid)) {
        if ((replica == 0) && (lvCount == 0)) {
          if ((*m_LVUnique)[lvIndex]) { // bypass the singleton logical volume
            writeSolidToPV(pvName, solidName, (*m_Visible)[lvIndex], matID, pvID, solidID);
          } else {
            writeSolidToLV(lvName, solidName, (*m_Visible)[lvIndex], matID, lvID, solidID);
          }
        }
      } else {
        lvName.append("_R");
        lvName.append(std::to_string(replica));
        if ((*m_LVUnique)[lvIndex]) { // bypass the singleton logical volume
          writeSolidToPV(pvName, solidName, (*m_Visible)[lvIndex], matID, pvID + 0x00010000 * replica + pvCount, solidID);
        } else {
          writeSolidToLV(lvName, solidName, (*m_Visible)[lvIndex], matID, lvID + 0x00010000 * replica + lvCount, solidID);
        }
      }
      if (!(*m_LVUnique)[lvIndex]) {
        writeLVToPV(pvName, lvName, pvID + 0x00010000 * replica + pvCount, lvID + 0x00010000 * replica + lvCount);
      }
    } else { // plain replicated volume
      if ((axis == kRho) && (solid->GetEntityType() == "G4Tubs")) {
        solidName.append("_R");
        solidName.append(std::to_string(replica));
        solidID += 0x00010000 * replica;
      }
      if (m_UsePrototypes && !((axis == kRho) && (solid->GetEntityType() == "G4Tubs"))) {
        if ((replica == 0) && (lvCount == 0)) {
          if ((*m_LVUnique)[lvIndex]) { // bypass the singleton logical volume
            writeSolidToPV(pvName, solidName, (*m_Visible)[lvIndex], matID, pvID, solidID);
          } else {
            writeSolidToLV(lvName, solidName, (*m_Visible)[lvIndex], matID, lvID, solidID);
          }
        }
      } else {
        lvName.append("_R");
        lvName.append(std::to_string(replica));
        if ((*m_LVUnique)[lvIndex]) { // bypass the singleton logical volume
          writeSolidToPV(pvName, solidName, (*m_Visible)[lvIndex], matID, pvID + 0x00010000 * replica + pvCount, solidID);
        } else {
          writeSolidToLV(lvName, solidName, (*m_Visible)[lvIndex], matID, lvID + 0x00010000 * replica + lvCount, solidID);
        }
      }
      if (!(*m_LVUnique)[lvIndex]) {
        writeLVToPV(pvName, lvName, pvID + 0x00010000 * replica + pvCount, lvID + 0x00010000 * replica + lvCount);
      }
    }
  } else {
    if (m_UsePrototypes) {
      if (lvCount == 0) {
        if ((*m_LVUnique)[lvIndex]) { // bypass the singleton logical volume
          writeSolidToPV(pvName, solidName, (*m_Visible)[lvIndex], matID, pvID, solidID);
        } else {
          writeSolidToLV(lvName, solidName, (*m_Visible)[lvIndex], matID, lvID, solidID);
        }
      }
      if (pvCount == 0) {
        if (!(*m_LVUnique)[lvIndex]) writeLVToPV(pvName, lvName, pvID, lvID);
      }
    } else {
      //writeSolidToLV(lvName, solidName, (*m_Visible)[lvIndex], matID, lvID+lvCount, solidID);
      //writeLVToPV(pvName, lvName, pvID+pvCount, lvID+lvCount);
      writeSolidToPV(pvName, solidName, (*m_Visible)[lvIndex], matID, pvID + pvCount, solidID);
    }
    (*m_LVCount)[lvIndex]++;
    (*m_PVCount)[pvIndex]++;
  }
}

void FBXWriterModule::writePreamble(int modelCount, int materialCount, int geometryCount)
{
  std::time_t t = std::time(nullptr);
  struct tm* now = std::localtime(&t);
  m_File << "; FBX 7.3.0 project file" << std::endl <<
         "; Copyright (C) 1997-2010 Autodesk Inc. and/or its licensors." << std::endl <<
         "; All rights reserved." << std::endl << std::endl <<
         "FBXHeaderExtension:  {" << std::endl <<
         "\tFBXHeaderVersion: 1003" << std::endl <<
         "\tFBXVersion: 7300" << std::endl <<
         "\tCreationTime: \"" << std::put_time(now, "%F %T") << ":000\"" << std::endl <<
         //"\tCreationTimeStamp:  {" << std::endl <<
         //"\t\tVersion: 1000" << std::endl <<
         //"\t\tYear: " << now->tm_year + 1900 << std::endl <<
         //"\t\tMonth: " << now->tm_mon + 1 << std::endl <<
         //"\t\tDay: " << now->tm_mday << std::endl <<
         //"\t\tHour: " << now->tm_hour << std::endl <<
         //"\t\tMinute: " << now->tm_min << std::endl <<
         //"\t\tSecond: " << now->tm_sec << std::endl <<
         //"\t\tMillisecond: 0" << std::endl <<
         //"\t}" << std::endl <<
         "\tCreator: \"FBX SDK/FBX Plugins version 2013.3\"" << std::endl <<
         "\tSceneInfo: \"SceneInfo::GlobalInfo\", \"UserData\" {" << std::endl <<
         "\t\tType: \"UserData\"" << std::endl <<
         "\t\tVersion: 100" << std::endl <<
         "\t\tMetaData:  {" << std::endl <<
         "\t\t\tVersion: 100" << std::endl <<
         "\t\t\tTitle: \"Belle II Detector\"" << std::endl <<
         "\t\t\tSubject: \"Detector Geometry Model\"" << std::endl <<
         "\t\t\tAuthor: \"Belle II Collaboration\"" << std::endl <<
         "\t\t\tKeywords: \"\"" << std::endl <<
         "\t\t\tRevision: \"\"" << std::endl <<
         "\t\t\tComment: \"\"" << std::endl <<
         "\t\t}" << std::endl <<
         "\t}" << std::endl <<
         "}" << std::endl << std::endl;

  m_File << "GlobalSettings:  {" << std::endl <<
         "\tVersion: 1000" << std::endl <<
         "\tProperties70:  {" << std::endl <<
         "\t\tP: \"UpAxis\", \"int\", \"Integer\", \"\",1" << std::endl <<
         "\t\tP: \"UpAxisSign\", \"int\", \"Integer\", \"\",1" << std::endl <<
         "\t\tP: \"FrontAxis\", \"int\", \"Integer\", \"\",2" << std::endl <<
         "\t\tP: \"FrontAxisSign\", \"int\", \"Integer\", \"\",1" << std::endl <<
         "\t\tP: \"CoordAxis\", \"int\", \"Integer\", \"\",0" << std::endl <<
         "\t\tP: \"CoordAxisSign\", \"int\", \"Integer\", \"\",1" << std::endl <<
         "\t\tP: \"OriginalUpAxis\", \"int\", \"Integer\", \"\",1" << std::endl <<
         "\t\tP: \"OriginalUpAxisSign\", \"int\", \"Integer\", \"\",1" << std::endl <<
         "\t\tP: \"UnitScaleFactor\", \"double\", \"Number\", \"\",1" << std::endl <<
         "\t\tP: \"OriginalUnitScaleFactor\", \"double\", \"Number\", \"\",1" << std::endl <<
         "\t\tP: \"AmbientColor\", \"ColorRGB\", \"Color\", \"\",1,1,1" << std::endl <<
         "\t\tP: \"DefaultCamera\", \"KString\", \"\", \"\", \"Producer Perspective\"" << std::endl <<
         "\t\tP: \"TimeMode\", \"enum\", \"\", \"\",0" << std::endl <<
         "\t\tP: \"TimeSpanStart\", \"KTime\", \"Time\", \"\",0" << std::endl <<
         "\t\tP: \"TimeSpanStop\", \"KTime\", \"Time\", \"\",10" << std::endl <<
         "\t\tP: \"CustomFrameRate\", \"double\", \"Number\", \"\",-1" << std::endl <<
         "\t}" << std::endl <<
         "}" << std::endl << std::endl;

  m_File << "Documents:  {" << std::endl <<
         "\tCount: 1" << std::endl <<
         "\tDocument: 4000000000, \"\", \"Scene\" {" << std::endl <<
         "\t\tProperties70:  {" << std::endl <<
         "\t\t\tP: \"SourceObject\", \"object\", \"\", \"\"" << std::endl <<
         "\t\t\tP: \"ActiveAnimStackName\", \"KString\", \"\", \"\", \"\"" << std::endl <<
         "\t\t}" << std::endl <<
         "\t\tRootNode: 0" << std::endl <<
         "\t}" << std::endl <<
         "}" << std::endl << std::endl;

  m_File << "References:  {" << std::endl <<
         "}" << std::endl << std::endl;

  m_File << "Definitions:  {" << std::endl <<
         "\tVersion: 100" << std::endl <<
         "\tCount: 4" << std::endl <<
         "\tObjectType: \"GlobalSettings\" {" << std::endl <<
         "\t\tCount: 1" << std::endl <<
         "\t}" << std::endl;
  m_File << "\tObjectType: \"Model\" {" << std::endl <<
         "\t\tCount: " << modelCount << std::endl <<
         "\t\tPropertyTemplate: \"FbxNode\" {" << std::endl <<
         "\t\t\tProperties70:  {" << std::endl <<
         "\t\t\t\tP: \"QuaternionInterpolate\", \"enum\", \"\", \"\",0" << std::endl <<
         "\t\t\t\tP: \"RotationOffset\", \"Vector3D\", \"Vector\", \"\",0,0,0" << std::endl <<
         "\t\t\t\tP: \"RotationPivot\", \"Vector3D\", \"Vector\", \"\",0,0,0" << std::endl <<
         "\t\t\t\tP: \"ScalingOffset\", \"Vector3D\", \"Vector\", \"\",0,0,0" << std::endl <<
         "\t\t\t\tP: \"ScalingPivot\", \"Vector3D\", \"Vector\", \"\",0,0,0" << std::endl <<
         "\t\t\t\tP: \"TranslationActive\", \"bool\", \"\", \"\",0" << std::endl <<
         "\t\t\t\tP: \"TranslationMin\", \"Vector3D\", \"Vector\", \"\",0,0,0" << std::endl <<
         "\t\t\t\tP: \"TranslationMax\", \"Vector3D\", \"Vector\", \"\",0,0,0" << std::endl <<
         "\t\t\t\tP: \"TranslationMinX\", \"bool\", \"\", \"\",0" << std::endl <<
         "\t\t\t\tP: \"TranslationMinY\", \"bool\", \"\", \"\",0" << std::endl <<
         "\t\t\t\tP: \"TranslationMinZ\", \"bool\", \"\", \"\",0" << std::endl <<
         "\t\t\t\tP: \"TranslationMaxX\", \"bool\", \"\", \"\",0" << std::endl <<
         "\t\t\t\tP: \"TranslationMaxY\", \"bool\", \"\", \"\",0" << std::endl <<
         "\t\t\t\tP: \"TranslationMaxZ\", \"bool\", \"\", \"\",0" << std::endl <<
         "\t\t\t\tP: \"RotationOrder\", \"enum\", \"\", \"\",0" << std::endl <<
         "\t\t\t\tP: \"RotationSpaceForLimitOnly\", \"bool\", \"\", \"\",0" << std::endl <<
         "\t\t\t\tP: \"RotationStiffnessX\", \"double\", \"Number\", \"\",0" << std::endl <<
         "\t\t\t\tP: \"RotationStiffnessY\", \"double\", \"Number\", \"\",0" << std::endl <<
         "\t\t\t\tP: \"RotationStiffnessZ\", \"double\", \"Number\", \"\",0" << std::endl <<
         "\t\t\t\tP: \"AxisLen\", \"double\", \"Number\", \"\",10" << std::endl <<
         "\t\t\t\tP: \"PreRotation\", \"Vector3D\", \"Vector\", \"\",0,0,0" << std::endl <<
         "\t\t\t\tP: \"PostRotation\", \"Vector3D\", \"Vector\", \"\",0,0,0" << std::endl <<
         "\t\t\t\tP: \"RotationActive\", \"bool\", \"\", \"\",0" << std::endl <<
         "\t\t\t\tP: \"RotationMin\", \"Vector3D\", \"Vector\", \"\",0,0,0" << std::endl <<
         "\t\t\t\tP: \"RotationMax\", \"Vector3D\", \"Vector\", \"\",0,0,0" << std::endl <<
         "\t\t\t\tP: \"RotationMinX\", \"bool\", \"\", \"\",0" << std::endl <<
         "\t\t\t\tP: \"RotationMinY\", \"bool\", \"\", \"\",0" << std::endl <<
         "\t\t\t\tP: \"RotationMinZ\", \"bool\", \"\", \"\",0" << std::endl <<
         "\t\t\t\tP: \"RotationMaxX\", \"bool\", \"\", \"\",0" << std::endl <<
         "\t\t\t\tP: \"RotationMaxY\", \"bool\", \"\", \"\",0" << std::endl <<
         "\t\t\t\tP: \"RotationMaxZ\", \"bool\", \"\", \"\",0" << std::endl <<
         "\t\t\t\tP: \"InheritType\", \"enum\", \"\", \"\",0" << std::endl <<
         "\t\t\t\tP: \"ScalingActive\", \"bool\", \"\", \"\",0" << std::endl <<
         "\t\t\t\tP: \"ScalingMin\", \"Vector3D\", \"Vector\", \"\",0,0,0" << std::endl <<
         "\t\t\t\tP: \"ScalingMax\", \"Vector3D\", \"Vector\", \"\",0,0,0" << std::endl <<
         "\t\t\t\tP: \"ScalingMinX\", \"bool\", \"\", \"\",0" << std::endl <<
         "\t\t\t\tP: \"ScalingMinY\", \"bool\", \"\", \"\",0" << std::endl <<
         "\t\t\t\tP: \"ScalingMinZ\", \"bool\", \"\", \"\",0" << std::endl <<
         "\t\t\t\tP: \"ScalingMaxX\", \"bool\", \"\", \"\",0" << std::endl <<
         "\t\t\t\tP: \"ScalingMaxY\", \"bool\", \"\", \"\",0" << std::endl <<
         "\t\t\t\tP: \"ScalingMaxZ\", \"bool\", \"\", \"\",0" << std::endl <<
         "\t\t\t\tP: \"GeometricTranslation\", \"Vector3D\", \"Vector\", \"\",0,0,0" << std::endl <<
         "\t\t\t\tP: \"GeometricRotation\", \"Vector3D\", \"Vector\", \"\",0,0,0" << std::endl <<
         "\t\t\t\tP: \"GeometricScaling\", \"Vector3D\", \"Vector\", \"\",1,1,1" << std::endl <<
         "\t\t\t\tP: \"MinDampRangeX\", \"double\", \"Number\", \"\",0" << std::endl <<
         "\t\t\t\tP: \"MinDampRangeY\", \"double\", \"Number\", \"\",0" << std::endl <<
         "\t\t\t\tP: \"MinDampRangeZ\", \"double\", \"Number\", \"\",0" << std::endl <<
         "\t\t\t\tP: \"MaxDampRangeX\", \"double\", \"Number\", \"\",0" << std::endl <<
         "\t\t\t\tP: \"MaxDampRangeY\", \"double\", \"Number\", \"\",0" << std::endl <<
         "\t\t\t\tP: \"MaxDampRangeZ\", \"double\", \"Number\", \"\",0" << std::endl <<
         "\t\t\t\tP: \"MinDampStrengthX\", \"double\", \"Number\", \"\",0" << std::endl <<
         "\t\t\t\tP: \"MinDampStrengthY\", \"double\", \"Number\", \"\",0" << std::endl <<
         "\t\t\t\tP: \"MinDampStrengthZ\", \"double\", \"Number\", \"\",0" << std::endl <<
         "\t\t\t\tP: \"MaxDampStrengthX\", \"double\", \"Number\", \"\",0" << std::endl <<
         "\t\t\t\tP: \"MaxDampStrengthY\", \"double\", \"Number\", \"\",0" << std::endl <<
         "\t\t\t\tP: \"MaxDampStrengthZ\", \"double\", \"Number\", \"\",0" << std::endl <<
         "\t\t\t\tP: \"PreferedAngleX\", \"double\", \"Number\", \"\",0" << std::endl <<
         "\t\t\t\tP: \"PreferedAngleY\", \"double\", \"Number\", \"\",0" << std::endl <<
         "\t\t\t\tP: \"PreferedAngleZ\", \"double\", \"Number\", \"\",0" << std::endl <<
         "\t\t\t\tP: \"LookAtProperty\", \"object\", \"\", \"\"" << std::endl <<
         "\t\t\t\tP: \"UpVectorProperty\", \"object\", \"\", \"\"" << std::endl <<
         "\t\t\t\tP: \"Show\", \"bool\", \"\", \"\",1" << std::endl <<
         "\t\t\t\tP: \"NegativePercentShapeSupport\", \"bool\", \"\", \"\",1" << std::endl <<
         "\t\t\t\tP: \"DefaultAttributeIndex\", \"int\", \"Integer\", \"\",0" << std::endl <<
         "\t\t\t\tP: \"Freeze\", \"bool\", \"\", \"\",0" << std::endl <<
         "\t\t\t\tP: \"LODBox\", \"bool\", \"\", \"\",0" << std::endl <<
         "\t\t\t\tP: \"Lcl Translation\", \"Lcl Translation\", \"\", \"A\",0,0,0" << std::endl <<
         "\t\t\t\tP: \"Lcl Rotation\", \"Lcl Rotation\", \"\", \"A\",0,0,0" << std::endl <<
         "\t\t\t\tP: \"Lcl Scaling\", \"Lcl Scaling\", \"\", \"A\",1,1,1" << std::endl <<
         "\t\t\t\tP: \"Visibility\", \"Visibility\", \"\", \"A\",1" << std::endl <<
         "\t\t\t\tP: \"Visibility Inheritance\", \"Visibility Inheritance\", \"\", \"\",1" << std::endl <<
         "\t\t\t}" << std::endl <<
         "\t\t}" << std::endl <<
         "\t}" << std::endl;
  m_File << "\tObjectType: \"Material\" {" << std::endl <<
         "\t\tCount: " << materialCount << std::endl <<
         "\t\tPropertyTemplate: \"FbxSurfacePhong\" {" << std::endl <<
         "\t\t\tProperties70:  {" << std::endl <<
         "\t\t\t\tP: \"ShadingModel\", \"KString\", \"\", \"\", \"Phong\"" << std::endl <<
         "\t\t\t\tP: \"MultiLayer\", \"bool\", \"\", \"\",0" << std::endl <<
         "\t\t\t\tP: \"EmissiveColor\", \"ColorRGB\", \"Color\", \"A\",0,0,0" << std::endl <<
         "\t\t\t\tP: \"EmissiveFactor\", \"double\", \"Number\", \"A\",0" << std::endl <<
         "\t\t\t\tP: \"AmbientColor\", \"ColorRGB\", \"Color\", \"A\",0,0,0" << std::endl <<
         "\t\t\t\tP: \"AmbientFactor\", \"double\", \"Number\", \"A\",0" << std::endl <<
         "\t\t\t\tP: \"DiffuseColor\", \"ColorRGB\", \"Color\", \"A\",0,0,0" << std::endl <<
         "\t\t\t\tP: \"DiffuseFactor\", \"double\", \"Number\", \"A\",1" << std::endl <<
         "\t\t\t\tP: \"Bump\", \"Vector3D\", \"Vector\", \"\",0,0,0" << std::endl <<
         "\t\t\t\tP: \"NormalMap\", \"Vector3D\", \"Vector\", \"\",0,0,0" << std::endl <<
         "\t\t\t\tP: \"BumpFactor\", \"double\", \"Number\", \"\",1" << std::endl <<
         "\t\t\t\tP: \"TransparentColor\", \"ColorRGB\", \"Color\", \"A\",0,0,0" << std::endl <<
         "\t\t\t\tP: \"TransparencyFactor\", \"double\", \"Number\", \"A\",0" << std::endl <<
         "\t\t\t\tP: \"DisplacementColor\", \"ColorRGB\", \"Color\", \"\",0,0,0" << std::endl <<
         "\t\t\t\tP: \"DisplacementFactor\", \"double\", \"Number\", \"\",1" << std::endl <<
         "\t\t\t\tP: \"VectorDisplacementColor\", \"ColorRGB\", \"Color\", \"\",0,0,0" << std::endl <<
         "\t\t\t\tP: \"VectorDisplacementFactor\", \"double\", \"Number\", \"\",1" << std::endl <<
         "\t\t\t\tP: \"SpecularColor\", \"ColorRGB\", \"Color\", \"A\",0,0,0" << std::endl <<
         "\t\t\t\tP: \"SpecularFactor\", \"double\", \"Number\", \"A\",0" << std::endl <<
         "\t\t\t\tP: \"ShininessExponent\", \"double\", \"Number\", \"A\",20" << std::endl <<
         "\t\t\t\tP: \"ReflectionColor\", \"ColorRGB\", \"Color\", \"A\",0,0,0" << std::endl <<
         "\t\t\t\tP: \"ReflectionFactor\", \"double\", \"Number\", \"A\",0" << std::endl <<
         "\t\t\t}" << std::endl <<
         "\t\t}" << std::endl <<
         "\t}" << std::endl;
  /*
  m_File << "\tObjectType: \"Material\" {" << std::endl <<
            "\t\tCount: " << materialCount << std::endl <<
            "\t\tPropertyTemplate: \"FbxSurfaceLambert\" {" << std::endl <<
            "\t\t\tProperties70:  {" << std::endl <<
            "\t\t\t\tP: \"ShadingModel\", \"KString\", \"\", \"\", \"Lambet\"" << std::endl <<
            "\t\t\t\tP: \"MultiLayer\", \"bool\", \"\", \"\",0" << std::endl <<
            "\t\t\t\tP: \"EmissiveColor\", \"ColorRGB\", \"Color\", \"A\",0,0,0" << std::endl <<
            "\t\t\t\tP: \"EmissiveFactor\", \"double\", \"Number\", \"A\",0" << std::endl <<
            "\t\t\t\tP: \"AmbientColor\", \"ColorRGB\", \"Color\", \"A\",0,0,0" << std::endl <<
            "\t\t\t\tP: \"AmbientFactor\", \"double\", \"Number\", \"A\",0" << std::endl <<
            "\t\t\t\tP: \"DiffuseColor\", \"ColorRGB\", \"Color\", \"A\",0,0,0" << std::endl <<
            "\t\t\t\tP: \"DiffuseFactor\", \"double\", \"Number\", \"A\",1" << std::endl <<
            "\t\t\t\tP: \"Bump\", \"Vector3D\", \"Vector\", \"\",0,0,0" << std::endl <<
            "\t\t\t\tP: \"NormalMap\", \"Vector3D\", \"Vector\", \"\",0,0,0" << std::endl <<
            "\t\t\t\tP: \"BumpFactor\", \"double\", \"Number\", \"\",1" << std::endl <<
            "\t\t\t\tP: \"TransparentColor\", \"ColorRGB\", \"Color\", \"A\",0,0,0" << std::endl <<
            "\t\t\t\tP: \"TransparencyFactor\", \"double\", \"Number\", \"A\",0" << std::endl <<
            "\t\t\t\tP: \"DisplacementColor\", \"ColorRGB\", \"Color\", \"\",0,0,0" << std::endl <<
            "\t\t\t\tP: \"DisplacementFactor\", \"double\", \"Number\", \"\",1" << std::endl <<
            "\t\t\t\tP: \"VectorDisplacementColor\", \"ColorRGB\", \"Color\", \"\",0,0,0" << std::endl <<
            "\t\t\t\tP: \"VectorDisplacementFactor\", \"double\", \"Number\", \"\",1" << std::endl <<
            "\t\t\t}" << std::endl <<
            "\t\t}" << std::endl <<
            "\t}" << std::endl;
  */
  m_File << "\tObjectType: \"Geometry\" {" << std::endl <<
         "\t\tCount: " << geometryCount << std::endl <<
         "\t\tPropertyTemplate: \"FbxMesh\" {" << std::endl <<
         "\t\t\tProperties70:  {" << std::endl <<
         "\t\t\t\tP: \"Color\", \"ColorRGB\", \"Color\", \"\",1,1,1" << std::endl <<
         "\t\t\t\tP: \"BBoxMin\", \"Vector3D\", \"Vector\", \"\",0,0,0" << std::endl <<
         "\t\t\t\tP: \"BBoxMax\", \"Vector3D\", \"Vector\", \"\",0,0,0" << std::endl <<
         "\t\t\t\tP: \"Primary Visibility\", \"bool\", \"\", \"\",1" << std::endl <<
         "\t\t\t\tP: \"Casts Shadows\", \"bool\", \"\", \"\",0" << std::endl <<
         "\t\t\t\tP: \"Receive Shadows\", \"bool\", \"\", \"\",0" << std::endl <<
         "\t\t\t}" << std::endl <<
         "\t\t}" << std::endl <<
         "\t}" << std::endl <<
         "}" << std::endl << std::endl;

}

void FBXWriterModule::writePolyhedron(G4VSolid* solid, G4Polyhedron* polyhedron, const std::string& name,
                                      unsigned long long solidID)
{
  if (polyhedron) {
    polyhedron->SetNumberOfRotationSteps(120);
    m_File << "\t; Solid " << solid->GetName() << " of type " << solid->GetEntityType() << std::endl <<
           "\tGeometry: " << solidID << ", \"Geometry::" << name << R"(", "Mesh" {)" << std::endl <<
           "\t\tVertices: *" << polyhedron->GetNoVertices() * 3 << " {" << std::endl << "\t\t\ta: ";
    std::streampos startOfLine = m_File.tellp();
    for (int j = 1; j <= polyhedron->GetNoVertices(); ++j) {
      m_File << (j == 1 ? "" : ",") <<
             polyhedron->GetVertex(j).x() << "," <<
             polyhedron->GetVertex(j).y() << "," <<
             polyhedron->GetVertex(j).z();
      if (m_File.tellp() - startOfLine > 100) {
        startOfLine = m_File.tellp();
        m_File << std::endl << "\t\t\t\t";
      }
    }
    m_File << std::endl << "\t\t}" << std::endl;

    std::vector<int> vertices;
    for (int k = 1; k <= polyhedron->GetNoFacets(); ++k) {
      G4bool notLastEdge = true;
      G4int ndx = -1, edgeFlag = 1;
      do {
        notLastEdge = polyhedron->GetNextVertexIndex(ndx, edgeFlag);
        if (notLastEdge) {
          vertices.push_back(ndx - 1);
        } else {
          vertices.push_back(-ndx);
        }
      } while (notLastEdge);
    }
    m_File << "\t\tPolygonVertexIndex: *" << vertices.size() << " {" << std::endl << "\t\t\ta: ";
    startOfLine = m_File.tellp();
    for (unsigned int j = 0; j < vertices.size(); ++j) {
      m_File << (j == 0 ? "" : ",") << vertices[j];
      if (m_File.tellp() - startOfLine > 100) {
        startOfLine = m_File.tellp();
        m_File << std::endl << "\t\t\t\t";
      }
    }
    m_File << std::endl << "\t\t}" << std::endl;

    m_File << "\t\tGeometryVersion: 124" << std::endl <<
           "\t\tLayerElementNormal: 0 {" << std::endl <<
           "\t\t\tVersion: 101" << std::endl <<
           // "\t\t\tName: \"\"" << std::endl <<
           "\t\t\tMappingInformationType: \"ByPolygonVertex\"" << std::endl <<
           "\t\t\tReferenceInformationType: \"Direct\"" << std::endl <<
           "\t\t\tNormals: *" << vertices.size() * 3 << " {" << std::endl << "\t\t\t\ta: ";
    startOfLine = m_File.tellp();
    unsigned int j = 0;
    for (int k = 1; k <= polyhedron->GetNoFacets(); ++k) {
      G4Normal3D normal = polyhedron->GetUnitNormal(k);
      do {
        m_File << (j == 0 ? "" : ",") << normal.x() << "," << normal.y() << "," << normal.z();
        if (m_File.tellp() - startOfLine > 100) {
          startOfLine = m_File.tellp();
          m_File << std::endl << "\t\t\t\t";
        }
      } while (vertices[j++] >= 0);
    }
    m_File << std::endl << "\t\t\t}" << std::endl << "\t\t}" << std::endl <<
           "\t\tLayerElementMaterial: 0 {" << std::endl <<
           "\t\t\tVersion: 101" << std::endl <<
           // "\t\t\tName: \"\"" << std::endl <<
           "\t\t\tMappingInformationType: \"AllSame\"" << std::endl <<
           "\t\t\tReferenceInformationType: \"IndexToDirect\"" << std::endl <<
           "\t\t\tMaterials: *1 {" << std::endl <<
           "\t\t\t\ta: 0" << std::endl <<
           "\t\t\t}" << std::endl <<
           "\t\t}" << std::endl <<
           "\t\tLayer: 0 {" << std::endl <<
           "\t\t\tVersion: 100" << std::endl <<
           "\t\t\tLayerElement:  {" << std::endl <<
           "\t\t\t\tType: \"LayerElementNormal\"" << std::endl <<
           "\t\t\t\tTypedIndex: 0" << std::endl <<
           "\t\t\t}" << std::endl <<
           "\t\t\tLayerElement:  {" << std::endl <<
           "\t\t\t\tType: \"LayerElementMaterial\"" << std::endl <<
           "\t\t\t\tTypedIndex: 0" << std::endl <<
           "\t\t\t}" << std::endl <<
           "\t\t}" << std::endl <<
           "\t}" << std::endl;
  } else {
    B2INFO("Polyhedron representation of solid " << name << " cannot be created");
  }
}

void FBXWriterModule::writePVModelNode(G4VPhysicalVolume* physVol, const std::string& pvName, unsigned long long pvID)
{
  G4RotationMatrix* rot = physVol->GetObjectRotation();
  G4ThreeVector move = physVol->GetObjectTranslation();
  // FBX uses the Tait-Bryan version of the Euler angles (X then Y then Z rotation)
  double yaw = std::atan2(rot->yx(), rot->xx()) * 180.0 / M_PI;
  if (fabs(yaw) < 1.0E-12) yaw = 0.0;
  double pitch = -std::asin(rot->zx()) * 180.0 / M_PI;
  if (fabs(pitch) < 1.0E-12) pitch = 0.0;
  double roll = std::atan2(rot->zy(), rot->zz()) * 180.0 / M_PI;
  if (fabs(roll) < 1.0E-12) roll = 0.0;
  m_File << "\t; PhysVol " << physVol->GetName();
  if (physVol->IsReplicated()) {
    m_File << " (replicated: copy " << physVol->GetCopyNo() << ")";
  }
  m_File << ", placing LogVol " << physVol->GetLogicalVolume()->GetName() << std::endl <<
         "\tModel: " << pvID << ", \"Model::" << pvName << R"(", "Null" {)" << std::endl <<
         "\t\tVersion: 232" << std::endl <<
         "\t\tProperties70:  {" << std::endl <<
         "\t\t\tP: \"Lcl Translation\", \"Lcl Translation\", \"\", \"A\"," <<
         move.x() << "," << move.y() << "," << move.z() << std::endl <<
         "\t\t\tP: \"Lcl Rotation\", \"Lcl Rotation\", \"\", \"A\"," <<
         roll << "," << pitch << "," << yaw << std::endl <<
         "\t\t}" << std::endl <<
         "\t\tShading: T" << std::endl <<
         "\t\tCulling: \"CullingOff\"" << std::endl <<
         "\t}" << std::endl;
}

void FBXWriterModule::writeSolidToLV(const std::string& lvName, const std::string& solidName, bool visible,
                                     unsigned long long matID, unsigned long long lvID, unsigned long long solidID)
{
  m_File << "\t; Solid Geometry::" << solidName << ", LogVol Model::lv_" << lvName << std::endl <<
         "\t" << (visible ? "" : "; ") << "C: \"OO\"," << solidID << "," << lvID << std::endl << std::endl <<
         "\t; Color Material::" << lvName << ", LogVol Model::lv_" << lvName << std::endl <<
         "\t" << (visible ? "" : "; ") << "C: \"OO\"," << matID << "," << lvID << std::endl << std::endl;
}

void FBXWriterModule::writeSolidToPV(const std::string& pvName, const std::string& solidName, bool visible,
                                     unsigned long long matID, unsigned long long pvID, unsigned long long solidID)
{
  m_File << "\t; Solid Geometry::" << solidName << ", PhysVol Model::" << pvName << std::endl <<
         "\t" << (visible ? "" : "; ") << "C: \"OO\"," << solidID << "," << pvID << std::endl << std::endl <<
         "\t; Color Material::" << pvName << ", PhysVol Model::" << pvName << std::endl <<
         "\t" << (visible ? "" : "; ") << "C: \"OO\"," << matID << "," << pvID << std::endl << std::endl;
}

void FBXWriterModule::writeLVToPV(const std::string& pvName, const std::string& lvName, unsigned long long pvID,
                                  unsigned long long lvID)
{
  m_File << "\t; LogVol Model::lv_" << lvName << ", PhysVol Model::" << pvName << std::endl <<
         "\tC: \"OO\"," << lvID << "," << pvID << std::endl << std::endl;
}

void FBXWriterModule::writePVToParentLV(const std::string& pvNameDaughter, const std::string& lvName,
                                        unsigned long long pvIDDaughter, unsigned long long lvID)
{
  m_File << "\t; PhysVol Model::" << pvNameDaughter << ", parent LogVol Model::lv_" << lvName << std::endl <<
         "\tC: \"OO\"," << pvIDDaughter << "," << lvID << std::endl << std::endl;
}

void FBXWriterModule::writePVToParentPV(const std::string& pvNameDaughter, const std::string& pvName,
                                        unsigned long long pvIDDaughter, unsigned long long pvID)
{
  m_File << "\t; PhysVol Model::" << pvNameDaughter << ", parent PhysVol Model::" << pvName << std::endl <<
         "\tC: \"OO\"," << pvIDDaughter << "," << pvID << std::endl << std::endl;
}

// The code in GEANT4 geometry/solids/Boolean/src/G4BooleanSolid.cc is buggy so avoid it.
// Recursively combine the polyhedrons of two solids to make a resulting polyhedron.
HepPolyhedron* FBXWriterModule::getBooleanSolidPolyhedron(G4VSolid* solid)
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
