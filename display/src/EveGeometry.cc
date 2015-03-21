#include <display/EveGeometry.h>

#include <framework/logging/Logger.h>
#include <framework/utilities/FileSystem.h>

#include <TGeoManager.h>
#include <TEveManager.h>
#include <TEveGeoNode.h>
#include <TEveGeoShapeExtract.h>
#include <TEveScene.h>
#include <TFile.h>

#include <cassert>

using namespace Belle2;

void EveGeometry::addGeometry(bool fullgeo)
{
  if (!gGeoManager)
    return;

  B2DEBUG(100, "Setting up geometry for TEve...");
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
  setVolumeColor("PXD.Envelope", kGreen + 3);
  setVolumeColor("SVD.Envelope", kOrange + 8);
  setVolumeColor("logical_ecl", kOrange - 3);
  setVolumeColor("BKLM.EnvelopeLogical", kGreen + 3);
  setVolumeColor("Endcap_1", kGreen + 3);
  setVolumeColor("Endcap_2", kGreen + 3);

  TGeoNode* top_node = gGeoManager->GetTopNode();
  assert(top_node != NULL);
  TEveGeoTopNode* eve_top_node = new TEveGeoTopNode(gGeoManager, top_node);
  eve_top_node->IncDenyDestroy();
  eve_top_node->SetVisLevel(2);
  gEve->AddGlobalElement(eve_top_node);

  //don't show full geo unless turned on by user
  eve_top_node->SetRnrSelfChildren(fullgeo, fullgeo);

  //expand geometry in eve list (otherwise one needs three clicks to see that it has children)
  eve_top_node->ExpandIntoListTreesRecursively();

  B2DEBUG(100, "Loading geometry projections...");

  const std::string extractPath = FileSystem::findFile("/data/display/geometry_extract.root");
  TFile* f = TFile::Open(extractPath.c_str(), "READ");
  TEveGeoShapeExtract* gse = dynamic_cast<TEveGeoShapeExtract*>(f->Get("Extract"));
  TEveGeoShape* gs = TEveGeoShape::ImportShapeExtract(gse, 0);
  gs->SetRnrSelf(false);
  gs->IncDenyDestroy();
  gs->SetName("Minimal geometry extract");
  delete f;

  //I want to show full geo in unprojected view,
  //but I still need to add the extract to the geometry scene...
  gEve->AddGlobalElement(gs);

  gs->SetRnrSelfChildren(false, !fullgeo);
  B2DEBUG(100, "Done.");
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
  TEveGeoTopNode* eve_top_node = dynamic_cast<TEveGeoTopNode*>(gEve->GetGlobalScene()->FirstChild());
  if (!eve_top_node) {
    B2ERROR("Couldn't find TEveGeoTopNode");
    return;
  }
  eve_top_node->ExpandIntoListTrees();
  eve_top_node->SaveExtract("geometry_extract.root", "Extract", false);

  //this doesn't work too well (i.e. crashes when geometry is drawn)
  //eve_top_node->ExpandIntoListTreesRecursively();
  //eve_top_node->SaveExtract("display_geometry_full.root", "Extract", false);
  gGeoManager = my_tgeomanager;
}
