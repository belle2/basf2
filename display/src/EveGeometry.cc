#include <display/EveGeometry.h>

#include <geometry/GeometryManager.h>
#include <framework/logging/Logger.h>
#include <framework/utilities/FileSystem.h>
#include <framework/utilities/ColorPalette.h>

#include <TGeoManager.h>
#include <TPRegexp.h>
#include <TEveManager.h>
#include <TEveGeoNode.h>
#include <TEveGeoShapeExtract.h>
#include <TEveScene.h>
#include <TFile.h>

#include <cassert>

using namespace Belle2;
using namespace Belle2::TangoPalette;

namespace {
  static TEveGeoTopNode* s_eveTopNode = nullptr;
  static TEveGeoShape* s_simplifiedShape = nullptr;
}

void EveGeometry::addGeometry()
{
  B2DEBUG(100, "Setting up geometry for TEve...");
  if (!gEve)
    B2FATAL("gEve must be set up before EveGeometry!");

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

  /*
  //disable display of especially detailed subdetectors
  disableVolume("logical_ecl", true);
  disableVolume("LVCryo");
  //disable all daughters of logical_CDC individually
  TGeoVolume* cdc = gGeoManager->FindVolumeFast("logicalCDC");
  if (!cdc) {
    B2WARNING("Volume logicalCDC not found!");
  } else {
    const int nNodes = cdc->GetNdaughters();
    for (int i = 0; i < nNodes; i++) {
      cdc->GetNode(i)->GetVolume()->SetVisibility(false);
    }
  }
  //reenable CDC boundaries
  enableVolume("logicalInnerWall_0_Shield", false);
  enableVolume("logicalOuterWall_0_Shield", false);
  enableVolume("logicalBackwardCover2", false);
  enableVolume("logicalForwardCover3", false);
  enableVolume("logicalForwardCover4", false);
  //ring things in endcap region
  disableVolume("LVPoleTip");
  disableVolume("LVPoleTip2");
  disableVolume("LVPoleTip3");
  //magnets and beam pipes outside IR (shielding left in)
  disableVolume("logi_A1spc1_name");
  disableVolume("logi_B1spc1_name");
  disableVolume("logi_D1spc1_name");
  disableVolume("logi_E1spc1_name");
  */

  //set some nicer colors (at top level only)
  setVolumeColor("PXD.Envelope", getTColorID("Chameleon", 2));
  setVolumeColor("SVD.Envelope", getTColorID("Orange", 3));
  setVolumeColor("logical_ecl", getTColorID("Orange", 1));
  setVolumeColor("BKLM.EnvelopeLogical", getTColorID("Chameleon", 1));
  setVolumeColor("Endcap_1", getTColorID("Chameleon", 1));
  setVolumeColor("Endcap_2", getTColorID("Chameleon", 1));

  TGeoNode* top_node = gGeoManager->GetTopNode();
  assert(top_node != NULL);
  s_eveTopNode = new TEveGeoTopNode(gGeoManager, top_node);
  s_eveTopNode->IncDenyDestroy();
  s_eveTopNode->SetVisLevel(2);
  gEve->AddGlobalElement(s_eveTopNode);

  //don't show full geo unless turned on by user (will be set by setVisualisationMode())
  bool fullgeo = false;
  s_eveTopNode->SetRnrSelfChildren(fullgeo, fullgeo);

  //expand geometry in eve list (otherwise one needs three clicks to see that it has children)
  s_eveTopNode->ExpandIntoListTreesRecursively();

  B2DEBUG(100, "Loading geometry projections...");

  const std::string extractPath = FileSystem::findFile("/data/display/geometry_extract.root");
  TFile* f = TFile::Open(extractPath.c_str(), "READ");
  TEveGeoShapeExtract* gse = dynamic_cast<TEveGeoShapeExtract*>(f->Get("Extract"));
  s_simplifiedShape = TEveGeoShape::ImportShapeExtract(gse, 0);
  s_simplifiedShape->SetRnrSelf(false);
  s_simplifiedShape->IncDenyDestroy();
  s_simplifiedShape->SetName("Minimal geometry extract");
  delete f;

  //TOP was rotated, let's remove the wrong shapes from the extract
  std::list<TEveElement*> top_bars;
  TPRegexp re("Support.*");
  s_simplifiedShape->FindChildren(top_bars, re);
  B2ASSERT("No TOP bars found?", !top_bars.empty());
  for (TEveElement* el : top_bars) {
    el->Destroy();
  }
  //and add fixed ones instead
  const std::string extractPathTop = FileSystem::findFile("/data/display/geometry_extract_top.root");
  f = TFile::Open(extractPathTop.c_str(), "READ");
  TEveGeoShapeExtract* gsetop = dynamic_cast<TEveGeoShapeExtract*>(f->Get("Extract"));
  TEveGeoShape* top_extract = TEveGeoShape::ImportShapeExtract(gsetop, 0);
  top_extract->SetRnrSelf(false);
  s_simplifiedShape->AddElement(top_extract);
  delete f;

  //I want to show full geo in unprojected view,
  //but I still need to add the extract to the geometry scene...
  gEve->AddGlobalElement(s_simplifiedShape);

  s_simplifiedShape->SetRnrSelfChildren(false, !fullgeo);
  B2DEBUG(100, "Done.");
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
  TObjArray* volumes = gGeoManager->GetListOfVolumes();
  for (int i = 0; i < volumes->GetEntriesFast(); i++) {
    TGeoVolume* volume = static_cast<TGeoVolume*>(volumes->At(i));
    volume->SetTransparency(percent);
  }
}

double EveGeometry::getMaxR()
{
  TGeoVolume* top_node = gGeoManager->GetTopNode()->GetVolume();
  double p[3] = { 380.0, 0.0, 0.0 }; //ok for normal Belle II geometry
  while (!top_node->Contains(p)) {
    p[0] *= 0.8;
  }
  return p[0];
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
