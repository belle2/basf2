/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <display/EveGeometry.h>

#include <geometry/GeometryManager.h>
#include <framework/logging/Logger.h>
#include <framework/utilities/FileSystem.h>
#include <framework/utilities/ColorPalette.h>

#include <TGeoManager.h>
#include <TPRegexp.h>
#include <TEveManager.h>
#include <TEveGeoNode.h>
#include <TEveGeoShape.h>
#include <TEveGeoShapeExtract.h>
#include <TFile.h>

#include <cassert>

using namespace Belle2;
using namespace Belle2::TangoPalette;

namespace {
  static TEveGeoTopNode* s_eveTopNode = nullptr;
  static TEveGeoShape* s_simplifiedShape = nullptr;
  static std::vector<std::string> s_hideVolumes = {};
  static std::vector<std::string> s_deleteVolumes = {};
  static std::string s_eveGeometryExtractPath = "/data/display/geometry_extract.root";
}

void EveGeometry::addGeometry(EType visMode)
{
  B2DEBUG(100, "Setting up geometry for TEve...");
  if (!gEve)
    B2FATAL("gEve must be set up before EveGeometry!");

  if (visMode == c_Full) {
    if (!gGeoManager) { //TGeo geometry not initialized, do it ourselves
      //convert geant4 geometry to TGeo geometry
      geometry::GeometryManager& geoManager = geometry::GeometryManager::getInstance();
      geoManager.createTGeoRepresentation();
      if (!gGeoManager) {
        B2FATAL("Couldn't create TGeo geometry! Please make sure you add the Geometry module before Display");
        return;
      }
    }
    //set colours by atomic mass number
    gGeoManager->DefaultColors();

    setTransparency(90);
    //set some nicer colors (at top level only)
    setVolumeColor("PXD.Envelope", getTColorID("Chameleon", 2));
    setVolumeColor("SVD.Envelope", getTColorID("Orange", 3));
    setVolumeColor("logical_ecl", getTColorID("Orange", 1));
    setVolumeColor("BKLM.EnvelopeLogical", getTColorID("Chameleon", 1));
    setVolumeColor("Endcap_1", getTColorID("Chameleon", 1));
    setVolumeColor("Endcap_2", getTColorID("Chameleon", 1));

    for (auto& volume : s_hideVolumes)
      disableVolume(volume.c_str(), false);

    TGeoNode* top_node = gGeoManager->GetTopNode();
    assert(top_node != NULL);
    s_eveTopNode = new TEveGeoTopNode(gGeoManager, top_node);
    s_eveTopNode->IncDenyDestroy();
    s_eveTopNode->SetVisLevel(2);

    gEve->AddGlobalElement(s_eveTopNode);

    //expand geometry in eve list (otherwise one needs three clicks to see that it has children)
    s_eveTopNode->ExpandIntoListTreesRecursively();

  }
  B2DEBUG(100, "Loading geometry projections...");

  const std::string extractPath = FileSystem::findFile(s_eveGeometryExtractPath);
  TFile* f = TFile::Open(extractPath.c_str(), "READ");
  TEveGeoShapeExtract* gse = dynamic_cast<TEveGeoShapeExtract*>(f->Get("Extract"));
  s_simplifiedShape = TEveGeoShape::ImportShapeExtract(gse, 0);
  s_simplifiedShape->SetRnrSelf(false);
  s_simplifiedShape->IncDenyDestroy();
  s_simplifiedShape->SetName("Minimal geometry extract");
  delete f;

  //I want to show full geo in unprojected view,
  //but I still need to add the extract to the geometry scene...
  gEve->AddGlobalElement(s_simplifiedShape);

  setVisualisationMode(visMode);

  // Allow deletion only for full geometry
  if (s_eveTopNode) {
    for (auto& volumeRegExp : s_deleteVolumes) {
      removeChildrenByRegExp(s_eveTopNode, volumeRegExp);
    }
  }

  B2DEBUG(100, "Done.");
}

void EveGeometry::removeChildrenByRegExp(TEveElement* parent, const std::string& pattern)
{
  TPRegexp reAll(".*");
  bool onlyChildren = false;
  std::string regexp = pattern;

  // For patterns with leading '#', do delete only
  // children elements (and remove the '#' for regexp)
  if (pattern.substr(0, 1) == std::string("#")) {
    regexp = pattern.substr(1);
    onlyChildren = true;
  }

  TPRegexp reMatch(regexp.c_str());

  std::list<TEveElement*> children;
  parent->FindChildren(children, reAll);

  for (TEveElement* child : children) {
    if (reMatch.MatchB(child->GetElementName())) {
      if (onlyChildren) {
        B2INFO("Removing children of " << child->GetElementName());
        child->DestroyElements();
      } else {
        B2INFO("Removing " << child->GetElementName());
        child->Destroy();
      }
    } else {
      removeChildrenByRegExp(child, pattern);
    }
  }
}

void EveGeometry::setVisualisationMode(EType visMode)
{
  bool fullgeo = (visMode == c_Full);
  if (s_eveTopNode)
    s_eveTopNode->SetRnrSelfChildren(fullgeo, fullgeo);
  s_simplifiedShape->SetRnrSelfChildren(false, !fullgeo);
}

void EveGeometry::enableVolume(const char* name, bool only_daughters, bool enable)
{
  if (!gGeoManager) return;
  TGeoVolume* vol = gGeoManager->FindVolumeFast(name);
  if (vol) {
    if (!only_daughters)
      vol->SetVisibility(enable);
    vol->SetVisDaughters(enable);
  } else {
    B2DEBUG(100, "Volume " << name << " not found?");
  }
}

void EveGeometry::disableVolume(const char* name, bool only_daughters) { enableVolume(name, only_daughters, false); }

void EveGeometry::setVolumeColor(const char* name, Color_t col)
{
  if (!gGeoManager) return;
  TGeoVolume* vol = gGeoManager->FindVolumeFast(name);
  if (vol) {
    //while TGeoVolume derives from TAttFill, the line color actually is important here
    vol->SetLineColor(col);
  } else {
    B2DEBUG(100, "Volume " << name << " not found?");
  }
}

void EveGeometry::setTransparency(int percent)
{
  if (!gGeoManager) return;
  TObjArray* volumes = gGeoManager->GetListOfVolumes();
  for (int i = 0; i < volumes->GetEntriesFast(); i++) {
    TGeoVolume* volume = static_cast<TGeoVolume*>(volumes->At(i));
    volume->SetTransparency(percent);
  }
}

double EveGeometry::getMaxR()
{
  if (gGeoManager && gGeoManager->GetTopNode()) {
    TGeoVolume* top_node = gGeoManager->GetTopNode()->GetVolume();
    double p[3] = { 380.0, 0.0, 0.0 }; //ok for normal Belle II geometry
    while (!top_node->Contains(p)) {
      p[0] *= 0.8;
    }
    return p[0];
  } else if (s_simplifiedShape) {
    s_simplifiedShape->ComputeBBox();
    const float* bbox = s_simplifiedShape->GetBBox();
    return std::min(380.f, std::max({std::abs(bbox[0]), bbox[1], std::abs(bbox[2]), bbox[2]}));
  }
  // fallback to something reasonable
  return 380.;
}

void EveGeometry::saveExtract()
{
  TGeoManager* my_tgeomanager = gGeoManager;
  if (!s_eveTopNode) {
    B2ERROR("Couldn't find TEveGeoTopNode");
    return;
  }
  s_eveTopNode->ExpandIntoListTrees();
  s_eveTopNode->SaveExtract("geometry_extract.root", "Extract", false);

  //this doesn't work too well (i.e. crashes when geometry is drawn)
  //s_eveTopNode->ExpandIntoListTreesRecursively();
  //s_eveTopNode->SaveExtract("display_geometry_full.root", "Extract", false);
  gGeoManager = my_tgeomanager;
}

void EveGeometry::setCustomExtractPath(const std::string& extractPath)
{
  s_eveGeometryExtractPath = extractPath;
}

void EveGeometry::setHideVolumes(const std::vector<std::string>& volumes)
{
  s_hideVolumes = volumes;
}

void EveGeometry::setDeleteVolumes(const std::vector<std::string>& volumes)
{
  s_deleteVolumes = volumes;
}
