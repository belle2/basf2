/*************************************************************************
*  BASF2 (Belle Analysis Framework 2)                                    *
*  Copyright(C) 2010 - Belle II Collaboration                            *
*                                                                        *
*  Author: The Belle II Collaboration                                    *
*  Contributors: Leo Piilonen                                            *
*                                                                        *
*  This software is provided "as is" without any warranty.               *
*************************************************************************/

#include <bklm/geobklm/GeoBKLMBelleII.h>
#include <bklm/geobklm/BKLMGeometryPar.h>
#include <bklm/simbklm/BKLMSensitiveDetector.h>

#include <framework/gearbox/GearDir.h>
#include <framework/gearbox/Unit.h>

#include <TGeoVolume.h>
#include <TGeoTube.h>
#include <TGeoPgon.h>
#include <TGeoBBox.h>
#include <TGeoCompositeShape.h>

using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Creator
//-----------------------------------------------------------------

GeoBKLMBelleII regGeoBKLMBelleII;

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------
GeoBKLMBelleII::GeoBKLMBelleII() : CreatorBase("BKLMBelleII")
{
  setDescription("Creates the TGeo objects for the BKLM geometry of the Belle II detector.");

  //We use the "klm_barrel_" prefix to flag our sensitive volumes
  addSensitiveDetector("klm_barrel_", new BKLMSensitiveDetector("BKLMSensitiveDetector"));
}

GeoBKLMBelleII::~GeoBKLMBelleII()
{
}

void GeoBKLMBelleII::create(GearDir& content)
{

  //----------------------------------------
  //         Get Materials
  //----------------------------------------

  TGeoMedium* medIron = gGeoManager->GetMedium("Iron");
  TGeoMedium* medAir = gGeoManager->GetMedium("Air");
  TGeoMedium* medAluminum = gGeoManager->GetMedium("Aluminum");
  TGeoMedium* medCopper = gGeoManager->GetMedium("Copper");
  TGeoMedium* medReadout = gGeoManager->GetMedium("RPCReadout");
  TGeoMedium* medGlass = gGeoManager->GetMedium("RPCGlass");
  TGeoMedium* medGas = gGeoManager->GetMedium("RPCGas");
  if (!(medIron && medAir && medAluminum && medCopper && medReadout && medGlass && medGas)) {
    B2FATAL("GeoBKLMBelleII: missing materials(s) during detector creation\n"
            << (medIron     ? "" : "   Iron (defined in BasicMaterials.xml)\n")
            << (medAir      ? "" : "   Air (defined in BasicMaterials.xml)\n")
            << (medAluminum ? "" : "   Aluminum (defined in BasicMaterials.xml)")
            << (medCopper   ? "" : "   Copper (defined in BKLMMaterials.xml)")
            << (medReadout  ? "" : "   Readout (defined in BKLMMaterials.xml)")
            << (medGlass    ? "" : "   Glass (defined in BKLMMaterials.xml)")
            << (medGas      ? "" : "   Gas (defined in BKLMMaterials.xml)"));
  }

  //----------------------------------------
  //           Build BKLM
  //----------------------------------------

  BKLMGeometryPar* geoPar = BKLMGeometryPar::instance();

  TGeoTranslation* noTranslation = new TGeoTranslation(0.0, 0.0, 0.0);
  TGeoRotation* globalRot = new TGeoRotation("BKLMGlobalRotation", 0.0, geoPar->getRotation(), 0.0);
  TGeoVolumeAssembly* volGrpBKLM = addSubdetectorGroup("BKLM", new TGeoCombiTrans(0.0, 0.0, geoPar->getOffsetZ(), globalRot));

  double sectorDphi = 360.0 / geoPar->getNSector();
  CLHEP::Hep3Vector size = geoPar->getGapSize(0, false);
  double gapLength = size.z() * 2.0;
  double ribShift0 = (geoPar->getRibThickness() / 2.0) / sin(M_PI / geoPar->getNSector());
  TGeoTranslation* ribShift        = new TGeoTranslation("ribShift", -ribShift0, 0.0, 0.0);
  TGeoTranslation* ribShiftInverse = new TGeoTranslation(ribShift0, 0.0, 0.0);
  ribShift->RegisterYourself();
  double dx = geoPar->getModuleGlassHeight() + geoPar->getModuleGasHeight() / 2.0;
  TGeoTranslation* innerGasTranslation = new TGeoTranslation(-dx, 0.0, 0.0);
  TGeoTranslation* outerGasTranslation = new TGeoTranslation(+dx, 0.0, 0.0);
  TGeoRotation* chimneyRot = new TGeoRotation();
  chimneyRot->RotateY(90.0);
  TGeoCombiTrans* chimneyTransform = new TGeoCombiTrans(0.0, 0.0, 0.0, chimneyRot);

  TGeoVolume* fbVol;
  TGeoCompositeShape* fbShape;
  TGeoVolume* sectorVol;
  TGeoCompositeShape* sectorShape;
  TGeoVolume* layerVol;
  TGeoPgon*   layerShape;
  TGeoVolume* gapVol;
  TGeoVolume* chimneyGapVol;
  TGeoVolume* chimneyVol;
  TGeoVolume* chimneyHousingVol;
  TGeoVolume* chimneyShieldVol;
  TGeoVolume* chimneyPipeVol;
  TGeoVolume* moduleVol;
  TGeoVolume* readoutVol;
  TGeoVolume* electrodeVol;
  TGeoVolume* gasVol;
  TGeoVolume* capVol;
  TGeoCompositeShape* capShape;
  TGeoVolume* kiteVol;
  TGeoVolume* cableVol;
  TGeoVolume* braceVol;
  TGeoCompositeShape* voidShape;
  TGeoVolume* voidVol;
  TGeoVolume* supportPlateVol;
  TGeoPgon*   temp;
  char name[4];
  char gasName[30];
  TGeoTube* solenoidCutoutShape = new TGeoTube(0.0, geoPar->getSolenoidOuterRadius(), geoPar->getHalfLength() * 1.01);
  solenoidCutoutShape->SetName("solenoidCutout");
  for (int fb = 0; fb < 2; fb++) {
    temp = new TGeoPgon("BKLMpgon", geoPar->getPhi(), 360.0, geoPar->getNSector(), 2);
    temp->DefineSection(0, 0.0,                     0.0, geoPar->getOuterRadius());
    temp->DefineSection(1, geoPar->getHalfLength(), 0.0, geoPar->getOuterRadius());
    fbShape = new TGeoCompositeShape((fb == 0 ? "BKLM_F" : "BKLM_B"), "BKLMpgon-solenoidCutout");
    fbVol = new TGeoVolume((fb == 0 ? "BKLM_F" : "BKLM_B"), fbShape, medAir);
    fbVol->SetAttVisibility(true);
    fbVol->SetLineColor(kBlue);
    TGeoRotation* fbRot = new TGeoRotation("BKLMfbRotation", 0.0, 0.0, 0.0);
    if (fb == 1) { fbRot->ReflectZ(true); }
    volGrpBKLM->AddNode(fbVol, fb, new TGeoCombiTrans(0.0, 0.0, 0.0, fbRot));
    for (int sect = 0; sect < geoPar->getNSector(); sect++) {
      bool hasChimney = (fb == 1) && (sect == 2);
      sprintf(name, "S%d", sect);
      temp = new TGeoPgon("Sectorpgon", geoPar->getPhi(), sectorDphi, 1, 2);
      temp->DefineSection(0, 0.0,                     0.0, geoPar->getOuterRadius());
      temp->DefineSection(1, geoPar->getHalfLength(), 0.0, geoPar->getOuterRadius());
      sectorShape = new TGeoCompositeShape("Sector", "Sectorpgon-solenoidCutout");
      sectorVol = new TGeoVolume(name, sectorShape, medIron);
      sectorVol->SetAttVisibility(true);
      sectorVol->SetLineColor(kBlue);
      TGeoRotation* sectorRot = new TGeoRotation("BKLMSectorRotation", 0.0, 0.0, sectorDphi * sect);
      fbVol->AddNode(sectorVol, sect, new TGeoCombiTrans(0.0, 0.0, 0.0, sectorRot));
      // Install layers in the sector
      for (int lyr = 0; lyr < geoPar->getNLayer() ; lyr++) {
        sprintf(name, "L%02d", lyr);
        layerVol = gGeoManager->MakePgon(name, medIron, geoPar->getPhi(), sectorDphi, 1, 2);
        layerShape = (TGeoPgon*)layerVol->GetShape();
        layerShape->DefineSection(0, 0.0,       geoPar->getLayerInnerRadius(lyr), geoPar->getLayerOuterRadius(lyr));
        layerShape->DefineSection(1, gapLength, geoPar->getLayerInnerRadius(lyr), geoPar->getLayerOuterRadius(lyr));
        layerVol->SetAttVisibility(true);
        layerVol->SetLineColor(kGreen);
        sectorVol->AddNode(layerVol, lyr, noTranslation);
        size = geoPar->getGapSize(lyr, hasChimney);
        double gapZ = size.z();
        sprintf(name, "G%02d", lyr);
        gapVol = gGeoManager->MakeBox(name, medAir, size.x(), size.y(), size.z());
        gapVol->SetAttVisibility(true);
        gapVol->SetLineColor(kRed);
        layerVol->AddNode(gapVol, 0, new TGeoTranslation(geoPar->getGapMiddleRadius(lyr), 0.0, size.z()));
        if (hasChimney) {
          CLHEP::Hep3Vector sizeChimney = geoPar->getChimneySize(lyr);
          CLHEP::Hep3Vector posChimney = geoPar->getChimneyPosition(lyr);
          size.setY((size.y() - sizeChimney.y()) / 2.0);
          size.setZ(sizeChimney.z());
          sprintf(name, "H%02d", lyr);
          chimneyGapVol = gGeoManager->MakeBox(name, medAir, size.x(), size.y(), size.z());
          chimneyGapVol->SetAttVisibility(true);
          chimneyGapVol->SetLineColor(kRed);
          layerVol->AddNode(chimneyGapVol, 1, new TGeoTranslation(geoPar->getGapMiddleRadius(lyr), +(size.y() + sizeChimney.y()), gapLength - size.z()));
          layerVol->AddNode(chimneyGapVol, 2, new TGeoTranslation(geoPar->getGapMiddleRadius(lyr), -(size.y() + sizeChimney.y()), gapLength - size.z()));
          sizeChimney.setZ(sizeChimney.z() - geoPar->getChimneyCoverThickness() / 2.0);
          posChimney.setZ(posChimney.z() - geoPar->getChimneyCoverThickness() / 2.0);
          sprintf(name, "C%02d", lyr);
          chimneyVol = gGeoManager->MakeBox(name, medAir, sizeChimney.x(), sizeChimney.y(), sizeChimney.z());
          chimneyVol->SetAttVisibility(true);
          chimneyVol->SetLineColor(kBlue);
          layerVol->AddNode(chimneyVol, 3, new TGeoTranslation(posChimney.x(), posChimney.y(), posChimney.z()));
          chimneyHousingVol = gGeoManager->MakeTube("chimneyHousing", medIron, geoPar->getChimneyHousingInnerRadius(), geoPar->getChimneyHousingOuterRadius(), sizeChimney.x());
          chimneyVol->AddNode(chimneyHousingVol, 0, chimneyTransform);
          chimneyShieldVol = gGeoManager->MakeTube("chimneyShield", medCopper, geoPar->getChimneyShieldInnerRadius(), geoPar->getChimneyShieldOuterRadius(), sizeChimney.x());
          chimneyVol->AddNode(chimneyShieldVol, 1, chimneyTransform);
          chimneyPipeVol = gGeoManager->MakeTube("chimneyPipe", medAluminum, geoPar->getChimneyPipeInnerRadius(), geoPar->getChimneyPipeOuterRadius(), sizeChimney.x());
          chimneyVol->AddNode(chimneyPipeVol, 2, chimneyTransform);
        }
        sprintf(name,  "M%02d", lyr);
        size = geoPar->getModuleSize(lyr, hasChimney);
        moduleVol = gGeoManager->MakeBox(name, medAluminum, size.x(), size.y(), size.z());
        moduleVol->SetAttVisibility(true);
        moduleVol->SetLineColor(kBlack);
        gapVol->AddNode(moduleVol, 0, new TGeoTranslation(geoPar->getModuleMiddleRadius(lyr) - geoPar->getGapMiddleRadius(lyr), 0.0, size.z() - gapZ));
        sprintf(name,  "R%02d", lyr);
        size = geoPar->getReadoutSize(lyr, hasChimney);
        readoutVol = gGeoManager->MakeBox(name, medReadout, size.x(), size.y(), size.z());
        readoutVol->SetAttVisibility(true);
        readoutVol->SetLineColor(kBlack);
        moduleVol->AddNode(readoutVol, 0, noTranslation);
        sprintf(name,  "E%02d", lyr);
        size = geoPar->getElectrodeSize(lyr, hasChimney);
        electrodeVol = gGeoManager->MakeBox(name, medGlass, size.x(), size.y(), size.z());
        electrodeVol->SetAttVisibility(true);
        electrodeVol->SetLineColor(kBlack);
        readoutVol->AddNode(electrodeVol, 0, noTranslation);
        sprintf(gasName,  "klm_barrel_gas_%d_%d_%02d_0", fb, sect, lyr);
        size = geoPar->getGasSize(lyr, hasChimney);
        gasVol = gGeoManager->MakeBox(gasName, medGas, size.x(), size.y(), size.z());
        gasVol->SetAttVisibility(true);
        gasVol->SetLineColor(kBlack);
        electrodeVol->AddNode(gasVol, 0, innerGasTranslation);
        sprintf(gasName,  "klm_barrel_gas_%d_%d_%02d_1", fb, sect, lyr);
        gasVol = gGeoManager->MakeBox(gasName, medGas, size.x(), size.y(), size.z());
        gasVol->SetAttVisibility(true);
        gasVol->SetLineColor(kBlack);
        electrodeVol->AddNode(gasVol, 1, outerGasTranslation);
      }
      // Install cap at the +Z end of the sector - this contains the support kites/bar and "cables"
      sprintf(name, "C%d", sect);
      temp = new TGeoPgon("Cappgon", geoPar->getPhi(), sectorDphi, 1, 2);
      temp->DefineSection(0, gapLength,        0.0, geoPar->getOuterRadius());
      temp->DefineSection(1, geoPar->getHalfLength(), 0.0, geoPar->getOuterRadius());
      capShape = new TGeoCompositeShape("Cap", "Cappgon-solenoidCutout");
      capVol = new TGeoVolume(name, capShape, medAir);
      sectorVol->AddNode(capVol, geoPar->getNLayer(), noTranslation);
      size = geoPar->getGapSize(0, false);
      double rminKite = geoPar->getLayerInnerRadius(1) + size.x() * 2.0;
      kiteVol = gGeoManager->MakePgon("barrelkite", medIron, geoPar->getPhi(), sectorDphi, 1, 2);
      TGeoPgon* kiteShape = (TGeoPgon*)kiteVol->GetShape();
      kiteShape->DefineSection(0, gapLength,        geoPar->getLayerInnerRadius(1) + size.x() * 2.0, geoPar->getOuterRadius());
      kiteShape->DefineSection(1, geoPar->getHalfLength(), geoPar->getLayerInnerRadius(1) + size.x() * 2.0, geoPar->getOuterRadius());
      kiteVol->SetAttVisibility(true);
      kiteVol->SetLineColor(kGreen);
      capVol->AddNode(kiteVol, 0, noTranslation);
      size.setX((geoPar->getOuterRadius() - rminKite) / 2.0);
      size.setY(geoPar->getCablesWidth() / 2.0);
      size.setZ((geoPar->getHalfLength() - gapLength) / 2.0);
      cableVol = gGeoManager->MakeBox("barrelcables", medAluminum, size.x(), size.y(), size.z());
      cableVol->SetAttVisibility(true);
      cableVol->SetLineColor(kRed);
      TGeoTranslation* cableTranslate = new TGeoTranslation((geoPar->getOuterRadius() + rminKite) / 2.0, 0.0, (gapLength + geoPar->getHalfLength()) / 2.0);
      kiteVol->AddNode(cableVol, 0, cableTranslate);
      size.setY((hasChimney ? geoPar->getBraceWidthChimney() : geoPar->getBraceWidth()) / 2.0);
      braceVol = gGeoManager->MakeBox("barrelbrace", medIron, size.x(), size.y(), size.z());
      braceVol->SetAttVisibility(true);
      braceVol->SetLineColor(kBlue);
      cableVol->AddNode(braceVol, 0, noTranslation);
      // Install inner void in the sector - this contains the support structure for innermost detector
      // "ribShift" leaves radial solenoid-support iron ribs at low- and high-phi sides of the sector
      sprintf(name, "V%d", sect);
      temp = new TGeoPgon("Voidpgon", geoPar->getPhi(), sectorDphi, 1, 2);
      temp->DefineSection(0, 0.0,       0.0, geoPar->getLayerInnerRadius(0) - ribShift0);
      temp->DefineSection(1, gapLength, 0.0, geoPar->getLayerInnerRadius(0) - ribShift0);
      voidShape = new TGeoCompositeShape("Void", "Voidpgon-solenoidCutout:ribShift");
      voidVol = new TGeoVolume(name, voidShape, medAir);
      sectorVol->AddNode(voidVol, geoPar->getNLayer() + 1, ribShiftInverse);
      if (sect <= geoPar->getNSector() / 2) {
        size = geoPar->getSupportPlateSize(hasChimney);
        supportPlateVol = gGeoManager->MakeBox("supportplate", medAluminum, size.x(), size.y(), size.z());
        voidVol->AddNode(supportPlateVol, 0, new TGeoTranslation(geoPar->getLayerInnerRadius(0) - size.x() - ribShift0, 0.0, size.z()));
        // *DIVOT* missing the iron brackets that hold this support plate
      }
    }
  }

}
